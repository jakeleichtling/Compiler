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
	
lex.yy.o:				lex.yy.c ../LexicalAnalyzer/tokens.h
								gcc -c lex.yy.c
	
lex.yy.c:				../LexicalAnalyzer/lexer.l
								flex ../LexicalAnalyzer/lexer.l
	
bparser.tab.o:	bparser.tab.c
								gcc -c bparser.tab.c

bparser.tab.c:	bparser.y
								bison $(BISONFlAGS) bparser.y

bparser.tab.h:	bparser.y
								bison $(BISONFlAGS) bparser.y

ast.o:					ast.c
								gcc -c ast.c

cparser.o:			cparser.c
								gcc -c cparser.c

clean:					
								rm ./*.output
								rm cparser lex.yy.o lex.yy.c bparser.tab.o bparser.tab.c bparser.tab.h ast.o cparser.o
