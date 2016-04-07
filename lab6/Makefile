CC=gcc
CFLAGS=-Wall -g

main: main.o lexer.o parser.o lexeme_stack.o symbol_table.o
	$(CC) $(CFLAGS) -o main main.o lexer.o parser.o lexeme_stack.o symbol_table.o

main.o: main.c lexer.h parser.h
	$(CC) $(CFLAGS) -c main.c

symbol_table.o: symbol_table.c symbol_table.h
	$(CC) $(CFLAGS) -c symbol_table.c

lexer.o: lexer.c lexer.h symbol_table.h
	$(CC) $(CFLAGS) -c lexer.c

parser.o: parser.c parser.h lexer.h lexeme_stack.h symbol_table.h
	$(CC) $(CFLAGS) -c parser.c

lexeme_stack.o: lexeme_stack.c lexeme_stack.h
	$(CC) $(CFLAGS) -c lexeme_stack.c

clean:
	rm *.o main
