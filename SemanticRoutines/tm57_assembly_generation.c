/* tm57_assembly_generation.c
 * 
 * Function definitions for generating assembly code from
 * intermediate code.
 *
 * Jake Leichtling & Derek Salama
 * 5/29/2013
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

// Method to print RM instructions for int offsets
// increments assebly line # after print
void print_rm_int(ass_op op, int dest_r, int offset, int r);

// Method to print RM for float offsets
// increments assebly line # after print
void print_rm_float(ass_op op, int dest_r, float offset, int r);

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

// Generate assembly instructions for the current quad, starting at
//   the assembly index indicated in backpatch_jump_quads[quad_index]
void insert_jump_assembly();

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

// The number of global variables, imported from ast_node_processing.c
extern int num_global_vars;
// The number of global temps, imported from quad.c
extern int num_global_temps;

// The symnode of the main function (declared in symtab.h)
extern symnode main_func_symnode;

// The current assembly instruction index
int assembly_index;
// The index of the quad currently being considered
int quad_index;
// The assembly index of the initial call to main's control transfer instruction
int initial_main_call_ctrl_xfer_assembly_index;
// Space used so far by assembly directives;
//   starts at 4 so as not to override DADDR_SIZE in dMem[0] at initialization
int constant_stack_ptr = 4;

// The strings corresponding to quad ops, defined in quad.c
extern char *quad_op_string[];

// Is debugging mode on?
extern int djdebug;

const int program_ctr_reg = 7; // points to the instruction in front of the one currently being executed
const int stack_ptr_reg = 6; // points to the location of the top element of the stack
const int frame_ptr_reg = 5;
const int global_ptr_reg = 4; // points to the highest dMem address, where the global variables begin

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

  // Initializes the stack pointer to DADDR_SIZE - 1,
  //   which is initially stored in dMem[0]
  print_rm_int(LD, stack_ptr_reg, 0, 0);

  // sp <- sp + 1 = DADDR_SIZE
  print_rm_int(LDA, stack_ptr_reg, 1, stack_ptr_reg);

  // global_ptr_reg <- DADDR_SIZE
  print_rm_int(LDA, global_ptr_reg, 0, stack_ptr_reg);

  // Make room for global variables (including temps)
  // r0 <- number of global variables/temps
  print_rm_int(LDC, 0, num_global_vars + num_global_temps, 0);
  // r1 <- -8
  print_rm_int(LDC, 1, -8, 0);
  // r0 <- num global vars * -8
  print_ro(MUL, 0, 0, 1);
  // sp <- sp + (num global vars * -8)
  print_ro(ADD, stack_ptr_reg, stack_ptr_reg, 0);

  // Iterate through quads to generate assembly
  for (quad_index = 0; quad_index < next_quad_index; quad_index++) {
    // Save the assembly index for the start of the quad in the corresponding
    //   bucket of quad_assembly_index
    quad_assembly_index[quad_index] = assembly_index;

    // Print the quad as a comment
    print_quad_comment();

    generate_quad_assembly();
  }

  // Go back and patch quads that jump to an assembly instruction
  for (quad_index = 0; quad_index < next_quad_index; quad_index++) {
    if (backpatch_jump_quads[quad_index] >= 0) {
      insert_jump_assembly();
    }
  }

  // Fill in the control transfer assembly instruction for the initial main call
  assembly_index = initial_main_call_ctrl_xfer_assembly_index;
  int main_func_addr = main_func_symnode->var_addr;
  print_rm_int(LDC, program_ctr_reg, main_func_addr, 0);

  // Close the assembly file
  fclose(file);
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
      // Push return address (instruction following function call) onto top of stack
      //   1.) decrement stack pointer by 4 bytes
      print_rm_int(LDA, stack_ptr_reg, -4, stack_ptr_reg);
      //   2.) r0 <- 5 + PC
      print_rm_int(LDA, 0, 5, program_ctr_reg);
      //   3.) push r0 (the return address) onto the stack
      print_rm_int(ST, 0, 0, stack_ptr_reg);

      // Push the current frame pointer
      //   1.) decrement stack pointer by 4 bytes
      print_rm_int(LDA, stack_ptr_reg, -4, stack_ptr_reg);
      //   2.) push the value of the FP register
      print_rm_int(ST, frame_ptr_reg, 0, stack_ptr_reg);

      // Set the FP register to point to the top of the stack
      print_rm_int(LDA, frame_ptr_reg, 0, stack_ptr_reg);

      // Transfer control to the function being called:
      //   Get the assembly address of the callee and
      int callee_addr = curr_quad->arg1->value.var_node->var_addr;
      //   load that address into the PC register
      print_rm_int(LDC, program_ctr_reg, callee_addr, 0);

      // When the callee returns, if there is a return value (r3), store it
      if (curr_quad->arg1->value.var_node->var_type == inttype) {
        gen_store_int(curr_quad->arg2->value.var_node, 3);
      } else if (curr_quad->arg1->value.var_node->var_type == doubletype) {
        gen_store_float(curr_quad->arg2->value.var_node, 3);
      }

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
      char *str = curr_quad->arg1->value.var_node->name;
			int str_len = strlen(str);
      int start_addr = curr_quad->arg1->value.var_node->var_addr;

      // r0 <- 0
      print_ro(SUB, 0,0,0);

      int i;
      for (i = 0; i < str_len; i++) {
        int loc = start_addr + i;

        // r1 <- char at loc
        print_rm_int(LDB, 1, loc, 0);

        // Print r1
        print_ro(OUTB, 1, 0, 0);
      }

			return;
		}
    case int_to_float_op:
    {
			// Load the int value into r1
      gen_load_int(curr_quad->arg2->value.var_node, 1);

      // Convert the int value to a float value in fr0
      print_ro(CVTIF, 0, 1, 0);

      // Save the float value in fr0 into the float variable
      gen_store_float(curr_quad->arg1->value.var_node, 0);

			return;
		}
    case assn_int_to_var_op:
    {
			// Load the int variable's value into r0
      gen_load_int(curr_quad->arg2->value.var_node, 0);

      // Store r0 into the int variable
      gen_store_int(curr_quad->arg1->value.var_node, 0);

			return;
		}
    case assn_int_to_arraysub_op:
    {
			// Get the offset into the array in terms of number of elements and put it in r0
      gen_load_int(curr_quad->arg2->value.var_node, 0);

      // r1 <- 8
      print_rm_int(LDC, 1, 8, 0);

      // r0 <- element offset * 8 = byte offset into the array
      print_ro(MUL, 0, 0, 1);

      // Get the absolute address of the first element of the array and put it in r1
      symnode array_id_node = curr_quad->arg1->value.var_node;
      switch(array_id_node->mem_addr_type) {
          case off_fp:
              print_rm_int(LD, 1, array_id_node->var_addr, frame_ptr_reg);
              break;

          case global:
              print_rm_int(LD, 1, array_id_node->var_addr, global_ptr_reg);
              break;
      }

      // Calculate r0 <- r0 + r1 to get the address of the destination bucket in the array
      print_ro(ADD, 0, 0, 1);

      // Load the int variable's value into r1
      gen_load_int(curr_quad->arg3->value.var_node, 1);

      // Store r1 into the array bucket (address in r0)
      print_rm_int(ST, 1, 0, 0);

			return;
		}
    case assn_int_from_arraysub_op:
    {
      // Get the offset into the array in terms of number of elements and put it in r0
      gen_load_int(curr_quad->arg3->value.var_node, 0);

      // r1 <- 8
      print_rm_int(LDC, 1, 8, 0);

      // r0 <- element offset * 8 = byte offset into the array
      print_ro(MUL, 0, 0, 1);

      // Get the absolute address of the first element of the array and put it in r1
      symnode array_id_node = curr_quad->arg2->value.var_node;
      switch(array_id_node->mem_addr_type) {
          case off_fp:
              print_rm_int(LD, 1, array_id_node->var_addr, frame_ptr_reg);
              break;

          case global:
              print_rm_int(LD, 1, array_id_node->var_addr, global_ptr_reg);
              break;
      }

      // Calculate r0 <- r0 + r1 to get the address of the destination bucket in the array
      print_ro(ADD, 0, 0, 1);

      // Load the int value from the array bucket (address in r0) into r1
      print_rm_int(LD, 1, 0, 0);

      // Store the int value into the int variable
      gen_store_int(curr_quad->arg1->value.var_node, 1);

      return;
    }
    case assn_float_to_var_op:
    {
      // Load the double variable's value into fr0
      gen_load_float(curr_quad->arg2->value.var_node, 0);

      // Store fr0 into the double variable
      gen_store_float(curr_quad->arg1->value.var_node, 0);

      return;
    }
    case assn_float_to_arraysub_op:
    {
      // Get the offset into the array in terms of number of elements and put it in r0
      gen_load_int(curr_quad->arg2->value.var_node, 0);

      // r1 <- 8
      print_rm_int(LDC, 1, 8, 0);

      // r0 <- element offset * 8 = byte offset into the array
      print_ro(MUL, 0, 0, 1);

      // Get the absolute address of the first element of the array and put it in r1
      symnode array_id_node = curr_quad->arg1->value.var_node;
      switch(array_id_node->mem_addr_type) {
          case off_fp:
              print_rm_int(LD, 1, array_id_node->var_addr, frame_ptr_reg);
              break;

          case global:
              print_rm_int(LD, 1, array_id_node->var_addr, global_ptr_reg);
              break;              
      }

      // Calculate r0 <- r0 + r1 to get the address of the destination bucket in the array
      print_ro(ADD, 0, 0, 1);

      // Load the double value into fr1
      gen_load_float(curr_quad->arg3->value.var_node, 1);

      // Store fr1 into the array bucket (address in r0)
      print_rm_int(STF, 1, 0, 0);

      return;
    }
    case assn_float_from_arraysub_op:
    {
      // Get the offset into the array in terms of number of elements and put it in r0
      gen_load_int(curr_quad->arg3->value.var_node, 0);

      // r1 <- 8
      print_rm_int(LDC, 1, 8, 0);

      // r0 <- element offset * 8 = byte offset into the array
      print_ro(MUL, 0, 0, 1);

      // Get the absolute address of the first element of the array and put it in r1
      symnode array_id_node = curr_quad->arg2->value.var_node;
      switch(array_id_node->mem_addr_type) {
          case off_fp:
              print_rm_int(LD, 1, array_id_node->var_addr, frame_ptr_reg);
              break;

          case global:
              print_rm_int(LD, 1, array_id_node->var_addr, global_ptr_reg);
              break;              
      }

      // Calculate r0 <- r0 + r1 to get the address of the destination bucket in the array
      print_ro(ADD, 0, 0, 1);

      // Load the double value from the array bucket (address in r0) into fr1
      print_rm_int(LDF, 1, 0, 0);

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

    case lt_ints_op:
    {
      // Load the left int argument into r0
      gen_load_int(curr_quad->arg2->value.var_node, 0);

      // Load the right int argument into r1
      gen_load_int(curr_quad->arg3->value.var_node, 1);

      // r0 <- r0 - r1
      print_ro(SUB, 0, 0, 1);

      // If r0 < 0, jump to L1 (2 instructions)
      print_rm_int(JLT, 0, 2, program_ctr_reg);

      // Put 0 in r0 and jump to L2 (1 instruction)
      print_rm_int(LDC, 0, 0, 0);
      print_rm_int(LDA, program_ctr_reg, 1, program_ctr_reg);

      // L1: Put 1 in r0
      print_rm_int(LDC, 0, 1, 0);

      // L2: Store r0 into the int variable
      gen_store_int(curr_quad->arg1->value.var_node, 0);

			return;
		}
    case lt_floats_op:
    {
      // Load the left float argument into fr0
      gen_load_float(curr_quad->arg2->value.var_node, 0);

      // Load the right float argument into fr1
      gen_load_float(curr_quad->arg3->value.var_node, 1);

      // fr0 <- fr0 - fr1
      print_ro(SUBF, 0, 0, 1);

      // If fr0 < 0, jump to L1 (2 instructions)
      print_rm_int(JFLT, 0, 2, program_ctr_reg);

      // Put 0.0 in fr0 and jump to L2 (1 instruction)
      print_rm_float(LDFC, 0, 0.0, 0);
      print_rm_int(LDA, program_ctr_reg, 1, program_ctr_reg);

      // L1: Put 1.0 in fr0
      print_rm_int(LDC, 0, 1.0, 0);

      // L2: Store fr0 into the double variable
      gen_store_float(curr_quad->arg1->value.var_node, 0);

      return;
    }
    case leq_ints_op:
    {
      // Load the left int argument into r0
      gen_load_int(curr_quad->arg2->value.var_node, 0);

      // Load the right int argument into r1
      gen_load_int(curr_quad->arg3->value.var_node, 1);

      // r0 <- r0 - r1
      print_ro(SUB, 0, 0, 1);

      // If r0 <= 0, jump to L1 (2 instructions)
      print_rm_int(JLE, 0, 2, program_ctr_reg);

      // Put 0 in r0 and jump to L2 (1 instruction)
      print_rm_int(LDC, 0, 0, 0);
      print_rm_int(LDA, program_ctr_reg, 1, program_ctr_reg);

      // L1: Put 1 in r0
      print_rm_int(LDC, 0, 1, 0);

      // L2: Store r0 into the int variable
      gen_store_int(curr_quad->arg1->value.var_node, 0);

      return;
    }
    case leq_floats_op:
    {
      // Load the left float argument into fr0
      gen_load_float(curr_quad->arg2->value.var_node, 0);

      // Load the right float argument into fr1
      gen_load_float(curr_quad->arg3->value.var_node, 1);

      // fr0 <- fr0 - fr1
      print_ro(SUBF, 0, 0, 1);

      // If fr0 <= 0, jump to L1 (2 instructions)
      print_rm_int(JFLE, 0, 2, program_ctr_reg);

      // Put 0.0 in fr0 and jump to L2 (1 instruction)
      print_rm_float(LDFC, 0, 0.0, 0);
      print_rm_int(LDA, program_ctr_reg, 1, program_ctr_reg);

      // L1: Put 1.0 in fr0
      print_rm_int(LDC, 0, 1.0, 0);

      // L2: Store fr0 into the double variable
      gen_store_float(curr_quad->arg1->value.var_node, 0);

      return;
    }
    case gt_ints_op:
    {
      // Load the left int argument into r0
      gen_load_int(curr_quad->arg2->value.var_node, 0);

      // Load the right int argument into r1
      gen_load_int(curr_quad->arg3->value.var_node, 1);

      // r0 <- r0 - r1
      print_ro(SUB, 0, 0, 1);

      // If r0 > 0, jump to L1 (2 instructions)
      print_rm_int(JGT, 0, 2, program_ctr_reg);

      // Put 0 in r0 and jump to L2 (1 instruction)
      print_rm_int(LDC, 0, 0, 0);
      print_rm_int(LDA, program_ctr_reg, 1, program_ctr_reg);

      // L1: Put 1 in r0
      print_rm_int(LDC, 0, 1, 0);

      // L2: Store r0 into the int variable
      gen_store_int(curr_quad->arg1->value.var_node, 0);

      return;
    }
    case gt_floats_op:
    {
      // Load the left float argument into fr0
      gen_load_float(curr_quad->arg2->value.var_node, 0);

      // Load the right float argument into fr1
      gen_load_float(curr_quad->arg3->value.var_node, 1);

      // fr0 <- fr0 - fr1
      print_ro(SUBF, 0, 0, 1);

      // If fr0 > 0, jump to L1 (2 instructions)
      print_rm_int(JFGT, 0, 2, program_ctr_reg);

      // Put 0.0 in fr0 and jump to L2 (1 instruction)
      print_rm_float(LDFC, 0, 0.0, 0);
      print_rm_int(LDA, program_ctr_reg, 1, program_ctr_reg);

      // L1: Put 1.0 in fr0
      print_rm_int(LDC, 0, 1.0, 0);

      // L2: Store fr0 into the double variable
      gen_store_float(curr_quad->arg1->value.var_node, 0);

      return;
    }
    case geq_ints_op:
    {
      // Load the left int argument into r0
      gen_load_int(curr_quad->arg2->value.var_node, 0);

      // Load the right int argument into r1
      gen_load_int(curr_quad->arg3->value.var_node, 1);

      // r0 <- r0 - r1
      print_ro(SUB, 0, 0, 1);

      // If r0 >= 0, jump to L1 (2 instructions)
      print_rm_int(JGE, 0, 2, program_ctr_reg);

      // Put 0 in r0 and jump to L2 (1 instruction)
      print_rm_int(LDC, 0, 0, 0);
      print_rm_int(LDA, program_ctr_reg, 1, program_ctr_reg);

      // L1: Put 1 in r0
      print_rm_int(LDC, 0, 1, 0);

      // L2: Store r0 into the int variable
      gen_store_int(curr_quad->arg1->value.var_node, 0);

      return;
    }
    case geq_floats_op:
    {
      // Load the left float argument into fr0
      gen_load_float(curr_quad->arg2->value.var_node, 0);

      // Load the right float argument into fr1
      gen_load_float(curr_quad->arg3->value.var_node, 1);

      // fr0 <- fr0 - fr1
      print_ro(SUBF, 0, 0, 1);

      // If fr0 >= 0, jump to L1 (2 instructions)
      print_rm_int(JFGE, 0, 2, program_ctr_reg);

      // Put 0.0 in fr0 and jump to L2 (1 instruction)
      print_rm_float(LDFC, 0, 0.0, 0);
      print_rm_int(LDA, program_ctr_reg, 1, program_ctr_reg);

      // L1: Put 1.0 in fr0
      print_rm_int(LDC, 0, 1.0, 0);

      // L2: Store fr0 into the double variable
      gen_store_float(curr_quad->arg1->value.var_node, 0);

      return;
    }
    case eq_ints_op:
    {
      // Load the left int argument into r0
      gen_load_int(curr_quad->arg2->value.var_node, 0);

      // Load the right int argument into r1
      gen_load_int(curr_quad->arg3->value.var_node, 1);

      // r0 <- r0 - r1
      print_ro(SUB, 0, 0, 1);

      // If r0 == 0, jump to L1 (2 instructions)
      print_rm_int(JEQ, 0, 2, program_ctr_reg);

      // Put 0 in r0 and jump to L2 (1 instruction)
      print_rm_int(LDC, 0, 0, 0);
      print_rm_int(LDA, program_ctr_reg, 1, program_ctr_reg);

      // L1: Put 1 in r0
      print_rm_int(LDC, 0, 1, 0);

      // L2: Store r0 into the int variable
      gen_store_int(curr_quad->arg1->value.var_node, 0);

      return;
    }
    case eq_floats_op:
    {
      // Load the left float argument into fr0
      gen_load_float(curr_quad->arg2->value.var_node, 0);

      // Load the right float argument into fr1
      gen_load_float(curr_quad->arg3->value.var_node, 1);

      // fr0 <- fr0 - fr1
      print_ro(SUBF, 0, 0, 1);

      // If fr0 == 0, jump to L1 (2 instructions)
      print_rm_int(JFEQ, 0, 2, program_ctr_reg);

      // Put 0.0 in fr0 and jump to L2 (1 instruction)
      print_rm_float(LDFC, 0, 0.0, 0);
      print_rm_int(LDA, program_ctr_reg, 1, program_ctr_reg);

      // L1: Put 1.0 in fr0
      print_rm_int(LDC, 0, 1.0, 0);

      // L2: Store fr0 into the double variable
      gen_store_float(curr_quad->arg1->value.var_node, 0);

      return;
    }
    case neq_ints_op:
    {
      // Load the left int argument into r0
      gen_load_int(curr_quad->arg2->value.var_node, 0);

      // Load the right int argument into r1
      gen_load_int(curr_quad->arg3->value.var_node, 1);

      // r0 <- r0 - r1
      print_ro(SUB, 0, 0, 1);

      // If r0 != 0, jump to L1 (2 instructions)
      print_rm_int(JNE, 0, 2, program_ctr_reg);

      // Put 0 in r0 and jump to L2 (1 instruction)
      print_rm_int(LDC, 0, 0, 0);
      print_rm_int(LDA, program_ctr_reg, 1, program_ctr_reg);

      // L1: Put 1 in r0
      print_rm_int(LDC, 0, 1, 0);

      // L2: Store r0 into the int variable
      gen_store_int(curr_quad->arg1->value.var_node, 0);

      return;
    }
    case neq_floats_op:
    {
      // Load the left float argument into fr0
      gen_load_float(curr_quad->arg2->value.var_node, 0);

      // Load the right float argument into fr1
      gen_load_float(curr_quad->arg3->value.var_node, 1);

      // fr0 <- fr0 - fr1
      print_ro(SUBF, 0, 0, 1);

      // If fr0 != 0, jump to L1 (2 instructions)
      print_rm_int(JFNE, 0, 2, program_ctr_reg);

      // Put 0.0 in fr0 and jump to L2 (1 instruction)
      print_rm_float(LDFC, 0, 0.0, 0);
      print_rm_int(LDA, program_ctr_reg, 1, program_ctr_reg);

      // L1: Put 1.0 in fr0
      print_rm_int(LDC, 0, 1.0, 0);

      // L2: Store fr0 into the double variable
      gen_store_float(curr_quad->arg1->value.var_node, 0);

      return;
    }
    case bang_op:
    {
			// Load the expression into r0
      gen_load_int(curr_quad->arg2->value.var_node, 0);

      // If r0 == 0, jump to L1 (2 instructions)
      print_rm_int(JEQ, 0, 2, program_ctr_reg);

      // r0 <- 0 (false)
      print_rm_int(LDC, 0, 0, 0);

      // Jump to L2 (1 instruction)
      print_rm_int(LDA, program_ctr_reg, 1, program_ctr_reg);

      // L1: r0 <- 1 (true)
      print_rm_int(LDC, 0, 1, 0);

      // L2: Store r0 in the result variable
      gen_store_int(curr_quad->arg1->value.var_node, 0);

			return;
		}
    case int_neg_op:
    {
			// r0 <- int variable's value (arg2)
      gen_load_int(curr_quad->arg2->value.var_node, 0);

      // r1 <- -1
      print_rm_int(LDC, 1, -1, 0);

      // r0 <- r0 * r1 = int variable's value * -1
      print_ro(MUL, 0, 0, 1);

      // result variable (arg1) <- r0
      gen_store_int(curr_quad->arg1->value.var_node, 0);

			return;
		}
    case float_neg_op:
    {
      // fr0 <- double variable's value (arg2)
      gen_load_float(curr_quad->arg2->value.var_node, 0);

      // fr1 <- -1.0
      print_rm_float(LDFC, 1, -1.0, 0);

      // fr0 <- fr0 * fr1 = double variable's value * -1.0
      print_ro(MULF, 0, 0, 1);

      // result variable (arg1) <- fr0
      gen_store_float(curr_quad->arg1->value.var_node, 0);

      return;
		}
    case var_inc_op:
    {
			// r1 <- 1
      print_rm_int(LDC, 1, 1, 0);

      // r0 <- int variable's value
      gen_load_int(curr_quad->arg1->value.var_node, 0);

      // r0 <- r0 + r1 = int variable's value + 1
      print_ro(ADD, 0, 0, 1);

      // Store r0 into the int variable
      gen_store_int(curr_quad->arg1->value.var_node, 0);

			return;
		}
    case array_inc_op:
    {
      // Get the offset into the array in terms of number of elements and put it in r0
      gen_load_int(curr_quad->arg2->value.var_node, 0);

      // r1 <- 8
      print_rm_int(LDC, 1, 8, 0);

      // r0 <- element offset * 8 = byte offset into the array
      print_ro(MUL, 0, 0, 1);

      // Get the absolute address of the first element of the array and put it in r1
      symnode array_id_node = curr_quad->arg1->value.var_node;
      switch(array_id_node->mem_addr_type) {
          case off_fp:
              print_rm_int(LD, 1, array_id_node->var_addr, frame_ptr_reg);
              break;

          case global:
              print_rm_int(LD, 1, array_id_node->var_addr, global_ptr_reg);
              break;              
      }

      // Calculate r0 <- r0 + r1 to get the address of the destination bucket in the array
      print_ro(ADD, 0, 0, 1);

      // Load the original value of the array bucket into r1
      print_rm_int(LD, 1, 0, 0);

      // Increment the original value of the array bucket: r1 <- r1 + 1
      print_rm_int(LDA, 1, 1, 1);

      // Store r1 into the array bucket (address in r0)
      print_rm_int(ST, 1, 0, 0);

      return;
		}
    case var_dec_op:
    {
      // r1 <- -1
      print_rm_int(LDC, 1, -1, 0);

      // r0 <- int variable's value
      gen_load_int(curr_quad->arg1->value.var_node, 0);

      // r0 <- r0 + r1 = int variable's value + -1
      print_ro(ADD, 0, 0, 1);

      // Store r0 into the int variable
      gen_store_int(curr_quad->arg1->value.var_node, 0);

      return;
		}
    case array_dec_op:
    {
      // Get the offset into the array in terms of number of elements and put it in r0
      gen_load_int(curr_quad->arg2->value.var_node, 0);

      // r1 <- 8
      print_rm_int(LDC, 1, 8, 0);

      // r0 <- element offset * 8 = byte offset into the array
      print_ro(MUL, 0, 0, 1);

      // Get the absolute address of the first element of the array and put it in r1
      symnode array_id_node = curr_quad->arg1->value.var_node;
      switch(array_id_node->mem_addr_type) {
          case off_fp:
              print_rm_int(LD, 1, array_id_node->var_addr, frame_ptr_reg);
              break;

          case global:
              print_rm_int(LD, 1, array_id_node->var_addr, global_ptr_reg);
              break;              
      }

      // Calculate r0 <- r0 + r1 to get the address of the destination bucket in the array
      print_ro(ADD, 0, 0, 1);

      // Load the original value of the array bucket into r1
      print_rm_int(LD, 1, 0, 0);

      // Decrement the original value of the array bucket: r1 <- r1 + -1
      print_rm_int(LDA, 1, -1, 1);

      // Store r1 into the array bucket (address in r0)
      print_rm_int(ST, 1, 0, 0);

      return;
		}
    case if_false_op:
    {
      // Register the quad for backpatching in backpatch_jump_quads
      backpatch_jump_quads[quad_index] = assembly_index;

      // Increment assembly_index to make room:
      //   1 instruction for loading the expression and then 2 for conditionally jumping
      assembly_index += 3;

      return;
		}
    case if_true_op:
    {
      // Register the quad for backpatching in backpatch_jump_quads
      backpatch_jump_quads[quad_index] = assembly_index;

      // Increment assembly_index to make room:
      //   1 instruction for loading the expression and then 2 for conditionally jumping
      assembly_index += 3;

      return;
    }
    case goto_op:
    {
			// Register the quad for backpatching in backpatch_jump_quads
      backpatch_jump_quads[quad_index] = assembly_index;

      // Increment assembly_index to make room
      assembly_index++;

			return;
		}
    case read_int_op:
    {
      // r0 <- int from stdin
			print_ro(IN, 0, 0, 0);

      // int variable <- r0
      gen_store_int(curr_quad->arg1->value.var_node, 0);

			return;
		}
    case read_double_op:
    {
      // fr0 <- double from stdin
      print_ro(INF, 0, 0, 0);

      // double variable <- fr0
      gen_store_float(curr_quad->arg1->value.var_node, 0);

      return;
		}
    case func_decl_op:
    {
			// Save the assembly index of the first instruction of the function
      //   into the function's symnode
      symnode func_symnode = curr_quad->arg1->value.var_node;
      func_symnode->var_addr = assembly_index;

      // Make room for variables and temps
      symnode func_id_node = curr_quad->arg1->value.var_node;
      int num_vars_and_temps = func_id_node->num_vars + func_id_node->num_temps;
      print_rm_int(LDC, 0, num_vars_and_temps, 0); // r0 <- num_vars_and_temps
      print_rm_int(LDC, 1, -8, 0); // r1 <- -8
      print_ro(MUL, 0, 0, 1); // r0 <- num_vars_and_temps * -8
      print_ro(ADD, stack_ptr_reg, stack_ptr_reg, 0); // sp <- sp + (-8 * num_vars_and_temps)

			return;
		}
    case push_param_op:
    {
			// Make room for the parameter: sp <- sp + -8
      print_rm_int(LDA, stack_ptr_reg, -8, stack_ptr_reg);

      // Push the parameter
      symnode param_node = curr_quad->arg1->value.var_node;
      if (param_node->var_type == inttype) {
        // Load the value of the parameter into r0
        gen_load_int(param_node, 0);

        // Push r0 onto the stack
        print_rm_int(ST, 0, 0, stack_ptr_reg);
      } else if (param_node->var_type == doubletype) {
        // Load the value of the parameter into fr0
        gen_load_float(param_node, 0);

        // Push fr0 onto the stack
        print_rm_int(STF, 0, 0, stack_ptr_reg);
      }

			return;
		}
    case pop_params_op:
    {
      int pop_size = curr_quad->arg1->value.int_value * 8;

      // load 8 * num params into r0
      print_rm_int(LDC, 0, pop_size, 0);

      // add size of param array to sp
      print_ro(ADD, stack_ptr_reg, stack_ptr_reg, 0);
      return;
    }
    case alloc_array_op:
    {
			// r0 <- size of the array
      gen_load_int(curr_quad->arg2->value.var_node, 0);

      // r1 <- -8
      print_rm_int(LDC, 1, -8, 0);

      // r0 <- size of the array * -8
      print_ro(MUL, 0, 0, 1);

      // SP <- SP + (-8 * size of the array)
      print_ro(ADD, stack_ptr_reg, stack_ptr_reg, 0);

      // Get the absolute address of the array ID (pointer to the first element) and put it in r1
      symnode array_id_node = curr_quad->arg1->value.var_node;
      switch(array_id_node->mem_addr_type) {
          case off_fp:
              print_rm_int(LDA, 1, array_id_node->var_addr, frame_ptr_reg);
              break;

          case global:
              print_rm_int(LDA, 1, array_id_node->var_addr, global_ptr_reg);
              break;              
      }

      // Store the address of element 0 (equal to value of SP) into the array ID's address (in r1)
      print_rm_int(ST, stack_ptr_reg, 0, 1);

			return;
		}
    case return_op:
    {
      //Save value of return expression into r3 if it exists
      if (curr_quad->arg1 != NULL) {
        symnode retexp = curr_quad->arg1->value.var_node;
        switch(retexp->var_type) {
          case voidtype:
            //do nothing
            break;
          case no_type:
            //do nothing
            break;
          case inttype:
            //load int into r3
            gen_load_int(retexp, 3);
            break;
          case doubletype:
            //load double into r3
            gen_load_float(retexp, 3);
            break;
        }
      }

      // cleanup 1) sp = fp pops all locals and temps
      print_rm_int(LDA, stack_ptr_reg, 0, frame_ptr_reg);

      // cleanup 2) increment sp by 8 to pop ret addr and control link
      // sp <-- sp + 8
      print_rm_int(LDA, stack_ptr_reg, 8, stack_ptr_reg);

      // save ret addr in r2 (4(fp))
      print_rm_int(LD, 2, 4, frame_ptr_reg);

      // set fp to value at control link (0(fp)) 
      print_rm_int(LD, frame_ptr_reg, 0, frame_ptr_reg) ;

      //return control (set pc to r2)
      print_rm_int(LDA, program_ctr_reg, 0, 2);
			return;
		}
    case assign_int_literal:
    {
      // Load the int literal value into r0
      print_rm_int(LDC, 0, curr_quad->arg2->value.int_value, 0);

      // Store r0 into the int variable
      gen_store_int(curr_quad->arg1->value.var_node, 0);

      return;
		}
    case assign_double_literal:
    {
			// Load the double literal value into fr0
      print_rm_float(LDFC, 0, curr_quad->arg2->value.double_value, 0);

      // Store fr0 into the double variable
      gen_store_float(curr_quad->arg1->value.var_node, 0);

			return;
		}
    case initial_main_call:
    {
      // Push return address (instruction following function call) onto top of stack
      //   1.) decrement stack pointer by 4 bytes
      print_rm_int(LDA, stack_ptr_reg, -4, stack_ptr_reg);
      //   2.) r0 <- 5 + PC
      print_rm_int(LDA, 0, 5, program_ctr_reg);
      //   3.) push r0 (the return address) onto the stack
      print_rm_int(ST, 0, 0, stack_ptr_reg);

      // Push the current frame pointer
      //   1.) decrement stack pointer by 4 bytes
      print_rm_int(LDA, stack_ptr_reg, -4, stack_ptr_reg);
      //   2.) push the value of the FP register
      print_rm_int(ST, frame_ptr_reg, 0, stack_ptr_reg);

      // Set the FP register to point to the top of the stack
      print_rm_int(LDA, frame_ptr_reg, 0, stack_ptr_reg);

      // Make room for the control transfer instruction
      initial_main_call_ctrl_xfer_assembly_index = assembly_index;
      assembly_index++;

      // When main returns, halt
      print_ro(HALT, 0, 0, 0);

      return;
    }
    case store_string_op:
    {
      // Get the string symnode
      symnode string_node = curr_quad->arg1->value.var_node;

      // Save start address
      string_node->var_addr = constant_stack_ptr;

      // Print .STRING directive
      fprintf(file, "%s\t%d\t\"%s\"\n", ".STRING", constant_stack_ptr, string_node->name);

      // Increment constant ptr
      int length = strlen(string_node->name) + 1; // + 1 because it is null terminated
      constant_stack_ptr += length;

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
            print_rm_int(LD, dest_r, node->var_addr, frame_ptr_reg);
            break;

          case global:
            print_rm_int(LD, dest_r, node->var_addr, global_ptr_reg);
            break;            
    }
}

void gen_load_float(symnode node, int dest_r) {
    switch(node->mem_addr_type) {
        case off_fp:
            print_rm_int(LDF, dest_r, node->var_addr, frame_ptr_reg);
            break;

        case global:
            print_rm_int(LDF, dest_r, node->var_addr, global_ptr_reg);
            break;       
    }
}

void gen_store_int(symnode node, int source_r) {
    switch(node->mem_addr_type) {
        case off_fp:
            print_rm_int(ST, source_r, node->var_addr, frame_ptr_reg);
            break;

        case global:
            print_rm_int(ST, source_r, node->var_addr, global_ptr_reg);
            break;       
    }
}

void gen_store_float(symnode node, int source_r) {
    switch(node->mem_addr_type) {
        case off_fp:
            print_rm_int(STF, source_r, node->var_addr, frame_ptr_reg);
            break;

        case global:
          print_rm_int(STF, source_r, node->var_addr, global_ptr_reg);
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

// Method to print RM for int offsets
void print_rm_int(ass_op op, int dest_r, int offset, int r) {
    char* op_str = ass_op_str[op];
    fprintf(file, "%d:\t%s\t%d,%d(%d)\n", assembly_index, op_str, dest_r, offset, r);
    assembly_index++;
}

// Method to print RM for float offsets
void print_rm_float(ass_op op, int dest_r, float offset, int r) {
    char* op_str = ass_op_str[op];
    fprintf(file, "%d:\t%s\t%d,%f(%d)\n", assembly_index, op_str, dest_r, offset, r);
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
    fprintf(file, ") (line: %d)\n", curr_quad->line_num);
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
      fprintf(file, "ID: %s (addr: %d)", the_quad_arg->value.var_node->name, the_quad_arg->value.var_node->var_addr);
      break;
    default:
      fprintf(file, "arg type not specified");
      break;
  }
}

// Generate assembly instructions for the current quad, starting at
//   the assembly index indicated in backpatch_jump_quads[quad_index]
void insert_jump_assembly() {
  // The jump quad
  quad jump_quad = quad_array[quad_index];

  // The assembly index to which the jump instruction(s) should be written
  int jump_assembly_index = backpatch_jump_quads[quad_index];

  // assembly_index <- jump_assembly_index so we can use print_ro and print_rm_int
  assembly_index = jump_assembly_index;

  // The quad index of the destination of the jump
  int dest_quad_index;

  // The assembly index of the destinatino of the jump
  int dest_assembly_index;

  if (jump_quad->op == if_false_op)
  {
    // Load the value of the expression into r0 or fr0
    symnode exp_symnode = jump_quad->arg1->value.var_node;
    if (exp_symnode->var_type == inttype) {
      gen_load_int(exp_symnode, 0);
    } else if (exp_symnode->var_type == doubletype) {
      gen_load_float(exp_symnode, 0);
    }

    // Get the destination quad index
    dest_quad_index = jump_quad->arg2->value.int_value;

    // Get the destination assembly index
    dest_assembly_index = quad_assembly_index[dest_quad_index];

    // r1 <- 0
    print_rm_int(LDC, 1, 0, 0);

    // Generate the jump assembly instruction;
    //   op depends on type of expression
    if (exp_symnode->var_type == inttype) {
      print_rm_int(JEQ, 0, dest_assembly_index, 1);
    } else if (exp_symnode->var_type == doubletype) {
      print_rm_int(JFEQ, 0, dest_assembly_index, 1);
    }
  }
  else if (jump_quad->op == if_true_op)
  {
    // Load the value of the expression into r0 or fr0
    symnode exp_symnode = jump_quad->arg1->value.var_node;
    if (exp_symnode->var_type == inttype) {
      gen_load_int(exp_symnode, 0);
    } else if (exp_symnode->var_type == doubletype) {
      gen_load_float(exp_symnode, 0);
    }

    // Get the destination quad index
    dest_quad_index = jump_quad->arg2->value.int_value;

    // Get the destination assembly index
    dest_assembly_index = quad_assembly_index[dest_quad_index];

    // r1 <- 0
    print_rm_int(LDC, 1, 0, 0);

    // Generate the jump assembly instruction;
    //   op depends on type of expression
    if (exp_symnode->var_type == inttype) {
      print_rm_int(JNE, 0, dest_assembly_index, 1);
    } else if (exp_symnode->var_type == doubletype) {
      print_rm_int(JFNE, 0, dest_assembly_index, 1);
    }
  }
  else if (jump_quad->op == goto_op)
  {
    // Get the destination quad index
    dest_quad_index = jump_quad->arg1->value.int_value;

    // Get the destination assembly index
    dest_assembly_index = quad_assembly_index[dest_quad_index];

    // Generate the jump assembly instruction
    print_rm_int(LDC, program_ctr_reg, dest_assembly_index, 0);
  }
}

// TODO: String storage at 0 and printing
