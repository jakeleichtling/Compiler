CC = gcc
CFLAGS =

.SUFFIXES: .c

SRCS = symtab.c test_symtab.c
OBJS = $(SRCS:.c=.o)

.c.o:
	$(CC) -c $(CFLAGS) $<

symtab: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f $(OBJS) symtab

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
