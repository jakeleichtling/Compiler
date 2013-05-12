#!/bin/bash

# Name: run_all_tests.sh
#
# Purpose: A bash script for running the Parser on the text of all test input files and saving the output in corresponding test output files.
#
# Derek Salama & Jake Leichtling
# CS57
# 4/10/2013

# Note: Must be run from the Parser/testing directory for file paths to work.

cd ..
make
cd ./testing

echo "---------------------------------------------------------"

# echo "~~~~~~~~~~~~~~~~~~~ bellman-ford.c57.in ~~~~~~~~~~~~~~~~~"
# cat bellman-ford.c57.in | ../djcc > bellman-ford.c57.out

# echo "~~~~~~~~~~~~~~~~~~~ dijkstra.c57.in ~~~~~~~~~~~~~~~~~~~~~"
# cat dijkstra.c57.in | ../djcc > dijkstra.c57.out

# echo "~~~~~~~~~~~~~~ binary_operations.c57.in ~~~~~~~~~~~~~~~~"
# cat binary_operations.c57.in | ../djcc > binary_operations.c57.out

echo "~~~~~~~~~~~~~~ temp_test.c57.in ~~~~~~~~~~~~~~~~"
cat temp_test.c57.in | ../djcc > temp_test.c57.out

echo "---------------------------------------------------------"