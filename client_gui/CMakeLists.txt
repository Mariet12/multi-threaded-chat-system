find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

set(CLIENT_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    SocketClient.cpp
    SocketClient.h
    ShmClient.cpp
    ShmClient.h
)

add_executable(chat_client ${CLIENT_SOURCES})

target_link_libraries(chat_client
    common
    Qt5::Widgets
    pthread
    rt
)
