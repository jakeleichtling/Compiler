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
  assn_op,

  read_op,
  geq_op,
  ifF_op,
  lab_op,
  gt_op,
  mult_op,
  sub_op,
  goto_op,
  halt_op,
  enter_scope_op,
  leave_scope_op,
  new_var_op
};

/* types of quad args */
enum quad_arg_type {
  no_arg_type,
  int_arg,
  dbl_arg,
  id_arg
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
  ast_node var_node;
  quad_arg arg1;
  quad_arg arg2;
  quad_arg arg3;
};

// Creates a quad and adds it to the quad array
quad generate_quad(enum quad_op, quad_arg, quad_arg, quad_arg);

// Retroactively changes an argument of a previously generated quad
void patch_quad(quad, int arg_index, quad_arg new_quad_arg);

// Creates a quad argument. Fields must be set manually
quad_arg generate_quad_arg();

// Set the temp prefix to the function name and reset the temp count at 1
void set_temp_prefix(char *func_name);

// Get a new temp with the current function name as the prefix
quad_arg get_new_temp(symboltable symtab, enum vartype var_type);

// Return right-side temps after assignment so that they can be reused
void return_temps(int num_temps_returned);

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