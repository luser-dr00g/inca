#ifndef XVERB_H_
#define XVERB_H_
#include "common.h"

#define XVERBS_FOREACH(_) \
    /*name verb adverb*/\
    _('/', 0x1f, '/') \
    _('\\', 0x1e, '\\') \
/**/
struct xverb {
    int id;
    verb verb;
    verb adverb;
};

void init_xverb(symtab st);

#endif
