/* 
 * ast.h
 *
 * File defining an enum and a struct for node types in an abstract
 * syntax tree.
 *
 * Written by THC for CS 57.  You should modify this file as
 * appropriate.
 *
 */

#ifndef AST_H_
#define AST_H_

/* You should fill in the various node types.  The following are given
   to you to start with.  You may add or remove node types as you
   wish. */
typedef enum { ROOT,
	       SEQ,
	       OP_ASSIGN, OP_PLUS, OP_MINUS, OP_NEG, OP_TIMES, OP_DIVIDE,
	       OP_EQUALS,
	       IF_STMT, IF_ELSE_STMT,
	       ID, 
	       INT_LITERAL, DOUBLE_LITERAL } ast_node_type;

/* Structure for nodes of the abstract syntax tree.  Uses the
   left-child/right-sibling representation, so that each node can have
   a variable number of children.  You should add or remove fields as
   appropriate. */
typedef struct ast_node_struct *ast_node;
struct ast_node_struct {
  ast_node_type node_type;
  ast_node left_child, right_sibling;
  union {
    char * string;		/* for ID */
    int int_value;		/* for INT_LITERAL */
    double double_value;	/* for DOUBLE_LITERAL */
  } value;
};

/* Create a node with a given token type and return a pointer to the
   node. */
ast_node create_ast_node(ast_node_type node_type);

/* Print the contents of a subtree of an abstract syntax tree, given
   the root of the subtree and the depth of the subtree root. */
void print_ast(ast_node root, int depth);

#endif
