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

#include "array.h"
#include "encoding.h"
#include "symtab.h"

#include "verbs.h"
#include "adverbs.h"
#include "print.h"
#include "exec.h"
#include "number.h"
#include "verb_private.h"

object vtab[VERB_NOOP];

void common(object *ap, object *wp){
    //promote smaller number to matching type
}


object scalarop_lit_lit(object a, dyad func, object w, char op, verb v){
    DEBUG(1,"wlit ");
    object z = null;
    int64_t t;
    switch(op){
    case '+':
        t = (int64_t)getval(a) + getval(w);
        if (t>((1U<<24)-1) || t<=~(int64_t)((1U<<24)-1))
            z = cache(NUMBER, number_add(new_number_lit(getval(a)), new_number_lit(getval(w))));
        else
            z = newdata(LITERAL, t);
        break;
    case '-':
        t = (int64_t)getval(a) - getval(w);
        if (t>((1U<<24)-1) || t<=~(int64_t)((1U<<24)-1))
            z = cache(NUMBER, number_sub(new_number_lit(getval(a)), new_number_lit(getval(w))));
        else
            z = newdata(LITERAL, t);
        break;
    case '*':
        t = (int64_t)getval(a) * getval(w);
        if (t>((1U<<24)-1) || t<=~(int64_t)((1U<<24)-1))
            z = cache(NUMBER, number_mul(new_number_lit(getval(a)), new_number_lit(getval(w))));
        else
            z = newdata(LITERAL, t);
        break;
    case '/': //z = newdata(LITERAL, getval(a) / getval(w)); break;
        z = cache(NUMBER, number_div(new_number_lit(getval(a)), new_number_lit(w)));
        break;
    case '%': z = newdata(LITERAL, getval(w) % getval(a));
              break;
    default: printf("bad op\n");
             break;
    }
    DEBUG(1,"z=%08x(%d,%d)\n", z, gettag(z), getval(z));
    return z;
}

object scalarop_num_num(object a, dyad func, object w, char op, verb v){
    switch(op){
    case '+': return cache(NUMBER, number_add(getptr(a), getptr(w)));
    case '-': return cache(NUMBER, number_sub(getptr(a), getptr(w)));
    case '*': return cache(NUMBER, number_mul(getptr(a), getptr(w)));
    case '/': return cache(NUMBER, number_div(getptr(a), getptr(w)));
    case '%': return cache(NUMBER, number_mod(getptr(a), getptr(w)));
    default: printf("bad op\n"); break;
    }
    return null;
}

object scalarop_lit_arr(object a, dyad func, object w, char op, verb v){
    DEBUG(1,"warr ");
    array W = getptr(w);
    if (W->rank == 0
	|| (W->rank == 1 && W->dims[0] == 1)) {
      w = *elem(W, 0);
      return scalarop(a, func, w, op, v);
    }
    array Z = array_new_rank_pdims(W->rank, W->dims);
    int n = productdims(W->rank, W->dims);
    int scratch[W->rank];
    for (int i=0; i<n; ++i){
      vector_index(i, W->dims, W->rank, scratch);
      int z = func(a, *elema(W, scratch), v);
      DEBUG(1,"z[%d]=%08x(%d,%d)\n",
	    i, z, gettag(z), getval(z));
      *elema(Z, scratch) = z;
    }
    int z = cache(ARRAY, Z);
    DEBUG(1,"result=%08x(%d,%d)\n",
	  z, gettag(z), getval(z));
    IFDEBUG(1,print(z, 0, 1));
    return z;
}

object scalarop_arr_lit(array A, dyad func, object w, char op, verb v){
    DEBUG(1,"wlit ");
    array Z = array_new_rank_pdims(A->rank, A->dims);
    int n = productdims(A->rank, A->dims);
    int scratch[A->rank];
    for (int i=0; i<n; ++i){
      vector_index(i, A->dims, A->rank, scratch);
      *elema(Z, scratch) = func(*elema(A, scratch), w, v);
    }
    return cache(ARRAY, Z);
}

object scalarop_arr_arr(object a, array A, dyad func, object w, char op, verb v){
    DEBUG(1,"warr ");
    array W = getptr(w);
    if (W->rank == 0
	|| (W->rank == 1 && W->dims[0] == 1)){
      w = *elem(W, 0);
      return scalarop(a, func, w, op, v);
    }
    array Z = array_new_rank_pdims(W->rank, W->dims);
    int n = productdims(W->rank, W->dims);
    int scratch[W->rank];
    for (int i=0; i<n; ++i){
      vector_index(i, W->dims, W->rank, scratch);
      *elema(Z, scratch) =
	func(*elema(A, scratch), *elema(W, scratch), v);
    }
    return cache(ARRAY, Z);
}

object scalarop(object a, dyad func, object w, char op, verb v){
    DEBUG(1,"scalarop ");
recheck:
    switch(gettag(a)){
    case NUMBER:
        switch(gettag(w)){
        case NUMBER: return scalarop_num_num(a, func, w, op, v);
        case LITERAL:
        return scalarop_num_num(a, func, cache(NUMBER, new_number_lit(w)), op, v);
        case ARRAY: return scalarop_lit_arr(a, func, w, op, v);
        }
    case LITERAL:
        DEBUG(1,"alit ");
        switch(gettag(w)){
        case NUMBER:
        return scalarop_num_num(cache(NUMBER, new_number_lit(a)), func, w, op, v);
        case LITERAL: return scalarop_lit_lit(a, func, w, op, v);
        case ARRAY: return scalarop_lit_arr(a, func, w, op, v);
        }
        break;
    case ARRAY: {
        DEBUG(1,"aarr ");
        array A = getptr(a);
        if (A->rank == 0
                || (A->rank == 1 && A->dims[0] == 1)) {
            a = *elem(A, 0);
            goto recheck;
        }
        switch(gettag(w)){
        case NUMBER:
        case LITERAL: return scalarop_arr_lit(A, func, w, op, v);
        case ARRAY: return scalarop_arr_arr(a, A, func, w, op, v);
        }
    }
    }
    return null;
}

object scalaropfunc_lit_lit(object a, dyad func, object w, object opfunc(int,int), verb v){
    return opfunc(getval(a), getval(w));
}

object scalaropfunc_num_num(number_ptr a, dyad func, number_ptr w,
        object opfunc(int,int), object numfunc(number_ptr,number_ptr), verb v){
    return numfunc(a, w);
}

object scalaropfunc_lit_arr(object a, dyad func, object w, object opfunc(int,int), verb v){
    array W = getptr(w);
    array Z = array_new_rank_pdims(W->rank, W->dims);
    int n = productdims(W->rank, W->dims);
    int scratch[W->rank];
    for (int i=0; i<n; ++i){
        vector_index(i, W->dims, W->rank, scratch);
        *elema(Z, scratch) = func(a, *elema(W, scratch), v);
    }
    return cache(ARRAY, Z);
}

object scalaropfunc_arr_lit(array A, dyad func, object w, object opfunc(int,int), verb v){
    array Z = array_new_rank_pdims(A->rank, A->dims);
    int n = productdims(A->rank, A->dims);
    int scratch[A->rank];
    for (int i=0; i<n; ++i){
        vector_index(i, A->dims, A->rank, scratch);
        *elema(Z, scratch) = func(*elema(A, scratch), w, v);
    }
    return cache(ARRAY, Z);
}

object scalaropfunc_arr_arr(object a, array A, dyad func, object w, object opfunc(int,int), verb v){
    array W = getptr(w);
    array Z = array_new_rank_pdims(W->rank, W->dims);
    int n = productdims(W->rank, W->dims);
    int scratch[W->rank];
    for (int i=0; i<n; ++i){
      vector_index(i, W->dims, W->rank, scratch);
      *elema(Z, scratch) = func(*elema(A, scratch), *elema(W, scratch), v);
    }
    return cache(ARRAY, Z);
}

object scalaropfunc(object a, dyad func, object w,
        object opfunc(int,int), object numfunc(number_ptr,number_ptr), verb v){
recheck:
    switch(gettag(a)){
    case NUMBER: switch(gettag(w)){
        case NUMBER:
            return scalaropfunc_num_num(getptr(a),func,getptr(w), opfunc,numfunc,v);
        case LITERAL:
            return scalaropfunc_num_num(getptr(a),func,new_number_lit(w), opfunc,numfunc,v);
        case ARRAY:
            return scalaropfunc_lit_arr(a,func,w, opfunc, v);
        } break;
    case LITERAL: switch(gettag(w)){
        case NUMBER:
            return scalaropfunc_num_num(new_number_lit(a),func,getptr(w), opfunc,numfunc,v);
        case LITERAL:
            return scalaropfunc_lit_lit(a, func, w, opfunc, v);
        case ARRAY:
            return scalaropfunc_lit_arr(a, func, w, opfunc, v);
        }
        break;
    case ARRAY: {
        array A = getptr(a);
        if (A->rank == 0
                || (A->rank == 1 && A->dims[0] == 1)){
            a = *elem(A, 0);
            goto recheck;
        }
        switch(gettag(w)){
        case NUMBER:
            return scalaropfunc_arr_lit(A, func, w, opfunc, v);
        case LITERAL:
            return scalaropfunc_arr_lit(A, func, w, opfunc, v);
        case ARRAY:
            return scalaropfunc_arr_arr(a, A, func, w, opfunc, v);
        }
    }
    }
    return null;
}

object scalarmonad(monad func, object w, char op, verb v){
    switch(gettag(w)){
    case LITERAL: switch(op){
        case '-': return newdata(LITERAL, - getval(w));
        }
    case NUMBER: switch(op){
        case '-': return cache(NUMBER, number_neg(getptr(w)));
        }
    case ARRAY: {
        array W = getptr(w);
        array Z = array_new_rank_pdims(W->rank, W->dims);
        int n = productdims(W->rank, W->dims);
        int scratch[W->rank];
        for (int i=0; i<n; ++i){
            vector_index(i, W->dims, W->rank, scratch);
            *elema(Z, scratch) = func(*elema(W, scratch), v);
        }
        return cache(ARRAY, Z);
    }
    }
    return null;
}

object scalarmonadfunc(monad func, object w,
        int opfunc(int), number_ptr numfunc(number_ptr), verb v){
    switch(gettag(w)){
    case LITERAL: return newdata(LITERAL, opfunc(getval(w)));
    case NUMBER: return cache(NUMBER, numfunc(getptr(w)));
    case ARRAY: {
        array W = getptr(w);
        array Z = array_new_rank_pdims(W->rank, W->dims);
        int n = productdims(W->rank, W->dims);
        int scratch[W->rank];
        for (int i=0; i<n; ++i){
            vector_index(i, W->dims, W->rank, scratch);
            *elema(Z, scratch) = func(*elema(W, scratch), v);
        }
        return cache(ARRAY, Z);
    }
    }
}

object vid (object w, verb v){
    if (gettag(w)==ARRAY) return cache(ARRAY, makesolid(getptr(w)));
    return w;
}

object vplus (object a, object w, verb v){
    common(&a,&w);
    DEBUG(1,"plus %08x(%d,%d),%08x(%d,%d)\n",
            a, gettag(a), getval(a),
            w, gettag(w), getval(w));

    if (gettag(a)==LITERAL && gettag(w)==ARRAY){
        array W = getptr(w);
        if (W->type == function){
            if (W->func == constant){ W->data[1] += a; }
            if (W->func == j_vector){ W->cons += a; }
            return w;
        }
    }

    return SCALAROP(a,vplus,w,+,v);
}


object vneg(object w, verb v){
    if (gettag(w)==ARRAY){
        array W = getptr(w);
        if (W->type == function){
            if (W->func == constant){ W->data[1] = -W->data[1]; }
            if (W->func == j_vector){ W->weight[W->rank-1] *= -1; }
            return w;
        }
    }

    return SCALARMONAD(vneg,w,-,v);
}

object vminus(object a, object w, verb v){
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

    return SCALAROP(a,vminus,w,-,v);
}


object vdivide(object a, object w, verb v){
    common(&a, &w);

    return SCALAROP(a,vdivide,w,/,v);
}

object vrecip(object w, verb v){
    return vdivide(1, w, VT(DIV));
}


object vsignum (object w, verb v){
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

object vtimes (object a, object w, verb v){
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


    return SCALAROP(a,vtimes,w,*,v);
}

object vabs (object w, verb v){
    return scalarmonadfunc(vabs, w, (abs), number_abs, v);
}

static inline void swap(object *x, object *y){
    object t = *x;
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

object oresid(int a, int w){
    return newdata(LITERAL, resid(a,w));
}

object onumber_mod(number_ptr a, number_ptr w){
    return cache(NUMBER, number_mod(a, w));
}

object vresidue (object a, object w, verb v){
    DEBUG(1,"residue\n");
    IFDEBUG(1,print(a, 0, 1));
    IFDEBUG(1,print(w, 0, 1));
    common(&a, &w);
    swap(&a, &w);

    return scalaropfunc(a, vresidue, w, oresid, onumber_mod, v);
}


int ipow(int x, int y){
    int z = 1;
    for (int i=0; i<y; ++i)
        z *= x;
    return z;
}

object oipow(int x, int y){
    object z = newdata(LITERAL, 1);
    for (int i=0; i<y; ++i){
        z = vtimes(z, newdata(LITERAL, x), VT(MUL));
        IFDEBUG(1, print(z,0,1));
    }
    return z;
}

object onumber_pow(number_ptr a, number_ptr w){
    return cache(NUMBER, number_pow(a,w));
}

object vpow (object a, object w, verb v){
    common(&a, &w);
    return scalaropfunc(a, vpow, w, oipow, onumber_pow, v);
}


object vshapeof (object w, verb v){
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

void mcopy(object *dest, object *src, int n){
    int i;
    for (i=0; i<n; i++)
        dest[i] = src[i];
}

object vreshape (object a, object w, verb v){
    DEBUG(1,"reshape\n");
    IFDEBUG(1,print(a, 0, 1));
    IFDEBUG(1,print(w, 0, 1));
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
            //printarray(z, 0);
            IFDEBUG(1, printindexdisplay(z));
            return cache(ARRAY, z);
        }
        }//switch
    }//switch
    return w;
}

object vtally (object w, verb v){
    switch(gettag(w)){
        case ARRAY: {
            array a = getptr(w);
            return newdata(LITERAL, a->rank?a->dims[0]:1);
        }
    }
    return newdata(LITERAL, 1);
}


object viota (object w, verb v){
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


object vravel (object w, verb v){
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

object vcat (object a, object w, verb v){
    DEBUG(1,"cat\n");
    IFDEBUG(1, print(a,0, 1));
    IFDEBUG(1, print(w,0, 1));
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


object vbox (object w, verb v){
    return cache(ARRAY, scalar(w));
}

object vlessthan (object a, object w, verb v){
}


object vunbox (object w, verb v){
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

object vgreaterthan (object a, object w, verb v){
}


int ieq(int x, int y){
    return x==y;
}

object oieq(int x, int y){
    return newdata(LITERAL, ieq(x,y));
}

object onumber_eq(number_ptr a, number_ptr w){
    return cache(NUMBER, number_eq(a,w));
}

object veq (object a, object w, verb v){
    object z = scalaropfunc(a, veq, w, oieq, onumber_eq, v);
    if (gettag(z)==NUMBER)
        z = newdata(LITERAL, number_get_int(getptr(z)));
    return z;
}


int ine(int x, int y){
    return x!=y;
}

object oine(int x, int y){
    return newdata(LITERAL, ine(x,y));
}

object onumber_ne(number_ptr a, number_ptr w){
    return cache(NUMBER, number_ne(a,w));
}

object vne (object a, object w, verb v){
    object z = scalaropfunc(a, vne, w, oine, onumber_ne, v);
    if (gettag(z)==NUMBER)
        z = newdata(LITERAL, number_get_int(getptr(z)));
    return z;
}


object vlink (object a, object w, verb v){
    switch(gettag(w)){
        case ARRAY: return vcat(vbox(a,v),w,v);
    }
    return vcat(vbox(a,v),vbox(w,v),v);
}

object vprenul (object w, verb v){
    return vlink(null,w,v);
}


object vhead (object w, verb v){
    switch(gettag(w)){
        case ARRAY: {
            array W = getptr(w);
            if (W->rank == 0) return getfill(w);
            return  cache(ARRAY, slice(W, 0));
        }
    }
    return null;
}

object vtake (object a, object w, verb v){
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


object vbehead (object w, verb v){
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

object vdrop (object a, object w, verb v){
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


object vectorindexleft(object a, object w, verb v){
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

object unitindexleft(object a, object w, verb v){
    DEBUG(1, "unitindexleft\n");
    IFDEBUG(1, print(a, 0, 1));
    IFDEBUG(1, print(w, 0, 1));
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


object vindexleft(object a, object w, verb v){
    return unitindexleft(a, w, v);
    //return vectorindexleft(a, w, v);
}

object vindexright(object a, object w, verb v){
    return vindexleft(w,a,v);
}


object vbase(object a, object w, verb v){
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
    object pr = areduce(vtab[VERB_PLUS],v);
    int (*plusreduce)(int,verb) = ((verb)getptr(pr))->monad;

    object ts = abackscan(vtab[VERB_MUL],v);
    int (*timesscan)(int,verb) = ((verb)getptr(ts))->monad;
    return plusreduce(
            vtimes(w,
                vdrop(1, vcat( timesscan(a,getptr(ts)), 1, VT(CAT)),
                    VT(DROP)),
                VT(MUL)),
            getptr(pr));
}


object vencode(object a, object w, verb v){
    DEBUG(1,"------\nencode\n");
    IFDEBUG(1,print(a,0, 1));
    IFDEBUG(1,print(w,0, 1));
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
                            object drop = vdrop(-1,a, VT(DROP));
                            DEBUG(1,"drop:");
                            IFDEBUG(1,print(drop, 0, 1));
                            if (A->dims[0]
                                    && *elem(A, A->dims[0]-1)== 0)
                                return vcat( vencode(drop,
                                            0,VT(ENC)),
                                        w,VT(CAT));
                            object tail = vtake(-1, a, VT(TAKE));
                            IFDEBUG(1,print(tail, 0, 1));
                            object mod = vresidue(tail, w, VT(MOD));
                            IFDEBUG(1,print(mod, 0, 1));
                            object e = vencode( drop,
                                        vdivide( vminus(w,
                                                mod, VT(SUB)),
                                            tail, VT(DIV)),
                                        VT(ENC));
                            IFDEBUG(1,print(e, 0, 1));
                            object z = vcat(e, mod, VT(CAT));
                            //z = vreshape(A->dims[0], z, VT(RHO));
                            z = vtake(-A->dims[0], z, VT(TAKE));
                            IFDEBUG(1,print(z, 0, 1));
                            return z;
                    }
                    }
                }
                }
    }
    return null;
}

object vcompress(object a, object w, verb v){
    DEBUG(1,"compress\n");
    IFDEBUG(1,print(a,0, 1));
    IFDEBUG(1,print(w,0, 1));
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

object vexpand(object a, object w, verb v){
    DEBUG(1,"expand\n");
    IFDEBUG(1,print(a,0, 1));
    IFDEBUG(1,print(w,0, 1));
    object sum = areduce(vtab[VERB_PLUS],v);
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


object vreverse(object w, verb v){
    //printf("reverse\n");
    //print(w, 0, 1);
    object shapew = vshapeof(w, VT(RHO));
    //print(shapew, 0, 1);
    object plus = vplus(-1,shapew, VT(PLUS));
    //print(plus, 0, 1);
    object iot = viota(vreshape(nil, shapew, VT(RHO)), VT(IOTA));
    //print(iot, 0, 1);
    object idx = vminus(plus, iot , VT(SUB));
    //print(idx, 0, 1);
    object idxw = vectorindexleft(idx, w, VT(INDL));
    //print(idxw, 0, 1);
    return idxw;
}


object vrotate(object a, object w, verb v){
    //printf("rotate\n");
    object shapew = vshapeof(w, VT(RHO));
    //print(shapew, 0, 1);
    object iot = viota(shapew, VT(IOTA));
    //print(iot, 0, 1);
    object plus = vplus(a, iot, VT(PLUS));
    //print(plus, 0, 1);
    object idx = vresidue(shapew, plus, VT(MOD));
    //print(idx, 0, 1);
    return vectorindexleft(idx, w, VT(INDL));
}


object vconceal(object w, verb v){
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


object vreveal(object w, verb v){
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

object vnil(verb v){
    printf("niladic verb\n");
    return 42;
}

object vnoresult(object w, verb v){
    printf("!no result!\n");
    return mark;
}

object vnoresultd(object a, object w, verb v){
    return vnoresult(w,v);
}


/*  if called within a block structure,
 *  cause transfer of control to specified line
 */
object vbranch(object w, verb v){
    switch(gettag(w)){
    case LITERAL: return newdata(LABEL, getval(w));
    case ARRAY: {
        array W = getptr(w);
        return newdata(LABEL, 
                W->dims[0]? getval(*elem(W,0)): 0);
    }
    default: return newdata(LABEL, 0);
    }
}

/* Handler functions for del func defs
 */

void def_extra(analysis a, symtab child){
    if (a->extra){
        array x = getptr(a->extravars);
        for (int i=0; i<x->dims[0]; ++i)
            def(child, *elem(x,i), mark,0);
    }
}

object call_execute(object body, symtab child, analysis a){
    int last_was_assn;
    DEBUG(1, "call_execute\n");
    object ret = execute(body, child, &last_was_assn);
    if (a->result){
        object result = find(child, a->resultvar);
        IFDEBUG(1, print(a->resultvar, 0, 1);
                   print(result, 0, 1); );
        return result;
    } else {
        return ret;
    }
}

object ndel(verb v){
    DEBUG(1, "ndel\n");
    object body = v->f;
    symtab env = getptr(v->g);
    analysis a = getptr(v->h);
    symtab child = makesymtabchain(env, 10);
    def_extra(a, child);
    return call_execute(body, child, a);
}

object mdel(object w, verb v){
    DEBUG(1, "mdel\n");
    object body = v->f;
    symtab env = getptr(v->g);
    analysis a = getptr(v->h);
    symtab child = makesymtabchain(env, 10);
    def_extra(a, child);
    def(child, a->omega, w,0);
    return call_execute(body, child, a);
}

object ddel(object a, object w, verb v){
    DEBUG(1, "ddel\n");
    object body = v->f;
    symtab env = getptr(v->g);
    analysis an = getptr(v->h);
    symtab child = makesymtabchain(env, 10);
    def_extra(an, child);
    def(child, an->alpha, a,0);
    def(child, an->omega, w,0);
    return call_execute(body, child, an);
}


/* Handler functions for direct defs
 */

object ndfn(verb v){
    IFDEBUG(4, print(v->f, 0, 1););
    object expr = v->f;
    symtab env = getptr(v->g);
    int last_was_assn;
    return execute(expr, env, &last_was_assn);
}

object mdfn(object w, verb v){
    object expr = v->f;
    symtab env = getptr(v->g);
    symtab child = makesymtabchain(env, 10);
    def(child, newdata(PCHAR, 0x2375), w,0);
    int last_was_assn;
    return execute(expr, child, &last_was_assn);
}

object ddfn(object a, object w, verb v){
    object expr = v->f;
    symtab env = getptr(v->g);
    symtab child = makesymtabchain(env, 10);
    def(child, newdata(PCHAR, 0x237a), a,0);
    def(child, newdata(PCHAR, 0x2375), w,0);
    int last_was_assn;
    return execute(expr, child, &last_was_assn);
}


void init_vb(symtab st){
    verb v;
#define nnone 0
#define mnone 0
#define dnone 0
    VERBS_FOREACH(st,DEFINE_VERB_IN_ENV)
#undef nnone
#undef mnone
#undef dnone
}

