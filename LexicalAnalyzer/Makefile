scanner:		lex.yy.o scanner.o
						gcc -o scanner lex.yy.o scanner.o -lfl

lex.yy.o:		lex.yy.c
						gcc -c lex.yy.c

lex.yy.c:		scanner.l
						flex scanner.l

scanner.o:	scanner.c
						gcc -c scanner.c

clean:
						rm ./*.o
						rm lex.yy.c
						rm scanner