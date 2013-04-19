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


statement_list : 
  statement_list statement {
    ast_node t = $1;
    if (t != null) {
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
  expression_statement {$$ = $1; }
 |  compound_statement {$$ = $1; }
 |  if_statement       {$$ = $1; }
 |  while_statement    {$$ = $1; } 
 |  do_while_statement {$$ = $1; }
 |  for_statement      {$$ = $1; }
 |  return_statement   {$$ = $1; }
 |  read_statement     {$$ = $1; }
 |  print_statement    {$$ = $1; }
;
expression_statement : 
  expression ';'  {$$ = $1; }
 |  ';'           {$$ = NULL; };
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
	$$ = NULL;
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
|  r_value {
	$$ = $1;
  }
;
var : 
  IDENT {
	ast_node t = create_ast_node(ID);
	t->value.string = add_string(id_table, savedText);	
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
| '-'expression %prec UMINUS {
	ast_node t = create_ast_node(OP_NEG);	
	t->left_child = $2;
	$$ = t;
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
  IDENT  {
	ast_node t_id = create_ast_node(ID);
	t_id->value.string = add_string(id_table, savedText);
	$1 = t_id;
  }'(' args ')' {
	ast_node t = create_ast_node(FTN_CALL);
	t->left_child = $1;
	t->left_child->right_sibling = $4;
	$$ = t;	
  }
;
args : 
  arg_list  {
	$$ = $1;
  }
| /* empty */ {
	$$ = NULL;
  }
;

/*
 * TODO: we should never get an empty arg_list followed
 * by a comma
 */
arg_list : arg_list ',' expression {
	ast_node t = $1;
	if (t != NULL) {
		t = rightmost_sibling(t);
		t->right_sibling = $2;
		$$ = $1;
	} else
		$$ = $2;	 
  }
|  expression {
	$$ = $1;
  }
;
