/* Xverbs are an abstraction to handle polymorphic symbols
 * such as '/' which can be a verb or an adverb.
 *
 * The verb and adverb must be defined with non-conflicting
 * identifiers. The xverb definition uses these two
 * definitions to select its components and then defines
 * the "superposition" under (presumably) one of the same
 * identifiers.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "encoding.h"
#include "symtab.h"
#include "verbs.h"
#include "xverb.h"
#include "debug.h"

#define XVERBTAB_DEF(id, vrb, adv) \
    p=(int[]){newdata(PCHAR, vrb)}; \
    n=1; \
    t=findsym(st, &p, &n, 0); \
    DEBUG(0,"X%08x(%d,%d)\n", \
            t->val, gettag(t->val), getval(t->val)); \
    v=getptr(t->val); \
\
    p=(int[]){newdata(PCHAR, adv)}; \
    n=1; \
    t=findsym(st, &p, &n, 0); \
    DEBUG(0,"X%08x(%d,%d)\n", \
            t->val, gettag(t->val), getval(t->val)); \
    a=getptr(t->val); \
\
    x=malloc(sizeof*x); \
    *x=(struct xverb){newdata(PCHAR, id), v, a}; \
    def(st, newdata(PCHAR, id), cache(XVERB, x));

void init_xverb(symtab st){
    verb a,v;
    xverb x;
    symtab t;
    int *p;
    int n;
    XVERBTAB(XVERBTAB_DEF)
}

