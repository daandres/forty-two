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
 * checks whether the return type of a function and the type of the returned value match or not
 * @param functionReturnType the return type of the function as defined in the definition
 * @param returnedType the type of the value which should be returned
 * @return returns 0 for true (types match) or 1 for false (type mismatch)
 */
//=>STATUS: könnte funktionieren ;)
int CheckFunctionReturnTyp(types_t functionReturnType, types_t returnedType) {
	if (functionReturnType == intType && returnedType == intType) {
		return 0;
	}
	else if (functionReturnType == intArrayType && returnedType == intArrayType) {
		return 0;
	}
	else if (functionReturnType == voidType && returnedType == voidType) {
		return 0;
	}
	return 1;
}

/**
 * checks whether the type of a value matches the type of the variable the value should be assigned to
 * @param assignmentTarger the type of the variable to which the value is assigned
 * @param toAssign	the type of the value which should be assigned
 * @return returns 0 for true (types match) or 1 for false (type mismatch)
 */
//=>STATUS: könnte funktionieren ;)
int CheckAssignment(types_t assignmentTarger, types_t toAssign) {
	if(assignmentTarger == toAssign) return 0;
	return 1;
}

/**
 * 
 */
//=>STATUS: könnte funktionieren ;)
int CheckArithmeticalExpression(types_t initTypeOne, types_t initTypeTwo, types_t* returnValue) {
	if (initTypeOne == intType && initTypeTwo == intType) {
		returnValue = intType;
		return 0;
	}
	return 1;
}

/**
 * Check wether the definition of a function is compliant with its declaration.
 *
 * @param params parameters of the definition
 * @param funcName Name of the current function-context
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
