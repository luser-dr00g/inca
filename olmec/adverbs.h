#include "common.h"

#define ADVERBS_FOREACH(param,_) \
/*      base,   nilad, monad,     dyad,  f, g, h, mr,lr,rr,mdesc,ddesc*/ \
_(param,'&',    nnone, mnone,     amp,   0, 0, 0, 0, 0, 0, none, compose functions or curry argument) \
_(param,'@',    nnone, mnone,     atop,  0, 0, 0, 0, 0, 0, none, compose functions ) \
_(param,'/',    nnone, areduce,   dnone, 0, 0, 0, 0, 0, 0, reduce using verb, none) \
_(param,'\\',   nnone, ascan,     dnone, 0, 0, 0, 0, 0, 0, scan using verb, none) \
_(param,0x2340, nnone, abackscan, dnone, 0, 0, 0, 0, 0, 0, scan right-to-left using verb, none) \
_(param,0x00a8, nnone, mnone,     rank,  0, 0, 0, 0, 0, 0, none, derive new verb with specified or borrowed rank) \
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

typedef struct {
    int result;
    object resultvar;
    int arity;
    object alpha;
    object func;
    object omega;
    int extra;
    object extravars;
} *analysis;

object del(array head, array body, symtab env, symtab child);
object dfn(object w, symtab env);
object amp(object a, object w, verb v);
object rank(object a, object w, verb v);
void init_av(symtab st);

