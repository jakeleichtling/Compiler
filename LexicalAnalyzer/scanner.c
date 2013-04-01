#include <stdio.h>
#include "tokens.h"
extern char *yytext;
extern int yyleng;

int main(void)
{
    int token;

    while ((token = yylex()) != EOFTOKEN) {
        printf("%s: \"%s\" (%d)", tokenStrings[token - TOKEN_VALUE_OFFSET], yytext, yyleng);
    }

    return 0;
}