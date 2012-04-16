/*
 * symtab.h
 *
 *  Created on: Mar 8, 2012
 *      Author: andres
 */

#ifndef SYMTAB_H_
#define SYMTAB_H_

#include "uthash.h"
#include "utlist.h"
#include <stdio.h>

//Moritz: Habe types um ArrayType erg�nzt
typedef enum types {
	voidType = 0, intType = 1, intArrayType = 2, ArrayType = 3
} typeEnum;

typedef enum symbol {
	symFunction, symVariable
} symbolEnum;

typedef enum prototype {
	no = 0, proto = 1
} prototypeEnum;

typedef struct sym_variable {
	enum types varType;
	int offsetAddress;
	int size; // Größe eines Arrays
} sym_variable;

typedef struct function_param {
    char *name;
    enum types varType;
    struct function_param *prev, *next;
} function_param;

typedef struct sym_function {
	enum types returnType;
	enum prototype protOrNot;
    function_param *callVar;
	struct sym_union* local_variables;
	char* interCode;
} sym_function;

union variable_or_function {
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
int printSymTable(char* filename);

#endif /* SYMTAB_H_ */
