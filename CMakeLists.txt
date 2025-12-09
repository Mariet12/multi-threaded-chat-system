# MIT License - Multi-Threaded Chat System
cmake_minimum_required(VERSION 3.10)
project(os_chat_project VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find Qt5
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/shared)

# Add subdirectories
add_subdirectory(shared)
add_subdirectory(server)
add_subdirectory(client_gui)
add_subdirectory(tests)
