# Name: Makefile
#
# Purpose: A program defining dependencies among the Symbol Table's constituent code files, also providing
#          commands of the form "make <command name>" to build the Symbol Table and/or its constituent parts
#          or to clean the directory of compilation output.
#
# Derek Salama & Jake Leichtling
# CS57
# 4/29/2013

CFLAGS =

symtab_test:	symtab_test.o symtab.o
	gcc $(CFLAGS) -o symtab_test symtab_test.o symtab.o

symtab_test.o:	symtab_test.c symtab.h
	gcc $(CFLAGS) -c symtab_test.c

symtab.o:	symtab.c symtab.h
	gcc $(CFLAGS) -c symtab.c

clean:
	rm -f ./*.o
	rm -f symtab_test