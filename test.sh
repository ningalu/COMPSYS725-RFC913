#!/bin/bash
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

PASS="${GREEN}Test Passed${NC}"
TEST_SEPARATOR="${YELLOW}|----------------------------------------|${NC}"

LOGIN_TESTS=../tests/login/*
DONE_TESTS=../tests/done/*
TYPE_TESTS=../tests/type/*
LIST_TESTS=../tests/list/*
CDIR_TESTS=../tests/cdir/*
KILL_TESTS=../tests/kill/*
NAME_TESTS=../tests/name/*
RETR_TESTS=../tests/retr/*
STOR_TESTS=../tests/stor/*

test_res () {
    if [ $? -eq 0 ]
    then
        echo -e $PASS
    fi
}

cd ./build

echo "Login Tests"
for f in $LOGIN_TESTS
do
    echo "Test: " $f
    bash $f | ./Client
    test_res
done
echo -e $TEST_SEPARATOR

echo "Done Tests"
for f in $DONE_TESTS
do
    echo "Test: " $f
    bash $f | ./Client
    test_res
done
echo -e $TEST_SEPARATOR

echo "Transmission Type Tests"
for f in $TYPE_TESTS
do
    echo "Test: " $f
    bash $f | ./Client
    test_res
done
echo -e $TEST_SEPARATOR

echo "Directory Listing Tests"
for f in $LIST_TESTS
do
    echo "Test: " $f
    bash $f | ./Client
    test_res
done
echo -e $TEST_SEPARATOR

echo "Change Directory Tests"
for f in $CDIR_TESTS
do
    echo "Test: " $f
    bash $f | ./Client
    test_res
done
echo -e $TEST_SEPARATOR

echo "Delete File Tests"
# These tests are destructive so they restore /build/resources on every attempt
# The effects these commands have on the file structure are checked by the test scripts
for f in $KILL_TESTS
do
    cd ../
    bash ./rsc.sh
    cd ./build
    echo "Test: " $f
    bash $f | ./Client
    test_res
done
echo -e $TEST_SEPARATOR

echo "Rename File Tests"
# These tests are destructive so they restore /build/resources on every attempt
# The effects these commands have on the file structure are checked by the test scripts
for f in $NAME_TESTS
do
    cd ../
    bash ./rsc.sh
    cd ./build
    echo "Test: " $f
    bash $f | ./Client
    test_res
done
echo -e $TEST_SEPARATOR

echo "Retrieve File Tests"
# These tests are destructive so they restore /build/resources on every attempt
# The effects these commands have on the file structure are checked by the test scripts
for f in $RETR_TESTS
do
    cd ../
    bash ./rsc.sh
    cd ./build
    echo "Test: " $f
    bash $f | ./Client
    test_res
done
echo -e $TEST_SEPARATOR

echo "Store File Tests"
# These tests are destructive so they restore /build/resources on every attempt
# The effects these commands have on the file structure are checked by the test scripts
for f in $STOR_TESTS
do
    cd ../
    bash ./rsc.sh
    cd ./build
    echo "Test: " $f
    bash $f | ./Client
    test_res
done
echo -e $TEST_SEPARATOR
