// code_gen.h

#ifndef CODE_GEN_H
#define CODE_GEN_H

typedef char BOOL;
#define TRUE   (BOOL)1
#define FALSE  (BOOL)0

typedef struct {
	char* name; // name from symbol table
	BOOL val;   // calculated value
} Var;


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
		char* var;       // name of var from symbol table
		BOOL constVal;   // constant
		int ptr;         // pointer to other triad
	};
} TriadOp;

typedef struct triad {
	Operations operation;
	TriadOp first, second;
	struct triad* next;
} Triad;

int initVarTable();
int addVar(char* name, BOOL val);
BOOL* getVariable(const char* name);
void freeVarTable();
void printVarTable();

int printTriadOp(TriadOp op);
int printTriad(Triad* tr);
void setOperation(Triad* tr, Operations operation);
int setOperand(TriadOp* op, TriadOpType t, void* val);
Triad* createTriad();
int appendTriad(Triad * tr);
void printTriadList();
void clearTriadList();

#endif // CODE_GEN_H
