#include <stdlib.h>
#include <stdio.h>
#include "tm57_assembly_generation.h"

/* ~~~~~~~~~~~~~~~ Function Prototypes ~~~~~~~~~~~~~~~~~~~ */

// Generate the assembly instructions corresponding to the quad currently
//   being considered. If this quad is a jump, then set the corresponding
//   bucket of backpatch_jump_quads and increment assembly_index to make room.
void generate_quad_assembly();

// Method to print RO instructions
// increments assebly line # after print
void print_ro(ass_op op, int dest_r, int r1, int r2);

// Method to print RM instructions
// increments assebly line # after print
void print_rm(ass_op op, int dest_r, int offset, int r);

// Save variable at node w/ value from source register
void gen_store_int(symnode node, int source_r);

// Load variable at node into destination register
void gen_load_int(symnode node, int dest_r);

/* ~~~~~~~~~~~~~~~~~~~~~ Variables ~~~~~~~~~~~~~~~~~~~~~~~~ */

// Declared in djcc.c; provided at the command line, or defaults
extern char *assembly_file_name;

// file stream for assembly printing
//TODO: open/close
FILE *fp;

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
int global_ptr;

const int stack_ptr_reg = 6;
const int frame_ptr_reg = 5;

/* ~~~~~~~~~~~~~~~ Function Definitions ~~~~~~~~~~~~~~~~~~~ */

// Iterate through the quad_array and generates assembly for each quad,
//   filling out the quad_assembly_lines array accordingly
void generate_program_assembly()
{
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

    generate_quad_assembly();
  }

  // TODO: Go back and patch quads that jump to an assembly instruction
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

    }
    case print_int_op:
    {

    }
    case print_float_op:
    {

    }
    case print_string_op:
    {

    }
    case int_to_float_op:
    {

    }
    case assn_int_to_var_op:
    {

    }
    case assn_int_to_arraysub_op:
    {

    }
    case assn_int_from_arraysub_op:
    {

    }
    case assn_float_to_var_op:
    {

    }
    case assn_float_to_arraysub_op:
    {

    }
    case assn_float_from_arraysub_op:
    {

    }
    case add_ints_op:
    {

    }
    case add_floats_op:
    {

    }
    case sub_ints_op:
    {

    }
    case sub_floats_op:
    {

    }
    case mult_ints_op:
    {

    }
    case mult_floats_op:
    {

    }
    case div_ints_op:
    {

    }
    case div_floats_op:
    {

    }
    case mod_op:
    {

    }
    case lt_ints_op:
    {

    }
    case lt_floats_op:
    {

    }
    case leq_ints_op:
    {

    }
    case leq_floats_op:
    {

    }
    case gt_ints_op:
    {

    }
    case gt_floats_op:
    {

    }
    case geq_ints_op:
    {

    }
    case geq_floats_op:
    {

    }
    case eq_ints_op:
    {

    }
    case eq_floats_op:
    {

    }
    case neq_ints_op:
    {

    }
    case neq_floats_op:
    {

    }
    case and_ints_op:
    {

    }
    case and_floats_op:
    {

    }
    case or_ints_op:
    {

    }
    case or_floats_op:
    {

    }
    case int_bang_op:
    {

    }
    case float_bang_op:
    {

    }
    case int_neg_op:
    {

    }
    case float_neg_op:
    {

    }
    case var_inc_op:
    {

    }
    case array_inc_op:
    {

    }
    case var_dec_op:
    {

    }
    case array_dec_op:
    {

    }
    case if_false_op:
    {

    }
    case goto_op:
    {

    }
    case read_int_op:
    {

    }
    case read_double_op:
    {

    }
    case halt_op:
    {

    }
    case func_decl_op:
    {

    }
    case push_param_op:
    {

    }
    case alloc_array_op:
    {

    }
    case return_op:
    {

    }
    case assign_int_literal:
    {

    }
    case assign_double_literal:
    {

    }
  }
}

// Generate assembly for standard int binary ops
void gen_standard_binary_op(ass_op op, symnode dest, symnode larg, symnode rarg) {
    //load larg into r0
    gen_load_int(larg, 0);

    //load rarg into r1
    gen_load_int(rarg, 1);

    //execute, store in r0
    print_ro(op, 0, 0, 1);

    //store in destination
    gen_store_int(dest, 0);
}

void gen_load_int(symnode node, int dest_r) {
    switch(node->mem_addr_type) {
        case off_fp:
            print_rm(LD, dest_r, node->var_addr, frame_ptr_reg);
            break;

        case global:
            print_rm(LD, dest_r, node->var_addr, global_ptr);
            break;

        case absolute:
            //put 0 in reg0
            print_rm(LDC, 0, 0, 0);

            //load offset(0) 
            print_rm(LD, dest_r, node->var_addr, 0);
            break;
    }
}

void gen_store_int(symnode node, int source_r) {
    switch(node->mem_addr_type) {
        case off_fp:
            print_rm(ST, source_r, node->var_addr, frame_ptr_reg);
            break;

        case global:
            print_rm(ST, source_r, node->var_addr, global_ptr);
            break;

        case absolute:
            //put 0 in reg0
            print_rm(LDC, 0, 0, 0);

            print_rm(ST, source_r, node->var_addr, 0);
            break;
    }
}



// Generate assembly for standard double binary ops

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
    char* opp_str = ass_op_str[op];
    fprintf(fp, "%d:   %s   %d,%d,%d\n", assembly_index++, opp_str, dest_r, r1, r2);
}

// Method to print RM
void print_rm(ass_op op, int dest_r, int offset, int r) {
    char* opp_str = ass_op_str[op];
    fprintf(fp, "%d:   %s   %d,%d(%d)\n", assembly_index++, opp_str, dest_r, offset, r);
}

// Make sure you clear the assembly file before writing into it (don't want to just append)
