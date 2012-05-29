/*
 * symtab.c
 *
 *  Created on: Mar 8, 2012
 *      Author: andres
 */

#include "symtab.h"
#include <string.h>

sym_union_t *sym_table = NULL;
function_param_t *param_list = NULL; //Speichert die Parameterlisten f�r Funktionsdefinitionen und Funcktionsaufrufe
int guenter = 2;

/**
 * looks in the global symbol table for an entry with the given name
 * @param symName the name which shall be looked for
 * @return returns a pointer to the symbol table entry of the variable if there is one with the given name or else NULL if no match was found
 */
sym_union_t* searchGlobal(char* symName) { /* Kann Funktion und Variable zurückliefern */
	debug("SymTab: searchGlobal started for '%s'.", symName);
	sym_union_t* found_entry = NULL;
	//found_entry = (sym_union_t*) malloc(sizeof(sym_union_t));

	if (found_entry == NULL) {
		//warning("could not allocate memory");
		//return NULL;
	}

	HASH_FIND_STR(sym_table, symName, found_entry);


	return found_entry;
}

/**
 * Search inside the local-symbol table (for the function-name given) if the given variable
 * exists or not. The Procedure automatically checks whether the variable is inside the
 * parameter-list or inside the global table.
 * @param symName the name which shall be looked for
 * @param funcName the name of the function in which scope the search shall take place
 * @return returns a pointer to the entry of the variable if there is one with the given name or else NULL if no match was found
 */
sym_union_t* searchLocal(char* symName, char* funcName) { /* Kann nur Variable zur�ckliefern */
	debug("SymTab: searchLocal '%s' in '%s' started.", symName, funcName);

	sym_union_t* function = searchGlobal(funcName);


	if (function == NULL || function->symbolType != symFunction) {
		return NULL;
	}
	if (function->vof.symFunction.local_variables == NULL && function->vof.symFunction.callVar == NULL) {
		return NULL;
	}

	function_param_t *fparam;
	sym_union_t* found_variable = NULL;

	//Check wether the symbol is in the parameter-list. If the list is Null,
	//the block is skipped automatically
	DL_FOREACH(param_list,fparam){
		if(strcmp(fparam->name, symName)==0){
			found_variable = (sym_union_t *) malloc(sizeof(sym_union_t));

			found_variable->vof.symVariable.varType = fparam->varType;
			found_variable->name = fparam->name;
			found_variable->symbolType = symVariable;
		}
	}

	/*
	 * It is absolutely sufficient to check whether the found_variable is not equal to NULL
	 * because it is not possible that there is the same variable in both the parameter_list
	 * and the local variable-list. This is caused by the fact, that the procedure returns a
	 * value unequal to NULL when the variable was found.
	 */

	if(found_variable == NULL)
	{
		HASH_FIND_STR(function->vof.symFunction.local_variables, symName, found_variable);
	}

	return found_variable;
}

/**
 * first starts to look for a variable in local scope and if the search was not successful continues the search in the global symbol table
 * @param symName the name which shall be looked for
 * @param funcName the name of the function in which scope the search shall begin
 * @return returns a pointer to the entry of the variable if there is one with the given name or else NULL if no match was found 
 */
sym_union_t* searchBoth(char* symName, char* funcName) { /* Kann nur Variable zurückliefern */
	debug("SymTab: searchBoth '%s' in '%s' started.\n", symName, funcName);
	sym_union_t* found_entry = searchLocal(symName, funcName);
	if (found_entry == NULL) {
		found_entry = searchGlobal(symName);
	}
	return found_entry;
}

/**
 * inserts a function into the symbol table
 * @param symName name of the function and therefore name(index) of the entry in the symbol table
 * @param func the "object" with the needed information about the function
 * @return returns 0 if creating a new symbol was successful or 1 if there is already a symbol with the given name(index)
 */
int insertFuncGlobal(char* symName, sym_function_t func) {
	debug("SymTab: New Function '%s' in global.", symName);
	if (searchGlobal(symName) == NULL) {
		sym_union_t* new_entry;
		new_entry = (sym_union_t *) malloc(sizeof(sym_union_t));
		if (new_entry == NULL)
			return 1;
		new_entry->symbolType = symFunction;
		new_entry->name = symName;

		new_entry->vof.symFunction = func;
		new_entry->vof.symFunction.local_variables = NULL;
		HASH_ADD_KEYPTR(hh, sym_table, new_entry->name, strlen(new_entry->name), new_entry);
		return 0;
	}
	return 1;
}

//Noch nicht ausführlich getestet!!!
/**
 * alters the entered information of an already in the symbol table entered function
 * @param symName name of the function which shall be altered
 * @param func the "object" with the new/altered information about the function
 * @return returns 0 if change was successful or 1 if there is no entry in the symbol table with the given name
 */
int alterFuncGlobal(char* symName, sym_function_t func) {
	sym_union_t* entry = searchGlobal(symName);
	if (entry != NULL) {
		entry->vof.symFunction = func;
		return 0;
	}
	return 1;
}

/**
 * inserts a variable of global scope into the symbol table
 * @param symName name of the variable and therefore name(index) of the entry in the symbol table
 * @param var the "object" with the needed information about the variable
 * @return returns 0 if creating a new symbol was successful or 1 if there is already a symbol with the given name(index)
 */
int insertVarGlobal(char* symName, sym_variable_t var) {
	debug("SymTab: New Variable '%s' in global.", symName); //Moritz: Habe 'or Function' rausgenommen
	if (searchGlobal(symName) == NULL) {
		sym_union_t* new_entry;
		new_entry = (sym_union_t *) malloc(sizeof(sym_union_t));
		if (new_entry == NULL)
			return 1;
		new_entry->symbolType = symVariable;
		new_entry->name = symName;
		new_entry->vof.symVariable = var; //TODO anpassen
		HASH_ADD_KEYPTR(hh, sym_table, new_entry->name, strlen(new_entry->name), new_entry);
		return 0;
	}
	return 1;
}

//Noch nicht ausführlich getestet!!!
/**
 * alters the entered information of an already in the global symbol table entered variable
 * @param symName name of the variable which shall be altered
 * @param var the "object" with the new/altered information about the variable
 * @return returns 0 if change was successful or 1 if there is no entry in the symbol table with the given name
 */
int alterVarGlobal(char* symName, sym_variable_t var) {
	sym_union_t* entry = searchGlobal(symName);
	if (entry != NULL) {
		entry->vof.symVariable = var;
		return 0;
	}
	return 1;
}

/**
 * inserts a variable of local scope into the symbol table or the table of call variables of the given function
 * @param symName name of the variable and therefore name(index) of the entry in the symbol table or call variable table
 * @param the name of the function in which scope the entry of the variable shall take place
 * @param var the "object" with the needed information about the variable
 * @param varCall defines whether the variabal shall be defined in the local symbol table (0) or in the call variables (1) of the function
 * @return returns 0 if creating a new symbol was successful or 1 if there is already a symbol either in the local symbol table or the call variable table with the given name(index)
 */
int insertVarLocal(char* symName, char* funcName, sym_variable_t var, int varCall) { //varCall: 0 => lokale Variable, 1=> call Variable
	debug("SymTab: New Variable '%s' local in '%s'.", symName, funcName);

	sym_union_t* function = searchGlobal(funcName);

	if (function != NULL && function->symbolType == symFunction && searchLocal(symName, funcName) == NULL) {
		sym_union_t* new_entry;
		new_entry = (sym_union_t *) malloc(sizeof(sym_union_t));
		if (new_entry == NULL)
			return 1;
		new_entry->symbolType = symVariable;
		new_entry->name = symName;
		debug("Local var inserted. Name: '%s' \n", new_entry->name);
		new_entry->vof.symVariable = var;
		HASH_ADD_KEYPTR(hh, function->vof.symFunction.local_variables, new_entry->name, strlen(new_entry->name),
		      new_entry);
		return 0;
	}
	return 1;
}
/**
 * Purges(removes) all elements of the given doubly-linked list using uthash.
 *
 * @parm params list of parameters
 */
void PurgeParameters(function_param_t* params){

	function_param_t* element; //Pointer to the current element in the iteration
	function_param_t* temp; //tmp pointer to the iteration algorithm

	DL_FOREACH_SAFE(params, element, temp){
		DL_DELETE(params, element);
	}

}

/**
 * Own method for inserting call parameter definitions. As this happens during declaration,
 * there is no need for checking the local sym-table for occurence.
 * @parm funcName name of the function to insert the parameters in
 * @parm parm actual list of the parameters
 */
int insertCallVarLocal(char* funcName, function_param_t* parm) {
	debug("SymTab: New Call-Parameter-list in '%s'.\n", funcName);

	sym_union_t* function = searchGlobal(funcName);
	//No need to check for local variables, as the local part does not exist yet.

	if (function != NULL && function->symbolType == symFunction) {
		//Purge the old list, if any
		PurgeParameters(function->vof.symFunction.callVar);

		function->vof.symFunction.callVar = parm;
		return 0;
	}else
	{
		debug("No param inserted in %s. But Why???\n", funcName);
	}
	return 1;
}







//Noch nicht ausführlich getestet!!!
/**
 * alters the entered information of an already in either a local symbol table or call variable table entered variable
 * @param symName name of the variable which shall be altered
 * @param funcName name of the function in which scope the variable is defined
 * @param var the "object" with the new/altered information about the variable
 * @return returns 0 if change was successful or 1 if there is no entry in the symbol table with the given name
 */
int alterVarLocal(char* symName, char* funcName, sym_variable_t var) {
	sym_union_t* entry = searchLocal(symName, funcName);
	if (entry != NULL) {
		entry->vof.symVariable = var;
		return 0;
	}else
	{
		debug("No param inserted in %s. But Why???\n", funcName);
	}
	return 1;
}

/**
 * prints out the finite symbol table
 * @param filename the name of the file in which the symbol table shall be printed out
 */
int printSymTable(char* filename) {
	FILE *datei;
	datei = fopen(filename, "a");
	if (datei == NULL) {
		fprintf(stderr, "Fehler beim oeffnen der IR Datei.\n");
		return 1;
	}
	debug("SymTab: Datei '%s' geöffnet.\n", filename);

	fprintf(datei, "/* **************** */\n");
	fprintf(datei, "/* Symbol Table     */\n\n");

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
					fprintf(datei, "Typ: Array, Größe: %i \n", act->vof.symVariable.size);
					break;
				case None:
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
				case ArrayType:
					fprintf(datei, "Rückgabewert: Array\n");
					break;
				case None:
				break;
			}

			fprintf(datei, "Parameter: ");
			function_param_t* element;

			DL_FOREACH(act->vof.symFunction.callVar, element){
				switch (element->varType) {
					case voidType:
						fprintf(datei, "void->");
						break;
					case intType:
						fprintf(datei, "int->");
						break;
					case intArrayType:
						fprintf(datei, "int-Array->");
						break;
					case ArrayType:
						fprintf(datei, "Array->");
						break;
					case None:
					break;
				}
				fprintf(datei,"%s, ",element->name);
			}

			fprintf(datei,"\n");

//			if (act->vof.symFunction.interCode != NULL) {
//				fprintf(datei, "Intercode: \n %s", act->vof.symFunction.interCode);
//			}

			if (act->vof.symFunction.local_variables != NULL) {
				for (subvar = act->vof.symFunction.local_variables; subvar != NULL; subvar = subvar->hh.next) {
					fprintf(datei, "\t-----------------------------------\n");

					if (subvar->symbolType == symVariable) {
						if (subvar->name == NULL) {
							fprintf(datei, "\tVariable Name = null\n");
						} else {
							fprintf(datei, "\tVariablen Name: %s \n", subvar->name);
						}
						switch (subvar->vof.symVariable.varType) {
							case voidType:
								break;
							case intType:
								fprintf(datei, "\tTyp: int \n");
								break;
							case intArrayType:
								fprintf(datei, "\tTyp: int-Array, Größe: %i \n", subvar->vof.symVariable.size);
								break;
							case ArrayType:
								fprintf(datei, "\tTyp: Array WTF MORITZ!!!, Größe: %i \n", subvar->vof.symVariable.size);
								break;
							case None:
							break;
						}

						fprintf(datei, "\tOffset Adresse: %i \n", subvar->vof.symVariable.offsetAddress);
					}
				}
			}
		}

	}

	fprintf(datei, "\n/* Symbol Table End */\n");
	fprintf(datei, "/* **************** */\n\n");
	fclose(datei);
	debug("Symbol Table printed.\n");

	return 0;
}

//*************!!!Wird sp�ter in typecheck.h ausgelagert!!!*****************//
