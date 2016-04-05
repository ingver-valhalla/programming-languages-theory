// main.cpp

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

void printUsage(char* prog)
{
	printf("Usage: %s INPUT_FILE [OUTPUT_FILE]\n", prog);
}

int main(int argc, char** argv)
{
	FILE* fi = NULL;

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

