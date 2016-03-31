
#define ADVERBTAB_DEF(id,...) \
    v=malloc(sizeof*v); \
    *v=(struct verb){newdata(PCHAR, id), __VA_ARGS__}; \
    def(st, newdata(PCHAR, id), cache(ADVERB, v));

enum { NN, NV, VN, VV };
#define CONJCASE(a,w) \
    (qverb(a)*2+qverb(w))

#define DECLFG \
    verb fv = getptr(v->f); \
    monad *f1 = fv?fv->monad:0; \
    dyad *f2 = fv?fv->dyad:0; \
    verb gv = getptr(v->g); \
    monad *g1 = gv?gv->monad:0; \
    dyad *g2 = gv?gv->dyad:0; 


