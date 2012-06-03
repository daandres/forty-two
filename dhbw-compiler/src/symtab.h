/*
 * symtab.h
 *
 *  Created on: Mar 8, 2012
 *      Author: andres
 */

#ifndef SYMTAB_H_
#define SYMTAB_H_

//#include "main.h"
#include "uthash.h"
#include "utlist.h"
#include <stdio.h>

//The typ None is used during parsing to mark unresolved returntypes (like during the function-definition)
typedef enum types {
	voidType = 0, intType = 1, intArrayType = 2, ArrayType = 3, None = 4
} types_t;

typedef enum symbol {
	symFunction, symVariable
} symbol_t;

typedef enum prototype {
	no = 0, proto = 1
} prototype_t;

typedef struct sym_variable {
	types_t varType;
	int offsetAddress;
	int size; // Größe eines Arrays
} sym_variable_t;

typedef struct function_param {
	char *name;
	types_t varType;
	struct function_param *prev, *next;
} function_param_t;

typedef struct sym_function {
	types_t returnType;
	prototype_t protOrNot;
	function_param_t *callVar /* = NULL */;
	struct sym_union* local_variables;
	char* interCode;
} sym_function_t;

typedef union variable_or_function {
	sym_variable_t symVariable;
	sym_function_t symFunction;
} variable_or_function_t;

typedef struct sym_union {
	symbol_t symbolType; /* 1 -> variable; 0 -> function */
	variable_or_function_t vof;
	UT_hash_handle hh; /* makes this structure hashable */
	char* name;
} sym_union_t;

sym_union_t* searchGlobal(char* symName); /* Kann Funktion und Variable zur�ckliefern */
sym_union_t* searchLocal(char* symName, char* funcName); /* Kann nur Variable zur�ckliefern */
sym_union_t* searchBoth(char* symName, char* funcName);
int insertFuncGlobal(char* symName, sym_function_t func);
int alterFuncGlobal(char* symName, sym_function_t func);
int insertVarGlobal(char* symName, sym_variable_t var);
int alterVarGlobal(char* symName, sym_variable_t var);
int insertVarLocal(char* symName, char* funcName, sym_variable_t var, int varCall);
int insertCallVarLocal(char* funcName, function_param_t* parm);
int alterVarLocal(char* symName, char* funcName, sym_variable_t var);
int printSymTable(char* filename);

#endif /* SYMTAB_H_ */
