// lexeme_stack.h

#ifndef LEXEME_STACK
#define LEXEME_STACK

#include "lexer.h"

extern Lexeme* lexStack;
extern int lexStackSize;
extern int lexStackTop;

int initStack();
void stackPush(Lexeme lex);
Lexeme stackPeek();
Lexeme stackPop();
int freeStack();
void printStack();

#endif // LEXEME_STACK
