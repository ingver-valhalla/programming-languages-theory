// parser.c

#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "lexeme_stack.h"
#include "symbol_table.h"
#include "code_gen.h"

//#define DEBUG
#define OPTIMIZE

/*
 *typedef short BOOL;
 *#define TRUE ((BOOL)1)
 *#define FALSE ((BOOL)0)
 */

/*
 * Grammar:
 *
 * P ::= PS | S
 * S ::= (I,E)
 * E ::= E"|"T | T
 * T ::= T&M | M
 * M ::= ~M | (E) | I | D
 * I ::= IA | A
 * A ::= a | .. | z
 * D ::= true | false
 *
 * I and D are considered terminals.
 * Lexer takes identifiers and constants as individual lexemes.
 */

void parse(FILE* fi);
int handleLexeme(Lexeme lex);
int rollUp(Lexeme lex);
int specialRollUp();
int findRule(Lexeme* roll, int size);
int getIndex(char c);
RelationType getRelation(LexemeType l, LexemeType r);
int isTerminal(LexemeType t);
int isOperation(LexemeType t);
int evalExpression(Lexeme* arr, int size, BOOL* result);


typedef struct {
	char  left;
	char* right;
} Rule;

const Rule ruleSet[] = {
	{ 'P', "PS"    },
	{ 'P', "S"     },
	{ 'S', "(I,E)" },
	{ 'E', "E|T"   },
	{ 'E', "T"     },
	{ 'T', "T&M"   },
	{ 'T', "M"     },
	{ 'M', "~M"    },
	{ 'M', "(E)"   },
	{ 'M', "I"     },
	{ 'M', "D"     }
};

const int rulesCount = sizeof(ruleSet) / sizeof(Rule);

const char alphabet[] = "#PSETM(,)|&~ID";
const int alphabetSize = sizeof(alphabet) / sizeof(char);
/*const char nonterminals[] = ;*/
/*const char terminals[] = ;*/

const RelationType precMatrix[14][14] = {
	//  #   P   S   E   T   M   (   ,   )   |   &   ~   I   D
	 { ' ','+','<',' ',' ',' ','<',' ',' ',' ',' ',' ',' ',' ' }, // #
	 { '=',' ','=',' ',' ',' ','<',' ',' ',' ',' ',' ',' ',' ' }, // P
	 { '>',' ',' ',' ',' ',' ','>',' ',' ',' ',' ',' ',' ',' ' }, // S
	 { ' ',' ',' ',' ',' ',' ',' ',' ','=','=',' ',' ',' ',' ' }, // E
	 { ' ',' ',' ',' ',' ',' ',' ',' ','>','>','=',' ',' ',' ' }, // T
	 { ' ',' ',' ',' ',' ',' ',' ',' ','>','>','>',' ',' ',' ' }, // M
	 { ' ',' ',' ','+','<','<','<',' ',' ',' ',' ','<','+','<' }, // (
	 { ' ',' ',' ','+','<','<','<',' ',' ',' ',' ','<','<','<' }, // ,
	 { '>',' ',' ',' ',' ',' ','>',' ','>','>','>',' ',' ',' ' }, // )
	 { ' ',' ',' ',' ','+','<','<',' ',' ',' ',' ','<','<','<' }, // |
	 { ' ',' ',' ',' ',' ','=','<',' ',' ',' ',' ','<','<','<' }, // &
	 { ' ',' ',' ',' ',' ','=','<',' ',' ',' ',' ','<','<','<' }, // ~
	 { ' ',' ',' ',' ',' ',' ',' ','=','>','>','>',' ',' ',' ' }, // I
	 { ' ',' ',' ',' ',' ',' ',' ',' ','>','>','>',' ',' ',' ' }, // D
};

int findRule(Lexeme* roll, int size)
{
	int ruleIndex = 0;
	int i = 0;
	char* p = NULL;

	for( ; ruleIndex < rulesCount; ++ruleIndex) {


#ifdef DEBUG
		printf("checking rule %s\n", ruleSet[ruleIndex].right);
#endif


//		for( i = size - 1, p = ruleSet[ruleIndex].right; i >= 0 && *p; --i, ++p ) {
//			if(roll[i].type != *p) {
//				break;
//			}
//		}
		for( i = 0, p = ruleSet[ruleIndex].right; i < size && *p; ++i, ++p ) {
			if(roll[i].type != *p) {
				break;
			}
		}
		if(!*p && i >= size) {
			return ruleIndex;
		}
	}
	return -1;
}

int evalExpression(Lexeme* roll, int size, BOOL* result)
{
	BOOL* varValue;
	Triad* tr = createTriad();
	// assignment
	if(size == 5) {
		if(!addVar(roll[1].ident, roll[3].val)) {
			free(tr);
			return 0;
		}

		/*printVarTable();*/
		setOperation(tr, opAssign);
		setOperand(&tr->first, trPtr, NULL);
		setOperand(&tr->second, trPtr, NULL);
		appendTriad(tr);
		*result = roll[1].val;
	}
	// binary operation
	else if(size == 3) {
		if(roll[1].type == '|') {
			*result = roll[0].val | roll[2].val;
			setOperation(tr, opOr);
		}
		else if(roll[1].type == '&') {
			*result = roll[0].val & roll[2].val;
			setOperation(tr, opAnd);
		}
		// for M ::= (E)
		else {
			*result = roll[1].val;
			free(tr);
			return 1;
		}
		setOperand(&tr->first, trPtr, NULL);
		setOperand(&tr->second, trPtr, NULL);
		appendTriad(tr);
	}
	// unary operation ~
	else if(size == 2) {
		if(roll[1].type == '~') {
			*result = !roll[0].val;
			setOperation(tr, opNot);
			setOperand(&tr->first, trPtr, NULL);
			appendTriad(tr);
		}
		// for P ::= PS
		else {
			free(tr);
			*result = roll[0].val;
		}
	}
	else if(size == 1) {
		if(roll[0].type == lexIdent) {
			varValue = getVariable(roll[0].ident);
			if(varValue == NULL) {
				fprintf(stderr, "var `%s' is not defined\n", roll[0].ident);
				free(tr);
				return 0;
			}
			setOperation(tr, opVar);
			setOperand(&tr->first, trVar, roll[0].ident);
			appendTriad(tr);
			*result = *varValue;
		}
		else if(roll[0].type == lexConst) {
			setOperation(tr, opConst);
			setOperand(&tr->first, trConst, &roll[0].val);
			appendTriad(tr);
			*result = roll[0].val;
		}
		// for E ::= T, T ::= M, ...
		else {
			*result = roll[0].val;
		}
	}
	return 1;
}

int getIndex(char c)
{
	int i = 0;
	for( ; i < alphabetSize; ++i) {
		if(alphabet[i] == c)
			return i;
	}
	return -1;
}

RelationType getRelation(LexemeType l, LexemeType r)
{
	int li = getIndex(l);
	if(li < 0) {
		return relNone;
	}
	int ri = getIndex(r);
	if(ri < 0) {
		return relNone;
	}
	return precMatrix[li][ri];
}

int specialRollUp(Lexeme T)
{
	Lexeme special = {'E'}, top = stackPop();
	Triad* orTriad = createTriad();
	while(top.type != 'E') {
		if(top.type == lexError) {
			free(orTriad);
			return 0;
		}
		top = stackPop();
	}
	special.val = top.val | T.val;
	top = stackPop();


#ifdef DEBUG
	printf("handling SPECIAL\n");
#endif


	if(!handleLexeme(special)) {
		free(orTriad);
		return 0;
	}
	setOperation(orTriad, opOr);
	setOperand(&orTriad->first, trPtr, NULL);
	setOperand(&orTriad->second, trPtr, NULL);
	appendTriad(orTriad);
	return 1;
}

int rollUp(Lexeme lex)
{
#define maxRollSize 5
	// top of stack will be placed here
	// max rule length = 5
	Lexeme roll[maxRollSize];
	int rollSize = 0;
	BOOL rollValue = FALSE;

	int readyToRoll = 0;
	int preRelMet = 0;

	int ruleIndex = -1;

	Lexeme top;
	Lexeme rolled;
	RelationType lRel, rRel;
	int i = 0;

    const Lexeme* stack = getStackData();
    if(stack == NULL) {
#ifdef DEBUG
        printf("rollUp: Stack is not allocated\n");
#endif
        return 0;
    }
    int stackSize = getStackSize();
    if(stackSize == 0) {
#ifdef DEBUG
        printf("rollUp: Stack is not allocated\n");
#endif
        return 0;
    }
    int stackIter = 0;

#ifdef DEBUG
	printf("ROLLING UP:\n");
#endif

    printStack();
    while(stackIter < stackSize) {
        // searching for '<' of '+' relation
        if(stack[stackIter].type == lexRelation) {
            if(stack[stackIter].rel == relPre
               || stack[stackIter].rel == relPB)
            {
                int it = stackIter + 1;
                // get lexemes from stack
                while(it < stackSize && rollSize < maxRollSize) {
                    if(stack[it].type != lexRelation) {
                        roll[rollSize++] = stack[it];
                    }
                    ++it;
                }

#ifdef DEBUG
                printf("roll[] :");
                for(i = 0; i < rollSize; ++i) {
                    printLexeme(roll[i]);
                }
                printf("\n");
                fflush(stdout);
#endif

                ruleIndex = findRule(roll, rollSize);
                if(ruleIndex < 0) {
#ifdef DEBUG
                    printf("no such rule\n");
                    fflush(stdout);
#endif
                    rollSize = 0;
                }
                else {
#ifdef DEBUG
                    printf("found rule: %c -> %s\n",
                           ruleSet[ruleIndex].left,
                           ruleSet[ruleIndex].right);
                    fflush(stdout);
#endif
                    if(!evalExpression(roll, rollSize, &rollValue)) {
#ifdef DEBUG
                        printf("Can't evaluate expression\n");
                        fflush(stdout);
#endif
                        return 0;
                    }

                    rolled.type = ruleSet[ruleIndex].left;
                    rolled.val = rollValue;
#ifdef DEBUG
                    printf("rolled : "); printLexeme(rolled); putchar('\n');
                    fflush(stdout);
#endif
                    while(--it >= stackIter)
                        stackPop();
#ifdef DEBUG
                    printStack();
#endif

                    if(!handleLexeme(rolled)) {
                        return 0;
                    }
                    if(!handleLexeme(lex)) {
#ifdef DEBUG
                        printf("rollUp: can't handle last\n");
                        fflush(stdout);
#endif
                        return 0;
                    }
                    return 1;
                }

            }
        }
        ++stackIter;
    }

	return 0;
#undef maxRollSize
}

// processes a lexeme according to its type
int handleLexeme(Lexeme lex)
{
	Lexeme top = stackPeek();
	Lexeme rel = {lexRelation};
	if(top.type == lexError) {


#ifdef DEBUG
		printf("handleLexeme: got lexError\n");
#endif


		return 0;
	}
	else {
		rel.rel = getRelation(top.type, lex.type);
		if(rel.rel == relNone) {


#ifdef DEBUG
			printf("handleLexeme: No such relation\n");
#endif


			fprintf(stderr, "Syntax error\n");
			return 0;
		}
		if(rel.rel == relPost) {


#ifdef DEBUG
			printf("\nhandleLexeme: `%c' => ROLL UP\n", lex.type);
#endif


			if(!rollUp(lex)) {


#ifdef DEBUG
				printf("rollUp failed\n");
//				printf("Syntax error\n");
#endif


				return 0;
			}

		}
		else {


#ifdef DEBUG
			printf("handleLexeme: `%c' => SHIFT\n", lex.type);
#endif


			if(lex.type == lexComma) {
				Triad* varLoad = createTriad();
				setOperation(varLoad, opVar);
				setOperand(&varLoad->first, trVar, stackPeek().ident);
				appendTriad(varLoad);
			}
			if(!stackPush(rel)) {
				return 0;
			}
			if(!stackPush(lex)) {
				return 0;
			}
		}
	}


#ifdef DEBUG
	printStack();
	printf("FINISHED HANDLING "); printLexeme(lex); printf("\n");
	printf("\n");
#endif


	return 1;
}

int init()
{
	if(!initSymTable()) {
		fprintf(stderr, "init: can't init symbol table\n");
		return 0;
	}
	if(!initLexStack()) {
		freeSymTable();
		fprintf(stderr, "init: can't init stack\n");
		return 0;
	}
	if(!initVarTable()) {
		freeSymTable();
		freeStack();
		fprintf(stderr, "init: can't init var table\n");
	}
	return 1;
}

void terminate()
{
	freeSymTable();
	freeStack();
	freeVarTable();
}

// gets lexemes one-by-one and makes syntactic analysis
void parse(FILE* fi)
{
	if(!init())
		return;

	Lexeme lex = { lexBorder };
	if(!stackPush(lex)) {
		terminate();
		return;
	}


#ifdef DEBUG
	printf("Initial stack state: ");
	printStack();
#endif


	while(lexBorder != (lex = getLexeme(fi)).type) {
		if(lex.type == lexError || !handleLexeme(lex)) {
			fprintf(stderr, "\nError occured while parsing a file\n");
			terminate();
			return;
		}


#ifdef DEBUG
		printStack();
#endif


	}
	if(lex.type == lexBorder && !handleLexeme(lex)) {
		fprintf(stderr, "\nError occured while parsing a file\n");
	}


#ifdef DEBUG
	printf("Ended parsing\n");
	printStack();
	printSymTable();
	printVarTable();
	putchar('\n');
#endif


	printTriadList();
	putchar('\n');

#ifdef OPTIMIZE
	printf("\n\n// Optimized:\n");
	if(!optimize()) {
	    printf("Failed optimization\n");
	    terminate();
        return;
	}
	printTriadList();
	putchar('\n');
#endif

	terminate();
}
