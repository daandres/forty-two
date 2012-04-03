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

enum types {
	voidType = 0, intType = 1, intArrayType = 2
} typeEnum;

enum symbol {
	symFunction, symVariable
} symbolEnum;

enum prototype {
	no = 0, proto = 1
} prototypeEnum;

typedef struct sym_variable {
	char name[255];
	enum types varType;
	int* address;
	int size;
	UT_hash_handle hh; /* makes this structure hashable */
} sym_variable;

typedef struct sym_function {
	char name[255];
	enum types returnType;
	enum prototype;
	sym_variable* callVars;
	sym_variable* lokalVars;
	char* interCode; //TODO Pointer setzen;
} sym_function;

union variable_or_function{
	sym_variable symVariable;
	sym_function symFunction;
};

typedef struct sym_union {
	enum symbol symbolType; /* 1 -> variable; 0 -> function */
	union variable_or_function vof;
	UT_hash_handle hh; /* makes this structure hashable */
	char* name;
} sym_union;

sym_union* searchGlobal(); /* Kann Funktion und Variable zur�ckliefern */
sym_union* searchLocal(); /* Kann nur Variable zur�ckliefern */
int insertSymGlobal();
int insertSymLocal();

//ICH RAFFE DEN SCHEI? EINFACH NICHT

#endif /* SYMTAB_H_ */
