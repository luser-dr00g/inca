
#define VERBTAB_ENUM(name, ...) \
    VERB_ ## name,
enum { VERBTAB(VERBTAB_ENUM) VERB_NOOP };

int vtab[VERB_NOOP];

int vectorindexleft(int a, int w, verb v);

// yield verb from verbtab given enum short name
#define VT(x) getptr(vtab[VERB_##x])

#define mnone 0
#define dnone 0
#define VERBTAB_DEF(name, id,...) \
    v=malloc(sizeof*v); \
    *v=(struct verb){newdata(PCHAR, id), __VA_ARGS__}; \
    def(st, newdata(PCHAR, id), vtab[VERB_##name] = cache(VERB, v));
#undef mnone
#undef dnone


#define scalarop(a,func,w,op,v) \
    switch(gettag(a)){ \
aliteral: \
    case LITERAL: switch(gettag(w)){ \
        case LITERAL: return newdata(LITERAL, getval(a) op getval(w)); \
        case ARRAY: { \
                array W = getptr(w); \
                array Z=array_new_rank_pdims(W->rank,W->dims); \
                int n=productdims(W->rank,W->dims); \
                int scratch[W->rank]; \
                for (int i=0; i<n; ++i){ \
                    vector_index(i,W->dims,W->rank,scratch); \
                    *elema(Z,scratch) = func(a, *elema(W,scratch), v); \
                } \
                return cache(ARRAY, Z); \
        } \
    } \
    case ARRAY: { \
        array A = getptr(a); \
        if (A->rank == 1 && A->dims[0] == 1) { \
            a = *elem(A,0); \
            goto aliteral; \
        } \
        switch(gettag(w)){ \
        case LITERAL: { \
                array Z=array_new_rank_pdims(A->rank,A->dims); \
                int n=productdims(A->rank,A->dims); \
                int scratch[A->rank]; \
                for (int i=0; i<n; ++i){ \
                    vector_index(i,A->dims,A->rank,scratch); \
                    *elema(Z,scratch) = func(*elema(A,scratch), w, v); \
                } \
                return cache(ARRAY, Z); \
        } \
        case ARRAY: { \
                array W = getptr(w); \
                array Z=array_new_rank_pdims(W->rank,W->dims); \
                int n=productdims(W->rank,W->dims); \
                int scratch[W->rank]; \
                for (int i=0; i<n; ++i){ \
                    vector_index(i,W->dims,W->rank,scratch); \
                    *elema(Z,scratch) = \
                        func(*elema(A,scratch), *elema(W,scratch), v); \
                } \
                return cache(ARRAY, Z); \
        } \
        } \
    } \
    }

#define scalaropfunc(a,f,w,func,v) \
    switch(gettag(a)){ \
aliteral: \
    case LITERAL: switch(gettag(w)){ \
        case LITERAL: return newdata(LITERAL, func(getval(a), getval(w))); \
        case ARRAY: { \
                array W = getptr(w); \
                array Z=array_new_rank_pdims(W->rank,W->dims); \
                int n=productdims(W->rank,W->dims); \
                int scratch[W->rank]; \
                for (int i=0; i<n; ++i){ \
                    vector_index(i,W->dims,W->rank,scratch); \
                    *elema(Z,scratch) = f(a, *elema(W,scratch), v); \
                } \
                return cache(ARRAY, Z); \
        } \
    } \
    case ARRAY: { \
        array A = getptr(a); \
        if (A->rank == 1 && A->dims[0] == 1) { \
            a = *elem(A,0); \
            goto aliteral; \
        } \
        switch(gettag(w)){ \
        case LITERAL: { \
                array Z=array_new_rank_pdims(A->rank,A->dims); \
                int n=productdims(A->rank,A->dims); \
                int scratch[A->rank]; \
                for (int i=0; i<n; ++i){ \
                    vector_index(i,A->dims,A->rank,scratch); \
                    *elema(Z,scratch) = f(*elema(A,scratch), w, v); \
                } \
                return cache(ARRAY, Z); \
        } \
        case ARRAY: { \
                array W = getptr(w); \
                array Z=array_new_rank_pdims(W->rank,W->dims); \
                int n=productdims(W->rank,W->dims); \
                int scratch[W->rank]; \
                for (int i=0; i<n; ++i){ \
                    vector_index(i,W->dims,W->rank,scratch); \
                    *elema(Z,scratch) = \
                        f(*elema(A,scratch), *elema(W,scratch), v); \
                } \
                return cache(ARRAY, Z); \
        } \
        } \
    } \
    }

#define scalarmonad(func,w,op,v) \
    switch(gettag(w)){ \
    case LITERAL: return newdata(LITERAL, op getval(w)); \
    case ARRAY: { \
        array W = getptr(w); \
        array Z=array_new_rank_pdims(W->rank,W->dims); \
        int n=productdims(W->rank,W->dims); \
        int scratch[W->rank]; \
        for (int i=0; i<n; ++i){ \
            vector_index(i,W->dims,W->rank,scratch); \
            *elema(Z,scratch) = func(*elema(W,scratch), v); \
        } \
        return cache(ARRAY, Z); \
    } \
    }

#define scalarmonadfunc(f,w,func,v) \
    switch(gettag(w)){ \
    case LITERAL: return newdata(LITERAL, func(getval(w))); \
    case ARRAY: { \
        array W = getptr(w); \
        array Z=array_new_rank_pdims(W->rank,W->dims); \
        int n=productdims(W->rank,W->dims); \
        int scratch[W->rank]; \
        for (int i=0; i<n; ++i){ \
            vector_index(i,W->dims,W->rank,scratch); \
            *elema(Z,scratch) = f(*elema(W,scratch), v); \
        } \
        return cache(ARRAY, Z); \
    } \
    }

