#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "quad.h"

extern quad *quad_array;
extern int next_quad_index;
extern int quad_array_size;

extern symboltable id_table;

char *curr_func_name;
int temp_count = 0;

/* human readable quad operation names */
char *quad_op_string[] = {
  "call_func_op",
  "print_int_op",
  "print_float_op",
  "print_string_op",
  "int_to_float_op",
  "assn_op"
};

void add_quad_to_array(quad new_quad);

// Creates a quad and adds it to the quad array
quad generate_quad(enum quad_op _op, quad_arg _arg1, quad_arg _arg2, quad_arg _arg3)
{
  quad new_quad = (quad) calloc(1, sizeof(struct quad));
  new_quad->op = _op;
  new_quad->arg1 = _arg1;
  new_quad->arg2 = _arg2;
  new_quad->arg3 = _arg3;

  add_quad_to_array(new_quad);
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

// Creates a quad argument. Fields must be set manually
quad_arg generate_quad_arg()
{
  quad_arg new_quad_arg = (quad_arg) calloc(1, sizeof(struct quad_arg));
}

// Set the temp prefix to the function name and reset the temp count at 1
void set_temp_prefix(char *func_name)
{
  curr_func_name = strdup(func_name);
  temp_count = 0;
}

// Get a new temp with the current function name as the prefix
quad_arg get_new_temp(symboltable symtab, enum vartype var_type)
{
  int digits_counter_num = temp_count / 10;
  int num_digits = 1;
  while (digits_counter_num != 0) {
    num_digits++;
    digits_counter_num /= 10;
  }

  int basename_len = strlen(curr_func_name) + num_digits + 4;
  char *basename = calloc(basename_len + 1, sizeof('a'));
  snprintf(basename, basename_len + 1, "temp%d", temp_count);

  // Make the symbol table node for the temp
  symnode temp_symnode = insert_into_symboltable_with_prefix(symtab, basename, curr_func_name);
  temp_symnode->var_type = var_type;
  temp_symnode->node_type = val_node;

  // Make the quad_arg for the temp, pointing to the symnode
  quad_arg new_quad_arg = generate_quad_arg();
  new_quad_arg->value.var_node = temp_symnode;
  new_quad_arg->arg_type = id_arg;

  temp_count++;
  return new_quad_arg;
}

// Recursive function for generating intermediate code using quads.
//  If applicable, returns the quad_arg that holds the result of an operation
quad_arg generate_intermediate_code(ast_node node)
{
  if (!node)
    return;

  quad_arg arg1, arg2, arg3;
  arg1 = generate_quad_arg();
  arg2 = generate_quad_arg();
  arg3 = generate_quad_arg();

  quad_arg left_arg = NULL;
  quad_arg right_arg = NULL;
  quad_arg result_arg = NULL;
  quad_arg temp1 = NULL;

  switch (node->node_type) {
    case ROOT:
      generate_intermediate_code(node->left_child);
      break;
    case ID:
      // Nada
      break;
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
      // TODO, ahhhhh
      // declaring
      // assigning
      // using value
      break;
    case ARRAY_NONSUB:
      // Nada
      break;
    case OP_ASSIGN:
      // TODO!!!
      break;
    case OP_ADD:
      left_arg = generate_intermediate_code(node->left_child);
      right_arg = generate_intermediate_code(node->left_child->right_sibling);

      if (node->left_child->data_type == inttype && node->left_child->right_sibling->data_type == inttype) {
        result_arg = get_new_temp(id_table, inttype);
        generate_quad(add_ints, result_arg, left_arg, right_arg);
      } else {
        result_arg = get_new_temp(id_table, doubletype);

        if (node->left_child->data_type == doubletype && node->left_child->right_sibling->data_type == doubletype) {
          generate_quad(add_floats, result_arg, left_arg, right_arg);
        } else if (node->left_child->data_type == doubletype) {
          temp1 = get_new_temp(id_table, doubletype);
          generate_quad(int_to_float_op, temp1, right_arg, NULL);
          generate_quad(add_floats, result_arg, left_arg, temp1);
        } else if (node->left_child->right_sibling->data_type == doubletype) {
          temp1 = get_new_temp(id_table, doubletype);
          generate_quad(int_to_float_op, temp1, left_arg, NULL);
          generate_quad(add_floats, result_arg, temp1, right_arg);
        }
      }

      break;
    case OP_SUB:

      break;
    case OP_MULT:

      break;
    case OP_DIV:

      break;
    case OP_MOD:

      break;
    case OP_LT:

      break;
    case OP_LEQ:

      break;
    case OP_GT:

      break;
    case OP_GEQ:

      break;
    case OP_EQ:

      break;
    case OP_NEQ:

      break;
    case OP_AND:

      break;
    case OP_OR:

      break;
    case OP_BANG:

      break;
    case OP_NEG:

      break;
    case OP_INC:

      break;
    case OP_DEC:

      break;
    case FUNC_DECL:

      break;
    case VAR_DECL:

      break;
    case FORMAL_PARAM:

      break;
    case SEQ:

      break;
    case IF_STMT:

      break;
    case WHILE_LOOP:

      break;
    case DO_WHILE_LOOP:

      break;
    case FOR_STMT:

      break;
    case RETURN_STMT:

      break;
    case READ_STMT:

      break;
    case PRINT_STMT:
      arg1->arg_type = id_arg;

      if (node->left_child->node_type == STRING_LITERAL) {
        arg1->value.var_node = node->left_child->value.sym_node;
        generate_quad(print_string_op, arg1, NULL, NULL);
      } else {
        quad_arg expr_val = generate_intermediate_code(node->left_child);

        if (node->left_child->data_type == inttype) {
          generate_quad(print_int_op, expr_val, NULL, NULL);
        } else if (node->left_child->data_type == doubletype) {
          generate_quad(print_float_op, expr_val, NULL, NULL);
        }
      }

      break;
    case STRING_LITERAL:
      // Nada
      break;
    case INT_LITERAL:
      // Nada
      break;
    case DOUBLE_LITERAL:
      // Nada
      break;
    case FUNC_CALL:
      arg1->arg_type = id_arg;
      generate_quad(call_func_op, arg1, NULL, NULL);
      break;
    case EMPTY_EXPR:
      // Nada
      break;
  }

  return result_arg;
}

// Add a quad to the array
void add_quad_to_array(quad new_quad)
{
  if (next_quad_index >= quad_array_size) {
    fprintf(stderr, "The quad array has been filled!\n");
    exit(1);
  }
}

// Prints the quad array in human-readable format
void print_quad_array()
{
  int i;
  for (i = 0; i < next_quad_index && i < quad_array_size; i++)
    print_quad(quad_array[i]);
}

// Prints the quad in human-readable format
void print_quad(quad the_quad)
{
  printf("(%s, ", quad_op_string[the_quad->op]);
  print_quad_arg(the_quad->arg1);
  print_quad_arg(the_quad->arg2);
  print_quad_arg(the_quad->arg3);
  printf("\n");
}

// Prints the quad_arg in human-readable format
void print_quad_arg(quad_arg the_quad_arg)
{
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