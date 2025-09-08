#include "shared_memory.hpp"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <json/json.h>

SharedMemory::SharedMemory(const std::string& name, size_t size)
    : shm_name_("/" + name),
    sem_write_name_("/" + name + "_write_sem"),
    sem_read_name_("/" + name + "_read_sem"),
    shm_size_(size + 1), // +1 for null terminator
    shm_fd_(-1),
    shm_ptr_(nullptr),
    sem_write_(SEM_FAILED),
    sem_read_(SEM_FAILED),
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
    shm_fd_ = shm_open(shm_name_.c_str(), O_CREAT | O_RDWR, 0666);
    if (shm_fd_ == -1) {
        perror("shm_open");
        return false;
    }

    // Set size
    if (ftruncate(shm_fd_, shm_size_) == -1) {
        perror("ftruncate");
        return false;
    }

    // Map shared memory
    shm_ptr_ = mmap(nullptr, shm_size_, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
    if (shm_ptr_ == MAP_FAILED) {
        perror("mmap");
        return false;
    }

    // Initialize with empty string
    memset(shm_ptr_, 0, shm_size_);
    return true;
}

bool SharedMemory::open_shared_memory() {
    shm_fd_ = shm_open(shm_name_.c_str(), O_RDWR, 0666);
    if (shm_fd_ == -1) {
        return false;
    }

    shm_ptr_ = mmap(nullptr, shm_size_, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
    if (shm_ptr_ == MAP_FAILED) {
        perror("mmap");
        return false;
    }

    return true;
}

bool SharedMemory::create_semaphores() {
    // Create write semaphore (initialized to 1 - available for writing)
    sem_write_ = sem_open(sem_write_name_.c_str(), O_CREAT, 0666, 1);
    if (sem_write_ == SEM_FAILED) {
        perror("sem_open write");
        return false;
    }

    // Create read semaphore (initialized to 0 - nothing to read initially)
    sem_read_ = sem_open(sem_read_name_.c_str(), O_CREAT, 0666, 0);
    if (sem_read_ == SEM_FAILED) {
        perror("sem_open read");
        return false;
    }

    return true;
}

bool SharedMemory::open_semaphores() {
    sem_write_ = sem_open(sem_write_name_.c_str(), 0);
    if (sem_write_ == SEM_FAILED) {
        return false;
    }

    sem_read_ = sem_open(sem_read_name_.c_str(), 0);
    if (sem_read_ == SEM_FAILED) {
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
    if (sem_wait(sem_write_) == -1) {
        perror("sem_wait write");
        return false;
    }

    // Write data to shared memory
    memcpy(shm_ptr_, data.c_str(), data.size());
    static_cast<char*>(shm_ptr_)[data.size()] = '\0';

    // Post read semaphore
    if (sem_post(sem_read_) == -1) {
        perror("sem_post read");
        return false;
    }

    return true;
}

std::string SharedMemory::read_data() {
    if (!is_initialized_) {
        return "ERROR: Shared memory not initialized";
    }

    // Wait for read semaphore
    if (sem_wait(sem_read_) == -1) {
        perror("sem_wait read");
        return "ERROR: Failed to acquire read semaphore";
    }

    // Read data from shared memory
    std::string data(static_cast<char*>(shm_ptr_));

    // Post write semaphore
    if (sem_post(sem_write_) == -1) {
        perror("sem_post write");
    }

    return data;
}

void SharedMemory::clear_memory() {
    if (shm_ptr_ && is_initialized_) {
        memset(shm_ptr_, 0, shm_size_);
    }
}

std::string SharedMemory::get_status_json() const {
    Json::Value root;

    root["shared_memory"]["name"] = shm_name_;
    root["shared_memory"]["size"] = static_cast<Json::UInt64>(shm_size_);
    root["shared_memory"]["initialized"] = is_initialized_;

    if (is_initialized_ && shm_ptr_) {
        std::string content(static_cast<char*>(shm_ptr_));
        root["shared_memory"]["content"] = content;
        root["shared_memory"]["content_length"] = static_cast<Json::UInt64>(content.size());
    }
    else {
        root["shared_memory"]["content"] = "";
        root["shared_memory"]["content_length"] = 0;
    }

    // Get semaphore values
    int write_val = -1, read_val = -1;
    if (sem_write_ != SEM_FAILED) {
        sem_getvalue(sem_write_, &write_val);
    }
    if (sem_read_ != SEM_FAILED) {
        sem_getvalue(sem_read_, &read_val);
    }

    root["semaphores"]["write_semaphore"]["name"] = sem_write_name_;
    root["semaphores"]["write_semaphore"]["value"] = write_val;
    root["semaphores"]["write_semaphore"]["available"] = (write_val > 0);

    root["semaphores"]["read_semaphore"]["name"] = sem_read_name_;
    root["semaphores"]["read_semaphore"]["value"] = read_val;
    root["semaphores"]["read_semaphore"]["available"] = (read_val > 0);

    Json::StreamWriterBuilder writer;
    return Json::writeString(writer, root);
}

void SharedMemory::cleanup() {
    // Unmap shared memory
    if (shm_ptr_ && shm_ptr_ != MAP_FAILED) {
        munmap(shm_ptr_, shm_size_);
    }

    // Close shared memory file descriptor
    if (shm_fd_ != -1) {
        close(shm_fd_);
    }

    // Close semaphores
    if (sem_write_ != SEM_FAILED) {
        sem_close(sem_write_);
    }
    if (sem_read_ != SEM_FAILED) {
        sem_close(sem_read_);
    }

    is_initialized_ = false;
}