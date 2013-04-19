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
    ID,
    INT_TYPE, DBL_TYPE, VOID_TYPE,
    ARRAY_SUB, ARRAY_NONSUB,
    OP_ASSIGN, OP_ADD, OP_SUB, OP_MULT, OP_DIV, OP_MOD, 
    OP_LT, OP_LEQ, OP_GT, OP_GEQ, OP_EQ, OP_NEQ, OP_AND, OP_OR, 
    OP_BANG, OP_NEG,
    OP_INC, OP_DEC,  
    FUNC_DECL, VAR_DECL, FORMAL_PARAM,
    SEQ,
    IF_STMT, WHILE_LOOP, DO_WHILE_LOOP, FOR_STMT,
    RETURN_STMT,
    READ_STMT, PRINT_STMT,
    STRING_LITERAL, INT_LITERAL, DOUBLE_LITERAL,
    FUNC_CALL,
    EMPTY_EXPR
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
