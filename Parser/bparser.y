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
extern int parseError;

extern char *savedText;
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
| RETURNTOKEN expr {
  ast_node t = create_ast_node(RETURN_STMT);
  t->left_child = $2;
  $$ = t; }
| PRINTTOKEN expr {
  ast_node t = create_ast_node(PRINT_STMT);
  t->left_child = $2;
  $$ = t;
  }
| READTOKEN {
  ast_node t = create_ast_node(READ_STMT);
  $$ = t;
  }
| IDENT '[' ']' {
  ast_node t = create_ast_node(ARRAY_NONSUBSCRIPTED);
  $$ = t;
  }
| IDENT '[' expr ']' {
  ast_node t = create_ast_node(ARRAY_SUBSCRIPTED);
  $$ = t;
  }
| expr '<' expr {
  ast_node t = create_ast_node(OP_LT);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t;
  }
| expr LEQTOKEN expr {
  ast_node t = create_ast_node(OP_LEQ);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t;
  }
| expr '>' expr {
  ast_node t = create_ast_node(OP_GT);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t;
  }
| expr GEQTOKEN expr {
  ast_node t = create_ast_node(OP_GEQ);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t;
  }
| expr EQTOKEN expr {
  ast_node t = create_ast_node(OP_EQ);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t;
  }
| expr NEQTOKEN expr {
  ast_node t = create_ast_node(OP_NEQ);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t;
  }
| expr ANDTOKEN expr {
  ast_node t = create_ast_node(OP_AND);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t;
  }
| expr ORTOKEN expr {
  ast_node t = create_ast_node(OP_OR);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t;
  }
| '!' expr {
  ast_node t = create_ast_node(OP_BANG);
  t->left_child = $2;
  $$ = t;
  }
| expr '/' expr {
  ast_node t = create_ast_node(OP_DIVIDE);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t;
  }
| expr '%' expr {
  ast_node t = create_ast_node(OP_MOD);
  t->left_child = $1;
  t->left_child->right_sibling = $3;
  $$ = t;
  }
;

%%

int yyerror(char *s) {
  parseError = 1;
  fprintf(stderr, "%s at line %d\n", s, lineNumber);
  return 0;
}
