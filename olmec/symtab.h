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

struct magic {
    object (*get)(symtab node);
    void (*put)(symtab node, object val);
};

symtab makesymtab(int n);
symtab makesymtabchain(symtab root, int n);

/*  mode=0: prefix match
    mode=1: defining search */
symtab findsym(symtab st, object **spp, int *n, int mode, int bias);

/* get/set node value */
object getsym(symtab node);
void putsym(symtab node, object val);

void def(symtab st, object name, object v, int bias);

object find(symtab st, object name);

#define define_symbol(st, ...) \
    (define_symbol_n)(st, PP_NARG(__VA_ARGS__), __VA_ARGS__)
void (define_symbol_n)(symtab st, int n, ... /* ..., v */);

#define symbol_value(st, ...) \
    (symbol_value_n)(st, PP_NARG(__VA_ARGS__), __VA_ARGS__)
object (symbol_value_n)(symtab st, int n, ...);

#endif
