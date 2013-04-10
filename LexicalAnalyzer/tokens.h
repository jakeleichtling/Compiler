/* Name: tokens.h
 *
 * Purpose: A header file, to be included in the compilation of the Lexical Analyzer, that defines
 *          integer values for each token (starting at 258), as well as string representations
 *          of each token.
 *
 * Derek Salama & Jake Leichtling
 * CS57
 * 4/10/2013
 */

#define TOKEN_VALUE_OFFSET 258

#define ID 258
#define NUMCONST 259
#define FNUMCONST 260
#define STRINGCONST 261
#define ILLEGALTOKEN 262
#define EOFTOKEN 263
#define ELSETOKEN 264
#define IFTOKEN 265
#define INTTOKEN 266
#define RETURNTOKEN 267
#define VOIDTOKEN 268
#define WHILETOKEN 269
#define FORTOKEN 270
#define DOTOKEN 271
#define DOUBLETOKEN 272
#define READTOKEN 273
#define PRINTTOKEN 274
#define INCREMENTTOKEN 275
#define DECREMENTTOKEN 276
#define ANDTOKEN 277
#define ORTOKEN 278
#define LEQTOKEN 279
#define GEQTOKEN 280
#define EQTOKEN 281
#define NEQTOKEN 282
#define OTHER 283

static char *tokenStrings[] =
{
    "ID",
    "NUMCONST",
    "FNUMCONST",
    "STRINGCONST",
    "ILLEGALTOKEN",
    "EOFTOKEN",
    "ELSETOKEN",
    "IFTOKEN",
    "INTTOKEN",
    "RETURNTOKEN",
    "VOIDTOKEN",
    "WHILETOKEN",
    "FORTOKEN",
    "DOTOKEN",
    "DOUBLETOKEN",
    "READTOKEN",
    "PRINTTOKEN",
    "INCREMENTTOKEN",
    "DECREMENTTOKEN",
    "ANDTOKEN",
    "ORTOKEN",
    "LEQTOKEN",
    "GEQTOKEN",
    "EQTOKEN",
    "NEQTOKEN",
    "OTHER"
};