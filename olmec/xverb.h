#ifndef XVERB_H_
#define XVERB_H_
#include "common.h"

#define XVERBS_FOREACH(param,_) \
    /*name verb adverb*/\
    _(param,'/', 0x1f, '/') \
    _(param,'\\', 0x1e, '\\') \
/**/
struct xverb {
    object id;
    verb verb;
    verb adverb;
};

void init_xverb(symtab st);

#endif
