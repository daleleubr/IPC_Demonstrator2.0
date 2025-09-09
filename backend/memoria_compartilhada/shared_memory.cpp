#include "shared_memory.hpp"
#include <sstream>
#include <vector>
#include <cstdio>
#include <cstring>

static std::wstring dir_of_module() {
    wchar_t buf[MAX_PATH];
    DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    std::wstring path(buf, buf + n);
    auto pos = path.find_last_of(L"\\/");
    if (pos != std::wstring::npos) path.resize(pos);
    return path;
}

SharedMemory::SharedMemory(size_t size) : shm_size_(size ? size : 1024) {}

SharedMemory::~SharedMemory() {
    if (view_) { UnmapViewOfFile(view_); view_ = nullptr; }
    if (h_map_) { CloseHandle(h_map_); h_map_ = nullptr; }
    if (h_file_ != INVALID_HANDLE_VALUE) { CloseHandle(h_file_); h_file_ = INVALID_HANDLE_VALUE; }
}

std::wstring SharedMemory::get_file_path() const {
    // arquivo fica ao lado do .exe: <pasta_do_exe>\ipc_shm.bin
    return dir_of_module() + L"\\ipc_shm.bin";
}

bool SharedMemory::initialize() {
    if (initialized_) return true;

    std::wstring file_path = get_file_path();

    // 1) abre/cria arquivo
    h_file_ = CreateFileW(
        file_path.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (h_file_ == INVALID_HANDLE_VALUE) {
        return false;
    }

    // garante tamanho
    LARGE_INTEGER sz{};
    if (!GetFileSizeEx(h_file_, &sz) || (size_t)sz.QuadPart < shm_size_) {
        LARGE_INTEGER pos{};
        pos.QuadPart = (LONGLONG)shm_size_;
        if (!SetFilePointerEx(h_file_, pos, nullptr, FILE_BEGIN) || !SetEndOfFile(h_file_)) {
            return false;
        }
    }

    // 2) mapping
    h_map_ = CreateFileMappingW(h_file_, nullptr, PAGE_READWRITE, 0, 0, nullptr);
    if (!h_map_) return false;

    view_ = static_cast<char*>(MapViewOfFile(h_map_, FILE_MAP_ALL_ACCESS, 0, 0, shm_size_));
    if (!view_) return false;

    initialized_ = true;
    return true;
}

bool SharedMemory::write_data(const std::string& data) {
    if (!initialized_) return false;
    size_t n = data.size();
    if (n >= shm_size_) n = shm_size_ - 1; // deixa 1 byte p/ '\0'
    memcpy(view_, data.data(), n);
    view_[n] = '\0';
    FlushViewOfFile(view_, n + 1);
    return true;
}

std::string SharedMemory::read_data() {
    if (!initialized_) return "ERROR: not initialized";
    return std::string(view_ ? view_ : "");
}

void SharedMemory::clear_memory() {
    if (!initialized_) return;
    memset(view_, 0, shm_size_);
    FlushViewOfFile(view_, shm_size_);
}

std::string SharedMemory::json_escape(const std::string& s) {
    std::string out; out.reserve(s.size() + 16);
    for (unsigned char c : s) {
        switch (c) {
        case '\"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b";  break;
        case '\f': out += "\\f";  break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        default:
            if (c < 0x20) { char b[7]; std::snprintf(b, sizeof(b), "\\u%04x", c); out += b; }
            else out += (char)c;
        }
    }
    return out;
}

std::string SharedMemory::get_status_json() const {
    std::ostringstream os;
    std::string content = std::string(view_ ? view_ : "");
    os << "{"
        << "\"shared_memory\":{"
        << "\"file\":\"" << json_escape(std::string(get_file_path().begin(), get_file_path().end())) << "\","
        << "\"size\":" << shm_size_ << ","
        << "\"initialized\":" << (initialized_ ? "true" : "false") << ","
        << "\"content\":\"" << json_escape(content) << "\","
        << "\"content_length\":" << content.size()
        << "}"
        << "}";
    return os.str();
}
