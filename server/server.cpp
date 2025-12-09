/*
 * MIT License
 * Multi-Threaded Socket Server for Chat System
 */

#include "common.h"
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <sstream>

// Client information
struct ClientInfo {
    int socket_fd;
    std::string username;
    std::thread thread;
    bool active;
    
    ClientInfo(int fd) : socket_fd(fd), active(true) {}
};

// Global server state
std::vector<std::shared_ptr<ClientInfo>> clients;
std::mutex clients_mutex;
bool server_running = true;
int server_socket = -1;

// Signal handler for graceful shutdown
void signal_handler(int signum) {
    std::cout << "\n[SERVER] Shutting down gracefully...\n";
    server_running = false;
    if (server_socket != -1) {
        close(server_socket);
    }
}

// Broadcast message to all clients except sender
void broadcast_message(const std::string& message, int sender_fd) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    
    for (auto& client : clients) {
        if (client->active && client->socket_fd != sender_fd) {
            ssize_t sent = send(client->socket_fd, message.c_str(), message.length(), MSG_NOSIGNAL);
            if (sent == -1) {
                std::cerr << "[SERVER] Failed to send to client " << client->username << "\n";
                client->active = false;
            }
        }
    }
}

// Send online users list to specific client
void send_user_list(int client_fd) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    
    std::stringstream ss;
    ss << "{\"type\":\"userlist\",\"users\":[";
    bool first = true;
    for (const auto& client : clients) {
        if (client->active && !client->username.empty()) {
            if (!first) ss << ",";
            ss << "\"" << client->username << "\"";
            first = false;
        }
    }
    ss << "]}\n";
    
    std::string msg = ss.str();
    send(client_fd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
}

// Read a complete line (newline-terminated message)
std::string read_line(int socket_fd) {
    std::string line;
    char c;
    
    while (true) {
        ssize_t n = recv(socket_fd, &c, 1, 0);
        if (n <= 0) {
            return ""; // Connection closed or error
        }
        if (c == '\n') {
            break;
        }
        line += c;
    }
    
    return line;
}

// Handle individual client connection
void handle_client(std::shared_ptr<ClientInfo> client) {
    std::cout << "[SERVER] New client connected (fd: " << client->socket_fd << ")\n";
    
    // Request username
    std::string welcome = "{\"type\":\"welcome\",\"text\":\"Please send your username\"}\n";
    send(client->socket_fd, welcome.c_str(), welcome.length(), 0);
    
    // Read username
    std::string username_line = read_line(client->socket_fd);
    if (username_line.empty()) {
        std::cout << "[SERVER] Client disconnected before sending username\n";
        close(client->socket_fd);
        return;
    }
    
    // Parse username (simple JSON parse)
    size_t user_pos = username_line.find("\"user\":\"");
    if (user_pos != std::string::npos) {
        size_t start = user_pos + 8;
        size_t end = username_line.find("\"", start);
        client->username = username_line.substr(start, end - start);
    } else {
        client->username = "Anonymous";
    }
    
    std::cout << "[SERVER] Client identified as: " << client->username << "\n";
    
    // Notify all clients about new user
    std::string join_msg = create_json_message("SERVER", client->username + " joined the chat");
    broadcast_message(join_msg, -1);
    
    // Send user list to new client
    send_user_list(client->socket_fd);
    
    // Message loop
    while (server_running && client->active) {
        std::string message = read_line(client->socket_fd);
        
        if (message.empty()) {
            // Client disconnected
            std::cout << "[SERVER] Client " << client->username << " disconnected\n";
            client->active = false;
            break;
        }
        
        std::cout << "[SERVER] Message from " << client->username << ": " << message << "\n";
        
        // Broadcast to all other clients
        broadcast_message(message + "\n", client->socket_fd);
    }
    
    // Cleanup
    std::string leave_msg = create_json_message("SERVER", client->username + " left the chat");
    broadcast_message(leave_msg, -1);
    
    close(client->socket_fd);
}

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--port" && i + 1 < argc) {
            port = std::stoi(argv[i + 1]);
            i++;
        }
    }
    
    // Setup signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "[SERVER] Failed to create socket\n";
        return 1;
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Bind socket
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "[SERVER] Failed to bind to port " << port << "\n";
        close(server_socket);
        return 1;
    }
    
    // Listen
    if (listen(server_socket, 10) == -1) {
        std::cerr << "[SERVER] Failed to listen\n";
        close(server_socket);
        return 1;
    }
    
    std::cout << "[SERVER] Listening on port " << port << "\n";
    
    // Accept loop
    while (server_running) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_socket, (sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            if (server_running) {
                std::cerr << "[SERVER] Failed to accept connection\n";
            }
            continue;
        }
        
        // Create client info and thread
        auto client = std::make_shared<ClientInfo>(client_fd);
        client->thread = std::thread(handle_client, client);
        client->thread.detach(); // Detach to allow independent cleanup
        
        // Add to clients list (thread-safe)
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(client);
        }
    }
    
    // Cleanup
    std::cout << "[SERVER] Cleaning up...\n";
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (auto& client : clients) {
            if (client->active) {
                close(client->socket_fd);
            }
        }
    }
    
    close(server_socket);
    std::cout << "[SERVER] Shutdown complete\n";
    
    return 0;
}
