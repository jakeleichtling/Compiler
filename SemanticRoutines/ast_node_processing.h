#ifndef AST_NODE_PROCESSING_H_
#define AST_NODE_PROCESSING_H_

#include "ast.h"

// Performs a pre-order traversal of the syntax tree to set the types of identifiers.
//   Uses a multi-level symbol table to keep track of scoping; then ID names are mangled and saved in a flat symbol table.
int fill_id_types(ast_node node);

// Performs a post-order traversal of the syntax tree to check type compatibilities
//   throughout the program.
int type_check(ast_node node);

#endif