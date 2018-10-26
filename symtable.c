#include <tree.h>
#include <symtable.h>
#include <stdio.h>
#include <stdlib.h>

extern int yylineno;

/* string values for scopes */
char *scope[2] = { "GLOBAL", "LOCAL" };

extern char *nodeNames[];

char *nodeKindNames[3] = {"var", "arg", "func"};

char * typeNames[3] = {"int", "char", "void"};

baseType getType(int nodeType)
{
   baseType type;
   switch (nodeType)
   {
      case(0):
        type = INT;
        break;
      case(1):
        type = CHAR;
        break;
      case(2):
        type = VOID;
        break;
      default:
         printf("Internal Error: Invalid data type index\n");
         exit(EXIT_FAILURE);
         break;
   }
   return type;
}

/* prints the entries in the symbol tables */
void ST_print()
{
   printf("GLOBAL SYMBOL TABLE\n");
   int index = 0;
   while (symtable[index]->key)
   {
     printf("Key: %s\tType: %s\tScope: %s\tKind: %s\n", symtable[index]->key, typeNames[symtable[index]->type->allBaseType], scope[symtable[index]->scope], nodeKindNames[symtable[index]->kind]);
     if (symtable[index]->next->key)
     {
        DataItem *current;
        current = (DataItem *) malloc(sizeof(DataItem)); 
        current = symtable[index]->next;
        printf("LLKey: %s\tType: %s\tScope: %s\tKind: %s\n", current->key, typeNames[current->type->allBaseType], scope[current->scope], nodeKindNames[current->kind]);
     
      	while (current->next)
      	{ current = current->next; }
     }
     index++;
   }
   /* loop again, print the local symbol tables */
   int globalIndex = 0;
   while (symtable[globalIndex]->key)
   {
      /* if we have a local symbol table and it has at least one key*/
      if (symtable[globalIndex]->localSymbolCount > 0)
      {
        printf("\nFUNCTION SYMBOL TABLE (%s)\n", symtable[globalIndex]->key);
      }
        int localIndex;
        for (localIndex = 0; localIndex < symtable[globalIndex]->localSymbolCount; localIndex++)
        {
          DataItem * current;
          current = (DataItem *) malloc(sizeof(DataItem));
          current = symtable[globalIndex]->localtable[localIndex];
          printf("Key: %s\tType: %s\tScope: %s\tKind: %s\n", current->key, typeNames[current->type->allBaseType], scope[current->scope], nodeKindNames[current->kind]);
        }
    globalIndex++;
   }
}
/* walks complete tree and inserts any identifier it finds into the symbol table */
void ST_walkTree(tree *node, int nestLevel, int funcIndex, int localSymbolCount)
{
  int i;
  int j; 
 
  char *nodeType = nodeNames[node->nodeKind];

  /* GLOBAL VARIABLE INSERTION */
  if (strcmp(nodeType, "varDecl") == 0 && strcmp(nodeNames[node->parent->nodeKind], "declList") == 0)
  {
     funcIndex = -1;
     allTypes *varTree = malloc(sizeof(allTypes));
     tree * varType = getFirstChild(node);
     varTree->allBaseType = getType(varType->val);
     if (getChild(node, 2))
     {
         tree *arrglobTree = getChild(node, 2);
         varTree->arrType.size = arrglobTree->val;
     }
     int globalVarIndex = ST_insert(getSecondChild(node)->name, varTree, 0, 0, symtable);
     if (globalVarIndex < 0)
     { genErrorWithPlaceholder("Multiply declared variable:", getSecondChild(node)->name, 1, yylineno); }
  }
  
  /* FUNCTION INSERTION */
  if (strcmp(nodeType, "funcDecl") == 0)
  {
     allTypes *funcTree = malloc(sizeof(allTypes));
     tree *retType = getFirstChild(node);
     funcTree->fType.returnType = retType->val;
     /* function name */
     tree *identifier = getSecondChild(node);
     /* insert the function */

     funcIndex = ST_insert(identifier->name, funcTree, 0, 2, symtable);
     /* Check: Same function declared twice */
     if (funcIndex < 0)
     { genErrorWithPlaceholder("Multiply declared function:", identifier->name, 1, yylineno); }

   /* I know this should really be dynamic based on number of args and variables in the function, but I haven't had time to get to it */
   if (funcIndex >= 0)
   {
      for (i = 0; i < MAXIDS; i++)
      { symtable[funcIndex]->localtable[i] = malloc(sizeof(DataItem));}
   }
   }
     /* ARG LIST INSERTION. */
     /* Can currently be a formalDecl (single) under the funcDecl or a formalDeclList (multi) with formalDecls because I'm still struggling with simplifying the AST structure.*/ 
   if (strcmp(nodeType, "formalDecl") == 0 && strcmp(nodeNames[node->parent->nodeKind], "formalDeclList") ==0 || strcmp(nodeType, "formalDecl") == 0 && strcmp(nodeNames[node->parent->nodeKind], "funcDecl") == 0)
   {
      allTypes *argTree = malloc(sizeof(allTypes));
      /* TODO: formal declwill not have an int const child for array must name node accordingly*/
      tree * argType = getFirstChild(node);
      argTree->allBaseType = getType(argType->val);
     
      int argindex = ST_insert(getSecondChild(node)->name, argTree, 1, 1, symtable[funcIndex]->localtable);
      symtable[funcIndex]->localSymbolCount++;
     }
 
  /* LOCAL VARIABLE INSERTION */
  if (strcmp(nodeType,"varDecl") == 0 && strcmp(nodeNames[node->parent->nodeKind], "localDeclList") == 0)
  {
      allTypes *varTree = malloc(sizeof(allTypes));
      tree * varType = getFirstChild(node);
      varTree->allBaseType = getType(varType->val);
      /* third child is a const value, signaling an array */
      if (getChild(node, 2))
      {
          tree *arrTree = getChild(node, 2);
         varTree->arrType.size = arrTree->val;
       }
      int localVarIndex = ST_insert(getSecondChild(node)->name, varTree, 1, 0, symtable[funcIndex]->localtable);  
      /* arrays, INTCONST (any const actually, val is set to the val)*/
      /* Check: multiple var decls in same scope */
      if (localVarIndex < 0)
      { genErrorWithPlaceholder("Multiply declared variable:", getSecondChild(node)->name, 1, yylineno); }
      symtable[funcIndex]->localSymbolCount++;
  }
  /* FUNCTION CALLs */
  if (strcmp(nodeType, "funcCallExpr") == 0)
  {
     char *funcName = getFirstChild(node)->name;
     /* Adding special case so if I call output(int) without declaring, no error 
        so I can use the output(int) calls in the codegen part of the project */
     if (strcmp(funcName, "output") != 0)
     {
     /* Check: undefined function */
     int entryIndex = ST_lookup(funcName, symtable);
    
     if (entryIndex < 0)
     { genErrorWithPlaceholder("Undefined function:", funcName, 1, yylineno); }
     
     /* Check: declaration/call mismatch  # of args*/
     /*first child is always funcCallExpr-->identifier */
     int argListCount = 0;
     int argTableCount = 0;
 
     if (node->numChildren > 1)
     {
        tree * secondChild = getSecondChild(node);
        /* more than one arg */
        if (strcmp(nodeNames[secondChild->nodeKind], "argList") == 0)
        {
           /*Count args in argList*/
           argListCount = walkSubTree(secondChild, "identifier", 1);
           printf("argListCount %d\n", argListCount);
        }
        /* for example: foo(a+b) */
        if (strcmp(nodeNames[secondChild->nodeKind], "expression") == 0)
        {
           argListCount = 1;
        }
     }
     if (node->numChildren == 2)
     {
         /* for example foo(a) or foo(1) */
         argListCount =1;
     }
     
     int i;
     for (i = 0; i < symtable[entryIndex]->localSymbolCount; i++)
     {
        /* if it's an arg (kind = 1) */
        if (symtable[entryIndex]->localtable[i]->kind == 1)
        { argTableCount++; }
     }
     /* if (argTableCount != argListCount)
     { genErrorWithPlaceholder("Call mismatch. Wrong number of arguments:", funcName, 1, yylineno); }*/

      /* Check: declaration/call mismatch  type of args*/
      
      /*get function call name and index*/
      char* funcCallName = getFirstChild(node)->name;
      
      allTypes * argTypes = getListTypes(funcIndex, getSecondChild(node), 10, argListCount);
      for(i = 0; i < argTableCount; i++)
      {
        /* if parameter stored in symbol table has same type as argument passed in, continue */
        if (symtable[funcIndex]->localtable[i]->type->allBaseType == argTypes[i].allBaseType)
        { continue; }
        else
        { genErrorWithPlaceholder("Type mismatch. Argument type does not match function declaration:", funcName, 1, yylineno); }
      }   
  }
  }
 

  /* Check: Undeclared variable */
  if (strcmp(nodeType, "assignStmt") == 0 && strcmp(nodeNames[getFirstChild(node)->nodeKind], "identifier") == 0 )
  {
     char *varName = getFirstChild(node)->name;
     int globalIndexCheck = ST_lookup(varName, symtable);
     int localIndexCheck = ST_lookup(varName, symtable[funcIndex]->localtable);
     if (globalIndexCheck < 0 && localIndexCheck < 0)
     { genErrorWithPlaceholder("Undeclared variable:", varName, 1, yylineno); }     
     else
     {
        /* literal assignment */
        if(getSecondChild(node) && strcmp(nodeNames[getSecondChild(node)->nodeKind], "intConst") == 0)
        {

           if (globalIndexCheck != -1)
           {
              if(symtable[globalIndexCheck]->type->allBaseType != 0)
               { genErrorWithPlaceholder("Type mismatch on assignment:", varName, 1, yylineno); }  
              symtable[globalIndexCheck]->scalarValue = getSecondChild(node)->val;
           }
           if (localIndexCheck != -1)
           {  
               if(symtable[funcIndex]->localtable[localIndexCheck]->type->allBaseType != 0)
               { genErrorWithPlaceholder("Type mismatch on assignment:", varName, 1, yylineno); }  
               symtable[funcIndex]->localtable[localIndexCheck]->scalarValue = getSecondChild(node)->val;
           }
         }
         /* Check assignment mismatch variables */
         if(strcmp(nodeNames[getFirstChild(node)->nodeKind], "identifier") == 0 && strcmp(nodeNames[getSecondChild(node)->nodeKind], "identifier") == 0)
         {
            int lhsglobal = ST_lookup(getFirstChild(node)->name, symtable);
            int lhslocal = ST_lookup(getFirstChild(node)->name, symtable[funcIndex]->localtable);

            int rhsglobal = ST_lookup(getSecondChild(node)->name, symtable);
            int rhslocal = ST_lookup(getSecondChild(node)->name, symtable[funcIndex]->localtable);
            
            if (lhsglobal != -1)
            {
                if(rhsglobal != -1 && symtable[rhsglobal]->type->allBaseType != symtable[lhsglobal]->type->allBaseType)
                { genErrorWithPlaceholder("Type mismatch in assigment to:", getFirstChild(node)->name, 1, yylineno); }

                if(rhslocal != -1 && symtable[funcIndex]->localtable[rhslocal]->type->allBaseType != symtable[lhsglobal]->type->allBaseType)
                { genErrorWithPlaceholder("Type mismatch in assigment to:", getFirstChild(node)->name, 1, yylineno); }
            }
            if (lhslocal != -1)
            {
             
                if(rhsglobal != -1 && symtable[rhsglobal]->type->allBaseType != symtable[funcIndex]->localtable[lhslocal]->type->allBaseType)
                { genErrorWithPlaceholder("Type mismatch in assigment to:", getFirstChild(node)->name, 1, yylineno); }
                
                
                if(rhslocal != -1 && symtable[funcIndex]->localtable[rhslocal]->type->allBaseType != symtable[funcIndex]->localtable[lhslocal]->type->allBaseType)
                { genErrorWithPlaceholder("Type mismatch in assigment to:", getFirstChild(node)->name, 1, yylineno); }
            }
         }
        
        /*have a declared variable ...checks array variable with non int type and type mismatch RHS  and out of bounds index for array*/
        /* Check: out of range int literal for array*/
        tree * stmtID = getFirstChild(node);
        /*assign-identifer-intconst (nested) is literal*/
        
        if (getFirstChild(stmtID) && strcmp(nodeNames[getFirstChild(stmtID)->nodeKind], "intConst") == 0)
        {
           int constVal = getFirstChild(stmtID)->val;
           if (globalIndexCheck != -1)
           {
              if (symtable[globalIndexCheck]->type->arrType.size < constVal)
              { genErrorWithPlaceholder("Index out of bounds:", varName, 1, yylineno); }
           }
           if (localIndexCheck != -1)
           {
              if (symtable[funcIndex]->localtable[localIndexCheck]->type->arrType.size < constVal)
              { genErrorWithPlaceholder("Index out of bounds:", varName, 1, yylineno); }
           }
        }
       /* Check: Index array with non-int variable or int variable out of bounds */
       /* assign-identifier-identifier(nested) is [var] */
       if (getFirstChild(stmtID) && strcmp(nodeNames[getFirstChild(stmtID)->nodeKind], "identifier") == 0)
       {
          int globalArrVarIndex = ST_lookup(getFirstChild(stmtID)->name, symtable);
          int localArrVarIndex = ST_lookup(getFirstChild(stmtID)->name, symtable[funcIndex]->localtable);
          int globalStmtIndex = ST_lookup(stmtID->name, symtable); 
          int localStmtIndex = ST_lookup(stmtID->name, symtable[funcIndex]->localtable);
          if (globalArrVarIndex == -1 && localArrVarIndex == -1)
          { genErrorWithPlaceholder("Undeclared variable:", getFirstChild(stmtID)->name, 1, yylineno); } 
          if (globalArrVarIndex != -1)
          {
             /* basetype enum int = 0 */
              if (symtable[globalArrVarIndex]->type->allBaseType != 0)
              { genErrorWithPlaceholder("Indexing array with non-integer type:", varName, 1, yylineno); }
              else
              {
                if (symtable[funcIndex]->localtable[localStmtIndex]->type->arrType.size < symtable[globalArrVarIndex]->scalarValue)
                    { genErrorWithPlaceholder("Index out of bounds:", varName, 1, yylineno); }
              }  
          }
          if (localArrVarIndex != -1)
          {
              /* basetype enum int = 0 */
              if(symtable[funcIndex]->localtable[localArrVarIndex]->type->allBaseType != 0)
              { genErrorWithPlaceholder("Indexing array with non-integer type:", varName, 1, yylineno); }
              else
              {
                 /* need sibling of the identifer*/
                 tree * assignID = getFirstChild(stmtID);
                 tree * valID = getSecondChild(stmtID);
                 
                 if (localStmtIndex != -1)
                 {
                    if (symtable[funcIndex]->localtable[localArrVarIndex]->scalarValue > symtable[funcIndex]->localtable[localStmtIndex]->type->arrType.size)
                    { genErrorWithPlaceholder("Index out of bounds:", varName, 1, yylineno); }
                 }
                 if (globalStmtIndex != -1)
                 {
                   if (symtable[funcIndex]->localtable[localArrVarIndex]->scalarValue > symtable[globalStmtIndex]->type->arrType.size)
              	   { genErrorWithPlaceholder("Index out of bounds:", varName, 1, yylineno); } 
                 }
               }
          }
      }
    
     }   
  }
  if(getSecondChild(node) && strcmp(nodeType, "assignStmt") == 0 && strcmp(nodeNames[getSecondChild(node)->nodeKind], "funcCallExpr") ==0)
  {
     
     int globalIndexCheck = ST_lookup(getFirstChild(node)->name, symtable);
     int localIndexCheck = ST_lookup(getFirstChild(node)->name, symtable[funcIndex]->localtable);
     int funcIndexCheck = ST_lookup(getFirstChild(getSecondChild(node))->name, symtable);
     if( funcIndexCheck == -1)
     { genErrorWithPlaceholder("Assigning to undeclared function:", getSecondChild(node)->name, 1, yylineno); }
   
     if (localIndexCheck != -1)
     {
        int varType = symtable[funcIndex]->localtable[localIndexCheck]->type->allBaseType;
        int retType = symtable[funcIndexCheck]->type->fType.returnType;
        if (varType != retType)
        { genErrorWithPlaceholder("Type mismatch variable and function call:", getFirstChild(node)->name, 1, yylineno); }
     }
     if (globalIndexCheck != -1)
     {
        int varType = symtable[globalIndexCheck]->type->allBaseType;
        int retType = symtable[funcIndexCheck]->type->fType.returnType;
        if (varType != retType)
         { genErrorWithPlaceholder("Type mismatch variable and function call:", getFirstChild(node)->name, 1, yylineno); }
     }
  }
  for (i = 0; i < node->numChildren; i++)  
  {
     if (funcIndex == -1)
     { ST_walkTree(getChild(node, i), nestLevel + 1, funcIndex, 0); }
     else
     { ST_walkTree(getChild(node, i), nestLevel + 1, funcIndex, symtable[funcIndex]->localSymbolCount); }
  }
}

/* inserts a symbol into the symbol table. id is a pointer to a node in the tree */
/*returns -1 if already exists in table*/
int ST_insert(char *id, allTypes *type, int scope, int kind, DataItem **table) {
   DataItem *newItem; 
   newItem = (DataItem *) malloc(sizeof(DataItem));
   newItem->key = id;
   newItem->type = type; 
   newItem->scope = scope;
   newItem->kind = kind;
   newItem->next = (DataItem *) malloc(sizeof(DataItem));
   int index = ST_lookup(id, table);
   /* new entry */
   if (index == -1)
   {
      int newIndex = 0;
      while (table[newIndex]->key != NULL)
      { newIndex++; }
      table[newIndex] = newItem;
      index = newIndex;
   }
   /* entry with same name, different scope */
   else if(index != -1 && table[index]->scope != scope)
   {
      DataItem *current;
      current = (DataItem *) malloc(sizeof(DataItem)); 
      current = table[index];
      while (!current->next)
      { current->next = current; }
      current->next = newItem; 
   }
   /* duplicate entry scope and name */
   else if(index != -1 && table[index]->scope == scope)
   { return -1; }
   return index;
}

/* returns the index of the node in the symbol table 
-1 if not found. */
int ST_lookup(char *id, DataItem **table) {
  int flag = -1;
  int index = 0;
  while (table[index]->key)
  {
     /* found */
     if (strcmp(table[index]->key, id) == 0)
     {
        flag = index;
        return;
     }
     else
     { index++; }
  } 
  return flag;
}

/* returns array of all type values of provided kind of node in tree */
allTypes * getListTypes(int tableIndex, tree *node, int kind, int size)
{
    allTypes *typeTree = malloc(sizeof(allTypes) * size);
    walkTypeTree(tableIndex, typeTree, node, kind);
    return typeTree;
}
/* walks the type tree, checking types and storing them */
allTypes * walkTypeTree(int tableIndex, allTypes * typeTree, tree *node, int kind)
{
  if (node->nodeKind == kind)
  {
     /* check both tables for variable */
     int localargIndex = ST_lookup(node->name, symtable[tableIndex]->localtable);
     int globalargIndex = ST_lookup(node->name, symtable);
     if (localargIndex != -1)
     { typeTree->allBaseType = symtable[tableIndex]->localtable[localargIndex]->type->allBaseType; }
     else if (globalargIndex != -1)
     {  typeTree->allBaseType = symtable[globalargIndex]->type->allBaseType; }
     /* Check: undeclared variable in in function argument */
     else
     { genErrorWithPlaceholder("Undeclared variable used as argument:", node->name, 1, yylineno);}
     typeTree++;
  }
  int j;
  for (j = 0; j < node->numChildren; j++)  
  { typeTree = walkTypeTree(tableIndex, typeTree, getChild(node, j), kind); }
  return typeTree;
}



