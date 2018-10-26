%{

#include <stdio.h>
#include <tree.h>
#include <string.h>
#include <symtable.h>
#include <errors.h>
#include <cgen.h>

extern int yylineno;
  
enum nodeTypes {PROGRAM, DECL, DECLLIST, FUNDECL, VARDECL, TYPESPEC, ASSIGNSTMT, LOOPSTMT, CONDSTMT, EXPR, IDENTIFIER, VAR, RETURNSTMT, COMPOUNDSTMT, STATEMENTLIST, STATEMENT, ADDEXPR, FACTOR, TERM, FUNCCALL, ARGLIST, CHAR_CONST, INT_CONST, STR_CONST, FORMALDECL, OPADD, OPSUB, OPMUL, OPDIV, FUNBODY, LOCALDECLLIST, FORMALDECLLIST, OPLTE, OPLT, OPGTE, OPGT, OPEQ, OPNEQ, OPASSIGN};

enum dataType {INT_TYPE, CHAR_TYPE, VOID_TYPE };

tree *ast;  /* pointer to AST root */

%}

%union 
{
  int value;
  struct treenode *node;
  char *strval;
}
/* these are all assoc. with value, which maps them to a node, or with stval*/
%token <value> LPAREN RPAREN
%token <value> COMMA 
%token <value> SEMICLN 
%token <value> LSQ_BRKT RSQ_BRKT
%token <value> LCRLY_BRKT RCRLY_BRKT
%token <value> KWD_IF KWD_ELSE
%token <value> KWD_WHILE
%token <value> OPER_ASGN
%token <value> OPER_ADD
%token <value> OPER_SUB
%token <value> OPER_MUL
%token <value> OPER_DIV
%token <value> OPER_LT
%token <value> OPER_GT
%token <value> OPER_GTE
%token <value> OPER_LTE
%token <value> OPER_EQ
%token <value> OPER_NEQ
%token <value> KWD_INT
%token <value> KWD_CHAR 
%token <value> KWD_STRING
%token <value> KWD_VOID
%token <value> KWD_RETURN
%token <strval> ID
%token <value> INTCONST
%token <value> CHARCONST
%token <value> STRCONST
%token <value> ILLEGAL_TOK

%left OPER_GTE OPER_LTE OPER_EQ OPER_NEQ '>' '<'
%left  '+'  '-'
%left  '*'  '/'
%right OPER_ASGN


%type <node> program decl declList funDecl varDecl typeSpecifier assignStmt condStmt loopStmt expression var returnStmt compoundStmt statementList addExpr factor term funcCallExpr argList statement formalDecl relop addop mulop funBody localDeclList formalDeclList

%start program 

%%

program		: declList
		{
		  tree *progNode = maketree(PROGRAM);
                  addChild(progNode, $1);
                 
	          ast = progNode;
	        }
		;

declList	: decl
                {
                   $$ = $1;
                }
		| declList decl
                {
                   tree *declListNode = maketree(DECLLIST);
                   addChild(declListNode, $1);
                   addChild(declListNode, $2);
              
                   $$ = declListNode;
                }                  
		;

decl		: varDecl /* global variable*/
                {
                  $$ = $1;
                }
		| funDecl
                {
                   $$ = $1;
                }
		;

varDecl 	: typeSpecifier ID LSQ_BRKT INTCONST RSQ_BRKT SEMICLN
		{
                   tree *declNode = maketree(VARDECL);
                   addChild(declNode, $1);
                   addChild(declNode, maketreeWithNameVal(IDENTIFIER, $2));
                   addChild(declNode, maketreeWithVal(INT_CONST, $4));
                   $$ = declNode;
		}
		| typeSpecifier ID SEMICLN
		 {
                  
		   tree *declNode = maketree(VARDECL);
                   addChild(declNode, $1);
                   addChild(declNode, maketreeWithNameVal(IDENTIFIER, $2));
                   
		   $$ = declNode;
	      	}
		;

typeSpecifier	: KWD_INT
		{
		  $$ = maketreeWithVal(TYPESPEC, INT_TYPE);
		}
		| KWD_CHAR 
                {
		  $$ = maketreeWithVal(TYPESPEC, CHAR_TYPE);
		}
		| KWD_VOID
                {
		  $$ = maketreeWithVal(TYPESPEC, VOID_TYPE);
		}
		;

funDecl		: typeSpecifier ID LPAREN formalDeclList RPAREN funBody
		{
                  tree *funDeclNode = maketree(FUNDECL);
		  addChild(funDeclNode, $1);
                  addChild(funDeclNode, maketreeWithNameVal(IDENTIFIER, $2));
                  addChild(funDeclNode, $4);
		  addChild(funDeclNode, $6);
		  $$ = funDeclNode;
		}
		| typeSpecifier ID LPAREN RPAREN funBody
		{
		  tree *funDecl = maketree(FUNDECL);
		  addChild(funDecl, $1);
                  addChild(funDecl, maketreeWithNameVal(IDENTIFIER, $2));
                  addChild(funDecl, $5);
		  $$ = funDecl;
		}
		;

formalDeclList	: formalDecl
                {
                   $$ = $1;
                }
		| formalDecl COMMA formalDeclList
                {
                   tree *formalNode = maketree(FORMALDECLLIST);
                   addChild(formalNode, $1);
                   addChild(formalNode, $3);
            
                   $$ = formalNode;
                }
		;

formalDecl	: typeSpecifier ID
                {
                   tree *formalNode = maketree(FORMALDECL);
                   addChild(formalNode, $1);
                   addChild(formalNode, maketreeWithNameVal(IDENTIFIER, $2)); 
                   $$ = formalNode;
                }
		| typeSpecifier ID LSQ_BRKT RSQ_BRKT
                {
                   tree *formalNode = maketree(FORMALDECL);
                   addChild(formalNode, $1);
                   addChild(formalNode, maketreeWithNameVal(IDENTIFIER, $2)); 
                   $$ = formalNode;
		}
		;

funBody		: LCRLY_BRKT localDeclList statementList RCRLY_BRKT
                {
                   tree *funNode = maketree(FUNBODY);
                   addChild(funNode, $2); 
                   addChild(funNode, $3);
                   $$ = funNode;
                }
		;

localDeclList	:
                {
                   $$ = NULL;
                }
		| varDecl localDeclList
                {
                   tree *localDeclNode = maketree(LOCALDECLLIST);
                   addChild(localDeclNode, $1);  
                   addChild(localDeclNode, $2);
                   $$ = localDeclNode;
                }
		;

statementList	: { $$ = NULL; }
      		| statement statementList
                  {
                     tree *statementNode = maketree(STATEMENTLIST);
                     addChild(statementNode, $1);
                     addChild(statementNode, $2);
                     $$ = statementNode;
                  }
		;

statement	: compoundStmt
                  {
                     $$ = $1;
		  }
		| assignStmt
                  {
		     $$ = $1;
		  }
		| condStmt
                  {
                     $$ = $1;
		  }
		| loopStmt
                  {
                    $$ = $1;
		  }
		| returnStmt
                  {
                     $$ = $1;
		  }
		;

compoundStmt	: LCRLY_BRKT statementList RCRLY_BRKT
                {
                   tree *compoundNode = maketree(COMPOUNDSTMT);
                   addChild(compoundNode, $2);
                   $$ = compoundNode;
                }
		;

assignStmt	: var OPER_ASGN expression SEMICLN
		{
		   tree *assignNode = maketree(ASSIGNSTMT);
		   addChild(assignNode, $1);
                   addChild(assignNode, $3);
		   $$ = assignNode;
		}
		| expression SEMICLN
                {
                   tree * assignNode = maketree(ASSIGNSTMT);
                   addChild(assignNode, $1);
                   $$ = assignNode;
                }
		;

condStmt	: KWD_IF LPAREN expression RPAREN statement
		{
		   tree *ifNode = maketree(CONDSTMT);
		   addChild(ifNode, $3);
                   addChild(ifNode, $5);
		   $$ = ifNode;
	        }
		| KWD_IF LPAREN expression RPAREN statement KWD_ELSE statement
 		{
	           tree *elseNode = maketree(CONDSTMT);
                
		   addChild(elseNode, $3);
                   addChild(elseNode, $5);
                   addChild(elseNode, $7);
		   $$ = elseNode;
                }
		;

loopStmt	: KWD_WHILE LPAREN expression RPAREN statement		
		{
		   tree *whileNode = maketree(LOOPSTMT);
		   addChild(whileNode, $3);
                   addChild(whileNode, $5);
		   $$ = whileNode;
	        }
		;

returnStmt	: KWD_RETURN SEMICLN
                {
		  tree *returnNode = maketree(RETURNSTMT);
                  $$ = returnNode;
		}
		| KWD_RETURN expression SEMICLN
                {
		  tree *returnNode = maketree(RETURNSTMT);
                  addChild(returnNode, $2);
                  $$ = returnNode;
		}
		;

var		: ID
		{
		   $$ = maketreeWithNameVal(IDENTIFIER, $1);
	      	}
		| ID LSQ_BRKT addExpr RSQ_BRKT
                {
   		  tree *arrayNode = maketreeWithNameVal(IDENTIFIER, $1);
		  addChild(arrayNode, $3);
	 	  $$ = arrayNode;
 		}
		;

expression	: addExpr
		{
                  $$ = $1;
 		}
		| expression relop addExpr
		{
                  tree * relopTree = maketree($2->nodeKind);
                  addChild(relopTree, $1);
                  addChild(relopTree, $3);
                  $$ = relopTree;
 		}
		;
relop		: OPER_LTE
		{
		  $$ = maketreeWithVal(OPLTE, yyval.value);
		}
		| OPER_LT
		{
		  $$ = maketreeWithVal(OPLT, yyval.value);
		}
		| OPER_GT
		{
		  $$ = maketreeWithVal(OPGT, yyval.value);
		}
		| OPER_GTE
		{
		  $$ = maketreeWithVal(OPGTE, yyval.value);
		}
		| OPER_EQ
		{
		  $$ = maketreeWithVal(OPEQ, yyval.value);
		}
		| OPER_NEQ
		{
		  $$ = maketreeWithVal(OPNEQ, yyval.value);
		}
		;

addExpr		: term
                {
                   $$ = $1;
                }
		| addExpr addop	term
                {
                   tree * addOpNode = maketree($2->nodeKind);
                   addChild(addOpNode, $1);
                   addChild(addOpNode, $3);
                   $$ = addOpNode;
                }
		;

addop		: OPER_ADD
		{
		  $$ = maketreeWithVal(OPADD, yyval.value);
		}
		| OPER_SUB
		{
		  $$ = maketreeWithVal(OPSUB, yyval.value);
		}
		;

term		: factor
		{
                   $$ = $1;
		}
		| term mulop factor
                {
                   tree * mulopTree = maketree($2->nodeKind);
                   addChild(mulopTree, $1);
                   addChild(mulopTree, $3);
                   $$ = mulopTree;
		}
		;

mulop		: OPER_MUL
		{
		  $$ = maketreeWithVal(OPMUL, yyval.value);
		}
		| OPER_DIV
		{
		  $$ = maketreeWithVal(OPDIV, yyval.value);
		}
		;

factor		: LPAREN expression RPAREN
                {
                  
                   tree *factorNode = maketree(FACTOR);
                   addChild(factorNode, $2);
                   $$ = factorNode;
		}
		| var
                {
                   $$ = $1;
		}
		| funcCallExpr
                {
                   $$ = $1;
		}
		| INTCONST
                {
		  $$ = maketreeWithVal(INT_CONST, $1);
		}
		| CHARCONST
                {
                   $$ = maketreeWithVal(CHAR_CONST, $1);
		}
		| STRCONST
                {
                   $$ = maketreeWithVal(STR_CONST, $1);
		}
		;

funcCallExpr	: ID LPAREN argList RPAREN
		{
                   tree *funcNode = maketree(FUNCCALL);
	          addChild(funcNode, maketreeWithNameVal(IDENTIFIER, $1));
                  addChild(funcNode, $3);
                  $$ = funcNode;
		}
		| ID LPAREN RPAREN
		{
		  tree *funcNode = maketree(FUNCCALL);
                  addChild(funcNode, maketreeWithNameVal(IDENTIFIER, $1));
                  $$ = funcNode;
		}
		;

argList		: expression
                {
                  $$ = $1;
                }
		| argList COMMA expression
                {
                  tree *argNode = maketree(ARGLIST);
                  addChild(argNode, $1);
                  addChild(argNode, $3);
                  $$ = argNode;
                }
		;

%%

/* prints the symbol table IFF the ast tree isn't null,
which it would be if there were a syntax error in the program
being parsed */
void callSymTablePrint() {
  if (ast)
  {
     ST_print();
  }
}

/* prints the AST tree IFF the ast tree isn't null,
which it would be if there were a syntax error in the program
being parsed */
void callAstPrint() {
   if (ast)
   {
      printAst(ast, 1);
   }
}

/* walks the complete tree IFF the ast tree isn't null to 
build the symbol table */

void callWalkTree() {
  if (ast)
  {
     ST_walkTree(ast, 1, -1, -1);
  }
}

/* generates the assembly file IFF the ast tree is correct */
void callGenerateCode() {
  if (ast)
  {
     generateCode(ast);
  }
}

/* prints the AR table with most recent AR at the head */
void callARsPrint() {
  if(ast)
  {
     printARs();
  }
}

int yyerror(char * msg) {
  printf("Syntax Error, Line %d \n", yylineno);
  return 0;
}
