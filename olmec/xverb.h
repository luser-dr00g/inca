
#define XVERBTAB(_) \
    /*name verb adverb*/\
    _('/', 0x1f, '/') \
    _('\\', 0x1e, '\\') \
/**/
typedef struct xverb {
    int id;
    verb verb;
    verb adverb;
} *xverb;

void init_xverb(symtab st);

