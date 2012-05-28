/*
 * typecheck.c
 *
 *  Created on: Mar 8, 2012
 *      Author: marcel
 */

#include "typecheck.h"
#include "symtab.h"

//function_param_t *param_list_call = NULL;
//function_param_t *param_list_func = NULL;

int checkFunctionDefinition(function_param_t* params, char* funcName);

int CheckFunctionReturnTyp(int initTypeOne, int initTypeTwo) {
	types_t typeOne = initTypeOne;
	types_t typeTwo = initTypeTwo;
	if (typeOne == intType || typeOne == intType) {
		if (typeTwo == intType || typeTwo == intType || typeTwo == num) {
			return 0;
		}
	} else if (typeOne == intArrayType) {
		if (typeTwo == intArrayType) {
			return 0;
		}
	}
	return 1;
}

int CheckAssignment(int one, int two) {
	//Abgleich der Typen der Variablen
	return 0;
}

int CheckArithmeticalExpression(int initTypeOne, int initTypeTwo, int* returnValue) {
	types_t typeOne = initTypeOne;
	types_t typeTwo = initTypeTwo;
	if (typeOne == intType || typeOne == intType) {
		if (typeTwo == intType || typeTwo == intType || typeTwo == num) {
			returnValue = intType;
			return 0;
		}
	} else if (typeOne == intArrayType) {
		if (typeTwo == intArrayType) {
			returnValue = intArrayType;
			return 0;
		}
	}
	return 1;
}

/**
 * Check wether the definition of a function is compliant with its declaration.
 *
 * @parm params parameters of the definition
 * @parm funcName Name of the current function-context
 */
int checkFunctionDefinition(function_param_t* params, char* funcName){

	//1) Get Function entry to obtain the current parameter-list
	sym_union_t* function = searchGlobal(funcName);

	//2) Compare parameter-lists
	if (function != NULL && function->symbolType == symFunction) {

		if(function->vof.symFunction.callVar != NULL){
			if(params == NULL){
				return 1; //there is a declaration but the definition remains empty
			}
			else{
				//As both the definition and the declaration are not empty, we need to checkt the parametertypes
				return validateParameter(function->vof.symFunction.callVar, params);
			}
		}

		return 0;
	}
	else{
		return 1;
	}
}
