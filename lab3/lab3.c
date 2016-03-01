#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ID_LEN 20
#define MAX_VARS 20

typedef short BOOL;
#define TRUE 1
#define FALSE 0

typedef struct var {
	char name[MAX_ID_LEN+1];
	BOOL val;
} var;

var vars[MAX_VARS];

FILE * f = NULL;
int ch = EOF;
int var_count = 0;
int last_assigned = -1;


void procS();
BOOL procE();
BOOL procT();
BOOL procM();
BOOL * procL();
BOOL procI();


void get()
{
	if( ch != '\n' ) {
		putchar( ch );
	}
	ch = getc( f );
}

void skip_spaces()
{
	while( isspace( ch ) )
		get();
}

void skip_trailing()
{
	while( isspace( ch ) )
		ch = getc( f );
}

BOOL is_letter( char c )
{
	return c >= 'a' && c <= 'z';
}

void print_var( int n )
{
	if( n >= 0 && n < MAX_VARS ) {
		printf( "%s = %s\n", vars[n].name,
		    vars[n].val ? "true" : "false" );
	}
	else {
		fprintf( stderr, "There is no variable with index %d\n", n );
	}
}

void print_all()
{
	int i;
	printf( "Defined %d variables:\n", var_count );
	for( i = 0; i < var_count; ++i ) {
		print_var( i );
	}
}

void error( const char * msg )
{
	/* printing last symbol */
	/*if( ch != '\n' )*/
		/*putchar( ch );*/

	printf( "\n Last symbol: %c\n", ch );
	printf( "\n\nE: " );
	puts( msg );
	exit( 1 );
}

BOOL procI()
{
	char buf[MAX_ID_LEN+1] = { 0 };
	int i = 0;

	/* reading literal */
	while( is_letter( ch ) && i < MAX_ID_LEN ) {
		buf[i] = ch;
		++i;
		get();
	}

	if( ch == EOF ) {
		error( "Unexpected end of file" );
	}

	/* no literal provided */
	if( i == 0 ) {
		error( "Expected identifier or constant" );
	}

	/* identifier reached max length but not ended yet */
	if( i == MAX_ID_LEN && is_letter( ch ) ) {
		error( "Too long identifier" );
	}

	/* constants */
	if( strcmp( buf, "true" ) == 0 ) {
		return TRUE;
	}
	if( strcmp( buf, "false" ) == 0 ) {
		return FALSE;
	}

	/* getting value of variable */
	for( i = 0; i < var_count; ++i ) {
		if( strcmp( vars[i].name, buf ) == 0 ) {
			return vars[i].val;
		}
	}

	error( "Undefined identifier" );
	return vars[var_count].val;
}

BOOL * procL()
{
	char buf[MAX_ID_LEN+1] = { 0 };
	int i = 0;

	/* reading literal */
	while( is_letter( ch ) && i < MAX_ID_LEN ) {
		buf[i] = ch;
		++i;
		get();
	}

	if( ch == EOF ) {
		error( "Unexpected end of file" );
	}

	/* no identifier provided */
	if( i == 0 ) {
		error( "Expected identifier" );
	}

	/* identifier reached max length but not ended yet */
	if( i == MAX_ID_LEN && is_letter( ch ) ) {
		error( "Too long identifier" );
	}

	/* trying to use "true" or "false" as identifier */
	if( strcmp( buf, "true" ) == 0
	    || strcmp( buf, "false" ) == 0 )
	{
		error( "Constants \"true\" and \"false\" can't be used as identifiers" );
	}

	/* checking if identifier is already defined */
	for( i = 0; i < var_count; ++i ) {
		if( strcmp( vars[i].name, buf ) == 0 ) {
			last_assigned = i;
			return &vars[i].val;
		}
	}

	/* defining new variable */
	if( var_count >= MAX_VARS ) {
		error( "Too much variables" );
	}

	strncpy( vars[var_count].name, buf, MAX_ID_LEN );
	last_assigned = var_count;
	return &vars[var_count++].val;
}

BOOL procM()
{
	BOOL val;
	if( ch == '~' ) {
		get();
		skip_spaces();
		val = !procM();
	}
	else if( ch == '(' ) {
		get();
		skip_spaces();
		/*putchar('E');*/
		val = procE();
		/*putchar( '/' );*/
		/*putchar('E');*/
		skip_spaces();
		if( ch != ')' ) {
			error( "Syntax error: probably missed closing `)'" );
		}
		get();
	}
	else if( is_letter( ch ) ) {
		val = procI();
	}
	else {
		error( "Syntax error" );
	}
	return val;
}

BOOL procT()
{
	BOOL val;
	/*putchar('M');*/
	val = procM();
	/*putchar( '/' );*/
	/*putchar('M');*/
	skip_spaces();
	if( ch == '&' ) {
		get();
		skip_spaces();
		/*putchar('T');*/
		val &= procT();
		/*putchar( '/' );*/
		/*putchar('T');*/
	}
	return val;
}

BOOL procE()
{
	BOOL val;
	/*putchar('T');*/
	val = procT();
	/*putchar( '/' );*/
	/*putchar('T');*/
	skip_spaces();
	if( ch == '|' ) {
		get();
		skip_spaces();
		/*putchar('E');*/
		val |= procE();
		/*putchar( '/' );*/
		/*putchar('E');*/
	}
	return val;
}

void procS( int num )
{
	printf( "  Operator #%d:\t", num );
	if( ch == '(' ) {
		get();
		skip_spaces();
		BOOL * val = procL();

		skip_spaces();

		if( ch != ',' ) {
			error( "Syntax error: expected `,'" );
		}
		get();

		skip_spaces();
		*val = procE();
		skip_spaces();

		if( ch != ')' ) {
			error( "Syntax error: expected `)'" );
		}
	}
	else {
		error( "Expected operator" );
	}
	get();
}

void parse()
{
	printf( "Begin parsing\n" );
	get();
	int op = 1;
	while( ch != EOF ) {
		skip_spaces();
		if( ch != EOF ) {
			procS( op );
			printf( "  ->  " );
			print_var( last_assigned );
			skip_trailing();
		}
		++op;
	}
	printf( "End parsing\n\n" );
	print_all();
}

int main( int argc, char * argv[] )
{
	/*printf( "TRUE & TRUE = %d\n", TRUE & TRUE );*/
	/*printf( "FALSE & TRUE = %d\n", FALSE & TRUE );*/
	/*printf( "TRUE & FALSE = %d\n", TRUE & FALSE );*/
	/*printf( "FALSE & FALSE = %d\n", FALSE & FALSE );*/
	/*printf( "TRUE | TRUE = %d\n", TRUE | TRUE );*/
	/*printf( "FALSE | TRUE = %d\n", FALSE | TRUE );*/
	/*printf( "TRUE | FALSE = %d\n", TRUE | FALSE );*/
	/*printf( "FALSE | FALSE = %d\n", FALSE | FALSE );*/
	/*printf( "~TRUE = %d\n", !TRUE );*/
	/*printf( "~FALSE = %d\n", !FALSE );*/

	if( argc != 2 ) {
		printf( "Usage: %s FILENAME\n", argv[0] );
		return 0;
	}

	f = fopen( argv[1], "rb" );
	if( f == NULL ) {
		perror( "open()" );
		exit(1);
	}

	parse();

	fclose( f );
	return 0;
}
