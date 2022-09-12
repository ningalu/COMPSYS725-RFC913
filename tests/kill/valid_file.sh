#!/bin/bash
echo "user 1"
echo "kill ./server1.txt"
echo "done"
if [ -f ./resources/Server/server1.txt ]; then
    exit 1
fi
