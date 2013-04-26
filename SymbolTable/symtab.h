/* symtab.h
 * Declarations for the symbol table.
 * Written by THC for CS 57.
 * You should extend the structs and functions as appropriate.
 */

#ifndef SYMTAB_H_
#define SYMTAB_H_

/* Node types */
enum nodetype {
  variable,
  function,
  stringconst
};

/* Variable types */
enum vartype {
  inttype,
  doubletype
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

/* Set the name in this node. */
void set_node_name(symnode node, char *name);

/* Set the node type of this node. */
void set_node_nodetype(symnode node, enum nodetype type);

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
symnode insert_into_symboltable(symboltable symtab, char *name);

/* Lookup an entry in a symbol table.  If found return a pointer to it
   and fill in level.  Otherwise, return NULL and level is
   undefined. */
symnode lookup_in_symboltable(symboltable symtab, char *name, int *level);

/* Enter a new scope. */
void enter_scope(symboltable symtab);

/* Leave a scope. */
void leave_scope(symboltable symtab);

#endif
