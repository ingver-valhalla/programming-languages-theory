// code_gen.c

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
	printf("=========================\n");
}

// the list of triads of intremediate code
Triad * listHead = NULL;
Triad * listTail = NULL;

#define numStackChunk 10
int* numStack = NULL;
int numStackCap = 0;
int numStackTop = 0;
int triadsCount = 0;

int initNumStack()
{
	if(numStack != NULL) {
		fprintf(stderr, "initNumStack: stack is already allocated\n");
		return 0;
	}
	numStack = (int*) malloc(sizeof(int) * numStackChunk);
	if(numStack == NULL) {
		perror("initNumStack: malloc()");
		return 0;
	}
	numStackCap = numStackChunk;
	return 1;
}

int enlargeNumStack()
{
	if(numStack == NULL) {
		fprintf(stderr, "enlargeNumStack: stack isn't allocated\n");
		return 0;
	}
	int* t = (int*) realloc(numStack,
	                        sizeof(int)*(numStackCap + numStackChunk));
	if(t == NULL) {
		perror("realloc()");
		return 0;
	}
	numStack = t;
	numStackCap += numStackChunk;
	return 1;
}

int numStackPush(int num)
{
	if(numStack == NULL) {
		fprintf(stderr, "numStackPush(): stack is not allocated\n");
	}
	if(numStackTop >= numStackCap) {
		if(!enlargeNumStack()) {
			fprintf(stderr, "numStackPush(): failed to enlarge stack\n");
			return 0;
		}
	}
	numStack[numStackTop++] = num;
	return 1;
}

int numStackPop()
{
	if(numStack == NULL) {
		fprintf(stderr, "stackPop(): stack is not allocated\n");
		return -1;
	}
	if(numStackTop < 1) {
		fprintf(stderr, "stackPop(): stack is empty\n");
		return -1;
	}
	return numStack[--numStackTop];
}

int freeNumStack()
{
	if(numStack == NULL) {
		fprintf(stderr, "freeStack(): stack is not allocated\n");
		return 0;
	}
	free(numStack);
	numStack = NULL;
	numStackCap = 0;
	numStackTop = 0;
	return 1;
}

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

int printTriad(Triad* tr)
{
	if( tr == NULL )
		return 0;

    printf("%4d:\t ", tr->number);
	printf("%c(", (char)tr->operation );
	if(!printTriadOp(tr->first)) {
		return 0;
	}
	printf(", " );
	if(!printTriadOp(tr->second)) {
		return 0;
	}
	printf(")\n" );
	return 1;
}

void setOperation(Triad* tr, Operations op)
{
	tr->operation = op;
}
int setOperand(TriadOp* op, TriadOpType t, void* val)
{
	op->type = t;
	switch(t) {
		case trVar:
			op->var = (char*) val;
			break;
		case trConst:
			op->constVal = *(BOOL*) val;
			break;
		case trPtr:
			/*op->ptr = *(int *)val;*/
			break;
		case trDummy:
			break;
		default:
			fprintf(stderr, "setOperand(): Wrong operand type\n");
			return 0;
	}
	return 1;
}

Triad* createTriad()
{
	Triad* p = (Triad *)calloc(sizeof(Triad), 1);
	if(p == NULL) {
		perror("createTriad: calloc()");
	}
	return p;
}

int appendTriad(Triad* tr)
{
	if(tr == NULL)
		return 0;
	if(numStack == NULL && !initNumStack())
		return 0;

	tr->next = NULL;
	if(listTail == NULL) {
		listHead = tr;
		listTail = tr;
	}
	else {
		listTail->next = tr;
		listTail = tr;
	}
	if(tr->second.type == trPtr) {
		tr->second.ptr = numStackPop();
		if(tr->second.ptr < 0) {
			return 0;
		}
	}
	if(tr->first.type == trPtr) {
		tr->first.ptr = numStackPop();
		if(tr->first.ptr < 0) {
			return 0;
		}
	}
	if(tr->operation != opAssign) {
		if(!numStackPush(triadsCount+1)) {
			return 0;
		}
	}
	tr->number = ++triadsCount;
	return 1;
}

void printTriadList()
{
	int i = 0;
	Triad* p = listHead;

	while(p != NULL) {
		++i;
		//printf("%4d:\t ", i);
		printTriad( p );
		p = p->next;
	}
}

void clearTriadList()
{
	while(listHead != NULL) {
		Triad *p = listHead;
		listHead = listHead->next;
		free(p);
	}
	listTail = NULL;
	freeNumStack();
}

int findTriad( int num, Triad ** pTriad, Triad ** pPrev )
{
	if( pTriad == NULL || pPrev == NULL ) {
		return 1;
	}
	*pTriad = listHead;
	while( *pTriad != NULL ) {
		if( (*pTriad)->number == num ) {
			return 1;
		}
		*pPrev = *pTriad;
		*pTriad= (*pTriad)->next;
	}
	fprintf( stderr, "Triad #%d not found", num );
	return 0;
}

int removeTriad( Triad ** pTriad, Triad * prev )
{
	if( pTriad == NULL || *pTriad == NULL )
	{
		return 0;
	}
	if( prev == NULL )
		listHead = (*pTriad)->next;
	else {
		prev->next = (*pTriad)->next;
	}
	free( *pTriad );
	*pTriad = NULL;
	return 1;
}

int optimize()
{
	Triad * p = listHead;
	Triad * q = NULL;
	Triad * qprev = NULL;
	TriadOp * toChange = NULL;
	BOOL val = FALSE;

	while( p != NULL ) {
		// First rule:
		// If the only operand of an unary operation or the right operand of a
		// binary operation (except assignment) is a pointer to a constant
		// loading triad, it's replaced by a constant itself and corresponding
		// triad is deleted.

		// checking arity of the operation
		// the only unary operation is opNot
		if( p->operation == opNot ) {
			toChange = &p->first;
		}
		else {
			toChange = &p->second;
		}
		// apply first rule
		if( toChange->type == trPtr && p->operation != opAssign ) {
			if(!findTriad( toChange->ptr, &q, &qprev )) {
				return 0;
			}
			if( q->operation == opConst ) {
				setOperand( toChange, trConst, &q->first.constVal );
				removeTriad( &q, qprev );
			}
		}


		// Second rule:
		// If the only operand of the unary operation is a constant, evaluate
		// the operation and replace the triad with constant loading.

		// apply second rule
		if( p->operation == opNot && toChange->type == trConst ) {
			setOperation( p, opConst );
			val = !toChange->constVal;
			setOperand( toChange, trConst, &val);
		}


		//  Third rule:
		//  If the first operand of a binary operation (except assignment) is a
		//  pointer on constant loading and the second operand is a constant,
		//  evaluate operation and replace the triad with loading of a result.
		//  Previous loading operator is deleted.

		// check arity
		if( p->operation != opNot && p->operation != opAssign ) {
			// apply third rule
			if( p->first.type == trPtr && p->second.type == trConst ) {
				findTriad( p->first.ptr, &q, &qprev );
				if( q->operation == opConst ) {
					if( p->operation == opOr ) {
						val = q->first.constVal | p->second.constVal;
					}
					else if( p->operation == opAnd ) {
						val = q->first.constVal & p->second.constVal;
					}
					else {
						fprintf( stderr, "optimize(): Invalid operation: %c", p->operation );
						return 0;
					}
					setOperation( p, opConst );
					setOperand( &p->first, trConst, &val );
					setOperand( &p->second, trDummy, NULL );
					removeTriad( &q, qprev );
				}
			}
		}


		// Fourth rule:
		// If the first operand of the assigment is pointer to variable triad,
		// it is replaced by the name of a variable. Variable triad is deleted

		// apply fourth rule
		if( p->operation == opAssign ) {
			findTriad( p->first.ptr, &q, &qprev );
			if( q->operation == opVar ) {
				setOperand( &p->first, trVar, q->first.var );
				removeTriad( &q, qprev );
			}
			else {
				fprintf( stderr, "optimize(): Fatal error. Only variables can be assigned to" );
				clearTriadList();
				return 0;
			}
		}
        p = p->next;
    }
    return 1;
}
