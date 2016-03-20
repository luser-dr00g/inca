#include "common.h"

#define ADVERBS_FOREACH(_) \
/*base,   nilad, monad,     dyad,  f, g, h, mr,lr,rr*/ \
_('&',    nnone, mnone,     amp,   0, 0, 0, 0, 0, 0 ) \
_('@',    nnone, mnone,     atop,  0, 0, 0, 0, 0, 0 ) \
_('/',    nnone, areduce,   dnone, 0, 0, 0, 0, 0, 0 ) \
_('\\',   nnone, ascan,     dnone, 0, 0, 0, 0, 0, 0 ) \
_(0x2340, nnone, abackscan, dnone, 0, 0, 0, 0, 0, 0 ) \
/**/
/* see verbs.h for struct verb {} def */

#define nnone vnil
#define mnone areduce
#define dnone amp
#define DECLARE_ADVERB_FUNCTIONS(id, fnilad, fmonad, fdyad, ...) \
    nilad fnilad; \
    monad fmonad; \
    dyad fdyad;
ADVERBS_FOREACH(DECLARE_ADVERB_FUNCTIONS)
#undef nnone
#undef mnone
#undef dnone
#undef DECLARE_ADVERB_FUNCTIONS

#define DERIV(id,...) \
    (v=malloc(sizeof*v), \
    *v=(struct verb){newdata(PCHAR, id), __VA_ARGS__}, \
    cache(VERB, v))

void init_av(symtab st);

