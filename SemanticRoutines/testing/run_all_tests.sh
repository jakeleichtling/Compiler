#!/bin/bash

# Name: run_all_tests.sh
#
# Purpose: A bash script for running the Parser on the text of all test input files and saving the output in corresponding test output files.
#
# Derek Salama & Jake Leichtling
# CS57
# 5/29/2013

# Note: Must be run from the SemanticRoutines/testing directory for file paths to work.

cd ..
make
cd ./testing

echo "---------------------------------------------------------"

echo "~~~~~~~~~~~~~~~~~~~ bellman-ford.c57.in ~~~~~~~~~~~~~~~~~"
../djcc bellman-ford.c57 bellman-ford.tm57

# echo "~~~~~~~~~~~~~~~~~~~ dijkstra.c57.in ~~~~~~~~~~~~~~~~~~~~~"
#../djcc dijkstra.c57.in dijkstra.tm57
#$TM57_PATH dijkstra.tm57 > dijkstra.output

# echo "~~~~~~~~~~~~~~ binary_operations.c57.in ~~~~~~~~~~~~~~~~"

echo "---------------------------------------------------------"