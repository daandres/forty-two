/*
 * ir_code.h
 *
 *  Created on: Mar 18, 2012
 *      Author: andres
 */

#ifndef IR_CODE_H_
#define IR_CODE_H_

#include <stdio.h>
#include "main.h"
#define START 100

/*
 * This struct is for Expressions.
 */
typedef struct IRTYPE {
	int* true; 		// true exit
	int* false;		// false exit
	int* next; 		// next exit
	int quad; 		// number of statement
	char* idName; 	// variable or temporary variable name
} IRT;

enum opcode {
	OP_ASSIGN, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_MIN,

	OP_IFEQ, OP_IFNE, OP_IFGT, OP_IFGE, OP_IFLT, OP_IFLE, OP_GOTO,

	OP_RETURN_VOID, OP_RETURN_VAL, OP_CALL_VOID, OP_PARAM,

	OP_ARRAY_LOAD, OP_ARRAY_STORE
};

void genStmt(enum opcode, int op_one, int op_two, int op_three, int paramcount);
void printIrCode(char* f);

char* newtemp();
int* makelist(int nquad);
int* merge(int* list1, int* list2);
void backpatch(int* list, int nquad);

#endif /* IR_CODE_H_ */
