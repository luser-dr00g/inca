#ifndef SYMBOL_H_
#define SYMBOL_H_
#include "common.h"
/* symbol table */

struct symtab {
    object key;
    object value;
    int n;
    symtab *tab /*[n]*/ ;
    symtab prev; //==NULL in root and all leafs. used to chain (stack) new roots.
};

symtab makesymtab(int n);
/*  mode=0: prefix match
    mode=1: defining search */
symtab findsym(symtab st, object **spp, int *n, int mode);
void def(symtab st, object name, object v);
#define define_symbol(st, ...) \
    (define_symbol_n)(st, PP_NARG(__VA_ARGS__), __VA_ARGS__)
void (define_symbol_n)(symtab st, int n, ... /* ..., v */);
#define symbol_value(st, ...) \
    (symbol_value_n)(st, PP_NARG(__VA_ARGS__), __VA_ARGS__)
object (symbol_value_n)(symtab st, int n, ...);

#endif
