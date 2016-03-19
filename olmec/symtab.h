#ifndef SYMBOL_H_
#define SYMBOL_H_
#include "common.h"
/* symbol table */

struct symtab {
    int key;
    int val;
    int n;
    symtab *tab /*[n]*/ ;
};

symtab makesymtab(int n);
/*  mode=0: prefix match
    mode=1: defining search */
symtab findsym(symtab st, int **spp, int *n, int mode);
void def(symtab st, int name, int v);

#endif
