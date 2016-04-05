CC=gcc
CFLAGS=-Wall -g

all: main.o lexer.o parser.o lexeme_stack.o
	$(CC) $(CFLAGS) -o main main.o lexer.o parser.o lexeme_stack.o

main.o: main.c lexer.h parser.h
	$(CC) $(CFLAGS) -c main.c

lexer.o: lexer.c lexer.h
	$(CC) $(CFLAGS) -c lexer.c

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -c parser.c

lexeme_stack.o: lexeme_stack.c lexeme_stack.h
	$(CC) $(CFLAGS) -c lexeme_stack.c

clean:
	rm *.o main
