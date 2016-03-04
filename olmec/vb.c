#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ar.h"
#include "en.h"
#include "st.h"

#include "vb.h"
#include "vb_private.h"
#include "debug.h"

void common(int *ap, int *wp){
    //promote smaller number to matching type
}


int vid (int w, verb v){
    if (gettag(w)==ARRAY) return cache(ARRAY, makesolid(getptr(w)));
    return w;
}

int vplus (int a, int w, verb v){
    common(&a,&w);

    if (gettag(a)==LITERAL && gettag(w)==ARRAY){
        array W = getptr(w);
        if (W->type == function){
            if (W->func == constant){ W->data[1] += a; }
            if (W->func == j_vector){ W->cons += a; }
            return w;
        }
    }

    scalarop(a,vplus,w,+,v)

    return null;
}


int vneg(int w, verb v){
    if (gettag(w)==ARRAY){
        array W = getptr(w);
        if (W->type == function){
            if (W->func == constant){ W->data[1] = -W->data[1]; }
            if (W->func == j_vector){ W->weight[W->rank-1] *= -1; }
            return w;
        }
    }

    scalarmonad(vneg,w,-,v)

    return null;
}

int vminus(int a, int w, verb v){
    common(&a,&w);

    if (gettag(a)==LITERAL && gettag(w)==ARRAY){
        array W = getptr(w);
        if (W->type == function){
            if (W->func == constant){ W->data[1] = getval(a)-W->data[1]; }
            if (W->func == j_vector){ // a - (i*x+y)
                W->weight[W->rank-1] = -W->weight[W->rank-1];
                W->cons = a - W->cons;
            }
            return w;
        }
    }

    scalarop(a,vminus,w,-,v)

    return null;
}


int vdivide(int a, int w, verb v){
    common(&a, &w);

    scalarop(a,vdivide,w,/,v)

    return null;
}

int vrecip(int w, verb v){
    scalarop(1,vdivide,w,/,v)
    return null;
}


int vsignum (int w, verb v){
    switch(gettag(w)){
    case LITERAL: return getval(w)>0?1: getval(w)<0?-1: 0;
    case ARRAY: {
        array W = getptr(w);
        if (W->type == function){
            if (W->func == constant){ *W->data = vsignum(*W->data, v); }
            return w;
        } 
        array Z = array_new_dims(W->rank, W->dims);
        int n = productdims(W->rank, W->dims);
        int scratch[W->rank];
        int i;
        for (i=0; i<n; i++) {
            vector_index(i,W->dims,W->rank,scratch);
            *elema(Z,scratch) = vsignum(*elema(W,scratch), v);
        }
        return cache(ARRAY, Z);
    }
    }
    return null;
}

int vtimes (int a, int w, verb v){
    common(&a, &w);

    if (gettag(a)==LITERAL && gettag(w)==ARRAY){
        array W = getptr(w);
        if (W->type == function){
            if (W->func == constant){ *W->data *= getval(a); }
            if (W->func == j_vector){
                W->weight[W->rank-1] *= getval(a);
                W->cons *= getval(a);
            }
            return w;
        } 
    }

    scalarop(a,vtimes,w,*,v)

    return null;
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
                    for (i=0; i<wn; i++){
                        vector_index(i,W->dims,W->rank,scratch);
                        z->data[i] = *elema(W, scratch);
                    }
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
                    if (W->type==function && W->func==constant)
                        return vreshape(a,W->data[1],v);
                    int n=productdims(A->dims[0], A->data);
                    array z=array_new_dims(A->dims[0], A->data);
                    int wn=productdims(W->rank, W->dims);
                    int scratch[W->rank];
                    int i;
                    wn=n>wn?wn:n;
                    for (i=0; i<wn; i++){
                        vector_index(i,W->dims,W->rank,scratch);
                        z->data[i] = *elema(W, scratch);
                    }
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
            DEBUG(1,"%d %d %d\n", W->rank, W->dims[0], W->data[0]);
            int n = productdims(W->dims[0],W->data);
            DEBUG(1,"%d\n", n);
            array I = iota(n);
            int i = cache(ARRAY, I);
            DEBUG(1,"%08x(%d,%d)\n", i, gettag(i), getval(i));
            int z = vreshape(w,i,v);
            DEBUG(1,"%08x(%d,%d)\n", z, gettag(z), getval(z));
            return z;
        }
    }
    return null;
}


int vravel (int w, verb v){
    switch(gettag(w)){
        case LITERAL: {
            return cache(ARRAY, scalar(w));
        }
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
        case LITERAL:
            switch(gettag(w)){
                case ARRAY: return cache(ARRAY, cat(scalar(a),getptr(w)));
                case LITERAL: return cache(ARRAY, vector(a,w));
            }
        case ARRAY: 
            switch(gettag(w)){
                case ARRAY: return cache(ARRAY, cat(getptr(a),getptr(w)));
                case LITERAL: return cache(ARRAY, cat(getptr(a),scalar(w)));
            }
    }
    return cache(ARRAY, vector(a,w));
}


int vbox (int w, verb v){
    return cache(ARRAY, scalar(w));
}

int vlessthan (int a, int w, verb v){
}


int vunbox (int w, verb v){
    switch(gettag(w)){
        case ARRAY: {
            array W = getptr(w);
            if (W->rank==0
                || W->rank==1 && W->dims[0]==1)
            w = *elem(W,0);
        }
    }
    return w;
}

int vgreaterthan (int a, int w, verb v){
}


int vlink (int a, int w, verb v){
    switch(gettag(w)){
        case ARRAY: return vcat(vbox(a,v),w,v);
    }
    return vcat(vbox(a,v),vbox(w,v),v);
}

int vprenul (int w, verb v){
    return vlink(null,w,v);
}


int vhead (int w, verb v){
    switch(gettag(w)){
        case ARRAY: {
            array W = getptr(w);
            if (W->rank == 0) return getfill(w);
            return  cache(ARRAY, slice(W, 0));
        }
    }
    return null;
}

int vtake (int a, int w, verb v){
}


int vbehead (int w, verb v){
    switch(gettag(w)){
        case ARRAY: {
            array W = getptr(w);
            if (W->rank == 0) return getfill(w);
            int s[W->rank];
            int f[W->rank];
            s[0]=1;
            for (int i=1; i<W->rank; i++)
                s[i]=0;
            for (int i=0; i<W->rank; i++)
                f[i]=W->dims[i]-1;
            return cache(ARRAY, slices(W, s, f));
        }
    }
}

int vdrop (int a, int w, verb v){
}


int vindexleft(int a, int w, verb v){
    switch(gettag(a)){
    case LITERAL:
        switch(gettag(w)){
        case LITERAL: return a==0? w : getfill(w);
        case ARRAY: {
            array W = getptr(w);
            switch(W->rank){
                case 1: return a>=0&&a<W->dims[0]? *elem(W,a):
                        a>-W->dims[0]? *elem(W,W->dims[0]+a):
                        getfill(*elem(W,0));
                default:
                        return cache(ARRAY, slice(W, a));
            }
        }
        }
    case ARRAY:
        ;
    }
}

int vindexright(int a, int w, verb v){
    return vindexleft(w,a,v);
}


void init_vb(symtab st){
    verb v;
    VERBTAB(VERBTAB_DEF)
}

