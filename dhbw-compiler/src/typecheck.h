/*
 * typecheck.h
 *
 *  Created on: Mar 8, 2012
 *      Author: marcel
 */

#ifndef TYPECHECK_H_
#define TYPECHECK_H_

typedef enum typesForCheck {
	voidType = 0, intg = 1, intArrayEntry = 2, intArray = 3, num = 4
} typesForCheck_t;

int checkAssignment(int initTypeOne, int initTypeTwo);
int checkFunctionCall(int one, int two);
int checkOperation(int initTypeOne, int initTypeTwo, int* returnValue);

#endif /* TYPECHECK_H_ */
