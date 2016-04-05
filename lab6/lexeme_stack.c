// lexeme_stack.c

#include <stdlib.h>
#include <stdio.h>
#include "lexeme_stack.h"


#define lexStackChunk 50
LexemeType* lexStack = NULL;
int lexStackSize = 0;
int lexStackTop = -1;

int initStack()
{
	lexStack = (LexemeType*) malloc(sizeof(LexemeType)*lexStackChunk);
	if(lexStack == NULL) {
		perror("malloc()");
		return 0;
	}
	lexStackSize = lexStackChunk;
	return 1;
}

int enlargeStack()
{
	lexStack = (LexemeType*) realloc(lexStack,
	                                 sizeof(LexemeType) * (lexStackSize + lexStackChunk));
	if(lexStack == NULL) {
		perror("realloc()");
		return 0;
	}
	lexStackSize += lexStackChunk;
	return 1;
}

void stackPush(LexemeType lex)
{
	if(lexStack == NULL) {
		fprintf(stderr, "stackPush(): stack is not allocated\n");
	}
	if(lexStackTop >= lexStackSize) {
		if(!enlargeStack()) {
			fprintf(stderr, "stackPush(): failed to enlarge stack\n");
			return;
		}
	}
	lexStack[++lexStackTop] = lex;
}

LexemeType stackPop()
{
	if(lexStack == NULL) {
		fprintf(stderr, "stackPop(): stack is not allocated\n");
	}
	if(lexStackTop < 0) {
		fprintf(stderr, "stackPop(): stack is empty\n");
	}
	return lexStack[lexStackTop--];
}

int freeStack()
{
	if(lexStack == NULL) {
		fprintf(stderr, "destroyStack(): stack is not allocated\n");
		return 0;
	}
	free(lexStack);
	return 1;
}
