// lab1_4.c

#include <stdio.h>
#include <stdlib.h>
/* Автомат - это множество состояний и переходов, поэтому и реализовывать его надо
 *  как автомат, а не что-либо другое типа вложенных циклов и условий с goto.
 *  Первичны состояния, а в каждом состоянии свой набор символов, которые приводят
 *  к переходу из данного состояния в другое. **/

/* перечисление всех состояний автомата */
typedef enum States { Normal    , Slash   , Comment,
                      CPPComment, Asterisk, String ,
                      CharLit
} States;
    
int main(int argc, char ** argv)//
{
	FILE * fi, * fo;         /* входной и выходной файл */
	States State = Normal;   /* текущее состояние */
	int/*space*/c = 0;           /* считанный символ (только один текущий!) */
	int prev = 0;
	/* все, никакие переменные больше не нужны, этих вполне достаточно */

	//int i = 10;
	//int j = i//*fake*/2;
	//printf( "j = %d", j );
	
	if (argc != 3)
	{
		/* по заданию, программа должна получать имена входного и выходного
		 * файлов при помощи параметров командной строки, поэтому здесь можно
		 * вывести сообщение об ошибке, инструкцию и т.п. А можно обойтись и
		 * без этой проверки вообще, следующих двух условных конструкций будет
		 * достаточно */
		return 3;   /* это зачем? почему 3? подумайте. */
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

	while ((c=fgetc(fi)) != EOF)  /* считываем символ и проверяем, не конец ли файла? */
	{
		switch (State)   /* если нет, то обрабатываем в зависимости от текущего состояния */
		{
		case Normal:
			if (c == '/') {         /* если встретили слэш, */
				State = Slash;     /* то перешли в соответствующее состояние */
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
				fputc(c, fo);      /* иначе дублируем символ в выходной поток */
			}
			break;

		case Slash:          /* если предыдущим был слэш, то ... */
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
			if( c == '"' && prev != '\\' ) {
				fputc( c, fo );
				State = Normal;
			}
			else {
				fputc( c, fo );
			}
			break;

		case CharLit:
			if( c == '\'' && prev != '\\' ) {
				fputc( c, fo );
				State = Normal;
			}
			else {
				fputc( c, fo );
			}
			break;

		default:
			fprintf( stderr, "Fatal error: unknown state" );
			exit( 4 );
		/* и т.д. обрабатываем все состояния автомата, причем сохранять
		 * символ в выходной файл можно либо в каждой секции case ..., */
		}

		/* ... либо здесь в зависимости от нового (State) и/или предшествующего (для
		 *      него тогда потребуется, конечно, еще одна переменная) состояния */
		prev = c;
	}

	fclose(fi);    /* не забывайте закрывать файлы! */
	fclose(fo);    /* особенно выходной, открытый для записи */
	return 0;      /* а это зачем? */
}
