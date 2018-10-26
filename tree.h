#ifndef TREE_H
#define TREE_H

#define MAXCHILDREN 12

typedef struct treenode tree; 

/* tree node */
struct treenode {
  int nodeKind;
  int numChildren;
  int val;
  char* name;
  tree *parent;
  tree *children[MAXCHILDREN];
};

/* builds sub tree with zero children  */
tree * maketree(int kind);

/* builds sub tree with leaf node */
tree * maketreeWithVal(int kind, int val);

/* builds sub tree with leaf node and name */
tree * maketreeWithNameVal(int kind, char *name);

tree * maketreeWithChild(int kind, tree *child);

/* adds a child to the parent tree */
void addChild(tree *parent, tree *child);

tree * getChildByKind(tree *node, char *kind);

tree * getNestedChildByKind(tree *node, char *kind, int nestLevel);

/* walk tree, return number of children of specified kind */
int countChildren(tree *node, char *kind);

int walkSubTree(tree *node, char* kind, int nestLevel);

/* prints the AST */
void printAst(tree *root, int nestLevel);

int isNodeName(tree * node, char *kind);

/* tree manipulation macros */ 
/* if you are writing your compiler in C, you would want to have a large collection of these */

#define nextAvailChild(node) node->children[node->numChildren] 
#define getChild(node, index) node->children[index]
#define getFirstChild(node) node->children[0]
#define getSecondChild(node) node->children[1]
#define getThirdChild(node) node->children[2]
#define getNodeName(node) nodeNames[node->nodeKind]

#endif
