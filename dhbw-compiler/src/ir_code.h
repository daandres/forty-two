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
} IRTYPE_t;

typedef enum opcode {
	OP_ASSIGN, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_MIN,

	OP_IFEQ, OP_IFNE, OP_IFGT, OP_IFGE, OP_IFLT, OP_IFLE, OP_GOTO,

	OP_RETURN_VOID, OP_RETURN_VAL, OP_CALL_VOID, OP_CALL_RET,

	OP_ARRAY_LOAD, OP_ARRAY_STORE
} opcode_t;

typedef struct IRCODE {
	int quad;			// number of statement
	opcode_t op; 	// operator
	char* op_one;			// first or result operand
	char* op_two; 		// second or left operand
	char* op_three; 		// third or right operand
	int paramcount; 	// number of parameters... if an op needs less then three operands
	struct IRCODE* previous;		// Pointer to previous quadrupel
	struct IRCODE* self;			// Pointer to itself
	struct IRCODE* next;			// Pointer to next quadrupel
} IRCODE_t;

typedef struct IRLIST {
	struct IRLIST* self;			// Pointer to itself
	struct IRLIST* next;			// Pointer to next quadrupel
	IRCODE_t* item;			// Pointer to Quadrupel, which is element of this list
} IRLIST_t;

void genStmt(enum opcode op, char* op_one, char* op_two, char* op_three, int paramcount);
void printIrCode(char* f);

char* newtemp();
IRLIST_t* makelist(IRCODE_t* nquad);
IRLIST_t* merge(IRLIST_t* list1, IRLIST_t* list2);
void backpatch(IRLIST_t* list);


extern int nextquad;
#endif /* IR_CODE_H_ */
