/*
 * symtab.c
 *
 *  Created on: Mar 8, 2012
 *      Author: andres
 */

#include "symtab.h"

// sym_entry *sym_table = NULL;

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

 sym_union* searchGlobal(char varName[]) { /* Kann Funktion und Variable zurückliefern */
	 printf("search %s Global startet.", varName);
 }

 sym_variable* searchLocal(char varName[], char funcName[]) { /* Kann nur Variable zurückliefern */
	 printf("search %s Local in %s startet.", varName, funcName);
 }

 int insertSymGlobal(char varName[]) {
	 printf("New Variable or Function %s in Local.", varName);
 }

 int insertSymLocal(char varName[], char funcName[]) {
	 printf("New Variable %s in %s.", varName, funcName);
 }
