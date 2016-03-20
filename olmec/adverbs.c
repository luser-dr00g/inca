/* Adverbs and Conjunctions
 *
 * These re-use the same structure as verbs, and generate
 * new verb structures dynamically for a concrete representation
 * of a "derived verb" which is the result of an adverb or conjunction.
 *
 * That is, these functions take other functions (verbs) as arguments.
 * 
 * & amp implements currying a left or right argument to a function,
 * yielding a monadic derived verb. For 2 verbs it creates a 
 * derived verb which is the composition f o g.
 * 
 * @ atop performs a similar but simpler f o g composition.
 *
 * / areduce yields a derived verb which is a reduction wrt the argument verb.
 *
 * \ ascan yields a derived verb which is yields a series of partial reductions
 * of increasing prefixes of the argument.
 *
 * abackscan behaves like ascan but yields the partial reductions of decreasing
 * suffixes of the argument (`back`wards `scan`)
 */
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "encoding.h"
#include "symtab.h"
#include "verbs.h"
#include "exec.h"

#include "adverbs.h"
#include "adverb_private.h"

object domerr(object w, verb v){
    return null;
}

object withl(object w, verb v){ DECLFG; return g2(v->f, w, gv); }
object withr(object w, verb v){ DECLFG; return f2(w, v->g, fv); }
object on1(object w, verb v){ DECLFG; return f1(g1(w,gv),fv); }
object on2(object a, object w, verb v){ DECLFG; return f2(g1(a,gv),g1(w,gv),fv); }

object amp(object a, object w, verb v){
    switch(CONJCASE(a,w)){
        case NN: return domerr(0,v);
        case NV: return DERIV('&', NULL, withl, NULL, a, w, 0, 0, 0, 0);
        case VN: return DERIV('&', NULL, withr, NULL, a, w, 0, 0, 0, 0);
        case VV: return DERIV('&', NULL, on1,   on2,  a, w, 0, 0, 0, 0);
    }
}


object atop2(object a, object w, verb v){ DECLFG; return f1(g2(a,w,gv),fv); }

object atop(object a, object w, verb v){
    switch(CONJCASE(a,w)){
        case NN: return domerr(0,v);
        case NV: return domerr(0,v);
        case VN: return domerr(0,v);
        case VV: {
            v = getptr(w);
            return DERIV('@', NULL, on1, atop2, a, w, 0, v->mr, v->lr, v->rr);
        } 
    }
}


object reduce(object w, verb v){
    DECLFG;
    switch(gettag(w)){
    case LITERAL: return w;
    case ARRAY: {
        array W = getptr(w);
        switch(W->rank){
        case 1: switch(W->dims[0]){
                case 0: return getfill(fv->id);
                case 1: return *elem(W,0);
                default: {
#if 0
                    int z=*elem(W,W->dims[0]-1);
                    for (int i=W->dims[0]-2; i>=0; i--)
                        z=f2(*elem(W,i),z,v);
#endif
                    return f2(*elem(W,0), reduce(vdrop(1,w,v), v), v);
                }
                }
        }
    }
    }
    return null;
}

object areduce(object w, verb v){
    return DERIV('/', NULL, reduce, 0, w, 0, 0, 0, 0, 0);
}


object scan(object w, verb v){
    DECLFG;
    switch(gettag(w)){
    case LITERAL: return w;
    case ARRAY: {
        array W = getptr(w);
        switch(W->rank){
        case 1: switch(W->dims[0]){
                case 0: return getfill(fv->id);
                case 1: return *elem(W,0);
                default: {
                    array z = array_new_rank_pdims(W->rank, W->dims);
                    int n = W->dims[0];
#if 0
                    *elem(z,0) = *elem(W,0);
                    for (int i=1; i<n; ++i)
                        *elem(z,i) = f2(*elem(z,i-1), *elem(W,i), v);
#endif
                    for (int i=0; i<n; ++i)
                        *elem(z,i) = reduce(vtake(i+1,w,v), v);
                    return cache(ARRAY, z);
                }
                }
        }
    }
    }
    return null;
}

object ascan(object w, verb v){
    return DERIV('\\', NULL, scan, 0, w, 0, 0, 0, 0, 0);
}


object backscan(object w, verb v){
    DECLFG;
    switch(gettag(w)){
    case LITERAL: return w;
    case ARRAY: {
        array W = getptr(w);
        switch(W->rank){
        case 1: switch(W->dims[0]){
                case 0: return getfill(fv->id);
                case 1: return *elem(W,0);
                default: {
                    array z = array_new_rank_pdims(W->rank, W->dims);
                    int n = W->dims[0];
                    for (int i=0; i<n; ++i)
                        *elem(z,i) = reduce(vdrop(i, w, v), v);
                    return cache(ARRAY, z);
                }
                }
        }
    }
    }
    return null;
}

object abackscan(object w, verb v){
    return DERIV(0x2340, NULL, backscan, 0, w, 0, 0, 0, 0, 0);
}

void init_av(symtab st){
    verb v;
#define nnone 0
#define mnone 0
#define dnone 0
    ADVERBS_FOREACH(ADVERBTAB_DEF)
#undef nnone
#undef mnone
#undef dnone
}

