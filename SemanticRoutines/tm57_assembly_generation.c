#include <stdlib.h>
#include <stdio.h>
#include "tm57_assembly_generation.h"
#include "quad.h"

/* ~~~~~~~~~~~~~~~ Function Prototypes ~~~~~~~~~~~~~~~~~~~ */

// Generate the assembly instructions corresponding to the quad currently
//   being considered. If this quad is a jump, then set the corresponding
//   bucket of backpatch_jump_quads and increment assembly_index to make room.
void generate_quad_assembly();

// Generate assembly for standard int binary ops: add_ints_op, sub_ints_op, mult_ints_op, div_ints_op
void gen_standard_int_binary_op(ass_op op, quad_arg dest_arg, quad_arg l_arg, quad_arg r_arg);

// Generate assembly for standard float binary ops
void gen_standard_float_binary_op(ass_op op, quad_arg dest_arg, quad_arg l_arg, quad_arg r_arg);

// Method to print RO instructions
// increments assebly line # after print
void print_ro(ass_op op, int dest_r, int r1, int r2);

// Method to print RM instructions
// increments assebly line # after print
void print_rm(ass_op op, int dest_r, int offset, int r);

// Save variable at node w/ value from source register
void gen_store_int(symnode node, int source_r);

// Save variable at node w/ value from source register
void gen_store_float(symnode node, int source_r);

// Load variable at node into destination register
void gen_load_int(symnode node, int dest_r);

// Load variable at node into destination register
void gen_load_float(symnode node, int dest_r);

// Outputs the quad as a comment in the assembly file for debugging
void print_quad_comment();
void print_quad_arg_comment();

/* ~~~~~~~~~~~~~~~~~~~~~ Variables ~~~~~~~~~~~~~~~~~~~~~~~~ */

// Declared in djcc.c; provided at the command line, or defaults
extern char *assembly_file_name;

// file stream for assembly printing
FILE *file;

// Specifies the index of the assembly line that corresponds to the beginning of each quad;
//   indices are parallel to the quad_array
int *quad_assembly_index;
// Flags jump quads that have been skipped in the initial generation and need
//   to be back patched now that the jump addresses have been determined;
//   non-negative ints indicate the assembly line index that the jump should be written into
int *backpatch_jump_quads;

// The array of quads declared and filled out in quad.c
extern quad *quad_array;
// Points to the bucket just after the last quad in quad_array
extern int next_quad_index;

// The current assembly instruction index
int assembly_index;
// The index of the quad currently being considered
int quad_index;

// pointer to location of global variables on stack
// TODO: set this!
int global_ptr_reg;

// The strings corresponding to quad ops, defined in quad.c
extern char *quad_op_string[];

// Is debugging mode on?
extern int djdebug;

const int stack_ptr_reg = 6;
const int frame_ptr_reg = 5;

/* ~~~~~~~~~~~~~~~ Function Definitions ~~~~~~~~~~~~~~~~~~~ */

// Iterate through the quad_array and generates assembly for each quad,
//   filling out the quad_assembly_lines array accordingly
void generate_program_assembly()
{
  // Open the assembly file (create it if it doesn't exist) and clear it
  file = fopen(assembly_file_name, "w");
  if (!file) {
    fprintf(stderr, "Unable to open/create file of name %s to hold generated assembly instructions\n", assembly_file_name);
    exit(1);
  }

  quad_assembly_index = calloc(next_quad_index, sizeof(int));
  backpatch_jump_quads = calloc(next_quad_index, sizeof(int));

  // Initialize backpatch_jump_quads buckets to -1
  int i;
  for (i = 0; i < next_quad_index; i++) {
    backpatch_jump_quads[i] = -1;
  }

  assembly_index = 0;

  for (quad_index = 0; quad_index < next_quad_index; quad_index++) {
    // Save the assembly index for the start of the quad in the corresponding
    //   bucket of quad_assembly_index
    quad_assembly_index[quad_index] = assembly_index;

    // Print the quad as a comment if debugging
    if (djdebug) {
      print_quad_comment();
    }

    generate_quad_assembly();
  }

  // TODO: Go back and patch quads that jump to an assembly instruction

  // Close the assembly file

}

// Generate the assembly instructions corresponding to the quad currently
//   being considered. If this quad is a jump, then set the corresponding
//   bucket of backpatch_jump_quads and increment assembly_index to appropriate room.
void generate_quad_assembly()
{
  quad curr_quad = quad_array[quad_index];

  switch (curr_quad->op)
  {
    case call_func_op:
    {
			// TODO
			return;
		}
    case print_int_op:
    {
      // Load int value of variable into r0
      gen_load_int(curr_quad->arg1->value.var_node, 0);

      // Print out the int value in r0
      print_ro(OUT, 0, 0, 0);

			return;
		}
    case print_float_op:
    {
      // Load float value of variable into r0
      gen_load_float(curr_quad->arg1->value.var_node, 0);

      // Print out the float value in fr0
      print_ro(OUTF, 0, 0, 0);

      return;
		}
    case print_string_op:
    {
			// TODO
			return;
		}
    case int_to_float_op:
    {
			// Load the int value into r1
      gen_load_int(curr_quad->arg2->value.var_node, 1);

      // Convert the int value to a float value in fr0
      print_ro(CVTIF, 0, 1, 0);

      // Save the float value in fr0 into the float variable
      gen_load_float(curr_quad->arg1->value.var_node, 0);

			return;
		}
    case assn_int_to_var_op:
    {
			// Load the int literal value into r0
      print_rm(LDC, 0, curr_quad->arg2->value.int_value, 0);

      // Store r0 into the int variable
      gen_store_int(curr_quad->arg1->value.var_node, 0);

			return;
		}
    case assn_int_to_arraysub_op:
    {
			// Get the offset into the array, assuming buckets of 8 bytes, and put it in r0
      gen_load_int(curr_quad->arg2->value.var_node, 0);

      // Get the absolute address of the array and put it in r1
      symnode array_id_node = curr_quad->arg1->value.var_node;
      switch(array_id_node->mem_addr_type) {
          case off_fp:
              print_rm(LDA, 1, array_id_node->var_addr, frame_ptr_reg);
              break;

          case global:
              print_rm(LDA, 1, array_id_node->var_addr, global_ptr_reg);
              break;

          case absolute:
              //put 0 in reg3
              print_rm(LDC, 3, 0, 0);

              print_rm(LDA, 1, array_id_node->var_addr, 3);
              break;
      }

      // Calculate r0 <- r0 + r1 to get the address of the destination bucket in the array
      print_ro(ADD, 0, 0, 1);

      // Load the int value into r1
      gen_load_int(curr_quad->arg3->value.var_node, 1);

      // Store r1 into the array bucket (address in r0)
      print_rm(ST, 1, 0, 0);

			return;
		}
    case assn_int_from_arraysub_op:
    {
      // Get the offset into the array, assuming buckets of 8 bytes, and put it in r0
      gen_load_int(curr_quad->arg3->value.var_node, 0);

      // Get the absolute address of the array and put it in r1
      symnode array_id_node = curr_quad->arg2->value.var_node;
      switch(array_id_node->mem_addr_type) {
          case off_fp:
              print_rm(LDA, 1, array_id_node->var_addr, frame_ptr_reg);
              break;

          case global:
              print_rm(LDA, 1, array_id_node->var_addr, global_ptr_reg);
              break;

          case absolute:
              //put 0 in reg3
              print_rm(LDC, 3, 0, 0);

              print_rm(LDA, 1, array_id_node->var_addr, 3);
              break;
      }

      // Calculate r0 <- r0 + r1 to get the address of the destination bucket in the array
      print_ro(ADD, 0, 0, 1);

      // Load the int value from the array bucket (address in r0) into r1
      print_rm(LD, 1, 0, 0);

      // Store the int value into the int variable
      gen_store_int(curr_quad->arg1->value.var_node, 1);

      return;
    }
    case assn_float_to_var_op:
    {
      // Load the double literal value into fr0
      print_rm(LDFC, 0, curr_quad->arg2->value.double_value, 0);

      // Store fr0 into the double variable
      gen_store_float(curr_quad->arg1->value.var_node, 0);

      return;
    }
    case assn_float_to_arraysub_op:
    {
      // Get the offset into the array, assuming buckets of 8 bytes, and put it in r0
      gen_load_int(curr_quad->arg2->value.var_node, 0);

      // Get the absolute address of the array and put it in r1
      symnode array_id_node = curr_quad->arg1->value.var_node;
      switch(array_id_node->mem_addr_type) {
          case off_fp:
              print_rm(LDA, 1, array_id_node->var_addr, frame_ptr_reg);
              break;

          case global:
              print_rm(LDA, 1, array_id_node->var_addr, global_ptr_reg);
              break;

          case absolute:
              //put 0 in reg3
              print_rm(LDC, 3, 0, 0);

              print_rm(LDA, 1, array_id_node->var_addr, 3);
              break;
      }

      // Calculate r0 <- r0 + r1 to get the address of the destination bucket in the array
      print_ro(ADD, 0, 0, 1);

      // Load the double value into fr1
      gen_load_float(curr_quad->arg3->value.var_node, 1);

      // Store fr1 into the array bucket (address in r0)
      print_rm(STF, 1, 0, 0);

      return;
    }
    case assn_float_from_arraysub_op:
    {
      // Get the offset into the array, assuming buckets of 8 bytes, and put it in r0
      gen_load_int(curr_quad->arg3->value.var_node, 0);

      // Get the absolute address of the array and put it in r1
      symnode array_id_node = curr_quad->arg2->value.var_node;
      switch(array_id_node->mem_addr_type) {
          case off_fp:
              print_rm(LDA, 1, array_id_node->var_addr, frame_ptr_reg);
              break;

          case global:
              print_rm(LDA, 1, array_id_node->var_addr, global_ptr_reg);
              break;

          case absolute:
              //put 0 in reg3
              print_rm(LDC, 3, 0, 0);

              print_rm(LDA, 1, array_id_node->var_addr, 3);
              break;
      }

      // Calculate r0 <- r0 + r1 to get the address of the destination bucket in the array
      print_ro(ADD, 0, 0, 1);

      // Load the double value from the array bucket (address in r0) into fr1
      print_rm(LDF, 1, 0, 0);

      // Store the double value into the double variable
      gen_store_float(curr_quad->arg1->value.var_node, 1);

      return;
    }
    case add_ints_op:
    {
      gen_standard_int_binary_op(ADD, curr_quad->arg1, curr_quad->arg2, curr_quad->arg3);
      return;
    }
    case add_floats_op:
    {
			gen_standard_float_binary_op(ADDF, curr_quad->arg1, curr_quad->arg2, curr_quad->arg3);
			return;
		}
    case sub_ints_op:
    {
			gen_standard_int_binary_op(SUB, curr_quad->arg1, curr_quad->arg2, curr_quad->arg3);
			return;
		}
    case sub_floats_op:
    {
			gen_standard_float_binary_op(SUBF, curr_quad->arg1, curr_quad->arg2, curr_quad->arg3);
			return;
		}
    case mult_ints_op:
    {
      gen_standard_int_binary_op(MUL, curr_quad->arg1, curr_quad->arg2, curr_quad->arg3);
			return;
		}
    case mult_floats_op:
    {
			gen_standard_float_binary_op(MULF, curr_quad->arg1, curr_quad->arg2, curr_quad->arg3);
			return;
		}
    case div_ints_op:
    {
			gen_standard_int_binary_op(DIV, curr_quad->arg1, curr_quad->arg2, curr_quad->arg3);
			return;
		}
    case div_floats_op:
    {
			gen_standard_float_binary_op(DIVF, curr_quad->arg1, curr_quad->arg2, curr_quad->arg3);
			return;
		}
    case mod_op:
    {
      gen_standard_int_binary_op(MOD, curr_quad->arg1, curr_quad->arg2, curr_quad->arg3);
			return;
		}

    // Derek
    case lt_ints_op:
    {
			// TODO
			return;
		}
    case lt_floats_op:
    {
			// TODO
			return;
		}
    case leq_ints_op:
    {
			// TODO
			return;
		}
    case leq_floats_op:
    {
			// TODO
			return;
		}
    case gt_ints_op:
    {
			// TODO
			return;
		}
    case gt_floats_op:
    {
			// TODO
			return;
		}
    case geq_ints_op:
    {
			// TODO
			return;
		}
    case geq_floats_op:
    {
			// TODO
			return;
		}
    case eq_ints_op:
    {
			// TODO
			return;
		}
    case eq_floats_op:
    {
			// TODO
			return;
		}
    case neq_ints_op:
    {
			// TODO
			return;
		}
    case neq_floats_op:
    {
			// TODO
			return;
		}
    case and_ints_op:
    {
			// TODO
			return;
		}
    case and_floats_op:
    {
			// TODO
			return;
		}
    case or_ints_op:
    {
			// TODO
			return;
		}
    case or_floats_op:
    {
			// TODO
			return;
		}
    case int_bang_op:
    {
			// TODO
			return;
		}
    case float_bang_op:
    {
			// TODO
			return;
		}
    case int_neg_op:
    {
			// TODO
			return;
		}
    case float_neg_op:
    {
			// TODO
			return;
		}
    case var_inc_op:
    {
			// TODO
			return;
		}
    case array_inc_op:
    {
			// TODO
			return;
		}
    case var_dec_op:
    {
			// TODO
			return;
		}
    case array_dec_op:
    {
			// TODO
			return;
		}
    case if_false_op:
    {
			// TODO
			return;
		}
    case goto_op:
    {
			// TODO
			return;
		}
    case read_int_op:
    {
			// TODO
			return;
		}
    case read_double_op:
    {
			// TODO
			return;
		}
    case halt_op:
    {
			// TODO
			return;
		}
    case func_decl_op:
    {
			// TODO
			return;
		}
    case push_param_op:
    {
			// TODO
			return;
		}
    case alloc_array_op:
    {
			// TODO
			return;
		}
    case return_op:
    {
			// TODO
			return;
		}
    case assign_int_literal:
    {
			// TODO
			return;
		}
    case assign_double_literal:
    {
			// TODO
			return;
		}
  }
}

// Generate assembly for standard int binary ops
void gen_standard_int_binary_op(ass_op op, quad_arg dest_arg, quad_arg l_arg, quad_arg r_arg) {
    symnode l_symnode = l_arg->value.var_node;
    symnode r_symnode = r_arg->value.var_node;
    symnode dest_symnode = dest_arg->value.var_node;

    //load l_symnode into r0
    gen_load_int(l_symnode, 0);

    //load r_symnode into r1
    gen_load_int(r_symnode, 1);

    //execute, store in r0
    print_ro(op, 0, 0, 1);

    //store in destination
    gen_store_int(dest_symnode, 0);
}

// Generate assembly for standard float binary ops
void gen_standard_float_binary_op(ass_op op, quad_arg dest_arg, quad_arg l_arg, quad_arg r_arg) {
    symnode l_symnode = l_arg->value.var_node;
    symnode r_symnode = r_arg->value.var_node;
    symnode dest_symnode = dest_arg->value.var_node;

    //load l_symnode into r0
    gen_load_float(l_symnode, 0);

    //load r_symnode into r1
    gen_load_float(r_symnode, 1);

    //execute, store in r0
    print_ro(op, 0, 0, 1);

    //store in destination
    gen_store_float(dest_symnode, 0);
}

void gen_load_int(symnode node, int dest_r) {
    switch(node->mem_addr_type) {
        case off_fp:
            print_rm(LD, dest_r, node->var_addr, frame_ptr_reg);
            break;

        case global:
            print_rm(LD, dest_r, node->var_addr, global_ptr_reg);
            break;

        case absolute:
            //put 0 in reg3
            print_rm(LDC, 3, 0, 0);

            //load offset(0) 
            print_rm(LD, dest_r, node->var_addr, 3);
            break;
    }
}

void gen_load_float(symnode node, int dest_r) {
    switch(node->mem_addr_type) {
        case off_fp:
            print_rm(LDF, dest_r, node->var_addr, frame_ptr_reg);
            break;

        case global:
            print_rm(LDF, dest_r, node->var_addr, global_ptr_reg);
            break;

        case absolute:
            //put 0 in reg3
            print_rm(LDC, 3, 0, 0);

            //load offset(0) 
            print_rm(LDF, dest_r, node->var_addr, 3);
            break;
    }
}

void gen_store_int(symnode node, int source_r) {
    switch(node->mem_addr_type) {
        case off_fp:
            print_rm(ST, source_r, node->var_addr, frame_ptr_reg);
            break;

        case global:
            print_rm(ST, source_r, node->var_addr, global_ptr_reg);
            break;

        case absolute:
            //put 0 in reg3
            print_rm(LDC, 3, 0, 0);

            print_rm(ST, source_r, node->var_addr, 3);
            break;
    }
}

void gen_store_float(symnode node, int source_r) {
    switch(node->mem_addr_type) {
        case off_fp:
            print_rm(STF, source_r, node->var_addr, frame_ptr_reg);
            break;

        case global:
            print_rm(STF, source_r, node->var_addr, global_ptr_reg);
            break;

        case absolute:
            //put 0 in reg3
            print_rm(LDC, 3, 0, 0);

            print_rm(STF, source_r, node->var_addr, 3);
            break;
    }
}

// Define enum and corresponding strings
char* ass_op_str[] = {
    "HALT",
    "IN",
    "OUT",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "ADDF",
    "SUBF",
    "MULF",
    "DIVF",
    "MOD",
    "CVTIF",
    "CVTFI",
    "LD",
    "LDA",
    "LDC",
    "ST",
    "JLT",
    "JLE",
    "JGE",
    "JGT",
    "JEQ",
    "JNE",
    "LDF",
    "STF",
    "JFLT",
    "JFLE",
    "JFGE",
    "JFGT",
    "JFEQ",
    "JFNE",
    "INF",
    "OUTF",
    "LDFC",
    "LDB",
    "STB",
    "INB",
    "OUTB"
};

// Method to print RO
void print_ro(ass_op op, int dest_r, int r1, int r2) {
    char* op_str = ass_op_str[op];
    fprintf(file, "%d:\t%s\t%d,%d,%d\n", assembly_index, op_str, dest_r, r1, r2);
    assembly_index++;
}

// Method to print RM
void print_rm(ass_op op, int dest_r, int offset, int r) {
    char* op_str = ass_op_str[op];
    fprintf(file, "%d:\t%s\t%d,%d(%d)\n", assembly_index, op_str, dest_r, offset, r);
    assembly_index++;
}


// Outputs the quad as a comment in the assembly file for debugging
void print_quad_comment()
{
    quad curr_quad = quad_array[quad_index];

    fprintf(file, "\n*\t%d:\t(%s, ", quad_index, quad_op_string[curr_quad->op]);
    print_quad_arg_comment(curr_quad->arg1);
    fprintf(file, ", ");
    print_quad_arg_comment(curr_quad->arg2);
    fprintf(file, ", ");
    print_quad_arg_comment(curr_quad->arg3);
    fprintf(file, ")\n");
}

void print_quad_arg_comment(quad_arg the_quad_arg)
{
  if (!the_quad_arg) {
    fprintf(file, "null");
    return;
  }

  switch (the_quad_arg->arg_type) {
    case int_arg:
      fprintf(file, "int: %d", the_quad_arg->value.int_value);
      break;
    case dbl_arg:
      fprintf(file, "double: %f", the_quad_arg->value.double_value);
      break;
    case id_arg:
      fprintf(file, "ID: %s", the_quad_arg->value.var_node->name);
      break;
    default:
      fprintf(file, "arg type not specified");
      break;
  }
}