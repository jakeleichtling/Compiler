/*
 * bparser.y
 *
 * Bison file for C57 Parser
 * -generates parse tree using yyless()
 * -node type: ast_node
 * 
 * Derek Salama & Jake Leichtling
 * CS57
 * 4/19/2013
 */
%{
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ast.h"
#include "symtab.h"

#define YYSTYPE ast_node
#define YYDEBUG 1

extern int yylex();
int yyerror(char *s);
extern char *yytext;

extern int lineNumber;
extern ast_node root;
extern int parseError;

extern char *savedText;
extern symboltable id_table;
extern symboltable stringconst_table;

char *error_string;
char *prefix = "";
%}

/* lexer tokens */
%token INTTOKEN DOUBLETOKEN VOIDTOKEN IDENT FNUMCONST NUMCONST READTOKEN PRINTTOKEN INCREMENTTOKEN DECREMENTTOKEN IFTOKEN ELSETOKEN FORTOKEN DOTOKEN WHILETOKEN STRINGCONST LEQTOKEN GEQTOKEN EQTOKEN NEQTOKEN ANDTOKEN ORTOKEN RETURNTOKEN ILLEGALSTRINGTOKEN OTHER

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

%start program

/* if-else conflict */
%expect 1

%%

/* Grammar as follows closely mimicks official C57 grammar as documented on course website */
program :
  declaration_list {
    ast_node t_root = create_ast_node(ROOT);
    t_root->line_num = lineNumber;
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
    t->line_num = lineNumber;
    t->left_child = $1;
    t->left_child->right_sibling = $2;
    $$ = t;
  }
;

type_specifier :
  INTTOKEN {
    ast_node t = create_ast_node(INT_TYPE);
    t->line_num = lineNumber;
    $$ = t;
  }
| DOUBLETOKEN {
    ast_node t = create_ast_node(DBL_TYPE);
    t->line_num = lineNumber;
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
    t_id->line_num = lineNumber;
    t_id->value.sym_node = insert_into_symboltable(id_table, savedText);
    $$ = t_id;
  }
;

var_decl :
  ident {
    $$ = $1;
    $$->value.sym_node->node_type = val_node;
  }
| ident '=' expression {
    ast_node t = create_ast_node(OP_ASSIGN);
    t->line_num = lineNumber;
    t->left_child = $1;
    t->left_child->value.sym_node->node_type = val_node;
    t->left_child->right_sibling = $3;
    $$ = t;
  }
| ident '[' expression ']' {
    ast_node t = create_ast_node(ARRAY_SUB);
    t->line_num = lineNumber;
    t->left_child = $1;
    t->left_child->value.sym_node->node_type = array_node;
    t->left_child->right_sibling = $3;
    $$ = t;
  }
;

func_declaration :
  type_specifier ident '(' formal_params ')' {
    prefix = strdup($2->value.sym_node->name);

    ast_node t = create_ast_node(FUNC_DECL);
    t->line_num = lineNumber;
    t->left_child = $1;
    t->left_child->right_sibling = $2;
    t->left_child->right_sibling->value.sym_node->node_type = func_node;
    t->left_child->right_sibling->right_sibling = $4;
    $$ = t;
  } compound_statement {
    rightmost_sibling($$->left_child)->right_sibling = $7;
    prefix = "";
  }
| VOIDTOKEN ident '(' formal_params ')' compound_statement {
    ast_node t = create_ast_node(FUNC_DECL);
    t->line_num = lineNumber;
    t->left_child = create_ast_node(VOID_TYPE);
    t->left_child->line_num = lineNumber;
    t->left_child->right_sibling = $2;
    t->left_child->right_sibling->value.sym_node->node_type = func_node;
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
    t->line_num = lineNumber;
    t->left_child = $1;
    t->left_child->right_sibling = $2;
    t->left_child->right_sibling->value.sym_node->node_type = val_node;
    $$ = t;
  }
| type_specifier ident '[' ']' {
    ast_node t = create_ast_node(ARRAY_NONSUB);
    t->line_num = lineNumber;
    t->left_child = $1;
    t->left_child->right_sibling = $2;
    t->left_child->right_sibling->value.sym_node->node_type = array_node;
    $$ = t;
  }
;

compound_statement :
  '{' local_declarations statement_list '}' {
    ast_node t = create_ast_node(SEQ);
    t->line_num = lineNumber;
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
  t->line_num = lineNumber;
	t->left_child = $3;
	t->left_child->right_sibling = $5;
	$$ = t;
  }
 | IFTOKEN '(' expression ')' statement ELSETOKEN statement {
	ast_node t = create_ast_node(IF_STMT);
  t->line_num = lineNumber;
	t->left_child = $3;
	t->left_child->right_sibling = $5;
	t->left_child->right_sibling->right_sibling = $7;
	$$ = t;
  }
;

while_statement : 
  WHILETOKEN '(' expression ')' statement {
	ast_node t = create_ast_node(WHILE_LOOP);
  t->line_num = lineNumber;
	t->left_child = $3;
	t->left_child->right_sibling = $5;
	$$ = t;
  }
;

do_while_statement : 
  DOTOKEN statement WHILETOKEN '(' expression ')' ';' {
	ast_node t = create_ast_node(DO_WHILE_LOOP);
  t->line_num = lineNumber;
	t->left_child = $2;
	t->left_child->right_sibling = $5;
	$$ = t;
  }
;

for_statement : 
  FORTOKEN '(' for_header_expression ';' for_header_expression ';' for_header_expression ')' statement {
	ast_node t = create_ast_node(FOR_STMT);
  t->line_num = lineNumber;
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
  $$->line_num = lineNumber;
  } 
;

return_statement : 
  RETURNTOKEN ';' {
	ast_node t = create_ast_node(RETURN_STMT);
  t->line_num = lineNumber;
	$$ = t;
  } 
|  RETURNTOKEN expression ';' {
	ast_node t = create_ast_node(RETURN_STMT);
  t->line_num = lineNumber;
	t->left_child = $2;
	$$ = t;
  }
;

read_statement : 
  READTOKEN var ';' {
	ast_node t = create_ast_node(READ_STMT);
  t->line_num = lineNumber;
	t->left_child = $2;
	$$ = t;
  }
;

print_statement : 
  PRINTTOKEN expression ';' {
	ast_node t = create_ast_node(PRINT_STMT);
  t->line_num = lineNumber;
	t->left_child = $2;
	$$ = t;
  } 
| PRINTTOKEN STRINGCONST ';' { 
	ast_node t = create_ast_node(PRINT_STMT);
  t->line_num = lineNumber;
	ast_node t_str = create_ast_node(STRING_LITERAL);
  t_str->line_num = lineNumber;
	t_str->value.sym_node = insert_into_symboltable(stringconst_table, savedText);
	t->left_child = t_str;
	$$ = t;
  }
/* ERROR HANDLING for malformed strings */
| PRINTTOKEN ILLEGALSTRINGTOKEN {
    error_string = "Malformed string: Newlines not allowed. Use \\n instead?";
    yyerror("syntax error");
  }
;

expression : 
  var '=' expression {
	ast_node t = create_ast_node(OP_ASSIGN);
  t->line_num = lineNumber;
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
  t->line_num = lineNumber;
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
;
r_value : 
  expression '+' expression {
	ast_node t = create_ast_node(OP_ADD);
  t->line_num = lineNumber;
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }  
| expression '-' expression  {
	ast_node t = create_ast_node(OP_SUB);
  t->line_num = lineNumber;
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression '*' expression  {
	ast_node t = create_ast_node(OP_MULT);
  t->line_num = lineNumber;
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression '/'  expression  {
	ast_node t = create_ast_node(OP_DIV);
  t->line_num = lineNumber;
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression '%' expression  {
	ast_node t = create_ast_node(OP_MOD);
  t->line_num = lineNumber;
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression '<' expression  {
	ast_node t = create_ast_node(OP_LT);
  t->line_num = lineNumber;
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression LEQTOKEN expression {
	ast_node t = create_ast_node(OP_LEQ);
  t->line_num = lineNumber;
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression '>' expression  {
	ast_node t = create_ast_node(OP_GT);
  t->line_num = lineNumber;
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression GEQTOKEN expression {
	ast_node t = create_ast_node(OP_GEQ);
  t->line_num = lineNumber;
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression EQTOKEN expression {
	ast_node t = create_ast_node(OP_EQ);
  t->line_num = lineNumber;
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression NEQTOKEN expression {
	ast_node t = create_ast_node(OP_NEQ);
  t->line_num = lineNumber;
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression ANDTOKEN expression {
	ast_node t = create_ast_node(OP_AND);
  t->line_num = lineNumber;
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| expression ORTOKEN expression {
	ast_node t = create_ast_node(OP_OR);
  t->line_num = lineNumber;
	t->left_child = $1;
	t->left_child->right_sibling = $3;
	$$ = t;
  }
| '!' expression {
	ast_node t = create_ast_node(OP_BANG);
  t->line_num = lineNumber;
	t->left_child = $2;
	$$ = t;
  }
| '-' expression %prec UMINUS {
	ast_node t = create_ast_node(OP_NEG);
  t->line_num = lineNumber;
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
  t->line_num = lineNumber;
	t->left_child = $2;
	$$ = t;
  }
| DECREMENTTOKEN var {
	ast_node t = create_ast_node(OP_DEC);
  t->line_num = lineNumber;
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
  t->line_num = lineNumber;
	t->value.int_value = atoi(savedText);
	$$ = t;
  }
| FNUMCONST {
	ast_node t = create_ast_node(DOUBLE_LITERAL);
  t->line_num = lineNumber;
	t->value.double_value = atof(savedText);
	$$ = t;
  }
;

call : 
  ident '(' args ')' {
	ast_node t = create_ast_node(FUNC_CALL);
  t->line_num = lineNumber;
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
  fprintf(stderr, "%s at line %d", s, lineNumber);

  if (error_string) {
    fprintf(stderr, ": \"%s\"\n", error_string);
    error_string = NULL;
  }

  fprintf(stderr, "\n");
  return 1;
}

// Set the type of a function ID from a function declaration
void set_func_decl_type(ast_node func_decl_node)
{
  enum vartype var_type;
  switch (func_decl_node->left_child->node_type) {
    case INT_TYPE:
      var_type = inttype;
      break;
    case DBL_TYPE:
      var_type = doubletype;
      break;
    case VOID_TYPE:
      var_type = voidtype;
  }

  ast_node id_node = func_decl_node->left_child->right_sibling;
  id_node->value.sym_node->var_type = var_type;
}

// Set the type of a formal parameter in a function signature
void set_formal_param_type(ast_node formal_param_node)
{
  enum vartype var_type;
  switch (formal_param_node->left_child->node_type) {
    case INT_TYPE:
      var_type = inttype;
      break;
    case DBL_TYPE:
      var_type = doubletype;
      break;
  }

  ast_node id_node = formal_param_node->left_child->right_sibling;
  id_node->value.sym_node->var_type = var_type;
}

// Set the types of all variables declared in the same statement
void set_var_decl_types(ast_node var_decl_node)
{
  enum vartype var_type;
  switch (var_decl_node->left_child->node_type) {
    case INT_TYPE:
      var_type = inttype;
      break;
    case DBL_TYPE:
      var_type = doubletype;
      break;
  }

  ast_node decl_child_node;
  for (decl_child_node = var_decl_node->left_child->right_sibling; decl_child_node != NULL; decl_child_node = decl_child_node->right_sibling) {
    if (decl_child_node->node_type == OP_ASSIGN) {
      decl_child_node->left_child->value.sym_node->var_type = var_type;
    } else if (decl_child_node->node_type == ARRAY_SUB) {
      decl_child_node->left_child->value.sym_node->var_type = var_type;
    } else {
      decl_child_node->value.sym_node->var_type = var_type;
    }
  }
}

// Set the type of an array formal parameter (non-subscripted)
void set_array_formal_param_decl_type(ast_node array_formal_param_decl_node)
{
  enum vartype var_type;
  switch (array_formal_param_decl_node->left_child->node_type) {
    case INT_TYPE:
      var_type = inttype;
      break;
    case DBL_TYPE:
      var_type = doubletype;
      break;
  }

  ast_node id_node = array_formal_param_decl_node->left_child->right_sibling;
  id_node->value.sym_node->var_type = var_type;
}

// Performs a pre-order traversal of the syntax tree to set the types of identifiers
int fill_id_types(ast_node node)
{
  // Set types if we have a declaration (function or variable) or a parameter
  switch (node->node_type) {
    case FUNC_DECL:
      set_func_decl_type(node);
      break;
    case FORMAL_PARAM:
      set_formal_param_type(node);
      break;
    case VAR_DECL:
      set_var_decl_types(node);
      break;
    case ARRAY_NONSUB:
      set_array_formal_param_decl_type(node);
      break;
    default: // just recurse for other types of nodes
      break;
  }

  ast_node child;
  for (child = node->left_child; child != NULL; child = child->right_sibling) {
    fill_id_types(child);
  }
}

int type_check(ast_node node)
{
  int error_count = 0;

  ast_node child;
  for (child = node->left_child; child != NULL; child = child->right_sibling) {
    error_count += type_check(child);
  }

  switch (node->node_type) {
    case ROOT:

      break;
    case ID:

      break;
    case INT_TYPE:

      break;
    case DBL_TYPE:

      break;
    case VOID_TYPE:

      break;
    case ARRAY_SUB:

      break;
    case ARRAY_NONSUB:

      break;
    case OP_ASSIGN:

      break;
    case OP_ADD:

      break;
    case OP_SUB:

      break;
    case OP_MULT:

      break;
    case OP_DIV:

      break;
    case OP_MOD:

      break;
    case OP_LT:

      break;
    case OP_LEQ:

      break;
    case OP_GT:

      break;
    case OP_GEQ:

      break;
    case OP_EQ:

      break;
    case OP_NEQ:

      break;
    case OP_AND:

      break;
    case OP_OR:

      break;
    case OP_BANG:

      break;
    case OP_NEG:

      break;
    case OP_INC:

      break;
    case OP_DEC:

      break;
    case FUNC_DECL:

      break;
    case VAR_DECL:

      break;
    case FORMAL_PARAM:

      break;
    case SEQ:

      break;
    case IF_STMT:

      break;
    case WHILE_LOOP:

      break;
    case DO_WHILE_LOOP:

      break;
    case FOR_STMT:

      break;
    case RETURN_STMT:

      break;
    case READ_STMT:

      break;
    case PRINT_STMT:

      break;
    case STRING_LITERAL:

      break;
    case INT_LITERAL:

      break;
    case DOUBLE_LITERAL:

      break;
    case FUNC_CALL:

      break;
    case EMPTY_EXPR:

      break;
  }
}
