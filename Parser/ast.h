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
typedef enum {
    ROOT,
    SEQ,
    IF_STMT, IF_ELSE_STMT,
    RETURN_STMT,
    WHILE_LOOP,
    DO_WHILE_LOOP,
    FOR_LOOP,
    READ_STMT,
    PRINT_STMT,
    OP_INCREMENT, OP_DECREMENT,
    OP_AND, OP_OR, OP_BANG,
    OP_LEQ, OP_GEQ, OP_EQ, OP_NEQ, OP_LT, OP_GT,
    OP_PLUS, OP_MINUS, OP_TIMES, OP_DIVIDE, OP_MOD, OP_NEG,
    OP_ASSIGN,
    ARRAY_SUBSCRIPTED, ARRAY_NONSUBSCRIPTED,
    INT_LITERAL, DOUBLE_LITERAL, STRING_LITERAL,
    ID,
    INT_DECL, DOUBLE_DECL,
    FUNCTION_DEF, FUNCTION_PTT,
    VOID_FUNCTION_SIG, INT_FUNCTION_SIG, DOUBLE_FUNCTION_SIG,
    INT_PARAM, DOUBLE_PARAM, INT_ARRAY_PARAM, DOUBLE_ARRAY_PARAM
} ast_node_type; 

/* Structure for nodes of the abstract syntax tree.  Uses the
   left-child/right-sibling representation, so that each node can have
   a variable number of children.  You should add or remove fields as
   appropriate. */
typedef struct ast_node_struct *ast_node;
struct ast_node_struct {
  ast_node_type node_type;
  ast_node left_child, right_sibling;
  union {
    char * string;		/* for ID or STRING_LITERAL */
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

ast_node rightmost_sibling(ast_node t);

#endif
