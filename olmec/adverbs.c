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
#include "print.h"

#include "adverbs.h"
#include "adverb_private.h"

typedef struct vtable {
    verb fv;
    monad *f1;
    dyad *f2;
    verb gv;
    monad *g1;
    dyad *g2;
} vtable;
vtable loadv(verb v){
    verb fv = getptr(v->f);
    monad *f1 = fv?fv->monad:0;
    dyad *f2 = fv?fv->dyad:0;
    verb gv = getptr(v->g);
    monad *g1 = gv?gv->monad:0;
    dyad *g2 = gv?gv->dyad:0; 
    return (vtable){fv,f1,f2,gv,g1,g2};
}

object create_derived_verb(
			   object id,
			   nilad *nilad,
			   monad *monad,
			   dyad *dyad,
			   object f, object g, object h,
			   int mr, int lr, int rr){
    verb v = malloc(sizeof*v);
    *v = (struct verb){ newdata(PCHAR, id), nilad, monad, dyad, f, g, h, mr, lr, rr};
    return cache(VERB, v);
}

object domerr(object w, verb v){
    return null;
}

object withl(object w, verb v){ vtable vt=loadv(v); return vt.g2(v->f, w, vt.gv); }
object withr(object w, verb v){ vtable vt=loadv(v); return vt.f2(w, v->g, vt.fv); }
object on1(object w, verb v){ vtable vt=loadv(v); return vt.f1(vt.g1(w,vt.gv),vt.fv); }
object on2(object a, object w, verb v){ vtable vt=loadv(v); return vt.f2(vt.g1(a,vt.gv),vt.g1(w,vt.gv),vt.fv); }

object amp(object a, object w, verb v){
    switch(CONJCASE(a,w)){
        case NN: return domerr(0,v);
        case NV: return create_derived_verb('&', NULL, withl, NULL, a, w, 0, 0, 0, 0);
        case VN: return create_derived_verb('&', NULL, withr, NULL, a, w, 0, 0, 0, 0);
        case VV: return create_derived_verb('&', NULL, on1,   on2,  a, w, 0, 0, 0, 0);
    }
}


object atop2(object a, object w, verb v){ vtable vt=loadv(v); return vt.f1(vt.g2(a,w,vt.gv),vt.fv); }

object atop(object a, object w, verb v){
    switch(CONJCASE(a,w)){
        case NN: return domerr(0,v);
        case NV: return domerr(0,v);
        case VN: return domerr(0,v);
        case VV: {
            v = getptr(w);
            return create_derived_verb('@', NULL, on1, atop2, a, w, 0, v->mr, v->lr, v->rr);
        } 
    }
}


object reduce(object w, verb v){
    vtable vt=loadv(v);
    switch(gettag(w)){
    case LITERAL: return w;
    case ARRAY: {
        array W = getptr(w);
        switch(W->rank){
        case 1: switch(W->dims[0]){
                case 0: return getfill(vt.fv->id);
                case 1: return *elem(W,0);
                default: {
#if 0
                    int z=*elem(W,W->dims[0]-1);
                    for (int i=W->dims[0]-2; i>=0; i--)
                        z=f2(*elem(W,i),z,v);
#endif
                    return vt.f2(*elem(W,0), reduce(vdrop(1,w,v), v), v);
                }
                }
        }
    }
    }
    return null;
}

object areduce(object w, verb v){
    return create_derived_verb('/', NULL, reduce, 0, w, 0, 0, 0, 0, 0);
}


object scan(object w, verb v){
    vtable vt=loadv(v);
    switch(gettag(w)){
    case LITERAL: return w;
    case ARRAY: {
        array W = getptr(w);
        switch(W->rank){
        case 1: switch(W->dims[0]){
                case 0: return getfill(vt.fv->id);
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
    return create_derived_verb('\\', NULL, scan, 0, w, 0, 0, 0, 0, 0);
}


object backscan(object w, verb v){
    vtable vt=loadv(v);
    switch(gettag(w)){
    case LITERAL: return w;
    case ARRAY: {
        array W = getptr(w);
        switch(W->rank){
        case 1: switch(W->dims[0]){
                case 0: return getfill(vt.fv->id);
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
    return create_derived_verb(0x2340, NULL, backscan, 0, w, 0, 0, 0, 0, 0);
}


object rank(object a, object w, verb v){
    switch(CONJCASE(a,w)){
        case NN: return null;
        case NV: return null;
        case VN: {
            array W = getptr(w);
            verb va = getptr(a);
            switch(W->dims[0]){
                default:
                case 0: return null;
                case 1: return
                        create_derived_verb(getval(va->id),
                                va->nilad, va->monad, va->dyad,
                                0, 0, 0, *elem(W,0), *elem(W,0), *elem(W,0));
                case 2: return
                        create_derived_verb(getval(va->id),
                                va->nilad, va->monad, va->dyad,
                                0, 0, 0, *elem(W,1), *elem(W,0), *elem(W,1));
                case 3: return
                        create_derived_verb(getval(va->id),
                                va->nilad, va->monad, va->dyad,
                                0, 0, 0, *elem(W,0), *elem(W,1), *elem(W,2));
            }
        }
        case VV: {
            verb va = getptr(a);
            verb vw = getptr(w);
            return create_derived_verb(getval(va->id),
                    va->nilad, va->monad, va->dyad,
                    va->f, va->g, va->h, vw->mr, vw->lr, vw->rr);
        }
    }
}

int contains(object needle, object haystack){
    switch (gettag(haystack)){
    case PCHAR:
        return needle == haystack;
    case ARRAY:
    case PROG:
        {
            array a = getptr(haystack);
            for (int i=0; i<a->dims[0]; i++){
                if (contains(needle, *elem(a, i)))
                    return 1;
            }
        }
    }
    return 0;
}

object dfn(object w, symtab env){
    DEBUG(1, "dfn %08x(%d,%d)\n", w, gettag(w), getval(w));
    IFDEBUG(1, print(w, 0););
    int has_alpha = contains(newdata(PCHAR, 0x237a), w);
    int has_omega = contains(newdata(PCHAR, 0x2375), w);
    return create_derived_verb(newdata(CHAR, 'D'),
                !has_alpha && !has_omega ? ndfn : 0,
                !has_alpha && has_omega ? mdfn : 0,
                has_alpha && has_omega ? ddfn : 0,
                w, cache(SYMTAB, env), 0, 0, 0, 0);
}


void adverbtab_def(
        object id,
        nilad *nilad,
        monad *monad,
        dyad *dyad,
        object f, object g, object h, /* operator arguments */
        int mr, int lr, int rr, /* monadic,left,right rank*/
        symtab st){
    verb v;
    v=malloc(sizeof*v);
    *v=(struct verb){newdata(PCHAR, id), nilad, monad, dyad, f,g,h, mr,lr,rr};
    def(st, newdata(PCHAR, id), cache(ADVERB, v));
}

#define ADVERBTAB_DEF(st, id,...) \
    adverbtab_def(id, __VA_ARGS__, st);

void init_av(symtab env){
#define nnone 0
#define mnone 0
#define dnone 0
    ADVERBS_FOREACH(env, ADVERBTAB_DEF)
#undef nnone
#undef mnone
#undef dnone
}

