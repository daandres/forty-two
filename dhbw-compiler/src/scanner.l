/****************************************************************************/
/* This file was created for the forty-two yacp compiler project.           */ 
/* Have a look at our code repository: https://code.google.com/p/forty-two/ */ 
/* Authors: Marcel Schroeder, Moritz Hader, Daniel Andres                   */ 
/****************************************************************************/

%{
    #include "ir_code.h"
 
	#include "parser.h"	
	#include <stdio.h>
	#include <string.h>
	/* handle locations */
	int yycolumn = 0;

	#define YY_USER_ACTION \
		yylloc.first_line = yylineno;			\
		yylloc.last_line = yylineno; 			\
    	yylloc.first_column = yycolumn;		\
    	yylloc.last_column = yycolumn + yyleng;	\
    	if (yyleng == 1 && strcmp(yytext,"\n")==0) {	\
      		yylloc.first_column = 0;			\
      		yylloc.last_column = 0;				\
      		yycolumn = 0;                       \
    	} else {								\
     		 yycolumn += yyleng;                \
    	}											
  	
  
%}

/* 
 * Default setting for flex which indicates that yytext, which is one of the main connections
 * between the scanner and parser, is handled as pointer of char instead of an array of char. 
*/
%pointer

/* save the line number of terminals */
%option yylineno

/* finish after scanning the provided file */
%option noyywrap		

number 		0|([1-9][0-9]*)
identifier 	([a-zA-Z]|\_)[0-9a-zA-Z_]*
comment 	\/\/(.)*\n

%%
do							{	return DO; }
while						{	return WHILE; }
if							{	return IF; }
else						{	return ELSE; }
int							{	return INT; }
void						{	return VOID; }
return						{	return RETURN; }
\:							{	return COLON; }
\,							{	return COMMA; }
\;							{	return SEMICOLON; }
\{							{	return BRACE_OPEN; }
\}							{	return BRACE_CLOSE; }

{number}					{	//printf("LEX NUM - %d\n", atoi(yytext));
								yylval.i = atoi(yytext);
								return NUM; }
{identifier}				{	//printf("LEX ID - %s\n", strdup(yytext));
								//yylval.lexem = strdup(yytext);
								yylval.lexem = yytext;
								return ID; }

\=\=						{	return EQ; }
\=							{	return ASSIGN; }
\<\<						{	return SHIFT_LEFT; }
\<							{	return LS; }
\>\>						{	return SHIFT_RIGHT; }
\>							{	return GT; }
\<=							{	return LSEQ; }
\>=							{	return GTEQ; }
\|\|						{	return LOGICAL_OR; }
\!							{	return LOGICAL_NOT; }
\&&							{	return LOGICAL_AND; }
\!=							{	return NE; }
\+							{	return PLUS; }
\-							{	return MINUS; }
\*							{	return MUL; }
\[							{	return BRACKET_OPEN; }
\]							{	return BRACKET_CLOSE; }
\(							{	return PARA_OPEN; }
\)							{	return PARA_CLOSE; }
{comment}					;

\n							;
[ \t]						;
.							{	fprintf(stderr,"Unexpected token!\n"); 
								yyterminate(); }
%%
//<<EOF>>					{	fprintf(stderr,"EOF\n");return END_OF_FILE; } auskommentiert, da EOF automatisch als Eingabe Ende erkannt wird...
