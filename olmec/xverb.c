/* Xverbs are an abstraction to handle polymorphic symbols
 * such as '/' which can be a verb or an adverb.
 *
 */

#define XVERBTAB(_) \
    /*name verb adverb*/\
    _('/', 0x1f, '/') \
/**/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "encoding.h"
#include "symtab.h"
#include "verbs.h"
#include "xverb.h"

#define XVERBTAB_DEF(id, vrb, adv) \
    p=(int[]){newdata(PCHAR, vrb)}; \
    n=1; \
    t=findsym(st, &p, &n, 0); \
    printf("X%08x(%d,%d)\n", \
            t->val, gettag(t->val), getval(t->val)); \
    v=getptr(t->val); \
\
    p=(int[]){newdata(PCHAR, adv)}; \
    n=1; \
    t=findsym(st, &p, &n, 0); \
    printf("X%08x(%d,%d)\n", \
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

