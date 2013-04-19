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

%token INTTOKEN DOUBLETOKEN VOIDTOKEN IDENT FNUMCONST NUMCONST READTOKEN PRINTTOKEN INCREMENTTOKEN DECREMENTTOKEN IFTOKEN ELSETOKEN FORTOKEN DOTOKEN WHILETOKEN STRINGCONST LEQTOKEN GEQTOKEN EQTOKEN NEQTOKEN ANDTOKEN ORTOKEN RETURNTOKEN ILLEGALTOKEN OTHER

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
%right INCREMENTTOKEN DECREMENTTOKEN '!'
%left UPLUS UMINUS

%expect 1

%%

program :
  declaration_list {
    ast_node t_root = create_ast_node(ROOT);
    t_root->left_child = $1;
    root = ($$ = t_root);
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
  type_specifier var_decl_list ';' {
    ast_node t = create_ast_node(VAR_DECL);
    t->left_child = $1;
    t->left_child->right_sibling = $2;
    $$ = t;
  }
;

type_specifier :
  INTTOKEN {
    ast_node t = create_ast_node(INT_TYPE);
    $$ = t;
  }
| DOUBLETOKEN {
    ast_node t = create_ast_node(DBL_TYPE);
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

ident : IDENT {
    ast_node t_id = create_ast_node(ID);
    t_id->value.string = add_string(id_table, savedText);
    $$ = t_id;
  }
;

var_decl :
  ident {
    $$ = $1
  }
| ident '=' expression {
    ast_node t = create_ast_node(OP_ASSIGN);
    t->left_child = $1;
    t->left_child->right_sibling = $3;
    $$ = t;
  }
| ident '[' expression ']' {
    ast_node t = create_ast_node(ARRAY_SUB);
    t->left_child = $1;
    t->left_child->right_sibling = $3;
    $$ = t;
  }
;

func_declaration :
  type_specifier ident '(' formal_params ')' compound_statement {
    ast_node t = create_ast_node(FUNC_DECL);
    t->left_child = $1;
    t->left_child->right_sibling = $2;
    t->left_child->right_sibling->right_sibling = $4;
    rightmost_sibling(t->left_child)->right_sibling = $6;
    $$ = t;
  }
| VOIDTOKEN ident '(' formal_params ')' compound_statement {
    ast_node t = create_ast_node(FUNC_DECL);
    t->left_child = create_ast_node(VOID_TYPE);
    t->left_child->right_sibling = $2;
    t->left_child->right_sibling->right_sibling = $4;
    rightmost_sibling(t->left_child)->right_sibling = $6;
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
  type_specifier ident {
    ast_node t = create_ast_node(FORMAL_PARAM);
    t->left_child = $1;
    t->left_child->right_sibling = $2;
    $$ = t;
  }
| type_specifier ident '[' ']' {
    ast_node t = create_ast_node(ARRAY_NONSUB);
    t->left_child = $1;
    t->left_child->right_sibling = $2;
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

statement_list : 
  statement_list statement {
    ast_node t = $1;
    if (t != NULL) {
	t = rightmost_sibling(t);
	t->right_sibling = $2;
	$$ = $1;
    } else
	$$ = $2;
  }
|  /* empty */ {
    $$ = NULL;
  }
;

statement : 
  expression_statement { $$ = $1; }
 |  compound_statement { $$ = $1; }
 |  if_statement       { $$ = $1; }
 |  while_statement    { $$ = $1; } 
 |  do_while_statement { $$ = $1; }
 |  for_statement      { $$ = $1; }
 |  return_statement   { $$ = $1; }
 |  read_statement     { $$ = $1; }
 |  print_statement    { $$ = $1; }
;
expression_statement : 
  expression ';'  { $$ = $1; }
 |  ';'           { $$ = NULL; };
;

// Note: Resolve the ambiguity in the productions for if_statement in the usual way, by matching each else with the closest previous unmatched then_part.
if_statement : 
   IFTOKEN '('expression')' statement {
	ast_node t = create_ast_node(IF_STMT);
	t->left_child = $3;
	t->left_child->right_sibling = $5;
	$$ = t;
  }
 | IFTOKEN '(' expression ')' statement ELSETOKEN statement {
	ast_node t = create_ast_node(IF_STMT);
	t->left_child = $3;
	t->left_child->right_sibling = $5;
	t->left_child->right_sibling->right_sibling = $7;
	$$ = t;
  }
;

while_statement : 
  WHILETOKEN '(' expression ')' statement {
	ast_node t = create_ast_node(WHILE_LOOP);
	t->left_child = $3;
	t->left_child->right_sibling = $5;
	$$ = t;
  }
;

do_while_statement : 
  DOTOKEN statement WHILETOKEN '(' expression ')' ';' {
	ast_node t = create_ast_node(DO_WHILE_LOOP);
	t->left_child = $2;
	t->left_child->right_sibling = $5;
	$$ = t;
  }
;

for_statement : 
  FORTOKEN '(' for_header_expression ';' for_header_expression ';' for_header_expression ')' statement {
	ast_node t = create_ast_node(FOR_STMT);
	t->left_child = $3;
	t->left_child->right_sibling = $5;
	t->left_child->right_sibling->right_sibling = $7;
	t->left_child->right_sibling->right_sibling->right_sibling = $9;
	$$ = t;
  }
;

for_header_expression : expression  {
	$$ = $1;
  }
| /* empty */ {
	$$ = create_ast_node(EMPTY_EXPR);
  } 
;

return_statement : 
  RETURNTOKEN ';' {
	ast_node t = create_ast_node(RETURN_STMT);
	$$ = t;
  } 
|  RETURNTOKEN expression ';' {
	ast_node t = create_ast_node(RETURN_STMT);
	t->left_child = $2;
	$$ = t;
  }
;

read_statement : 
  READTOKEN var ';' {
	ast_node t = create_ast_node(READ_STMT);	
	t->left_child = $2;
	$$ = t;
  }
;

print_statement : 
  PRINTTOKEN expression ';' {
	ast_node t = create_ast_node(PRINT_STMT);	
	t->left_child = $2;
	$$ = t;
  } 
| PRINTTOKEN STRINGCONST ';' { // check this
	ast_node t = create_ast_node(PRINT_STMT);	
	ast_node t_str = create_ast_node(STRING_LITERAL);
	t_str->value.string = add_string(stringconst_table, savedText);
	t->left_child = t_str;
	$$ = t;
  }
;

expression : 
  var '=' expression {
	ast_node t = create_ast_node(OP_ASSIGN);
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;	
  }
| r_value {
	$$ = $1;
  }
;
var : 
  ident {
	$$ = $1;
  }
| ident '[' expression ']' {
	ast_node t = create_ast_node(ARRAY_SUB);
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
;
r_value : 
  expression '+' expression {
	ast_node t = create_ast_node(OP_ADD);	
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }  
| expression '-' expression  {
	ast_node t = create_ast_node(OP_SUB);	
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression '*' expression  {
	ast_node t = create_ast_node(OP_MULT);	
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression '/'  expression  {
	ast_node t = create_ast_node(OP_DIV);	
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression '%' expression  {
	ast_node t = create_ast_node(OP_MOD);	
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression '<' expression  {
	ast_node t = create_ast_node(OP_LT);	
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression LEQTOKEN expression {
	ast_node t = create_ast_node(OP_LEQ);	
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression '>' expression  {
	ast_node t = create_ast_node(OP_GT);	
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression GEQTOKEN expression {
	ast_node t = create_ast_node(OP_GEQ);	
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression EQTOKEN expression {
	ast_node t = create_ast_node(OP_EQ);	
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression NEQTOKEN expression {
	ast_node t = create_ast_node(OP_NEQ);	
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression ANDTOKEN expression {
	ast_node t = create_ast_node(OP_AND);	
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression ORTOKEN expression {
	ast_node t = create_ast_node(OP_OR);	
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| '!' expression {
	ast_node t = create_ast_node(OP_BANG);	
	t->left_child = $2;
	$$ = t;
  }
| '-' expression %prec UMINUS {
	ast_node t = create_ast_node(OP_NEG);	
	t->left_child = $2;
	$$ = t;
  }
| '+' expression %prec UPLUS {
    $$ = $2;
  }
| var {
	$$ = $1;
  }
| INCREMENTTOKEN var {
	ast_node t = create_ast_node(OP_INC);	
	t->left_child = $2;
	$$ = t;
  }
| DECREMENTTOKEN var {
	ast_node t = create_ast_node(OP_DEC);	
	t->left_child = $2;
	$$ = t;
  }
| '(' expression ')' {
	$$ = $2;
  }
| call {
	$$ = $1;
  }
| NUMCONST {
	ast_node t = create_ast_node(INT_LITERAL);
	t->value.int_value = atoi(savedText);
	$$ = t;
  }
| FNUMCONST {
	ast_node t = create_ast_node(DOUBLE_LITERAL);
	t->value.double_value = atoi(savedText);
	$$ = t;
  }
;

call : 
  ident '(' args ')' {
	ast_node t = create_ast_node(FUNC_CALL);
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;	
  }
;

args : 
  arg_list {
	$$ = $1;
  }
| /* empty */ {
	$$ = NULL;
  }
;

arg_list : arg_list ',' expression {
	ast_node t = $1;
    t = rightmost_sibling(t);
	t->right_sibling = $3;
	$$ = $1;
  }
|  expression {
	$$ = $1;
  }
;

%%

int yyerror(char *s) {
  parseError = 1;
  fprintf(stderr, "%s at line %d: %s\n", s, lineNumber, savedText);
  return 0;
}
