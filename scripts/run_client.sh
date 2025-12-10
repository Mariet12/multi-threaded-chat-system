#!/bin/bash
# MIT License - Run chat client

MODE="socket"
USER="user_$RANDOM"
IP="127.0.0.1"
PORT=5000
SHMNAME="/os_chat_shm"

while [[ $# -gt 0 ]]; do
    case $1 in
        --mode|-m)
            MODE="$2"
            shift 2
            ;;
        --user|-u)
            USER="$2"
            shift 2
            ;;
        --ip|-i)
            IP="$2"
            shift 2
            ;;
        --port|-p)
            PORT="$2"
            shift 2
            ;;
        --shmname|-s)
            SHMNAME="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "Starting chat client..."
echo "Mode: $MODE"
echo "User: $USER"

if [ "$MODE" = "socket" ]; then
    echo "Server: $IP:$PORT"
else
    echo "Shared Memory: $SHMNAME"
fi

./build/client_gui/chat_client
