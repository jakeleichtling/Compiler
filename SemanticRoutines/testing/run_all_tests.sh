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

echo "~~~~~~~~~~~~~~~~~~~ bellman-ford.c57.in ~~~~~~~~~~~~~~~~~"
cat bellman-ford.c57.in | ../djcc > bellman-ford.c57.out

echo "~~~~~~~~~~~~~~~~~~~ dijkstra.c57.in ~~~~~~~~~~~~~~~~~~~~~"
cat dijkstra.c57.in | ../djcc > dijkstra.c57.out

echo "~~~~~~~~~~~~~~ loops_within_loops.c57.in ~~~~~~~~~~~~~~~~"
cat loops_within_loops.c57.in | ../djcc > loops_within_loops.c57.out

echo "~~~~~~~~~~~~~ complicated_expressions.c57.in ~~~~~~~~~~~~"
cat complicated_expressions.c57.in | ../djcc > complicated_expressions.c57.out

echo "~~~~~~~~~~~~~~~~ string_newline.c57.in ~~~~~~~~~~~~~~~~~~"
cat string_newline.c57.in | ../djcc > string_newline.c57.out

echo "~~~~~~~~~~~~~~~ decl_after_print.c57.in ~~~~~~~~~~~~~~~~~"
cat decl_after_print.c57.in | ../djcc > decl_after_print.c57.out

echo "---------------------------------------------------------"