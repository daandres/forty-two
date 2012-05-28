/*
 * typecheck.h
 *
 *  Created on: Mar 8, 2012
 *      Author: marcel
 */

#ifndef TYPECHECK_H_
#define TYPECHECK_H_


/*typedef enum typesForCheck {
	voidType = 0, intg = 1, intArrayEntry = 2, intArray = 3, num = 4
} typesForCheck_t;
*/
int CheckFunctionReturnTyp(int initTypeOne, int initTypeTwo);
int CheckAssignment(int one, int two);
int CheckArithmeticalExpression(int initTypeOne, int initTypeTwo, int* returnValue);
int checkFunctionDefinition(function_param_t* params, char* funcName);

#endif /* TYPECHECK_H_ */
