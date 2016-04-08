// lexeme_stack.h

#ifndef LEXEME_STACK
#define LEXEME_STACK

#include "lexer.h"

int initLexStack();
int stackPush(Lexeme lex);
Lexeme stackPeek();
Lexeme stackPop();
int freeStack();
void printStack();

#endif // LEXEME_STACK
