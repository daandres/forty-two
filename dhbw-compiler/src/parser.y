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
  sym_function sfun;
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
%type <airt>  function_definition function_declaration
%type <airt> stmt_list stmt stmt_block stmt_conditional stmt_loop expression function_call primary function_call_parameters
%type <etyp> type
%type <svar> function_parameter identifier_declaration variable_declaration function_header
%type <lexem> function_context_declaration
%%

/*****Epsilonproductions******/
function_context_declaration
	:  /* empty */  { func_dec = 1; }  //Denotes, that the following declarations(identifiers) are for parameter-purpose only
/****************************/

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
     : variable_declaration COMMA identifier_declaration	{	
									sym_variable var;
									if($3.varType == ArrayType){
    	 	 	 	 	 	 	 	 	 if($1.varType == intType) {
    	 	 	 	 	 	 	 	 		 var.varType = intArrayType;
    	 	 	 	 	 	 	 	 	 } else {
    	 	 	 	 	 	 	 	 		 yyerror("Error: Only Integer arrays are valid.");
    	 	 	 	 	 	 	 	 	 }
     	 	 	 	 	 	 	 	 } else {
     	 	 	 	 	 	 	 		 var.varType = $1.varType;
     	 	 	 	 	 	 	 	 }
									$$.varType = $1.varType;
									var.name = $3.name;
									if(function_context == NULL){
										insertVarGlobal(var.name, var);
									} else {
										insertVarLocal(var.name, function_context, var, 0);
									}
								}
     | type identifier_declaration	{	sym_variable var;
     									if($2.varType == ArrayType){
    	 	 	 	 	 	 	 	 	 if($1 == intType) {
    	 	 	 	 	 	 	 	 		 var.varType = intArrayType;
    	 	 	 	 	 	 	 	 		 } else {
    	 	 	 	 	 	 	 	 			 yyerror("Error: Only Integer arrays are valid.");
    	 	 	 	 	 	 	 	 		 }
     	 	 	 	 	 	 	 		 } else {
     	 	 	 	 	 	 	 			 var.varType = $1;
     	 	 	 	 	 	 	 		 }
										$$.varType = var.varType;
										var.name = $2.name;	
										if(function_context == NULL){
											insertVarGlobal(var.name, var);
										} else {
											insertVarLocal(var.name, function_context, var, 0);
										}
									}
     ;
     


identifier_declaration
     : ID BRACKET_OPEN NUM BRACKET_CLOSE	{ sym_variable var;
     	 	 	 	 	 	 	 	 	 	 var.name = $1;
     	 	 	 	 	 	 	 	 	 	 var.varType = ArrayType; //Type is not known yet.Thus we use the typeless ArrayType
     	 	 	 	 	 	 	 	 	 	 $$ = var;
     	 	 	 	 	 	 	 	 	 	 	 	 	 	 }
     | ID	{sym_variable var;
	 	 	 var.name = $1;
	 	 	 $$ = var;
     	 	 	 	 	 	 }
     ;

function_definition
     : function_header PARA_CLOSE BRACE_OPEN stmt_list BRACE_CLOSE
     | function_header function_parameter_list PARA_CLOSE BRACE_OPEN stmt_list BRACE_CLOSE
     ;

function_declaration
     : function_header PARA_CLOSE	{ 	 
    	 	 	 	 	 	 	 	 sym_function func;
    	 	 	 	 	 	 	 	 func.returnType = $1.varType;
     	 	 	 	 	 	 	 	 func.protOrNot = proto; 
     	 	 	 	 	 	 	printf("VAL: %s\n", function_context);
     	 	 	 	 	 	 	 	 if(insertFuncGlobal($1.name, func) != 1){
     	 	 	 	 	 	 	 		 yyerror("Error while declaring function ", $1.name, " Function was already declared.");
										 return;
									 }else{
										 printf("Function %s declared. \n\n", $1.name);
									 };
     	 	 	 	 	 	 	 	 	 	 	 	 	 } 
     | function_header function_parameter_list PARA_CLOSE { 
    	 	 	 	 	 	 	 	 	 	 	 	 	 	 sym_function func;
	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 func.returnType = $1.varType;
															 func.protOrNot = proto; //An welcher stelle muss 'no' gesetzt werden???
															 
															 
															 printf("VAL: %s\n", function_context);
															 
															 if(insertFuncGlobal($1.name, func) != 1){
																 yyerror("Error while declaring function ", $1.name, " Function was already declared.");
																 return;
															 }else{
																 printf("Function %s declared. \n\n", $1.name);
															 };
															 
<<<<<<< HEAD
															 //printf("Value: %s \n", $4.name);
															// insertVarLocal($4.name, $2, $4, 1);
=======
						     	 	 	 	 	 	 	 	 
															 
						     	 	 	 	 	 	 	 	 DL_FOREACH(param_list,fparam) {
						     	 	 	 	 	 	 	 		 var.varType = fparam->varType;
						     	 	 	 	 	 	 	 		 var.name = fparam->name;
						     	 	 	 	 	 	 	 		 
						     	 	 	 	 	 	 	 		 DL_DELETE(param_list,fparam);
						     	 	 	 	 	 	 	 		 
						     	 	 	 	 	 	 	 		 //TODO: Check for successfull insertion
						     	 	 	 	 	 	 	 		 insertVarLocal(var.name, function_context, var, 1);
						     	 	 	 	 	 	 	 	 }
						     	 	 	 	 	 	 	 	 
						     	 	 	 	 	 	 	 	 function_context = NULL;
>>>>>>> branch 'master' of https://code.google.com/p/forty-two/
														 }
     ;

//Selfmade function header to define the ID before the parameter_list and statements are processed
function_header
	: type ID PARA_OPEN {
							$$.varType = $1;
							$$.name = $2;
							function_context = $2;
												}
	;
	
function_parameter_list //TODO: proper function_parameter_list
     : function_parameter { /*$$ = &$1;*/}
     | function_parameter_list COMMA function_parameter {   /*sym_variable *var = (sym_variable *) malloc(sizeof(sym_variable)+sizeof($1)); 
     	 	 	 	 	 	 	 	 	 	 	 	 	 	int i;
     	 	 	 	 	 	 	 	 	 	 	 	 	 	int border = sizeof(var)/sizeof(var[0]);
     	 	 	 	 	 	 	 	 	 	 	 	 	 	
     	 	 	 	 	 	 	 	 	 	 	 	 	 	for(i = 0; i<border-1;i++){
    	 	 	 	 	 	 	 	 	 	 	 	 	 		var[i] = $1[i];
    	 	 	 	 	 	 	 	 	 	 	 	 	 	}
     	 	 	 	 	 	 	 	 	 	 	 	 	 	var[i+1] = $3;

     	 	 	 	 	 	 	 	 	 	 	 	 	 	$$ = var; *///Zu fehleranfŠllig/unpraktisch. versuche die produktion fŸr declaration/definition aufzuspalten

     	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 }
     ;
	
function_parameter
     : type identifier_declaration { sym_variable var;
									if($2.varType == ArrayType){
    	 	 	 	 	 	 	 	 	 if($1 == intType)
    	 	 	 	 	 	 	 	 	 {
    	 	 	 	 	 	 	 	 		 var.varType = intArrayType;
    	 	 	 	 	 	 	 	 	 }
    	 	 	 	 	 	 	 	 	 else
    	 	 	 	 	 	 	 	 	 {
    	 	 	 	 	 	 	 	 		 yyerror("Error: Only Integer arrays are valid.");
    	 	 	 	 	 	 	 	 		 return;
    	 	 	 	 	 	 	 	 	 };
     	 	 	 	 	 	 	 	 }else{
     	 	 	 	 	 	 	 		 var.varType = $1;
     	 	 	 	 	 	 	 	 }
									var.name = $2.name;
									$$=var;
     								//insertVarLocal(var.name, function_context, var, 1);
     	 	 	 	 	 	 	 	 //$$.varType = $2.varType; //Hab ich doch schon oben gemacht
    	 	 	 	 	 	 	 	 	 	 	 	 	 	 }
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
