#!/bin/bash
# MIT License - Cleanup shared memory and semaphores

echo "Cleaning up shared memory resources..."

rm -f /dev/shm/os_chat_shm
rm -f /dev/shm/sem.os_chat_mutex
rm -f /dev/shm/sem.os_chat_full
rm -f /dev/shm/sem.os_chat_empty

echo "Cleanup complete!"
