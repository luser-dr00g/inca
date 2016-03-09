/* Xverbs are an abstraction to handle the '/'
 * which can be a verb or an adverb.
 *
 */

#define XVERBTAB(_) \
    /*name verb adverb*/\
    _('/', 0x1f, '/') \
/**/

#include "verbs.h"
#include "symtab.h"

typedef struct xverb {
    int base;
    verb verb;
    verb adverb;
} *xverb;

#define XVERBTAB_DEF(id, verb, adverb) \
    p=&(int[]){verb}; \
    n=1; \
    t=findsym(st, &p, &n, 0); \
    v=t->val; \
    p=&(int[]){adverb}; \
    n=1; \
    t=findsym(st, &p, &n, 0); \
    a=t->val; \
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

