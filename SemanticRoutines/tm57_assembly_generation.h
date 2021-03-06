/* tm57_assembly_generation.h
 * 
 * Declarations for generating assembly code from
 * intermediate code.
 *
 * Jake Leichtling & Derek Salama
 * 5/29/2013
 */

#ifndef TM57_ASSEMBLY_GENERATION_H_
#define TM57_ASSEMBLY_GENERATION_H_

// Iterate through the quad_array and generates assembly for each quad,
//   filling out the quad_assembly_lines array accordingly
void generate_program_assembly();

typedef enum {
	HALT,
	IN,
	OUT,
	ADD,
	SUB,
	MUL,
	DIV,
	ADDF,
	SUBF,
	MULF,
	DIVF,
	MOD,
	CVTIF,
	CVTFI,
	LD,
	LDA,
	LDC,
	ST,
	JLT,
	JLE,
	JGE,
	JGT,
	JEQ,
	JNE,
	LDF,
	STF,
	JFLT,
	JFLE,
	JFGE,
	JFGT,
	JFEQ,
	JFNE,
	INF,
	OUTF,
	LDFC,
	LDB,
	STB,
	INB,
	OUTB

} ass_op;

#endif