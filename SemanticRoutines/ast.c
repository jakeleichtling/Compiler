/* ast.c
 *
 * Written by THC for CS 57.  Relies on an enum and a struct for
 * various node types that appear in ast.h.  You should modify the
 * enum and struct as appropriate.
 *
 * This file contains functions to create a node and to print out an
 * abstract syntax tree, for debugging.
 * ----------------------------------------------------------------
 *
 *  Modified by Derek Salama and Jake Leichtling
 *  for use with their parser
 *  CS57
 *  4/19/2013
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

extern char *node_type_string[];
extern char *var_type_string[];

/* Define a table of token types and their associated strings.  You
   should modify this table as appropriate.  The order of entries
   should match the order of enumerated values in ast_node_type. */
struct token_lookup {
  char *token;
  ast_node_type node_type;
};
static struct token_lookup token_table[] = {
    { "ROOT", ROOT },
    { "ID", ID },
    { "INT_TYPE", INT_TYPE },
    { "DBL_TYPE", DBL_TYPE },
    { "VOID_TYPE", VOID_TYPE },
    { "ARRAY_SUB", ARRAY_SUB },
    { "ARRAY_NONSUB", ARRAY_NONSUB },
    { "OP_ASSIGN", OP_ASSIGN },
    { "OP_ADD", OP_ADD },
    { "OP_SUB", OP_SUB },
    { "OP_MULT", OP_MULT },
    { "OP_DIV", OP_DIV },
    { "OP_MOD", OP_MOD },
    { "OP_LT", OP_LT },
    { "OP_LEQ", OP_LEQ },
    { "OP_GT", OP_GT },
    { "OP_GEQ", OP_GEQ },
    { "OP_EQ", OP_EQ },
    { "OP_NEQ", OP_NEQ },
    { "OP_AND", OP_AND },
    { "OP_OR", OP_OR },
    { "OP_BANG", OP_BANG },
    { "OP_NEG", OP_NEG },
    { "OP_INC", OP_INC },
    { "OP_DEC", OP_DEC },
    { "FUNC_DECL", FUNC_DECL },
    { "VAR_DECL", VAR_DECL },
    { "FORMAL_PARAM", FORMAL_PARAM },
    { "SEQ", SEQ },
    { "IF_STMT", IF_STMT },
    { "WHILE_LOOP", WHILE_LOOP },
    { "DO_WHILE_LOOP", DO_WHILE_LOOP },
    { "FOR_STMT", FOR_STMT },
    { "RETURN_STMT", RETURN_STMT },
    { "READ_STMT", READ_STMT },
    { "PRINT_STMT", PRINT_STMT },
    { "STRING_LITERAL", STRING_LITERAL },
    { "INT_LITERAL", INT_LITERAL },
    { "DOUBLE_LITERAL", DOUBLE_LITERAL },
    { "FUNC_CALL", FUNC_CALL },
    { "EMPTY_EXPR", EMPTY_EXPR },
    { NULL,0 }
};

char *mem_addr_type_string[] = {
  "off_fp",
  "global",
  "absolute"
};

/* Create a node with a given token type and return a pointer to the
   node. */
ast_node create_ast_node(ast_node_type node_type) {
  ast_node new_node = malloc(sizeof(struct ast_node_struct));
  new_node->node_type = node_type;
  new_node->left_child = new_node->right_sibling = NULL;
  return new_node;
}

/* Print the contents of a subtree of an abstract syntax tree, given
   the root of the subtree and the depth of the subtree root. */
void print_ast(ast_node root, int depth) {
  /* Print two spaces for every level of depth. */
  int i;
  for (i = 0; i < depth; i++)
    printf("  ");

  print_ast_node(root);

  printf("\n");

  /* Recurse on each child of the subtree root, with a depth one
     greater than the root's depth. */
  ast_node child;
  for (child = root->left_child; child != NULL; child = child->right_sibling)
    print_ast(child, depth + 1);
}

void print_ast_node(ast_node node)
{
  /* Print the node type. */
  printf("%s ", token_table[node->node_type].token);

  /* Print attributes specific to node types. */
  switch (node->node_type) {
  case ID:      /* print the id */
  {
    symnode sym_node = node->value.sym_node;
    printf("%s (%p), node type: %s, var type: %s, address: %d (%s), num vars: %d",
      sym_node->name,
      sym_node,
      node_type_string[sym_node->node_type],
      var_type_string[sym_node->var_type],
      sym_node->var_addr,
      mem_addr_type_string[sym_node->mem_addr_type],
      sym_node->num_vars);
    break;
  }

  case INT_LITERAL:   /* print the int literal */
    printf("%d", node->value.int_value);
    break;

  case DOUBLE_LITERAL:    /* print the double literal */
    printf("%f", node->value.double_value);
    break;

  case STRING_LITERAL:      /* print the string */
    printf("%s (%p)", node->value.sym_node->name, node->value.sym_node);
    break;

  default:
    break;
  }

  printf("\t--> data type: %s, return type: %s", var_type_string[node->data_type], var_type_string[node->return_type]);
}

/* Iterate to last sibling in LL */
ast_node rightmost_sibling(ast_node t)
{
  for (; t->right_sibling != NULL; t = t->right_sibling); // <-- bitchin'

  return t;
}