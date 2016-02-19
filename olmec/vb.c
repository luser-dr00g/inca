#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

#include "ar.h"
#include "en.h"
#include "st.h"

#include "vb.h"

void common(int *ap, int *wp){
    //promote smaller number to matching type
}


int vid (int w, verb v){ return w; }
int vplus (int a, int w, verb v){
    common(&a,&w);
    switch(gettag(w)){
        case LITERAL: return newdata(LITERAL, getval(a)+getval(w));
    }
    return null;
}

int vneg(int w, verb v){
    switch(gettag(w)){
        case LITERAL: return newdata(LITERAL, -getval(w));
    }
    return null;
}
int vminus(int a, int w, verb v){
    common(&a,&w);
    switch(gettag(w)){
        case LITERAL: return newdata(LITERAL, getval(a)-getval(w));
    }
    return null;
}

int vshapeof (int w, verb v){
    switch(gettag(w)){
        case ARRAY: {
            array a = getptr(w);
            return cache(ARRAY, copy(cast(a->dims,a->rank)));
        }
    }
    return null;
}
int vreshape (int a, int w, verb v){
}

int vtally (int w, verb v){
    switch(gettag(w)){
        case ARRAY: {
            array a = getptr(w);
            return newdata(LITERAL, a->rank?a->dims[0]:1);
        }
    }
    return newdata(LITERAL, 1);
}


int viota (int w, verb v){
    switch(gettag(w)){
        case LITERAL: return cache(ARRAY, iota(w));
    }
    return null;
}

#define VERBTAB_DEF(id,...) \
    v=malloc(sizeof*v); \
    *v=(struct verb){id,__VA_ARGS__}; \
    def(st, newdata(CHAR, id), cache(VERB, v));

void init_vb(symtab st){
    verb v;
    VERBTAB(VERBTAB_DEF)
}

