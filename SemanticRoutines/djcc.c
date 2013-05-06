#include <stdlib.h>
#include <stdio.h>
#include "bparser.tab.h"
#include "ast.h"
#include "symtab.h"
#include "quad.h"

ast_node root = NULL;

symboltable id_table;
symboltable stringconst_table;

int parseError = 0;

int main()
{
  int haveRoot = 0; // 0 means we have a root

  id_table = create_symboltable();
  stringconst_table = create_symboltable();

  // yydebug = 1;
  haveRoot = yyparse();

  if(parseError)
      fprintf(stderr, "WARING: There were parse errors.\nParse tree may be ill-formed.\n");

  if (haveRoot == 0) {
      fill_id_types(root);
      print_ast(root, 0);
  } else
      fprintf(stderr, "%s\n", "No root :(\n");

  return 0;
}