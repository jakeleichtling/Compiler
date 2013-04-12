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

cparser:				lex.yy.o bparser.tab.o ast.o cparser.o
								gcc -o parser lex.yy.o bparser.tab.o ast.o cparser.o -lfl
	
lex.yy.o:				lex.yy.c
								gcc -c lex.yy.c
	
lex.yy.c:				flexer.l bparser.tab.h
								flex flexer.l
	
bparser.tab.o:	bparser.tab.c bparser.tab.h
								gcc -c bparser.tab.c

bparser.tab.c:	bparser.y
								bison $(BISONFlAGS) bparser.y

bparser.tab.h:	bparser.y
								bison $(BISONFlAGS) bparser.y

ast.o:					ast.c ast.h
								gcc -c ast.c ast.h

cparser.o:			cparser.c
								gcc -c cparser.c

clean:					
								rm -f ./*.output
								rm -f parser
								rm -f cparser lex.yy.o lex.yy.c bparser.tab.o bparser.tab.c bparser.tab.h ast.o cparser.o
