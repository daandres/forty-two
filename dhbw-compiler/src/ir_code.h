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
#include "symtab.h"
#define START 100
#define TMP 100 // Counter für Temp Variables Names

/*
 * This struct is for Expressions.
 */
typedef struct IRTYPE {
	int* true; 		// true exit
	int* false;		// false exit
	int* next; 		// next exit
	int quad; 		// number of statement
	types_t type;	// type of terminal / non-terminal
	char* idName; 	// variable or temporary variable name
	int lval; //0=not a valid lvalv, 1=valid lval
} IRTYPE_t;

typedef enum opcode {
	OP_ASSIGN, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_MIN,

	OP_IFEQ, OP_IFNE, OP_IFGT, OP_IFGE, OP_IFLT, OP_IFLE, OP_GOTO,

	OP_RETURN_VOID, OP_RETURN_VAL, OP_CALL_VOID, OP_CALL_RET,

	OP_ARRAY_LOAD, OP_ARRAY_STORE
} opcode_t;

typedef struct IRCODE {
	int quad;					// number of statement
	opcode_t op; 				// operator
	char* op_one;				// first or result operand
	char* op_two; 				// second or left operand
	char* op_three; 			// third or right operand
	int paramcount; 			// number of parameters... if an op needs less then three operands
	struct IRCODE* previous;// Pointer to previous quadrupel
	struct IRCODE* self;		// Pointer to itself
	struct IRCODE* next;		// Pointer to next quadrupel
} IRCODE_t;

typedef struct IRLIST {
	struct IRLIST* self;		// Pointer to itself
	struct IRLIST* next;		// Pointer to next quadrupel
	IRCODE_t* item;			// Pointer to Quadrupel, which is element of this list
} IRLIST_t;

IRCODE_t* genStmt(enum opcode op, char* op_one, char* op_two, char* op_three, int paramcount);
void printIrCode(char* f);

char* newtemp();
IRLIST_t* makelist(IRCODE_t* nquad);
IRLIST_t* merge(IRLIST_t* list1, IRLIST_t* list2);
void backpatch(IRLIST_t* list, int nquad);
void changeIRCode(IRCODE_t* code_quad, enum opcode newop, char* op_one, char* op_two, char* op_three, int paramcount);
void delLastQuad();


extern int nextquad; // Number of next quadrupel, marked as extern, so that it is known in parser.y
extern IRCODE_t* code_quad;	// currrent code_quadrupel marked as extern, so that it is known in parser.y
#endif /* IR_CODE_H_ */
