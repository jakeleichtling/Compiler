# Name: Makefile
#
# Purpose: A program defining dependencies among the Lexical Analyzer's constituent code files, also providing
#					 commands of the form "make <command name>" to build the Lexical Analyzer and/or its constituent parts
# 				 or to clean the directory of compilation output.
#
# Derek Salama & Jake Leichtling
# CS57
# 4/10/2013

clexer:			lex.yy.o clexer.o
						gcc -o clexer lex.yy.o clexer.o -lfl

lex.yy.o:		lex.yy.c tokens.h
						gcc -c lex.yy.c

lex.yy.c:		lexer.l
						flex lexer.l

clexer.o:		clexer.c tokens.h
						gcc -c clexer.c

clean:
						rm ./*.o
						rm lex.yy.c
						rm clexer