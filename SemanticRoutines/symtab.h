/* symtab.h
 * Declarations for the symbol table.
 * Written by THC for CS 57.
 * You should extend the structs and functions as appropriate.
 * -------------------------
 * Modified by Derek Salama & Jake Leichtling
 * 4/29/2013
 *
 * Changes:
 * -Added enum "vartype" for convenient type checking on
 *  both variables and functions.
 */

#ifndef SYMTAB_H_
#define SYMTAB_H_

/* Variable and return types, used for type checking */
enum vartype {
  no_type,
  inttype,
  doubletype,
  voidtype
};

/* The thing that the identifier identifies. Not applicable in the string constant table */
enum nodetype {
  func_node,
  val_node,
  array_node
};

/* Indicates the type of the memory address */
enum memaddrtype {
  off_fp, // relative to FP for local function variables
  global, // relative to the global variable poitner for global variabls
};

/* Node in a linked list within the symbol table. */
typedef struct symnode *symnode;
struct symnode {
  char *name;	    /* name in this symnode */
  symnode next;	    /* next symnode in list */
  /* Other attributes go here. */
  enum vartype var_type; // The type of a variable or return type of a function (irrelevant for the string table)
  int var_addr; // irrelevant for string constants; assembly index for functions
  enum memaddrtype mem_addr_type;
  enum nodetype node_type; // The thing that the identifier identifies (irrelevant for the string table)
  int array_size; // only relevant for nodes of type array_node
  char *mangled_name; // irrelevant for the string table

  // For functions only:
  int num_vars; // number of variables for a function
  int num_temps; // number of temp variables for a function
  int num_params; // number of formal parameters
  symnode *param_symnode_array; // array of pointers to the formal param symnodes for type checking
};

// The symnode of the main function
symnode main_func_symnode;

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
  int num_nodes; // used for name mangling so that each mangle is unique
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

void print_symboltable(symboltable symtab);

/* Constructs an array of symnodes pointing to every symnode in the symboltable */
symnode *get_symnode_array(symboltable symtab, int *array_size);

#endif