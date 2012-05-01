/*
 * typecheck.c
 *
 *  Created on: Mar 8, 2012
 *      Author: marcel
 */

#include "typecheck.h"

//function_param_t *param_list_call = NULL;
//function_param_t *param_list_func = NULL;

int checkAssignment (int initTypeOne, int initTypeTwo) {
	enum typesForCheck typeOne = initTypeOne;
	enum typesForCheck typeTwo = initTypeTwo;
	if(typeOne == intg || typeOne == intArrayEntry) {
		if(typeTwo == intg || typeTwo == intArrayEntry || typeTwo == num) {
			return 1;
		}
	}
	else if(typeOne == intArray) {
		if(typeTwo == intArray) {
			return 1;
		}
	}
	return 0;
}

int checkFunctionCall (int one, int two) {
	//Abgleich der Typen der Variablen
	return 0;
}

int checkOperation (int initTypeOne, int initTypeTwo, int* returnValue) {
	enum typesForCheck typeOne = initTypeOne;
	enum typesForCheck typeTwo = initTypeTwo;
	if(typeOne == intg || typeOne == intArrayEntry) {
		if(typeTwo == intg || typeTwo == intArrayEntry || typeTwo == num) {
			returnValue = intg;
			return 1;
		}
	}
	else if(typeOne == intArray) {
		if(typeTwo == intArray) {
			returnValue = intArray;
			return 1;
		}
	}
	return 0;
}
