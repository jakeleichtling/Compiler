/* flexer.l
 *
 * A flex input file that specifies the rules for scanning a C57 input program. Contains both rules for pattern matching
 * and a function for looking up keywords and distinguishing them from identifiers.
 *
 * Derek Salama & Jake Leichtling
 * 5/29/2013
 */

%{
#include <string.h>
#include "bparser.tab.h"

int lineNumber = 1;

int keywordLookup(char *);
char *savedText;
%}

white [ \t]+

letter [a-zA-Z]
digit [0-9]
id ({letter}|_)({letter}|{digit}|_)*
integer {digit}+
float {integer}"."{integer}?|"."{integer}

%x string
%x comment

%%

{white}         /* skip whitespace, but we need to count line numbers */
\n              lineNumber++;

{id}            {
    savedText = strdup(yytext);
    return keywordLookup(yytext);
}
{integer}     {
    savedText = strdup(yytext);
    return NUMCONST;
}
{float}    {
    savedText = strdup(yytext);
    return FNUMCONST;
}

\"              BEGIN(string);
<string>\n      {
    savedText = strdup(yytext);
    return ILLEGALSTRINGTOKEN;
    lineNumber++;
}
<string>\\\"    yymore();
<string>\"      {
    BEGIN(INITIAL);
    yytext[--yyleng] = '\0';
    savedText = strdup(yytext);
    return STRINGCONST;
}
<string>.       yymore();

\/\*            BEGIN(comment);
<comment>\*\/   BEGIN(INITIAL);
<comment>.      /* skip comment text */
<comment>\n     lineNumber++; /* eat up multiline comments */

\+              return '+';
-               return '-';
\*              return '*';
\/              return '/';
%               return '%';
!               return '!';
\<              return '<';
>               return '>';
=               return '=';
;               return ';';
,               return ',';
\(              return '(';
\)              return ')';
\[              return '[';
]               return ']';
\{              return '{';
\}              return '}';

\+\+            return INCREMENTTOKEN;
--              return DECREMENTTOKEN;
&&              return ANDTOKEN;
\|\|            return ORTOKEN;
\<=             return LEQTOKEN;
>=              return GEQTOKEN;
==              return EQTOKEN;
!=              return NEQTOKEN;
.               return OTHER;
<<EOF>>         return 0;

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
            {  NULL, IDENT }
        };

    KeywordTokenPair *keywordTokenPair;

    keywordTokenPairs[KEYWORD_COUNT - 1].keyword = s;

    for (keywordTokenPair = keywordTokenPairs; strcmp(keywordTokenPair->keyword, s) != 0; keywordTokenPair++);

    return keywordTokenPair->token;
}
