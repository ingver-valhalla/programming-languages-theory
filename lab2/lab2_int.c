// lab2_int.c

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum States {
	Normal      , Slash   , Comment   ,
	CPPComment  , Asterisk, String    ,
	EscapedStr  , CharLit , EscapedChr,
	Dec         , Oct     , Oct2      ,
	Hex         , Long    , Unsigned  ,
	UnsignedLong, Z
} States;


int isoct( int c )
{
	return (c >= '0' && c <= '7');
}
    

int main( int argc, char ** argv )
{
	FILE * fi, * fo;
	States State = Normal;
	int c = 0;
	int line = 1;
	int column = 1;
	
	if( argc != 3 )
	{
		return 3;
	}
	fi = fopen( argv[1], "rb" );
	if (!fi)
	{
		fprintf( stderr, "Input file \"%s/*\" open error.\n", argv[1] );
		return 1;
	}
	fo = fopen( argv[2], "wb" );
	if( !fo )
	{
		fclose( fi );
		fprintf( stderr, "Output file \"%s\" open error.\n", argv[2] );
		return 2;
	}
	
	while( (c = fgetc( fi )) != EOF ) {
		
		switch( State ) {
			
			case Normal:
				if( c == '/' ) {
					State = Slash;
				}
				else if( c == '"' ) {
					State = String;
				}
				else if( c == '\'' ) {
					State = CharLit;
				}
				else if( isdigit( c ) ) {
					if( c == '0' ) {
						State = Oct;
					}
					else {
						State = Dec;
					}
					fprintf( fo, "%3d,%-3d    %c", line, column, c );
				}
				else if( isalnum( c ) ) {
					State = Z;
				}
				
				break;
				
			case Slash:
				if( c == '/' ) {
					State = CPPComment;
				}
				else if( c == '*' ) {
					State = Comment;
				}
				else {
					State = Normal;
				}
				break;
				
			case Comment:
				if( c == '*' ) {
					State = Asterisk;
				}
				break;
				
			case CPPComment:
				if( c == '\n' || c == '\r' ) {
					State = Normal;
				}
				break;
				
			case Asterisk:
				if( c == '/' ) {
					State = Normal;
				}
				else if( c != '*' ) {
					State = Comment;
				}
				break;
				
			case String:
				if( c == '\\' ) {
					State = EscapedStr;
				}
				else if( c == '"' ) {
					State = Normal;
				}
				break;
				
			case CharLit:
				if( c == '\\' ) {
					State = EscapedChr;
				}
				else if( c == '\'' ) {
					State = Normal;
				}
				break;
				
			case EscapedStr:
				State = String;
				break;
				
			case EscapedChr:
				State = CharLit;
				break;
				
			case Dec:
				if( c == 'l' ) {
					fputc( c, fo );
					State = Long;
				}
				else if( c == 'u' ) {
					fputc( c, fo );
					State = Unsigned;
				}
				else if( (!isdigit( c ) && isalnum( c ))
				         || c == '_'
				         || c == '.' )
				{
					fputc( c, fo );
					fprintf( fo, "\t\tERROR\n" );
					State = Z;
				}
				else if( isdigit( c ) ) {
					fputc( c, fo );
				}
				else {
					State = Normal;
					fputs( "\t\tint\n", fo );
				}
				break;
				
			case Oct:
				if( c == 'x' ) {
					fputc( c, fo );
					State = Hex;
				}
				else if( c == 'l' ) {
					fputc( c, fo );
					State = Long;
				}
				else if( c == 'u' ) {
					fputc( c, fo );
					State = Unsigned;
				}
				else if( (!isoct( c ) && isalnum( c ))
				         || c == '_'
				         || c == '.' )
				{
					fputc( c, fo );
					fprintf( fo, "\t\tERROR\n" );
					State = Z;
				}
				else if( isoct( c ) ) {
					fputc( c, fo );
					State = Oct2;
				}
				else {
					State = Normal;
					fputs( "\t\tint\n", fo );
				}
				break;
				
			case Oct2:
				if( c == 'l' ) {
					fputc( c, fo );
					State = Long;
				}
				else if( c == 'u' ) {
					fputc( c, fo );
					State = Unsigned;
				}
				else if( (!isoct( c ) && isalnum( c ))
				         || c == '_'
				         || c == '.' )
				{
					fputc( c, fo );
					fprintf( fo, "\t\tERROR\n" );
					State = Z;
				}
				else if( isoct( c ) ) {
					fputc( c, fo );
				}
				else {
					State = Normal;
					fprintf( fo, "\t\tint\n" );
				}
				break;
				
			case Hex:
				if( c == 'l' ) {
					fputc( c, fo );
					State = Long;
				}
				else if( c == 'u' ) {
					fputc( c, fo );
					State = Unsigned;
				}
				else if( (!isxdigit( c ) && isalnum( c ))
				         || c == '_'
				         || c == '.' )
				{
					fputc( c, fo );
					fprintf( fo, "\t\tERROR\n" );
					State = Z;
				}
				else if( isxdigit( c ) ) {
					fputc( c, fo );
				}
				else {
					State = Normal;
					fprintf( fo, "\t\tint\n" );
				}
				break;
				
			case Long:
				if( c == 'u' ) {
					State = UnsignedLong;
					fputc( c, fo );
				}
				else if( isalnum( c ) || c == '_' || c == '.' ) {
					fputc( c, fo );
					fprintf( fo, "\t\tERROR\n" );
					State = Z;
				}
				else {
					fputs( "\t\tlong int\n", fo );
					State = Normal;
				}
				break;
				
			case Unsigned:
				if( c == 'l' ) {
					fputc( c, fo );
					State = UnsignedLong;
				}
				else if( isalnum( c ) || c == '_' || c == '.' ) {
					fputc( c, fo );
					fprintf( fo, "\t\tERROR\n" );
					State = Z;
				}
				else {
					fputs( "\t\tunsigned int\n", fo );
					State = Normal;
				}
				break;
				
			case UnsignedLong:
				if( isalnum( c ) || c == '_' || c == '.' ) {
					fputc( c, fo );
					fprintf( fo, "\t\tERROR\n" );
					State = Z;
				}
				else {
					fputs( "\t\tunsigned long\n", fo );
					State = Normal;
				}
				break;
				
			case Z:
				if( !(isalnum( c ) || c == '_' || c == '.') ) {
					State = Normal;
				}
				break;
				
			default:
				fprintf( stderr, "Fatal error: unknown state" );
				exit( 4 );
		}
		
		++column;
		if( c == '\n' ) {
			++line;
			column = 1;
		}
	}
	
	fclose( fi );
	fclose( fo );
	return 0;
}
