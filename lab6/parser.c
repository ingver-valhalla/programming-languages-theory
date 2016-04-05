// parser.c

#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "lexeme_stack.h"

/*
 *typedef short BOOL;
 *#define TRUE ((BOOL)1)
 *#define FALSE ((BOOL)0)
 */

/*
 * Grammar:
 * 
 * P ::= PS | S
 * S ::= (LE)
 * L ::= I,                 // identefier followed by comma
 * E ::= E"|"T | T
 * T ::= T&M | M
 * M ::= ~M | (E) | I | D
 * I ::= a | .. | z
 * D ::= true | false
 * 
 * I and D are considered terminals.
 * Lexer takes identifiers and constants as individual lexemes.
 */


// TODO: define a struct for lexeme
typedef LexemeType Lexeme;

// precedence relations
typedef enum {
	relNone  = ' ',
	relBasis = '=',
	relPre   = '<',
	relPost  = '>'
} RelationType;

/*
 *typedef enum {
 *    symR,
 *    symS,
 *    symL,
 *    symE,
 *    symT,
 *    symM,
 *    symOr,
 *    symAnd,
 *    symNot,
 *    symLParen,
 *    symRParen,
 *    symComma,
 *    symId,
 *    symConst
 *} Symbols;
 */

/*
 *typedef struct {
 *    char first;
 *    char second;
 *    RelationType type;
 *} Relation;
 */



/*
 *const char alphabet[] = "RSLETM|&~(),ID";
 *const int alphabetSize = sizeof(alphabet);
 *const char nonterminals[] = "PSLETM";
 *const char terminals[] = "|&~(),ID";
 */



const RelationType precMatrix[14][14] = {
	//  R   S   L   E   T   M   |   &   ~   (   )   ,   I   D
	 { ' ','=',' ',' ',' ',' ',' ',' ',' ','<',' ',' ',' ',' ' }, // R
	 { ' ','>',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' }, // S
	 { ' ',' ',' ','=','<','<',' ',' ','<','<',' ',' ','<','<' }, // L
	 { ' ',' ',' ',' ',' ',' ','=',' ',' ',' ','=',' ',' ',' ' }, // E
	 { ' ',' ',' ',' ',' ',' ','>','=',' ',' ','>',' ',' ',' ' }, // T
	 { ' ',' ',' ',' ',' ',' ','>','>',' ',' ','>',' ',' ',' ' }, // M
	 { ' ',' ',' ',' ','=','<',' ',' ','<','<',' ',' ','<','<' }, // |
	 { ' ',' ',' ',' ',' ','=',' ',' ','<','<',' ',' ','<','<' }, // &
	 { ' ',' ',' ',' ',' ','=',' ',' ','<','<',' ',' ','<','<' }, // ~
	 { ' ',' ','=','=','<','<',' ',' ','<','<',' ',' ','<','<' }, // (
	 { ' ','>',' ',' ',' ',' ','>','>',' ',' ','>',' ',' ',' ' }, // )
	 { ' ',' ',' ','>',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' }, // ,
	 { ' ',' ',' ',' ',' ',' ','>','>',' ',' ','>','=',' ',' ' }, // I
	 { ' ',' ',' ',' ',' ',' ','>','>',' ',' ','>',' ',' ',' ' }  // D
};


// processes a lexeme according to its type
void handleLexeme(Lexeme lex)
{

}


// gets lexemes one-by-one and makes syntactic analysis
void parse(FILE* fi)
{
	Lexeme lex;
	while(lexNone != (lex = getLexeme(fi))) {
		if(lex == lexError) {
			fprintf(stderr, "\nError occured while parsing a file\n");
			break;
		}
		putchar(lex);
		handleLexeme(lex);
	}

	putchar('\n');
}
