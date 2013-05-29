#!/bin/bash

# run_all_error_tests.sh
#
# A bash script for running the compiler on all error tests {1-5,6a-6d,7-20}_error_test.c57.
# These C57 input files correspond to the error tests enumerated in ERROR_TESTING_README.
#
# Derek Salama & Jake Leichtling
# 5/29/2013

# Note: Must be run from the testing directory for file paths to work.

cd ..
make
cd ./testing

echo "---------------------------------------------------------"

echo "1.) Newline in string (1_error_test.c57)"
../djcc 1_error_test.c57
echo

echo "2.) Main function that takes parameters (2_error_test.c57)"
../djcc 2_error_test.c57
echo

echo "3.) Main function that has a non-void return type (3_error_test.c57)"
../djcc 3_error_test.c57
echo

echo "4.) No main function (4_error_test.c57)"
../djcc 4_error_test.c57
echo

echo "5.) Call an undeclared function (5_error_test.c57)"
../djcc 5_error_test.c57
echo

echo "6a.) Pass array when value expected (6a_error_test.c57)"
../djcc 6a_error_test.c57
echo

echo "6b.) Pass value when array expected (6b_error_test.c57)"
../djcc 6b_error_test.c57
echo

echo "6c.) Pass double when int expected (6c_error_test.c57)"
../djcc 6c_error_test.c57
echo

echo "6d.) Pass the wrong number of parameters (6d_error_test.c57)"
../djcc 6d_error_test.c57
echo

echo "7.) Array variable in mathematical operation (7_error_test.c57)"
../djcc 7_error_test.c57
echo

echo "8.) Array variable in mathematical comparison (8_error_test.c57)"
../djcc 8_error_test.c57
echo

echo "9.) Two variables of the same name in the same scope (9_error_test.c57)"
../djcc 9_error_test.c57
echo

echo "10.) Use an undeclared variable (10_error_test.c57)"
../djcc 10_error_test.c57
echo

echo "11.) Initialization of variable in outter scope of a function with same name as a parameter (11_error_test.c57)"
../djcc 11_error_test.c57
echo

echo "12.) Index into an array with a float (12_error_test.c57)"
../djcc 12_error_test.c57
echo

echo "13.) Subscript a non-array variable (13_error_test.c57)"
../djcc 13_error_test.c57
echo

echo "14.) Assign to an array pointer (14_error_test.c57)"
../djcc 14_error_test.c57
echo

echo "15.) Assign from an array pointer (15_error_test.c57)"
../djcc 15_error_test.c57
echo

echo "16.) Implicit cast of double to int (16_error_test.c57)"
../djcc 16_error_test.c57
echo

echo "17.) Logical operation with float (17_error_test.c57)"
../djcc 17_error_test.c57
echo

echo "18.) Increment an array pointer (18_error_test.c57)"
../djcc 18_error_test.c57
echo

echo "19.) Return types of function declaration and function body don't match (19_error_test.c57)"
../djcc 19_error_test.c57
echo

echo "20.) Non-void function without a return statement (20_error_test.c57)"
../djcc 20_error_test.c57

echo "---------------------------------------------------------"