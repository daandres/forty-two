/****************************************************************************/
/* This file was created for the forty-two yacp compiler project.		 */ 
/* Have a look at our code repository: https://code.google.com/p/forty-two/ */ 
/* Authors: Marcel Schroeder, Moritz Hader, Daniel Andres			    */ 
/****************************************************************************/
 
%{
  #include "ir_code.h"
	
  #include <stdio.h>
  #include <stdarg.h>
  #include <stdlib.h>
	
  #define YYERROR_VERBOSE
  int yylex(void);
  
  char* function_context = NULL;  //!=NULL wenn wir in einem Funktionscontext sind. Die Zeichenkette entspricht dann dem Namen der Funktion
  
  //Flag to indicate if the code is syntacitcal correct. 1 means correct, else not; Assume the code is correct, when not change this value
  int correct = 1;
%}
%code requires{
	#include "symtab.h"
	extern sym_union_t *sym_table;
	extern function_param_t *param_list;
}

%union {
	int i;
	char *lexem;
	IRTYPE_t airt; //for TAC generation
	types_t etyp;
	sym_union_t sunion;
	sym_function_t sfun;
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
%type <airt> stmt_list stmt stmt_block stmt_conditional stmt_loop expression function_call function_call_parameters
%type <etyp> type
%type <sunion> function_parameter identifier_declaration variable_declaration function_header
%type <lexem> function_def
%type <airt> M_svQuad M_NextListAndNewIRCLine M_nextAndsv primary
%%

/*****Epsilonproductions******/
function_def
	:  /* empty */  {  
		sym_function_t func; //The returntypeis left blank for now. will be added in the definition
		func.protOrNot = no;
		
		if(insertFuncGlobal(function_context, func) == 1){
			warning("Function %s was used before it was declared. Adding declaration automatically.", function_context);
		}else{
		}
		if(param_list != NULL){
			function_param_t *fparam;
			DL_FOREACH(param_list,fparam)
				debug("Variable: %s of Type:%d", fparam->name, fparam->varType);
			insertCallVarLocal(function_context, param_list);
			param_list = NULL;
		}
	} 

M_svQuad
	: /* empty */ { 
		$$.quad = nextquad;
	}
M_NextListAndNewIRCLine
	: /* empty */ { 
		$$.next = makelist(nextquad);
		genStmt(OP_GOTO, nextquad, nextquad);
	}
M_nextAndsv
	: M_NextListAndNewIRCLine M_svQuad { 
		$$.quad = $2.quad;
	}
/****************************/

program
	: program_element_list {
		if(correct == 1)
			info("\nSyntax correct!\n");
		else 
			warning("Syntax not correct! See Log for more information\n");
	}
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
	: INT {
		$$ = intType; /*Using typedefinitions from symtab.h instead of lexems for types*/
	} 
	| VOID {$$ = voidType;}
	;

variable_declaration //TODO: Check if all the variables have the same Type;
	: variable_declaration COMMA identifier_declaration	{	
		sym_union_t var;
		if($3.vof.symVariable.varType == ArrayType) {
			if($1.vof.symVariable.varType == intType) {
				var.vof.symVariable.varType = intArrayType;
			} else {
				yyerror("Error: Only Integer arrays are valid.");
				//exit(1);
			}
		} else {
			var.vof.symVariable.varType = $1.vof.symVariable.varType;
		}
		$$.vof.symVariable.varType = $1.vof.symVariable.varType;
		var.name = $3.name;
		if(function_context == NULL){
			insertVarGlobal(var.name, var.vof.symVariable);
		} else {
			insertVarLocal(var.name, function_context, var.vof.symVariable, 0);
		}
	}
	| type identifier_declaration	{
		sym_union_t var;
		if($2.vof.symVariable.varType == ArrayType){
			if($1 == intType) {
				var.vof.symVariable.varType = intArrayType;
			} else {
				yyerror("Error: Only Integer arrays are valid.");
    	 	 	//exit(1);
    	 	}
    	} else {
    		var.vof.symVariable.varType = $1;
		}
		$$.vof.symVariable.varType = var.vof.symVariable.varType;
		var.name = $2.name;	
		if(function_context == NULL){
			insertVarGlobal(var.name, var.vof.symVariable);
		} else {
			insertVarLocal(var.name, function_context, var.vof.symVariable, 0);
		}
	}
	;
	


identifier_declaration
	: ID BRACKET_OPEN NUM BRACKET_CLOSE	{ 
		sym_union_t var;
		var.name = $1;
		var.vof.symVariable.varType = ArrayType; //Type is not known yet.Thus we use the typeless ArrayType
		$$ = var;
	}
	| ID	{
		sym_union_t var;
	 	var.name = $1;
	  	$$ = var;
	}
	;

function_definition
	: function_header PARA_CLOSE BRACE_OPEN function_def stmt_list BRACE_CLOSE {    
		sym_union_t* function = searchGlobal($1.name);
		function->vof.symFunction.returnType = $1.vof.symFunction.returnType;
		param_list = NULL;
		function_context = NULL;
    }
	| function_header function_parameter_list PARA_CLOSE BRACE_OPEN function_def stmt_list BRACE_CLOSE {	
		//sym_union_t* function = (sym_union_t *) malloc(sizeof(sym_union_t));
		//if (function == NULL)
		//	yyerror("could not allocate memory");		
		sym_union_t* function = searchGlobal($1.name);
		function->vof.symFunction.returnType = $1.vof.symFunction.returnType;
		param_list = NULL;
		function_context = NULL;
	}
	;

function_declaration
	: function_header PARA_CLOSE	{
		sym_function_t func;
		func.returnType = $1.vof.symVariable.varType;
		func.protOrNot = proto; 
		if(insertFuncGlobal($1.name, func) != 0){
			yyerror("Error while declaring function %s. Function was already declared.", $1.name);
			//exit(1);
		}else{
			debug("Function %s declared. \n", $1.name);
		}
		param_list = NULL; //Set the listpointer to NULL, so the next declaration can start anew
		function_context = NULL;
	} 
	| function_header function_parameter_list PARA_CLOSE { 
    	sym_function_t func;
	 	func.returnType = $1.vof.symVariable.varType;
		func.protOrNot = proto; //An welcher stelle muss 'no' gesetzt werden???
		if(insertFuncGlobal($1.name, func) != 0){
			yyerror("Error while declaring function %s. Function was already declared.", $1.name);
			//exit(1);
		}else{
			debug("Function %s declared. \n", $1.name);
		}
		function_param_t *fparam;
 	 	DL_FOREACH(param_list,fparam) 
 	 		debug("Variable: %s of Type:%d\n", fparam->name, fparam->varType);
		insertCallVarLocal(function_context, param_list);
		param_list = NULL; //Set the listpointer to NULL, so the next declaration can start anew
		function_context = NULL;
	}
	;

//Selfmade function header to define the ID before the parameter_list and statements are processed
function_header
	: type ID PARA_OPEN {
		$$.vof.symFunction.returnType= $1;
		$$.name = $2;
		function_context = $2;
	}
	;
	
	
function_parameter_list
	: function_parameter { 
		function_param_t* param = (function_param_t*)malloc(sizeof(function_param_t));
		if(param == NULL){
			warning("could not allocate memory");
			 	//exit(1); 
		}
		param->name = $1.name;
		param->varType = $1.vof.symVariable.varType;
		DL_APPEND(param_list,param);
	}
	| function_parameter_list COMMA function_parameter {  
		function_param_t* param = (function_param_t*)malloc(sizeof(function_param_t));
		if(param == NULL){
			warning("could not allocate memory");
			//exit(1); 
		}
		param->name = $3.name;
		param->varType = $3.vof.symVariable.varType;
		DL_APPEND(param_list,param);
	}
	;
	
function_parameter
	: type identifier_declaration { 
		sym_union_t var;
		if($2.vof.symVariable.varType == ArrayType){
    		if($1 == intType) {
    	 		var.vof.symVariable.varType = intArrayType;
    	 	} else {
    	 		yyerror("Error: Only Integer arrays are valid.");
    	 	 	//exit(1);
    	 	 }
	 	 }else{
			var.vof.symVariable.varType = $1;
		 }
		var.name = $2.name;
		$$=var;
    }
	;
									
stmt_list
	: /* empty: epsilon */

	| stmt_list stmt
	;

//TODO: OPTIONAL:Detect returnstatement when it is called and unreachable code is detected.	
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
	| expression EQ expression {
			//genStmt(OP_IFEQ, $1.idName, $3.idName, NULL, 3);
	}
	| expression NE expression {
			//genStmt(OP_IFNE, $1.idName, $3.idName, NULL, 3);
	}
	| expression LS expression {
			//genStmt(OP_IFLT, $1.idName, $3.idName, NULL, 3);
	} 
	| expression LSEQ expression {
			//genStmt(OP_IFLE, $1.idName, $3.idName, NULL, 3);
	} 
	| expression GTEQ expression {
			//genStmt(OP_IFGT, $1.idName, $3.idName, NULL, 3);
	} 
	| expression GT expression {
			//char* temp = newtemp();
			//genStmt(OP_IFGE, temp, $1.idName, $3.idName, 3);
	}
	| expression PLUS expression {
			//$$.idName = newtemp();
			//genStmt(OP_MUL, newtemp(), $1.idName, $3.idName, 3);
	}
	| expression MINUS expression {
			//strcpy($$.idName, newtemp());
			//genStmt(OP_SUB, $$.idName, $1.idName, $3.idName, 3);
	}
	| expression MUL expression {
			//strcpy($$.idName, newtemp());
			//genStmt(OP_ADD, $$.idName, $1.idName, $3.idName, 3);
	}
	| MINUS expression %prec UNARY_MINUS {
			//genStmt(OP_MIN, $2.idName, NULL, NULL, 1);
	}
	| PLUS expression %prec UNARY_PLUS {
			//genStmt(OP_ADD, $2.idName, 0, NULL, 2);
	}
	| ID BRACKET_OPEN primary BRACKET_CLOSE
	| PARA_OPEN expression PARA_CLOSE
	| function_call
	| primary { 
		$$.idName = $1.idName;
	}
	;

primary
	: NUM {
		//sprintf($$.idName, "%i", $1);
	}
	| ID {
    	if(function_context != NULL){
    		if(searchBoth($1, function_context) == NULL){
    	 	 	yyerror("Identifier %s has not been defined.",$1);
    	 	 	//exit(1);
    	 	 }
    	 } else {
    	 	yyerror("Identifiers can only be used within function context.");
    	 	//exit(1);
    	 }
    	 $$.idName = $1;
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
	correct = 0;
	if(cc_options.silent == 0){
	va_list ap;
	va_start(ap, s);

	if(yylloc.first_line)
		//fprintf(stderr, "%d.%d: ", yylloc.first_line, yylloc.first_column);
		fprintf(stderr, "%d.%d-%d.%d: ", yylloc.first_line, yylloc.first_column, yylloc.last_line, yylloc.last_column);
	vfprintf(stderr, s, ap);
	fprintf(stderr, "\n");
	}
}
