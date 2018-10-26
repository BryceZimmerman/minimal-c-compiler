#include<errors.h>
#include<stdio.h>
#include<stdlib.h>

/* string values for scopes */

char *errorLevel[2] = { "WARN", "ERROR" };

// 0 for warning, 1 for error
void genError(char* msg, int level, int line)
{

   switch (level)
   {
      case 0:
         printf("Warning: %s Line: %d\n", msg, line);
         break;
      case 1:
         printf("Error: %s Line: %d\n", msg, line);
         exit(EXIT_FAILURE);
      default:
         break;
   }   
}

void genErrorWithPlaceholder(char* msg, char* placeholder, int level, int line)
{

   switch (level)
   {
      case 0:
         printf("Warning: %s %s Line: %d\n", msg, placeholder, line);
         break;
      case 1:
         printf("Error: %s %s Line: %d\n", msg, placeholder, line);
         exit(EXIT_FAILURE);
      default:
         break;
   }   
}

