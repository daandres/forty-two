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
char* Newtemp() {
	static int count = 1;
	char* ch = (char*) malloc(sizeof(char) * 5);
	sprintf(ch, ".t%d", count++);
	return ch;
}
/*
 * This function generates a true/falselist with a given quadruple  (next quad = nquad)
 */
int* Makelist(int nquad) {
	int* list = (int*) malloc(sizeof(int) * 15);
	list[0] = nquad;
	list[1] = 0;
	return list;
}
/*
 * This function merges two lists.
 */
int* Merge(int* list1, int* list2) {
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
void Backpatch(int* list, int nquad) {
	char addr[10];
	sprintf(addr, "%d", nquad);
	while (*list != 0) {
		int index = *list++ - START;
		strcat(code[index], addr);
	}
}

/*
 * This function generates a new quadrupel.
 */
void Gen() {
	nextquad++;
}
