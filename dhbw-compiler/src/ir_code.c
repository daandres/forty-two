/*
 * ir_code.c
 *
 *  Created on: Mar 18, 2012
 *      Author: andres
 */

#include "ir_code.h"
#include <stdlib.h>
#include <string.h>

int nextquad = START; // addresse of TAC
char code[25][50]; // Code Blocks of TAC

/*
 * This fuction generats a new temp identifier for the TAC. (.t1, .t2)
 */
char* newtemp() {
	static int count = 1;
	char* tmp = (char*) malloc(sizeof(char) * 5);
	sprintf(tmp, ".t%d", count++);
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
	while (list1[count1] != 0)
		count1++;
	while (list1[count2] != 0) {
		list1[count1] = list2[count2];
		count1++;
		count2++;
	}
	return temp;
}
/*
 * This function backpatches temp identifiers with its addresses...
 */
void backpatch(int* list, int nquad) {
	char addr[10];
	sprintf(addr, "%d", nquad);
	while (*list != 0) {
		int index = *list++ - START;
		strcat(code[index], addr);
	}
}

/*
 * This function generates a the next instruction number of the next quadrupel.
 */
void genStmt(enum opcode op, int op_one, int op_two, int op_three, int paramcount) {

	nextquad++;
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



	fprintf(f, "\n/* IR code End      */\n");
	fprintf(f, "/* **************** */\n\n");
	fclose(f);
	debug("IR code printed\n");
}
