// lexeme_stack.h

#ifndef LEXEME_STACK
#define LEXEME_STACK

#include "lexer.h"

int initStack();
int enlargeStack();
void stackPush(LexemeType lex);
LexemeType stackPop();
int freeStack();

#endif // LEXEME_STACK
