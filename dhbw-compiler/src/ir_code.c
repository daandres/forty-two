/*
 * ir_code.c
 *
 *  Created on: Mar 18, 2012
 *      Author: andres
 */

#include "ir_code.h"
#include <stdlib.h>
#include <string.h>

int nextquad = START;	// addresse of TAC
IRC* code; 					// Code Blocks of TAC
int size = 0; 				// size of dynamic code array
static int count = 1;

/*
 * This fuction generats a new temp identifier for the TAC. (.t1, .t2)
 */
char* newtemp() {
	char* tmp = (char*) malloc(sizeof(char) * 5);
	sprintf(tmp, ".t%0d", count++);
	return tmp;
}
/*
 * This function generates a true/falselist with a given quadruple  (next quad = nquad)
 */
int* makelist(int nquad) {
	int* list = (int*) malloc(sizeof(int) * 15);
	list[0] = nquad;
	list[1] = 0;
	return list;
}
/*
 * This function merges two lists.
 */
int* merge(int* list1, int* list2) {
	int* temp = list1, count1 = 0, count2 = 0;
	// Zähle Elemente in Liste 1
	while (list1[count1] != 0)
		count1++;
	// Hänge Liste 2 an das Ende von Liste 1
	// FIXME list hat genau 15 Elemente Platz --> dymanisch mit realloc
	while (list1[count2] != 0) {
		list1[count1] = list2[count2];
		count1++;
		count2++;
	}
	return temp;
}

/**
 * This function appends the jump markers to the statements in the codes.
 */
void setMissingParm(int index, char* value) {
	switch (code[index].paramcount) {
	case 3:
		code[index].op_three = value;
		break;
	case 2:
		code[index].op_two = value;
		break;
	case 1:
		code[index].op_one = value;
		break;
	case 0:
		break;
	}
}

/*
 * This function backpatches temp identifiers with its addresses...
 */
void backpatch(int* list, int nquad) {
	char* addr = "";
	sprintf(addr, "%d", nquad);
	// Setze für jedes Listenelement die Adresse nquad
	while (*list != 0) {
		int index = *list++ - START;
		setMissingParm(index, addr);
	}
}

/*
 * This function generates a new entry in code array and the next instruction number.
 */
void genStmt(int quad, enum opcode op, char* op_one, char* op_two, char* op_three, int paramcount) {
	size++;
	code = (IRC *) realloc(code, size * sizeof(IRC));
	(code + size - 1)->quad = quad;
	(code + size - 1)->op = op;
	strcpy((code + size - 1)->op_one, op_one);
	strcpy((code + size - 1)->op_two, op_two);
	strcpy((code + size - 1)->op_three, op_three);
	(code + size - 1)->paramcount = paramcount;
	nextquad++;
}

char* formatIrCode(int i) {
	char * s = "";
	switch (code[i].op) {
	case OP_ASSIGN:
		sprintf(s, "%d\t%s = %s", code[i].quad, code[i].op_one, code[i].op_two);
		break;
	case OP_ADD:
		sprintf(s, "%d\t%s = %s + %s", code[i].quad, code[i].op_one, code[i].op_two, code[i].op_three);
		break;
	case OP_SUB:
		sprintf(s, "%d\t%s = %s - %s", code[i].quad, code[i].op_one, code[i].op_two, code[i].op_three);
		break;
	case OP_MUL:
		sprintf(s, "%d\t%s = %s * %s", code[i].quad, code[i].op_one, code[i].op_two, code[i].op_three);
		break;
	case OP_DIV:
		sprintf(s, "%d\t%s = %s / %s", code[i].quad, code[i].op_one, code[i].op_two, code[i].op_three);
		break;
	case OP_MOD:
		sprintf(s, "%d\t%s = %s %s %s", code[i].quad, code[i].op_one, code[i].op_two, "%", code[i].op_three);
		break;
	case OP_MIN:
		sprintf(s, "%d\t%s = - %s", code[i].quad, code[i].op_one, code[i].op_two);
		break;
	case OP_IFEQ:
		sprintf(s, "%d\tIF %s == %s GOTO %s", code[i].quad, code[i].op_one, code[i].op_two, code[i].op_three);
		break;
	case OP_IFNE:
		sprintf(s, "%d\tIF %s != %s GOTO %s", code[i].quad, code[i].op_one, code[i].op_two, code[i].op_three);
		break;
	case OP_IFGT:
		sprintf(s, "%d\tIF %s > %s GOTO %s", code[i].quad, code[i].op_one, code[i].op_two, code[i].op_three);
		break;
	case OP_IFGE:
		sprintf(s, "%d\tIF %s >= %s GOTO %s", code[i].quad, code[i].op_one, code[i].op_two, code[i].op_three);
		break;
	case OP_IFLT:
		sprintf(s, "%d\tIF %s < %s GOTO %s", code[i].quad, code[i].op_one, code[i].op_two, code[i].op_three);
		break;
	case OP_IFLE:
		sprintf(s, "%d\tIF %s <= %s GOTO %s", code[i].quad, code[i].op_one, code[i].op_two, code[i].op_three);
		break;
	case OP_GOTO:
		sprintf(s, "%d\tGOTO %s", code[i].quad, code[i].op_one);
		break;
	case OP_RETURN_VOID:
		sprintf(s, "%d\tRETURN", code[i].quad);
		break;
	case OP_RETURN_VAL:
		sprintf(s, "%d\tRETURN %s", code[i].quad, code[i].op_one);
		break;
	case OP_CALL_VOID:
		sprintf(s, "%d\tCALL %s, (%s)", code[i].quad, code[i].op_one, code[i].op_two);
		break;
	case OP_CALL_RET:
		sprintf(s, "%d\t%s = CALL %s, (%s)", code[i].quad, code[i].op_one, code[i].op_two, code[i].op_three);
		break;
	case OP_ARRAY_LOAD:
		sprintf(s, "%d\t%s = %s[%s]", code[i].quad, code[i].op_one, code[i].op_two, code[i].op_three);
		break;
	case OP_ARRAY_STORE:
		sprintf(s, "%d\t%s[%s] = %s", code[i].quad, code[i].op_one, code[i].op_two, code[i].op_three);
		break;
	}
	return s;
}

void printIrCode(char* fn) {
	FILE *f;
	f = fopen(fn, "a");
	if (f == NULL) {
		fprintf(stderr, "Fehler beim oeffnen der IR Datei.\n");
		return;
	}
	fprintf(f, "/* **************** */\n");
	fprintf(f, "/* IR code          */\n\n");

	char* tmp;
	for (int i = 0; i < size; i++) {
		tmp = formatIrCode(i);
		fprintf(f, "%s\n", tmp);
	}

	fprintf(f, "\n/* IR code End      */\n");
	fprintf(f, "/* **************** */\n\n");
	fclose(f);
	debug("IR code printed\n");
}
