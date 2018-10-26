 /* definitions */

%{
#include <stdio.h>
#include <string.h>
#include "y.tab.h"

int  yycol = 0;
char string_buf[100];
char *string_buf_ptr;
char const_buf[100];
char *const_buf_ptr;

%}
 /* All str and comment code with minor tweaks from: http://dinosaur.compilertools.net/flex/manpage.html because it took me a long time to understand custom actions */
%x str comment constchar

newline         \n
whitespace      [ \t]+
digit           [0-9]
number          0|[1-9]{digit}*
letter          [a-zA-Z]
identifier      {letter}[a-zA-Z0-9]* 
literal         {letter}+
charconst       ({letter}|{digit})

%%
\"	   string_buf_ptr = string_buf; BEGIN(str);
\'         const_buf_ptr = const_buf; BEGIN(constchar);
"/*"	   BEGIN(comment);

<str>\"	   { /* saw closing quote - all done */
             BEGIN(INITIAL);
             yycol+=yyleng;
             *string_buf_ptr = '\0';
             yytext = string_buf;
             return STRCONST;}

<str>\n	                  { /* error - unterminated string constant */
                            printf("Unterminated string (%d : %d)\n", yylineno, yycol);}

<str>\\n                  *string_buf_ptr++ = '\n'; 
<str>\\t                  *string_buf_ptr++ = '\t';
<str>\\r                  *string_buf_ptr++ = '\r';
<str>\\b                  *string_buf_ptr++ = '\b';
<str>\\f                  *string_buf_ptr++ = '\f';

<str>\\(.|\n)             *string_buf_ptr++ = yytext[1]; 

<str>[^\\\n\"]+	          {
		            char *yptr = yytext;
		            while ( *yptr )
			       *string_buf_ptr++ = *yptr++;}
<constchar>{charconst}    { *const_buf_ptr++ = yytext[0];}
<constchar>\'             { 
                             BEGIN(INITIAL); 
                             yycol+=yyleng; 
                             const_buf_ptr = '\0'; 
                             yytext = const_buf; 
                             return CHARCONST;}
<comment>[^*\n]*          { /* eat anything that's not a '*' */}
<comment>"*"+[^*/\n]*     {/* eat up '*'s not followed by '/'s */}
<comment>\n		  {yylineno++; yycol = 0;}
<comment>"*"+"/"          {BEGIN(INITIAL);}
<comment><<EOF>>          {printf("Unterminated comment (%d : %d)\n", yylineno, yycol); yyterminate();}

{newline}                 {yycol = 0; yylineno++;}
{whitespace}              {/*ignore whitespace*/}

 /* reserved words */
if            {yycol+=yyleng; return KWD_IF;}
else          {yycol+=yyleng; return KWD_ELSE;}
while         {yycol+=yyleng; return KWD_WHILE;}
int           {yycol+=yyleng; return KWD_INT;}
string        {yycol+=yyleng; return KWD_STRING;}
char          {yycol+=yyleng; return KWD_CHAR;}
return        {yycol+=yyleng; return KWD_RETURN;}
void          {yycol+=yyleng; return KWD_VOID;}

 /* constants and identifiers */
{identifier}  {yycol+=yyleng; 
	       yylval.strval = (char *) malloc(yyleng *sizeof(char *)); 
               strcpy(yylval.strval, yytext); 
               return ID;}
{number}      {yycol+=yyleng; yylval.value = atoi(yytext); return INTCONST;}

 /* operators and symbols */
"+"           {yycol+=yyleng; return OPER_ADD;}
"-"           {yycol+=yyleng; return OPER_SUB;}
"*"           {yycol+=yyleng; return OPER_MUL;}
"/"           {yycol+=yyleng; return OPER_DIV;}
"<="          {yycol+=yyleng; return OPER_LTE;}
"<"           {yycol+=yyleng; return OPER_LT;}
">="          {yycol+=yyleng; return OPER_GTE;}
">"           {yycol+=yyleng; return OPER_GT;}
"!="          {yycol+=yyleng; return OPER_NEQ;}
"=="          {yycol+=yyleng; return OPER_EQ;}
"="           {yycol+=yyleng; return OPER_ASGN;}
"["           {yycol+=yyleng; return LSQ_BRKT;}
"]"           {yycol+=yyleng; return RSQ_BRKT;}
"{"           {yycol+=yyleng; return LCRLY_BRKT;}
"}"           {yycol+=yyleng; return RCRLY_BRKT;}
"("           {yycol+=yyleng; return LPAREN;}
")"           {yycol+=yyleng; return RPAREN;}
","           {yycol+=yyleng; return COMMA;}
";"           {yycol+=yyleng; return SEMICLN;}
"@"           {/* skip special mC */}
.             {return ILLEGAL_TOK;}

<<EOF>>       {yyterminate();}
%%


