// symbol_table.h

#ifndef SYMBOL_TABLE
#define SYMBOL_TABLE

extern char** symTable;

int initSymTable();
int addSymbol(char * symbol);
void freeSymTable();
void printSymTable();

#endif // SYMBOL_TABLE
