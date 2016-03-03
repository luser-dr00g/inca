
#define scalarop(a,func,w,op,v) \
    switch(gettag(a)){ \
    case LITERAL: switch(gettag(w)){ \
        case LITERAL: return newdata(LITERAL, getval(a) op getval(w)); \
        case ARRAY: { \
                array W = getptr(w); \
                array Z=array_new_dims(W->rank,W->dims); \
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
                array Z=array_new_dims(A->rank,A->dims); \
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
                array Z=array_new_dims(W->rank,W->dims); \
                int n=productdims(W->rank,W->dims); \
                int scratch[W->rank]; \
                int i; \
                for (i=0; i<n; i++){ \
                    vector_index(i,W->dims,W->rank,scratch); \
                    *elema(Z,scratch) = func(*elema(A,scratch), *elema(W,scratch), v); \
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
        array Z=array_new_dims(W->rank,W->dims); \
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

