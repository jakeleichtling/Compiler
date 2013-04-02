%{
#include <string.h>
#include "tokens.h"

int keywordLookup(char *);
%}

%%



%%

int keywordLookup(char *s)
{
    struct keywordTokenPair {
       char *keyword;
       int token;
    };
    typedef struct keywordTokenPair KeywordTokenPair;

    const int KEYWORD_COUNT = 11;

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