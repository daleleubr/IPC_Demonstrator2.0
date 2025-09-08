#ifndef SHARED_MEMORY_HPP
#define SHARED_MEMORY_HPP

#include <string>
#include <cstddef>
#include <windows.h>

class SharedMemory {
public:
    SharedMemory(const std::string& name, size_t size);
    ~SharedMemory();

    bool initialize();
    bool write_data(const std::string& data);
    std::string read_data();
    void clear_memory();
    std::string get_status_json() const;

    // Prevent copying
    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;

private:
    std::string shm_name_;
    std::string sem_write_name_;
    std::string sem_read_name_;
    size_t shm_size_;

    HANDLE hMapFile_;
    void* shm_ptr_;

    HANDLE sem_write_;
    HANDLE sem_read_;

    bool is_initialized_;

    bool create_shared_memory();
    bool open_shared_memory();
    bool create_semaphores();
    bool open_semaphores();
    void cleanup();
};

#endif // SHARED_MEMORY_HPP