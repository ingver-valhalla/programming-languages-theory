// lexer.c

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

int c = ' ';

void get(FILE* fi)
{
	c = getc(fi);
}

int isLetter(int c)
{
	if(c >= 'a' && c <= 'z')
		return 1;

	return 0;
}

int isPunct(char c)
{
	return c == lexOr
	    || c == lexAnd
	    || c == lexNeg
	    || c == lexLParen
	    || c == lexRParen
	    || c == lexComma;
}

#define LEXEME_CHUNK 10

LexemeType readLexeme(FILE* fi)
{
	char* lexeme = (char*) malloc(LEXEME_CHUNK + 1);
	int lexemeSize = LEXEME_CHUNK;
	char* t = NULL;
	int curPos = 0;
	LexemeType lexType = 0;

	while(c != EOF) {
		if(!isLetter(c))
			break;
	
		if(curPos == lexemeSize) {
			t = (char*) realloc(lexeme, lexemeSize + LEXEME_CHUNK + 1);
			if(t == NULL) {
				perror("readLexeme: realloc():");
				free(lexeme);
				return lexError;
			}
			lexeme = t;
			lexemeSize += LEXEME_CHUNK;
		}
		lexeme[curPos++] = c;
		get(fi);
	}

	lexeme[curPos] = '\0';

	if(0 == strcmp("true", lexeme)
		|| 0 == strcmp("false", lexeme))
	{
		lexType = lexConst;
	}
	else {
		lexType = lexIdent;
	}

	free(lexeme);
	return lexType;
}

LexemeType getLexeme(FILE* fi)
{
	LexemeType lex = 0;

	while(isspace(c)) {
		get(fi);
	}

	if(isLetter(c)) {
		lex = readLexeme(fi);
	}
	else if(isPunct(c)) {
		lex = c;
		get(fi);
	}
	else if(c == EOF) {
		lex = lexNone;
	}
	else {
		fprintf(stderr, "\nForbidden character: `%c'\n", c);
		lex = lexError;
	}

	return lex;
}
