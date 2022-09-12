#!/bin/bash
echo "user 1"
echo "retr ./big.txt"
echo "send"
echo "done"
cmp ./resources/Server/big.txt ./resources/Client/big.txt