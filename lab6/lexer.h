// lexer.h

#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

// boolean constants
typedef char BOOL;
#define TRUE (BOOL)1
#define FALSE (BOOL)0

extern const char* STRUE;
extern const char* SFALSE;

// type of lexemes
typedef enum {
	lexError  = '!',
	lexBorder = '#',
	lexIdent  = 'I',
	lexConst  = 'D',
	lexOr     = '|',
	lexAnd    = '&',
	lexNeg    = '~',
	lexLParen = '(',
	lexRParen = ')',
	lexComma  = ',',
	lexRelation = 'r'
} LexemeType;

// precedence relations
typedef enum {
	relNone  = ' ',
	relBasis = '=',
	relPre   = '<',
	relPB    = '+',
	relPost  = '>'
} RelationType;

typedef struct lexeme {
	LexemeType type;
	union {
		char* ident;      // identifier lexeme
		BOOL val;         // bool const lexeme
		RelationType rel; // precedence relation type
	};
} Lexeme;

Lexeme getLexeme(FILE* fi);
void printLexeme(Lexeme lex);

#endif // LEXER_H
