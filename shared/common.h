/*
 * MIT License
 * Common definitions for Multi-Threaded Chat System
 */

#ifndef COMMON_H
#define COMMON_H

#include <cstring>
#include <ctime>
#include <string>
#include <atomic>

// Message protocol constants
constexpr int MAX_USERNAME_LEN = 32;
constexpr int MAX_TIMESTAMP_LEN = 32;
constexpr int MAX_MESSAGE_TEXT_LEN = 512;
constexpr int SHARED_MEMORY_CAPACITY = 64;
constexpr int DEFAULT_PORT = 5000;
constexpr const char* DEFAULT_SHM_NAME = "/os_chat_shm";
constexpr const char* SEM_MUTEX_NAME = "/os_chat_mutex";
constexpr const char* SEM_FULL_NAME = "/os_chat_full";
constexpr const char* SEM_EMPTY_NAME = "/os_chat_empty";

// Message structure for shared memory
struct ChatMessage {
    char username[MAX_USERNAME_LEN];
    char timestamp[MAX_TIMESTAMP_LEN];
    char text[MAX_MESSAGE_TEXT_LEN];
    bool valid; // Flag to check if message slot is valid
    
    ChatMessage() : valid(false) {
        std::memset(username, 0, MAX_USERNAME_LEN);
        std::memset(timestamp, 0, MAX_TIMESTAMP_LEN);
        std::memset(text, 0, MAX_MESSAGE_TEXT_LEN);
    }
    
    void set(const std::string& user, const std::string& time, const std::string& msg) {
        strncpy(username, user.c_str(), MAX_USERNAME_LEN - 1);
        strncpy(timestamp, time.c_str(), MAX_TIMESTAMP_LEN - 1);
        strncpy(text, msg.c_str(), MAX_MESSAGE_TEXT_LEN - 1);
        valid = true;
    }
};

// Shared memory layout - Ring buffer with synchronization
struct SharedMemoryLayout {
    std::atomic<int> write_index;
    std::atomic<int> read_index;
    int capacity;
    int active_users;
    ChatMessage messages[SHARED_MEMORY_CAPACITY];
    
    SharedMemoryLayout() : write_index(0), read_index(0), 
                          capacity(SHARED_MEMORY_CAPACITY), active_users(0) {}
};

// Utility function to get current timestamp
inline std::string get_timestamp() {
    time_t now = time(nullptr);
    char buf[MAX_TIMESTAMP_LEN];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", gmtime(&now));
    return std::string(buf);
}

// JSON message format helper
inline std::string create_json_message(const std::string& user, 
                                      const std::string& text,
                                      const std::string& time = "") {
    std::string timestamp = time.empty() ? get_timestamp() : time;
    return "{\"user\":\"" + user + "\",\"time\":\"" + timestamp + 
           "\",\"text\":\"" + text + "\"}\n";
}

#endif // COMMON_H
