/* ast_node_processing.h
 *
 * Declarations for filling out ID attributes and type checking.
 * fill_id_types(ast_node node) is called first on the root of the abstract
 * syntax tree, and it creates symnodes for each unique identifier and fills
 * out inherited attributes of these nodes. Next, type_check(ast_node) is called on the root
 * of the abstract syntax tree and fills out synthesized attributes of the ast_nodes
 * for the purpose of checking type consistency.
 *
 * Jake Leichtling & Derek Salama
 * 5/29/2013
 */

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