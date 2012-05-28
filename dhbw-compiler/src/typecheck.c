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




/**
 * 
 */
//=>STATUS: könnte funktionieren ;)
int CheckFunctionReturnTyp(types_t initTypeOne, types_t initTypeTwo) {
	if (initTypeOne == intType && initTypeTwo == intType) {
		return 0;
	}
	else if (initTypeOne == intArrayType && initTypeTwo == intArrayType) {
		return 0;
	}
	else if (initTypeOne == voidType && initTypeTwo == voidType) {
		return 0;
	}
	return 1;
}

/**
 * 
 */
//=>STATUS: könnte funktionieren ;)
int CheckAssignment(types_t assignmentTarger, types_t toAssign) {
	if(assignmentTarger == toAssign) return 0;
	return 1;
}

/**
 * 
 */
//=>STATUS: kein Plan
int CheckArithmeticalExpression(int initTypeOne, int initTypeTwo, int* returnValue) {
	types_t typeOne = initTypeOne;
	types_t typeTwo = initTypeTwo;
	if (typeOne == intType || typeOne == intType) {
		if (typeTwo == intType || typeTwo == intType) {
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
