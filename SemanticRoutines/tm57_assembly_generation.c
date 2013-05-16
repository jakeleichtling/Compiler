#include <stdlib.h>
#include <stdio.h>
#include "tm57_assembly_generation.h"

/* ~~~~~~~~~~~~~~~ Function Prototypes ~~~~~~~~~~~~~~~~~~~ */

// Generate the assembly instructions corresponding to the quad currently
//   being considered. If this quad is a jump, then set the corresponding
//   bucket of backpatch_jump_quads and increment assembly_index to make room.
void generate_quad_assembly();

/* ~~~~~~~~~~~~~~~~~~~~~ Variables ~~~~~~~~~~~~~~~~~~~~~~~~ */

// Declared in djcc.c; provided at the command line, or defaults
extern char *assembly_file_name;

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

/* ~~~~~~~~~~~~~~~ Function Definitions ~~~~~~~~~~~~~~~~~~~ */

// Iterate through the quad_array and generates assembly for each quad,
//   filling out the quad_assembly_lines array accordingly
void generate_program_assembly()
{
  quad_assembly_index = calloc(next_quad_index, sizeof(int));
  backpatch_jump_quads = calloc(next_quad_index, sizeof(int));
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


// Generate assembly for standard double binary ops
