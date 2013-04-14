#include <stdio.h>
#include "ast.h"
#include "sst.h"

ast_node root = NULL;

extern int yyparse();
extern int yydebug;

Sst id_table;
Sst stringconst_table;

int parseError = 0;

int main()
{
    int haveRoot = 0;         /* 0 means we will have a root */

    id_table = create_sst(-1);
    stringconst_table = create_sst(-1);

    // yydebug = 1;
    haveRoot = yyparse();

    if(parseError)
        fprintf(stderr, "WARING: There were parse errors.\nParse tree may be ill-formed.\n");

    if (haveRoot == 0)
        print_ast(root, 0);
    else
        fprintf(stderr, "%s\n", "No root :(\n");

    return 0;
}