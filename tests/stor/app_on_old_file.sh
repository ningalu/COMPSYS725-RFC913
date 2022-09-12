#!/bin/bash
echo "user 1"
echo "stor APP ./server1.txt"
echo "size 1"
echo "done"

if [ ! -f ./resources/Server/client1.txt ]; then
    exit 1
fi