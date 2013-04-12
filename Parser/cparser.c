#include <stdio.h>
#include "ast.h"

ast_node root = NULL;

extern int yyparse();
extern int yydebug;
int parseError = 0;

int main()
{
    int haveRoot = 0;         /* 0 means we will have a root */

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