/* Verbs
 *
 * Verbs are the "actions" that the interpreter performs upon
 * the 'nouns' which are their arguments. As described in
 * the encoding module, many data types are packed into 
 * integer handles; thus most verbs have a very simple 
 * function signature:
 *
 *   int monad(int a, verb v);
 *   int dyad(int a, int w, verb v);
 *
 *  a and w are always the left and right arguments, respectively.
 *  They are named after 'alpha' and 'omega' which they visually
 *  resemble. The verb argument is a pointer to a verb structure 
 *  containing the verb's instance data. If it is a derived verb,
 *  resulting from an adverb, then the verb struct will contain
 *  the base verb used by the derived verb.
 *
 *  Verb functions use the encoding functions gettag(), getval(),
 *  and getptr() to crack the integer handles into their hidden
 *  structures. 
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "array.h"
#include "encoding.h"
#include "symtab.h"

#include "verbs.h"
#include "adverbs.h"
#include "verb_private.h"
#include "print.h"

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
            if (W->func == constant){
                W->data[1] = getval(a)-W->data[1];
                return w;
            }
#if 0
            if (W->func == j_vector){ // a - (i*x+y)
                W->weight[W->rank-1] = -W->weight[W->rank-1];
                W->cons = a - W->cons;
                return w;
            }
#endif
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
    //scalarop(1,vdivide,w,/,v)
    return vdivide(1, w, VT(DIV));
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


int vabs (int w, verb v){
    scalarmonadfunc(vabs, w, (abs), v);

    return null;
}

static inline void swap(int *x, int *y){
    int t = *x;
            *x = *y;
                 *y = t;
}

int resid(int a, int w){
    if (a==0) return w;
    if (a<0)
        return -resid(-a,w);
    if (w<0)
        return resid(a,w+a);
    return w % a;
}

int vresidue (int a, int w, verb v){
    DEBUG(1,"residue\n");
    IFDEBUG(1,print(a, 0));
    IFDEBUG(1,print(w, 0));
    common(&a, &w);

    scalaropfunc(a,vresidue,w,resid,v)

    return null;
}


int vshapeof (int w, verb v){
    switch(gettag(w)){
        //case LITERAL: return 1;
        case ARRAY: {
            array a = getptr(w);
            if (a->rank==0)
                return nil;
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
    DEBUG(1,"reshape\n");
    IFDEBUG(1,print(a, 0));
    IFDEBUG(1,print(w, 0));
recheck:
    switch(gettag(a)){
    case LITERAL:
        if (getval(a)==0) return newdata(LITERAL, gettag(w));
        switch(gettag(w)){
        case CHAR: {
            int n = getval(a);
            array z=array_new_dims(n);
            for (int i=0; i<n; ++i)
                z->data[i] = w;
            return cache(ARRAY, z);
        }
        case LITERAL: 
            return cache(ARRAY, array_new_function(1,&a, (int[]){1,w},2,constant));
        case ARRAY: {
            int n=getval(a);
            if (n==0) return nil;
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
        case CHAR: {
            array A = getptr(a);
            if (A->rank != 1){ printf("RANK ERROR\n"); return null; }
            A = makesolid(A);
            int n = productdims(A->dims[0], A->data);
            array z = array_new_rank_dims(A->dims[0], A->data);
            for (int i=0; i<n; ++i)
                z->data[i] = w;
            return cache(ARRAY, z);
        }
        case LITERAL: {
            array A=getptr(a);
            A = makesolid(A);
            array z=array_new_function(A->dims[0],A->data, (int[]){1,w},2,constant);
            return cache(ARRAY, z);
        }
        case ARRAY: {
            array A=getptr(a);
            if (A->rank == 0){ a = A->data[0]; goto recheck; }
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
recheck:
    switch(gettag(w)){
        case LITERAL: return cache(ARRAY, iota(w));
        case ARRAY: {
            array W = getptr(w);
            if (W->rank == 0){ w = W->data[0]; goto recheck; }
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
#if 0
            array W = getptr(w);
            array z = copy(W);
            z->rank = 1; 
            z->dims[0] = productdims(W->rank, W->dims);
#endif
            array W = getptr(w);
            int n = productdims(W->rank, W->dims);
            array Z = array_new_dims(n);
            int scratch[W->rank];
            for (int i=0; i<n; ++i)
                *elem(Z, i) = *elema(W, vector_index(i, W->dims, W->rank, scratch));
            return cache(ARRAY, Z);
        }
    }
    return w;
}

int vcat (int a, int w, verb v){
    DEBUG(1,"cat\n");
    IFDEBUG(1, print(a,0));
    IFDEBUG(1, print(w,0));
    switch(gettag(a)){
        case NULLOBJ:
            return w;
        case CHAR:
        case LITERAL:
            switch(gettag(w)){
                case NULLOBJ: return a;
                case ARRAY: return cache(ARRAY, cat(scalar(a),getptr(w)));
                case CHAR:
                case LITERAL: return cache(ARRAY, vector(a,w));
            }
        case ARRAY: 
            switch(gettag(w)){
                case NULLOBJ: return a;
                case ARRAY: return cache(ARRAY, cat(getptr(a),getptr(w)));
                case CHAR:
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
                if (a <= n) return a==n?w:vectorindexleft(viota(a, v), w, v);
                else return vcat(w, vreshape(a-n, getfill(w), v), v);
            } else if (a < 0){
                if (a >= -n) return vectorindexleft(vplus(viota(-a, v), n+a, v), w, v);
                else return vcat(vreshape(abs(a+n), getfill(w), v), w, v);
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
    DEBUG(1, "drop\n");
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
                DEBUG(1,"a=%d,n=%d,a-signof(a)*n=%d\n", a, n, a-signof(a)*n);
                return vtake(a-signof(a)*n, w, v);
            }
        }
    }
    DEBUG(1,"nil= %08x(%d,%d)\n", nil, gettag(nil), getval(nil));
    return nil;
}


int vectorindexleft(int a, int w, verb v){
    switch(gettag(a)){
    case LITERAL:
        switch(gettag(w)){
        case LITERAL: return a==0? w : getfill(w);
        case ARRAY: {
            array W = getptr(w);
            switch(W->rank){
                case 1: return a>=0&&a<W->dims[0]? *elem(W,a):
                        a>-W->dims[0]? *elem(W,W->dims[0]+a):
                        //getfill(*elem(W,0))
                        null
                        ;
                default:
                        return cache(ARRAY, slice(W, a));
            }
        }
        }
    case ARRAY: {
        array z=copy(getptr(a));
        int n=productdims(z->rank,z->dims);
        for (int i=0; i< n; ++i)
            z->data[i] = vectorindexleft(z->data[i], w, v);
        return cache(ARRAY, z);
    }
    } //switch
}

int unitindexleft(int a, int w, verb v){
    DEBUG(1, "unitindexleft\n");
    IFDEBUG(1, print(a, 0));
    IFDEBUG(1, print(w, 0));
recheck:
    switch(gettag(a)){
    case LITERAL:
        DEBUG(1, "LIT\n");
            return vectorindexleft( vbase( vshapeof(w, VT(RHO)),
                        vreveal(a, VT(REVL)), VT(BASE)),
                    vravel(w, VT(CAT)), VT(INDL));
    case ARRAY:
        {
            array A = getptr(a);
            if (A->rank==0 ||
                    (A->rank == 1 && A->dims[0] == 1)) {
                DEBUG(1, "VEC\n");
                return vectorindexleft( vbase( vshapeof(w, VT(RHO)),
                            vreveal(a, VT(REVL)), VT(BASE)),
                        vravel(w, VT(CAT)), VT(INDL));
            }
            DEBUG(1, "ARR\n");
            if (A->rank == 1) {
                array Z = array_new_rank_dims(A->rank, A->dims);
                int n = A->dims[0]; //productdims(A->rank, A->dims);
                for (int i=0; i<n; ++i)
                    Z->data[i] = vindexleft(*elem(A,i), w, VT(INDL));
                return cache(ARRAY, Z);
            } else {
                return vreshape(vshapeof(a, VT(RHO)),
                            vindexleft(vravel(a, VT(CAT)), w, VT(INDL)), VT(RHO));
            }
        }
    }
    return null;
}


int vindexleft(int a, int w, verb v){
    return unitindexleft(a, w, v);
    //return vectorindexleft(a, w, v);
}

int vindexright(int a, int w, verb v){
    return vindexleft(w,a,v);
}


int vbase(int a, int w, verb v){
    switch(gettag(a)){
    case LITERAL:
        switch(gettag(w)){
            case LITERAL: break;
            case ARRAY: a = vreshape(vshapeof(w,VT(RHO)),a,VT(RHO)); break;
        }
    case ARRAY:
        switch(gettag(w)){
            case LITERAL: w = vreshape(vshapeof(a,VT(RHO)),w,VT(RHO)); break;
            case ARRAY: {
                array A = getptr(a);
                array W = getptr(w);
                if (A->rank == 0)
                    a = vreshape(vshapeof(w,VT(RHO)),a,VT(RHO));
                if (W->rank == 0)
                    w = vreshape(vshapeof(a,VT(RHO)),w,VT(RHO));
            }
        }
    }
    int pr = areduce(vtab[VERB_PLUS],v);
    int (*plusreduce)(int,verb) = ((verb)getptr(pr))->monad;
    int times = DERIV(MODE1('='),vsignum,vtimes,0,0,0,0,0,0);
    int ts = abackscan(vtab[VERB_MUL],v);
    int (*timesscan)(int,verb) = ((verb)getptr(ts))->monad;
    return plusreduce(
            vtimes(w,
                vdrop(1, vcat( timesscan(a,getptr(ts)), 1, VT(CAT)),
                    VT(DROP)),
                VT(MUL)),
            getptr(pr));
}


int vencode(int a, int w, verb v){
    DEBUG(1,"------\nencode\n");
    IFDEBUG(1,print(a,0));
    IFDEBUG(1,print(w,0));
    if (gettag(w)==ARRAY){
        array W = getptr(w);
        if (W->rank==0 || productdims(W->rank, W->dims)==1){
            int scratch[W->rank];
            w = *elema(W,vector_index(0,W->dims,W->rank,scratch));
        }
    }
    array A;
    switch(gettag(a)){
    case NULLOBJ: return w;
    case LITERAL: A = scalar(a); // fallthrough
    case ARRAY: switch(gettag(w)){
                case LITERAL: {
                    array A = getptr(a);
                    DEBUG(1,"A->rank=%d\n",A->rank);
                    switch(A->rank){
                    case 1: {
                            int drop = vdrop(-1,a, VT(DROP));
                            DEBUG(1,"drop:");
                            IFDEBUG(1,print(drop, 0));
                            if (A->dims[0]
                                    && *elem(A, A->dims[0]-1)== 0)
                                return vcat( vencode(drop,
                                            0,VT(ENC)),
                                        w,VT(CAT));
                            int tail = vtake(-1, a, VT(TAKE));
                            IFDEBUG(1,print(tail, 0));
                            int mod = vresidue(tail, w, VT(MOD));
                            IFDEBUG(1,print(mod, 0));
                            int e = vencode( drop,
                                        vdivide( vminus(w,
                                                mod, VT(SUB)),
                                            tail, VT(DIV)),
                                        VT(ENC));
                            IFDEBUG(1,print(e, 0));
                            int z = vcat(e, mod, VT(CAT));
                            //z = vreshape(A->dims[0], z, VT(RHO));
                            z = vtake(-A->dims[0], z, VT(TAKE));
                            IFDEBUG(1,print(z, 0));
                            return z;
                    }
                    }
                }
                }
    }
    return null;
}

int vcompress(int a, int w, verb v){
    DEBUG(1,"compress\n");
    IFDEBUG(1,print(a,0));
    IFDEBUG(1,print(w,0));
recheck:
    switch (gettag(a)){
    case LITERAL: if (getval(a)) return w;
                  break;
recheckw:
    case ARRAY: switch (gettag(w)){
                case CHAR:
                case LITERAL:
                    return vcompress(a,
                            vreshape(vshapeof(a,VT(RHO)),
                                w,VT(RHO)),VT(COMP));
                case ARRAY: {
                    array A = getptr(a);
                    if (A->rank==0){ a=A->data[0]; goto recheck; }
                    array W = getptr(w);
                    if (W->rank==0){ w=W->data[0]; goto recheckw; }
                    if (A->rank == W->rank){
                        int eq = 1;
                        for (int i=0; i<A->rank; ++i)
                            if (A->dims[i]!=W->dims[i])
                                eq = 0;
                        if (eq && productdims(A->rank,A->dims)!=0) {
                            return vcat(
                                    vcompress(vectorindexleft(0,
                                            a, VT(INDL)),
                                        vectorindexleft(0, w, VT(INDR)),
                                        VT(COMP)),
                                    vcompress(vdrop(1, a, VT(DROP)),
                                        vdrop(1, w, VT(DROP)),
                                        VT(COMP)),
                                    VT(CAT));
                        }
                    }
                }
                }
    }
    return null;
}

int vexpand(int a, int w, verb v){
    DEBUG(1,"expand\n");
    IFDEBUG(1,print(a,0));
    IFDEBUG(1,print(w,0));
    int sum = areduce(vtab[VERB_PLUS],v);
    int (*sumf)(int,verb) = ((verb)getptr(sum))->monad;

recheck:
    switch (gettag(a)) {
    case LITERAL: switch (gettag(w)) {
                  case CHAR:
                  case LITERAL:
                      DEBUG(1,"LL\n");
                      if (getval(a))
                          return vravel(w,VT(CAT));
                      break;
                  case ARRAY: {
                      array W = getptr(w);
                      if (productdims(W->rank, W->dims) == 1)
                          if (getval(a))
                              return vravel(w,VT(CAT));
                          
                      break;
                  }
                  } break;
recheckw:
    case ARRAY: switch (gettag(w)){
                case CHAR:
                case LITERAL: {
                    DEBUG(1,"AL\n");
                    return vexpand(a,
                            vreshape(sumf(a,getptr(sum)),
                                w,VT(RHO)),VT(EXP));
                }
                case ARRAY: {
                    DEBUG(1,"AA\n");
                    array A = getptr(a);
                    if (A->rank==0) { a = A->data[0]; goto recheck; }
                    array W = getptr(w);
                    switch (W->rank){
                    case 0: w = W->data[0]; goto recheckw;
                    case 1: 
                        if (W->dims[0] == sumf(a,getptr(sum))) {
                            if (A->dims[0] == 0) return getfill(w);
                            int x = *elem(A, 0);
                            if (gettag(x)==LITERAL && getval(x)){
                                return vcat(vtake(1,w,VT(TAKE)),
                                        vexpand(vdrop(1,a,VT(DROP)),
                                            vdrop(1,w,VT(DROP)),
                                            VT(EXP)),
                                        VT(CAT));
                            } else {
                                return vcat(getfill(w),
                                        vexpand(vdrop(1,a,VT(DROP)),
                                            w, VT(EXP)), VT(CAT));
                            }
                        }
                    }
                }
                }
    }
    return null;
}


int vreverse(int w, verb v){
    //printf("reverse\n");
    //print(w, 0);
    int shapew = vshapeof(w, VT(RHO));
    //print(shapew, 0);
    int plus = vplus(-1,shapew, VT(PLUS));
    //print(plus, 0);
    int iot = viota(vreshape(nil, shapew, VT(RHO)), VT(IOTA));
    //print(iot, 0);
    int idx = vminus(plus, iot , VT(SUB));
    //print(idx, 0);
    int idxw = vectorindexleft(idx, w, VT(INDL));
    //print(idxw, 0);
    return idxw;
}


int vrotate(int a, int w, verb v){
    //printf("rotate\n");
    int shapew = vshapeof(w, VT(RHO));
    //print(shapew, 0);
    int iot = viota(shapew, VT(IOTA));
    //print(iot, 0);
    int plus = vplus(a, iot, VT(PLUS));
    //print(plus, 0);
    int idx = vresidue(shapew, plus, VT(MOD));
    //print(idx, 0);
    return vectorindexleft(idx, w, VT(INDL));
}


int vconceal(int w, verb v){
    switch(gettag(w)){
    case LITERAL:
    case CHAR: return w;
    case ARRAY: {
        array W = getptr(w);
        if (W->rank==0
                //|| (W->rank==1 && W->dims[0]==1)
                )
            return w;
    }
    }
    return cache(ARRAY, scalar(w));
}


int vreveal(int w, verb v){
    switch(gettag(w)){
    case ARRAY: {
        array W = getptr(w);
        if (W->rank==0
                //|| (W->rank==1 && W->dims[0]==1)
                )
            return *elem(W, 0);
    }
    }
    return w;
}


void init_vb(symtab st){
    verb v;
#define mnone 0
#define dnone 0
    VERBS_FOREACH(DEFINE_VERB_IN_ENV)
#undef mnone
#undef dnone
}

