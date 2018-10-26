#ifndef CGEN_H
#define CGEN_H

typedef struct Record {
   int space; /* space to allocate */
   char * functionName;
   char *returnAddress;
   int returnValue;  /* only supporting ints for data types */
   int returnValueRegister;
   int numLocals; /* number of local vars */
   struct Record *next;
   struct DataItem *param; /* only supporting 1 formal param */
   struct DataItem **locals;
} Record; 

int nextReg();
void resetReg();
void findFunctionCalls(tree *node, int nestLevel);
void emitGlobals();
void generate(tree *node, int nestLevel);
void generateCode(tree *node);
int calcSpace(int numVars); 
int offset(int varPosition, int calculatedSpace);
void emitFunction(tree *node, char * funcName);
void emitStatements(Record * AR, tree *node,  int nestLevel);
void emitAssignment(Record * AR, tree * identifierNode, tree * valueNode);
void emitFuncCall(Record * AR, tree * funcCallNode);
void emitLoop(); 
void emitPre();
void emitPost();
int emitExpression(Record * AR, tree * exprNode);
void emitOutput(int x); 
void printARs();
struct Record * findAR(char * funcName);
void addToARList(tree *node, char * funcName, int symIndex);
DataItem * lookupARLocal(Record * AR, char * name);
#endif

