/*
 * typecheck.h
 *
 *  Created on: Mar 8, 2012
 *      Author: marcel
 */

#ifndef TYPECHECK_H_
#define TYPECHECK_H_

#include "symtab.h"

/*typedef enum typesForCheck {
	voidType = 0, intg = 1, intArrayEntry = 2, intArray = 3, num = 4
} typesForCheck_t;
*/

int CheckArithmeticalExpression(types_t initTypeOne, types_t initTypeTwo, types_t* returnValue);
int checkFunctionDefinition(function_param_t* params, char* funcName);
int CheckAssignment(types_t assignmentTarger, types_t toAssign);
int CheckFunctionReturnTyp(char* funcname, types_t returnedType);

int validateDefinition(function_param_t* params, char* funcName);
int checkFunctionDefinition(function_param_t* params, char* funcName);
int validateParameter(function_param_t* first, function_param_t* second);

#endif /* TYPECHECK_H_ */
