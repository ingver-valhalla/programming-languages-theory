// lexer.h

#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

// type of lexemes
typedef enum {
	lexError  = 0,
	lexNone   = EOF,
	lexIdent  = 'I',
	lexConst  = 'D',
	lexOr     = '|',
	lexAnd    = '&',
	lexNeg    = '~',
	lexLParen = '(',
	lexRParen = ')',
	lexComma  = ','
} LexemeType;

LexemeType getLexeme(FILE* fi);

#endif // LEXER_H
