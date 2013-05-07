/* symtab.c
 * Functions for the symbol table.
 * Written by THC for CS 57.
 * -------------------------
 * Modified by Derek Salama & Jake Leichtling
 * 4/29/2013
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "symtab.h"

#define NOHASHSLOT -1

/*
 * Functions for symnodes.
 */

/* Create a symnode and return a pointer to it. */
static symnode create_symnode(char *name, int num_nodes) {
  symnode node = malloc(sizeof(struct symnode));
  node->name = strdup(name);
  node->next = NULL;

  // Mangle the name
  int digits_counter_num = num_nodes / 10;
  int num_digits = 1;
  while (digits_counter_num != 0) {
    num_digits++;
    digits_counter_num /= 10;
  }
  int mangled_name_len = strlen(name) + num_digits + 1;
  node->mangled_name = calloc(mangled_name_len + 1, sizeof('a'));
  snprintf(node->mangled_name, mangled_name_len + 1, "%d$%s", num_nodes, name);

  return node;
}

/* Destroy a symnode. */
static void destroy_symnode(symnode node) {
  free(node->name);
  free(node);
}

/* Set the variable type of this node. */
void set_node_vartype(symnode node, enum vartype type)
{
  node->var_type = type;
}

/* Set the address of the variable in this node. */
void set_node_addr(symnode node, int addr)
{
  node->varaddr = addr;
}

/* Does the identifier in this node equal name? */
int name_is_equal(symnode node, char *name) {
  return !strcmp(node->name, name);
}

/*
 * Functions for symhashtables.
 */

/* Create an empty symhashtable and return a pointer to it.  The
   parameter entries gives the initial size of the table. */
static symhashtable create_symhashtable(int entries) {
  symhashtable hashtable = malloc(sizeof(struct symhashtable));
  hashtable->size = entries;
  hashtable->table = calloc(entries, sizeof(struct symnode));
  int i;
  for (i = 0; i < entries; i++)
    hashtable->table[i] = NULL;
  hashtable->outer_scope = NULL;
  hashtable->level = -1;
  return hashtable;
}

/* Destroy a symhashtable. */
static void destroy_symhashtable(symhashtable hashtable) {
  int i;
  for (i = 0; i < hashtable->size; i++) {
    symnode node, next;

    for (node = hashtable->table[i]; node != NULL; node = next) {
      next = node->next;
      destroy_symnode(node);
    }
  }
}

/* Peter Weinberger's hash function, from Aho, Sethi, & Ullman
   p. 436. */
static int hashPJW(char *s, int size) {
  unsigned h = 0, g;
  char *p;

  for (p = s; *p != '\0'; p++) {
      h = (h << 4) + *p;
      if ((g = (h & 0xf0000000)) != 0)
	h ^= (g >> 24) ^ g;
    }

  return h % size;
}

/* Look up an entry in a symhashtable, returning either a pointer to
   the symnode for the entry or NULL.  slot is where to look; if slot
   == NOHASHSLOT, then apply the hash function to figure it out. */
static symnode lookup_symhashtable(symhashtable hashtable, char *name,
				   int slot) {
  symnode node;

  if (slot == NOHASHSLOT)
    slot = hashPJW(name, hashtable->size);

  for (node = hashtable->table[slot];
       node != NULL && !name_is_equal(node, name);
       node = node->next)
    ;

  return node;
}

/* Insert a new entry into a symhashtable, but only if it is not
   already present. */
static symnode insert_into_symhashtable(symhashtable hashtable, char *name, int *num_nodes_addr) {
  int slot = hashPJW(name, hashtable->size);
  symnode node = lookup_symhashtable(hashtable, name, slot);

  if (node == NULL) {
    node = create_symnode(name, *num_nodes_addr);
    (*num_nodes_addr)++;
    node->next = hashtable->table[slot];
    hashtable->table[slot] = node;
  }

  return node;
}


/*
 * Functions for symboltables.
 */

static const int HASHSIZE = 211;

/* Create an empty symbol table. */
symboltable create_symboltable() {
  symboltable symtab = malloc(sizeof(struct symboltable));
  symtab->num_nodes = 0;
  symtab->inner_scope = create_symhashtable(HASHSIZE);
  symtab->inner_scope->outer_scope = NULL;
  symtab->inner_scope->level = 0;
  return symtab;
}

/* Destroy a symbol table. */
void destroy_symboltable(symboltable symtab) {
  symhashtable table, outer;

  for (table = symtab->inner_scope; table != NULL; table = outer) {
    outer = table->outer_scope;
    destroy_symhashtable(table);
  }
}

/* Insert an entry into the innermost scope of symbol table.  First
   make sure it's not already in that scope.  Return a pointer to the
   entry. */
symnode insert_into_symboltable(symboltable symtab, char *name) {
  if (symtab->inner_scope == NULL) {
    fprintf(stderr, "Error: inserting into an empty symbol table\n");
    exit(1);
  }

  symnode node = lookup_symhashtable(symtab->inner_scope, name, NOHASHSLOT);

  if (node == NULL)
    node = insert_into_symhashtable(symtab->inner_scope, name, &(symtab->num_nodes));

  return node;
}

/* Lookup an entry in a symbol table.  If found return a pointer to it
   and fill in level.  Otherwise, return NULL and level is
   undefined. */
symnode lookup_in_symboltable(symboltable symtab, char *name, int *level) {
  symnode node;
  symhashtable hashtable;

  for (node = NULL, hashtable = symtab->inner_scope;
       node == NULL && hashtable != NULL;
       hashtable = hashtable->outer_scope) {
    node = lookup_symhashtable(hashtable, name, NOHASHSLOT);
    *level = hashtable->level;
  }

  return node;
}

/* Enter a new scope. */
void enter_scope(symboltable symtab) {
  symhashtable hashtable = create_symhashtable(HASHSIZE);
  hashtable->outer_scope = symtab->inner_scope;
  hashtable->level = symtab->inner_scope->level + 1;
  symtab->inner_scope = hashtable;
}

/* Leave a scope. */
void leave_scope(symboltable symtab) {
  symhashtable hashtable = symtab->inner_scope;
  symtab->inner_scope = hashtable->outer_scope;
  destroy_symhashtable(hashtable);
}
