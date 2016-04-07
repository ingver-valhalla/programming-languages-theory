// parser.c

#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "lexeme_stack.h"
#include "symbol_table.h"

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

typedef struct {
	char  left;
	const char* right;
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
const char nonterminals[] = "PSLETM";
const char terminals[] = "(,)|&~ID";

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

int findRule(const char* s)
{
	int i = 0;
	for( ; i < rulesCount; ++i) {
		if(0 == strcmp(ruleSet[i].right, s)) {
			return i;
		}
	}
	return -1;
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

int handleLexeme(Lexeme lex);

int specialRollUp()
{
	Lexeme special = {'E'}, top = stackPop();
	while(top.type != 'E') {
		if(top.type == lexError) {
			return 0;
		}
		top = stackPop();
	}
	top = stackPop();
	/*printf("handling SPECIAL\n");*/
	if(!handleLexeme(special))
		return 0;
	return 1;
}

int rollUp(Lexeme lex)
{
#define maxRollLen 5
	// top of stack will be placed here
	// max rule length = 5
	char roll[5+1] = {0};
	int i = 4;
	int readyToRoll = 0;
	int ruleIndex = -1;
	Lexeme top;
	Lexeme rolled;
	RelationType lRel, rRel;

	/*printf("ROLLING UP:\n");*/
	while(1) {
		top = stackPop();
		if(top.type == lexError) {
			return 0;
		}

		/*printf("\npopped: ");*/
		/*printLexeme(top);*/
		/*putchar('\n');*/
		/*printStack();*/

		if(top.type == lexRelation) {
			if(top.rel == relPre || top.rel == relPB) {
				/*printf("pre-relation\n");*/
				readyToRoll = 1;
			}
		}
		else if(i < 0) {
			/*printf("roll is full\n");*/
			readyToRoll = 1;
		}
		else {
			roll[i--] = top.type;
		}
		/*printf("roll[] : %s\n", roll + i + 1);*/

		if(readyToRoll) {
			ruleIndex = findRule(roll + i + 1);
			if(ruleIndex < 0) {
				/*printf("no such rule\n");*/
				if(i < 0) {
					/*printf("ended roll up\n");*/
					return 0;
				}
				else {
					/*printf("\nCONTINUE\n");*/
					readyToRoll = 0;
					continue;
				}
			}
			else {
				rolled.type = ruleSet[ruleIndex].left;
				/*printf("rolled : "); printLexeme(rolled); putchar('\n');*/

				lRel = getRelation(stackPeek().type, rolled.type);

				/*printf("handle rolled `%c'>>>>>\n", rolled.type);*/
				if(!handleLexeme(rolled)) {
					return 0;
				}

				rRel = getRelation(rolled.type, lex.type);
				if(lRel == relPB && rRel == relPost) {
					/*printf("SUPER SPECIAL SITUATION!!!!\n");*/
					if(!specialRollUp()) {
						/*printf("FATAL ERROR IN SPECIAL SITUATION\n");*/
						return 0;
					}
				}

				/*printf("handle last `%c'>>>>>\n", lex.type);*/
				if(!handleLexeme(lex)) {
					return 0;
				}
				break;
			}
		}
	}
	return 1;
#undef maxRollLen
}

// processes a lexeme according to its type
int handleLexeme(Lexeme lex)
{
	Lexeme top = stackPeek();
	Lexeme rel = {lexRelation};
	if(top.type == lexError) {
		printf("handleLexeme: got lexError\n");
		return 0;
	}
	else {
		rel.rel = getRelation(top.type, lex.type);
		if(rel.rel == relNone) {
			/*printf("No such relation\n");*/
			fprintf(stderr, "Syntax error\n");
			return 0;
		}
		if(rel.rel == relPost) {
			printf("\nhandleLexeme: `%c' => ROLL UP\n", lex.type);

			if(!rollUp(lex)) {
				fprintf(stderr, "rollUp failed\n");
				fprintf(stderr, "Syntax error\n");
				return 0;
			}

		}
		else {
			printf("handleLexeme: `%c' => SHIFT\n", lex.type);

			stackPush(rel);
			stackPush(lex);
		}
	}
	printStack();
	/*printf("FINISHED HANDLING "); printLexeme(lex); printf("\n");*/
	/*printf("\n");*/
	return 1;
}


// gets lexemes one-by-one and makes syntactic analysis
void parse(FILE* fi)
{
	if(!initSymTable()) {
		fprintf(stderr, "parse: can't init symbol table\n");
		return;
	}
	if(!initStack()) {
		freeSymTable();
		fprintf(stderr, "parse: can't init stack\n");
		return;
	}

	Lexeme lex = { lexBorder };
	stackPush(lex);
	printf("Initial stack state: ");
	printStack();

	while(lexBorder != (lex = getLexeme(fi)).type) {
		if(lex.type == lexError || !handleLexeme(lex)) {
			fprintf(stderr, "\nError occured while parsing a file\n");
			break;
		}
		/*printStack();*/
	}
	if(lex.type == lexBorder && !handleLexeme(lex)) {
		fprintf(stderr, "\nError occured while parsing a file\n");
	}
	printf("Ended parsing\n");
	printStack();
	printSymTable();
	freeSymTable();
	freeStack();
	putchar('\n');
}
