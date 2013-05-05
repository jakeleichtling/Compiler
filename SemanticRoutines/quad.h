#ifndef QUAD_H_
#define QUAD_H_

#include "ast.h"
#include "symtab.h"

/* quad operations */
enum quad_op {
  read_op,
  geq_op,
  ifF_op,
  assn_op,
  lab_op,
  gt_op,
  mult_op,
  sub_op,
  goto_op,
  print_op,
  halt_op,
  enter_scope_op,
  leave_scope_op,
  new_var_op
};

typedef struct quad_arg *quad_arg;
struct quad_arg {
  union {
    int int_value;
    double double_value;
    ast_node var_node;
  } value;
  enum vartype var_type;
};

typedef struct quad *quad;
struct quad {
  enum quad_op op;
  ast_node var_node;
  quad_arg arg1;
  quad_arg arg2;
  quad_arg arg3;
};

quad generate_quad(enum quad_op, quad_arg, quad_arg, quad_arg);

void patch_quad(quad, int arg_index, quad_arg new_quad_arg);

#endif