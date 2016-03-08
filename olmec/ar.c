#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ppnarg.h"

#include "ar.h"


int productdims(int rank, const int *dims){
    int z = 1;
    for (int i=0; i < rank; i++) {
        z *= dims[i];
    }
    return z;
}

void calcweight(int rank, const int *dims, int *weight){
    for (int i = 0, wgt = 1; i < rank; ++i){
        weight[rank-1-i] = wgt;
        wgt *= dims[rank-1-i];
    }
}

// create new array object
array array_new_rank_pdims(int rank, const int *dims){
    const int datasz = productdims(rank, dims);
    array z = malloc(sizeof *z + (2*rank+datasz)*sizeof(int));
    z->type = normal;
    z->rank = rank;
    z->cons = 0;

    z->dims = (int*)(z+1);
    memmove(z->dims,dims,rank*sizeof(int));

    z->weight = z->dims + rank;
    calcweight(rank, dims, z->weight);

    z->data = z->weight + rank;

    return z;
}



void loaddimsv(int rank, int *dims, va_list ap){
    for (int i=0; i<rank; i++){
        dims[i]=va_arg(ap,int);
    }
}

// create array, taking dims from variable argument list
array (array_new_rank_dims)(int rank, ...){
    va_list ap;
    int dims[rank];

    va_start(ap,rank);
    loaddimsv(rank,dims,ap);
    va_end(ap);

    return array_new_rank_pdims(rank,dims);
}



// as a convention, a->data[0]==1
// indicating 1 (additional) data item (after data[0])
// data[1] is the actual data
int *constant(array a,int idx){
    return a->data+1;
}

int *j_vector(array a,int idx){
    a->data[1] = idx;
    return a->data+1;
}

// create special function-type array
array array_new_function(int rank, const int *dims,
        const int *data, int datan, int *(*func)(array,int)){
    array z = malloc(sizeof *z + (2*rank+datan)*sizeof(int));
    z->type = function;
    z->func = func;
    z->rank = rank;
    z->cons = 0;

    z->dims = (int*)(z+1);
    memmove(z->dims,dims,rank*sizeof(int));

    z->weight = z->dims + rank;
    calcweight(rank, dims, z->weight);

    z->data = z->weight + rank;
    memmove(z->data, data, datan+sizeof(int));

    return z;
}



// create array object accessing existing array data
array cast_rank_pdims(int *data, int rank, const int *dims){
    array z=malloc(sizeof*z + (2*rank)*sizeof(int));
    z->type = indirect;
    z->rank = rank;
    z->cons = 0;

    z->dims = (int*)(z+1);
    memmove(z->dims,dims,rank*sizeof(int));

    z->weight = z->dims + rank;
    calcweight(rank, dims, z->weight);

    z->data = data;

    return z;
}

// create array accessing existing data taking dims from varargs
array (cast_rank_dims)(int *data, int rank, ...){
    va_list ap;
    int dims[rank];

    va_start(ap,rank);
    loaddimsv(rank,dims,ap);
    va_end(ap);

    return cast_rank_pdims(data, rank, dims);
}



// create a duplicate descriptor sharing array data
array clone(array a){
    array z=malloc(sizeof *z + (2*a->rank)*sizeof(int));
    z->type = indirect;
    z->rank = a->rank;
    z->cons = 0;

    z->dims = (int*)(z+1);
    memmove(z->dims,a->dims,z->rank*sizeof(int));

    z->weight = z->dims + z->rank;
    memmove(z->weight,a->weight,z->rank*sizeof(int));

    z->data = a->data;

    return z;
}

// create a new array object with data copied from array a
array copy(array a){
    const int datasz = productdims(a->rank,a->dims);
    array z=malloc(sizeof *z + (2*a->rank+datasz)*sizeof(int));
    z->type = normal;
    z->rank = a->rank;
    z->cons = 0;

    z->dims = (int*)(z+1);
    memmove(z->dims,a->dims,z->rank*sizeof(int));

    z->weight = z->dims + z->rank;
    calcweight(z->rank, z->dims, z->weight);

    z->data = z->weight + z->rank;
    int scratch[a->rank];
    for (int i=0; i < datasz; ++i){
        z->data[i] = *elema(a, vector_index(i,z->dims,z->rank,scratch));
    }

    return z;
}

int issolid(array a){
    int i,x;
    for (i=a->rank-1,x=1; i>=0; i--){
        if (a->weight[i] != x)
            return 0;
        x *= a->dims[i];
    }
    return 1;
}

array makesolid(array a){
    if (a->type==function || !issolid(a))
        return copy(a);
    return a;
}


// convert a ravel index to an index vector
int *vector_index(int ind, const int *dims, int n, int *vec){
    int i,t=ind, *z=vec;
    for (i=0; i<n; i++){
        z[n-1-i] = t % dims[n-1-i];
        t /= dims[n-1-i];
    }
    return z;
}

// convert index vector to ravel index
int ravel_index(const int *vec, const int *dims, int n){
    int i,z=*vec;
    for (i=0; i<n-1; i++){
        z *= dims[i];
        z += vec[i+1];
    }
    return z;
}


// nb. cannot run on the ravel with non-solid indirect array
int *elemr(array a, int idx){
    if (a->type==function) return a->func(a,idx);
    else return a->data+idx;
}

int *elema(array a, const int *ind){
    int idx = 0;
    for (int i=0; i<a->rank; i++){
        idx += ind[i] * a->weight[i];
    }
    return elemr(a,idx + a->cons);
}

int *elemv(array a, va_list ap){
    int idx = 0;
    for (int i=0; i<a->rank; i++){
        idx += va_arg(ap, int) * a->weight[i];
    }
    return elemr(a,idx + a->cons);
}

int *elem(array a, ...){
    va_list ap;
    int *z;

    va_start(ap,a);
    z = elemv(a,ap);
    va_end(ap);

    return z;
}


void swap(int *x, int *y){
    int t=*x;
          *x=*y;
             *y=t;
}

// elem(a,i,j) -> elem(a,j,i)
void transpose2(array a){
    swap(&a->dims[0],&a->dims[1]);
    swap(&a->weight[0],&a->weight[1]);
}

// rotate indices by shift amount
void transpose(array a, int shift){
    int i;
    int t;
    while(shift){
        if (shift>0){
            t=a->dims[0];
            for(i=1; i<a->rank; i++)
                a->dims[i-1]=a->dims[i];
            a->dims[a->rank-1]=t;
            t=a->weight[0];
            for(i=1; i<a->rank; i++)
                a->weight[i-1]=a->weight[i];
            a->weight[a->rank-1]=t;
            --shift;
        } else {
            t=a->dims[a->rank-1];
            for (i=a->rank-2; i>=0; i--)
                a->dims[i+1]=a->dims[i];
            a->dims[0]=t;
            t=a->weight[a->rank-1];
            for (i=a->rank-2; i>=0; i--)
                a->weight[i+1]=a->weight[i];
            a->weight[0]=t;
            ++shift;
        }
    }
}

// select new order of indexing with array of dimension indices
void transposea(array a, const int *spec){
    int dims[a->rank];
    int weight[a->rank];
    for (int i=0; i<a->rank; i++){
        dims[i] = a->dims[spec[i]];
        weight[i] = a->weight[spec[i]];
    }
    memcpy(a->dims, dims, a->rank*sizeof(int));
    memcpy(a->weight, weight, a->rank*sizeof(int));
}



// return new indirect array of one item of array
array slice(array a, int i){
    const int rank = a->rank-1;
    array z=malloc(sizeof *z + (2*rank)*sizeof(int));
    z->rank = rank;

    z->dims = (int *)(z+1);
    memcpy(z->dims, a->dims+1, z->rank*sizeof(int));

    z->weight = z->dims + z->rank;
    memcpy(z->weight, a->weight+1, z->rank*sizeof(int));
    z->cons = a->cons + i*a->weight[0];

    z->data = a->data;
    return z;
}

// return new indirect array selecting a single item (if 0<=spec[i]<dims[i])
// or all items (if spec[i]==-1) from each dimension
array slicea(array a, const int *spec){
    int rank = 0;
    for (int i=0; i<a->rank; ++i)
        rank += spec[i]==-1;

    int dims[rank];
    int weight[rank];
    for (int i=0,j=0; i<rank; ++i,++j){
        while (spec[j]!=-1) j++;
        if (j>=a->rank) break;
        dims[i] = a->dims[j];
        weight[i] = a->weight[j];
    }

    array z = cast_rank_pdims(a->data, rank, dims);
    memcpy(z->weight,weight,rank*sizeof(int));

    z->cons = a->cons;
    for (int j=0; j<a->rank; j++){
        if (spec[j]!=-1)
            z->cons += spec[j] * a->weight[j];
    }

    return z;
}

// select a contiguous range from s[i] to f[i] of each dimension
array slices(array a, const int *s, const int *f){
    int rank = 0;
    for (int i=0; i<a->rank; i++){
        rank += s[i] != f[i];
    }

    int dims[rank];
    int weight[rank];
    for (int i=0,j=0; i<rank; i++){
        while (s[j]==f[j]) ++j;
        dims[i] = 1 + (s[j]<f[j] ? f[j]-s[j] : s[j]-f[j] );
        weight[i] =    s[j]<f[j] ? a->weight[j] : -a->weight[j];
        ++j;
    }

    array z = cast_rank_pdims(a->data, rank, dims);
    memcpy(z->weight, weight, rank*sizeof(int));

    z->cons = a->cons;
    for (int i=0; i<a->rank; i++){
        z->cons += s[i] * a->weight[i];
    }

    return z;
}


// prepend extra unit axes
// extend(vector(...),1) -> 1xN row vector
array extend(array a, int extra){
    int rank = a->rank + extra;
    int dims[rank];
    for (int i=0; i<extra; i++)
        dims[i] = 1;
    memcpy(dims+extra, a->dims, a->rank*sizeof(int));
    return cast_rank_pdims(a->data, rank, dims);
}



// generate a 1 element vector, ie. a scalar array object
array scalar(int n){
    array z = array_new_dims(1);
    *elem(z,0) = n;
    return z;
}

// create a vector array object initialized with variable arguments
array (vector_n)(int n, ...){
    va_list ap;
    array z = array_new_dims(n);
    int i;

    va_start(ap,n);
    for (i=0; i<n; i++)
        *elem(z,i) = va_arg(ap, int);
    va_end(ap);
    return z;
}



// yield ravelled concatenation of two arrays 
array cat(array x, array y){
    int xsz = productdims(x->rank,x->dims);
    int ysz = productdims(y->rank,y->dims);
    int datasz = xsz + ysz;

    array z=array_new_dims(datasz);

    int scratch[x->rank+y->rank];  // cheap max
    for (int i=0; i<xsz; i++)
        *elem(z,i) = *elema(x,vector_index(i,x->dims,x->rank,scratch));
    for (int i=0; i<ysz; i++)
        *elem(z,xsz+i) = *elema(y,vector_index(i,y->dims,y->rank,scratch));

    return z;
}

// generate a j-vector
// which yields iota values as a function of argument indices
array iota(int n){
#if 0
    array z = array_new_dims(n);
    int i;
    for (i=0; i<n; i++)
        *elem(z,i) = i;
    return z;
#endif
    return array_new_function(1,&n,(int[]){1,0},2,j_vector);
}



#ifdef TESTMODULE
#include <stdlib.h>
#include <string.h>
#include "minunit.h"
int tests_run = 0;

static char *test_basic(){
    array a = array_new_rank_pdims(1, (int[]){4});
    *elem(a,3) = 12;
    test_case(*elem(a,3)!=12);

    array b = array_new_dims(4,5);
    *elem(b,3,4) = 5;
    test_case(*elem(b,3,4)!=5);

    array c = iota(4);
    test_case(*elem(c,3)!=3);

    //array d = iota(64);
    //array e = cast_dims(d->data, 2,2,2,2,2,2); // no longer works with j_vector iota
    //test_case(*elem(e, 1,1,1,1,1,1) != 63);

    //array f = cast_dims(d->data, 4,4,4); // no longer works with j_vector iota
    //test_case(*elem(f, 3,3,3) != 63);

    return 0;
}

static char *all_tests(){
    mu_run_test(test_basic);
    return 0;
}

int main(){

    char *result=all_tests();
    if (result != 0) {
        printf("%s\n",result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result != 0;

}

#endif //defined TESTMODULE
