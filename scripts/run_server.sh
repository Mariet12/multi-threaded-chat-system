#!/bin/bash
# MIT License - Run chat server

PORT=5000

if [ ! -z "$1" ]; then
    PORT=$1
fi

echo "Starting chat server on port $PORT..."
./build/server/chat_server --port $PORT
