# Name: Makefile
#
# Purpose: A program defining dependencies among the Parser's constituent code files, also providing
#					 commands of the form "make <command name>" to build the Parser and/or its constituent parts
# 				 or to clean the directory of compilation output.
#
# Derek Salama & Jake Leichtling
# CS57
# 4/19/2013

BISONFlAGS = -d -v
CFLAGS = 

parser:				  lex.yy.o bparser.tab.o ast.o cparser.o sst.o
								gcc $(CFLAGS) -o parser lex.yy.o bparser.tab.o ast.o cparser.o sst.o -lfl

valgrind:				lex.yy.c bparser.tab.c bparser.tab.h ast.c ast.h cparser.c sst.c sst.h
								gcc $(CFLAGS) -o parser lex.yy.c bparser.tab.c ast.c cparser.c sst.c -lfl
	
lex.yy.o:				lex.yy.c bparser.tab.h
								gcc $(CFLAGS) -c lex.yy.c
	
lex.yy.c:				flexer.l bparser.tab.h
								flex flexer.l
	
bparser.tab.o:	bparser.tab.c bparser.tab.h ast.h sst.h
								gcc $(CFLAGS) -c bparser.tab.c

bparser.tab.c:	bparser.y
								bison $(BISONFlAGS) bparser.y

bparser.tab.h:	bparser.y
								bison $(BISONFlAGS) bparser.y

ast.o:					ast.c ast.h
								gcc $(CFLAGS) -c ast.c

cparser.o:			cparser.c ast.h sst.h
								gcc $(CFLAGS) -c cparser.c

sst.o:					sst.c sst.h
								gcc $(CFLAGS) -c sst.c

sst_tester:			sst_tester.c sst.o
								gcc $(CFLAGS) -o sst_tester sst_tester.c sst.o

clean:					
								rm -f ./*.output
								rm -f parser
								rm -f cparser lex.yy.o lex.yy.c bparser.tab.o bparser.tab.c bparser.tab.h ast.o cparser.o
								rm -f sst_tester
								rm -f ./*.gch
								rm -f -r ./*.dSYM
