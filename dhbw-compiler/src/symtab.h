/*
 * symtab.h
 *
 *  Created on: Mar 8, 2012
 *      Author: andres
 */

#ifndef SYMTAB_H_
#define SYMTAB_H_

#include "uthash.h"
#include <stdio.h>

/* Überhaupt benötigt? */
struct list_node {
	struct sym_variable content;
	struct var_list_node *next;
};

enum types {
	voidType, intType, intArrayType
};

struct sym_variable {
	char name[];
	enum type varType;
	int* address;
	int size;
	UT_hash_handle hh; /* makes this structure hashable */
};

struct sym_function {
	char name[];
	enum type returnType;
	/* callVars => einfach verkettete Liste<sym_variable> oder doch Hashmap */
	/* lokalVars => einfach verkettete Liste<sym_variable> oder doch Hashmap */
	char interCode[] /* evtl. durch Pointer ersetzen */;
};

union variable_or_function {
	struct sym_variable variable;
	struct sym_function function;
};

struct sym_union {
	int whichOne; /* 0 -> variable; 1 -> function */
	union variable_or_funtion;
	UT_hash_handle hh; /* makes this structure hashable */
};

typedef struct sym_variable sym_variable;

typedef struct sym_function sym_function;

typedef struct sym_union sym_union;

sym_union* searchGlobal(); /* Kann Funktion und Variable zurückliefern */
sym_variable* searchLocal(); /* Kann nur Variable zurückliefern */
int insertSymGlobal();
int insertSymLocal();

#endif /* SYMTAB_H_ */
