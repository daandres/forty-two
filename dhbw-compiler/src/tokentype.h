//Um nur einen Typ fŸr die Tokens verwenden zu mŸssen, werden alle parameter in einem TOKENTYPE zusammengefasst
//Eine ausname bildet hier types_t, welches ein enum ist und deshalb nicht einbezogen wird.

#ifndef TOKENTYPE_H_
#define TOKENTYPE_H_

#include "symtab.h"
#include "uthash.h"

typedef struct TOKENTYPE {

	//IR-Code tyypen
	int* true; 		// true exit
	int* false;		// false exit
	int* next; 		// next exit
	int quad; 		// number of statement
	char* idName; 	// variable or temporary variable name

	//symbolltabelle-funktionen typen
	types_t returnType;
	prototype_t protOrNot;
	function_param_t *callVar;
	struct sym_union* local_variables;
	char* interCode;

	//symboltabelle-union typen
	symbol_t symbolType; /* 1 -> variable; 0 -> function */
	variable_or_function_t vof;
	UT_hash_handle hh; /* makes this structure hashable */
	char* name;

} TOKENTYPE_t;
