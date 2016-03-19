#ifndef SYMBOL_H_
#define SYMBOL_H_
#include "common.h"
/* symbol table */

struct symtab {
    int key;
    int value;
    int n;
    symtab *tab /*[n]*/ ;
};

symtab makesymtab(int n);
/*  mode=0: prefix match
    mode=1: defining search */
symtab findsym(symtab st, int **spp, int *n, int mode);
void def(symtab st, int name, int v);
#define define_symbol(st, ...) (define_symbol_n)(st, PP_NARG(__VA_ARGS__), __VA_ARGS__)
void (define_symbol_n)(symtab st, int n, ...);
#define symbol_value(st, ...) (symbol_value_n)(st, PP_NARG(__VA_ARGS__), __VA_ARGS__)
int (symbol_value_n)(symtab st, int n, ...);

#endif
