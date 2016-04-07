// symbol_table.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symbol_table.h"

#define symTableChunk 10
char** symTable = NULL;
int symTableCap = 0;    // capacity
int symTableSize = 0;   // count of symbols in table

int initSymTable()
{
	if(symTable != NULL) {
		fprintf(stderr, "initSymTable: symbol table already allocated\n");
		return 0;
	}

	if(NULL == (symTable = (char**) malloc(sizeof(char*)*symTableChunk))) {
		perror("initSymTable: malloc():");
		return 0;
	}
	symTableCap = symTableChunk;

	return 1;
}

int enlargeTable()
{
	if(symTable == NULL)
		return 0;

	char** t = (char**) realloc(symTable, sizeof(char*)*(symTableCap + symTableChunk));
	if(t == NULL) {
		perror("initSymTable: malloc():");
		return 0;
	}
	symTableCap += symTableChunk;

	return 1;
}

int addSymbol(char * symbol)
{
	if(symbol == NULL) {
		fprintf(stderr, "addSymbol: symbol table is not allocated\n");
		return 0;
	}

	int i = 0;
	for(; i < symTableSize; ++i) {
		if(0 == strcmp(symbol, symTable[i])) {
			/*printf("addSymbol: symbol `%s' already in table\n", symbol);*/
			return 1;
		}
	}
	if(symTableSize >= symTableCap) {
		if(!enlargeTable()) {
			fprintf(stderr, "addSymbol: can't resize symbol table\n");
			return 0;
		}
	}
	symTable[symTableSize++] = symbol;
	/*printf("addSymbol: symbol %s added\n", symbol);*/
	return 1;
}

void freeSymTable()
{
	int i = 0;
	if(symTable == NULL) {
		fprintf(stderr, "freeSymTable: symbol table isn't allocated\n");
	}
	else {
		for(i = 0; i < symTableSize; ++i) {
			if(symTable[i] == NULL) {
				fprintf(stderr, "symTable[%d] is not allocated\n", i);
			}
			else {
				free(symTable[i]);
			}
		}
		free(symTable);
	}
}

void printSymTable()
{
	if(symTable == NULL) {
		fprintf(stderr, "printSymTable: symbol table isn't allocated\n");
		return;
	}
	if(symTableSize == 0) {
		printf("Stack is empty\n");
		return;
	}
	int i = 0;
	printf("\n=====Symbol Table:=====\n");
	for( ; i < symTableSize; ++i) {
		printf("%d. %s\n", i, symTable[i]);
	}
	printf("=======================\n");
}
