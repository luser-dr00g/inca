#include "common.h"

#define ADVERBS_FOREACH(param,_) \
/*base,   nilad, monad,     dyad,  f, g, h, mr,lr,rr*/ \
_(param,'&',    nnone, mnone,     amp,   0, 0, 0, 0, 0, 0 ) \
_(param,'@',    nnone, mnone,     atop,  0, 0, 0, 0, 0, 0 ) \
_(param,'/',    nnone, areduce,   dnone, 0, 0, 0, 0, 0, 0 ) \
_(param,'\\',   nnone, ascan,     dnone, 0, 0, 0, 0, 0, 0 ) \
_(param,0x2340, nnone, abackscan, dnone, 0, 0, 0, 0, 0, 0 ) \
/**/
/* see verbs.h for struct verb {} def */

#define nnone vnil
#define mnone areduce
#define dnone amp
#define DECLARE_ADVERB_FUNCTIONS(param,id, fnilad, fmonad, fdyad, ...) \
    nilad fnilad; \
    monad fmonad; \
    dyad fdyad;
ADVERBS_FOREACH(0,DECLARE_ADVERB_FUNCTIONS)
#undef nnone
#undef mnone
#undef dnone
#undef DECLARE_ADVERB_FUNCTIONS

void init_av(symtab st);

