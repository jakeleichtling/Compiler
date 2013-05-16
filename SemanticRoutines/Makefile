# Name: Makefile
#
# Purpose: A program defining dependencies among the Compilers's constituent code files, also providing
#					 commands of the form "make <command name>" to build the Compiler and/or its constituent parts
# 				 or to clean the directory of compilation output.
#
# Derek Salama & Jake Leichtling
# CS57
# 5/29/2013

BISONFlAGS = -d -v
CFLAGS = -g -Wall

djcc:	djcc.o lex.yy.o bparser.tab.o ast.o symtab.o quad.o ast_node_processing.o tm57_assembly_generation.o
	gcc $(CFLAGS) -o djcc djcc.o lex.yy.o bparser.tab.o ast.o symtab.o quad.o ast_node_processing.o tm57_assembly_generation.o -lfl

valgrind:	djcc.c lex.yy.c bparser.tab.c bparser.tab.h ast.c ast.h symtab.c symtab.h quad.c quad.h ast_node_processing.c ast_node_processing.h
	gcc $(CFLAGS) -o djcc djcc.c lex.yy.c bparser.tab.c ast.c symtab.c quad.c ast_node_processing.c -lfl

djcc.o:	djcc.c bparser.tab.h ast.h symtab.h quad.h
	gcc $(CFLAGS) -c djcc.c

lex.yy.o:	lex.yy.c bparser.tab.h
	gcc -g -c lex.yy.c

lex.yy.c:	flexer.l bparser.tab.h
	flex flexer.l

bparser.tab.o:	bparser.tab.c bparser.tab.h	ast.h
	gcc $(CFLAGS) -c bparser.tab.c

bparser.tab.c:	bparser.y
	bison $(BISONFlAGS) bparser.y

bparser.tab.h:	bparser.y
	bison $(BISONFlAGS) bparser.y

ast.o:	ast.c ast.h
	gcc $(CFLAGS) -c ast.c

symtab.o:	symtab.c symtab.h
	gcc $(CFLAGS) -c symtab.c

quad.o:	quad.c quad.h
	gcc $(CFLAGS) -c quad.c

ast_node_processing.o:	ast_node_processing.c ast_node_processing.h
	gcc $(CFLAGS) -c ast_node_processing.c

tm57_assembly_generation.o: tm57_assembly_generation.c tm57_assembly_generation.h
	gcc $(CFLAGS) -c tm57_assembly_generation.c

new: clean djcc

clean:
	rm -f djcc
	rm -f ./*.output
	rm -f ./*.o
	rm -f lex.yy.c bparser.tab.c bparser.tab.h
	rm -f ./*.gch
	rm -f -r ./*.dSYM