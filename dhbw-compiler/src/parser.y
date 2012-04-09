/* 
 * parser.y - Parser utility for the DHBW compiler
 */
 
%{


  #include "ir_code.h"
  #include <stdio.h>
  #include <stdarg.h>
  #include <stdlib.h>
	
  #define YYERROR_VERBOSE
  int yylex(void);
  
  char* function_context = NULL;  //!=NULL wenn wir in einem Funktionscontext sind. Die Zeichenkette entspricht dann dem Namen der Funktion
  
  
%}
%code requires{
#include "symtab.h"
 extern sym_union *sym_table;

}

%union {
  int i;
  char *lexem;
  IRT airt; //for TAC generation
  typeEnum etyp;
  sym_variable svar;
}
 
%debug
%locations
%error-verbose
%start program

/*
 * One shift/reduce conflict is expected for the "dangling-else" problem. This
 * conflict however is solved by the default behavior of bison for shift/reduce 
 * conflicts (shift action). The default behavior of bison corresponds to what
 * we want bison to do: SHIFT if the lookahead is 'ELSE' in order to bind the 'ELSE' to
 * the last open if-clause. 
 */
%expect 1

%token DO WHILE
%token IF ELSE
%token INT VOID
%token RETURN
%token COLON COMMA SEMICOLON
%token BRACE_OPEN BRACE_CLOSE

%token <lexem> ID
%token <i> NUM

%right ASSIGN 
%left  LOGICAL_OR
%left  LOGICAL_AND
%left  EQ NE     
%left  LS LSEQ GTEQ GT 
%left  SHIFT_LEFT SHIFT_RIGHT
%left  PLUS MINUS     
%left  MUL
%right LOGICAL_NOT UNARY_MINUS UNARY_PLUS
%left  BRACKET_OPEN BRACKET_CLOSE PARA_OPEN PARA_CLOSE

%type <airt> program_element_list program_element 
%type <airt> variable_declaration function_definition function_declaration function_parameter_list
%type <airt> stmt_list stmt stmt_block stmt_conditional stmt_loop expression function_call primary function_call_parameters
%type <etyp> type
%type <svar> function_parameter identifier_declaration
%%

program
     : program_element_list
     ;

program_element_list
     : program_element_list program_element 
     | program_element 
     ;

program_element
     : variable_declaration SEMICOLON
     | function_declaration SEMICOLON
     | function_definition
     | SEMICOLON
     ;
									
type
     : INT {$$ = intType;} /*Using typedefinitions from symtab.h instead of lexems for types*/
     | VOID {$$ = voidType;}
     ;

variable_declaration
     : variable_declaration COMMA identifier_declaration	{	}
     | type identifier_declaration	{	}
     ;

identifier_declaration
     : ID BRACKET_OPEN NUM BRACKET_CLOSE	{ $$ } /* Declaration fŸr function_parameter*/
     | ID	{	}
     ;

function_definition
     : type ID PARA_OPEN PARA_CLOSE BRACE_OPEN stmt_list BRACE_CLOSE
     | type ID PARA_OPEN function_parameter_list PARA_CLOSE BRACE_OPEN stmt_list BRACE_CLOSE
     ;

function_declaration
     : type ID PARA_OPEN PARA_CLOSE	{ sym_function func;
     	 	 	 	 	 	 	 	 func.returnType = $1;
     	 	 	 	 	 	 	 	 func.protOrNot = proto; //An welcher stelle muss 'no' gesetzt werden???
     	 	 	 	 	 	 	 	 
     	 	 	 	 	 	 	 	 if(insertFuncGlobal($2, func) == 1){
     	 	 	 	 	 	 	 		 printf("Fehler bei Funktionsdeklaration. Deklaration konnte nicht angelegt werden");
     	 	 	 	 	 	 	 	 }else{
     	 	 	 	 	 	 	 		 printf("Funktion %s wurde Deklariert.", $2);
     	 	 	 	 	 	 	 	 };
     	 	 	 	 	 	 	 	 	 	 	 	 	 } 
     | type ID PARA_OPEN function_parameter_list PARA_CLOSE { sym_function func;
	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 func.returnType = $1;
															 func.protOrNot = proto; //An welcher stelle muss 'no' gesetzt werden???
															 
															 //TODO: Parameterliste via InsertVarLocal einfŸgen.
															 
															 if(insertFuncGlobal($2, func) == 1){
																 printf("Fehler bei Funktionsdeklaration. Deklaration konnte nicht angelegt werden");
															 }else{
																 printf("Funktion %s wurde Deklariert.", $2);
															 };
														 }
     ;

function_parameter_list
     : function_parameter
     | function_parameter_list COMMA function_parameter
     ;
	
function_parameter
     : type identifier_declaration
     ;
									
stmt_list
     : /* empty: epsilon */
     | stmt_list stmt
     ;

stmt
     : stmt_block
     | variable_declaration SEMICOLON
     | expression SEMICOLON
     | stmt_conditional
     | stmt_loop
     | RETURN expression SEMICOLON
     | RETURN SEMICOLON
     | SEMICOLON /* empty statement */
     ;

stmt_block
     : BRACE_OPEN stmt_list BRACE_CLOSE
     ;
	
stmt_conditional
     : IF PARA_OPEN expression PARA_CLOSE stmt
     | IF PARA_OPEN expression PARA_CLOSE stmt ELSE stmt
     ;
									
stmt_loop
     : WHILE PARA_OPEN expression PARA_CLOSE stmt
     | DO stmt WHILE PARA_OPEN expression PARA_CLOSE SEMICOLON
     ;
									
expression
     : expression ASSIGN expression		
     | expression LOGICAL_OR expression
     | expression LOGICAL_AND expression
     | LOGICAL_NOT expression
     | expression EQ expression
     | expression NE expression
     | expression LS expression 
     | expression LSEQ expression 
     | expression GTEQ expression 
     | expression GT expression
     | expression PLUS expression
     | expression MINUS expression
     | expression MUL expression
     | MINUS expression %prec UNARY_MINUS
     | PLUS expression %prec UNARY_PLUS
     | ID BRACKET_OPEN primary BRACKET_CLOSE
     | PARA_OPEN expression PARA_CLOSE
     | function_call
     | primary
     ;

primary
     : NUM	{//$$ = $1.i;
				}
     | ID	{// TODO Suche von ID in SymTab --> sind wir in einer Funktion? Wenn ja, wie wissen wir das?
			//$$ = $1.id;
			}
     ;

function_call
      : ID PARA_OPEN PARA_CLOSE
      | ID PARA_OPEN function_call_parameters PARA_CLOSE
      ;

function_call_parameters
     : function_call_parameters COMMA expression
     | expression
     ;

%%

void yyerror(char *s, ...){
  va_list ap;
  va_start(ap, s);

  if(yylloc.first_line)
    fprintf(stderr, "%d.%d-%d.%d: error: ", yylloc.first_line, yylloc.first_column,
	    yylloc.last_line, yylloc.last_column);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}
