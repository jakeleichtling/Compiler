lexer:		lex.yy.o lexer.o
						gcc -o lexer lex.yy.o lexer.o -lfl

lex.yy.o:		lex.yy.c tokens.h
						gcc -c lex.yy.c

lex.yy.c:		lexer.l
						flex lexer.l

lexer.o:	lexer.c tokens.h
						gcc -c lexer.c

clean:
						rm ./*.o
						rm lex.yy.c
						rm lexer