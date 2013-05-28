#!/bin/bash

# Name: run_all_tests.sh
#
# Compile all .c57 test files
#
# Derek Salama & Jake Leichtling
# CS57
# 5/29/2013

# Note: Must be run from the SemanticRoutines/testing directory for file paths to work.

cd ..
make
cd ./testing

echo "---------------------------------------------------------"

echo "~~~~~~~~~~~~~~~~~~~ bellman-ford.c57 ~~~~~~~~~~~~~~~~~"
../djcc bellman-ford.c57 bellman-ford.tm57

echo "~~~~~~~~~~~~~~~~~~~ dijkstra.c57 ~~~~~~~~~~~~~~~~~~~~~"
../djcc dijkstra.c57 dijkstra.tm57

echo "~~~~~~~~~~~~~~~~~~~ merge_sort.c57 ~~~~~~~~~~~~~~~~~~~~~"
../djcc merge_sort.c57 merge_sort.tm57

echo "~~~~~~~~~~~~~~~~~~~ decl_test.c57 ~~~~~~~~~~~~~~~~~~~~~"
../djcc decl_test.c57 decl_test.tm57

echo "~~~~~~~~~~~~~~~~~~~ op_test.c57 ~~~~~~~~~~~~~~~~~~~~~"
../djcc op_test.c57 op_test.tm57

echo "~~~~~~~~~~~~~~~~~~~ misc_test.c57 ~~~~~~~~~~~~~~~~~~~~~"
../djcc misc_test.c57 misc_test.tm57

echo "~~~~~~~~~~~~~~~~~~~ function_call_test.c57 ~~~~~~~~~~~~~~~~~~~~~"
../djcc function_call_test.c57 function_call_test.tm57

echo "~~~~~~~~~~~~~~~~~~~ variable_size_array_test.c57 ~~~~~~~~~~~~~~~~~~~~~"
../djcc variable_size_array_test.c57 variable_size_array_test.tm57

echo "---------------------------------------------------------"