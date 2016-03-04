#ifdef DEBUGMODE
    #define DEBUG(LVL,...) if (LVL<=DEBUGMODE) fprintf(stderr, __VA_ARGS__)
#else
    #define DEBUG(...)
#endif
