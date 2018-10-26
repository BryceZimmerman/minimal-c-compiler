#include <tree.h>
#include <symtable.h>
#include <cgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* file;

extern char *nodeNames[];
extern DataItem *symtable[];
extern char * typeNames[];
extern char * nodeKindNames[];

struct Record *head = NULL;
struct Record *current = NULL;
int usedRegisters = 0;
int usedLabels = 0;

int nextReg()
{
   usedRegisters++;
   return usedRegisters;
}

void resetReg()
{
   usedRegisters = 0;
}

int newLabel()
{
   usedLabels++;
   return usedLabels;
}

/* Go through the global symbol table, add any global variables to the .data section */
/* I'm setting them to 0 because the grammar doesn't allow initialization of global 
variables unless we're within a function body */
void emitGlobals()
{
  int index = 0;
  int varCount = 0;
  while (symtable[index]->key)
  {
    if (strcmp(nodeKindNames[symtable[index]->kind], "var") == 0)
    {
       if (varCount == 0)
       {
          fprintf(file, "\t%s\n", ".data");
       }
       varCount++;
       int length = strlen(symtable[index]->key) + 1;
       char globalLabel[length];
       strncat(globalLabel, symtable[index]->key, strlen(symtable[index]->key));
       strncat(globalLabel, ":", 1);

       fprintf(file, "%-20.20s.word    0\n", globalLabel);
    }
    index++;
  }
}

/* finds funcDecls so we can create a linked list of activation records */
void findFunctionCalls(tree *node, int nestLevel)
{
   int i;
   char *nodeType = nodeNames[node->nodeKind]; 

   if (strcmp(nodeType, "funcDecl") == 0)
   {
     char * funcName = getSecondChild(node)->name;
     int symIndex = ST_lookup(funcName, symtable);
     addToARList(node, funcName, symIndex);
   }
   for (i = 0; i < node->numChildren; i++)  
  {
    findFunctionCalls(getChild(node, i), nestLevel + 1);
  }
}

void generate(tree *node, int nestLevel)
{  
  int i;
  char *nodeType = nodeNames[node->nodeKind]; 

  if (strcmp(nodeType, "funcDecl") == 0)
  {
     emitFunction(node, getSecondChild(node)->name);
  }

  for (i = 0; i < node->numChildren; i++)  
  {
    generate(getChild(node, i), nestLevel + 1);
  }
}

void generateCode(tree *node)
{
     if(!node)
     { return; }
     emitPre();
     emitGlobals();
     findFunctionCalls(node, 1);
     /* if we have functions in the ARList */
     if (head)
     {
        fprintf(file, "\t.text\n");
     }
     generate(node, 1);
     emitPost();
}

int calcSpace(int numVars)
{
   int locals = numVars * 4;
   return locals + 24;
   /* locals + returnValue(4 always int) + 3 GP registers, 1 FP( 4*4) + globalptr (4) = locals + 24)*/
}

/* returns offset based on position of variable in AR */
int offset(int varPosition, int calculatedSpace)
{
   int offset;
   /* 0-based register numbers */
   if (varPosition == 0)
   {
     offset = calculatedSpace - 4;
   }
   else
   {
      offset = calculatedSpace - 4 - (varPosition * 4);
   }
   return offset;
}

void emitPre()
{
   file = fopen("out.asm", "w"); 
   fprintf(file, "\t#### Start of compiler generated code ####\n");
}

void emitPost()
{
   fprintf(file, "\t#### End of compiler generated code ####\n");
   fclose(file);
}

/* Passing in the funcDecl as node */
void emitFunction(tree *node, char * funcName)
{
  Record * AR = findAR(funcName);
  if (strcmp(funcName, "main") == 0)
  {
     fprintf(file, "\t.globl   main\n");
  }

  /* emit a label */
  fprintf(file, "%s:\n", AR->functionName);
  fprintf(file, "\t# function entry for %s\n", AR->functionName);
  
  /* allocate space for AR. */
  fprintf(file, "\tsubi $sp, $sp, %d\n\n", AR->space);
   
  /* save registers for local variables */
  fprintf(file, "\t# save registers\n");
  fprintf(file, "\tsw $fp, %d($sp)\n", AR->space);
  int i;
  for (i = 0; i < AR->numLocals; i++)
  {
     int varOffset = offset(i + 1, AR->space);
     fprintf(file, "\tsw $t%d, %d($sp)\n", AR->locals[i]->numRegister, varOffset);
  }
  
  /* save register for return value */
  int retOffset = offset(AR->numLocals + 1, AR->space);
  fprintf(file, "\t# save register for return value\n");
  fprintf(file, "\tsw $ra, %d($sp)\n", retOffset);

  /* set frame pointer */
  fprintf(file, "\taddi, $fp, $sp, %d\n\n", AR->space);

  fprintf(file, "\t# function body\n");
  tree *funBody = getChildByKind(node, "funBody");
  /* top-level statementList. Can contain other nested statementLists */
  tree *stateList = getChildByKind(funBody, "statementList");
  if (stateList)
  {
     emitStatements(AR, stateList, 1);
  }

   /* store return value */
 /*fprintf(file, "\tsw $ra, 0($fp)\n");*/


 fprintf(file, "\t# function exit for %s\n", funcName);
 if (strcmp(funcName, "main") == 0)
 {
   fprintf(file, "L0001:\n");
 }
 /* restore registers */
 fprintf(file, "\t#restore registers\n");
 fprintf(file, "\tlw $fp, %d($sp)\n", AR->space);
 int j;
  for (j = 0; j < AR->numLocals; j++)
  {
     int varOffset = offset(j + 1, AR->space);
     fprintf(file, "\tlw $t%d, %d($sp)\n", AR->locals[j]->numRegister, varOffset);
  }
 /* restore caller sp */
 fprintf(file, "\taddi $sp, $sp, %d\n", AR->space); 

if (strcmp(funcName, "main") == 0)
 {
   fprintf(file,"\tli $v0, 17\n");
   fprintf(file, "\tsyscall\n");   
 }
 else
 {
    /* return to caller */
 fprintf(file, "\tjr $ra\n");
 }
  
 fprintf(file, "\t# end %s()\n", funcName);
 /* reset where we're tracking number of use registers */
 resetReg();
} 

void emitStatements(Record * AR, tree *node,  int nestLevel)
{
   if (isNodeName(node, "assignStmt") == 0)
   {
       tree * firstChild = getFirstChild(node);
      if (isNodeName(firstChild, "identifier") == 0)
      {
         tree * secondChild = getSecondChild(node);
         /* simple assignment statements */
         /*if (strcmp(nodeNames[secondChild->nodeKind], "intConst") == 0 || strcmp(nodeNames[secondChild->nodeKind], "identifier") == 0)*/
         if (isNodeName(secondChild, "intConst") == 0 || isNodeName(secondChild, "identifier") == 0 )
         {
            emitAssignment(AR, firstChild, secondChild);
         }
         /* expressions or factors (which are (expression)) */
         if (isNodeName(secondChild, "+") == 0 || isNodeName(secondChild, "-") == 0 || isNodeName(secondChild, "*") == 0 || isNodeName(secondChild, "/") == 0)
         {
            fprintf(file, "\n\t# expression\n");
            emitExpression(AR, secondChild);
         }        
     }
     /* funcCalls */
     if (strcmp(nodeNames[firstChild->nodeKind], "funcCallExpr") == 0)
     {
        emitFuncCall(AR, firstChild);
     } 
   }

    /* conditionals */ 
     
     if (strcmp(nodeNames[node->nodeKind], "condStmt") == 0)
     { 
         tree * firstChild = getFirstChild(node);
         
         /* if else only */
         emitExpression(AR, firstChild);
         /* if */
       /*  tree * ifchild = getFirstChild(getSecondChild(node));
         emitStatements(AR, ifchild, 1);*/
       
         /* else - should be new label */ 
        /* tree * elseChild = getFirstChild(getThirdChild(node));*/
     }
     /* iterative while only */
     if (strcmp(nodeNames[node->nodeKind], "loopStmt") == 0)
     {
       /* emitLoops();*/
     }
     
  int i;
  for (i = 0; i < node->numChildren; i++)  
  { 
      emitStatements(AR, getChild(node, i), nestLevel + 1);
  }
}

void emitAssignment(Record * AR, tree * identifierNode, tree * valueNode)
{
   /* local var*/
   DataItem * identifier = lookupARLocal(AR, identifierNode->name);
   int globalFlag = 0;
   if(!identifier)
   {
      /* global var */
      int index = ST_lookup(identifierNode->name, symtable);
      identifier = symtable[index];
      identifier->numRegister = nextReg();
      globalFlag = 1;
   }
   /* ie x= y*/
   int value; 
   if (strcmp(nodeNames[valueNode->nodeKind], "identifier") == 0)
   {
      DataItem * valID = lookupARLocal(AR, valueNode->name);
      if(!valID)
      {
         /* global var */
         int index = ST_lookup(valueNode->name, symtable);
         valID = symtable[index];

      }
      value = valID->scalarValue;
   }
   if (strcmp(nodeNames[valueNode->nodeKind], "intConst") == 0)
   {
      value = valueNode->val;
   }
   fprintf(file, "\n\t# assignment for %s\n", identifier->key);
   if (globalFlag == 1)
   {
      fprintf(file, "\tla $t%d, %s\n", identifier->numRegister, identifier->key); 
   }
   else
   {
      fprintf(file, "\tli $t%d, %d\n", identifier->numRegister, value);
   }
   fprintf(file, "\tsw $t%d, %d($sp)\n", identifier->numRegister, offset(identifier->numRegister, AR->space));
   
}
    
void emitFuncCall(Record * AR, tree * funcCallNode)
{  
   tree * funcIdNode = getFirstChild(funcCallNode);
   tree * argNode = getSecondChild(funcCallNode);
   int argValue;
 
   /* args could be a local or global variable, an int, OR passed in from caller*/
   if(strcmp(nodeNames[argNode->nodeKind], "identifier")  == 0)
   {
       /* check local variables */
       DataItem * arg = lookupARLocal(AR, argNode->name);
       if (arg)
       {   
           argValue = arg->scalarValue;
       }  
       /* check global variables */
       else
       {
          int index = ST_lookup(argNode->name, symtable);
          if (index != -1)
          {
             argValue = symtable[index]->scalarValue;
          }
       }
   }
   if (strcmp(nodeNames[argNode->nodeKind], "intConst") == 0)
   {
      argValue = argNode->val; 
   }
   if (strcmp(funcIdNode->name, "output") == 0)
   {
      emitOutput(argValue);
   }
   else
   {
     fprintf(file, "\tli $a0, %d\n", argValue);
     fprintf(file, "\tjal %s\n", funcIdNode->name);
     fprintf(file, "\tmove $t0, $v0\n");
   }

}       


/* starting with expression node assignment node
* every expression in my AST has two operand children of the expression (+, -, * , /): 
where operand can be an intConst, an identifer, a factor (ex (a + b)), or another expression node (ex a * b) */
int emitExpression (Record * AR, tree * exprNode)
{
   int result, t1, t2;

   if (isNodeName(exprNode, "+") == 0 || isNodeName(exprNode, "-") == 0 || isNodeName(exprNode, "*") == 0 || isNodeName(exprNode, "/") == 0)
   {
      t1 = emitExpression(AR, getFirstChild(exprNode));
      t2 = emitExpression(AR, getSecondChild(exprNode));
    
      /* addition */
      if (isNodeName(exprNode, "+") == 0 )
      {
         result = nextReg();
         fprintf(file, "\tadd $t%d, $t%d, $t%d\n", result, t1, t2);
         fprintf(file, "\tsw $t%d, %d($sp)\n", result, offset(result, AR->space));
      }
      /*subtraction */
      if (isNodeName(exprNode, "-") == 0 )
      {
         result = nextReg();
         fprintf(file, "\tsub $t%d, $t%d, $t%d\n", result, t1, t2);
         fprintf(file, "\tsw $t%d, %d($sp)\n", result, offset(result, AR->space));
      }
       /* multiplication */
       if (isNodeName(exprNode, "*") == 0 )
      {
         result = nextReg();
         fprintf(file, "\tmul $t%d, $t%d, $t%d\n", result, t1, t2);
         fprintf(file, "\tsw $t%d, %d($sp)\n", result, offset(result, AR->space));
      }
       /* division */
       if (isNodeName(exprNode, "/") == 0 )
      {
         result = nextReg();
         fprintf(file, "\tdiv $t%d, $t%d, $t%d\n", result, t1, t2);
         fprintf(file, "\tsw $t%d, %d($sp)\n", result, offset(result, AR->space));
      }
    }

      /* conditionals */
     if (isNodeName(exprNode, "==") == 0)
     {
       t1 = emitExpression(AR, getFirstChild(exprNode));
       t2 = emitExpression(AR, getSecondChild(exprNode));
       result = nextReg(); 
      
       
       fprintf(file, "\n\t# == \n");
       fprintf(file, "\tbeq $t%d, $t%d, L%d\n", t1, t2, newLabel()); 
     }
     if (isNodeName(exprNode, "<") == 0)
     {
       t1 = emitExpression(AR, getFirstChild(exprNode));
       t2 = emitExpression(AR, getSecondChild(exprNode));
       result = nextReg();
       
       fprintf(file, "\n\t# less than \n");
       fprintf(file, "\tslt $t%d, $t%d, $t%d\n", result, t1, t2); 
       fprintf(file, "\tbeq $t%d, 1, L%d\n", result, newLabel());
     }
     if (isNodeName(exprNode, ">") == 0)
     {
       t1 = emitExpression(AR, getFirstChild(exprNode));
       t2 = emitExpression(AR, getSecondChild(exprNode));
       result = nextReg();
       
       fprintf(file, "\n\t# greater than \n");
       fprintf(file, "\tslt $t%d, $t%d, $t%d\n", result, t2, t1); 
       fprintf(file, "\tbeq $t%d, 1, L%d\n", result, newLabel());
     }
    if (isNodeName(exprNode, "identifier") == 0)
    {
       DataItem * identifier = lookupARLocal(AR, exprNode->name);
       if (identifier)
       {
          result = identifier->numRegister;
       }
       else
       {
         int index = ST_lookup(identifier->key, symtable);
         identifier = symtable[index];
         result = identifier->numRegister;
       }
    }
    if (isNodeName(exprNode, "intConst") == 0)
    {
       fprintf(file, "\tli $t%d, %d\n", result, exprNode->val);
    }
    /* (expression) */
    if (isNodeName(exprNode, "factor") == 0)
    {
       result = emitExpression(AR, getFirstChild(exprNode));
    }
    return result;  
}

void emitOutput(int x)
{
   fprintf(file, "\n\t# output\n");
   fprintf(file, "\tli $v0, 1\n");
   fprintf(file, "\tli $a0, %d\n", x);
   fprintf(file, "\tsyscall\n\n");
}

/* Creates a linked list of activation records using data from the AST and the symbol table
as an intermediate data format to use to generate code. The head of the list should be the
current record, while the tail would represent the first record created */
void addToARList(tree *node, char * funcName, int symIndex)
{
   
   /* Create record */
   Record *newRecord;
   newRecord = (Record *) malloc(sizeof(Record));
   newRecord->functionName = funcName;
   newRecord->numLocals = 0;

   /* Arguments (if any). Currently only supporting 1.*/
   tree * argument = getChildByKind(node, "formalDecl");
   if (argument)
   {
      int ind = ST_lookup(getSecondChild(argument)->name, symtable[symIndex]->localtable);
      newRecord->param = symtable[symIndex]->localtable[ind];
   }

   tree * funcBody = getChildByKind(node, "funBody");
   /* find local variables */
   tree * localDeclList = getChildByKind(funcBody, "localDeclList");
   if (localDeclList)
   {
      newRecord->numLocals = walkSubTree(localDeclList, "varDecl", 1); 

      newRecord->locals = malloc(sizeof(DataItem) * newRecord->numLocals);
      int j;
      int k = 0;
      for (j = 0; j < symtable[symIndex]->localSymbolCount; j++)
      {
         /* symtable order should match funcBody order */
         if (strcmp(nodeKindNames[symtable[symIndex]->localtable[j]->kind], "var") == 0)
         {
            newRecord->locals[k] = symtable[symIndex]->localtable[j];
            
            /* so I don't have to iterate later, store register numbers for later code generation */
            symtable[symIndex]->localtable[j]->numRegister = k; 
            k++;
            nextReg();
         }
      }
   }
   /* calculate space for AR */
   newRecord->space = calcSpace(newRecord->numLocals);
  
   tree * statementList = getChildByKind(funcBody, "statementList");
   if (statementList)
   {
      /* find return value */
      tree * returnNode = getNestedChildByKind(statementList, "returnStmt", 1);
      if(returnNode)   
      { 
         tree * intConst = getChildByKind(returnNode, "intConst"); 
	 if (intConst)
	 {
            newRecord->returnValue = getFirstChild(returnNode)->val;
	 }
	 tree * returnVar = getChildByKind(returnNode, "identifier");
	 if (returnVar)
	 {
            int ind = ST_lookup(returnVar->name, symtable);
	    if (ind != -1)
            {
               newRecord->returnValue = symtable[ind]->scalarValue;
 	    }
	    else
	    {
	       ind = ST_lookup(returnVar->name, symtable[symIndex]->localtable);
               newRecord->returnValue = symtable[symIndex]->localtable[ind]->scalarValue;
	    } 
        }
     }
  }
  
  if (head)
  {
     current->next = newRecord;
     current = current->next; 
  }
  else
  {
     head = newRecord;
     current = head;
  }
}

struct Record * findAR(char * funcName)
{
   struct Record * temp;
   temp = (Record *) malloc(sizeof(Record));
   temp = head;
   while (temp)
   {
      if (strcmp(funcName, temp->functionName) == 0)
      { 
        return temp; 
        break; 
      }
      else
      { temp = temp->next; }
   }
}

/* look up an individual local var in the AR's locals array */
DataItem * lookupARLocal(Record * AR, char * name)
{
   DataItem * found;
   int i;
   int index = -1;

   for (i = 0; i < AR->numLocals; i++)
   {

      if (strcmp(AR->locals[i]->key, name) == 0)
      {
         found = AR->locals[i];
         index = i;
         break;
      }
   }
   if (index == -1)
   { found = NULL; }
   return found;
}

/* prints ARs from most recent. The first AR should be at the tail */
void printARs()
{
   Record * temp = head;
   printf("\n***Active Records***\n");
   while (temp)
   {
      printf("Record for: %s \n", temp->functionName);
      printf("Return Value: %d \n", temp->returnValue);
      if (temp->param)
      {
      	  printf("Param Name: %s \n", temp->param->key);
      }
      if (temp->locals)
      {
         int i;
         for (i = 0; i < temp->numLocals; i++)
         {
            printf("Local Var %d: %s \n", i, temp->locals[i]->key);
         }
      }
      printf("Space: %d \n", temp->space);
      printf("\n");
      temp = temp->next;
   }
}

