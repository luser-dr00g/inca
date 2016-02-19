#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ar.h"
#include "en.h"
#include "st.h"
#include "vb.h"
#include "av.h"
#include "ex.h"

#define ADVERBTAB(_) \
/*base, monad, dyad, f, g, h, mr,lr,rr*/ \
_('&',  0,     amp,  0, 0, 0, 0, 0, 0 ) \
_('@',  0,     atop, 0, 0, 0, 0, 0, 0 ) \
/**/

#define ADVERBTAB_DEF(id,...) \
    v=malloc(sizeof*v); \
    *v=(struct verb){id,__VA_ARGS__}; \
    def(st, newdata(CHAR, id), cache(ADVERB, v));

#define DERIV(...) \
    v=malloc(sizeof*v), \
    *v=(struct verb){__VA_ARGS__}, \
    cache(VERB, v)

enum { NN, NV, VN, VV };
#define CONJCASE(a,w) \
    (qv(a)*2+qv(w))

#define DECLFG \
    verb fv = getptr(v->f); \
    int (*f1)(int,verb) = fv?fv->monad:0; \
    int (*f2)(int,int,verb) = fv?fv->dyad:0; \
    verb gv = getptr(v->g); \
    int (*g1)(int,verb) = gv?gv->monad:0; \
    int (*g2)(int,int,verb) = gv?gv->dyad:0; 


int domerr(int w, verb v){
    return null;
}

int withl(int w, verb v){ DECLFG; return g2(v->f, w, gv); }
int withr(int w, verb v){ DECLFG; return f2(w, v->g, fv); }
int on1(int w, verb v){ DECLFG; return f1(g1(w,gv),fv); }
int on2(int a, int w, verb v){ DECLFG; return f2(g1(a,gv),g1(w,gv),fv); }

int amp(int a, int w, verb v){
    switch(CONJCASE(a,w)){
        case NN: return domerr(0,v);
        case NV: return DERIV('&', withl, NULL, a, w, 0, 0, 0, 0);
        case VN: return DERIV('&', withr, NULL, a, w, 0, 0, 0, 0);
        case VV: return DERIV('&', on1,   on2,  a, w, 0, 0, 0, 0);
    }
}


int atop2(int a, int w, verb v){ DECLFG; return f1(g2(a,w,gv),fv); }

int atop(int a, int w, verb v){
    switch(CONJCASE(a,w)){
        case NN: return domerr(0,v);
        case NV: return domerr(0,v);
        case VN: return domerr(0,v);
        case VV: {
            v = getptr(w);
            return DERIV('@', on1, atop2, a, w, 0, v->mr, v->lr, v->rr);
        } 
    }
}

void init_av(symtab st){
    verb v;
    ADVERBTAB(ADVERBTAB_DEF)
}

