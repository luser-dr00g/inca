/* Adverbs and Conjunctions
 *
 * These re-use the same structure as verbs, and generate
 * new verb structures dynamically for a concrete representation
 * of a "derived verb" which is an adverb or conjunction's result.
 *
 */
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

#include "av.h"
#include "av_private.h"

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

