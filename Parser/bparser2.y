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

%token INTTOKEN DOUBLETOKEN VOIDTOKEN IDENT

program :
  declaration_list {
    $$ = $1;
  }
;

declaration_list :
  declaration_list declaration {
    ast_node t = $1;
    if (t != NULL) {
      t = rightmost_sibling(t);
      t->right_sibling = $2;
      $$ = $1;
    } else
      $$ = $2;
  }
| declaration {
    $$ = $1;
  }
;

declaration : 
  var_declaration {
    $$ = $1;
  }
| func_declaration {
    $$ = $1;
  }
;


var_declaration :
  var_type_specifier var_decl_list {
    $1->left_child = var_decl_list;
  }
;

var_type_specifier :
  INTTOKEN {
    ast_node t = create_ast_node(INT_VAR_DECL);
    $$ = t;
  }
| DOUBLETOKEN {
    ast_node t = create_ast_node(DBL_VAR_DECL);
    $$ = t;
  }
;

var_decl_list :
  var_decl_list ',' var_decl {
    ast_node t = $1;
    t = rightmost_sibling(t);
    t->right_sibling = $3;
    $$ = $1;
  }
| var_decl {
    $$ = $1;
  }
;

var_decl :
  IDENT {
    ast_node t = create_ast_node(ID);
    t->value.string = add_string(id_table, savedText);
    $$ = t;
  }
| IDENT {
    ast_node t_id = create_ast_node(ID);
    t_id->value.string = add_string(id_table, savedText);
    $1 = t_id;
  } '=' expression {
    ast_node t = create_ast_node(OP_ASSIGN);
    t->left_child = $1;
    t->left_child->right_sibling = $4;
    $$ = t;
  }
| IDENT {
    ast_node t_id = create_ast_node(ID);
    t_id->value.string = add_string(id_table, savedText);
    $1 = t_id;
  } '[' expression ']' {
    ast_node t = create_ast_node(ARRAY_SUB);
    t->left_child = $1;
    t->left_child->right_sibling = $4;
    $$ = t;
  }
;

func_declaration :
  func_type_specifier IDENT {
    ast_node t_id = create_ast_node(ID);
    t_id->value.string = add_string(id_table, savedText);
    $2 = t_id;
  } '(' formal_params ')' compound_statement {
    $1->left_child = $2;
    $1->left_child->right_sibling = $5;
    rightmost_sibling($1->left_child) = compound_statement;
    $$ = $1;
  }
;

func_type_specifier :
  INTTOKEN {
    ast_node t = create_ast_node(INT_FUNC_DECL);
    $$ = t;
  }
| DOUBLETOKEN {
    ast_node t = create_ast_node(DBL_FUNC_DECL);
    $$ = t;
  }
| VOIDTOKEN {
    ast_node t = create_ast_node(VOID_FUNC_DECL);
    $$ = t;
  }
;

formal_params :
  formal_list {
    $$ = $1;
  }
| VOIDTOKEN {
    $$ = NULL;
  }
| /* empty */ {
    $$ = NULL;
  }
;

formal_list :
  formal_list ',' formal_param {
    ast_node t = $1;
    t = rightmost_sibling(t);
    t->right_sibling = $3;
    $$ = $1;
  }
| formal_param {
    $$ = $1;
  }
;

formal_param :
  var_type_specifier IDENT {
    ast_node t_id = create_ast_node(ID);
    t_id->value.string = add_string(id_table, savedText);

    $1->left_child = t_id;
    $$ = $1;
  }
| var_type_specifier IDENT '[' ']' {
    ast_node t = create_ast_node(ARRAY_NONSUB);
    t->value.string = add_string(id_table, savedText);
    $$ = t;
  }
;

compound_statement :
  '{' local_declarations statement_list '}' {
    ast_node t = create_ast_node(SEQ);
    if ($2 != NULL) {
      t->left_child = $2;
      rightmost_sibling(t->left_child)->right_sibling = $3;
    } else
      t->left_child = $3;

    $$ = t;
  }
;

local_declarations :
  local_declarations var_declaration {
    ast_node t = $1;
    if (t != NULL) {
      t = rightmost_sibling(t);
      t->right_sibling = $2;
      $$ = $1;
    } else
      $$ = $2;
  }
| /* empty */ {
    $$ = NULL;
  }
;


// Stopped here

statement_list : statement_list statement  |  ε
statement : expression_statement  |  compound_statement  |  if_statement  |  while_statement  | 
        do_while_statement  |  for_statement  |  return_statement  |  read_statement  |  print_statement
expression_statement : expression ;  |  ;
if_statement : if ( expression ) statement  |  if ( expression ) statement else statement
Note: Resolve the ambiguity in the productions for if_statement in the usual way, by matching each else with the closest previous unmatched then_part.
while_statement : while ( expression ) statement
do_while_statement : do statement while ( expression ) ;
for_statement : for ( for_header_expression ; for_header_expression ; for_header_expression ) statement
for_header_expression : expression  |  ε
return_statement : return ;  |  return expression ;
read_statement : read var ;
print_statement : print expression ;  |  print STRING ;
expression : var = expression  |  r_value
var : ID  |  ID [ expression ]
r_value : expression + expression  |
        expression _ expression  |
        expression * expression  |
        expression / expression  |
        expression % expression  |
        expression < expression  |
        expression <= expression  |
        expression > expression  |
        expression >= expression  |
        expression == expression  |
        expression != expression  |
        expression && expression  |
        expression || expression  |
        ! expression  |
        _ expression  |
        var  |
        ++ var  |
        _ _ var  |
        ( expression )  |
        call  |
        NUM  |
        FNUM
call : ID ( args )
args : arg_list  |  ε
arg_list : arg_list , expression  |  expression