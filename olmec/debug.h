#ifdef DEBUGMODE
    #define DEBUG(LVL,...) if (LVL<=DEBUGMODE) fprintf(stderr, __VA_ARGS__)
    #define IFDEBUG(LVL,...) do if (LVL<=DEBUGMODE) { __VA_ARGS__; } while(0)
#else
    #define DEBUG(...)
    #define IFDEBUG(...)
#endif
