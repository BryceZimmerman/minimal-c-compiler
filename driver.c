#include <stdio.h>
#include <stdlib.h>
#include <tree.h>
#include <getopt.h>
#include <symtable.h>

int main (int argc, char **argv)
{
  int option = 0;
  int i;
  int j;
  for (i = 0; i < MAXIDS; i++)
  {
    symtable[i] = malloc(sizeof(DataItem));
  }
 
  /* Accepts just -p -s -h*/
   while ((option = getopt(argc, argv, "pacsh")) != -1)
   {
     switch(option)
     {
        case 'p':
           yyparse();
           callWalkTree();
           callAstPrint();	      
           break;
        case 'a': 
           yyparse();
           callWalkTree();
           callGenerateCode();
           callARsPrint();
           break;
        case 'c':
           yyparse();
           callWalkTree();
           callGenerateCode();
           break;
        case 's':
	   yyparse();
           callWalkTree();
           callSymTablePrint();
           break;
	case 'h':
           printf("Syntax: %s -p (prints AST, optional) -a (prints activation records, optional) -s (prints symbol table, optional) -c (generates .asm file)\n", argv[0]);
	   exit(0);
           break;
        default:
	  break;
     }
     exit(0);
   }
   yyparse();
   callWalkTree();
   return 0;
}
