/* Name: clexer.c
 *
 * Purpose: A driver for the scanner produced by running flex on the input file lexer.c. The main function repeatedly calls
 *          the yylex() function of the flex output, and then prints the result in a readable format.
 *
 * Derek Salama & Jake Leichtling
 * CS57
 * 4/10/2013
 */

#include <stdio.h>
#include "tokens.h"

extern char *yytext;
extern int yyleng;
extern int lineNumber;

int main(void)
{
    int token;

    while ((token = yylex()) != EOFTOKEN) {
        if (token <= 256)
            printf("%c (%d): \"%s\" (%d) -- %d\n", token, token, yytext, yyleng, lineNumber);
        else
            printf("%s (%d): \"%s\" (%d) -- %d\n", tokenStrings[token - TOKEN_VALUE_OFFSET], token, yytext, yyleng, lineNumber);
    }

    return 0;
}
