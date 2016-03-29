// lexer.c

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void printUsage(char* prog)
{
	printf("Usage: %s INPUT_FILE [OUTPUT_FILE]\n", prog);
}

int isLetter(int c)
{
	if(c >= 'a' && c <= 'z')
		return 1;
	return 0;
}

int isPunct(int c)
{
	char puncts[] = { '(', ',', ')', '|', '&', '~' };
	int i;

	for(i = 0; i < sizeof(puncts); ++i) {
		if(c == puncts[i])
			return 1;
	}

	return 0;
}

#define LEXEME_CHUNK 10

int readLexeme(FILE* fi, int c)
{
	int lexemeSize = LEXEME_CHUNK;
	char* lexeme = (char*) malloc(lexemeSize + 1);
	char* t = NULL;
	lexeme[0] = c;
	int curPos = 1;

	while(EOF != (c = getc(fi))) {
		if(!isLetter(c))
			break;

		if(curPos == lexemeSize) {
			t = (char*) realloc(lexeme, lexemeSize + LEXEME_CHUNK + 1);
			if(t == NULL) {
				free(lexeme);
				perror("readLexeme: realloc():");
				fclose(fi);
				exit(4);
			}
			lexeme = t;
			lexemeSize += LEXEME_CHUNK;
		}
		lexeme[curPos++] = c;
	}

	lexeme[curPos] = '\0';

	if(0 == strcmp("true", lexeme)
	   || 0 == strcmp("false", lexeme))
	{
		putchar('D');
	}
	else
		putchar('I');

	fprintf(stderr, "lexeme: %s\n", lexeme);

	free(lexeme);
	return c;
}

void parse(FILE* fi)
{
	int c = 0;

	c = getc(fi);
	while(c != EOF) {
		if(isspace(c)) {
			c = getc(fi);
		}
		else if(isLetter(c)) {
			c = readLexeme(fi, c);
		}
		else if(isPunct(c)) {
			putchar(c);
			c = getc(fi);
		}
		else {
			fprintf(stderr, "Forbidden character: %c\n", c);
			fclose(fi);
			exit(3);
		}
	}
}

int main(int argc, char** argv)
{
	FILE* fi;

	if(argc < 2 || argc > 3) {
		printUsage(argv[0]);
		exit(0);
	}

	fi = fopen(argv[1], "r");
	if(fi == NULL) {
		fprintf(stderr, "Can't open file %s\n", argv[1]);
		exit(1);
	}

	if(argc == 3) {
		/* redirecting stdout to OUTPUT_FILE */
		if( NULL == freopen(argv[2], "w", stdout)) {
			fprintf(stderr, "Can't open file %s\n", argv[2]);
			fclose(fi);
			exit(2);
		}
	}

	parse(fi);

	fclose(fi);

	return 0;
}

