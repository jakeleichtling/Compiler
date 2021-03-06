/* symtab.h
 * Declarations for the symbol table.
 * Written by THC for CS 57.
 * You should extend the structs and functions as appropriate.
 * -------------------------
 * Modified by Derek Salama & Jake Leichtling
 * 4/29/2013
 *
 * Changes:
 * -Added enum "nodetype" in symnode to distinguish between 
 *  nodes for string constants and identifiers for variables and functions
 * -Added enum "vartype" for convenient type checking on
 *  both variables and functions. Please note that this
 *  is irrelevant for string constant nodes and also does
 *  not effect "shadowing", as equality is only defined by
 *  name and nodetype.
 */

#ifndef SYMTAB_H_
#define SYMTAB_H_

/* Node types */
enum nodetype {
  identifier,
  stringconst
};

/* Variable and return types */
enum vartype {
  inttype,
  doubletype,
  voidtype
};

/* Node in a linked list within the symbol table. */

typedef struct symnode *symnode;
struct symnode {
  char *name;	    /* name in this symnode */
  symnode next;	    /* next symnode in list */
  /* Other attributes go here. */
  enum nodetype node_type;
  enum vartype var_type; // The type of a variable or return type of a function (irrelevant for string constants)
  int varaddr; // irrelevant for string constants
};

/* Set the variable type of this node. */
void set_node_vartype(symnode node, enum vartype type);

/* Set the address of the variable in this node. */
void set_node_addr(symnode node, int addr);

/* Does the identifier in this node equal name? */
int name_is_equal(symnode node, char *name);

/* Hash table for a given scope in a symbol table. */

typedef struct symhashtable *symhashtable;
struct symhashtable {
  int size;			/* size of hash table */
  symnode *table;		/* hash table */
  symhashtable outer_scope;	/* ptr to symhashtable in next outer scope */
  int level;			/* level of scope, 0 is outermost */
};

/* Symbol table for all levels of scope. */

typedef struct symboltable *symboltable;
struct symboltable {
  symhashtable inner_scope;	/* pointer to symhashtable of innermost scope */
};

/* Create an empty symbol table. */
symboltable create_symboltable();

/* Destroy a symbol table. */
void destroy_symboltable(symboltable symtab);

/* Insert an entry into the innermost scope of symbol table.  First
   make sure it's not already in that scope.  Return a pointer to the
   entry. */
symnode insert_into_symboltable(symboltable symtab, char *name, enum nodetype node_type);

/* Lookup an entry in a symbol table.  If found return a pointer to it
   and fill in level.  Otherwise, return NULL and level is
   undefined. */
symnode lookup_in_symboltable(symboltable symtab, char *name, int *level, enum nodetype node_type);

/* Enter a new scope. */
void enter_scope(symboltable symtab);

/* Leave a scope. */
void leave_scope(symboltable symtab);

#endif
