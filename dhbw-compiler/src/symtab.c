/*
 * symtab.c
 *
 *  Created on: Mar 8, 2012
 *      Author: andres
 */

#include "symtab.h"

sym_union *sym_table = NULL;
function_param *param_list = NULL;

sym_union* searchGlobal(char* symName) { /* Kann Funktion und Variable zurückliefern */
	debug("SymTab: searchGlobal startet for %s.", symName);
	sym_union* found_entry = NULL;
	found_entry = malloc(sizeof(sym_union));
	HASH_FIND_STR(sym_table, symName, found_entry);
	return found_entry;
}

sym_union* searchLocal(char* symName, char* funcName) { /* Kann nur Variable zur�ckliefern */
	debug("SymTab: searchLocal %s in %s startet.", symName, funcName);

	sym_union* function = searchGlobal(funcName);
	if (function == NULL || function->symbolType != symFunction) {
		return NULL;
	}
	if (function->vof.symFunction.local_variables == NULL) {
		return NULL;
	}
	sym_union* found_variable;
	HASH_FIND_STR(function->vof.symFunction.local_variables, symName, found_variable);
	return found_variable;
}

sym_union* searchBoth(char* symName, char* funcName) { /* Kann nur Variable zurückliefern */
	debug("SymTab: searchBoth %s in %s startet.", symName, funcName);
	sym_union* found_entry = searchLocal(symName, funcName);
	if (found_entry == NULL) {
		found_entry = searchGlobal(symName);
	}
	return found_entry;
}

int insertFuncGlobal(char* symName, sym_function func) {
	debug("SymTab: New Function %s in global.", symName);
	if (searchGlobal(symName) == NULL) {
		sym_union* new_entry;
		new_entry = (sym_union *) malloc(sizeof(sym_union));
		if (new_entry == NULL)
			return 0;
		new_entry->symbolType = symFunction;
		new_entry->name = symName;

		new_entry->vof.symFunction = func;
		new_entry->vof.symFunction.local_variables = NULL;
		HASH_ADD_KEYPTR(hh, sym_table, new_entry->name, strlen(new_entry->name), new_entry);
		return 1;
	}
	return 0;
}

//int alterFuncGlobal(char* symName, sym_function func) {
//	sym_union* entry = searchGlobal(symName);
//	if (entry != NULL) {
//		entry->vof.symFunction = func;
//		return 1;
//	}
//	return 0;
//}

int insertVarGlobal(char* symName, sym_variable var) {
	debug("SymTab: New Variable %s in global.", symName); //Moritz: Habe 'or Function' rausgenommen
	if (searchGlobal(symName) == NULL) {
		sym_union* new_entry;
		new_entry = (sym_union *) malloc(sizeof(sym_union));
		if (new_entry == NULL)
			return 0;
		new_entry->symbolType = symVariable;
		new_entry->name = symName;
		new_entry->vof.symVariable = var; //TODO anpassen
		HASH_ADD_KEYPTR(hh, sym_table, new_entry->name, strlen(new_entry->name), new_entry);
		return 1;
	}
	return 0;
}

//int alterVarGlobal(char* symName, sym_variable var) {
//	sym_union* entry = searchGlobal(symName);
//	if (entry != NULL) {
//		entry->vof.symVariable = var;
//		return 1;
//	}
//	return 0;
//}

int insertVarLocal(char* symName, char* funcName, sym_variable var, int varCall) { //varCall: 0 => lokale Variable, 1=> call Variable
	debug("SymTab: New Variable %s local in %s.", symName, funcName);

	sym_union* function = searchGlobal(funcName);

	if (function != NULL && function->symbolType == symFunction && searchLocal(symName, funcName) == NULL) {
		sym_union* new_entry;
		new_entry = (sym_union *) malloc(sizeof(sym_union));
		if (new_entry == NULL)
			return 0;
		new_entry->symbolType = symVariable;
		new_entry->name = symName;
		debug("Local var inserted. Name: %s \n", new_entry->name);
		new_entry->vof.symVariable = var;
		HASH_ADD_KEYPTR(hh, function->vof.symFunction.local_variables, new_entry->name, strlen(new_entry->name), new_entry);
		return 1;
	}
	return 0;
}

/* Moritz:
 * Own method for inserting call parameter definitions. As this happens during declaration,
 * there is no need for checking the local sym-table for occurence.
 */
int insertCallVarLocal(char* funcName, function_param* parm) {
	debug("SymTab: New Call-Parameter-list in %s.", funcName);

	sym_union* function = searchGlobal(funcName);

	if (function != NULL && function->symbolType == symFunction) {
		function->vof.symFunction.callVar = parm;
		return 1;
	}
	return 0;
}

//int alterVarLocal(char* symName, char* funcName, sym_variable var) {
//	sym_union* function = searchGlobal(funcName);
//	sym_union* entry = searchLocal(symName, funcName);
//	if (function != NULL && entry != NULL) {
//		entry->vof.symVariable = var;
//		return 1;
//	}
//	return 0;
//}

int printSymTable(char* filename) {
	FILE *datei;
	datei = fopen(filename, "a");
	if (datei == NULL) {
		fprintf(stderr, "Fehler beim oeffnen der IR Datei.\n");
		return 0;
	}
	debug("SymTab: Datei %s geöffnet.\n", filename);

	fprintf(datei, "/* **************** */\n");
	fprintf(datei, "/* Symbol Table     */\n\n");
	fprintf(datei, "Hallo, Welt\n");

	struct sym_union *act;
	struct sym_union *subvar;

	for (act = sym_table; act != NULL; act = act->hh.next) {
		fprintf(datei, "-----------------------------------\n");

		if (act->symbolType == symVariable) {
			if (act->name == NULL) {
				fprintf(datei, "Variable Name = null");
			} else {
				fprintf(datei, "Variablen Name: %s \n", act->name);
			}
			switch (act->vof.symVariable.varType) {
				case voidType:
					break;
				case intType:
					fprintf(datei, "Typ: int \n");
					break;
				case intArrayType:
					fprintf(datei, "Typ: int-Array, Größe: %i \n", act->vof.symVariable.size);
					break;
				case ArrayType:
					fprintf(datei, "Typ: Array WTF MORITZ!!!, Größe: %i \n", act->vof.symVariable.size);
					break;
			}
			fprintf(datei, "Offset Adresse: %i \n", act->vof.symVariable.offsetAddress);
		}

		else if (act->symbolType == symFunction) {
			if (act->name == NULL) {
				fprintf(datei, "Function Name = null\n");
			} else {
				fprintf(datei, "Function name: %s \n", act->name);
			}
			switch (act->vof.symFunction.returnType) {
				case voidType:
					fprintf(datei, "Rückgabewert: void\n");
					break;
				case intType:
					fprintf(datei, "Rückgabewert: int\n");
					break;
				case intArrayType:
					fprintf(datei, "Rückgabewert: int-Array\n");
					break;
			}

			if (act->vof.symFunction.interCode != NULL) {
				fprintf(datei, "Intercode: \n %s", act->vof.symFunction.interCode);
			}
			if (act->vof.symFunction.local_variables != NULL) { // really need this one?
				for (subvar = act->vof.symFunction.local_variables; subvar != NULL; subvar = subvar->hh.next) {
					fprintf(datei, "\t-----------------------------------\n");

					//if (act->symbolType == symVariable) {
						if (act->name == NULL) {
							fprintf(datei, "\tVariable Name = null\n");
						} else {
							fprintf(datei, "\tVariablen Name: %s \n", act->name);
						}
						switch (act->vof.symVariable.varType) {
							case intType:
								fprintf(datei, "\tTyp: int \n");
								break;
							case intArrayType:
								fprintf(datei, "\tTyp: int-Array, Größe: %i \n", act->vof.symVariable.size);
								break;
							case ArrayType:
								fprintf(datei, "\tTyp: Array WTF MORITZ!!!, Größe: %i \n", act->vof.symVariable.size);
								break;
						}

						fprintf(datei, "\tOffset Adresse: %i \n", act->vof.symVariable.offsetAddress);
					//}
				}
			}
		}

	}

	fprintf(datei, "\n/* Symbol Table End */\n");
	fprintf(datei, "/* **************** */\n\n");
	fclose(datei);
	debug("Symbol Table printed.\n");

	return 1;
}
