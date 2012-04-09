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
typedef struct IRTYPE {
		int* true;
		int* false;
		int* next;
		int quad;
		char pos[5];
	} IRT;


enum opcode {
	OP_ASSIGN, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MIN,

	OP_IFEQ, OP_IFNE, OP_IFGT, OP_IFGE, OP_IFLT, OP_IFLE, OP_GOTO,

	OP_RETURN_VOID, OP_RETURN_VAL, OP_CALL_VOID, OP_PARAM,

	OP_ARRAY_LOAD, OP_ARRAY_STORE
};

void gen(enum opcode, int target, int op_left, int op_right, int paramcount);

char* Newtemp();
int* Makelist(int nquad);
int* Merge(int* list1, int* list2);
void Backpatch(int* list, int nquad);
void Gen();


#endif /* IR_CODE_H_ */
