/*
 * ir_code.h
 *
 *  Created on: Mar 18, 2012
 *      Author: andres
 */

#ifndef IR_CODE_H_
#define IR_CODE_H_

#include <stdio.h>

enum opcode {
	ASSIGN,
	ADD,
	SUB,
	MUL,
	DIV,
	MINUS,
	//
	IFEQ,
	IFNE,
	IFGT,
	IFGE,
	IFLT,
	IFLE,
	GOTO,
	//
	RETURN,
	CALL
	//
	// MEMORY
};


void gen(enum opcode, int target, int op_left, int op_right, int paramcount);

void writeIrCode();

#endif /* IR_CODE_H_ */
