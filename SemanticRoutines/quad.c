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

// The symnode of the main function (declared in symtab.h)
extern symnode main_func_symnode;

/* human readable quad operation names */
char *quad_op_string[] = {
  "call_func_op",
  "print_int_op",
  "print_float_op",
  "print_string_op",
  "int_to_float_op",
  "assn_int_to_var_op",
  "assn_int_to_arraysub_op",
  "assn_int_from_arraysub_op",
  "assn_float_to_var_op",
  "assn_float_to_arraysub_op",
  "assn_float_from_arraysub_op",
  "add_ints_op", //
  "add_floats_op",
  "sub_ints_op", //
  "sub_floats_op",
  "mult_ints_op", //
  "mult_floats_op",
  "div_ints_op", //
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
  "int_neg_op",
  "float_neg_op",
  "bang_op",
  "var_inc_op",
  "array_inc_op",
  "var_dec_op",
  "array_dec_op",
  "if_false_op",
  "if_true_op",
  "goto_op",
  "read_int_op",
  "read_double_op",
  "func_decl_op",
  "push_param_op",
  "pop_params_op",
  "alloc_array_op",
  "return_op",
  "assign_int_literal",
  "assign_double_literal",
  "initial_main_call"
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
void push_params_recursively(int line_num, symnode callee_symnode, ast_node param_node, int i);

/* ~~~~~~~~~~~~~~~ Function Definitions ~~~~~~~~~~~~~~~~~~~ */

// Creates a quad and adds it to the quad array
quad generate_quad(int line_num, enum quad_op _op, quad_arg _arg1, quad_arg _arg2, quad_arg _arg3)
{
  quad new_quad = (quad) calloc(1, sizeof(struct quad));
  new_quad->op = _op;
  new_quad->arg1 = _arg1;
  new_quad->arg2 = _arg2;
  new_quad->arg3 = _arg3;
  new_quad->line_num = line_num;

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
    temp_symnode->var_addr = -8 * (curr_func_symnode_quad->num_vars + curr_func_symnode_quad->num_temps);
  } else {
    temp_symnode->mem_addr_type = absolute;
    num_global_temps++;
    temp_symnode->var_addr = -8 * (num_global_vars + num_global_temps);
  }

  // Make the quad_arg for the temp, pointing to the symnode
  quad_arg new_quad_arg = generate_quad_arg(id_arg);
  new_quad_arg->value.var_node = temp_symnode;

  temp_count++;
  return new_quad_arg;
}

// Recursive function for generating intermediate code using quads.
//  If applicable, returns the quad_arg that holds the result of an operation
quad_arg generate_intermediate_code(ast_node node)
{
  if (!node) {
    return NULL;
  }

  if (djdebug) {
    print_ast_node(node);
    printf("\n");
  }

  ast_node child;

  switch (node->node_type) {
    case ROOT:
    {
      // Generate code for the global variable declarations
      for (child = node->left_child; child != NULL; child = child->right_sibling) {
        if (child->node_type == VAR_DECL) {
          generate_intermediate_code(child);
        }
      }

      // Generate a quad for the initial call to main
      generate_quad(node->line_num, initial_main_call, NULL, NULL, NULL);

      // Generate code for the function declarations
      for (child = node->left_child; child != NULL; child = child->right_sibling) {
        if (child->node_type == FUNC_DECL) {
          generate_intermediate_code(child);
        }
      }

      return NULL;
    }
    case ID:
    {
      quad_arg id_quad_arg = generate_quad_arg(id_arg);
      id_quad_arg->value.var_node = node->value.sym_node;

      return id_quad_arg;
    }
    case INT_TYPE:
      return NULL;
    case DBL_TYPE:
      return NULL;
    case VOID_TYPE:
      return NULL;
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
        generate_quad(node->line_num, assn_int_from_arraysub_op, value_arg, array_arg, index_arg);
      } else {
        generate_quad(node->line_num, assn_float_from_arraysub_op, value_arg, array_arg, index_arg);
      }

      return value_arg;
    }
    case ARRAY_NONSUB:
      return NULL;
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
          generate_quad(node->line_num, assn_int_to_var_op, id_quad_arg, right_side_arg, NULL);

          return right_side_arg;
        } else { // subscripted arrays
          // Get the array ID symnode into an arg
          quad_arg array_id_arg = generate_quad_arg(id_arg);
          array_id_arg->value.var_node = node->left_child->left_child->value.sym_node;

          // Get the array index into an arg
          quad_arg index_arg = generate_intermediate_code(node->left_child->left_child->right_sibling);

          // Assign the right side to the array at the index
          generate_quad(node->line_num, assn_int_to_arraysub_op, array_id_arg, index_arg, right_side_arg);

          return right_side_arg;
        }
      } else { // floats, widening is possible
        // If the right side is an int, widen it to a float
        quad_arg right_side_float_arg;
        if (node->left_child->right_sibling->data_type == inttype) {
          right_side_float_arg = get_new_temp(flat_id_table, doubletype);
          generate_quad(node->line_num, int_to_float_op, right_side_float_arg, right_side_arg, NULL);
        } else {
          right_side_float_arg = right_side_arg;
        }

        if (node->left_child->node_type == ID) { // normal variables
          // Get the ID symnode into an arg
          quad_arg id_quad_arg = generate_quad_arg(id_arg);
          id_quad_arg->value.var_node = node->left_child->value.sym_node;

          // Assign the right side to the ID
          generate_quad(node->line_num, assn_float_to_var_op, id_quad_arg, right_side_float_arg, NULL);

          return right_side_float_arg;
        } else { // subscripted arrays
          // Get the array ID symnode into an arg
          quad_arg array_id_arg = generate_quad_arg(id_arg);
          array_id_arg->value.var_node = node->left_child->left_child->value.sym_node;

          // Get the array index into an arg
          quad_arg index_arg = generate_intermediate_code(node->left_child->left_child->right_sibling);

          // Assign the right side to the array at the index
          generate_quad(node->line_num, assn_float_to_arraysub_op, array_id_arg, index_arg, right_side_float_arg);

          return right_side_float_arg;
        }
      }
    }
    case OP_ADD:
      return generate_binary_op_with_widening(node, add_ints_op, add_floats_op);
    case OP_SUB:
      return generate_binary_op_with_widening(node, sub_ints_op, sub_floats_op);
    case OP_MULT:
      return generate_binary_op_with_widening(node, mult_ints_op, mult_floats_op);
    case OP_DIV:
      return generate_binary_op_with_widening(node, div_ints_op, div_floats_op);
    case OP_MOD:
      // We can use the standard binary operation code with widening because the type check
      //  ensures both operands are ints.
      return generate_binary_op_with_widening(node, mod_op, -1);
    case OP_LT:
      return generate_binary_op_with_widening(node, lt_ints_op, lt_floats_op);
    case OP_LEQ:
      return generate_binary_op_with_widening(node, leq_ints_op, leq_floats_op);
    case OP_GT:
      return generate_binary_op_with_widening(node, gt_ints_op, gt_floats_op);
    case OP_GEQ:
      return generate_binary_op_with_widening(node, geq_ints_op, geq_floats_op);
    case OP_EQ:
      return generate_binary_op_with_widening(node, eq_ints_op, eq_floats_op);
    case OP_NEQ:
      return generate_binary_op_with_widening(node, neq_ints_op, neq_floats_op);
    case OP_AND:
      {
        quad_arg result_arg = get_new_temp(flat_id_table, inttype);

        // Evaluate the left expression
        quad_arg left_exp_arg = generate_intermediate_code(node->left_child);

        // If it is false, jump to L1
        quad left_fail_jump_quad = generate_quad(node->line_num, if_false_op, left_exp_arg, NULL, NULL);

        // Evaluate the right expression
        quad_arg right_exp_arg = generate_intermediate_code(node->left_child->right_sibling);

        // If it is false, jump to L1
        quad right_fail_jump_quad = generate_quad(node->line_num, if_false_op, right_exp_arg, NULL, NULL);

        // Save 1 in the result argument and jump to the quad following the and operation
        quad_arg true_int_literal_arg = generate_quad_arg(int_arg);
        true_int_literal_arg->value.int_value = 1;
        generate_quad(node->line_num, assign_int_literal, result_arg, true_int_literal_arg, NULL);
        quad after_true_jump_quad = generate_quad(node->line_num, goto_op, NULL, NULL, NULL);

        // L1: Save 0 in the result argument
        quad_arg L1_arg = generate_quad_arg(int_arg);
        L1_arg->value.int_value = next_quad_index;
        quad_arg false_int_literal_arg = generate_quad_arg(int_arg);
        false_int_literal_arg->value.int_value = 0;
        generate_quad(node->line_num, assign_int_literal, result_arg, false_int_literal_arg, NULL);

        // Backpatch
        patch_quad(left_fail_jump_quad, 2, L1_arg);
        patch_quad(right_fail_jump_quad, 2, L1_arg);

        quad_arg after_and_arg = generate_quad_arg(int_arg);
        after_and_arg->value.int_value = next_quad_index;
        patch_quad(after_true_jump_quad, 1, after_and_arg);

        return result_arg;
      }
    case OP_OR:
    {
        quad_arg result_arg = get_new_temp(flat_id_table, inttype);

        // Evaluate the left expression
        quad_arg left_exp_arg = generate_intermediate_code(node->left_child);

        // If it is true, jump to L1
        quad left_succ_jump_quad = generate_quad(node->line_num, if_true_op, left_exp_arg, NULL, NULL);

        // Evaluate the right expression
        quad_arg right_exp_arg = generate_intermediate_code(node->left_child->right_sibling);

        // If it is true, jump to L1
        quad right_succ_jump_quad = generate_quad(node->line_num, if_true_op, right_exp_arg, NULL, NULL);

        // Save 0 in the result argument and jump to the quad following the or operation
        quad_arg false_int_literal_arg = generate_quad_arg(int_arg);
        false_int_literal_arg->value.int_value = 0;
        generate_quad(node->line_num, assign_int_literal, result_arg, false_int_literal_arg, NULL);
        quad after_false_jump_quad = generate_quad(node->line_num, goto_op, NULL, NULL, NULL);

        // L1: Save 1 in the result argument
        quad_arg L1_arg = generate_quad_arg(int_arg);
        L1_arg->value.int_value = next_quad_index;
        quad_arg true_int_literal_arg = generate_quad_arg(int_arg);
        true_int_literal_arg->value.int_value = 1;
        generate_quad(node->line_num, assign_int_literal, result_arg, true_int_literal_arg, NULL);

        // Backpatch
        patch_quad(left_succ_jump_quad, 2, L1_arg);
        patch_quad(right_succ_jump_quad, 2, L1_arg);

        quad_arg after_or_arg = generate_quad_arg(int_arg);
        after_or_arg->value.int_value = next_quad_index;
        patch_quad(after_false_jump_quad, 1, after_or_arg);

        return result_arg;
    }
    case OP_BANG:
    {
      quad_arg result_arg = get_new_temp(flat_id_table, inttype);
      quad_arg exp_arg = generate_intermediate_code(node->left_child);

      generate_quad(node->line_num, bang_op, result_arg, exp_arg, NULL);

      return result_arg;
    }
    case OP_NEG:
      return generate_single_operand(node, int_neg_op, float_neg_op);
    case OP_INC:
    {
      if (node->left_child->node_type == ID) {
        // Get the ID symnode
        quad_arg id_quad_arg = generate_quad_arg(id_arg);
        id_quad_arg->value.var_node = node->left_child->value.sym_node;

        generate_quad(node->line_num, var_inc_op, id_quad_arg, NULL, NULL);
        return id_quad_arg;
      } else { // Child is an ARRAY_SUB ast node
        // Get the ID symnode
        quad_arg id_quad_arg = generate_quad_arg(id_arg);
        symnode id_symnode = node->left_child->left_child->value.sym_node;
        id_quad_arg->value.var_node = id_symnode;

        // Put the index expression in an arg
        quad_arg index_arg = generate_intermediate_code(node->left_child->left_child->right_sibling);

        // Generate the quad to increment the value at that index
        generate_quad(node->line_num, array_inc_op, id_quad_arg, index_arg, NULL);

        // Put the new value at that index into a temp
        quad_arg value_temp = get_new_temp(flat_id_table, inttype);
        generate_quad(node->line_num, assn_int_from_arraysub_op, value_temp, id_quad_arg, index_arg);

        return value_temp;
      }
    }
    case OP_DEC:
      {
      if (node->left_child->node_type == ID) {
        // Get the ID symnode
        quad_arg id_quad_arg = generate_quad_arg(id_arg);
        id_quad_arg->value.var_node = node->left_child->value.sym_node;

        generate_quad(node->line_num, var_dec_op, id_quad_arg, NULL, NULL);
        return id_quad_arg;
      } else { // Child is an ARRAY_SUB ast node
        // Get the ID symnode
        quad_arg id_quad_arg = generate_quad_arg(id_arg);
        symnode id_symnode = node->left_child->left_child->value.sym_node;
        id_quad_arg->value.var_node = id_symnode;

        // Put the index expression in an arg
        quad_arg index_arg = generate_intermediate_code(node->left_child->left_child->right_sibling);

        // Generate the quad to decrement the value at that index
        generate_quad(node->line_num, array_dec_op, id_quad_arg, index_arg, NULL);

        // Put the new value at that index into a temp
        quad_arg value_temp = get_new_temp(flat_id_table, inttype);
        generate_quad(node->line_num, assn_int_from_arraysub_op, value_temp, id_quad_arg, index_arg);

        return value_temp;
      }
    }
    case FUNC_DECL:
    {
      quad_arg func_id_arg = generate_quad_arg(id_arg);
      func_id_arg->value.var_node = node->left_child->right_sibling->value.sym_node;
      generate_quad(node->line_num, func_decl_op, func_id_arg, NULL, NULL);

      curr_func_symnode_quad = node->left_child->right_sibling->value.sym_node;

      // Generate intermediate code for the function body
      generate_intermediate_code(rightmost_sibling(node->left_child));

      curr_func_symnode_quad = NULL;

      // If the return type is void, generate a blank return quad at the end of the function definition
      generate_quad(node->line_num, return_op, NULL, NULL, NULL);

      return NULL;
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

          generate_quad(node->line_num, alloc_array_op, array_id_arg, array_size_arg, NULL);
        } else if (child->node_type == OP_ASSIGN) {
          // Recurse on the assignment node
          generate_intermediate_code(child);
        }

        // Don't do anything for normal variable declarations (e.g. int x)
      }

      return NULL;
    }
    case FORMAL_PARAM:
      return NULL;
    case SEQ:
    {
      for (child = node->left_child; child != NULL; child = child->right_sibling) {
        generate_intermediate_code(child);
      }

      return NULL;
    }
    case IF_STMT:
    {
      // Generate the if condition
      quad_arg if_cond_arg = generate_intermediate_code(node->left_child);

      // Save the quad that checks the if condition to be patched
      quad exp_check_quad = generate_quad(node->line_num, if_false_op, if_cond_arg, NULL, NULL);

      // Generate the if body, including a jump quad to the end of the else stmt
      generate_intermediate_code(node->left_child->right_sibling);
      quad jump_quad = generate_quad(node->line_num, goto_op, NULL, NULL, NULL);

      // Patch the exp_check_qud to jump to the beginning of the else body
      quad_arg start_else_stmt_arg = generate_quad_arg(inttype);
      start_else_stmt_arg->value.int_value = next_quad_index;
      patch_quad(exp_check_quad, 2, start_else_stmt_arg);

      // If we have an else statement, generate the code for its body
      if (node->left_child->right_sibling->right_sibling != NULL) {
        generate_intermediate_code(node->left_child->right_sibling->right_sibling);
      }

      // Patch the jump quad at the end of the if body with the index following the if-else-stmt
      quad_arg end_else_stmt_arg = generate_quad_arg(inttype);
      end_else_stmt_arg->value.int_value = next_quad_index;
      patch_quad(jump_quad, 1, end_else_stmt_arg);

      return NULL;
    }
    case WHILE_LOOP:
    {
      // record the quad index of the expression
      quad_arg expr_quad_index_arg = generate_quad_arg(inttype);
      expr_quad_index_arg->value.int_value = next_quad_index;

      quad_arg expr_arg = generate_intermediate_code(node->left_child); // generate code for the while loop's expression
      quad expr_check_quad = generate_quad(node->line_num, if_false_op, expr_arg, NULL, NULL); // generate the quad that checks if the expression is true (destination quad filled in later)
      generate_intermediate_code(node->left_child->right_sibling); // generate quads for the interior of the while loop
      generate_quad(node->line_num, goto_op, expr_quad_index_arg, NULL, NULL); // jump back to the beginning of the loop

      // patch expr_check_quad to jump to after the loop if the expr is false
      quad_arg end_loop_index_arg = generate_quad_arg(inttype);
      end_loop_index_arg->value.int_value = next_quad_index;
      patch_quad(expr_check_quad, 2, end_loop_index_arg);

      return NULL;
    }
    case DO_WHILE_LOOP:
    {
      // record the quad index of the beginning of the loop body
      quad_arg start_loop_index_arg = generate_quad_arg(inttype);
      start_loop_index_arg->value.int_value = next_quad_index;

      generate_intermediate_code(node->left_child); // generate code for the loop body
      quad_arg expr_arg = generate_intermediate_code(node->left_child->right_sibling); // evaluate expr
      quad expr_check_quad = generate_quad(node->line_num, if_false_op, expr_arg, NULL, NULL); // check if expr is true (destination filled later)
      generate_quad(node->line_num, goto_op, start_loop_index_arg, NULL, NULL); // jump back to the beginning of the loop

      // patch expr_check_quad to jump to after loop if expr is false
      quad_arg end_loop_index_arg = generate_quad_arg(inttype);
      end_loop_index_arg->value.int_value = next_quad_index;
      patch_quad(expr_check_quad, 2, end_loop_index_arg);

      return NULL;
    }
    case FOR_STMT:
    {
      // for (x, y, z) body

      // Generate the initialization quads x
      generate_intermediate_code(node->left_child);

      // Generate the code for evaluating expression y, but first save the index of the first quad of evaluation
      quad_arg expr_eval_index_arg = generate_quad_arg(inttype);
      expr_eval_index_arg->value.int_value = next_quad_index;
      quad_arg expr_arg = generate_intermediate_code(node->left_child->right_sibling);

      // Check if expression y is true (destination filled later)
      quad expr_check_quad = generate_quad(node->line_num, if_false_op, expr_arg, NULL, NULL);

      // Generate code for the body of the loop, the iteration code z, and jumping back to the evaluation of y
      generate_intermediate_code(node->left_child->right_sibling->right_sibling->right_sibling);
      generate_intermediate_code(node->left_child->right_sibling->right_sibling);
      generate_quad(node->line_num, goto_op, expr_eval_index_arg, NULL, NULL);

      // Patch the expression check quad now that we know the index of the quad after the loop
      quad_arg end_loop_index_arg = generate_quad_arg(inttype);
      end_loop_index_arg->value.int_value = next_quad_index;
      patch_quad(expr_check_quad, 2, end_loop_index_arg);

      return NULL;
    }
    case RETURN_STMT:
    {
      // If the node has no children, then we are returning nothing
      if (node->left_child == NULL) {
        generate_quad(node->line_num, return_op, NULL, NULL, NULL);
      } else {

        // Generate code for the return expression
        quad_arg return_exp_arg = generate_intermediate_code(node->left_child);

        generate_quad(node->line_num, return_op, return_exp_arg, NULL, NULL);
     }

      return NULL;
    }
    case READ_STMT:
    {
      // create an argument holding the variable to be written to
      symnode var_symnode = node->left_child->value.sym_node;
      quad_arg var_arg = generate_quad_arg(id_arg);
      var_arg->value.var_node = var_symnode;

      // read into the temp
      if (var_symnode->var_type == inttype) {
        generate_quad(node->line_num, read_int_op, var_arg, NULL, NULL);
      } else if (var_symnode->var_type) {
        generate_quad(node->line_num, read_double_op, var_arg, NULL, NULL);
      }

      return NULL;
    }
    case PRINT_STMT:
    {
      // if the argument is a string literal then we can just print it out
      if (node->left_child->node_type == STRING_LITERAL) {
        quad_arg string_arg = generate_quad_arg(str_arg);
        string_arg->value.var_node = node->left_child->value.sym_node;
        generate_quad(node->line_num, print_string_op, string_arg, NULL, NULL);
      } else { // otherwise, evalutate the expression and then print according to data type
        quad_arg expr_arg = generate_intermediate_code(node->left_child);

        if (node->left_child->data_type == inttype) {
          generate_quad(node->line_num, print_int_op, expr_arg, NULL, NULL);
        } else if (node->left_child->data_type == doubletype) {
          generate_quad(node->line_num, print_float_op, expr_arg, NULL, NULL);
        }
      }

      return NULL;
    }
    case STRING_LITERAL:
      return NULL;
    case INT_LITERAL:
    {
      quad_arg int_lit_arg = generate_quad_arg(int_arg);
      int_lit_arg->value.int_value = node->value.int_value;
      quad_arg int_temp_arg = get_new_temp(flat_id_table, inttype);

      generate_quad(node->line_num, assign_int_literal, int_temp_arg, int_lit_arg, NULL);

      return int_temp_arg;
    }
    case DOUBLE_LITERAL:
    {
      quad_arg dbl_lit_arg = generate_quad_arg(dbl_arg);
      dbl_lit_arg->value.double_value = node->value.double_value;
      quad_arg dbl_temp_arg = get_new_temp(flat_id_table, doubletype);

      generate_quad(node->line_num, assign_double_literal, dbl_temp_arg, dbl_lit_arg, NULL);

      return dbl_temp_arg;
    }
    case FUNC_CALL:
    {
      // Push the actual parameters from back to front
      push_params_recursively(node->line_num, node->left_child->value.sym_node, node->left_child->right_sibling, 0);

      // Put the func symnode in func_id_arg
      quad_arg func_id_arg = generate_quad_arg(id_arg);
      func_id_arg->value.var_node = node->left_child->value.sym_node;

      // Make a temp to hold the return value if the function has a return type
      enum vartype func_ret_type = node->left_child->value.sym_node->var_type;
      quad_arg return_val_temp_arg = NULL;
      if (func_ret_type != voidtype) {
        return_val_temp_arg = get_new_temp(flat_id_table, func_ret_type);
        generate_quad(node->line_num, call_func_op, func_id_arg, return_val_temp_arg, NULL);
      } else {
        generate_quad(node->line_num, call_func_op, func_id_arg, NULL, NULL);
      }

      //pop params
      quad_arg num_params_arg = generate_quad_arg(int_arg);
      num_params_arg->value.int_value = node->left_child->value.sym_node->num_params;
      generate_quad(node->line_num, pop_params_op, num_params_arg, NULL, NULL);

      return return_val_temp_arg;
    }
    case EMPTY_EXPR:
      return NULL;
    default: // just to make -Wall happy
      return NULL;
  }
}

// Generates code for the standard binary operation with widening
//  +, -, *, /
quad_arg generate_binary_op_with_widening(ast_node node, enum quad_op quad_op_ints, enum quad_op quad_op_floats)
{
  quad_arg result_arg;
  quad_arg temp1;

  quad_arg left_arg = generate_intermediate_code(node->left_child);
  quad_arg right_arg = generate_intermediate_code(node->left_child->right_sibling);

  if (node->left_child->data_type == inttype && node->left_child->right_sibling->data_type == inttype) {
    result_arg = get_new_temp(flat_id_table, inttype);
    generate_quad(node->line_num, quad_op_ints, result_arg, left_arg, right_arg);
  } else {
    result_arg = get_new_temp(flat_id_table, doubletype);

    if (node->left_child->data_type == doubletype && node->left_child->right_sibling->data_type == doubletype) {
      generate_quad(node->line_num, quad_op_floats, result_arg, left_arg, right_arg);
    } else if (node->left_child->data_type == doubletype) {
      temp1 = get_new_temp(flat_id_table, doubletype);
      generate_quad(node->line_num, int_to_float_op, temp1, right_arg, NULL);
      generate_quad(node->line_num, quad_op_floats, result_arg, left_arg, temp1);
    } else if (node->left_child->right_sibling->data_type == doubletype) {
      temp1 = get_new_temp(flat_id_table, doubletype);
      generate_quad(node->line_num, int_to_float_op, temp1, left_arg, NULL);
      generate_quad(node->line_num, quad_op_floats, result_arg, temp1, right_arg);
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
    generate_quad(node->line_num, quad_op_int, result_arg, left_arg, NULL);
  } else if (node->left_child->data_type == doubletype) {
    result_arg = get_new_temp(flat_id_table, doubletype);
    generate_quad(node->line_num, quad_op_float, result_arg, left_arg, NULL);
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
    expand_quad_array();
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
  printf(") (%d)\n", the_quad->line_num);
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
void push_params_recursively(int line_num, symnode callee_symnode, ast_node param_node, int i)
{
  if (param_node == NULL) {
    return;
  }

  // Recurse on right sibling
  push_params_recursively(line_num, callee_symnode, param_node->right_sibling, i + 1);

  // Get the result of the param computation
  quad_arg param_arg = generate_intermediate_code(param_node);

  // Check if autowidening is necessary
  symnode formal_param = callee_symnode->param_symnode_array[i];
  if (formal_param->node_type != array_node &&
      formal_param->var_type == doubletype &&
      param_node->data_type == inttype) {
    quad_arg float_temp = get_new_temp(flat_id_table, doubletype);
    generate_quad(line_num, int_to_float_op, float_temp, param_arg, NULL);
    generate_quad(line_num, push_param_op, float_temp, NULL, NULL);
  } else {
    generate_quad(line_num, push_param_op, param_arg, NULL, NULL);
  }
}