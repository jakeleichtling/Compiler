%{
#include <string.h>
#include "tokens.h"

int keywordLookup(char *);
%}

white [ \t\n]+

letter [a-zA-Z]
digit [0-9]
id ({letter}|_)({letter}|{digit}|_)*
integer {digit}+
signedint [-+]?{integer}
fixedfloat {signedint}"."{integer}?|[-+]?"."{integer}

%x string
%x comment

%%

{white}         /* skip whitespace */

{id}            return keywordLookup(yytext);
{signedint}     return NUMCONST;
{fixedfloat}    return FNUMCONST;

\"              BEGIN(string);
<string>\n      return ILLEGALTOKEN;
<string>\\\"    yymore();
<string>\"      {
    BEGIN(INITIAL);
    yytext[--yyleng] = '\0';
    return STRINGCONST;
}
<string>.       yymore();

%%

int keywordLookup(char *s)
{
    struct keywordTokenPair {
       char *keyword;
       int token;
    };
    typedef struct keywordTokenPair KeywordTokenPair;

    const int KEYWORD_COUNT = 12;

    static KeywordTokenPair keywordTokenPairs[] =
        {
            { "else", ELSETOKEN },
            { "if", IFTOKEN },
            { "int", INTTOKEN },
            { "return", RETURNTOKEN },
            { "void", VOIDTOKEN },
            { "while", WHILETOKEN },
            { "for", FORTOKEN },
            { "do", DOTOKEN },
            { "double", DOUBLETOKEN },
            { "read", READTOKEN },
            { "print", PRINTTOKEN },
            {  NULL, ID }
        };

    KeywordTokenPair *keywordTokenPair;

    keywordTokenPairs[KEYWORD_COUNT - 1].keyword = s;

    for (keywordTokenPair = keywordTokenPairs; strcmp(keywordTokenPair->keyword, s) != 0; keywordTokenPair++);

    return keywordTokenPair->token;
}