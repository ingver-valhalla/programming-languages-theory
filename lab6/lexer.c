// lexer.c

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "symbol_table.h"

const char* STRUE = "true";
const char* SFALSE = "false";

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

#define lexChunk 10

Lexeme readLexeme(FILE* fi)
{
	char* lexeme = (char*) malloc(lexChunk + 1);
	int lexemeSize = lexChunk;
	char* t = NULL;
	int lexPos = 0;
	Lexeme lex;

	while(c != EOF) {
		if(!isLetter(c))
			break;

		if(lexPos == lexemeSize) {
			t = (char*) realloc(lexeme, lexemeSize + lexChunk + 1);
			if(t == NULL) {
				perror("readLexeme: realloc():");
				free(lexeme);
				lex.type = lexError;
				return lex;
			}
			lexeme = t;
			lexemeSize += lexChunk;
		}
		lexeme[lexPos++] = c;
		get(fi);
	}

	lexeme[lexPos] = '\0';

	if(0 == strcmp(STRUE, lexeme)) {
		lex.type = lexConst;
		lex.val = TRUE;
	}
	else if(0 == strcmp(SFALSE, lexeme)) {
		lex.type = lexConst;
		lex.val = FALSE;
	}
	else {
		/*printf("trying to add a symbol `%s'\n", lexeme);*/
		if(!addSymbol(lexeme)) {
			fprintf(stderr, "readLexeme(): can't add a symbol to table\n");
			free(lexeme);
			lex.type = lexError;
			return lex;
		}
		lex.type = lexIdent;
		lex.ident = lexeme;
		/*printf("added\n");*/
	}

	return lex;
}

Lexeme getLexeme(FILE* fi)
{
	Lexeme lex;

	while(isspace(c)) {
		get(fi);
	}

	if(isLetter(c)) {
		lex = readLexeme(fi);
	}
	else if(isPunct(c)) {
		lex.type = c;
		get(fi);
	}
	else if(c == EOF) {
		lex.type = lexBorder;
	}
	else {
		fprintf(stderr, "\nForbidden character: `%c'\n", c);
		lex.type = lexError;
	}

	return lex;
}
void printLexeme(Lexeme lex)
{
	switch(lex.type) {
		case lexRelation:
			printf("%c ", lex.rel);
			break;
		case lexIdent:
			printf("%c[%s] ", lex.type, lex.ident);
			/*printf("%c ", lex.type);*/
			break;
		case lexConst:
			printf("%c[%d] ", lex.type, lex.val);
			/*printf("%c ", lex.type);*/
			break;
		default:
			if(lex.type != '('
			   && lex.type != ')'
			   && lex.type != ','
			   && lex.type != '&'
			   && lex.type != '|'
			   && lex.type != '#')
			{
				printf("%c[%d] ", lex.type, lex.val);
			}
			else {
				printf("%c ", lex.type);
			}
	}
	fflush(stdout);
}
