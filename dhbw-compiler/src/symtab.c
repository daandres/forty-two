/*
 * symtab.c
 *
 *  Created on: Mar 8, 2012
 *      Author: andres
 */

#include "symtab.h"

 sym_union *sym_table = NULL;

//void * putsym(sym_entry *new_entry) {
//	printf("NEW Symbol in Symbol Table: %s", new_entry->name);
//	HASH_ADD_STR(sym_table, name, new_entry);
//	return 0;
//}
//
//sym_entry * getsym(char *search) {
//	sym_entry *found_entry;
//	HASH_FIND_STR(sym_table, search, found_entry);
//	return found_entry;
//}

 sym_union* searchGlobal(char symName[]) { /* Kann Funktion und Variable zurŸckliefern */
	 printf("search Global startet in %s.", symName);
	 sym_union* found_entry;
	 found_entry = (sym_union *) malloc(sizeof(*found_entry));
	 HASH_FIND_STR(sym_table, symName, found_entry);
	 sym_union* whole_entry = found_entry;
	 free(found_entry);
	 return whole_entry;
 }

 sym_union* searchLocal(char symName[], char funcName[]) { /* Kann nur Variable zurŸckliefern */
	 printf("search %s Local in %s startet.", symName, funcName);
	 sym_union* function = searchGlobal(funcName);
	 if(function == NULL || function->symbolType != 0) {
		 if(function->vof.symFunction.lokalVars == NULL && function->vof.symFunction.callVars == NULL) {
			 return NULL;
		 }
	 }
	 sym_variable* found_variable;
	 found_variable = (sym_variable *) malloc(sizeof(*found_variable));
	 HASH_FIND_STR(function->vof.symFunction.lokalVars, symName, found_variable);
	 if(found_variable == NULL) {
		 HASH_FIND_STR(function->vof.symFunction.callVars, symName, found_variable);
	 }

	 sym_union* whole_entry;
	 whole_entry = (sym_union *) malloc(sizeof(*whole_entry));
	 whole_entry->symbolType = symVariable;
	 whole_entry->vof.symVariable = *found_variable;
	 free(found_variable);
	 return whole_entry;
 }

 sym_union* searchBoth(char symName[], char funcName[]) { /* Kann nur Variable zurŸckliefern */
	 printf("search %s Local in %s startet.", symName, funcName);
	 //searchLocal
	 sym_union* found_entry = searchLocal(symName, funcName);
	 if(found_entry == NULL) {
		 found_entry = searchGlobal(symName);
	 }
	 return found_entry;
 }

 int insertFuncGlobal(char* symName, sym_function func) {
	 printf("New Variable or Function %s in Local.", symName);
	 if(searchGlobal(symName) == NULL) {
		 sym_union* new_entry;
		 new_entry = (sym_union *) malloc(sizeof(*new_entry));
		 new_entry->symbolType = symFunction;
		 new_entry->vof.symFunction = func;
		 new_entry->name = symName;
		 HASH_ADD_KEYPTR(hh, sym_table, new_entry->name, strlen(new_entry->name), new_entry);
		 return 1;
	 }
	 return 0;
 }

 int insertVarGlobal(char* symName, sym_variable var) {
	 printf("New Variable or Function %s in Local.", symName);
	 if(searchGlobal(symName) == NULL) {
		 sym_union* new_entry;
		 new_entry = (sym_union *) malloc(sizeof(*new_entry));
		 new_entry->symbolType = symFunction;
		 new_entry->vof.symVariable = var;
		 new_entry->name = symName;
		 HASH_ADD_KEYPTR(hh, sym_table, new_entry->name, strlen(new_entry->name), new_entry);
		 return 1;
	 }
	 return 0;
 }

 int insertVarLocal(char symName[255], char funcName[], sym_variable var) {
	 printf("New Variable %s in %s.", symName, funcName);
	 if(searchLocal(symName, funcName) == NULL) {
		 sym_union* new_entry;
		 new_entry = (sym_union *) malloc(sizeof(*new_entry));
		 new_entry->symbolType = symVariable;
		 new_entry->vof.symVariable = var;
		 new_entry->name = symName;
		 HASH_ADD_KEYPTR(hh, sym_table, new_entry->name, strlen(new_entry->name), new_entry);
		 return 1;
	 }   
	 return 0;	 
 }
