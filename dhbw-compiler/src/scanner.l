/* 
 * scanner.l - Scanner utility for the DHBW compiler
 */

%{
	#include "parser.h"
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
<<EOF>>						{	return END_OF_FILE; }

[1-9][0-9]*|0				{	yylval.integer = atoi(yytext);
								return NUM; }
([a-zA-Z]|\_)[0-9a-zA-Z_]*	{	return ID; }

\=							{	return ASSIGN; }
\|\|						{	return LOGICAL_OR; }
\!							{	return LOGICAL_NOT; }
\&&							{	return LOGICAL_AND; }
\==							{	return EQ; }
\!=							{	return NE; }
\<							{	return LS; }
\<=							{	return LSEQ; }
\>=							{	return GTEQ; }
\>							{	return GT; }
\+							{	return PLUS; }
\-							{	return MINUS; }
\*							{	return MUL; }
\/							{	return DIV; }
\%							{	return MOD; }
\!							{	return NOT; }
\[							{	return BRACKET_OPEN; }
\]							{	return BRACKET_CLOSE; }
\(							{	return PARA_OPEN; }
\)							{	return PARA_CLOSE; }

[ \t\n]						;
.							{	printf("Unexpected token!\n"); 
								yyterminate(); }
%%