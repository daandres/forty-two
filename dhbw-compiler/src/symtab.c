/*
 * symtab.c
 *
 *  Created on: Mar 8, 2012
 *      Author: andres
 */

#include "symtab.h"

 sym_union *sym_table = NULL;

 sym_union* searchGlobal(char symName[]) { /* Kann Funktion und Variable zurückliefern */
	 printf("search Global startet in %s.", symName);
	 sym_union* found_entry;
	 found_entry = (sym_union *) malloc(sizeof(*found_entry));
	 HASH_FIND_STR(sym_table, symName, found_entry);
	 sym_union* whole_entry = found_entry;
	 free(found_entry);
	 return whole_entry;
 }

 sym_union* searchLocal(char symName[], char funcName[]) { /* Kann nur Variable zur�ckliefern */
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
	 whole_entry->name = found_variable->name;
	 free(found_variable);
	 return whole_entry;
 }

 sym_union* searchBoth(char symName[], char funcName[]) { /* Kann nur Variable zurückliefern */
	 printf("search %s Local in %s startet.", symName, funcName);
	 sym_union* found_entry = searchLocal(symName, funcName);
	 if(found_entry == NULL) {
		 found_entry = searchGlobal(symName);
	 }
	 return found_entry;
 }

 int insertFuncGlobal(char* symName, sym_function func) {
	 printf("New Function %s in Local.", symName); //Moritz: Habe 'Variable or' rausgenommen
	 if(searchGlobal(symName) == NULL) {
		 sym_union* new_entry;
		 new_entry = (sym_union *) malloc(sizeof(*new_entry));
		 if(new_entry == NULL) return 0;
		 new_entry->symbolType = symFunction;
		 new_entry->vof.symFunction = func;
		 new_entry->name = symName;
		 HASH_ADD_KEYPTR(hh, sym_table, new_entry->name, strlen(new_entry->name), new_entry);
		 //free(new_entry);
		 return 1;
	 }
	 return 0;
 }

 int insertVarGlobal(char* symName, sym_variable var) {
	 printf("New Variable %s in Local.", symName); //Moritz: Habe 'or Function' rausgenommen
	 if(searchGlobal(symName) == NULL) {
		 sym_union* new_entry;
		 new_entry = (sym_union *) malloc(sizeof(*new_entry));
		 if(new_entry == NULL) return 0;
		 new_entry->symbolType = symFunction;
		 new_entry->vof.symVariable = var;
		 new_entry->name = symName;
		 HASH_ADD_KEYPTR(hh, sym_table, new_entry->name, strlen(new_entry->name), new_entry);
		 //free(new_entry);
		 return 1;
	 }
	 return 0;
 }

 int insertVarLocal(char* symName, char funcName[], sym_variable var, int varCall) { //varCall: 0 => lokale Variable, 1=> call Variable
	 printf("New Variable %s in %s.", symName, funcName);
	 sym_union* function = searchGlobal(funcName);
	 if(searchLocal(symName, funcName) == NULL && function != NULL) {
		 sym_variable* new_entry;
		 new_entry = (sym_variable *) malloc(sizeof(*new_entry));
		 if(new_entry == NULL) return 0;
		 new_entry = &var;
		 if(varCall == 0) {
			 HASH_ADD_KEYPTR(hh, function->vof.symFunction.lokalVars, new_entry->name, strlen(new_entry->name), new_entry);
		 }
		 else {
			 HASH_ADD_KEYPTR(hh, function->vof.symFunction.callVars, new_entry->name, strlen(new_entry->name), new_entry);
		 }
		 //free(new_entry);
		 return 1;
	 }   
	 return 0;	 
 }

 int printSymTable(char* name) {
	 FILE *datei;
	 datei = fopen ("testdatei.txt", "w");
	 if (datei == NULL) {
		 printf("Fehler beim oeffnen der Datei.");
		 return 1;
	 }
	 fprintf (datei, "Hallo, Welt\n");
	 fclose (datei);
	 
	 struct sym_union *act;
	 struct sym_variable *subVar;

	for(act=sym_table; act != NULL; act=act->hh.next) {
		printf("-----------------------------------");
		
		if(act->symbolType == 1) {
			printf("Variablen Name: %s \n", act->name);
			if(act->vof.symVariable.varType == intType) {
				printf("Typ: int \n");
			}
			else if(act->vof.symVariable.varType == intArrayType) {
				printf("Typ: int-Array, Größe: %i \n", act->vof.symVariable.size);
			}
			printf("Offset Adresse: %i \n", act->vof.symVariable.offsetAddress);			
		}
		
		else {
			printf("Function name: %s \n", act->name);
			printf("Aufrufvariablen: \n");
			for(subVar=act->vof.symFunction.callVars; subVar != NULL; subVar=subVar->hh.next) {
				printf("- Name: %s, ", subVar->name);
				if(subVar->varType == intType) {
					printf("Typ: int, ");
				}
				else if(subVar->varType == intArrayType) {
					printf("Typ: int-Array, Größe: %i, ", subVar->size);
				}
				printf("Offset Adresse: %i \n", subVar->offsetAddress);
			}
			if(act->vof.symFunction.returnType == voidType) {
				printf("Rückgabetyp: void \n");
			}
			else if(act->vof.symFunction.returnType == intType) {
				printf("Rückgabetyp: int \n");
			}
			else if(act->vof.symFunction.returnType == intArrayType) {
				printf("Rückgabetyp: int-Array \n");
			}
			if(act->vof.symFunction.protOrNot == proto) {
				printf("Prototyp! \n");
			}
			for(subVar=act->vof.symFunction.lokalVars; subVar != NULL; subVar=subVar->hh.next) {
				printf("- Name: %s, ", subVar->name);
				if(subVar->varType == intType) {
					printf("Typ: int, ");
				}
				else if(subVar->varType == intArrayType) {
					printf("Typ: int-Array, Größe: %i, ", subVar->size);
				}
				printf("Offset Adresse: %i \n", subVar->offsetAddress);
			}			
			printf("Zwischencode: \n %s \n", act->vof.symFunction.interCode);
		}
	 
	}
	 
	 return 0;
 }
