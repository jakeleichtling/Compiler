%{
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ast.h"

#define YYSTYPE ast_node
#define YYDEBUG 1

extern int yylex();
int yyerror(char *s);
extern char *yytext;

extern int lineNumber;
extern ast_node root;
//extern int parseError;

extern char *savedText;
%}


%token IDENT NUMCONST FNUMCONST STRINGCONST ELSETOKEN IFTOKEN INTTOKEN RETURNTOKEN VOIDTOKEN WHILETOKEN FORTOKEN DOTOKEN DOUBLETOKEN READTOKEN PRINTTOKEN INCREMENTTOKEN DECREMENTTOKEN ANDTOKEN ORTOKEN LEQTOKEN GEQTOKEN EQTOKEN NEQTOKEN ILLEGALTOKEN OTHER EOFTOKEN

/* TODO */
%right '='
%left '+' '-'
%left '*'
%left UMINUS

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
;

exprStmt : expr ';' { $$ = $1; }
| ';'               { $$ = NULL; }
| error ';'         { $$ = NULL; }
;

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


expr :
IDENT {
  ast_node t1 = create_ast_node(ID);
  t1->value.string = strdup(savedText);
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
| expr '-' expr {
  ast_node t = create_ast_node(OP_MINUS);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t; }
| expr '*' expr {
  ast_node t = create_ast_node(OP_TIMES);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t; }
| '-' expr %prec UMINUS {
  ast_node t = create_ast_node(OP_NEG);
  t->left_child = $2;
  $$ = t; }
| '(' expr ')' { $$ = $2; }
| NUMCONST {
  ast_node t = create_ast_node(INT_LITERAL);
  t->value.int_value = atoi(savedText);
  $$ = t; }
| IDENT {
  ast_node t = create_ast_node(ID);
  t->value.string = strdup(savedText);
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
;

%%

int yyerror(char *s) {
//  parseError = 1;
  fprintf(stderr, "%s at line %d\n", s, lineNumber);
  return 0;
}
