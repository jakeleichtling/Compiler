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

cat bellman-ford.c57.in | ../parser > bellman-ford.c57.out

cat dijkstra.c57.in | ../parser > dijkstra.c57.out

cat loops_within_loops.c57.in | ../parser > loops_within_loops.c57.out

cat complicated_expressions.c57.in | ../parser > complicated_expressions.c57.out

cat string_newline.c57.in | ../parser > string_newline.c57.out

cat decl_after_print.c57.in | ../parser > decl_after_print.c57.out

cd ..
make clean
cd ./testing