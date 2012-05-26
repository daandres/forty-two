/*
 * ir_code.c
 *
 *  Created on: Mar 18, 2012
 *      Author: andres
 */

#include "ir_code.h"
#include <stdlib.h>
#include <string.h>

int nextquad = START;		// addresse of TAC
//IRCODE_t* code; 			// Code Blocks of TAC
IRCODE_t* code_quad; 		// Pointer to current TAC quadrupel
IRCODE_t* first_code_quad; // First Quadrupel
int size = 0; 					// size of dynamic code array
int tmpCount = TMP;			// temp identifier code number

/*
 * This fuction generats a new temp identifier for the TAC. (.t1, .t2, ...)
 */
char* newtemp() {
	char* tmp = (char*) malloc(sizeof(char) * 5);
	if (tmp == NULL) {
		warning("could not allocate memory");
		//FIXME
		return (char*) 1;
	}
	sprintf(tmp, ".t%0d", tmpCount++);
	return tmp;
}
/*
 * This function generates a true/falselist with a given quadruple  (next quad = nquad)
 */
IRLIST_t* makelist(IRCODE_t* nquad) {

	IRLIST_t* list = (IRLIST_t*) malloc(sizeof(IRLIST_t));
	if (list == NULL) {
		warning("could not allocate memory for a new list");
		return NULL;
	}
	list->next = NULL;
	list->self = list;
	list->item = nquad;

	return list;
	//	int* list = (int*) malloc(sizeof(int) * 15);
	//	if (list == NULL) {
	//		warning("could not allocate memory");
	//		//FIXME list hat genau 15 Elemente Platz --> dymanisch mit realloc
	//		return (int*) 1;
	//	}
	//	list[0] = nquad;
	//	list[1] = 0;
	//	return list;
}
/*
 * This function merges two lists.
 */
IRLIST_t* merge(IRLIST_t* list1, IRLIST_t* list2) {

	// Gehe zum letzten Element der ersten Liste
	while (list1 != NULL)
		list1 = list1->next;
	// Hänge zweite Liste an die erste Liste...
	list1->next = list2;

	//	int* temp = list1, count1 = 0, count2 = 0;
	//	// Zähle Elemente in Liste 1
	//	while (list1[count1] != 0)
	//		count1++;
	//	// Hänge Liste 2 an das Ende von Liste 1
	//	// FIXME list hat genau 15 Elemente Platz --> dymanisch mit realloc
	//	while (list1[count2] != 0) {
	//		list1[count1] = list2[count2];
	//		count1++;
	//		count2++;
	//	}
	return list1;
}

/**
 * This function appends the jump markers to the statements in the codes.
 */
void setMissingParm(IRCODE_t* code, char* value) {
	switch (code->paramcount) {
		case 3:
			code->op_three = value;
			break;
		case 2:
			code->op_two = value;
			break;
		case 1:
			code->op_one = value;
			break;
		case 0:
			break;
	}
}

/*
 * This function backpatches temp identifiers with its addresses...
 */
void backpatch(IRLIST_t* list, int nquad) {
	char* addr = "";
	sprintf(addr, "%d", nquad);
	// Setze für jedes Listenelement die Adresse nquad
	while (list->next != NULL) {
		setMissingParm(list->item, addr);
		list = list->next;
	}
}

/*
 * This function generates a new entry in code array and the next instruction number.
 */
IRCODE_t* genStmt(enum opcode op, char* op_one, char* op_two, char* op_three, int paramcount) {
	size++;
	IRCODE_t* prev_code_quad;
	// Wenn das nicht das erste Quadrupel ist, speichere die Adresse in prev_code_quad, ansonsten setze das NULL
	if (code_quad != NULL)
		prev_code_quad = code_quad->self;
	else
		prev_code_quad = NULL;

	//code = (IRCODE_t *) realloc(code, size * sizeof(IRCODE_t));
	code_quad = (IRCODE_t *) malloc(sizeof(IRCODE_t));
	if (code_quad == NULL) {
		warning("could not allocate memory for code_quad");
		return NULL;
	}
	//	(code + size - 1)->quad = nextquad;
	//	(code + size - 1)->op = op;
	//	(code + size - 1)->op_one = op_one;
	//	(code + size - 1)->op_two = op_two;
	//	(code + size - 1)->op_three = op_three;
	//	(code + size - 1)->paramcount = paramcount;

	// setze vom vorherigen quadrupel das nächste quadrupel auf das neue
	if (prev_code_quad != NULL)
		prev_code_quad->next = code_quad;
	else
		first_code_quad = code_quad;

	// weise werte zu
	code_quad->quad = nextquad;
	code_quad->op = op;
	code_quad->op_one = op_one;
	code_quad->op_two = op_two;
	code_quad->op_three = op_three;
	code_quad->paramcount = paramcount;
	code_quad->self = code_quad;
	code_quad->previous = prev_code_quad;
	code_quad->next = NULL;

	//erhöhe Statement Zähler
	nextquad++;
	return code_quad;
}

char* formatIrCode(IRCODE_t* i) {
	char* s = "";
	//Wähle anhand des Operators aus welcher String in s geschrieben werden soll
	switch (i->op) {
		case OP_ASSIGN:
			sprintf(s, "%d\t%s = %s", i->quad, i->op_one, i->op_two);
			break;
		case OP_ADD:
			sprintf(s, "%d\t%s = %s + %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_SUB:
			sprintf(s, "%d\t%s = %s - %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_MUL:
			sprintf(s, "%d\t%s = %s * %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_DIV:
			sprintf(s, "%d\t%s = %s / %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_MOD:
			sprintf(s, "%d\t%s = %s %s %s", i->quad, i->op_one, i->op_two, "%", i->op_three);
			break;
		case OP_MIN:
			sprintf(s, "%d\t%s = - %s", i->quad, i->op_one, i->op_two);
			break;
		case OP_IFEQ:
			sprintf(s, "%d\tIF %s == %s GOTO %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_IFNE:
			sprintf(s, "%d\tIF %s != %s GOTO %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_IFGT:
			sprintf(s, "%d\tIF %s > %s GOTO %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_IFGE:
			sprintf(s, "%d\tIF %s >= %s GOTO %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_IFLT:
			sprintf(s, "%d\tIF %s < %s GOTO %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_IFLE:
			sprintf(s, "%d\tIF %s <= %s GOTO %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_GOTO:
			sprintf(s, "%d\tGOTO %s", i->quad, i->op_one);
			break;
		case OP_RETURN_VOID:
			sprintf(s, "%d\tRETURN", i->quad);
			break;
		case OP_RETURN_VAL:
			sprintf(s, "%d\tRETURN %s", i->quad, i->op_one);
			break;
		case OP_CALL_VOID:
			sprintf(s, "%d\tCALL %s, (%s)", i->quad, i->op_one, i->op_two);
			break;
		case OP_CALL_RET:
			sprintf(s, "%d\t%s = CALL %s, (%s)", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_ARRAY_LOAD:
			sprintf(s, "%d\t%s = %s[%s]", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_ARRAY_STORE:
			sprintf(s, "%d\t%s[%s] = %s", i->quad, i->op_one, i->op_two, i->op_three);
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

	code_quad = first_code_quad; // setze aktuelle code_quad wieder auf erstes...
	char* tmp;
	//Iteriere über alle Quadrupel, formatiere und schreibe sie in die Datei
	while (code_quad != NULL) {
		tmp = formatIrCode(code_quad);
		fprintf(f, "%s\n", tmp);
		code_quad = code_quad->next;
	}

	fprintf(f, "\n/* IR code End      */\n");
	fprintf(f, "/* **************** */\n\n");
	fclose(f);
	debug("IR code printed\n");
}
