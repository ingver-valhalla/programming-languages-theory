// code_gen.h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "code_gen.h"

#define varTableChunk 5
// the list of variables
Var* vars;
int varCount = 0;
int varTableCap = 0;

int initVarTable()
{
	if(vars != NULL) {
		fprintf(stderr, "initVarTable: var table is already allocated\n");
		return 0;
	}
	vars = (Var*) malloc(sizeof(Var) * varTableChunk);
	if(vars == NULL) {
		perror("initVarTable: malloc()");
		return 0;
	}
	varTableCap = varTableChunk;
	
	return 1;
}

int enlargeVarTable()
{
	if(vars == NULL) {
		fprintf(stderr, "enlargeVarTable: var table isn't allocated\n");
		return 0;
	}
	Var* t = (Var*) realloc(vars, sizeof(Var) * (varTableCap + varTableChunk));
	if(t == NULL) {
		perror("enlargeVarTable: realloc()");
		return 0;
	}
	vars = t;
	varTableCap += varTableChunk;
	
	return 1;
}

int addVar(char* name, BOOL val)
{
	if(vars == NULL) {
		fprintf(stderr, "addVar: var table isn't allocated\n");
		return 0;
	}
	int i = 0;
	Var var;
	if(name == NULL) {
		return 0;
	}
	for( ; i < varCount; ++i) {
		if(0 == strcmp(vars[i].name, name)) {
			vars[i].val = val;
			return 1;
		}
	}
	if(varCount >= varTableCap && !enlargeVarTable()) {
		return 0;
	}
	var.name = name;
	var.val = val;
	vars[varCount++] = var;

	return 1;
}


BOOL* getVariable(const char* name)
{
	if(vars == NULL) {
		fprintf(stderr, "addVar: var table isn't allocated\n");
		return NULL;
	}
	int i = 0;
	for( ; i < varCount; ++i) {
		if(0 == strcmp(vars[i].name, name)) {
			return &vars[i].val;
		}
	}
	return NULL;
}

void freeVarTable()
{
	if(vars == NULL)
		return;
	free(vars);
}

void printVarTable()
{
	int i = 0;
	printf("\n=====Variables Table=====\n");
	for( ; i < varCount; ++i) {
		printf("%s = %s\n", vars[i].name, vars[i].val ? "true" : "false");
	}
	printf("\n=========================\n");
}

// the list of triads of intremediate code
Triad * ListHead = NULL;
Triad * ListTail = NULL;
int triads_count = 0;


int printTriadOp(TriadOp op)
{
	switch(op.type) {
		case trVar:
			printf("%s", op.var);
			break;
		case trConst:
			printf("%s", op.constVal == TRUE ? "true" : "false");
			break;
		case trPtr:
			printf("^%d", op.ptr);
			break;
		case trDummy:
			printf("@");
			break;
		default:
			fprintf(stderr, "printTriadOp(): Wrong operand type\n");
			return 0;
	}
	return 1;
}

int print_triad(const Triad* tr)
{
	if( tr == NULL )
		return 0;
	
	printf("%c(", (char)tr->operation );
	if(!printTriadOp(tr->first)) {
		return 0;
	}
	printf(", " );
	if(!printTriadOp( tr->second )) {
		return 0;
	}
	printf(")\n" );
	return 1;
}

void setOperation(Triad* tr,Operations op)
{
	tr->operation = op;
}
int set_operand(TriadOp* op, TriadOpType t, void* val)
{
	op->type = t;
	switch(t) {
		case trVar:
			op->var = (char *)val;
			break;
		case trConst:
			op->constVal = *(BOOL *)val;
			break;
		case trPtr:
			op->ptr = *(int *)val;
			break;
		case trDummy:
			break;
		default:
			fprintf(stderr, "setOperand(): Wrong operand type\n");
			return 0;
	}
	return 1;
}

Triad* create_triad()
{
	Triad* p = (Triad *)calloc(sizeof(Triad), 1);
	if(p == NULL) {
		perror("createTriad: calloc()");
	}
	return p;
}

int append_triad(Triad* tr)
{
	if(tr == NULL)
		return 0;
	
	tr->next = NULL;
	if(ListTail == NULL) {
		ListHead = tr;
		ListTail = tr;
	}
	else {
		ListTail->next = tr;
		ListTail = tr;
	}
	return 1;
}

void print_list()
{
	int i = 0;
	Triad* p = ListHead;
	
	while(p != NULL) {
		++i;
		printf("%2d:\t ", i);
		print_triad( p );
		p = p->next;
	}
}

void clear_list()
{
	while(ListHead != NULL) {
		Triad *p = ListHead;
		ListHead = ListHead->next;
		free(p);
	}
	ListTail = NULL;
}
