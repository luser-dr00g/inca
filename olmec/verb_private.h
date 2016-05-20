
#define VERBTAB_ENUM(param,name, ...) \
    VERB_ ## name,
enum { VERBS_FOREACH(0,VERBTAB_ENUM) VERB_NOOP };

extern object vtab[VERB_NOOP];

object vectorindexleft(object a, object w, verb v);

// yield verb from verbtab given enum short name
#define VT(x) getptr(vtab[VERB_##x])

#define nnone 0
#define mnone 0
#define dnone 0
#define DEFINE_VERB_IN_ENV(st, name, id,...) \
    v=malloc(sizeof*v); \
    *v=(struct verb){newdata(PCHAR, id), __VA_ARGS__}; \
    def(st, newdata(PCHAR, id), vtab[VERB_##name] = cache(VERB, v));
#undef nnone
#undef mnone
#undef dnone

#define SCALAROP(a,func,w,op,v) \
    scalarop(a,func,w,*#op,v)

#define SCALARMONAD(func,w,op,v) \
    scalarmonad(func,w,*#op,v)

object scalarop(object a, dyad func, object w, char op, verb v);
