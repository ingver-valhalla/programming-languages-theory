// lexeme_stack.c

#include <stdlib.h>
#include <stdio.h>
#include "lexeme_stack.h"


#define lexStackChunk 10
Lexeme* lexStack = NULL;
int lexStackSize = 0;
int lexStackTop = 0;

int initLexStack()
{
	if(lexStack != NULL) {
		fprintf(stderr, "initLexStack: stack is already allocated\n");
		return 0;
	}
	lexStack = (Lexeme*) malloc(sizeof(Lexeme)*lexStackChunk);
	if(lexStack == NULL) {
		perror("malloc()");
		return 0;
	}
	lexStackSize = lexStackChunk;
	return 1;
}

int enlargeStack()
{
	if(lexStack == NULL) {
		fprintf(stderr, "enlargeStack: stack isn't allocated\n");
		return 0;
	}
	Lexeme* t = (Lexeme*) realloc(lexStack,
	                              sizeof(Lexeme)*(lexStackSize + lexStackChunk));
	if(t == NULL) {
		perror("realloc()");
		return 0;
	}
	lexStack = t;
	lexStackSize += lexStackChunk;
	return 1;
}

int stackPush(Lexeme lex)
{
	if(lexStack == NULL) {
		fprintf(stderr, "stackPush(): stack is not allocated\n");
	}
	if(lexStackTop >= lexStackSize) {
		if(!enlargeStack()) {
			fprintf(stderr, "stackPush(): failed to enlarge stack\n");
			return 0;
		}
	}
	lexStack[lexStackTop++] = lex;
	return 1;
}

Lexeme stackPop()
{
	Lexeme ret = stackPeek();
	--lexStackTop;
	return ret;
}

Lexeme stackPeek()
{
	Lexeme ret;
	if(lexStack == NULL) {
		fprintf(stderr, "stackPeek(): stack is not allocated\n");
		ret.type = lexError;
		return ret;
	}
	if(lexStackTop < 1) {
		fprintf(stderr, "stackPeek(): stack is empty\n");
		ret.type = lexError;
		return ret;
	}
	return lexStack[lexStackTop-1];
}

int freeStack()
{
	if(lexStack == NULL) {
		fprintf(stderr, "freeStack(): stack is not allocated\n");
		return 0;
	}
	free(lexStack);
	lexStack = NULL;
	lexStackSize = 0;
	lexStackTop = 0;
	return 1;
}

void printStack()
{
	int i = 0;
	/*printf("size: %d, bytes: %d, addr: %p\n", lexStackSize, bytes_allocated, lexStack);*/
	printf("STACK: ");
	for( ; i < lexStackTop; ++i) {
		printLexeme(lexStack[i]);
	}
	putchar('@');
	putchar('\n');
}
