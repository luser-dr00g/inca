#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ar.h"
#include "en.h"
#include "st.h"

#include "vb.h"

void common(int *ap, int *wp){
    //promote smaller number to matching type
}


int vid (int w, verb v){
    return w;
}

int vplus (int a, int w, verb v){
    common(&a,&w);
    switch(gettag(a)){
    case LITERAL:
        switch(gettag(w)){
        case LITERAL: return newdata(LITERAL, getval(a)+getval(w));
        case ARRAY: {
            array W = getptr(w);
            if (W->type == function){
                if (W->func == constant){ W->data[1] += a; }
                if (W->func == j_vector){ W->cons += a; }
                return w;
            } else {
            }
        }
        }
    }

    return null;
}


int vneg(int w, verb v){
    switch(gettag(w)){
    case LITERAL: return newdata(LITERAL, -getval(w));
    case ARRAY: {
        array W = getptr(w);
        if (W->type == function){
            if (W->func == constant){ W->data[1] = -W->data[1]; }
            if (W->func == j_vector){ W->weight[W->rank-1] *= -1; }
            return w;
        } else {
        }
    }
    }
    return null;
}

int vminus(int a, int w, verb v){
    common(&a,&w);
    switch(gettag(a)){
    case LITERAL:
        switch(gettag(w)){
        case LITERAL: return newdata(LITERAL, getval(a)-getval(w));
        case ARRAY: {
            array W = getptr(w);
            if (W->type == function){
                if (W->func == constant){ W->data[1] = getval(a)-W->data[1]; }
                if (W->func == j_vector){ // a - (i*x+y)
                    W->weight[W->rank-1] = -W->weight[W->rank-1];
                    W->cons = a - W->cons;
                }
                return w;
            } else {
            }
        }
        }
    }
    return null;
}


int vsignum (int w, verb v){
    switch(gettag(w)){
    case LITERAL: return getval(w)>0?1: getval(w)<0?-1: 0;
    case ARRAY: {
        array W = getptr(w);
        if (W->type == function){
            if (W->func == constant){ *W->data = vsignum(*W->data, v); }
            if (W->func == j_vector){
                array Z = array_new_dims(W->rank, W->dims);
                int n = productdims(W->rank, W->dims);
                int i;
                for (i=0; i<n; i++)
                    *elem(Z,i) = vsignum(*elem(W,i), v);
                return cache(ARRAY, Z);
            }
            return w;
        } else {
        }
    }
    }
    return null;
}

int vtimes (int a, int w, verb v){
    switch(gettag(a)){
    case LITERAL: 
        switch(gettag(w)){
        case LITERAL: return newdata(LITERAL, getval(a) * getval(w));
        case ARRAY: {
            array W = getptr(w);
            if (W->type == function){
                if (W->func == constant){ *W->data *= getval(a); }
                if (W->func == j_vector){
                    W->weight[W->rank-1] *= getval(a);
                    W->cons *= getval(a);
                }
                return w;
            } else {
            }
        }
        }
    }
}


int vshapeof (int w, verb v){
    switch(gettag(w)){
        case NULLOBJ: return newdata(LITERAL, 1);
        case ARRAY: {
            array a = getptr(w);
            return cache(ARRAY, cast(a->dims,a->rank));
        }
    }
    return null;
}

void mcopy(int *dest, int *src, int n){
    int i;
    for (i=0; i<n; i++)
        dest[i] = src[i];
}

int vreshape (int a, int w, verb v){
    switch(gettag(a)){
        case LITERAL:
            if (getval(a)==0)
                return newdata(LITERAL, gettag(w));
            switch(gettag(w)){
                case LITERAL: {
                    array z=array_new_function(1,&a,
                            (int[]){1,w},2,constant);
                    return cache(ARRAY, z);
                }
                case ARRAY: {
                    int n=getval(a);
                    array W=getptr(w);
                    int wn=productdims(W->rank,W->dims);
                    int scratch[W->rank];
                    array z=array_new(n);
                    int i;
                    wn=n>wn?wn:n;
                    for (i=0; i<wn; i++)
                        z->data[i] = *elema(W,vector_index(i,W->dims,W->rank,scratch));
                    if((n-=wn)>0) mcopy(z->data+wn,z->data,n);
                    return cache(ARRAY, z);
                }
            }
        case ARRAY:
            switch(gettag(w)){
                case LITERAL: {
                    array A=getptr(a);
                    array z=array_new_function(A->dims[0],A->data,
                            (int[]){1,w},2,constant);
                    return cache(ARRAY, z);
                }
                case ARRAY: {
                    array A=makesolid(getptr(a));
                    array W=getptr(w);
                    int n=productdims(A->dims[0], A->data);
                    array z=array_new_dims(A->dims[0], A->data);
                    int wn=productdims(W->rank, W->dims);
                    int scratch[W->rank];
                    int i;
                    wn=n>wn?wn:n;
                    for (i=0; i<wn; i++)
                        z->data[i] = *elema(W,vector_index(i,W->dims,W->rank,scratch));
                    if ((n-=wn)>0) mcopy(z->data+wn,z->data,n);
                    return cache(ARRAY, z);
                }
            }
    }
    return w;
}

int vtally (int w, verb v){
    switch(gettag(w)){
        case ARRAY: {
            array a = getptr(w);
            return newdata(LITERAL, a->rank?a->dims[0]:1);
        }
    }
    return newdata(LITERAL, 1);
}


int viota (int w, verb v){
    switch(gettag(w)){
        case LITERAL: return cache(ARRAY, iota(w));
        case ARRAY: {
            array W = getptr(w);
            printf("%d %d %d\n", W->rank, W->dims[0], W->data[0]);
            int n = productdims(W->dims[0],W->data);
            printf("%d\n", n);
            array I = iota(n);
            int i = cache(ARRAY, I);
            printf("%08x(%d,%d)\n", i, gettag(i), getval(i));
            int z = vreshape(w,i,v);
            printf("%08x(%d,%d)\n", z, gettag(z), getval(z));
            return z;
        }
    }
    return null;
}


int vravel (int w, verb v){
    switch(gettag(w)){
        case ARRAY: {
            array a = getptr(w);
            array z = copy(a);
            z->rank = 1; 
            z->dims[0] = productdims(a->rank, a->dims);
            return cache(ARRAY, z);
        }
    }
    return w;
}

int vcat (int a, int w, verb v){
    switch(gettag(a)){
        case ARRAY: 
            switch(gettag(w)){
                case ARRAY: return cache(ARRAY, cat(getptr(a),getptr(w)));
            }
    }
    return cache(ARRAY, vector(a,w));
}


int vraze (int w, verb v){
}

int vlink (int a, int w, verb v){
}


int box (int w, verb v){
}

int lessthan (int a, int w, verb v){
}


int unbox (int w, verb v){
}

int greaterthan (int a, int w, verb v){
}


int vhead (int w, verb v){
    switch(gettag(w)){
        case ARRAY: {
            array a = getptr(w);
            if (a->rank == 0) return getfill(w);
            return  cache(ARRAY, slice(a, 0));
        }
    }
    return null;
}

int vtake (int a, int w, verb v){
}


int vbehead (int w, verb v){
}

int vdrop (int a, int w, verb v){
}


#define VERBTAB_DEF(id,...) \
    v=malloc(sizeof*v); \
    *v=(struct verb){id,__VA_ARGS__}; \
    def(st, newdata(PCHAR, id), cache(VERB, v));

void init_vb(symtab st){
    verb v;
    VERBTAB(VERBTAB_DEF)
}

