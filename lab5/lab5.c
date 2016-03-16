#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define MAX_ID_LEN 20
#define MAX_VARS 20

typedef short BOOL;
#define TRUE ((BOOL)1)
#define FALSE ((BOOL)0)


typedef struct {
	char name[MAX_ID_LEN+1];
	BOOL val;
} Var;

Var vars[MAX_VARS];
int var_count = 0;
int last_assigned = -1;


typedef enum {
	opAssign = '=',
	opOr     = '|',
	opAnd    = '&',
	opNot    = '~',
	opVar    = 'V',
	opConst  = 'C',
} Operations;

typedef enum {
	trDummy,
	trVar,
	trConst,
	trPtr
} TriadOpType;

typedef struct {
	TriadOpType type;
	union {
		Var * var;
		BOOL const_val;
		int ptr;
	};
} TriadOp;

typedef struct triad {
	Operations operation;
	TriadOp first, second;
	int number;
	struct triad * next;
} Triad;

Triad * ListHead = NULL;
Triad * ListTail = NULL;
int triads_count = 0;

void print_tr_op( TriadOp op );
void print_triad( const Triad * tr );
void set_operation( Triad * tr, Operations operation );
void set_operand( TriadOp * op, TriadOpType, void * val );
Triad * create_triad();
void append_triad( Triad * tr );
void print_list();
void clear_list();


void optimize();
BOOL remove_triad( Triad ** pTriad, Triad * prev );
BOOL find_triad( int num, Triad ** pTriad, Triad ** pPrev );
void triad_not_found_error( int num );


FILE * fi = NULL;
FILE * fo = NULL;
int ch = EOF;
int line = 1;
int column = 1;


void error( const char * msg );


void procS();
int procE();
int procT();
int procM();
int procL();
int procI();


void print_tr_op( TriadOp op )
{
	switch( op.type ) {
		case trVar:
			fprintf( fo, "%s", op.var->name );
			break;
		case trConst:
			fprintf( fo, "%s", op.const_val == TRUE ? "true" : "false" );
			break;
		case trPtr:
			fprintf( fo, "^%d", op.ptr );
			break;
		case trDummy:
			fprintf( fo, "@" );
			break;
		default:
			error( "print_tr_op(): Wrong operand type" );
	}
}

void print_triad( const Triad * tr )
{
	if( tr == NULL )
		return;
    
    fprintf( fo, "%3d:\t ", tr->number );
	fprintf( fo, "%c(", (char)tr->operation );
	print_tr_op( tr->first );
	fprintf( fo, ", " );
	print_tr_op( tr->second );
	fprintf( fo, ")\n" );
}

void set_operation( Triad * tr, Operations op )
{
	tr->operation = op;

}
void set_operand( TriadOp * op, TriadOpType t, void * val )
{
	op->type = t;
	switch( t ) {
		case trVar:
			op->var = (Var *)val;
			break;
		case trConst:
			op->const_val = *(BOOL *)val;
			break;
		case trPtr:
			op->ptr = *(int *)val;
			break;
		case trDummy:
			break;
		default:
			error( "set_operand(): Wrong operand type" );
	}
}

Triad * create_triad()
{
	Triad * p = (Triad *)calloc( sizeof(Triad), 1 );
	if( p == NULL ) {
		perror("calloc()");
		fclose( fi );
		fclose( fo );
		exit( 1 );
	}
	return p;
}

void append_triad( Triad * tr )
{
	if( tr == NULL )
		return;

	tr->next = NULL;
	if( ListTail == NULL ) {
		ListHead = tr;
		ListTail = tr;
	}
	else {
		ListTail->next = tr;
		ListTail = tr;
	}
}

void print_list()
{
	Triad * p = ListHead;
	while( p != NULL ) {
		print_triad( p );
		p = p->next;
	}
}

void clear_list()
{
	while( ListHead != NULL ) {
		Triad *p = ListHead;
		ListHead = ListHead->next;
		free( p );
	}
	ListTail = NULL;
}

void get()
{
	if( ch != '\n' ) {
		putchar( ch );
	}
	ch = getc( fi );
	++column;
	if( ch == '\n' ) {
		++line;
		column = 1;
	}
}

void skip_spaces()
{
	while( isspace( ch ) )
		get();
}

void skip_trailing()
{
	while( isspace( ch ) )
		ch = getc( fi );
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
	if( ch != '\n' )
		putchar( ch );

	//printf( "\n Last symbol: %c\n", ch );
	printf( "\n\n[%d,%d] E: ", line, column );
	puts( msg );
	fclose( fi );
	fclose( fo );
	exit( 1 );
}

int procI()
{
	char buf[MAX_ID_LEN+1] = { 0 };
	int i = 0;
	BOOL val;
	Triad * triad;

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
		val = TRUE;

		triad = create_triad();
		set_operation( triad, opConst );
		set_operand( &triad->first, trConst, &val );
		set_operand( &triad->second, trDummy, NULL );
		append_triad( triad );

		return (triad->number = ++triads_count);
	}
	if( strcmp( buf, "false" ) == 0 ) {
		val = FALSE;

		triad = create_triad();
		set_operation( triad, opConst );
		set_operand( &triad->first, trConst, &val );
		set_operand( &triad->second, trDummy, NULL );
		append_triad( triad );
		
		return (triad->number = ++triads_count);
	}

	/* getting value of variable */
	for( i = 0; i < var_count; ++i ) {
		if( strcmp( vars[i].name, buf ) == 0 ) {

			triad = create_triad();
			set_operation( triad, opVar );
			set_operand( &triad->first, trVar, &vars[i] );
			set_operand( &triad->second, trDummy, NULL );
			append_triad( triad );
			return (triad->number = ++triads_count);
		}
	}

	error( "Undefined identifier" );
	return 0;
}

int procL()
{
	char buf[MAX_ID_LEN+1] = { 0 };
	int i = 0;
	Triad * triad;

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

			triad = create_triad();
			set_operation( triad, opVar );
			set_operand( &triad->first, trVar, &vars[i] );
			set_operand( &triad->second, trDummy, NULL );
			append_triad( triad );

			return (triad->number = ++triads_count);
		}
	}

	/* defining new variable */
	if( var_count >= MAX_VARS ) {
		error( "Too much variables" );
	}

	strncpy( vars[var_count].name, buf, MAX_ID_LEN );
	last_assigned = var_count++;

	triad = create_triad();
	set_operation( triad, opVar );
	set_operand( &triad->first, trVar, &vars[last_assigned] );
	set_operand( &triad->second, trDummy, NULL );
	append_triad( triad );

	return (triad->number = ++triads_count);
}

int procM()
{
	int triad_num;
	Triad * triad = NULL;

	if( ch == '~' ) {
		triad = create_triad();
		get();
		skip_spaces();
		triad_num = procM();
		set_operation( triad, opNot );
		set_operand( &triad->first, trPtr, &triad_num );
		set_operand( &triad->second, trDummy, NULL );
		append_triad( triad );
		return (triad->number = ++triads_count);
	}
	else if( ch == '(' ) {
		get();
		skip_spaces();
		triad_num = procE();
		skip_spaces();
		if( ch != ')' ) {
			error( "Syntax error: probably missed closing `)'" );
		}
		get();
		return triad_num;
	}
	else if( is_letter( ch ) ) {
		triad_num = procI();
		return triad_num;
	}
	else {
		error( "Syntax error" );
	}
	return 0;
}

int procT()
{
	int op1 = procM(), op2;
	Triad * triad = NULL;

	skip_spaces();
	if( ch == '&' ) {
		triad = create_triad();
		get();
		skip_spaces();
		op2 = procT();

		set_operation( triad, opAnd );
		set_operand( &triad->first, trPtr, &op1 );
		set_operand( &triad->second, trPtr, &op2 );
		append_triad( triad );

		return (triad->number = ++triads_count);
	}
	return op1;
}

int procE()
{
	int op1 = procT(), op2;
	Triad * triad = NULL;

	skip_spaces();
	if( ch == '|' ) {
		triad = create_triad();
		get();
		skip_spaces();
		op2 = procE();

		set_operation( triad, opOr );
		set_operand( &triad->first, trPtr, &op1 );
		set_operand( &triad->second, trPtr, &op2 );
		append_triad( triad );

		return (triad->number = ++triads_count);
	}
	return op1;
}

void procS( int num )
{
	printf( "  Operator #%d:\t", num );

	if( ch == '(' ) {
		Triad * triad = create_triad();
		int var, val;

		get();
		skip_spaces();
		var = procL();

		skip_spaces();

		if( ch != ',' ) {
			error( "Syntax error: expected `,'" );
		}
		get();

		skip_spaces();
		val = procE();
		skip_spaces();

		if( ch != ')' ) {
			error( "@Syntax error: expected `)'" );
		}

		set_operation( triad, opAssign );
		set_operand( &triad->first, trPtr, &var );
		set_operand( &triad->second, trPtr, &val );
		append_triad( triad );
		triad->number = ++triads_count;
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
			/*printf( "  ->  " );*/
			/*print_var( last_assigned );*/
			printf( "\n" );
			skip_trailing();
		}
		++op;
	}
	printf( "End parsing\n\n" );
	/*print_all();*/
}

/* fatal error on pointing on removed triad */
void triad_not_found_error( int num )
{
	fprintf( stderr, "Triad #%d not found", num );
	clear_list();
	fclose( fi );
	fclose( fo );
	exit(1);
}

/* searches for a triad with by number num, removes than returns it */
BOOL find_triad( int num, Triad ** pTriad, Triad ** pPrev )
{
	if( pTriad == NULL || pPrev == NULL ) {
		return FALSE;
	}
	*pTriad = ListHead;
	while( *pTriad != NULL ) {
		if( (*pTriad)->number == num ) {
			return TRUE;
		}
		*pPrev = *pTriad;
		*pTriad= (*pTriad)->next;
	}
	triad_not_found_error( num );
	return FALSE;
}

/* remove a triad from the list */
BOOL remove_triad( Triad ** pTriad, Triad * prev )
{
	if( pTriad == NULL || *pTriad == NULL )
	{
		return FALSE;
	}
	if( prev == NULL )
		ListHead = (*pTriad)->next;
	else {
		prev->next = (*pTriad)->next;
	}
	free( *pTriad );
	*pTriad = NULL;
	return TRUE;
}


void optimize()
{
	Triad * p = ListHead;
	Triad * q = NULL;
	Triad * qprev = NULL;
	TriadOp * to_change = NULL;
	BOOL val = FALSE;

	while( p != NULL ) {
		// First rule:
		// If the only operand of an unary operation or the right operand of a
		// binary operation (except assignment) is a pointer to a constant
		// loading triad, it's replaced by a constant itself and corresponding
		// triad is deleted.

		// checking arity of the operation
		// the only unary operation is opNot
		if( p->operation == opNot ) {
			to_change = &p->first;
		}
		else {
			to_change = &p->second;
		}
		// apply first rule
		if( to_change->type == trPtr && p->operation != opAssign ) {
			find_triad( to_change->ptr, &q, &qprev );
			if( q->operation == opConst ) {
				set_operand( to_change, trConst, &q->first.const_val );
				remove_triad( &q, qprev );
			}
		}


		// Second rule:
		// If the only operand of the unary operation is a constant, evaluate
		// the operation and replace the triad with constant loading.
		
		// apply second rule
		if( p->operation == opNot && to_change->type == trConst ) {
			set_operation( p, opConst );
			val = !to_change->const_val;
			set_operand( to_change, trConst, &val);
		}


		//  Third rule:
		//  If the first operand of a binary operation (except assignment) is a
		//  pointer on constant loading and the second operand is a constant,
		//  evaluate operation and replace the triad with loading of a result.
		//  Previous loading operator is deleted.
		
		// check arity
		if( p->operation != opNot && p->operation != opAssign ) {
			// apply third rule
			if( p->first.type == trPtr && p->second.type == trConst ) {
				find_triad( p->first.ptr, &q, &qprev );
				if( q->operation == opConst ) {
					if( p->operation == opOr ) {
						val = q->first.const_val | p->second.const_val;
					}
					else if( p->operation == opAnd ) {
						val = q->first.const_val & p->second.const_val;
					}
					else {
						fprintf( stderr, "optimize(): Invalid operation: %c", p->operation );
						clear_list();
						fclose( fi );
						fclose( fo );
						exit( 1 );
					}
					set_operation( p, opConst );
					set_operand( &p->first, trConst, &val );
					set_operand( &p->second, trDummy, NULL );
					remove_triad( &q, qprev );
				}
			}
		}


		// Fourth rule:
		// If the first operand of the assigment is pointer to variable triad,
		// it is replaced by the name of a variable. Variable triad is deleted

		// apply fourth rule
		if( p->operation == opAssign ) {
			find_triad( p->first.ptr, &q, &qprev );
			if( q->operation == opVar ) {
				set_operand( &p->first, trVar, q->first.var );
				remove_triad( &q, qprev );
			}
			else {
				fprintf( stderr, "optimize(): Fatal error. Only variables can be assigned to" );
				clear_list();
				fclose( fi );
				fclose( fo );
				exit( 1 );
			}
		}
        p = p->next;
    }
}

int main( int argc, char * argv[] )
{
	if( argc != 3 ) {
		printf( "Usage: %s INPUT OUTPUT\n", argv[0] );
		return 0;
	}

	fi = fopen( argv[1], "rb" );
	if( fi == NULL ) {
		perror( "open()" );
		exit(1);
	}

	fo = fopen( argv[2], "w" );
	if( fo == NULL ) {
		perror( "open()" );
		fclose( fi );
		exit(1);
	}

	parse();
	print_list();
	optimize();
	fprintf( fo, "\n\n// Optimized code:\n" );
	print_list();
	clear_list();

	fclose( fi );
	fclose( fo );
	return 0;
}
