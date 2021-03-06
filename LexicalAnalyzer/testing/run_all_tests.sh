#!/bin/bash

# Name: run_all_tests.sh
#
# Purpose: A bash script for running the Lexical Analyzer on the text of all test input files and saving the output in corresponding test output files.
#
# Derek Salama & Jake Leichtling
# CS57
# 4/10/2013

# Note: Must be run from the LexicalAnalyzer/testing directory for file paths to work.

cd ..
make
cd ./testing

cat operator_test.in | ../clexer > operator_test.out

cat string_test.in | ../clexer > string_test.out

cat keyword_test.in | ../clexer > keyword_test.out

cat numconst_test.in | ../clexer > numconst_test.out

cat id_test.in | ../clexer > id_test.out

cat bellman-ford.cs57.in | ../clexer > bellman-ford.cs57.out

cat small_test_program.c57.in | ../clexer > small_test_program.c57.out

cd ..
make clean
cd ./testing