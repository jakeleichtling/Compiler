/* djcc.c
 *
 * Defines the main function of the C57 compiler. The main function
 * is how the compiler is invoked from the command line, and the arguments
 * passed define the input C57 file and the output TM57 assembly file. Debugging
 * mode can also be set.
 *
 * Usage: djcc [-d] input_C57_file_path [output_TM57_file_path]
 *
 * Jake Leichtling & Derek Salama
 * 5/29/2013
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bparser.tab.h"
#include "ast.h"
#include "symtab.h"
#include "quad.h"
#include "ast_node_processing.h"
#include "tm57_assembly_generation.h"

extern quad *quad_array;

symboltable scoped_id_table; // Used to look up unmangled IDs with scoping
symboltable flat_id_table; // Used to store IDs with mangled names
symboltable stringconst_table; // Used throughout to save memory with string constants
symboltable id_name_table; // Used by the parser to save memory with ID names

ast_node root = NULL;
int parseError = 0;

int error_count = 0;

int djdebug = 0;
extern int yydebug;

extern FILE *yyin;

// Will be provided at the command line
char *input_file_name;
FILE *input_file_handle = NULL;

// Will be provided at the command line, or defaults
char *assembly_file_name = "assembly.tm57";

// The symnode of the main function (declared in symtab.h)
extern symnode main_func_symnode;

int yyparse();

int main(int argc, char *argv[])
{
  if (argc > 4 || argc < 2) {
    fprintf(stderr, "%s\n", "Usage: djcc [-d] input_C57_file_path [output_TM57_file_path]");
    exit(1);
  } else if (argc == 4) {
    if(strncmp(argv[1], "-d", 2) == 0) {
      djdebug = 1;
      input_file_name = argv[2];
      assembly_file_name = argv[3];
    } else {
      fprintf(stderr, "%s\n","Usage: djcc [-d] input_C57_file_path [output_TM57_file_path]");
      exit(1);
    }
  } else if (argc == 3) {
    if(strncmp(argv[1], "-d", 2) == 0) {
      djdebug = 1;
      input_file_name = argv[2];
    } else {
      input_file_name = argv[1];
      assembly_file_name = argv[2];
    }
  } else if (argc == 2) {
    if(strncmp(argv[1], "-d", 2) == 0) {
      fprintf(stderr, "%s\n","Usage: djcc [-d] input_C57_file_path [output_TM57_file_path]");
      exit(1);
    } else {
      input_file_name = argv[1];
    }
  }

  // Open the input C57 file for reading
  input_file_handle = fopen(input_file_name, "r");
  if (!input_file_handle) {
    fprintf(stderr, "Unable to read from C57 input file of name %s\n", input_file_name);
    exit(1);
  }

  // Set yyin to parse from the input file
  yyin = input_file_handle;

  int haveRoot = 0; // 0 means we have a root

  init_quad_array(-1);

  scoped_id_table = create_symboltable();
  flat_id_table = create_symboltable();
  stringconst_table = create_symboltable();
  id_name_table = create_symboltable();

  yydebug = djdebug;

  haveRoot = yyparse();

  if (parseError || haveRoot != 0) {
      fprintf(stderr, "Error: There were parse errors.\n");
      exit(1);
  } else {
      main_func_symnode = NULL;

      if (djdebug) {
        printf("\n~~~~~~~~~~~~~~ fill_id_types ~~~~~~~~~~\n");
      }
      if (fill_id_types(root) != 0) {
        exit (1);
      }

      if (main_func_symnode == NULL) {
        fprintf(stderr, "%s\n", "Error: No main function is defined.");
        exit(1);
      } else if (main_func_symnode->num_params != 0) {
        fprintf(stderr, "%s\n", "Error: The main function cannot take any parameters.");
        exit(1);
      }

      if (djdebug) {
        printf("\n~~~~~~~~~~~ type_check ~~~~~~~~~~~~\n");
      }
      if (type_check(root) != 0) {
        exit (1);
      }

      if (djdebug) {
        printf("\n~~~~~~~~~~~~ generate_intermediate_code ~~~~~~~~~~~\n");
      }
      generate_intermediate_code(root);
      
      if (djdebug) {
        printf("\n~~~~~~~~~~~ print_ast ~~~~~~~~~~~~\n");
        print_ast(root, 0);
      }

      if (djdebug) {
        printf("\n~~~~~~~~~~~ print_quad_array ~~~~~~~~~~~~\n");
        print_quad_array();
      }

      if (djdebug) {
        printf("\n~~~~~~~~~~~ print_symboltable(flat_id_table); ~~~~~~~~~~~~\n");
        print_symboltable(flat_id_table);
      }

      if (djdebug) {
        printf("\n~~~~~~~~~~~ generate_quad_assembly ~~~~~~~~~~~~\n");
      }
      generate_program_assembly();
  }

  return 0;
}