
#define mnone vid
#define dnone vplus
#define VERBTAB_DECL(base, monad, dyad, ...) \
    int monad(int,verb); \
    int dyad(int,int,verb);
VERBTAB(VERBTAB_DECL)
#undef mnone
#undef dnone

#define mnone 0
#define dnone 0
#define VERBTAB_DEF(id,...) \
    v=malloc(sizeof*v); \
    *v=(struct verb){newdata(PCHAR, id), __VA_ARGS__}; \
    def(st, newdata(PCHAR, id), cache(VERB, v));
#undef mnone
#undef dnone

#define scalarop(a,func,w,op,v) \
    switch(gettag(a)){ \
    case LITERAL: switch(gettag(w)){ \
        case LITERAL: return newdata(LITERAL, getval(a) op getval(w)); \
        case ARRAY: { \
                array W = getptr(w); \
                array Z=array_new_rank_pdims(W->rank,W->dims); \
                int n=productdims(W->rank,W->dims); \
                int scratch[W->rank]; \
                int i; \
                for (i=0; i<n; i++){ \
                    vector_index(i,W->dims,W->rank,scratch); \
                    *elema(Z,scratch) = func(a, *elema(W,scratch), v); \
                } \
                return cache(ARRAY, Z); \
        } \
    } \
    case ARRAY: { \
        array A = getptr(a); \
        switch(gettag(w)){ \
        case LITERAL: { \
                array Z=array_new_rank_pdims(A->rank,A->dims); \
                int n=productdims(A->rank,A->dims); \
                int scratch[A->rank]; \
                int i; \
                for (i=0; i<n; i++){ \
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
                int i; \
                for (i=0; i<n; i++){ \
                    vector_index(i,W->dims,W->rank,scratch); \
                    *elema(Z,scratch) = \
                        func(*elema(A,scratch), *elema(W,scratch), v); \
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
        int i; \
        for (i=0; i<n; i++){ \
            vector_index(i,W->dims,W->rank,scratch); \
            *elema(Z,scratch) = func(*elema(W,scratch), v); \
        } \
        return cache(ARRAY, Z); \
    } \
    }

