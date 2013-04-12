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

cat simple_program.in | ../parser > simple_program.out

echo -------------------------------------------------

cat simple_program.out

echo -------------------------------------------------

cd ..
make clean
cd ./testing