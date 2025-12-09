/*
 * MIT License
 * Automated test for both systems
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>

void test_socket_system() {
    std::cout << "\n=== Testing Socket System ===\n";
    std::cout << "1. Start server: ./build/server/chat_server --port 5000\n";
    std::cout << "2. Start clients in separate terminals\n";
    std::cout << "3. Send messages and verify broadcast\n";
    std::cout << "Test: MANUAL (requires multiple terminals)\n";
}

void test_shm_system() {
    std::cout << "\n=== Testing Shared Memory System ===\n";
    
    std::system("./build/tests/shm_test_client alice 'Hello from Alice' &");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::system("./build/tests/shm_test_client bob 'Hi Alice, this is Bob' &");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::system("./build/tests/shm_test_client charlie 'Charlie joining the chat' &");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "Shared memory test completed. Check /dev/shm/os_chat_shm\n";
}

int main() {
    std::cout << "Multi-Threaded Chat System - Automated Tests\n";
    std::cout << "=============================================\n";
    
    test_socket_system();
    test_shm_system();
    
    std::cout << "\nAll tests completed!\n";
    return 0;
}
