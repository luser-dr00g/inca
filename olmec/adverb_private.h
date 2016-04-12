
#define ADVERBTAB_DEF(id,...) \
    v=malloc(sizeof*v); \
    *v=(struct verb){newdata(PCHAR, id), __VA_ARGS__}; \
    def(st, newdata(PCHAR, id), cache(ADVERB, v));

enum { NN, NV, VN, VV };
#define CONJCASE(a,w) \
    (qverb(a)*2+qverb(w))


