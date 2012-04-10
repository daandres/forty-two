/*
 * symtab.c
 *
 *  Created on: Mar 8, 2012
 *      Author: andres
 */

#include "symtab.h"

 sym_union *sym_table = NULL;

 /***MORITZ: F�r funktionsparameter func_param****/
 func_param *param_list = NULL;
 /*********************************************/


 sym_union* searchGlobal(char* symName) { /* Kann Funktion und Variable zurückliefern */
	 printf("MARCEL: searchGlobal startet for %s. \n", symName);
	 sym_union* found_entry = NULL;
	 found_entry = (sym_union *) malloc(sizeof(*found_entry));
	 HASH_FIND_STR(sym_table, symName, found_entry);
	 sym_union* whole_entry = found_entry;
	 free(found_entry);
	 return whole_entry;
 }

 sym_union* searchLocal(char* symName, char* funcName) { /* Kann nur Variable zur�ckliefern */
	 printf("MARCEL: searchLocal %s in %s startet. \n", symName, funcName);
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

 sym_union* searchBoth(char* symName, char* funcName) { /* Kann nur Variable zurückliefern */
	 printf("MARCEL: searchBoth %s in %s startet. \n", symName, funcName);
	 sym_union* found_entry = searchLocal(symName, funcName);
	 if(found_entry == NULL) {
		 found_entry = searchGlobal(symName);
	 }
	 return found_entry;
 }

 int insertFuncGlobal(char* symName, sym_function func) {
	 printf("MARCEL: New Function %s in global. \n", symName);
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
 
 int alterFuncGlobal(char* symName, sym_function func) {
	 sym_union* entry = searchGlobal(symName);
	 if(entry != NULL) {
		 entry->vof.symFunction = func;
		 return 1;
	 }
	 return 0;
 }

 int insertVarGlobal(char* symName, sym_variable var) {
	 printf("MARCEL: New Variable %s in global. \n", symName); //Moritz: Habe 'or Function' rausgenommen
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
 
 int alterVarGlobal(char* symName, sym_variable var) {
	 sym_union* entry = searchGlobal(symName);
	 if(entry != NULL) {
		 entry->vof.symVariable = var;
		 return 1;
	 }
	 return 0;
 }

 int insertVarLocal(char* symName, char* funcName, sym_variable var, int varCall) { //varCall: 0 => lokale Variable, 1=> call Variable
	 printf("MARCEL: New Variable %s local in %s. \n", symName, funcName);
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
 
 int alterVarLocal(char* symName, char* funcName, sym_variable var) {
	 sym_union* function = searchGlobal(funcName);
	 sym_union* entry = searchLocal(symName, funcName);
	 if(function != NULL && entry != NULL) {
		 entry->vof.symVariable = var;
		 return 1;
	 }
	 return 0;
 }

 int printSymTable(char* name) {
	FILE *datei;
	datei = fopen ("testdatei.txt", "w");
	if(datei == NULL) {
		printf("Fehler beim oeffnen der Datei.");
		return 0;
	}
	printf("MARCEL: Datei geöffnet.");
	fprintf (datei, "Hallo, Welt\n");
	 
	struct sym_union *act;
	struct sym_variable *subVar;

	for(act=sym_table; act != NULL; act=act->hh.next) {
		fprintf(datei, "-----------------------------------");
		
		if(act->symbolType == 1) {
			fprintf(datei, "Variablen Name: %s \n", act->name);
			if(act->vof.symVariable.varType == intType) {
				fprintf(datei, "Typ: int \n");
			}
			else if(act->vof.symVariable.varType == intArrayType) {
				fprintf(datei, "Typ: int-Array, Größe: %i \n", act->vof.symVariable.size);
			}
			fprintf(datei, "Offset Adresse: %i \n", act->vof.symVariable.offsetAddress);			
		}
		
		else {
			fprintf(datei, "Function name: %s \n", act->name);
			fprintf(datei, "Aufrufvariablen: \n");
			for(subVar=act->vof.symFunction.callVars; subVar != NULL; subVar=subVar->hh.next) {
				fprintf(datei, "- Name: %s, ", subVar->name);
				if(subVar->varType == intType) {
					fprintf(datei, "Typ: int, ");
				}
				else if(subVar->varType == intArrayType) {
					fprintf(datei, "Typ: int-Array, Größe: %i, ", subVar->size);
				}
				fprintf(datei, "Offset Adresse: %i \n", subVar->offsetAddress);
			}
			if(act->vof.symFunction.returnType == voidType) {
				fprintf(datei, "Rückgabetyp: void \n");
			}
			else if(act->vof.symFunction.returnType == intType) {
				fprintf(datei, "Rückgabetyp: int \n");
			}
			else if(act->vof.symFunction.returnType == intArrayType) {
				fprintf(datei, "Rückgabetyp: int-Array \n");
			}
			if(act->vof.symFunction.protOrNot == proto) {
				fprintf(datei, "Prototyp! \n");
			}
			for(subVar=act->vof.symFunction.lokalVars; subVar != NULL; subVar=subVar->hh.next) {
				fprintf(datei, "- Name: %s, ", subVar->name);
				if(subVar->varType == intType) {
					fprintf(datei, "Typ: int, ");
				}
				else if(subVar->varType == intArrayType) {
					fprintf(datei, "Typ: int-Array, Größe: %i, ", subVar->size);
				}
				fprintf(datei, "Offset Adresse: %i \n", subVar->offsetAddress);
			}			
			fprintf(datei, "Zwischencode: \n %s \n", act->vof.symFunction.interCode);
		}
	 
	}
	fclose (datei);	 
	return 1;
 }
