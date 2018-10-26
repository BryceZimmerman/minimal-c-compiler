#ifndef SYMTABLE_H
#define SYMTABLE_H

#define MAXIDS 1000

typedef enum {INT, CHAR, VOID} baseType;   /* base type */

typedef struct {            /* array type */
   baseType arrDType;
   int size;
} arrayType;

typedef union {                           /* array or base type*/
   baseType scalarType;
   arrayType arrType;
} dataType;

typedef struct funcType {          /* function type*/
   baseType returnType;
   dataType *paramTypes;      
} funcType;  

typedef struct scalarType {
   int intVal;
} scalarType;

/* all types  */
typedef union {
  baseType allBaseType;
  arrayType arrType;
  funcType fType; 
} allTypes;

typedef struct DataItem {
  char *key;
  allTypes *type;
  int scope;
  int kind;
  int scalarValue;
  struct DataItem *next;
  struct DataItem *localtable[MAXIDS];
  int localSymbolCount;
  int numRegister;
} DataItem;

DataItem *symtable[MAXIDS];

baseType getType(int nodeType);

/* prints a table representation of the symbol table */
void ST_print();
/* inserts an entry into the symbol table */
int ST_insert(char *id, allTypes *type, int scope, int kind, DataItem **table);
/* looks up a key in the symbol table. Returns -1 if not found */
int ST_lookup(char *id, DataItem **table);
/* walks the tree and builds the symbol table */
void ST_walkTree(tree *node, int nestLevel, int funcIndex, int localSymbolCount);

allTypes * getListTypes(int tableIndex, tree *node, int kind, int size);
allTypes * walkTypeTree(int tableIndex, allTypes * typeTree, tree *node, int kind);

#endif
