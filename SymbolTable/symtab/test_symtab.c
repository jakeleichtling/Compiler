/*
 * TestSymtab.cc
 * Driver to test the symbol table functions.
 * Written by THC for CS 57.
 */

#include <stdio.h>
#include "symtab.h"

int main() {
  symboltable symtab = create_symboltable();

  char command[100];
  char name[200];

  symnode x;
  int level;

  printf("Now at level 0\n");

  do {
    printf("Command: ");
    scanf("%s", command);

    switch (command[0]) {
    case 'i':
      printf("insert name: ");
      scanf("%s", name);
      insert_into_symboltable(symtab, name);
      break;

    case 'l':
      printf("lookup name: ");
      scanf("%s", name);
      x = lookup_in_symboltable(symtab, name, &level);
      if (x == NULL)
	printf("%s not found\n", name);
      else
	printf("%s found at level %d\n", name, level);
      break;

    case '{':
      enter_scope(symtab);
      printf("Now at level %d\n", symtab->inner_scope->level);
      break;

    case '}':
      if (symtab->inner_scope->level <= 0) {
	printf("Left outermost scope...bye\n");
	return 0;
      }
      else {
	leave_scope(symtab);
	printf("Now at level %d\n", symtab->inner_scope->level);
      }
      break;
      
    case 'q':
      break;
      
    default:
      printf("Valid commands are i (insert), l (lookup), "
	     "{ (enter scope), } (leave scope), q (quit)\n");
      break;
    }
  }
  while (command[0] != 'q');

  destroy_symboltable(symtab);

  return 0;
}
