add_executable(shm_test_client shm_test_client.cpp)
target_link_libraries(shm_test_client common pthread rt)

add_executable(automated_test automated_test.cpp)
target_link_libraries(automated_test common pthread rt)
