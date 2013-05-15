#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "quad.h"

#define DEFAULT_QUAD_ARRAY_SIZE 4096

int next_quad_index;
int quad_array_size;
quad *quad_array;

// The number of global temp variables
int num_global_temps = 0;

symnode curr_func_symnode_quad;

// The number of global variables, imported from ast_node_processing.c
extern int num_global_vars;

extern symboltable flat_id_table;

extern int djdebug;

int temp_count = 0;

/* human readable quad operation names */
char *quad_op_string[] = {
  "call_func_op",
  "print_int_op",
  "print_float_op",
  "print_string_op",
  "int_to_float_op",
  "assn_var_op",
  "assn_to_arraysub_op",
  "assn_from_arraysub_op",
  "add_ints_op",
  "add_floats_op",
  "sub_ints_op",
  "sub_floats_op",
  "mult_ints_op",
  "mult_floats_op",
  "div_ints_op",
  "div_floats_op",
  "mod_op",
  "lt_ints_op",
  "lt_floats_op",
  "leq_ints_op",
  "leq_floats_op",
  "gt_ints_op",
  "gt_floats_op",
  "geq_ints_op",
  "geq_floats_op",
  "eq_ints_op",
  "eq_floats_op",
  "neq_ints_op",
  "neq_floats_op",
  "and_ints_op",
  "and_floats_op",
  "or_ints_op",
  "or_floats_op",
  "int_bang_op",
  "float_bang_op",
  "int_neg_op",
  "float_neg_op",
  "int_inc_op",
  "array_inc_op",
  "int_dec_op",
  "array_dec_op",
  "if_false_op",
  "goto_op",
  "read_int_op",
  "read_double_op",
  "halt_op",
  "func_decl_op",
  "push_param_op"
};

/* ~~~~~~~~~~~~~~~ Function Prototypes ~~~~~~~~~~~~~~~~~~~ */

// Generates code for the standard binary operation with widening
//  +, -, *, /
quad_arg generate_binary_op_with_widening(ast_node node, enum quad_op quad_op_ints, enum quad_op quad_op_floats);

// Generates code for the standard single operand operation
//  !, - (neg), ++, --
quad_arg generate_single_operand(ast_node node, enum quad_op quad_op_int, enum quad_op quad_op_float);

void add_quad_to_array(quad new_quad);

// Doubles the size of the quad_array and copies over the existing elements
void expand_quad_array();

// Push the parameters from back to front, using autowidening when necessary
void push_params_recursively(symnode callee_symnode, ast_node param_node, int i);

/* ~~~~~~~~~~~~~~~ Function Definitions ~~~~~~~~~~~~~~~~~~~ */

// Creates a quad and adds it to the quad array
quad generate_quad(enum quad_op _op, quad_arg _arg1, quad_arg _arg2, quad_arg _arg3)
{
  quad new_quad = (quad) calloc(1, sizeof(struct quad));
  new_quad->op = _op;
  new_quad->arg1 = _arg1;
  new_quad->arg2 = _arg2;
  new_quad->arg3 = _arg3;

  add_quad_to_array(new_quad);

  return new_quad;
}

// Retroactively changes an argument of a previously generated quad
void patch_quad(quad q, int arg_index, quad_arg new_quad_arg)
{
  switch (arg_index) {
    case 1:
      q->arg1 = new_quad_arg;
      break;
    case 2:
      q->arg2 = new_quad_arg;
      break;
    case 3:
      q->arg3 = new_quad_arg;
      break;
    default:
      fprintf(stderr, "patch_quad() requires an argument index between 1 and 3 (inclusive).");
  }
}

// Creates a quad argument. Value field must be set manually
quad_arg generate_quad_arg(enum quad_arg_type quad_arg_type)
{
  quad_arg new_quad_arg = (quad_arg) calloc(1, sizeof(struct quad_arg));
  new_quad_arg->arg_type = quad_arg_type;

  return new_quad_arg;
}

// Get a new temp with a unique name
quad_arg get_new_temp(symboltable symtab, enum vartype var_type)
{
  int digits_counter_num = temp_count / 10;
  int num_digits = 1;
  while (digits_counter_num != 0) {
    num_digits++;
    digits_counter_num /= 10;
  }

  int tempname_len = num_digits + 4;
  char *tempname = calloc(tempname_len + 1, sizeof('a'));
  snprintf(tempname, tempname_len + 1, "temp%d", temp_count);

  // Make the symbol table node for the temp
  symnode temp_symnode = insert_into_symboltable(symtab, tempname);
  temp_symnode->var_type = var_type;
  temp_symnode->node_type = val_node;

  // Set the memory address of the temp
  //   and increment the number of temps in the current function or global temps
  if (curr_func_symnode_quad != NULL) {
    temp_symnode->mem_addr_type = off_fp;
    curr_func_symnode_quad->num_temps++;
    temp_symnode->var_addr = -8 * (curr_func_symnode_quad->num_vars + curr_func_symnode_quad->num_temps) - 4;
  } else {
    temp_symnode->mem_addr_type = global;
    num_global_temps++;
    temp_symnode->var_addr = -8 * (num_global_vars + num_global_temps);
  }

  // Make the quad_arg for the temp, pointing to the symnode
  quad_arg new_quad_arg = generate_quad_arg(id_arg);
  new_quad_arg->value.var_node = temp_symnode;

  temp_count++;
  return new_quad_arg;
}

// TODO: Check to make sure all cases are covered!
// Recursive function for generating intermediate code using quads.
//  If applicable, returns the quad_arg that holds the result of an operation
quad_arg generate_intermediate_code(ast_node node)
{
  if (!node)
    return;

  if (djdebug) {
    print_ast_node(node);
    printf("\n");
  }

  ast_node child;

  quad_arg left_arg = NULL;
  quad_arg right_arg = NULL;
  quad_arg result_arg = NULL;
  quad_arg temp1 = NULL;

  switch (node->node_type) {
    case ROOT:
      for (child = node->left_child; child != NULL; child = child->right_sibling) {
        generate_intermediate_code(child);
      }

      break;
    case ID:
    {
      quad_arg id_quad_arg = generate_quad_arg(id_arg);
      id_quad_arg->value.var_node = node->value.sym_node;

      return id_quad_arg;
    }
    case INT_TYPE:
      // Nada
      break;
    case DBL_TYPE:
      // Nada
      break;
    case VOID_TYPE:
      // Nada
      break;
    case ARRAY_SUB:
    {
      // Get the array ID symnode
      symnode id_symnode = node->left_child->value.sym_node;

      // The temp where the value of the array at the index will be stored
      quad_arg value_arg = get_new_temp(flat_id_table, id_symnode->var_type);

      // The quad arg that points to the array
      quad_arg array_arg = generate_quad_arg(id_arg);
      array_arg->value.var_node = node->left_child->value.sym_node;

      // The quad arg specifying the index
      quad_arg index_arg = generate_intermediate_code(node->left_child->right_sibling);

      if (id_symnode->var_type == inttype) {
        generate_quad(assn_int_from_arraysub_op, value_arg, array_arg, index_arg);
      } else {
        generate_quad(assn_float_from_arraysub_op, value_arg, array_arg, index_arg);
      }
      return value_arg;
    }
    case ARRAY_NONSUB:
      // Nada
      break;
    case OP_ASSIGN:
    {
      // Put the right side of the assignment into an arg
      quad_arg right_side_arg = generate_intermediate_code(node->left_child->right_sibling);

      if (node->data_type == inttype) { // ints
        if (node->left_child->node_type == ID) { // normal variables
          // Get the ID symnode into an arg
          quad_arg id_quad_arg = generate_quad_arg(id_arg);
          id_quad_arg->value.var_node = node->left_child->value.sym_node;

          // Assign the right side to the ID
          generate_quad(assn_int_to_var_op, id_quad_arg, right_side_arg, NULL);

          return right_side_arg;
        } else { // subscripted arrays
          // Get the array ID symnode into an arg
          quad_arg array_id_arg = generate_quad_arg(id_arg);
          array_id_arg->value.var_node = node->left_child->left_child->value.sym_node;

          // Get the array index into an arg
          quad_arg index_arg = generate_intermediate_code(node->left_child->left_child->right_sibling);

          // Assign the right side to the array at the index
          generate_quad(assn_int_to_arraysub_op, array_id_arg, index_arg, right_side_arg);

          return right_side_arg;
        }
      } else { // floats, widening is possible
        // If the right side is an int, widen it to a float
        quad_arg right_side_float_arg;
        if (node->left_child->right_sibling->data_type == inttype) {
          right_side_float_arg = get_new_temp(flat_id_table, doubletype);
          generate_quad(int_to_float_op, right_side_float_arg, right_side_arg, NULL);
        } else {
          right_side_float_arg = right_side_arg;
        }

        if (node->left_child->node_type == ID) { // normal variables
          // Get the ID symnode into an arg
          quad_arg id_quad_arg = generate_quad_arg(id_arg);
          id_quad_arg->value.var_node = node->left_child->value.sym_node;

          // Assign the right side to the ID
          generate_quad(assn_float_to_var_op, id_quad_arg, right_side_float_arg, NULL);

          return right_side_float_arg;
        } else { // subscripted arrays
          // Get the array ID symnode into an arg
          quad_arg array_id_arg = generate_quad_arg(id_arg);
          array_id_arg->value.var_node = node->left_child->left_child->value.sym_node;

          // Get the array index into an arg
          quad_arg index_arg = generate_intermediate_code(node->left_child->left_child->right_sibling);

          // Assign the right side to the array at the index
          generate_quad(assn_float_to_arraysub_op, array_id_arg, index_arg, right_side_float_arg);

          return right_side_float_arg;
        }
      }
    }
    case OP_ADD:
      result_arg = generate_binary_op_with_widening(node, add_ints_op, add_floats_op);

      break;
    case OP_SUB:
      result_arg = generate_binary_op_with_widening(node, sub_ints_op, sub_floats_op);

      break;
    case OP_MULT:
      result_arg = generate_binary_op_with_widening(node, mult_ints_op, mult_floats_op);

      break;
    case OP_DIV:
      result_arg = generate_binary_op_with_widening(node, div_ints_op, div_floats_op);

      break;
    case OP_MOD:
      // We can use the standard binary operation code with widening because the type check
      //  ensures both operands are ints.
      result_arg = generate_binary_op_with_widening(node, mod_op, -1);

      break;
    case OP_LT:
      result_arg = generate_binary_op_with_widening(node, lt_ints_op, lt_floats_op);

      break;
    case OP_LEQ:
      result_arg = generate_binary_op_with_widening(node, leq_ints_op, leq_floats_op);

      break;
    case OP_GT:
      result_arg = generate_binary_op_with_widening(node, gt_ints_op, gt_floats_op);
 
      break;
    case OP_GEQ:
      result_arg = generate_binary_op_with_widening(node, geq_ints_op, geq_floats_op);

      break;
    case OP_EQ:
      result_arg = generate_binary_op_with_widening(node, eq_ints_op, eq_floats_op);

      break;
    case OP_NEQ:
      result_arg = generate_binary_op_with_widening(node, neq_ints_op, neq_floats_op);

      break;
    case OP_AND:
      result_arg = generate_binary_op_with_widening(node, and_ints_op, and_floats_op);

      break;
    case OP_OR:
      result_arg = generate_binary_op_with_widening(node, or_ints_op, or_floats_op);

      break;
    case OP_BANG:
      result_arg = generate_single_operand(node, int_bang_op, float_bang_op);

      break;
    case OP_NEG:
      result_arg = generate_single_operand(node, int_neg_op, float_neg_op);

      break;
    case OP_INC:
    {
      if (node->left_child->node_type == ID) {
        // Get the ID symnode
        quad_arg id_quad_arg = generate_quad_arg(id_arg);
        id_quad_arg->value.var_node = node->left_child->value.sym_node;

        generate_quad(var_inc_op, id_quad_arg, NULL, NULL);
        return id_quad_arg;
      } else { // Child is an ARRAY_SUB ast node
        // Get the ID symnode
        quad_arg id_quad_arg = generate_quad_arg(id_arg);
        symnode id_symnode = node->left_child->left_child->value.sym_node;
        id_quad_arg->value.var_node = id_symnode;

        // Put the index expression in an arg
        quad_arg index_arg = generate_intermediate_code(node->left_child->left_child->right_sibling);

        // Generate the quad to increment the value at that index
        generate_quad(array_inc_op, id_quad_arg, index_arg, NULL);

        // Put the new value at that index into a temp
        quad_arg value_temp = get_new_temp(flat_id_table, inttype);
        generate_quad(assn_int_from_arraysub_op, value_temp, id_quad_arg, index_arg);

        return value_temp;
      }
    }
    case OP_DEC:
      {
      if (node->left_child->node_type == ID) {
        // Get the ID symnode
        quad_arg id_quad_arg = generate_quad_arg(id_arg);
        id_quad_arg->value.var_node = node->left_child->value.sym_node;

        generate_quad(var_dec_op, id_quad_arg, NULL, NULL);
        return id_quad_arg;
      } else { // Child is an ARRAY_SUB ast node
        // Get the ID symnode
        quad_arg id_quad_arg = generate_quad_arg(id_arg);
        symnode id_symnode = node->left_child->left_child->value.sym_node;
        id_quad_arg->value.var_node = id_symnode;

        // Put the index expression in an arg
        quad_arg index_arg = generate_intermediate_code(node->left_child->left_child->right_sibling);

        // Generate the quad to decrement the value at that index
        generate_quad(array_dec_op, id_quad_arg, index_arg, NULL);

        // Put the new value at that index into a temp
        quad_arg value_temp = get_new_temp(flat_id_table, inttype);
        generate_quad(assn_int_from_arraysub_op, value_temp, id_quad_arg, index_arg);

        return value_temp;
      }
    }
    case FUNC_DECL:
    {
      quad_arg func_id_arg = generate_quad_arg(id_arg);
      func_id_arg->value.var_node = node->left_child->right_sibling->value.sym_node;
      generate_quad(func_decl_op, func_id_arg, NULL, NULL);

      curr_func_symnode_quad = node->left_child->right_sibling->value.sym_node;
      generate_intermediate_code(node->left_child->right_sibling->right_sibling);
      curr_func_symnode_quad = NULL;

      break;
    }
    case VAR_DECL:
    {
      ast_node child;
      for (child = node->left_child; child != NULL; child = child->right_sibling) {
        if (child->node_type == ARRAY_SUB) {
          // Get the array ID symnode into an arg
          quad_arg array_id_arg = generate_quad_arg(id_arg);
          array_id_arg->value.var_node = child->left_child->value.sym_node;

          // Get the size of the array into an arg
          quad_arg array_size_arg = generate_intermediate_code(child->left_child->right_sibling);

          generate_quad(alloc_array_op, array_id_arg, array_size_arg, NULL);
        } else if (child->node_type == OP_ASSIGN) {
          // Recurse on the assignment node
          generate_intermediate_code(child);
        }

        // Don't do anything for normal variable declarations (e.g. int x)
      }

      return NULL;
    }
    case FORMAL_PARAM:
      // Nada
      break;
    case SEQ:
      for (child = node->left_child; child != NULL; child = child->right_sibling)
        generate_intermediate_code(child);
      break;
    case IF_STMT:
      left_arg = generate_intermediate_code(node->left_child);
      quad exp_check_quad = generate_quad(if_false_op, left_arg, NULL, NULL);
      generate_intermediate_code(node->left_child->right_sibling);
      quad jump_quad = generate_quad(goto_op, left_arg, NULL, NULL);

      quad_arg start_else_stmt_arg = generate_quad_arg(inttype);
      start_else_stmt_arg->value.int_value = next_quad_index;
      patch_quad(exp_check_quad, 2, start_else_stmt_arg);

      generate_intermediate_code(node->left_child->right_sibling->right_sibling);
      quad_arg end_else_stmt_arg = generate_quad_arg(inttype);
      end_else_stmt_arg->value.int_value = next_quad_index;
      patch_quad(jump_quad, 1, end_else_stmt_arg);

      break;
    case WHILE_LOOP:
    {
      // record the quad index of the expression
      quad_arg expr_quad_index_arg = generate_quad_arg(inttype);
      expr_quad_index_arg->value.int_value = next_quad_index;

      quad_arg expr_arg = generate_intermediate_code(node->left_child); // generate code for the while loop's expression
      quad expr_check_quad = generate_quad(if_false_op, expr_arg, NULL, NULL); // generate the quad that checks if the expression is true (destination quad filled in later)
      generate_intermediate_code(node->left_child->right_sibling); // generate quads for the interior of the while loop
      generate_quad(goto_op, expr_quad_index_arg, NULL, NULL); // jump back to the beginning of the loop

      // patch expr_check_quad to jump to after the loop if the expr is false
      quad_arg end_loop_index_arg = generate_quad_arg(inttype);
      end_loop_index_arg->value.int_value = next_quad_index;
      patch_quad(expr_check_quad, 2, end_loop_index_arg);

      break;
    }
    case DO_WHILE_LOOP:
    {
      // record the quad index of the beginning of the loop body
      quad_arg start_loop_index_arg = generate_quad_arg(inttype);
      start_loop_index_arg->value.int_value = next_quad_index;

      generate_intermediate_code(node->left_child); // generate code for the loop body
      quad_arg expr_arg = generate_intermediate_code(node->left_child->right_sibling); // evaluate expr
      quad expr_check_quad = generate_quad(if_false_op, expr_arg, NULL, NULL); // check if expr is true (destination filled later)
      generate_quad(goto_op, start_loop_index_arg, NULL, NULL); // jump back to the beginning of the loop

      // patch expr_check_quad to jump to after loop if expr is false
      quad_arg end_loop_index_arg = generate_quad_arg(inttype);
      end_loop_index_arg->value.int_value = next_quad_index;
      patch_quad(expr_check_quad, 2, end_loop_index_arg);

      break;
    }
    case FOR_STMT:
      // TODO
      break;
    case RETURN_STMT:
      // TODO
      break;
    case READ_STMT:
    {
      // create a temp of the proper type
      enum vartype var_type = node->left_child->data_type;
      quad_arg temp_arg = get_new_temp(flat_id_table, var_type);

      // read into the temp
      if (var_type == inttype) {
        generate_quad(read_int_op, temp_arg, NULL, NULL);
      } else if (var_type == doubletype) {
        generate_quad(read_double_op, temp_arg, NULL, NULL);
      }

      // assign the temp to the var
      // TODO: wait until assign is finished

      break;
    }
    case PRINT_STMT:
    {
      // if the argument is a string literal then we can just print it out
      if (node->left_child->node_type == STRING_LITERAL) {
        quad_arg string_arg = generate_quad_arg(str_arg);
        string_arg->value.var_node = node->left_child->value.sym_node;
        generate_quad(print_string_op, string_arg, NULL, NULL);
      } else { // otherwise, evalutate the expression and then print according to data type
        quad_arg expr_arg = generate_intermediate_code(node->left_child);

        if (node->left_child->data_type == inttype) {
          generate_quad(print_int_op, expr_arg, NULL, NULL);
        } else if (node->left_child->data_type == doubletype) {
          generate_quad(print_float_op, expr_arg, NULL, NULL);
        }
      }

      break;
    }
    case STRING_LITERAL:
      // Nada
      break;
    case INT_LITERAL:
    {
      // TODO: change to use parsed int from node and store that into a temp using a quad
      quad_arg int_quad_arg = generate_quad_arg(int_arg);
      int_quad_arg->value.int_value = node->value.int_value;

      return int_quad_arg;
    }
    case DOUBLE_LITERAL:
      // TODO: change to use parsed double from node and store that into a temp using a quad
      // Nada
      break;
    case FUNC_CALL:
      // Push the actual parameters from back to front
      push_params_recursively(node->left_child->value.sym_node, node->left_child->right_sibling, 0);

      left_arg = generate_quad_arg(id_arg);
      left_arg->value.var_node = node->left_child->value.sym_node;
      generate_quad(call_func_op, left_arg, NULL, NULL);
      break;
    case EMPTY_EXPR:
      // Nada
      break;
  }

  return result_arg;
}

// Generates code for the standard binary operation with widening
//  +, -, *, /
quad_arg generate_binary_op_with_widening(ast_node node, enum quad_op quad_op_ints, enum quad_op quad_op_floats)
{
  quad_arg result_arg;
  quad_arg temp1;

  quad_arg left_arg = generate_intermediate_code(node->left_child);
  quad_arg right_arg = generate_intermediate_code(node->left_child->right_sibling);

  printf("@ %d <-> %d\n", node->left_child->data_type, node->left_child->right_sibling->data_type);

  if (node->left_child->data_type == inttype && node->left_child->right_sibling->data_type == inttype) {
    result_arg = get_new_temp(flat_id_table, inttype);
    generate_quad(quad_op_ints, result_arg, left_arg, right_arg);
  } else {
    result_arg = get_new_temp(flat_id_table, doubletype);

    if (node->left_child->data_type == doubletype && node->left_child->right_sibling->data_type == doubletype) {
      generate_quad(quad_op_floats, result_arg, left_arg, right_arg);
    } else if (node->left_child->data_type == doubletype) {
      temp1 = get_new_temp(flat_id_table, doubletype);
      generate_quad(int_to_float_op, temp1, right_arg, NULL);
      generate_quad(quad_op_floats, result_arg, left_arg, temp1);
    } else if (node->left_child->right_sibling->data_type == doubletype) {
      temp1 = get_new_temp(flat_id_table, doubletype);
      generate_quad(int_to_float_op, temp1, left_arg, NULL);
      generate_quad(quad_op_floats, result_arg, temp1, right_arg);
    }
  }

  return result_arg;
}

// Generates code for the standard single operand operation
//  !, - (neg), ++, --
quad_arg generate_single_operand(ast_node node, enum quad_op quad_op_int, enum quad_op quad_op_float)
{
  quad_arg result_arg;

  quad_arg left_arg = generate_intermediate_code(node->left_child);

  if (node->left_child->data_type == inttype) {
    result_arg = get_new_temp(flat_id_table, inttype);
    generate_quad(quad_op_int, result_arg, left_arg, NULL);
  } else if (node->left_child->data_type == doubletype) {
    result_arg = get_new_temp(flat_id_table, doubletype);
    generate_quad(quad_op_float, result_arg, left_arg, NULL);
  }

  return result_arg;
}

// Add a quad to the array
void add_quad_to_array(quad new_quad)
{
  if (djdebug) {
    printf("\t%d:\t", next_quad_index);
    print_quad(new_quad);
  }

  if (next_quad_index >= quad_array_size) {
    expand_quad_array;
  }

  quad_array[next_quad_index] = new_quad;
  next_quad_index++;
}

// Prints the quad array in human-readable format
void print_quad_array()
{
  int i;
  for (i = 0; i < next_quad_index && i < quad_array_size; i++) {
    printf("%d:\t", i);
    print_quad(quad_array[i]);
  }
}

// Prints the quad in human-readable format
void print_quad(quad the_quad)
{
  printf("(%s, ", quad_op_string[the_quad->op]);
  print_quad_arg(the_quad->arg1);
  printf(", ");
  print_quad_arg(the_quad->arg2);
  printf(", ");
  print_quad_arg(the_quad->arg3);
  printf(")\n");
}

// Prints the quad_arg in human-readable format
void print_quad_arg(quad_arg the_quad_arg)
{
  if (!the_quad_arg) {
    printf("null");
    return;
  }

  switch (the_quad_arg->arg_type) {
    case int_arg:
      printf("int: %d", the_quad_arg->value.int_value);
      break;
    case dbl_arg:
      printf("double: %f", the_quad_arg->value.double_value);
      break;
    case id_arg:
      printf("ID: %s", the_quad_arg->value.var_node->name);
      break;
    default:
      printf("arg type not specified");
      break;
  }
}

// Initialize the quad array and store it in quad_array global variable.
//   Passing a nonpositive size (e.g. -1) uses the default size.
//   There can only be one quad array at a time since variables are static and the array is global.
void init_quad_array(int size)
{
  if (size > 0) {
    quad_array_size = size;
  } else {
    quad_array_size = DEFAULT_QUAD_ARRAY_SIZE;
  }

  quad_array = calloc(quad_array_size, sizeof(quad));
  next_quad_index = 0;
}

// Doubles the size of the quad_array and copies over the existing elements
void expand_quad_array()
{
  // Allocate memory for the new quad_array
  quad_array_size *= 2;
  quad *new_quad_array = calloc(quad_array_size, sizeof(quad));

  // Copy over the existing struct quad pointers
  int i;
  for (i = 0; i < next_quad_index; i++) {
    new_quad_array[i] = quad_array[i];
  }

  // Free the old quad_array
  free(quad_array);

  quad_array = new_quad_array;
}

// Push the parameters from back to front, using autowidening when necessary
void push_params_recursively(symnode callee_symnode, ast_node param_node, int i)
{
  if (param_node == NULL) {
    return;
  }

  // Recurse on right sibling
  push_params_recursively(callee_symnode, param_node->right_sibling, i + 1);

  // Get the result of the param computation
  quad_arg param_arg = generate_intermediate_code(param_node);

  // Check if autowidening is necessary
  symnode formal_param = callee_symnode->param_symnode_array[i];
  if (formal_param->node_type != array_node &&
      formal_param->var_type == doubletype &&
      param_node->data_type == inttype) {
    quad_arg float_temp = get_new_temp(flat_id_table, doubletype);
    generate_quad(int_to_float_op, float_temp, param_arg, NULL);
    generate_quad(push_param_op, float_temp, NULL, NULL);
  } else {
    generate_quad(push_param_op, param_arg, NULL, NULL);
  }
}