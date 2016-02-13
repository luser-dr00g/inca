#include <stdint.h>
#include <stdlib.h>

#include "en.h"
#include "st.h"

#include "vb.h"

void common(int *ap, int *wp){
    //promote smaller object to matching type
}

int id (int w){ return w; }
int plus (int a, int w){
    common(&a,&w);
    return newdata(LITERAL, getval(a)+getval(w));
}

#define VERBTAB_DEF(id,...) \
    v=malloc(sizeof*v); \
    *v=(struct verb){id,__VA_ARGS__}; \
    def(st, newdata(CHAR, id), cache(VERB, v));

void init_vb(symtab st){
    verb v;
    VERBTAB(VERBTAB_DEF)
}

