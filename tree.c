#include<tree.h>
#include<stdio.h>
#include<stdlib.h>

/* string values for ast node types, makes tree output more readable */
char *nodeNames[40] = {"program", "decl", "declList", "funcDecl", "varDecl", "typeSpecfier", "assignStmt", "loopStmt", "condStmt", "expression", "identifier", "var", "returnStmt", "compoundStmt", "statementList", "statement", "addExpr", "factor", "term", "funcCallExpr", "argList", "charConst", "intConst", "strConst", "formalDecl", "+", "-", "*", "/", "funBody", "localDeclList", "formalDeclList", "<=", "<", ">=", ">", "==", "!="};

tree *maketree(int kind) {
  tree *this = (tree *) malloc(sizeof(struct treenode));
  this->nodeKind = kind;
  this->numChildren = 0;
  return this;
}

tree *maketreeWithVal(int kind, int val) {
  tree *this = (tree *) malloc(sizeof(struct treenode));
  this->nodeKind = kind;
  this->numChildren = 0;
  this->val = val;
  return this;
}

tree * maketreeWithNameVal(int kind, char *name)
{
  tree *this = (tree *) malloc(sizeof(struct treenode));
  this->nodeKind = kind;
  this->numChildren = 0;
  this->name = name;
  
  return this;
}

tree * maketreeWithChild(int kind, tree *child)
{
tree *this = (tree *) malloc(sizeof(struct treenode));
this->nodeKind = kind;
addChild(this, child);
this->numChildren++; /* added this increment during codegen */
return this;
}


void addChild(tree *parent, tree *child) {
  if (parent->numChildren == MAXCHILDREN) {
    printf("Cannot add child to parent node\n");
    exit(1);
  }
  if(child)
  {
  nextAvailChild(parent) = child;
  parent->numChildren++;
  child->parent = parent;
  }
}

/* returns first child of node that matches kind */
tree * getChildByKind(tree *node, char *kind)
{
  tree * localNode = NULL;
  int i;
  for (i = 0; i < node->numChildren; i++)
  {
     if (strcmp(kind, nodeNames[node->children[i]->nodeKind]) == 0)
     {
       localNode = node->children[i]; 
     }
  }
  return localNode;
}


/* recursively search for node that matches kind and return the first one we find.
useful for nested List node types */
tree * getNestedChildByKind(tree *node, char *kind, int nestLevel)
{
   tree * localNode = NULL;
   if (strcmp(nodeNames[node->nodeKind], kind) == 0)
   {
       return node;
   }
   int i;
   for (i = 0; i < node->numChildren && localNode == NULL; i++)
   {
      localNode = getNestedChildByKind(getChild(node, i), kind, nestLevel + 1);
   }
   return localNode;
}


int isNodeName(tree * node, char *kind)
{
   int isKind = strcmp(getNodeName(node), kind);
   return isKind;
}

/*recursively count a kind of node in a given tree. */
int walkSubTree(tree *node, char* kind, int nestLevel)
{
   int count = 0;
   if (strcmp(nodeNames[node->nodeKind], kind) == 0)
   {
      count++;
   }
  int i;
  for (i = 0; i < node->numChildren; i++)  
  { count += walkSubTree(getChild(node, i), kind, nestLevel + 1); }
  return count;
}

/* prints the complete AST */
void printAst(tree *node, int nestLevel) {
     printf("%s\n", nodeNames[node->nodeKind]);
  int i, j;

  for (i = 0; i < node->numChildren; i++)  
  {
    for (j = 0; j < nestLevel; j++) 
      printf("   ");
  printAst(getChild(node, i), nestLevel + 1);
  }
}



