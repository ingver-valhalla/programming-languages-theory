

#include <stdio.h>
#include <stdlib.h>
 

 
typedef enum States { Normal    , Slash   , Comment   ,
                      CPPComment, Asterisk, String    ,
                      EscapedStr, CharLit,  EscapedChr
} States;
    
int main(int argc, char ** argv)
{
	FILE * fi, * fo;          
	States State = Normal;    
	int c = 0;            
	 

	
	
	
	
	if (argc != 3)
	{
		 
		return 3;    
	}
	fi = fopen(argv[1], "rb");
	if (!fi)
	{
		fprintf(stderr, "Input file \"%s/*\" open error.\n", argv[1]);
		return 1;
	}
	fo = fopen(argv[2], "wb");
	if (!fo)
	{
		fclose(fi);
		fprintf(stderr, "Output file \"%s\" open error.\n", argv[2]);
		return 2;
	}

	while ((c=fgetc(fi)) != EOF)   
	{
		switch (State)    
		{
		case Normal:
			if (c == '/') {          
				State = Slash;      
			}
			else if( c == '"' ) {
				fputc( c, fo );
				State = String;
			}
			else if( c == '\'' ) {
				fputc( c, fo );
				State = CharLit;
			}
			else {
				fputc(c, fo);       
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
				fputc( '/', fo );
				fputc( c, fo );
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
				fputc( c, fo );
				State = Normal;
			}
			break;

		case Asterisk:
			if( c == '/' ) {
				fputc( ' ', fo );
				State = Normal;
			}
			else if( c != '*' ) {
				State = Comment;
			}
			break;

		case String:
			fputc( c, fo );
			if( c == '\\' ) {
				State = EscapedStr;
			}
			else if( c == '"' ) {
				State = Normal;
			}
			break;

		case CharLit:
			fputc( c, fo );
			if( c == '\\' ) {
				State = EscapedChr;
			}
			else if( c == '\'' ) {
				State = Normal;
			}
			break;
		case EscapedStr:
			fputc( c, fo );
			State = String;
			break;

		case EscapedChr:
			fputc( c, fo );
			State = CharLit;
			break;
            
		default:
			fprintf( stderr, "Fatal error: unknown state" );
			exit( 4 );
		 
		}

		 
	}

	fclose(fi);     
	fclose(fo);     
	return 0;       
}
