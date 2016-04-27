/* Xverbs are an abstraction to handle polymorphic symbols
 * such as '/' which can be a verb or an adverb.
 *
 * The verb and adverb must be defined with non-conflicting
 * identifiers. The xverb definition uses these two
 * definitions to select its components and then defines
 * the "superposition" under (presumably) one of the same
 * identifiers.
 */

#include <stdio.h>
#include <stdlib.h>

#include "encoding.h"
#include "symtab.h"
#include "verbs.h"
#include "xverb.h"

void define_xverb_in_env(int id, int vrb, int adv, symtab st){
    verb a,v;
    xverb x;
    symtab t;
    object *p;
    int n;

    p=(int[]){newdata(PCHAR, vrb)};
    n=1;
    t=findsym(st, &p, &n, 0);
    DEBUG(3,"X%08x(%d,%d)\n",
            t->value, gettag(t->value), getval(t->value));
    v=getptr(t->value);

    p=(int[]){newdata(PCHAR, adv)};
    n=1;
    t=findsym(st, &p, &n, 0);
    DEBUG(3,"X%08x(%d,%d)\n",
            t->value, gettag(t->value), getval(t->value));
    a=getptr(t->value);

    x=malloc(sizeof*x);
    *x=(struct xverb){newdata(PCHAR, id), v, a};
    def(st, newdata(PCHAR, id), cache(XVERB, x));
}

#define DEFINE_XVERB_IN_ENV(env,id, vrb, adv) \
    define_xverb_in_env(id, vrb, adv, env);

void init_xverb(symtab env){
    XVERBS_FOREACH(env,DEFINE_XVERB_IN_ENV)
}

