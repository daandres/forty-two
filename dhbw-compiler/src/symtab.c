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

 sym_union* searchGlobal(char symName[]) { /* Kann Funktion und Variable zurückliefern */
	 printf("search %s Global startet.", symName);
	 sym_union found_entry;
	 //HASH_FIND_STR(sym_table, symName, found_entry);
	 return NULL;
 }

 sym_union* searchLocal(char symName[], char funcName[]) { /* Kann nur Variable zurückliefern */
	 printf("search %s Local in %s startet.", symName, funcName);
	 sym_union found_entry;

	 return NULL;
 }

 sym_union* searchBoth(char symName[], char funcName[]) { /* Kann nur Variable zurückliefern */
	 printf("search %s Local in %s startet.", symName, funcName);
	 //searchLocal
	 sym_union found_entry;
	 found_entry = searchLocal(symName, funcName);
	 if(found_entry == NULL) {
		 found_entry = searchGlobal(symName);
	 }
	 return found_entry;
 }

 int insertFuncGlobal(char symName[255], sym_function func) {
	 printf("New Variable or Function %s in Local.", symName);
	 if(searchGlobal(symName) == NULL) {
		 sym_union new_entry;
		 new_entry.symbolType = symFunction;
		 new_entry.vof.symFunction = func;
		 //HASH_ADD_STR(sym_table, symName, new_entry);
		 return 1;
	 }
	 return 0;
 }

 int insertVarGlobal(char symName[255], sym_variable var) {
	 printf("New Variable or Function %s in Local.", symName);
	 if(searchGlobal(symName) == NULL) {
		 sym_union new_entry;
		 new_entry.symbolType = symVariable;
		 new_entry.vof.symVariable = var;
		 //HASH_ADD_STR(sym_table, symName, new_entry);
		 return 1;
	 }
	 return 0;
 }

 int insertVarLocal(char symName[255], char funcName[], sym_variable var) {
	 printf("New Variable %s in %s.", symName, funcName);
	 if(searchLocal(symName, funcName) == NULL) {
		 sym_union new_entry;
		 new_entry.symbolType = symVariable;
		 new_entry.vof.symVariable = var;
		 //HASH_ADD_STR(sym_table, symName, new_entry);
		 return 1;
	 }   
	 return 0;	 
 }
