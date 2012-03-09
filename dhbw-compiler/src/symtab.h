/*
 * symtab.h
 *
 *  Created on: Mar 8, 2012
 *      Author: andres
 */

#ifndef SYMTAB_H_
#define SYMTAB_H_

#include "uthash.h"
#include <stdio.h>

struct sym_entry {
	char *name;
	char *value;
	UT_hash_handle hh; /* makes this structure hashable */
};

typedef struct sym_entry sym_entry;

void *putsym();
sym_entry *getsym();

#endif /* SYMTAB_H_ */
