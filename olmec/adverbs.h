
#define ADVERBS_FOREACH(_) \
/*base,   monad,     dyad,  f, g, h, mr,lr,rr*/ \
_('&',    mnone,     amp,   0, 0, 0, 0, 0, 0 ) \
_('@',    mnone,     atop,  0, 0, 0, 0, 0, 0 ) \
_('/',    areduce,   dnone, 0, 0, 0, 0, 0, 0 ) \
_('\\',   ascan,     dnone, 0, 0, 0, 0, 0, 0 ) \
_(0x2340, abackscan, dnone, 0, 0, 0, 0, 0, 0 ) \
/**/
/* see verbs.h for struct verb {} def */

#define mnone areduce
#define dnone amp
#define DECLARE_ADVERB_FUNCTIONS(id, fmonad, fdyad, ...) \
    monad fmonad; \
    dyad fdyad;
ADVERBS_FOREACH(DECLARE_ADVERB_FUNCTIONS)
#undef mnone
#undef dnone
#undef DECLARE_ADVERB_FUNCTIONS

#define DERIV(id,...) \
    (v=malloc(sizeof*v), \
    *v=(struct verb){newdata(PCHAR, id), __VA_ARGS__}, \
    cache(VERB, v))

void init_av(symtab st);

