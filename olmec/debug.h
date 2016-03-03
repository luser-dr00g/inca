#ifdef DEBUGMODE
    #define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
    #define DEBUG(...)
#endif
