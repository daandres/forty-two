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
IRCODE_t* code_quad; 		// Pointer to current TAC quadrupel
IRCODE_t* first_code_quad; // First Quadrupel
int size = 0; 					// size of dynamic code array
int tmpCount = TMP;			// temp identifier code number

/*
 * This fuction generats a new temp identifier for the TAC. (.t1, .t2, ...)
 */
char* newtemp() {
	char* tmp = (char *) malloc(sizeof(char) * 6); // don't forget End of STring symbol during the malloc
	if (tmp == NULL) {
		warning("could not allocate memory");
		return NULL;
	}
	sprintf(tmp, ".t%d", tmpCount++);
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
}
/*
 * This function merges two lists.
 */
IRLIST_t* merge(IRLIST_t* list1, IRLIST_t* list2) {

	IRLIST_t* mergedlist = list1;
	if (list1 != NULL) { // Gehe zum letzten Element der ersten Liste
		while (list1->next != NULL)
			list1 = list1->next;
		// Hänge zweite Liste an die erste Liste...
		list1->next = list2;
		return mergedlist; // Hier wird mergedlist zurückgegeben, da dies der Anfang von der gemeinsamen liste 1 und 2 ist
	} else
		// falls erste Liste null, dann gebe die zweite wieder zurück...
		return list2;
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
	IRLIST_t* first_elemt = list;
	char* addr = (char *) malloc(sizeof(char) * 10); // don't forget End of String symbol during the malloc
	if (addr == NULL) {
		warning("could not allocate memory");
	}
	sprintf(addr, "%d", nquad);
	// Setze für jedes Listenelement die Adresse nquad
	while (list != NULL) {
		setMissingParm(list->item, addr);
		list = list->next;
	}

	//free(addr); // wenn einkommentiert, dann gibt es invald reads auf addr, was sehr komsich ist TODO
	free_IRLIST_t_rec(first_elemt);
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

	code_quad = (IRCODE_t *) malloc(sizeof(IRCODE_t));
	if (code_quad == NULL) {
		warning("could not allocate memory for code_quad");
		return NULL;
	}

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

/**
 * Changes an IR Code struct.
 */
//void changeIRCode(IRCODE_t* code_quad, enum opcode newop, char* op_one, char* op_two, char* op_three, int paramcount){
//	if(newop != -1)
//			code_quad->op = newop;
//	if(op_one != NULL)
//			code_quad->op_one = op_one;
//	if(op_two != NULL)
//			code_quad->op_two = op_two;
//	if(op_three != NULL)
//			code_quad->op_three = op_three;
//	if(paramcount != -1)
//			code_quad->paramcount = paramcount;
//}
//void setIRCodeToLastQuad(IRCODE_t* code) {
//	IRCODE_t* prev = code->previous;
//	IRCODE_t* next = code->next;
//}
/**
 * Löscht das aktuelle Quadrupel und setzt das vorige als aktuell. ACHTUNG: Memory wird hier nicht gefreed --> selbst machen
 */
void delLastQuad() {
	IRCODE_t* prev = code_quad->previous; // speicher das vorherige quadrupel in prev. wenn es keins gibt wird NULL gespeichert
	if (prev != NULL) // wenn prev != NULL ist soll von prev die Referenz zum nächsten QUadrupel gelöscht werden
		prev->next = NULL;
	code_quad = prev; // setze als aktuelles Quadrupel das vorherige, wenn prev ==  NULL dann gibt es kein Quadrupel mehr
	nextquad--; // dekrementiere Statement counter
}

void formatIrCode(char* code_string, IRCODE_t* i) {
	//Wähle anhand des Operators aus welcher String in s geschrieben werden soll
	switch (i->op) {
		case OP_ASSIGN:
			sprintf(code_string, "%d\t%s = %s", i->quad, i->op_one, i->op_two);
			break;
		case OP_ADD:
			sprintf(code_string, "%d\t%s = %s + %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_SUB:
			sprintf(code_string, "%d\t%s = %s - %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_MUL:
			sprintf(code_string, "%d\t%s = %s * %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_DIV:
			sprintf(code_string, "%d\t%s = %s / %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_MOD:
			sprintf(code_string, "%d\t%s = %s %s %s", i->quad, i->op_one, i->op_two, "%", i->op_three);
			break;
		case OP_MIN:
			sprintf(code_string, "%d\t%s = - %s", i->quad, i->op_one, i->op_two);
			break;
		case OP_SHL:
			sprintf(code_string, "%d\t%s = %s << %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_SHR:
			sprintf(code_string, "%d\t%s = %s >> %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_IFEQ:
			sprintf(code_string, "%d\tIF %s == %s GOTO %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_IFNE:
			sprintf(code_string, "%d\tIF %s != %s GOTO %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_IFGT:
			sprintf(code_string, "%d\tIF %s > %s GOTO %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_IFGE:
			sprintf(code_string, "%d\tIF %s >= %s GOTO %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_IFLT:
			sprintf(code_string, "%d\tIF %s < %s GOTO %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_IFLE:
			sprintf(code_string, "%d\tIF %s <= %s GOTO %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_GOTO:
			sprintf(code_string, "%d\tGOTO %s", i->quad, i->op_one);
			break;
		case OP_RETURN_VOID:
			sprintf(code_string, "%d\tRETURN", i->quad);
			break;
		case OP_RETURN_VAL:
			sprintf(code_string, "%d\tRETURN %s", i->quad, i->op_one);
			break;
		case OP_CALL_VOID:
			sprintf(code_string, "%d\tCALL %s, (%s)", i->quad, i->op_one, i->op_two);
			break;
		case OP_CALL_RET:
			sprintf(code_string, "%d\t%s = CALL %s, (%s)", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_ARRAY_LOAD:
			sprintf(code_string, "%d\t%s = %s[%s]", i->quad, i->op_one, i->op_two, i->op_three);
			break;
		case OP_ARRAY_STORE:
			sprintf(code_string, "%d\t%s[%s] = %s", i->quad, i->op_one, i->op_two, i->op_three);
			break;
	}
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
	char* tmp = NULL;
	//Iteriere über alle Quadrupel, formatiere und schreibe sie in die Datei
	while (code_quad != NULL) {
		tmp = (char *) malloc(210); //sizeof(char) * ((63 * 3) + 6 + 1 + 13) + 1;// 3 mal identifier + quadruperl number + tab + operator signs and spaces + end of string symbol
		if (tmp == NULL) {
			warning("could not allocate memory for code_quad %d, therefore noe ir_code line was generated",
			      code_quad->quad);
			continue;
		}
		formatIrCode(tmp, code_quad);
		fprintf(f, "%s\n", tmp);
		code_quad = code_quad->next;
		free(tmp);
	}

	fprintf(f, "\n/* IR code End      */\n");
	fprintf(f, "/* **************** */\n\n");
	fclose(f);
	debug("IR code printed\n");
}

// Free functions
/**
 * Frees the memory for one IRCODE_T struct
 */
void free_IRCODE_t(IRCODE_t* var) {
	if (var->op_one != NULL)
		free(var->op_one);
	if (var->op_two != NULL)
		free(var->op_two);
	if (var->op_three != NULL)
		free(var->op_three);
	free(var);
}

/**
 * Frees the memory for one IRCODE_T struct recursively
 */
void free_IRCODE_t_rec(IRCODE_t* var) {
	IRCODE_t* next = var->next;
	IRCODE_t* prev = var->previous;
	while (next != NULL) {
		IRCODE_t* tmp = next->next;
		free_IRCODE_t(next);
		next = tmp;
		tmp = NULL;
	}
	while (prev != NULL) {
		IRCODE_t* tmp = prev->previous;
		free_IRCODE_t(prev);
		prev = tmp;
		tmp = NULL;
	}
}

/**
 * Frees the memory for one IRTYPE_t struct
 */
void free_IRTYPE_t(IRTYPE_t* var) {
	if (var->true != NULL)
		free_IRLIST_t_rec(var->true);
	if (var->false != NULL)
		free_IRLIST_t_rec(var->false);
	if (var->next != NULL)
		free_IRLIST_t_rec(var->next);
	if (var->idName != NULL)
		free(var->idName);
}

/**
 * Frees the memory for one IRLIST_t struct
 */
void free_IRLIST_t(IRLIST_t* var) {
//	if (var->item != NULL)			// wird nicht gefreed, da ja nur die Liste gefreed werden soll und nicht das element an sich
//		free_IRCODE_t(var->item);
	free(var);
}

/**
 * Frees the memory for one IRLIST_t structrecursively
 */
void free_IRLIST_t_rec(IRLIST_t* var) {
	if (var != NULL) {
		IRLIST_t* next = var->next;
		while (next != NULL) {
			IRLIST_t* tmp = next->next;
			free_IRLIST_t(next);
			next = tmp;
			tmp = NULL;
		}
	}
}
