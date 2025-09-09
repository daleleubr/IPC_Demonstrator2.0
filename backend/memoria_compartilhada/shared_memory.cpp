#include "shared_memory.hpp"
#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>
static std::string escape_json(const std::string& s) {
    std::ostringstream o;
    for (unsigned char ch : s) {
        switch (ch) {
        case '\"': o << "\\\""; break;
        case '\\': o << "\\\\"; break;
        case '\b': o << "\\b";  break;
        case '\f': o << "\\f";  break;
        case '\n': o << "\\n";  break;
        case '\r': o << "\\r";  break;
        case '\t': o << "\\t";  break;
        default:
            if (ch < 0x20) {
                o << "\\u"
                    << std::hex << std::setw(4) << std::setfill('0')
                    << static_cast<int>(ch);
                o << std::dec;
            }
            else {
                o << static_cast<char>(ch);
            }
        }
    }
    return o.str();
}

SharedMemory::SharedMemory(const std::string& name, size_t size)
    : shm_name_(name),
    sem_write_name_(name + "_write_sem"),
    sem_read_name_(name + "_read_sem"),
    shm_size_(size + 1),
    hMapFile_(NULL),
    shm_ptr_(nullptr),
    sem_write_(NULL),
    sem_read_(NULL),
    is_initialized_(false) {
}

SharedMemory::~SharedMemory() {
    cleanup();
}

bool SharedMemory::initialize() {
    if (!open_shared_memory()) {
        if (!create_shared_memory()) {
            std::cerr << "Failed to create shared memory" << std::endl;
            return false;
        }
    }

    if (!open_semaphores()) {
        if (!create_semaphores()) {
            std::cerr << "Failed to create semaphores" << std::endl;
            cleanup();
            return false;
        }
    }

    is_initialized_ = true;
    return true;
}

bool SharedMemory::create_shared_memory() {
    hMapFile_ = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        (DWORD)shm_size_,
        shm_name_.c_str()
    );

    if (!hMapFile_) {
        std::cerr << "CreateFileMapping failed: " << GetLastError() << std::endl;
        return false;
    }

    shm_ptr_ = MapViewOfFile(hMapFile_, FILE_MAP_ALL_ACCESS, 0, 0, shm_size_);
    if (!shm_ptr_) {
        std::cerr << "MapViewOfFile failed: " << GetLastError() << std::endl;
        CloseHandle(hMapFile_);
        hMapFile_ = NULL;
        return false;
    }

    memset(shm_ptr_, 0, shm_size_);
    return true;
}

bool SharedMemory::open_shared_memory() {
    hMapFile_ = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, shm_name_.c_str());
    if (!hMapFile_) return false;

    shm_ptr_ = MapViewOfFile(hMapFile_, FILE_MAP_ALL_ACCESS, 0, 0, shm_size_);
    if (!shm_ptr_) {
        std::cerr << "MapViewOfFile failed: " << GetLastError() << std::endl;
        CloseHandle(hMapFile_);
        hMapFile_ = NULL;
        return false;
    }

    return true;
}

bool SharedMemory::create_semaphores() {
    sem_write_ = CreateSemaphoreA(NULL, 1, 1, sem_write_name_.c_str());
    if (!sem_write_) {
        std::cerr << "CreateSemaphore write failed: " << GetLastError() << std::endl;
        return false;
    }

    sem_read_ = CreateSemaphoreA(NULL, 0, 1, sem_read_name_.c_str());
    if (!sem_read_) {
        std::cerr << "CreateSemaphore read failed: " << GetLastError() << std::endl;
        CloseHandle(sem_write_);
        sem_write_ = NULL;
        return false;
    }

    return true;
}

bool SharedMemory::open_semaphores() {
    sem_write_ = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, sem_write_name_.c_str());
    if (!sem_write_) return false;

    sem_read_ = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, sem_read_name_.c_str());
    if (!sem_read_) {
        CloseHandle(sem_write_);
        sem_write_ = NULL;
        return false;
    }

    return true;
}

bool SharedMemory::write_data(const std::string& data) {
    if (!is_initialized_) return false;
    if (data.size() >= shm_size_) return false;

    if (WaitForSingleObject(sem_write_, INFINITE) != WAIT_OBJECT_0) return false;

    memcpy(shm_ptr_, data.c_str(), data.size());
    static_cast<char*>(shm_ptr_)[data.size()] = '\0';

    ReleaseSemaphore(sem_read_, 1, NULL);
    return true;
}

std::string SharedMemory::read_data() {
    if (!is_initialized_) return "ERROR: Shared memory not initialized";

    // Tenta consumir o semáforo sem bloquear (timeout 0 ms).
    DWORD w = WaitForSingleObject(sem_read_, 0);
    if (w == WAIT_OBJECT_0) {
        // Havia dado sinalizado: consome e libera escrita
        std::string data(static_cast<char*>(shm_ptr_));
        ReleaseSemaphore(sem_write_, 1, NULL);
        return data;
    }
    if (w == WAIT_TIMEOUT) {
        // Ninguém sinalizou (ex.: novo processo read). Retorna conteúdo atual mesmo assim.
        return std::string(static_cast<char*>(shm_ptr_));
    }
    // Qualquer erro inesperado
    return "ERROR: read wait failed";
}


void SharedMemory::clear_memory() {
    if (shm_ptr_ && is_initialized_) {
        memset(shm_ptr_, 0, shm_size_);
    }
}

std::string SharedMemory::get_status_json() const {
    std::ostringstream os;

    std::string content;
    if (is_initialized_ && shm_ptr_) {
        content = static_cast<char*>(shm_ptr_);
    }

    os << "{"
        << "\"shared_memory\":{"
        << "\"name\":\"" << escape_json(shm_name_) << "\","
        << "\"size\":" << (unsigned long long)shm_size_ << ","
        << "\"initialized\":" << (is_initialized_ ? "true" : "false") << ","
        << "\"content\":\"" << escape_json(content) << "\","
        << "\"content_length\":" << (unsigned long long)content.size()
        << "},"
        << "\"semaphores\":{"
        << "\"write_semaphore\":{"
        << "\"name\":\"" << escape_json(sem_write_name_) << "\","
        << "\"available\":" << (sem_write_ ? "true" : "false")
        << "},"
        << "\"read_semaphore\":{"
        << "\"name\":\"" << escape_json(sem_read_name_) << "\","
        << "\"available\":" << (sem_read_ ? "true" : "false")
        << "}"
        << "}"
        << "}";

    return os.str();
}

void SharedMemory::cleanup() {
    if (shm_ptr_) {
        UnmapViewOfFile(shm_ptr_);
        shm_ptr_ = nullptr;
    }
    if (hMapFile_) {
        CloseHandle(hMapFile_);
        hMapFile_ = NULL;
    }
    if (sem_write_) {
        CloseHandle(sem_write_);
        sem_write_ = NULL;
    }
    if (sem_read_) {
        CloseHandle(sem_read_);
        sem_read_ = NULL;
    }
    is_initialized_ = false;
}
