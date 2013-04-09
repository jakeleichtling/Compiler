#!/bin/bash

make

echo
echo "---------"
echo "OPERATORS"
echo "---------"
echo

./clexer < operator_test.c

echo
echo "-------"
echo "STRINGS"
echo "-------"
echo 

./clexer < string_test.c

echo
echo "-------"
echo "KEYWORDS"
echo "-------"
echo 

./clexer < keyword_test.c

echo
echo "-------"
echo "NUMBERS"
echo "-------"
echo 

./clexer < numconst_test.c

echo
echo "----"
echo "IDS"
echo "----"
echo 

./clexer < id_test.c 

echo
make clean

echo "TEST COMPLETE"
echo