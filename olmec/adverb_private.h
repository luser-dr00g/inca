
#define ADVERBTAB_DEF(id,...) \
    v=malloc(sizeof*v); \
    *v=(struct verb){newdata(PCHAR, id), __VA_ARGS__}; \
    def(st, newdata(PCHAR, id), cache(ADVERB, v));

enum { NN, NV, VN, VV };
#define CONJCASE(a,w) \
    (qverb(a)*2+qverb(w))

#define DECLFG \
    verb fv = getptr(v->f); \
    int (*f1)(int,verb) = fv?fv->monad:0; \
    int (*f2)(int,int,verb) = fv?fv->dyad:0; \
    verb gv = getptr(v->g); \
    int (*g1)(int,verb) = gv?gv->monad:0; \
    int (*g2)(int,int,verb) = gv?gv->dyad:0; 


