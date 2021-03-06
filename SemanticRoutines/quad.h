/* quad.h
 * 
 * Declarations for generating intermediate code
 * from an abstract syntax tree and the corresponding symbol table.
 *
 * Jake Leichtling & Derek Salama
 * 5/29/2013
 */

#ifndef QUAD_H_
#define QUAD_H_

#include "ast.h"
#include "symtab.h"

/* quad operations */
enum quad_op {
  call_func_op,
  print_int_op,
  print_float_op,
  print_string_op,
  int_to_float_op,
  assn_int_to_var_op,
  assn_int_to_arraysub_op,
  assn_int_from_arraysub_op,
  assn_float_to_var_op,
  assn_float_to_arraysub_op,
  assn_float_from_arraysub_op,
  add_ints_op,
  add_floats_op,
  sub_ints_op,
  sub_floats_op,
  mult_ints_op,
  mult_floats_op,
  div_ints_op,
  div_floats_op,
  mod_op,
  lt_ints_op,
  lt_floats_op,
  leq_ints_op,
  leq_floats_op,
  gt_ints_op,
  gt_floats_op,
  geq_ints_op,
  geq_floats_op,
  eq_ints_op,
  eq_floats_op,
  neq_ints_op,
  neq_floats_op,
  int_neg_op,
  float_neg_op,
  bang_op,
  var_inc_op,
  array_inc_op,
  var_dec_op,
  array_dec_op,
  if_false_op,
  if_true_op,
  goto_op,
  read_int_op,
  read_double_op,
  func_decl_op,
  push_param_op,
  pop_params_op,
  alloc_array_op, // (alloc_array_op, array id, arg holding integer size of array, -)
  return_op,
  assign_int_literal,
  assign_double_literal,
  initial_main_call,
  store_string_op
};

/* types of quad args */
enum quad_arg_type {
  no_arg_type,
  int_arg,
  dbl_arg,
  id_arg,
  str_arg
};

typedef struct quad_arg *quad_arg;
struct quad_arg {
  union {
    int int_value;
    double double_value;
    symnode var_node;
  } value;
  enum quad_arg_type arg_type;
};

typedef struct quad *quad;
struct quad {
  enum quad_op op;
  quad_arg arg1;
  quad_arg arg2;
  quad_arg arg3;

  int line_num;
};

// Initialize the quad array and store it in quad_array global variable.
//   Passing a size of -1 uses the default size.
//   There can only be one quad array at a time since variables are static and the array is global.
void init_quad_array(int size);

// Creates a quad and adds it to the quad array
quad generate_quad(int line_num, enum quad_op, quad_arg, quad_arg, quad_arg);

// Retroactively changes an argument of a previously generated quad
void patch_quad(quad, int arg_index, quad_arg new_quad_arg);

// Creates a quad argument. Value field must be set manually
quad_arg generate_quad_arg(enum quad_arg_type quad_arg_type);

// Get a new temp with the current function name as the prefix
quad_arg get_new_temp(symboltable symtab, enum vartype var_type);

// Recursive function for generating intermediate code using quads.
//  If applicable, returns the quad_arg that holds the result of an operation
quad_arg generate_intermediate_code(ast_node node);

// Prints the quad array in human-readable format
void print_quad_array();

// Prints the quad in human-readable format
void print_quad(quad the_quad);

// Prints the quad_arg in human-readable format
void print_quad_arg(quad_arg the_quad_arg);

#endif