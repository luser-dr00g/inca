#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "encoding.h"
#include "symtab.h"

#include "verbs.h"
#include "adverbs.h"
#include "verb_private.h"
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
            if (W->func == constant){
                W->data[1] = vsignum(W->data[1], v);
                return w;
            }
        } 
        array Z = array_new_rank_pdims(W->rank, W->dims);
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
            if (W->func == constant){ W->data[1] *= getval(a); }
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
        //case LITERAL: return 1;
        case ARRAY: {
            array a = getptr(w);
            int n = productdims(a->rank, a->dims);
            //if (n)
                return cache(ARRAY, cast_dims(a->dims,a->rank));
        }
    }
    return cache(ARRAY, vector_n(0));
}

void mcopy(int *dest, int *src, int n){
    int i;
    for (i=0; i<n; i++)
        dest[i] = src[i];
}

int vreshape (int a, int w, verb v){
    switch(gettag(a)){
    case LITERAL:
        if (getval(a)==0) return newdata(LITERAL, gettag(w));
        switch(gettag(w)){
        case LITERAL: 
            return cache(ARRAY, array_new_function(1,&a, (int[]){1,w},2,constant));
        case ARRAY: {
            int n=getval(a);
            array W=getptr(w);
            int wn=productdims(W->rank,W->dims);
            int scratch[W->rank];
            array z=array_new_dims(n);
            wn=n>wn?wn:n;
            for (int i=0; i<wn; i++)
                z->data[i] = *elema(W, vector_index(i,W->dims,W->rank,scratch));
            if((n-=wn)>0) mcopy(z->data+wn,z->data,n);
            return cache(ARRAY, z);
        }
        }//switch
    case ARRAY:
        switch(gettag(w)){
        case LITERAL: {
            array A=getptr(a);
            A = makesolid(A);
            array z=array_new_function(A->dims[0],A->data, (int[]){1,w},2,constant);
            return cache(ARRAY, z);
        }
        case ARRAY: {
            array A=getptr(a);
            if (A->rank != 1){ printf("RANK ERROR\n"); return null; }
            array W=getptr(w);
            if (W->type==function && W->func==constant)
                return vreshape(a,W->data[1],v);
            A = makesolid(A);
            int n=productdims(A->dims[0], A->data);
            array z=array_new_rank_pdims(A->dims[0], A->data);
            int wn=productdims(W->rank, W->dims);
            int scratch[W->rank];
            wn=n>wn?wn:n;
            for (int i=0; i<wn; i++)
                z->data[i] = *elema(W, vector_index(i,W->dims,W->rank,scratch));
            if ((n-=wn)>0) mcopy(z->data+wn,z->data,n);
            return cache(ARRAY, z);
        }
        }//switch
    }//switch
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
            array W = getptr(w);
            array z = copy(W);
            z->rank = 1; 
            z->dims[0] = productdims(W->rank, W->dims);
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
    if (gettag(a) == ARRAY) {
        array A = getptr(a);
        if (productdims(A->rank, A->dims) > 1) {
            printf("LENGTH ERROR\n");
            return null;
        }
        a = *elem(A,0);
    }
    if (gettag(w) == LITERAL) w = vravel(w, v);
    switch(gettag(a)){
    case LITERAL:
        a = getval(a);
        switch(gettag(w)){
        case ARRAY: {
            array W = getptr(w);
            int n = productdims(W->rank, W->dims);
            if (a > 0) {
                if (a <= n) return a==n?w:vindexleft(viota(a, v), w, v);
                else return vcat(w, vreshape(a-n, null, v), v);
            } else if (a < 0){
                if (a >= -n) return vindexleft(vplus(viota(-a, v), n+a, v), w, v);
                else return vcat(vreshape(abs(a+n), null, v), w, v);
            } else
                return null;
        }
        }
    }
    return null;
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

static inline int signof(int x){
    return x>=0? x==0? 0: 1: -1;
}

int vdrop (int a, int w, verb v){
    if (gettag(a)==ARRAY){
        array A = getptr(a);
        if (productdims(A->rank, A->dims) > 1){
            printf("LENGTH ERROR\n");
            return null;
        }
        a = *elem(A,0);
    }
    switch(gettag(a)){
    case LITERAL:
        switch(gettag(w)){
        case LITERAL:
            w = vravel(w,v); // fallthrough
        case ARRAY:
            a = getval(a);
            array W = getptr(w);
            int n = productdims(W->rank, W->dims);
            if (a==0) return w;
            if (0 <= abs(a) && abs(a) <= n){
                //printf("a=%d,n=%d,a-signof(a)*n=%d\n", a, n, a-signof(a)*n);
                return vtake(a-signof(a)*n, w, v);
            }
        }
    }
    return null;
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
    case ARRAY: {
        array z=copy(getptr(a));
        int n=productdims(z->rank,z->dims);
        for (int i=0; i< n; ++i)
            z->data[i] = vindexleft(z->data[i], w, v);
        return cache(ARRAY, z);
    }
    } //switch
}

int vindexright(int a, int w, verb v){
    return vindexleft(w,a,v);
}


int vbase(int a, int w, verb v){
    printf("base\n");
    switch(gettag(a)){
    case LITERAL:
        switch(gettag(w)){
            case LITERAL: break;
            case ARRAY: a = vreshape(vshapeof(w,v),a,v); break;
        }
    case ARRAY:
        switch(gettag(w)){
            case LITERAL: w = vreshape(vshapeof(a,v),w,v); break;
        }
    }
    int plus = DERIV('+',vid,vplus,0,0,0,0,0,0);
    int pr = areduce(plus,v);
    int (*plusreduce)(int,verb) = ((verb)getptr(pr))->monad;
    int times = DERIV(MODE1('='),vsignum,vtimes,0,0,0,0,0,0);
    int ts = abackscan(times,v);
    int (*timesscan)(int,verb) = ((verb)getptr(ts))->monad;
    int drop = DERIV('d',0,vdrop,0,0,0,0,0,0);
    int cat = DERIV(',',0,vcat,0,0,0,0,0,0);
    return plusreduce(
            vtimes(w,
                vdrop(1,
                    vcat(
                        timesscan(a,getptr(ts))
                        ,
                        1,
                        getptr(cat)),
                    getptr(drop)),
                getptr(times)),
            getptr(pr))
            ;
}

int vencode(int a, int w, verb v){
    printf("encode\n");
}

int vcompress(int a, int w, verb v){
    printf("compress\n");
}

int vexpand(int a, int w, verb v){
    printf("expand\n");
}




void init_vb(symtab st){
    verb v;
#define mnone 0
#define dnone 0
    VERBTAB(VERBTAB_DEF)
#undef mnone
#undef dnone
}

