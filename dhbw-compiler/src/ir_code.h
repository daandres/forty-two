/*
 * ir_code.h
 *
 *  Created on: Mar 18, 2012
 *      Author: andres
 */

#ifndef IR_CODE_H_
#define IR_CODE_H_

#include <stdio.h>
#define START 100

/*
 * This struct is for Expressions.
 */
typedef struct ETYPE {
		int* true;
		int* false;
		int* next;
		int quad;
		char place[5];
	} EXPTYPE;

enum opcode {
	ASS, ADD, SUB, MULt, DIV, MIN,

	IFEQ, IFNE, IFGT, IFGE, IFLT, IFLE, GOTO,

	RET, CALL

// MEMORY
};

void gen(enum opcode, int target, int op_left, int op_right, int paramcount);

char* Newtemp();
int* Makelist(int nquad);
int* Merge(int* list1, int* list2);
void Backpatch(int* list, int nquad);
void Gen();

#endif /* IR_CODE_H_ */
