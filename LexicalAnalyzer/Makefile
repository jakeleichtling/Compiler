clexer:		lex.yy.o clexer.o
						gcc -o clexer lex.yy.o clexer.o -lfl

lex.yy.o:		lex.yy.c tokens.h
						gcc -c lex.yy.c

lex.yy.c:		lexer.l
						flex lexer.l

clexer.o:	clexer.c tokens.h
						gcc -c clexer.c

clean:
						rm ./*.o
						rm lex.yy.c
						rm clexer