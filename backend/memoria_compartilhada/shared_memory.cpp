#include "shared_memory.hpp"
#include <iostream>
#include <cstring>
#include <windows.h>
#include <sstream>
#include <iomanip>

// Função para escapar caracteres especiais no JSON
std::string escape_json(const std::string& s) {
    std::ostringstream o;
    for (auto c = s.cbegin(); c != s.cend(); c++) {
        switch (*c) {
        case '"': o << "\\\""; break;
        case '\\': o << "\\\\"; break;
        case '\b': o << "\\b"; break;
        case '\f': o << "\\f"; break;
        case '\n': o << "\\n"; break;
        case '\r': o << "\\r"; break;
        case '\t': o << "\\t"; break;
        default:
            if ('\x00' <= *c && *c <= '\x1f') {
                o << "\\u"
                    << std::hex << std::setw(4) << std::setfill('0') << (int)*c;
            }
            else {
                o << *c;
            }
        }
    }
    return o.str();
}

SharedMemory::SharedMemory(const std::string& name, size_t size)
    : shm_name_(name),
    sem_write_name_(name + "_write_sem"),
    sem_read_name_(name + "_read_sem"),
    shm_size_(size + 1), // +1 for null terminator
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
    // Try to open existing shared memory first
    if (!open_shared_memory()) {
        // If it doesn't exist, create it
        if (!create_shared_memory()) {
            std::cerr << "Failed to create shared memory" << std::endl;
            return false;
        }
    }

    // Try to open existing semaphores
    if (!open_semaphores()) {
        // If they don't exist, create them
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
    // Create shared memory
    hMapFile_ = CreateFileMapping(
        INVALID_HANDLE_VALUE,   // use paging file
        NULL,                   // default security
        PAGE_READWRITE,         // read/write access
        0,                      // maximum object size (high-order DWORD)
        shm_size_,              // maximum object size (low-order DWORD)
        shm_name_.c_str()       // name of mapping object
    );

    if (hMapFile_ == NULL) {
        std::cerr << "CreateFileMapping failed: " << GetLastError() << std::endl;
        return false;
    }

    // Map shared memory
    shm_ptr_ = MapViewOfFile(
        hMapFile_,              // handle to map object
        FILE_MAP_ALL_ACCESS,    // read/write permission
        0,
        0,
        shm_size_
    );

    if (shm_ptr_ == NULL) {
        std::cerr << "MapViewOfFile failed: " << GetLastError() << std::endl;
        CloseHandle(hMapFile_);
        hMapFile_ = NULL;
        return false;
    }

    // Initialize with empty string
    memset(shm_ptr_, 0, shm_size_);
    return true;
}

bool SharedMemory::open_shared_memory() {
    hMapFile_ = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,    // read/write access
        FALSE,                  // do not inherit the name
        shm_name_.c_str()       // name of mapping object
    );

    if (hMapFile_ == NULL) {
        return false;
    }

    shm_ptr_ = MapViewOfFile(
        hMapFile_,              // handle to map object
        FILE_MAP_ALL_ACCESS,    // read/write permission
        0,
        0,
        shm_size_
    );

    if (shm_ptr_ == NULL) {
        std::cerr << "MapViewOfFile failed: " << GetLastError() << std::endl;
        CloseHandle(hMapFile_);
        hMapFile_ = NULL;
        return false;
    }

    return true;
}

bool SharedMemory::create_semaphores() {
    // Create write semaphore (initialized to 1 - available for writing)
    sem_write_ = CreateSemaphore(
        NULL,           // default security attributes
        1,              // initial count
        1,              // maximum count
        sem_write_name_.c_str()
    );

    if (sem_write_ == NULL) {
        std::cerr << "CreateSemaphore write failed: " << GetLastError() << std::endl;
        return false;
    }

    // Create read semaphore (initialized to 0 - nothing to read initially)
    sem_read_ = CreateSemaphore(
        NULL,           // default security attributes
        0,              // initial count
        1,              // maximum count
        sem_read_name_.c_str()
    );

    if (sem_read_ == NULL) {
        std::cerr << "CreateSemaphore read failed: " << GetLastError() << std::endl;
        CloseHandle(sem_write_);
        sem_write_ = NULL;
        return false;
    }

    return true;
}

bool SharedMemory::open_semaphores() {
    sem_write_ = OpenSemaphore(
        SEMAPHORE_ALL_ACCESS,
        FALSE,
        sem_write_name_.c_str()
    );

    if (sem_write_ == NULL) {
        return false;
    }

    sem_read_ = OpenSemaphore(
        SEMAPHORE_ALL_ACCESS,
        FALSE,
        sem_read_name_.c_str()
    );

    if (sem_read_ == NULL) {
        CloseHandle(sem_write_);
        sem_write_ = NULL;
        return false;
    }

    return true;
}

bool SharedMemory::write_data(const std::string& data) {
    if (!is_initialized_) {
        std::cerr << "Shared memory not initialized" << std::endl;
        return false;
    }

    if (data.size() >= shm_size_) {
        std::cerr << "Data too large for shared memory" << std::endl;
        return false;
    }

    // Wait for write semaphore
    if (WaitForSingleObject(sem_write_, INFINITE) != WAIT_OBJECT_0) {
        std::cerr << "WaitForSingleObject write failed: " << GetLastError() << std::endl;
        return false;
    }

    // Write data to shared memory
    memcpy(shm_ptr_, data.c_str(), data.size());
    static_cast<char*>(shm_ptr_)[data.size()] = '\0';

    // Post read semaphore
    if (!ReleaseSemaphore(sem_read_, 1, NULL)) {
        std::cerr << "ReleaseSemaphore read failed: " << GetLastError() << std::endl;
        return false;
    }

    return true;
}

std::string SharedMemory::read_data() {
    if (!is_initialized_) {
        return "ERROR: Shared memory not initialized";
    }

    // Wait for read semaphore
    if (WaitForSingleObject(sem_read_, INFINITE) != WAIT_OBJECT_0) {
        std::cerr << "WaitForSingleObject read failed: " << GetLastError() << std::endl;
        return "ERROR: Failed to acquire read semaphore";
    }

    // Read data from shared memory
    std::string data(static_cast<char*>(shm_ptr_));

    // Post write semaphore
    if (!ReleaseSemaphore(sem_write_, 1, NULL)) {
        std::cerr << "ReleaseSemaphore write failed: " << GetLastError() << std::endl;
    }

    return data;
}

void SharedMemory::clear_memory() {
    if (shm_ptr_ && is_initialized_) {
        memset(shm_ptr_, 0, shm_size_);
    }
}

std::string SharedMemory::get_status_json() const {
    std::string json = "{";

    json += "\"shared_memory\":{";
    json += "\"name\":\"" + shm_name_ + "\",";
    json += "\"size\":" + std::to_string(shm_size_) + ",";
    json += "\"initialized\":" + (is_initialized_ ? "true" : "false") + ",";

    if (is_initialized_ && shm_ptr_) {
        std::string content(static_cast<char*>(shm_ptr_));
        json += "\"content\":\"" + escape_json(content) + "\",";
        json += "\"content_length\":" + std::to_string(content.size());
    }
    else {
        json += "\"content\":\"\",";
        json += "\"content_length\":0";
    }
    json += "},";

    json += "\"semaphores\":{";
    json += "\"write_semaphore\":{";
    json += "\"name\":\"" + sem_write_name_ + "\",";
    json += "\"available\":" + (sem_write_ != NULL ? "true" : "false");
    json += "},";
    json += "\"read_semaphore\":{";
    json += "\"name\":\"" + sem_read_name_ + "\",";
    json += "\"available\":" + (sem_read_ != NULL ? "true" : "false");
    json += "}";
    json += "}";

    json += "}";
    return json;
}

void SharedMemory::cleanup() {
    // Unmap shared memory
    if (shm_ptr_) {
        UnmapViewOfFile(shm_ptr_);
        shm_ptr_ = nullptr;
    }

    // Close shared memory handle
    if (hMapFile_) {
        CloseHandle(hMapFile_);
        hMapFile_ = NULL;
    }

    // Close semaphores
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