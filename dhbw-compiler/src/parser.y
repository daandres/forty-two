/****************************************************************************/
/* This file was created for the forty-two yacp compiler project.		 */ 
/* Have a look at our code repository: https://code.google.com/p/forty-two/ */ 
/* Authors: Marcel Schroeder, Moritz Hader, Daniel Andres			    */ 
/****************************************************************************/
 
%{
	#include "ir_code.h"
	#include "typecheck.h"
	#include <stdio.h>
	#include <stdarg.h>
	#include <stdlib.h>
	#include <string.h>
	
	#define YYERROR_VERBOSE
	int yylex(void);
  	  
	//Funktionskontext. Enthaelt den Namen der Funktion, wenn er sich in ihr befindet. Anonsten wird der String '___#nktx&' abgespeichert.
	//Der Kryptische Name wurde gewaehlt, da es teilweise zu Problemen mit dem NULL-Wert kam.
	char* function_context = '___#nktx&';
  
	//Flag to indicate if the code is syntacitcal correct. 1 means correct, else not; Assume the code is correct, when not change this value
	int correct = 1;
  
	// This temporaray IRCODE pointer contains an IR Code for an Array Load. If the Array load was wrong it has to be changed and deleted
	IRCODE_t* arrayCodeTemp = NULL;
%}
%code requires{
	#include "symtab.h"
	extern sym_union_t *sym_table;
	extern function_param_t *param_list;
	extern int offset;
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
%token <lexem> NUM

%right ASSIGN 
%left  LOGICAL_OR
%left  LOGICAL_AND
%left  EQ NE
%left  LS LSEQ GTEQ GT 
%left  SHIFT_LEFT SHIFT_RIGHT
%left  PLUS MINUS
%left  MUL MOD DIV
%right LOGICAL_NOT UNARY_MINUS UNARY_PLUS
%left  BRACKET_OPEN BRACKET_CLOSE PARA_OPEN PARA_CLOSE

%type <etyp> type
%type <sunion> function_parameter identifier_declaration variable_declaration function_header 
%type <lexem> function_def
//%type <airt> program_element_list program_element 
%type <airt>  function_definition function_declaration 
%type <airt> stmt_list stmt stmt_block stmt_conditional stmt_loop 
%type <airt> expression function_call primary function_call_parameters
%type <airt> M_svQuad M_NextListAndGOTO 
%%


/****************************************************************************
*							Epsilonproductions								*
*																			*
****************************************************************************/

/**
 * Special epsilon-production to prepare the parameterlist for the subsequent statement-block.
 * Thus the semantic-ruleset inserts the function-context into the symbol-table and appends the 
 * parameter-list.
 */
function_def
	:  /* empty */  {  
		sym_function_t func; //The returntype is left blank for now. will be added in the definition
		func.returnType = None;
		func.protOrNot = proto;
		func.callVar = NULL;
		
		
		if(insertFuncGlobal(function_context, func)==1) { //Does the function allready exist? Otherwise it will be inserted
					
			
			//if the function allready exists, check if there is a parameter-missmatch
			if(checkFunctionDefinition(param_list, function_context) != 0) {
				
				yyerror("Conflicting parameter-types for function-definition '%s'",function_context);
			}
		}
		
		if(param_list != NULL){ //the new parameter-list overrides the one in the declaration. During declaration no intermediate code is created.
			insertCallVarLocal(function_context, param_list);
			//param_list = NULL;
		}
		genFuncNameQuad(function_context); // generate the name of function as IR Code
	} 

reset_param
	: /*empty*/ {
		PurgeParameters(param_list);//param_list = NULL; //Set the listpointer to NULL, so the next declaration can start anew
				param_list = NULL;
	}

M_svQuad
	: /* empty */ { 
		$$.quad = nextquad;
	}
M_NextListAndGOTO /*creates an empty goto statement and a nextlist*/
	: /* empty */ { 
		$$.next = makelist(genStmt(OP_GOTO, NULL, NULL, NULL, 1));
	}

M_NewLine /*creates a new line*/
	: /* empty */ { 
		genNewLine();
	}
/****************************************************************************
*								Program										*
*																			*
****************************************************************************/
/**
 * Root of the parse-tree that represents the programs-structure.
 */

program
	: program_element_list {
		if(correct == 1)
			info("\nSyntax correct!\n");
		else {
			warning("Syntax not correct! See Log for more information\n");
		//exit(1);
		}
	}
	;
	
/****************************************************************************
*							Symbol Table Part								*
*																			*
****************************************************************************/
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

/**
 * Defines the rulesets to detect the types INT and VOID (independent of array-indeces)
 */
type
	: INT {
		$$ = intType; /*Using typedefinitions from symtab.h instead of lexems for types*/
	} 
	| VOID {
		$$ = voidType;
	}
	;

/**
 * variable_declaration defines the ruleset that is required to detect and parse variable declarations. A
 * valid variable-declaration is either a type followed by an identifier-declaration or a variable declaration followed by a comma and an
 * identifier-declaration
 */
variable_declaration 
	: variable_declaration COMMA identifier_declaration	{	
		sym_union_t var;
		if($3.vof.symVariable.varType == ArrayType) {
			if($1.vof.symVariable.varType == intType || $1.vof.symVariable.varType == intArrayType) {
				var.vof.symVariable.varType = intArrayType;
				var.vof.symVariable.size = $3.vof.symVariable.size;
				
				//Calculate the offset for this type
				var.vof.symVariable.offsetAddress = offset; //add 4bytes (because of int) times the array size to the offset-counter
				offset += ($3.vof.symVariable.size * 4);
			} else {
				yyerror("Error: Only Integer arrays are valid.");
				//exit(1);
			}
		} else {
			var.vof.symVariable.varType = $1.vof.symVariable.varType;
			var.vof.symVariable.size = $3.vof.symVariable.size;
			
			if($3.vof.symVariable.varType == intType){ //check varType to determine offset, because otherwise it is void and of size 0
				//Calculate the offset for this type
				var.vof.symVariable.offsetAddress = offset; //add 4bytes (because of int) times the array size to the offset-counter
				offset += 4;
			} //else {
				//yyerror("Error: Only Integer variables are valid.");
				//exit(1);
			//}
		}
		$$.vof.symVariable.varType = $1.vof.symVariable.varType;
		var.name = $3.name;
		if(function_context == '___#nktx&'){
			if(insertVarGlobal(var.name, var.vof.symVariable) == 1){
				yyerror("Identifier %s has already been declared.", var.name);
			}
		} else {
			if(insertVarLocal(var.name, function_context, var.vof.symVariable, 0) == 1){
				yyerror("Identifier %s has already been declared in function %s.", var.name, function_context);
			}
		}
	}
	| type identifier_declaration {
		sym_union_t var;
		if($2.vof.symVariable.varType == ArrayType){
			if($1 == intType) {
				var.vof.symVariable.varType = intArrayType;
				var.vof.symVariable.size = $2.vof.symVariable.size;
				
				//Calculate the offset for this type
				var.vof.symVariable.offsetAddress = offset; //add 4bytes (because of int) times the array size to the offset-counter
				offset += ($2.vof.symVariable.size * 4);

			} else {
				yyerror("Error: Only Integer arrays are valid.");
    	 	 	//exit(1);
    	 	}
    	} else {
    		var.vof.symVariable.varType = $1;
    		var.vof.symVariable.size = $2.vof.symVariable.size;
    		
    		if($1 == intType){ //check varType to determine offset
    			//Calculate the offset for this type
				var.vof.symVariable.offsetAddress = offset; //add 4bytes (because of int) times the array size to the offset-counter
				offset += 4;
			} else {
				yyerror("Error: Only Integer variables are valid.");
				//exit(1);
			}
		}
		$$.vof.symVariable.varType = var.vof.symVariable.varType;
		$$.vof.symVariable.size = var.vof.symVariable.size;
		var.name = $2.name;	
		if(function_context == '___#nktx&'){
			if(insertVarGlobal(var.name, var.vof.symVariable) == 1){
				yyerror("Identifier %s has already been declared.", var.name);
			}
		} else {
			if(insertVarLocal(var.name, function_context, var.vof.symVariable, 0) == 1){
				yyerror("Identifier %s has already been declared in function %s.", var.name, function_context);
			}
		}
	}
	;

/**
 * Provides the ruleset do detect identifier-declarations for variables. An identifier-declaration can either be an identifier
 * or an identifier followed by an array-index.
 */
identifier_declaration
	: ID BRACKET_OPEN NUM BRACKET_CLOSE	{ 
		sym_union_t var;
		var.name = $1;
		var.vof.symVariable.varType = ArrayType; //Type is not known yet.Thus we use the typeless ArrayType
		var.vof.symVariable.size = atoi($3);
		$$ = var;
	}
	| ID { 
		sym_union_t var;
	 	var.name = $1;
	  	$$ = var;
		var.vof.symVariable.size = 1;
	}
	;
/*
 * Provides the Definition of a function. Contains the function-identifier, the parameterlist and the actual
 * body. The parameter-list, which is contained in the function_header, is processed using the function_def
 * rule.
 */
function_definition
	: function_header PARA_CLOSE BRACE_OPEN function_def {    
		sym_union_t* function = searchGlobal($1.name);
		
		if(function != NULL && function->vof.symFunction.protOrNot == proto){
			//The type None is set in the function_def in the case, that the function was defined before it was declared.
			//As the type is not known in function_def, it will be set after parsing the function_definition.
			if(function->vof.symFunction.returnType == None){
				function->vof.symFunction.returnType = $1.vof.symFunction.returnType;
			}
			
			if(function->vof.symFunction.returnType != $1.vof.symFunction.returnType){
				yyerror("Type-missmatch. The returntype for %s does not fit its declaration. Found (%s) but expected (%s).",$1.name, typeToString($1.vof.symFunction.returnType),typeToString(function->vof.symFunction.returnType));
			}
			
			//Declare the entry of the function to be no prototype anymore
			function->vof.symFunction.protOrNot = no;
		} else {
			yyerror("Duplicate declaration of function %s",function_context);
		}
	} stmt_list BRACE_CLOSE {
		function_context = '___#nktx&';
    }
	| function_header function_parameter_list PARA_CLOSE BRACE_OPEN function_def {	
		sym_union_t* function = searchGlobal($1.name);

		if(function->vof.symFunction.protOrNot == proto){
			//The type None is set in the function_def in the case, that the function was defined before it was declared.
			//As the type is not known in function_def, it will be set after parsing the function_definition.
			if(function->vof.symFunction.returnType == None){ 
				function->vof.symFunction.returnType = $1.vof.symFunction.returnType;
			}
			
			if(function->vof.symFunction.returnType != $1.vof.symFunction.returnType){
				yyerror("Type-missmatch. The returntype for %s does not fit its declaration. Found (%s) but expected (%s).",$1.name, typeToString($1.vof.symFunction.returnType),typeToString(function->vof.symFunction.returnType));
			}
			
			//Declare the entry of the function to be no prototype anymore
			function->vof.symFunction.protOrNot = no;
		}else{
			yyerror("Duplicate declaration of function %s",function_context);
		}
	}  stmt_list BRACE_CLOSE {					
		function_context = '___#nktx&';
	}
	;

/**
 * Ruleset to declare functions. Contains the function_header and optionally a function_parameter_list. Parameters are
 * directly inserted into the symbol-table(at the according function_context)
 */
function_declaration
	: function_header PARA_CLOSE	{
		sym_function_t func;
		func.returnType = $1.vof.symFunction.returnType;
		func.protOrNot = proto; 
		if(insertFuncGlobal($1.name, func) != 0){
			sym_union_t* entry = searchGlobal($1.name);
			
			
			if(entry != NULL && entry->vof.symFunction.callVar != NULL){
				//yyerror("Declaration of function %s: parameter-missmatch. Expected (%s) but found (%s)", $1.name, ParameterListToString(entry->vof.symFunction.callVar), "NULL");
				yyerror("Declaration of function %s: parameter-missmatch.", $1.name);
			}
			
			if(entry->vof.symFunction.returnType != $1.vof.symFunction.returnType){
				yyerror("Type-missmatch. The returntype for %s does not fit its declaration. Found (%s) but expected (%s).",$1.name, typeToString($1.vof.symFunction.returnType),typeToString(entry->vof.symFunction.returnType));
			}
			
			//exit(1);
		}else{
			debug("Function %s declared. \n", $1.name);
		}
		function_context = '___#nktx&';
	} 
	| function_header function_parameter_list PARA_CLOSE { 
    	sym_function_t func;
	 	func.returnType = $1.vof.symFunction.returnType;
		func.protOrNot = proto; 
		
		if(insertFuncGlobal($1.name, func) != 0){
			sym_union_t* entry = searchGlobal($1.name);
						
			if(validateDefinition(param_list, function_context)==1 ){
				//yyerror("Declaration of function %s: parameter-missmatch. Expected (%s) but found (%s)", $1.name, ParameterListToString(entry->vof.symFunction.callVar), ParameterListToString(param_list));
				yyerror("Declaration of function %s: parameter-missmatch.", $1.name);
			}
			
			if(entry->vof.symFunction.returnType != $1.vof.symFunction.returnType){
				yyerror("Type-missmatch. The returntype for %s does not fit its declaration. Found (%s) but expected (%s).",$1.name, typeToString($1.vof.symFunction.returnType),typeToString(entry->vof.symFunction.returnType));
			}
		}else{
			debug("Function %s declared. \n", $1.name);
			insertCallVarLocal(function_context, param_list);
		}
		
		function_param_t *fparam;
		
 	 	//DL_FOREACH(param_list,fparam) 
 	 		//debug("Variable: %s of Type:%d in function %s\n", fparam->name, fparam->varType, function_context);
 	 	
		//insertCallVarLocal(function_context, param_list);
		
		function_context = '___#nktx&';
	}
	;

/**
 * Selfmade function header to define the Identifier(ID) before the parameter_list and statements are processed
 */
function_header
	: type ID PARA_OPEN {
		$$.vof.symFunction.returnType= $1;
		$$.name = $2;
		function_context = $2;
		PurgeParameters(param_list);//param_list = NULL; //Set the listpointer to NULL, so the next declaration can start anew
		param_list = NULL;
	}
	;
	
/**
 * Provides the List of Parameters used by function-declarations and function-definitions.
 * The Parameter-List is stored in a global variable (linked-list) so the statement-blocks can access it while parsing.
 */	
function_parameter_list
	: function_parameter { 
		function_param_t* param = (function_param_t*)malloc(sizeof(function_param_t));
		if(param == NULL){
			yyerror("could not allocate memory");
			 	//exit(1); 
		}
		param->name = $1.name;
		param->varType = $1.vof.symVariable.varType;
		DL_APPEND(param_list,param);
	}
	| function_parameter_list COMMA function_parameter {  
		function_param_t* param = (function_param_t*)malloc(sizeof(function_param_t));
		if(param == NULL){
			yyerror("could not allocate memory");
			//exit(1); 
		}
		param->name = $3.name;
		param->varType = $3.vof.symVariable.varType;
		DL_APPEND(param_list,param);
	}
	;

/**
 * Function parameter consisting of a type followed by an identifier_declaraion.
 */
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
			
			
/****************************************************************************
*					Intermediate Code Generation Part						*
*																			*
****************************************************************************/	
								
stmt_list
	: /* empty: epsilon */{
	}

	| stmt_list M_svQuad stmt{
		backpatch($1.next, $2.quad); //patch the nextlist of stmt_list to M_svQuad
		//$$.next = $3.next; //the next statement after this stmt
	}
	;
/**
 * This ruleset defines general programm-statements. A statement can be a block, a variable, an expression,
 * controll- and looping-structures etc.
 */
stmt//TODO: OPTIONAL:Detect returnstatement when it is called and unreachable code is detected.	
	: stmt_block{
		$$.next = $1.next;
	}
	| variable_declaration SEMICOLON{
		$$.next = nextquad; //TODO: Check if nextquad comas after the declaration
	}
	| expression SEMICOLON{
		$$.next = $1.next;
	}
	| stmt_conditional{
		//$$.next = $1.next;
		backpatch($1.next, nextquad);
	}
	| stmt_loop{
		//$$.next = $1.next;
		backpatch($1.next, nextquad);
	}
	| RETURN expression SEMICOLON{ //Return that returns a actual value
		if(CheckFunctionReturnTyp(function_context, $2.type) == 0){
			backpatch($2.next,nextquad);//expression.next leads to the following statement
			genStmt(OP_RETURN_VAL, $2.idName, NULL, NULL, 1); // retrun value as the op says...
			//$$.next = nextquad; //the next statement after this is the nextquad
		} else {
			yyerror("Type missmatch in return statement. '%s' has not the return type '%s'", function_context, typeToString($2.type));	
		}
	}
	| RETURN SEMICOLON{//Empty returntype
		if(CheckFunctionReturnTyp(function_context, voidType) == 0){
			genStmt(OP_RETURN_VOID, NULL, NULL, NULL, 0); // retrun void as the op says...
			//$$.next = nextquad;
		} else {
			yyerror("Type missmatch in return statement. '%s' has not the return type 'void'", function_context);	
		}
	}
	| SEMICOLON /* empty statement */{
	}
	;

stmt_block
	: BRACE_OPEN stmt_list BRACE_CLOSE {
		$$.next = $2.next;
	}
	;
	
/**
 * Definitions to detect conditional-statements like if or if-else
 */
stmt_conditional
	: M_NewLine IF PARA_OPEN expression PARA_CLOSE M_svQuad stmt {
		backpatch($4.true, $6.quad); //backpatche den true Ausgang zum Statement der if Anweisung
		backpatch(merge($4.false, $4.next), nextquad); // backpatche den false Ausgang hinter die Statements der if anweisung
		genNewLine();
	}
	| M_NewLine IF PARA_OPEN expression PARA_CLOSE M_svQuad stmt ELSE M_NextListAndGOTO M_svQuad stmt { /* ELSE steht vor M_NextListAndGOTO damit es keine reduce/reduce conflict gibt*/
		backpatch($4.true, $6.quad); 	// backpatche true Ausgang mit true stmt block
		backpatch($4.false, $10.quad); 	// backpatche false Ausgang mit else stmt block
		backpatch($9.next, nextquad); 	// backpatche temp next list after true block mit dem nächsten quadrupel nach dem letzten stmt
										// falls expression true ist wird zum ersten M_svQUad gesprungen, die stmts ausgeführt und beim M_NextListAndGOTO über den False Teil hinweggesprungen; 
										// wenn expression false ist wird zum zweiten M_svQUad gesprungen
		//$$.next = merge($7.next, $11.next);
		//$$.next = merge($$.next, $9.quad); //FIXME: we would need to merge the nextlists with the quad for the 
		genNewLine();
	}
	;

/**
 * Definitions to detect loop-statements like while or Do-while.
 */
stmt_loop
	: M_NewLine WHILE PARA_OPEN M_svQuad expression PARA_CLOSE M_svQuad stmt{
		backpatch($5.true, $7.quad); 											// backpatche true ausgang mit begin des schleifen bodys
		backpatch(makelist(genStmt(OP_GOTO, NULL, NULL, NULL, 1)), $4.quad); 	//springe am Ende der Schleife immer wieder zum Kopf zurück, komplizierter Weg, aber so spart man sich eigenes allokieren con einem String hier im Parser		
		backpatch($5.false, nextquad); 											// backpatche sodass beim false ausgang aus der schleife herausgesprungen wird
		genNewLine();
	}
	| M_NewLine DO M_svQuad stmt WHILE PARA_OPEN M_svQuad expression PARA_CLOSE SEMICOLON{
		backpatch($8.true, $3.quad); 	//backpatche true ausgang mit begin der schleife
		backpatch($4.next, $7.quad);	//stmt.next wird zu expression hin gepatcht
		$$.next = $8.false; //Der false-fall der expression ist next
		//backpatch($8.false, nextquad); 	// backpatche fals ausgang mit ende der schleife
		
		backpatch(makelist(genStmt(OP_IFEQ, $8.idName, "1", NULL, 3)), $3.quad); // wenn die expression true (1) ist soll zurück zum anfang gesprungen werden
		genNewLine();
	}
	;
	
/**
 * Defines the set of possible expressions in the compiler/language. Possible expressions are assignments, comparisions, function-calls or
 * arithmetical expressions.
 */									
expression  /*Hier werden nicht genutzt Werte NULL gesetzt, damit klar ist was drin steht*/
	: expression ASSIGN expression {
				 
	    if ($1.lval == 1) { //Are the Types equal?
	   	 	 	if($1.type == intType){
	   	 	 		//Proceed for int
	   	 	 		if($3.type == intType || $3.type == intArrayType){
	   	 	 			//assign int
						$$.true = $3.true; // da ein gültiger lval weder eine ture/false/next liste hat kann die von $3 verwendet werden
						$$.false = $3.false; 
						$$.next = $3.next; 
						$$.quad = nextquad;
						$$.idName = $1.idName;
						$$.type = $1.type;
						$$.lval = 1; // TODO test:  a = b = c = 1;
						genStmt(OP_ASSIGN, $1.idName, $3.idName, NULL, 2);
	   	 	 		} else {
	   	 	 			yyerror("Typemissmatch in function %s. Illegal righthand-value. Not 'int' or 'int-Array'.", function_context);
	   	 	 		}
	   	 	 			
	   	 	 	} else if($1.type = intArrayType){
	   	 	 		if($3.type == intType || $3.type == intArrayType){
						//Proceed with array store
	   	 	 			if(code_quad !=NULL){ // wenn es kein vorheriges code_quad gibt wurde das array ohne direkten zugrif angesprochen (a anstatt a[i]), da das behandelte c subset keine pointer kennt kann a nichts zugewiesen werden. Wenn ein a[i] erkannt wurde wird ein code_quad erzeugt, somit lässt sich der richtige Zugriff erkennen
							IRCODE_t* temp_quad = code_quad; // aktuelles code_quad wird zwischengespeichert
							if(temp_quad->op_three == NULL) { // Falls kein index angegeben wurde wurde das array nicht richtig angesprochen -> Fehler
								yyerror("Type missmatch. Cannot use int Array without index.");
							}							

							delLastQuad(); //Lösche letztes Quadrupel da es eine falsche Array Operation war
						
							$$.true = $3.true; // da ein gültiger lval weder eine ture/false/next liste hat kann die von $3 verwendet werden
							$$.false = $3.false; 
							$$.next = $3.next; 
							$$.quad = nextquad;
							$$.idName = $1.idName;
							$$.type = intType;
							$$.lval = 1; // TODO es gibt doch auch sowas a = b = c = 1;
						
							genStmt(OP_ARRAY_STORE, temp_quad->op_two, temp_quad->op_three, $3.idName, 3);
							//free_IRCODE_t(temp_quad); // free den memory of altes aktuelles code_quad // gibt in valgrind invalid access wenn einkommentiert :(
							free(temp_quad); // only free struct but not the char* inside...
						} else {
							if($3.type == intType)
								yyerror("Type missmatch. Cannot assign int to an int Array (int[]=int)");
							else if ($3.type == intArrayType) 
								yyerror("Type missmatch. Cannot assign int* to an int Array (int[]=int*), because pointers aren't element to this c subset and in c it is also not possible.");
 							else
								yyerror("Type missmatch. Cannot assign int Array.");
						}
					} else {
						yyerror("Typemissmatch in function %s. Illegal righthand-value. Not 'int' or 'int-Array'.", function_context);
					}
	   	 	 	} else {
	   	 	 		yyerror("Typemissmatch in function %s. Illegal lefthand-value. Not 'int', 'int-Array' or numeric.", function_context);
	   	 	 	}
		}
	}
	| expression LOGICAL_OR M_svQuad expression {
		if (($1.type == intType || $1.type == intArrayType) && ($4.type == intType || $4.type == intArrayType)) {
			$$.idName = newtemp();

			backpatch($1.false, $3.quad); // False Ausgang von $1 springt zu $4
			IRCODE_t* truequad = genStmt(OP_ASSIGN, $$.idName, "1", NULL, 2); // wenn EQ true ist soll die expression den Wert 1 erhalten
			IRLIST_t* nextlist = makelist(genStmt(OP_GOTO, NULL, NULL, NULL, 1)); // überspringe den false wert  		
			genNewLine(); // zur lesbarkeit
			IRCODE_t* falsequad = genStmt(OP_ASSIGN, $$.idName, "0", NULL, 2); // wenn EQ true ist soll die expression den Wert 0 erhalten
			
			backpatch(merge($1.true, $4.true), truequad->quad);// True Ausgänge von $1 und $4 werden gemerged, da bei beiden die gesamte Expressieon true hat; backpatche mit truequad, sodass Wert 1 angenommen wird
			backpatch($4.false, falsequad->quad);// wenn auch noch $4 false ist, dann ist $$ auch false; backpatche mit falsequad, sodass Wert 0 angenommen wird
			backpatch(nextlist, nextquad);
			
			$$.true = NULL;
			$$.false = NULL;
			$$.next = NULL; 
			$$.quad = nextquad;
			$$.type = $1.type; // da $1.type und $4.type gleich sind ist es egal welches man nimmt
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Logical Or expressions are not both 'int', 'int-Array' or numeric.", function_context);
	   	}
	}
	| expression LOGICAL_AND M_svQuad expression {
		if (($1.type == intType || $1.type == intArrayType) && ($4.type == intType || $4.type == intArrayType)) {
			$$.idName = newtemp();
			
			backpatch($1.true, $3.quad); // True Ausgang von $1 springt zu $4
			
			IRCODE_t* truequad = genStmt(OP_ASSIGN, $$.idName, "1", NULL, 2); // wenn EQ true ist soll die expression den Wert 1 erhalten
			IRLIST_t* nextlist = makelist(genStmt(OP_GOTO, NULL, NULL, NULL, 1)); // überspringe den false wert  
			genNewLine(); // zur lesbarkeit
			IRCODE_t* falsequad = genStmt(OP_ASSIGN, $$.idName, "0", NULL, 2); // wenn EQ true ist soll die expression den Wert 0 erhalten
			
			backpatch($4.true, truequad->quad);// wenn auch noch $4 true ist, dann ist $$ auch true; backpatche mit truequad, sodass Wert 1 angenommen wird
			backpatch(merge($1.false, $4.false), falsequad->quad);// False Ausgänge von $1 und $4 werden gemerged, da bei beiden die gesamte Expressieon false hat; backpatche mit falsequad, sodass Wert 0 angenommen wird
			backpatch(nextlist, nextquad);
			
			$$.true = NULL;
			$$.false = NULL;
			$$.next = NULL; 
			$$.quad = nextquad;
			$$.type = $1.type; // da $1.type und $4.type gleich sind ist es egal welches man nimmt
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Logical And expressions are not both 'int', 'int-Array' or numeric.", function_context);
	   	}
	}
	| LOGICAL_NOT expression {
		if ($2.type == intType || $2.type == intArrayType) {
			$$.quad = nextquad;
			$$.type = $2.type;
			$$.idName = newtemp();
			$$.lval = 0;
			
			// eigentlich wird für logical_not kein IRCode benötigt, es müsten nur die true und false listen vertaischt werden... da aber in c logical_not ein integer wert zurück gibt wird hier trotzdem ircode erzeugt, auch wenn dieser manchmal dead_code ist (das muss dann später im ir_code wegoptimiert werden)
			
			genStmt(OP_ASSIGN, $$.idName, "1", NULL, 2); // assume $$.idName ist false (1)
			$$.true = merge($2.false, makelist(genStmt(OP_IFEQ, $2.idName, "0", NULL, 3))); // generiere Abfrage ob expression gleich 0 ist, erzeuge daraus eine Liste und merge die mit der übergebenen true Liste
			genStmt(OP_ASSIGN, $$.idName, "0", NULL, 2); // falss if oben falsch setze $$.idName zu true (0) 
			$$.false = merge($2.true, makelist(genStmt(OP_GOTO, NULL, NULL, NULL, 1))); // erzeuge GOTO als Liste die mit der fals Liste gemerged die neue false Liste ergibt
			$$.next = $2.next; 
		} else {
	   		yyerror("Typemissmatch in function %s. Logical Not expression is not 'int', 'int-Array' or numeric.", function_context);
	   	}
	}
	| expression EQ expression {
		if (($1.type == intType || $1.type == intArrayType) && ($3.type == intType || $3.type == intArrayType)) {
			$$.next = merge($1.next, $3.next); // merge hier da keine informationen für ein backpatch da sind 
			$$.quad = nextquad;
			$$.type = intType; // Ergebnis von Vergleichoperationen ist in C ein integer
			$$.idName = newtemp();
			genStmt(OP_ASSIGN, $$.idName, "1", NULL, 2); // wenn EQ true ist soll die expression den Wert 1 erhalten
			$$.true = makelist(genStmt(OP_IFEQ, $1.idName, $3.idName, NULL, 3)); // Generiere If Equal und erzeuge mit dem neuen quadrupel die TrueList.
			genStmt(OP_ASSIGN, $$.idName, "0", NULL, 2); // wenn EQ false ist soll die expression den Wert 0 erhalten
			$$.false = makelist(genStmt(OP_GOTO, NULL, NULL, NULL, 1)); // Generiere else und erzeuge mit dem neuen quadrupel die FalseList.
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Eq expressions are not both 'int', 'int-Array' or numeric.", function_context);
	   	}
	}
	| expression NE expression {
		if (($1.type == intType || $1.type == intArrayType) && ($3.type == intType || $3.type == intArrayType)) {
			$$.next =  merge($1.next, $3.next); // merge hier da keine informationen für ein backpatch da sind
			$$.quad = nextquad;
			$$.type = intType; // Ergebnis von Vergleichoperationen ist in C ein integer
			$$.idName = newtemp();
			genStmt(OP_ASSIGN, $$.idName, "1", NULL, 2); // wenn EQ true ist soll die expression den Wert 1 erhalten
			$$.true = makelist(genStmt(OP_IFNE, $1.idName, $3.idName, NULL, 3)); // Generiere If NOT Equal und erzeuge mit dem neuen quadrupel die TrueList.
			genStmt(OP_ASSIGN, $$.idName, "0", NULL, 2); // wenn EQ false ist soll die expression den Wert 0 erhalten
			$$.false = makelist(genStmt(OP_GOTO, NULL, NULL, NULL, 1)); // Generiere else und erzeuge mit dem neuen quadrupel die FalseList.
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Ne expressions are not both 'int', 'int-Array' or numeric.", function_context);
	   	}
	}
	| expression LS expression {
		if (($1.type == intType || $1.type == intArrayType) && ($3.type == intType || $3.type == intArrayType)) {
			$$.next =  merge($1.next, $3.next); // merge hier da keine informationen für ein backpatch da sind
			$$.quad = nextquad;
			$$.type = intType; // Ergebnis von Vergleichoperationen ist in C ein integer
			$$.idName = newtemp();
			genStmt(OP_ASSIGN, $$.idName, "1", NULL, 2); // wenn EQ true ist soll die expression den Wert 1 erhalten
			$$.true = makelist(genStmt(OP_IFLT, $1.idName, $3.idName, NULL, 3)); // Generiere If less then und erzeuge mit dem neuen quadrupel die TrueList.
			genStmt(OP_ASSIGN, $$.idName, "0", NULL, 2); // wenn EQ false ist soll die expression den Wert 0 erhalten
			$$.false = makelist(genStmt(OP_GOTO, NULL, NULL, NULL, 1)); // Generiere else und erzeuge mit dem neuen quadrupel die FalseList.
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Ls expressions are not both 'int', 'int-Array' or numeric.", function_context);
	   	}
	} 
	| expression LSEQ expression {
		if (($1.type == intType || $1.type == intArrayType) && ($3.type == intType || $3.type == intArrayType)) {
			$$.next = merge($1.next, $3.next); // merge hier da keine informationen für ein backpatch da sind
			$$.quad = nextquad;
			$$.type = intType; // Ergebnis von Vergleichoperationen ist in C ein integer
			$$.idName = newtemp();
			genStmt(OP_ASSIGN, $$.idName, "1", NULL, 2); // wenn EQ true ist soll die expression den Wert 1 erhalten
			$$.true = makelist(genStmt(OP_IFLE, $1.idName, $3.idName, NULL, 3)); // Generiere If less or Equal und erzeuge mit dem neuen quadrupel die TrueList.
			genStmt(OP_ASSIGN, $$.idName, "0", NULL, 2); // wenn EQ false ist soll die expression den Wert 0 erhalten
			$$.false = makelist(genStmt(OP_GOTO, NULL, NULL, NULL, 1)); // Generiere else und erzeuge mit dem neuen quadrupel die FalseList.
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Lseq expressions are not both 'int', 'int-Array' or numeric.", function_context);
	   	}
	} 
	| expression GTEQ expression {
		if (($1.type == intType || $1.type == intArrayType) && ($3.type == intType || $3.type == intArrayType)) {
			$$.next = merge($1.next, $3.next); // merge hier da keine informationen für ein backpatch da sind
			$$.quad = nextquad;
			$$.type = intType; // Ergebnis von Vergleichoperationen ist in C ein integer
			$$.idName = newtemp();
			genStmt(OP_ASSIGN, $$.idName, "1", NULL, 2); // wenn EQ true ist soll die expression den Wert 1 erhalten
			$$.true = makelist(genStmt(OP_IFGE, $1.idName, $3.idName, NULL, 3)); // Generiere If greater or Equal und erzeuge mit dem neuen quadrupel die TrueList.
			genStmt(OP_ASSIGN, $$.idName, "0", NULL, 2); // wenn EQ false ist soll die expression den Wert 0 erhalten
			$$.false = makelist(genStmt(OP_GOTO, NULL, NULL, NULL, 1)); // Generiere else und erzeuge mit dem neuen quadrupel die FalseList.
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Gteq expressions are not both 'int', 'int-Array' or numeric.", function_context);
	   	}
	} 
	| expression GT expression {
		if (($1.type == intType || $1.type == intArrayType) && ($3.type == intType || $3.type == intArrayType)) {
			$$.next = merge($1.next, $3.next); // merge hier da keine informationen für ein backpatch da sind
			$$.quad = nextquad;
			$$.type = intType; // Ergebnis von Vergleichoperationen ist in C ein integer
			$$.idName = newtemp();
			genStmt(OP_ASSIGN, $$.idName, "1", NULL, 2); // wenn EQ true ist soll die expression den Wert 1 erhalten
			$$.true = makelist(genStmt(OP_IFGT, $1.idName, $3.idName, NULL, 3)); // Generiere If greater then und erzeuge mit dem neuen quadrupel die TrueList.
			genStmt(OP_ASSIGN, $$.idName, "0", NULL, 2); // wenn EQ false ist soll die expression den Wert 0 erhalten
			$$.false = makelist(genStmt(OP_GOTO, NULL, NULL, NULL, 1)); // Generiere else und erzeuge mit dem neuen quadrupel die FalseList.
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Gt expressions are not both 'int', 'int-Array' or numeric.", function_context);
	   	}
	}
	| expression PLUS expression {
		if (($1.type == intType || $1.type == intArrayType) && ($3.type == intType || $3.type == intArrayType)) {
			$$.next = merge($1.next, $3.next); // merge hier da keine informationen für ein backpatch da sind
			$$.quad = nextquad;
			$$.type = intType; // Da nur int als Typ zur Berechnung zur Verfügung steht
			$$.idName = newtemp();
			$$.true = merge($1.true, $3.true); // merge hier da keine informationen für ein backpatch da sind
			$$.false = merge($1.false, $3.false); // merge hier da keine informationen für ein backpatch da sind
			genStmt(OP_ADD, $$.idName, $1.idName, $3.idName, 3);
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Plus expressions are not both 'int', 'int-Array' or numeric.", function_context);
	   	}
	}
	| expression MINUS expression {
		if (($1.type == intType || $1.type == intArrayType) && ($3.type == intType || $3.type == intArrayType)) {
			$$.next = merge($1.next, $3.next); // merge hier da keine informationen für ein backpatch da sind
			$$.quad = nextquad;
			$$.type = intType; // Da nur int als Typ zur Berechnung zur Verfügung steht
			$$.idName = newtemp();
			$$.true = merge($1.true, $3.true); // merge hier da keine informationen für ein backpatch da sind
			$$.false = merge($1.false, $3.false); // merge hier da keine informationen für ein backpatch da sind
			genStmt(OP_SUB, $$.idName, $1.idName, $3.idName, 3);
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Minus expressions are not both 'int', 'int-Array' or numeric.", function_context);
	   	}
	}
	| expression MUL expression {
		if (($1.type == intType || $1.type == intArrayType) && ($3.type == intType || $3.type == intArrayType)) {
			$$.next = merge($1.next, $3.next); // merge hier da keine informationen für ein backpatch da sind
			$$.quad = nextquad;
			$$.type = intType; // Da nur int als Typ zur Berechnung zur Verfügung steht
			$$.idName = newtemp();
			$$.true = merge($1.true, $3.true); // merge hier da keine informationen für ein backpatch da sind
			$$.false = merge($1.false, $3.false); // merge hier da keine informationen für ein backpatch da sind
			genStmt(OP_MUL, $$.idName, $1.idName, $3.idName, 3);
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Mul expressions are not both 'int', 'int-Array' or numeric.", function_context);
	   	}
	}
	| expression SHIFT_LEFT expression { //TODO
		if (($1.type == intType || $1.type == intArrayType) && ($3.type == intType || $3.type == intArrayType)) {
			$$.next = merge($1.next, $3.next); // merge hier da keine informationen für ein backpatch da sind
			$$.quad = NULL;
			$$.type = intType; // Da nur int als Typ zur Berechnung zur Verfügung steht
			$$.idName = newtemp();
			$$.true = merge($1.false, $3.false); // merge hier da keine informationen für ein backpatch da sind
			$$.false = merge($1.false, $3.false); // merge hier da keine informationen für ein backpatch da sind
			genStmt(OP_SHL, $$.idName, $1.idName, $3.idName, 3);
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Shift Left expressions are not both 'int', 'int-Array' or numeric.", function_context);
	   	}
	}
	| expression SHIFT_RIGHT expression { //TODO
		if (($1.type == intType || $1.type == intArrayType) && ($3.type == intType || $3.type == intArrayType)) {
			$$.next = merge($1.next, $3.next); // merge hier da keine informationen für ein backpatch da sind
			$$.quad = NULL;
			$$.type = intType; // Da nur int als Typ zur Berechnung zur Verfügung steht
			$$.idName = newtemp();
			$$.true = merge($1.true, $3.true); // merge hier da keine informationen für ein backpatch da sind
			$$.false = merge($1.false, $3.false); // merge hier da keine informationen für ein backpatch da sind
			genStmt(OP_SHR, $$.idName, $1.idName, $3.idName, 3);
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Shift Right expressions are not both 'int', 'int-Array' or numeric.", function_context);
	   	}
	}
	| expression MOD expression { //TODO test
		if (($1.type == intType || $1.type == intArrayType) && ($3.type == intType || $3.type == intArrayType)) {
			$$.next = merge($1.next, $3.next); // merge hier da keine informationen für ein backpatch da sind
			$$.quad = nextquad;
			$$.type = intType; // Da nur int als Typ zur Berechnung zur Verfügung steht
			$$.idName = newtemp();
			$$.true = merge($1.true, $3.true); // merge hier da keine informationen für ein backpatch da sind
			$$.false = merge($1.false, $3.false); // merge hier da keine informationen für ein backpatch da sind
			genStmt(OP_MOD, $$.idName, $1.idName, $3.idName, 3);
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Mod expressions are not both 'int', 'int-Array' or numeric.", function_context);
	   	}
	}
	| expression DIV expression { 
		if (($1.type == intType || $1.type == intArrayType) && ($3.type == intType || $3.type == intArrayType)) {
		
			// gen statement to check if $3 is 0 (div 0 not allowed)
			IRLIST_t* truelist = makelist(genStmt(OP_IFNE, $3.idName, "0", NULL, 3));
			IRLIST_t* falselist = makelist(genStmt(OP_GOTO, NULL, NULL, NULL, 1));		
			backpatch(truelist, nextquad); // setze true Ausgang auf das Statement mit der Berechnung
			
			$$.next = merge($1.next, $3.next); // merge hier da keine informationen für ein backpatch da sind
			$$.quad = nextquad;
			$$.type = intType; // Da nur int als Typ zur Berechnung zur Verfügung steht
			$$.idName = newtemp();
			$$.true = merge($1.true, $3.true); // merge hier da keine informationen für ein backpatch da sind
			$$.false = merge($1.false, $3.false); // merge hier da keine informationen für ein backpatch da sind
			genStmt(OP_DIV, $$.idName, $1.idName, $3.idName, 3);
			IRLIST_t* nextlist = makelist(genStmt(OP_GOTO, NULL, NULL, NULL, 1));	// nextlist um über die Zuweisung des neutralen Elements hiunwegzuspringen
			
			backpatch(falselist, nextquad); // setze false Ausgang der Überprüfung, ob 2. Operand 0 ist,  auf das Statement nach der Berechnung, da div 0 nicht erlaubt ist. // Ein Runtime Error wird nicht geworfen, da der Sprach Umfang zu klein ist			
			genStmt(OP_ASSIGN, $$.idName, "1", NULL, 2);	// Wenn div 0 ist wird der Wert auf 1 gesetzt ( neutrales Element der Divison)
			
			backpatch(nextlist, nextquad); // nextlist wird mit dem nächsten Element gebackpatcht...
			
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Div expressions are not both 'int', 'int-Array' or numeric.", function_context);
	   	}
	}
	| MINUS expression %prec UNARY_MINUS {
		if ($2.type == intType || $2.type == intArrayType) { // Hier auch ein Type checking machen, da nur int ein Minus haben dürfen. EIne FUnktion ohne Retrun nicht...
			$$.true = $2.true;
			$$.false = $2.false;
			$$.next = $2.next;
			$$.quad = nextquad;
			$$.type = $2.type;
			$$.idName = $2.idName;
			genStmt(OP_MIN, $2.idName, NULL, NULL, 1);
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Unary Minus expression is not 'int', 'int-Array' or numeric.", function_context);
	   	}
	}
	| PLUS expression %prec UNARY_PLUS {
		if ($2.type == intType || $2.type == intArrayType) { // Hier auch ein Type checking machen, da nur int ein Minus haben dürfen. EIne FUnktion ohne Retrun nicht...
			$$.true = $2.true;
			$$.false = $2.false;
			$$.next = $2.next;
			$$.quad = nextquad;
			$$.type = $2.type;
			$$.idName = newtemp();
			genStmt(OP_ADD, $$.idName, 0, $2.idName, 3);
			$$.lval = 0;
		} else {
	   		yyerror("Typemissmatch in function %s. Unary Plus expression is not 'int', 'int-Array' or numeric.", function_context);
	   	}
	}
	| ID BRACKET_OPEN primary BRACKET_CLOSE {
		if (/* TODO check types */1) { 
			$$.true = $3.true;
			$$.false = $3.false;
			$$.next = $3.next;
			$$.quad = nextquad;
			$$.type = intArrayType; // intArrayType da es hier nur int Array geben darf
			$$.idName = newtemp();
			$$.lval = 1;
			// ACHTUNG: hier wird angenommen, dass die Array Operation eine Load Operation ist. Ist dies nicht der Fall muss diese Operation gelöscht werden und eine Store Operation später hinzugefügt werden
			arrayCodeTemp = genStmt(OP_ARRAY_LOAD, $$.idName, $1, $3.idName, 3); 
		} else {
	   		yyerror("Typemissmatch in function %s. Array access", function_context);
	   	}
	}
	| PARA_OPEN expression PARA_CLOSE {
		$$.true = $2.true; 
		$$.false = $2.false;
		$$.next = $2.next; 
		$$.quad = $2.quad;
		$$.type = $2.type;
		$$.idName = $2.idName;
		$$.lval = 0;
	}
	| function_call {
		$$.true = $1.true; 
		$$.false = $1.false;
		$$.next = $1.next; 
		$$.quad = $1.quad;
		$$.idName = $1.idName;
		$$.type = $1.type;
		//printf("Function-Type: %d\n", $1.type);
		$$.lval = 0;
	}
	| primary { 
		$$.true = NULL; 
		$$.false = NULL;
		$$.next = NULL; 
		$$.quad = nextquad;
		$$.idName = $1.idName;
		$$.type = $1.type;
		$$.lval = $1.lval;
		//printf("Type: %d\n", $1.type);
	}
	;

/**
 * Defines primary-tokens that are used within expressions. Primary-tokens are either numbers or identifiers of
 * variables/functions defined before.
 */
primary
	: NUM {
		//sprintf($$.idName, "%d", $1);
    	$$.idName = $1;
		$$.type = intType;
		$$.lval = 0;
	}
	| ID { 
		if(function_context != '___#nktx&'){
			sym_union_t* found_entry = searchBoth($1, function_context);
			if(found_entry == NULL){
				yyerror("'%s' undeclared (first use in '%s')",$1, function_context);
			 	//exit(1);
			} else {
				$$.idName = $1;
				$$.type = found_entry->vof.symVariable.varType;
				$$.lval = 1;
			}
		} else {
			yyerror("Identifiers can only be used within function context.");
			//exit(1);
		}
	}
	;

/**
 * Ruleset to define the structure of functioncalls in the programming language. A functioncall can either
 * be empty (like fuu();) or contains a parameter-list (like fuu(1,2,a);)
 */
function_call
	 : ID PARA_OPEN PARA_CLOSE{
		//Check if the function was defined in symbol table
		sym_union_t* entry = searchGlobal($1);
		if(function_context != '___#nktx&'){
			if(entry != NULL){
				//Check if the function was defined
				if(entry->vof.symFunction.protOrNot != no){
					//yyerror("Function %s in '%s' was declared but never defined.",$1,function_context);
				} else {
					//Check if there is 'no' parameter-list, as there shouldn't be any.
					
					if(entry->vof.symFunction.callVar != NULL){
						//yyerror("Function %s in '%s': parameter-missmatch. Expected (%s) but found (NULL)", $1, function_context, ParameterListToString(entry->vof.symFunction.callVar));
						yyerror("Function '%s' in '%s': parameter-missmatch.)", $1, function_context);
					} else { 
						//TODO: Intermediate-Code for function-call
						if(entry->vof.symFunction.returnType == voidType || entry->vof.symFunction.returnType == None)
						genStmt(OP_CALL_VOID, $1, "", NULL, 2); 
						else {
							$$.idName = newtemp();
							genStmt(OP_CALL_RET, $$.idName, $1, "", 3); 
					 	}
					}
					//Set the return-value
					$$.type = entry->vof.symFunction.returnType;
				}
			} else {
				yyerror("Undefined symbol '%s' in function '%s'", $1,function_context);
			}
		} else {
			yyerror("Function-call '%s' can only be used within a function-context", $1, function_context);
		}
	}
	| ID PARA_OPEN reset_param function_call_parameters PARA_CLOSE {
		//Check if the function was defined in symbol table
		//warning("+++++++++++++++++++++\n\n");
		sym_union_t* entry = searchGlobal($1);
		
		if(function_context != '___#nktx&'){
			
			if(entry != NULL){
				//Check if the function was defined
				if(entry->vof.symFunction.protOrNot != no){
					//TODO: what to do if the definition of a function follows afterwards but it was declared???
					//yyerror("Function %s in '%s' was declared but never defined.",$1,function_context);
				} else {
					
					
					//Check if the parameter-list is available and correct (type)
					if(validateDefinition(param_list, $1) == 1){
						//yyerror("Function %s in '%s': parameter-missmatch. Expected (%s) but found (%s)", $1, function_context, ParameterListToString(entry->vof.symFunction.callVar), ParameterListToString(param_list));
						yyerror("Function '%s' in '%s': parameter-missmatch.", $1, function_context);
						
					} else {
						//TODO: Intermediate-Code for function-call
						if(entry->vof.symFunction.returnType == voidType || entry->vof.symFunction.returnType == None)
						genStmt(OP_CALL_VOID, $1, $4.idName, NULL, 2); 
						else {
							$$.idName = newtemp();
							genStmt(OP_CALL_RET, $$.idName, $1, $4.idName, 3); 
					 	}
					}
					//Set the return-value
					$$.type = entry->vof.symFunction.returnType;
				}
			} else {
				yyerror("Undefined symbol '%s' in function '%s'", $1, function_context);
			}
		} else {
			yyerror("Function-call '%s' can only be used within a function-context", $1);
		}
		
		//Clear the current param_list
		PurgeParameters(param_list);
		param_list = NULL;
	} 
	;

/**
 * Definition of the structure of the function-call-parameters. A function-call-parameter can be an arbitrary expression.
 */

function_call_parameters//We can reuse the param_list global-variable for the call_parameter-list, as it is will be overwritten after the call
	: function_call_parameters COMMA expression{
		function_param_t* param = (function_param_t*)malloc(sizeof(function_param_t));
		if(param == NULL){
			yyerror("could not allocate memory");
			//exit(1); 
		}
		param->name = $3.idName; //Generate a temporary name
		param->varType = $3.type; //obtain type from expression
		//printf("Type %d", $1.type);
		DL_APPEND(param_list,param);
		char * newIdName = (char *) malloc(strlen($1.idName) + 3 + strlen($3.idName)); // allokiere einen neuen String mit der länge vom neuen Parameteren und den bisher erkannten und dem trennzeichen ", "
		if(newIdName == NULL){
			warning("could not allocate memory for ne function call parameters string");
		}
		strcat(newIdName, $1.idName); // verknüpfe beide Strings in den neuen
		strcat(newIdName, ", "); // verknüpfe beide Strings in den neuen
		strcat(newIdName, $3.idName); // verknüpfe beide Strings in den neuen
		$$.idName = newIdName; 
	}
	| expression {
		function_param_t* param = (function_param_t*)malloc(sizeof(function_param_t));
		if(param == NULL){
			yyerror("could not allocate memory");
				//exit(1); 
		}
		
		param->name = $1.idName; //Obtain the temporary-name
		param->varType = $1.type; //obtain type from expression
		//printf("Type %d", $1.type);
		DL_APPEND(param_list,param);
		$$.idName = $1.idName;
	}
	;

%%

/**
* yyerror ist the error method for this parser. When it is called, the syntax will be makred as false and if the silent mode is deactivated, errors will beprinted on stderr.
* @parm like printf
*/
void yyerror(char *s, ...){
	correct = 0;	// set the syntax flag to 0, to indicate that the assumption of a correct syntax (1) was false
	if(cc_options.silent == 0){ // only proceed if the silent mode isn't activated
		va_list ap;			// new argument list
		va_start(ap, s);	// attach fucntion arguments to the newly crerated argument list ap
		
		if(yylloc.first_line)	// if there are location information print them
			//fprintf(stderr, "%d.%d: ", yylloc.first_line, yylloc.first_column);
			fprintf(stderr, "%d.%d-%d.%d: ", yylloc.first_line, yylloc.first_column, yylloc.last_line, yylloc.last_column);
		vfprintf(stderr, s, ap); // printf all items like printf from the arguments list
		fprintf(stderr, "\n");
	}
}
