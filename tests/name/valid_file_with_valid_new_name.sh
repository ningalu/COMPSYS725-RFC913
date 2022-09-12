#!/bin/bash
echo "user 1"
echo "name ./server1.txt"
echo "tobe server4.txt"
echo "done"
if [ -f ./build/resources/Server/server1.txt ]; then
    exit 1
fi