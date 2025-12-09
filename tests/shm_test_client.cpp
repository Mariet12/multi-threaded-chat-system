/*
 * MIT License
 * Headless Shared Memory Test Client
 */

#include "common.h"
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <cstring>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <username> <message>\n";
        return 1;
    }
    
    std::string username = argv[1];
    std::string message = argv[2];
    
    int shm_fd = shm_open(DEFAULT_SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        std::cerr << "Failed to open shared memory\n";
        return 1;
    }
    
    size_t shm_size = sizeof(SharedMemoryLayout);
    ftruncate(shm_fd, shm_size);
    
    SharedMemoryLayout* shm_ptr = (SharedMemoryLayout*)mmap(
        nullptr, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    if (shm_ptr == MAP_FAILED) {
        std::cerr << "Failed to map shared memory\n";
        close(shm_fd);
        return 1;
    }
    
    sem_t* sem_mutex = sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1);
    sem_t* sem_empty = sem_open(SEM_EMPTY_NAME, O_CREAT, 0666, SHARED_MEMORY_CAPACITY);
    sem_t* sem_full = sem_open(SEM_FULL_NAME, O_CREAT, 0666, 0);
    
    if (sem_mutex == SEM_FAILED || sem_empty == SEM_FAILED || sem_full == SEM_FAILED) {
        std::cerr << "Failed to open semaphores\n";
        munmap(shm_ptr, shm_size);
        close(shm_fd);
        return 1;
    }
    
    sem_wait(sem_mutex);
    if (shm_ptr->capacity == 0) {
        new (shm_ptr) SharedMemoryLayout();
    }
    sem_post(sem_mutex);
    
    sem_wait(sem_empty);
    sem_wait(sem_mutex);
    
    int write_idx = shm_ptr->write_index.load();
    ChatMessage& msg = shm_ptr->messages[write_idx];
    
    std::string timestamp = get_timestamp();
    msg.set(username, timestamp, message);
    
    shm_ptr->write_index.store((write_idx + 1) % SHARED_MEMORY_CAPACITY);
    
    sem_post(sem_mutex);
    sem_post(sem_full);
    
    std::cout << "[" << username << "] Message sent: " << message << "\n";
    
    sem_close(sem_mutex);
    sem_close(sem_empty);
    sem_close(sem_full);
    munmap(shm_ptr, shm_size);
    close(shm_fd);
    
    return 0;
}
