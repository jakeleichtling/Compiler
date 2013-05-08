#include <stdlib.h>
#include <stdio.h>
#include "bparser.tab.h"
#include "ast.h"
#include "symtab.h"
#include "quad.h"

#define MAX_NUM_QUADS 4096

int next_quad_index = 0;
int quad_array_size = MAX_NUM_QUADS;
quad *quad_array;

symboltable scoped_id_table; // Used to look up unmangled IDs with scoping
symboltable flat_id_table; // Used to store IDs with mangled names
symboltable stringconst_table;

ast_node root = NULL;
int parseError = 0;

int jldebug = 1;

int main()
{
  int haveRoot = 0; // 0 means we have a root

  quad_array = calloc(quad_array_size, sizeof(quad));

  scoped_id_table = create_symboltable();
  flat_id_table = create_symboltable();
  stringconst_table = create_symboltable();

  // yydebug = 1;
  haveRoot = yyparse();

  if(parseError)
      fprintf(stderr, "WARING: There were parse errors.\nParse tree may be ill-formed.\n");

  if (haveRoot == 0) {
      fill_id_types(root);
      generate_intermediate_code(root);
      printf("~~~~~~~~~~~~~~~~~~~~~~~\n");
      print_ast(root, 0);
      print_quad_array();
  } else {
      fprintf(stderr, "%s\n", "No root :(\n");
  }

  return 0;
}