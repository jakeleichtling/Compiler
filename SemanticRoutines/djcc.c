#include <stdlib.h>
#include <stdio.h>
#include "bparser.tab.h"
#include "ast.h"
#include "symtab.h"
#include "quad.h"
#include "ast_node_processing.h"
#include "tm57_assembly_generation.h"

extern quad *quad_array;

symboltable scoped_id_table; // Used to look up unmangled IDs with scoping
symboltable flat_id_table; // Used to store IDs with mangled names
symboltable stringconst_table; // Used throughout to save memory with string constants
symboltable id_name_table; // Used by the parser to save memory with ID names

ast_node root = NULL;
int parseError = 0;

int error_count = 0;

int djdebug = 1;

// Will be provided at the command line, or defaults
char *assembly_file_name = "assembly.tm57";

int yyparse();

int main()
{
  int haveRoot = 0; // 0 means we have a root

  init_quad_array(-1);

  scoped_id_table = create_symboltable();
  flat_id_table = create_symboltable();
  stringconst_table = create_symboltable();
  id_name_table = create_symboltable();

  // yydebug = 1;
  haveRoot = yyparse();

  if (parseError) {
      fprintf(stderr, "WARING: There were parse errors.\nParse tree may be ill-formed.\n");
  } else if (haveRoot == 0) {
      if (djdebug) {
        printf("\n~~~~~~~~~~~~~~ fill_id_types ~~~~~~~~~~\n");
      }
      fill_id_types(root);

      if (djdebug) {
        printf("\n~~~~~~~~~~~ type_check ~~~~~~~~~~~~\n");
      }
      type_check(root);

      if (djdebug) {
        printf("\n~~~~~~~~~~~~ generate_intermediate_code ~~~~~~~~~~~\n");
      }
      generate_intermediate_code(root);
      
      if (djdebug) {
        printf("\n~~~~~~~~~~~ print_ast ~~~~~~~~~~~~\n");
        print_ast(root, 0);
      }

      if (djdebug) {
        printf("\n~~~~~~~~~~~ print_quad_array ~~~~~~~~~~~~\n");
        print_quad_array();
      }

  } else {
      fprintf(stderr, "%s\n", "No root :(\n");
  }

  return 0;
}