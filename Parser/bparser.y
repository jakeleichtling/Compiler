%{
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ast.h"
#include "sst.h"

#define YYSTYPE ast_node
#define YYDEBUG 1

extern int yylex();
int yyerror(char *s);
extern char *yytext;

extern int lineNumber;
extern ast_node root;
extern int parseError;

extern char *savedText;
extern Sst id_table;
extern Sst stringconst_table;
%}


%token IDENT NUMCONST FNUMCONST STRINGCONST ELSETOKEN IFTOKEN INTTOKEN RETURNTOKEN VOIDTOKEN WHILETOKEN FORTOKEN DOTOKEN DOUBLETOKEN READTOKEN PRINTTOKEN INCREMENTTOKEN DECREMENTTOKEN ANDTOKEN ORTOKEN LEQTOKEN GEQTOKEN EQTOKEN NEQTOKEN ILLEGALTOKEN OTHER

/* from http://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B */
%nonassoc RETURNTOKEN
%right PRINTTOKEN
%right '='
%left ORTOKEN
%left ANDTOKEN
%left EQTOKEN NEQTOKEN
%left '<' LEQTOKEN '>' GEQTOKEN
%left '+' '-'
%left '*' '/' '%'
%right INCREMENTTOKEN DECREMENTTOKEN UPLUS UMINUS '!'

%expect 2 /* shift/reduce conflicts with if-else grammar */

%%

code : stmtList {
  ast_node t = create_ast_node(ROOT);
  t->left_child = $1;
  root = ($$ = t); }
;

stmtList : /* empty */ { $$ = NULL; }
| stmtList stmt {
  ast_node t = $1;
   if (t != NULL) {
     while (t->right_sibling != NULL)
       t = t->right_sibling;
     t->right_sibling = $2;
     $$ = $1;
   }
   else
     $$ = $2;
 }
;

stmt : exprStmt { $$ = $1; }
| compoundStmt  { $$ = $1; }
| ifStmt        { $$ = $1; }
| whileLoop     { $$ = $1; }
| doWhileLoop   { $$ = $1; }
| forLoop       { $$ = $1; }
;

exprStmt : expr ';' { $$ = $1; }
| ';'               { $$ = NULL; }
| error ';'         { $$ = NULL; }
| intdecl ';'       { $$ = $1; }
| doubledecl ';'    { $$ = $1; }
| functionsig compoundStmt {
  ast_node t = create_ast_node(FUNCTION_DEF);
  t->left_child = $1;
  t->left_child->right_sibling = $2;
 }
| functionsig ';' {
  ast_node t = create_ast_node(FUNCTION_PTT);
  t->left_child = $1;
 }
;

functionsig :
  rettype IDENT {
  ast_node t_id = create_ast_node(ID);
  t_id->value.string = add_string(id_table, savedText);
  $2 = t_id;
 } '(' paramlist ')' {
  ast_node t = create_ast_node(FUNCTION_SIG);
  t->left_child = $1;
  t->left_child->right_sibling = $2;
  t->left_child->right_sibling->right_sibling = $5;
  $$ = t;
}
;

rettype :
  VOIDTOKEN { $$ = create_ast_node(RET_VOID); }
| INTTOKEN { $$ = create_ast_node(RET_INT); }
| DOUBLETOKEN { $$ = create_ast_node(RET_DOUBLE); }
;

paramlist : /* empty */ { $$ = NULL; }
| paramlist ',' param {
  ast_node t = $1;
  if (t != NULL) {
    for (; t->right_sibling != NULL; t = t->right_sibling); // <-- bitchin'
    t->right_sibling = $3;
    $$ = $1;
  } else
    $$ = $3; }
;

param :
  INTTOKEN IDENT {
    ast_node t_id = create_ast_node(ID);
    t_id->value.string = add_string(id_table, savedText);

    ast_node t = create_ast_node(INT_PARAM);
    t->left_child = t_id;
    $$ = t;
  }
| DOUBLETOKEN IDENT {
    ast_node t_id = create_ast_node(ID);
    t_id->value.string = add_string(id_table, savedText);

    ast_node t = create_ast_node(DOUBLE_PARAM);
    t->left_child = t_id;
    $$ = t;
  }
| INTTOKEN IDENT '[' ']' {
    ast_node t_id = create_ast_node(ID);
    t_id->value.string = add_string(id_table, savedText);

    ast_node t = create_ast_node(INT_ARRAY_PARAM);
    t->left_child = t_id;
    $$ = t;
  }
| DOUBLETOKEN IDENT '[' ']' {
    ast_node t_id = create_ast_node(ID);
    t_id->value.string = add_string(id_table, savedText);

    ast_node t = create_ast_node(DOUBLE_ARRAY_PARAM);
    t->left_child = t_id;
    $$ = t;
  }
;

intdecl : INTTOKEN varlist  {
  ast_node t = create_ast_node(INT_DECL);
  t->left_child = $2;
  $$ = t; }
;

doubledecl : DOUBLETOKEN varlist  {
  ast_node t = create_ast_node(DOUBLE_DECL);
  t->left_child = $2;
  $$ = t; }
;

varlist : vardecl       { $$ = $1; }
| varlist ',' vardecl   {
  ast_node t = $1;
  while (t->right_sibling != NULL)
    t = t->right_sibling;
  t->right_sibling = $3;
  $$ = $1; }
;

vardecl :
IDENT {
  ast_node t1 = create_ast_node(ID);
  t1->value.string = add_string(id_table, savedText);
  $1 = t1;
 } '=' expr {
  ast_node t2 = create_ast_node(OP_ASSIGN);
  t2->left_child = $1;
  t2->left_child->right_sibling = $4;
  $$ = t2; }
| IDENT {
  ast_node t = create_ast_node(ID);
  t->value.string = add_string(id_table, savedText);
  $$ = t; }
| arraysub { $$ = $1; }

compoundStmt : '{' stmtList '}' {
  ast_node t = create_ast_node(SEQ);
  t->left_child = $2;
  $$ = t; }
;

ifStmt : IFTOKEN '(' expr ')' stmt { 
  ast_node t = create_ast_node(IF_STMT);
  t->left_child = $3;
  t->left_child->right_sibling = $5;
  $$ = t; }
| IFTOKEN '(' expr ')' stmt ELSETOKEN stmt {
  ast_node t = create_ast_node(IF_ELSE_STMT);
  t->left_child = $3;
  t->left_child->right_sibling = $5;
  t->left_child->right_sibling->right_sibling = $7;
  $$ = t; }
| IFTOKEN '(' error ')' stmt { $$ = NULL; }
| IFTOKEN '(' error ')' stmt ELSETOKEN stmt { $$ = NULL; }
;

whileLoop : WHILETOKEN '(' expr ')' stmt {
  ast_node t = create_ast_node(WHILE_LOOP);
  t->left_child = $3;
  t->left_child->right_sibling = $5; }
;

doWhileLoop : DOTOKEN stmt WHILETOKEN '(' expr ')' {
  ast_node t = create_ast_node(DO_WHILE_LOOP);
  t->left_child = $2;
  t->left_child->right_sibling = $5; }
;

forLoop : FORTOKEN '(' expr ';' expr ';' expr ')' stmt {
  ast_node t = create_ast_node(FOR_LOOP);
  t->left_child = $3;
  t->left_child->right_sibling = $5;
  t->left_child->right_sibling->right_sibling = $7;
  t->left_child->right_sibling->right_sibling->right_sibling = $9;
}

expr :
IDENT {
  ast_node t1 = create_ast_node(ID);
  t1->value.string = add_string(id_table, savedText);
  $1 = t1;
 } '=' expr {
  ast_node t2 = create_ast_node(OP_ASSIGN);
  t2->left_child = $1;
  t2->left_child->right_sibling = $4;
  $$ = t2; }
| expr '+' expr {
  ast_node t = create_ast_node(OP_PLUS);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t; }
| '+' expr %prec UPLUS { $$ = $2; }
| expr '-' expr {
  ast_node t = create_ast_node(OP_MINUS);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t; }
| '-' expr %prec UMINUS {
  ast_node t = create_ast_node(OP_NEG);
  t->left_child = $2;
  $$ = t; }
| expr '*' expr {
  ast_node t = create_ast_node(OP_TIMES);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t; }
| '(' expr ')' { $$ = $2; }
| NUMCONST {
  ast_node t = create_ast_node(INT_LITERAL);
  t->value.int_value = atoi(savedText);
  $$ = t; }
| FNUMCONST {
  ast_node t = create_ast_node(DOUBLE_LITERAL);
  t->value.double_value = atof(savedText);
  $$ = t; }
| STRINGCONST {
  ast_node t = create_ast_node(STRING_LITERAL);
  t->value.string = add_string(stringconst_table, savedText);
  $$ = t; }
| IDENT {
  ast_node t = create_ast_node(ID);
  t->value.string = add_string(id_table, savedText);
  $$ = t; }
| '(' error ')' { $$ = NULL; }
| INCREMENTTOKEN IDENT {
  ast_node t = create_ast_node(OP_INCREMENT);
  t->left_child = $2;
  $$ = t; }
| DECREMENTTOKEN IDENT {
  ast_node t = create_ast_node(OP_DECREMENT);
  t->left_child = $2;
  $$ = t; }
| RETURNTOKEN expr {
  ast_node t = create_ast_node(RETURN_STMT);
  t->left_child = $2;
  $$ = t; }
| PRINTTOKEN expr {
  ast_node t = create_ast_node(PRINT_STMT);
  t->left_child = $2;
  $$ = t; }
| READTOKEN {
  ast_node t = create_ast_node(READ_STMT);
  $$ = t; }
| arraysub { $$ = $1; }
| arraysub '=' expr {
  ast_node t = create_ast_node(OP_ASSIGN);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t; }
| expr '<' expr {
  ast_node t = create_ast_node(OP_LT);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t; }
| expr LEQTOKEN expr {
  ast_node t = create_ast_node(OP_LEQ);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t; }
| expr '>' expr {
  ast_node t = create_ast_node(OP_GT);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t; }
| expr GEQTOKEN expr {
  ast_node t = create_ast_node(OP_GEQ);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t; }
| expr EQTOKEN expr {
  ast_node t = create_ast_node(OP_EQ);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t; }
| expr NEQTOKEN expr {
  ast_node t = create_ast_node(OP_NEQ);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t; }
| expr ANDTOKEN expr {
  ast_node t = create_ast_node(OP_AND);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t; }
| expr ORTOKEN expr {
  ast_node t = create_ast_node(OP_OR);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t; }
| '!' expr {
  ast_node t = create_ast_node(OP_BANG);
  t->left_child = $2;
  $$ = t; }
| expr '/' expr {
  ast_node t = create_ast_node(OP_DIVIDE);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t; }
| expr '%' expr {
  ast_node t = create_ast_node(OP_MOD);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t; }
;

arraysub :
IDENT {
  ast_node t_id = create_ast_node(ID);
  t_id->value.string = add_string(id_table, savedText);
  $1 = t_id;
 } '[' expr ']' {
  ast_node t = create_ast_node(ARRAY_SUBSCRIPTED);
  t->left_child = $1;
  t->left_child->right_sibling = $4;
  $$ = t; }
;

%%

int yyerror(char *s) {
  parseError = 1;
  fprintf(stderr, "%s at line %d\n", s, lineNumber);
  return 0;
}
