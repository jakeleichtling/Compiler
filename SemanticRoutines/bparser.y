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
extern symboltable scoped_id_table;
extern symboltable flat_id_table;
extern symboltable stringconst_table;
extern symboltable id_name_table;
int scoped_id_table_level;

extern int error_count;
char *error_string;
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

    // just save the name of the ID for now
    t_id->value.sym_node = insert_into_symboltable(id_name_table, savedText);

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
  type_specifier ident '(' formal_params ')' compound_statement {
    ast_node t = create_ast_node(FUNC_DECL);
    t->line_num = lineNumber;
    t->left_child = $1;
    t->left_child->right_sibling = $2;
    t->left_child->right_sibling->value.sym_node->node_type = func_node;
    t->left_child->right_sibling->value.sym_node->decl_ast_node = t;
    t->left_child->right_sibling->right_sibling = $4;
    rightmost_sibling(t->left_child)->right_sibling = $6;
    $$ = t;
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
  '{' {
    enter_scope(scoped_id_table);

    ast_node t = create_ast_node(SEQ);
    t->line_num = lineNumber;

    $1 = t;
  } local_declarations statement_list '}' {
    ast_node t = $1;
    if ($3 != NULL) {
      t->left_child = $3;
      rightmost_sibling(t->left_child)->right_sibling = $4;
    } else {
      t->left_child = $4;
    }

    leave_scope(scoped_id_table);

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

/* ~~~~~~~~~~~~~~~ Function Prototypes ~~~~~~~~~~~~~~~~~~~ */

// Standard code for checking the types of the operands of a binary operation
//  and setting the type of the operation
void standard_binary_op_typecheck_widening(ast_node node);

// Standard code for checking the types of the operands of a binary operation that is of type int
//  and setting the type of the operation (<, <=, >, >=, == , !=)
void standard_binary_op_typecheck_int(ast_node node);

/* ~~~~~~~~~~~~~~~ Function Definitions ~~~~~~~~~~~~~~~~~~~ */

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

void mark_error(int ln, char *msg) {
  fprintf(stderr, "Type error found at line %d: %s\n", ln, msg);
  error_count++;
}

// Recursively call fill_id_types on chlidren and return 1 if any of the recursive calls return nonzero;
//   otherwise return 0
int children_fill_id_types(ast_node node)
{
  ast_node child;
  for (child = node->left_child; child != NULL; child = child->right_sibling) {
    if (fill_id_types(child) != 0) {
      return 1;
    }
  }

  return 0;
}

// Performs a pre-order traversal of the syntax tree to set the types of identifiers.
//   Uses a multi-level symbol table to keep track of scoping; then ID names are mangled and saved in a flat symbol table.
int fill_id_types(ast_node node)
{
  print_ast_node(node);
  printf("\n");
  // Set types if we have a declaration (function or variable) or a parameter
  switch (node->node_type) {
    case FUNC_DECL:
    {
      // Find the return type of the function declaration
      enum vartype var_type;
      switch (node->left_child->node_type) {
        case INT_TYPE:
          var_type = inttype;
          break;
        case DBL_TYPE:
          var_type = doubletype;
          break;
        case VOID_TYPE:
          var_type = voidtype;
      }

      // Get the func ID's ast node
      ast_node id_astnode = node->left_child->right_sibling;

      // Look up the name in the ID name table and make sure it doesn't already exist in the same scope
      char *basename = id_astnode->value.sym_node->name;
      symnode prev_scoped_id_symnode = lookup_in_symboltable(scoped_id_table, basename, &scoped_id_table_level);
      if (prev_scoped_id_symnode != NULL && scoped_id_table_level == scoped_id_table->inner_scope->level) {
        mark_error(node->line_num, "An ID of this name already exists in the same scope");
        return 1;
      }

      // Add the func ID to the scoped ID table, then add it with the mangled name to the flat ID table
      symnode scoped_id_symnode = insert_into_symboltable(scoped_id_table, basename);
      symnode flat_id_symnode = insert_into_symboltable(flat_id_table, scoped_id_symnode->mangled_name);

      // Set the node and return type of the sym_node, and point to the symnode from the id astnode
      flat_id_symnode->node_type = func_node;
      flat_id_symnode->var_type = var_type;
      id_astnode->value.sym_node = flat_id_symnode;

      // Enter into the function's scope
      enter_scope(scoped_id_table);

      // Recurse on the formal parameters and body
      ast_node child;
      for (child = id_astnode->right_sibling; child != NULL; child = child->right_sibling) {
        if (fill_id_types(child) != 0) {
          return 1;
        }
      }

      // Exit the function's scope
      leave_scope(scoped_id_table);

      return 0;
    }
    case FORMAL_PARAM:
    {
      // Find the type of the param
      enum vartype var_type;
      switch (node->left_child->node_type) {
        case INT_TYPE:
          var_type = inttype;
          break;
        case DBL_TYPE:
          var_type = doubletype;
          break;
      }

      // Get the param ID's ast node
      ast_node id_astnode = node->left_child->right_sibling;

      // Look up the name in the ID name table and make sure it doesn't already exist in the same scope
      char *basename = id_astnode->value.sym_node->name;
      symnode prev_scoped_id_symnode = lookup_in_symboltable(scoped_id_table, basename, &scoped_id_table_level);
      if (prev_scoped_id_symnode != NULL && scoped_id_table_level == scoped_id_table->inner_scope->level) {
        mark_error(node->line_num, "A formal parameter of this name already exists in this function declaration");
        return 1;
      }

      // Add the param ID to the scoped ID table, then add it with the mangled name to the flat ID table
      symnode scoped_id_symnode = insert_into_symboltable(scoped_id_table, basename);
      symnode flat_id_symnode = insert_into_symboltable(flat_id_table, scoped_id_symnode->mangled_name);

      // Set the node and var_type of the sym_node, and point to the symnode from the id astnode
      flat_id_symnode->node_type = val_node;
      flat_id_symnode->var_type = var_type;
      id_astnode->value.sym_node = flat_id_symnode;

      return 0;
    }
    case VAR_DECL:
    {
      // Find the type of the variables
      enum vartype var_type;
      switch (node->left_child->node_type) {
        case INT_TYPE:
          var_type = inttype;
          break;
        case DBL_TYPE:
          var_type = doubletype;
          break;
      }

      // Make the symnodes and set the types for all of the declared variables
      ast_node child;
      for (child = node->left_child->right_sibling; child != NULL; child = child->right_sibling) {
        if (child->node_type == ID) {
          // We already have the ID's ast node
          ast_node id_astnode = child;

          // Look up the name in the ID name table and make sure it doesn't already exist in the same scope
          char *basename = id_astnode->value.sym_node->name;
          symnode prev_scoped_id_symnode = lookup_in_symboltable(scoped_id_table, basename, &scoped_id_table_level);
          if (prev_scoped_id_symnode != NULL && scoped_id_table_level == scoped_id_table->inner_scope->level) {
            mark_error(node->line_num, "An ID of this name already exists in the same scope");
            return 1;
          }

          // Add the param ID to the scoped ID table, then add it with the mangled name to the flat ID table
          symnode scoped_id_symnode = insert_into_symboltable(scoped_id_table, basename);
          symnode flat_id_symnode = insert_into_symboltable(flat_id_table, scoped_id_symnode->mangled_name);

          // Set the node and var_type of the sym_node, and point to the symnode from the id astnode
          flat_id_symnode->node_type = val_node;
          flat_id_symnode->var_type = var_type;
          id_astnode->value.sym_node = flat_id_symnode;
        } else if (child->node_type == ARRAY_SUB || child->node_type == OP_ASSIGN) {
          // Get the ID's ast node
          ast_node id_astnode = child->left_child;

          // Look up the name in the ID name table and make sure it doesn't already exist in the same scope
          char *basename = id_astnode->value.sym_node->name;
          symnode prev_scoped_id_symnode = lookup_in_symboltable(scoped_id_table, basename, &scoped_id_table_level);
          if (prev_scoped_id_symnode != NULL && scoped_id_table_level == scoped_id_table->inner_scope->level) {
            mark_error(node->line_num, "An ID of this name already exists in the same scope");
            return 1;
          }

          // Add the param ID to the scoped ID table, then add it with the mangled name to the flat ID table
          symnode scoped_id_symnode = insert_into_symboltable(scoped_id_table, basename);
          symnode flat_id_symnode = insert_into_symboltable(flat_id_table, scoped_id_symnode->mangled_name);

          // Set the node and var_type of the sym_node, and point to the symnode from the id astnode
          if (child->node_type == ARRAY_SUB) {
            flat_id_symnode->node_type = array_node;
          } else {
            flat_id_symnode->node_type = val_node;
          }
          flat_id_symnode->var_type = var_type;
          id_astnode->value.sym_node = flat_id_symnode;

          // Record size if the subscript is an int literal
          if (id_astnode->right_sibling->node_type == INT_LITERAL) {
            flat_id_symnode->array_size = id_astnode->right_sibling->value.int_value;
          }

          // Recurse on the right child
          return fill_id_types(id_astnode->right_sibling);
        }
      }

      return 0;
    }
    case ARRAY_NONSUB:
    {
      // Find the type of the param
      enum vartype var_type;
      switch (node->left_child->node_type) {
        case INT_TYPE:
          var_type = inttype;
          break;
        case DBL_TYPE:
          var_type = doubletype;
          break;
      }

      // Get the param ID's ast node
      ast_node id_astnode = node->left_child->right_sibling;

      // Look up the name in the ID name table and make sure it doesn't already exist in the same scope
      char *basename = id_astnode->value.sym_node->name;
      symnode prev_scoped_id_symnode = lookup_in_symboltable(scoped_id_table, basename, &scoped_id_table_level);
      if (prev_scoped_id_symnode != NULL && scoped_id_table_level == scoped_id_table->inner_scope->level) {
        mark_error(node->line_num, "A formal parameter of this name already exists in this function declaration");
        return 1;
      }

      // Add the param ID to the scoped ID table, then add it with the mangled name to the flat ID table
      symnode scoped_id_symnode = insert_into_symboltable(scoped_id_table, basename);
      symnode flat_id_symnode = insert_into_symboltable(flat_id_table, scoped_id_symnode->mangled_name);

      // Set the node and var_type of the sym_node, and point to the symnode from the id astnode
      flat_id_symnode->node_type = array_node;
      flat_id_symnode->var_type = var_type;
      id_astnode->value.sym_node = flat_id_symnode;

      return 0;
    }
    case ROOT:
    {
      ast_node child;
      for (child = node->left_child; child != NULL; child = child->right_sibling) {
        if (fill_id_types(child) != 0) {
          return 1;
        }
      }

      return 0;
    }
    case ID:
    {
      // We already have the ID's ast_node
      ast_node id_astnode = node;

      // Look up the name in the ID name table and make sure it is accessible in the current scope
      char *basename = id_astnode->value.sym_node->name;
      symnode prev_scoped_id_symnode = lookup_in_symboltable(scoped_id_table, basename, &scoped_id_table_level);
      if (prev_scoped_id_symnode == NULL) {
        mark_error(node->line_num, "A variable is referenced but never declared in an accessible scope");
        return 1;
      }

      // Find the correct mangled symnode
      symnode flat_id_symnode = lookup_in_symboltable(flat_id_table, prev_scoped_id_symnode->mangled_name, &scoped_id_table_level);

      // The ast node points to the mangled sym_node of the proper declaration
      id_astnode->value.sym_node = flat_id_symnode;

      return 0;
    }
    case ARRAY_SUB:
      return children_fill_id_types(node);
    case OP_ASSIGN:
      return children_fill_id_types(node);
    case OP_ADD:
      return children_fill_id_types(node);
    case OP_SUB:
      return children_fill_id_types(node);
    case OP_MULT:
      return children_fill_id_types(node);
    case OP_DIV:
      return children_fill_id_types(node);
    case OP_MOD:
      return children_fill_id_types(node);
    case OP_LT:
      return children_fill_id_types(node);
    case OP_LEQ:
      return children_fill_id_types(node);
    case OP_GT:
      return children_fill_id_types(node);
    case OP_GEQ:
      return children_fill_id_types(node);
    case OP_EQ:
      return children_fill_id_types(node);
    case OP_NEQ:
      return children_fill_id_types(node);
    case OP_AND:
      return children_fill_id_types(node);
    case OP_OR:
      return children_fill_id_types(node);
    case OP_BANG:
      return children_fill_id_types(node);
    case OP_NEG:
      return children_fill_id_types(node);
    case OP_INC:
      return children_fill_id_types(node);
    case OP_DEC:
      return children_fill_id_types(node);
    case SEQ:
    {
      // Enter a new scope for the SEQ
      enter_scope(scoped_id_table);

      // Recurse on children
      ast_node child;
      for (child = node->left_child; child != NULL; child = child->right_sibling) {
        if (fill_id_types(child) != 0) {
          return 1;
        }
      }

      // Exit the SEQ's scope
      leave_scope(scoped_id_table);

      return 0;
    }
    case IF_STMT:
      return children_fill_id_types(node);
    case WHILE_LOOP:
      return children_fill_id_types(node);
    case DO_WHILE_LOOP:
      return children_fill_id_types(node);
    case FOR_STMT:
      return children_fill_id_types(node);
    case RETURN_STMT:
      return children_fill_id_types(node);
    case READ_STMT:
      return children_fill_id_types(node);
    case PRINT_STMT:
      return children_fill_id_types(node);
    case FUNC_CALL:
      return children_fill_id_types(node);
  }

  return 0;
}

void type_check(ast_node node)
{
  int error_count = 0;

  ast_node child;
  for (child = node->left_child; child != NULL; child = child->right_sibling) {
    type_check(child);
  }

  node->data_type = no_type;
  node->return_type = no_type;
  node->is_array_ptr = 0;

  switch (node->node_type) {
    case ROOT:
      break;
    case ID:
      node->data_type = node->value.sym_node->var_type;
      if (node->value.sym_node -> node_type == array_node) {
        node->is_array_ptr = 1;
      }
      break;
    case INT_TYPE:
      break;
    case DBL_TYPE:
      break;
    case VOID_TYPE:
      break;
    case ARRAY_SUB:
    {
      enum vartype sub_type = node->left_child->right_sibling->data_type;
      if (sub_type != inttype) {
        mark_error(node->line_num, "Array index is not an int");
      } else if (node->left_child->value.sym_node->node_type != array_node) {
        mark_error(node->line_num, "Subscripted variable is not an array");
      }

      node->data_type = node->left_child->value.sym_node->var_type;

      break;
    }
    case ARRAY_NONSUB:
      node->data_type = node->left_child->right_sibling->value.sym_node->var_type;
      break;
    case OP_ASSIGN:
    {
      if (node->left_child->node_type == ID && node->left_child->value.sym_node->node_type == array_node) {
        if (node->left_child->right_sibling->is_array_ptr == 0) {
          mark_error(node->line_num, "Cannot assign a non-array pointer to an array pointer variable");
        }

        node->is_array_ptr = 1;
      }

      enum vartype ltype = node->left_child->data_type;
      enum vartype rtype = node->left_child->right_sibling->data_type;

      // widen if assigning int to double
      int shouldWiden = (ltype == doubletype) && (rtype == inttype);
      if (rtype != ltype && !shouldWiden){
        mark_error(node->line_num, "Cannot assign a double to an int");
      }
      node->data_type = ltype;
      break;
    }
    case OP_ADD:
      standard_binary_op_typecheck_widening(node);
      break;
    case OP_SUB:
      standard_binary_op_typecheck_widening(node);
      break;
    case OP_MULT:
      standard_binary_op_typecheck_widening(node);
      break;
    case OP_DIV:
      standard_binary_op_typecheck_widening(node);
      break;
    case OP_MOD:
    {
      enum vartype ltype = node->left_child->data_type;
      enum vartype rtype = node->left_child->right_sibling->data_type;
      if (ltype != inttype || rtype != inttype) {
        mark_error(node->line_num, "An operand of mod is not of type int");  
      }
      node->data_type = inttype;
      break;
    }
    case OP_LT:
      standard_binary_op_typecheck_int(node);
    case OP_LEQ:
      standard_binary_op_typecheck_int(node);
    case OP_GT:
      standard_binary_op_typecheck_int(node);
    case OP_GEQ:
      standard_binary_op_typecheck_int(node);
    case OP_EQ:
      standard_binary_op_typecheck_int(node);
    case OP_NEQ:
      standard_binary_op_typecheck_int(node);
    case OP_AND:
    {
      enum vartype ltype = node->left_child->data_type;
      enum vartype rtype = node->left_child->right_sibling->data_type;
      if (ltype != inttype || rtype != inttype) {
        mark_error(node->line_num, "An operand of && is not an int");  
      }
      node->data_type = inttype;
      break;
    }
    case OP_OR:
    {
      enum vartype ltype = node->left_child->data_type;
      enum vartype rtype = node->left_child->right_sibling->data_type;
      if (ltype != inttype || rtype != inttype) {
        mark_error(node->line_num, "An operand of || is not an int");  
      }
      node->data_type = inttype;
      break;
    }
    case OP_BANG:
    {
      if (node->left_child->data_type != inttype) {
        mark_error(node->line_num, "The operand of ! is not an int");  
      }
      node->data_type = inttype;
      break;
    }
    case OP_NEG:
    {
      enum vartype dtype = node->left_child->data_type;
      if (dtype != inttype && dtype != doubletype) {
        mark_error(node->line_num, "The operand of - (unary minus) is not an int or double");  
      }
      node->data_type = dtype;
      break;
    }
    case OP_INC:
    {
      enum vartype dtype = node->left_child->data_type;
      if (dtype != inttype) {
        mark_error(node->line_num, "The operand of ++ is not an int");  
      }
      node->data_type = inttype;
      break;
    }
    case OP_DEC:
    {
      enum vartype dtype = node->left_child->data_type;
      if (dtype != inttype) {
        mark_error(node->line_num, "The operand of -- is not an int");  
      }
      node->data_type = inttype;
      break;
    }
    case FUNC_DECL:
    {
      enum vartype body_return_type = node->left_child->right_sibling->right_sibling->return_type;
      enum vartype decl_return_type = node->left_child->right_sibling->value.sym_node->var_type;

      if (!((body_return_type == no_type && decl_return_type == voidtype) || (body_return_type == decl_return_type))) {
        mark_error(node->line_num, "The return types of the function declaration and body do not match");  
      }

      node->return_type = decl_return_type;

      break;
    }
    case VAR_DECL:
      break;
    case FORMAL_PARAM:
      break;
    case SEQ:
    {
      ast_node child;
      for (child = node->left_child; child != NULL; child = child->right_sibling) {
        if (node->return_type == no_type) {
          node->return_type = child->return_type;
        } else if (child->return_type != no_type && node->return_type != child->return_type) {
          mark_error(node->line_num, "The type of this return statement conflicts with that of a previous return statement in this function");
        }
      }

      break;
    }
    case IF_STMT:
      node->return_type = node->left_child->right_sibling->return_type;

      break;
    case WHILE_LOOP:
      node->return_type = node->left_child->right_sibling->return_type;

      break;
    case DO_WHILE_LOOP:
      node->return_type = node->left_child->return_type;

      break;
    case FOR_STMT:
      node->return_type = node->left_child->right_sibling->right_sibling->right_sibling->return_type;

      break;
    case RETURN_STMT:
      node->return_type = node->left_child->data_type;

      break;
    case READ_STMT:
      break;
    case PRINT_STMT:
      break;
    case STRING_LITERAL:
      break;
    case INT_LITERAL:
      node->data_type = inttype;

      break;
    case DOUBLE_LITERAL:
      node->data_type = doubletype;

      break;
    case FUNC_CALL:
    {
      // get function's sym_node
      symnode func_sym_node = node->left_child->value.sym_node;

      // set data_type equal to return type of function
      node->data_type = func_sym_node->var_type;

      // get function declarations ast_node
      ast_node func_decl_ast_node = (ast_node) func_sym_node->decl_ast_node;

      // check parameter types in parallel


      break;
    }
    case EMPTY_EXPR:
      break;
  }
}

// Standard code for checking the types of the operands of a binary operation with widening
//  and setting the type of the operation (+, -, *, /)
void standard_binary_op_typecheck_widening(ast_node node)
{
  enum vartype ltype = node->left_child->data_type;
  enum vartype rtype = node->left_child->right_sibling->data_type;
  if ((ltype != inttype && ltype != doubletype) ||
      (rtype != inttype && rtype != doubletype)) {
    mark_error(node->line_num, "An operand is not of type int or double");  
  } else if ((ltype == doubletype) || (rtype == doubletype)) {
    node->data_type = doubletype;
  } else {
    node->data_type = inttype;
  }
}

// Standard code for checking the types of the operands of a binary operation that is of type int
//  and setting the type of the operation (<, <=, >, >=, == , !=)
void standard_binary_op_typecheck_int(ast_node node)
{
  enum vartype ltype = node->left_child->data_type;
  enum vartype rtype = node->left_child->right_sibling->data_type;
  if ((ltype != inttype && ltype != doubletype) ||
      (rtype != inttype && rtype != doubletype)) {
    mark_error(node->line_num, "An operand is not of type int or float");  
  }
  node->data_type = inttype;
}