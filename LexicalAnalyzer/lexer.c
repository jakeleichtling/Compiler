#include <stdio.h>
#include "tokens.h"
extern char *yytext;
extern int yyleng;

int main(void)
{
    int token;

    while ((token = yylex()) != EOFTOKEN) {
        printf("%s (%d): \"%s\" (%d)\n", tokenStrings[token - TOKEN_VALUE_OFFSET], token, yytext, yyleng);
    }

    return 0;
}