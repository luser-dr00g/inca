
#include "en.h"

#define VERBTAB(_) \
/*base, monad, dyad, f, g, h, mr,lr,rr*/ \
_('+',  id,    plus, 0, 0, 0, 0, 0, 0 ) \
/**/
typedef struct verb {
    int id;
    int (*monad)(int);
    int (*dyad)(int,int);
    int f,g,h; /* operator arguments */
    int mr,lr,rr; /* monadic,left,right rank*/
} *verb;

void common(int *ap, int *wp){
}

int id (int w){ return w; }
int plus (int a, int w){
    common(&a,&w);
}

#define VERBTAB_DEF(id,...) \
    v=malloc(sizeof*v); \
    *v=(struct verb){id,__VA_ARGS__}; \
    def(st, newdata(CHAR, id), cache(VERB, v));

void init_vb(symtab st){
    verb v;
    VERBTAB(VERBTAB_DEF)
}

