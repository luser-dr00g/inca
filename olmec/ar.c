#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ppnarg.h"

#include "ar.h"

int productdims(int rank, int dims[]){
    int i,z=1;
    for (i=0; i<rank; i++)
        z *= dims[i];
    return z;
}

array array_new_dims(int rank, int dims[]){
    int datasz;
    int i;
    int x;
    array z;
    datasz=productdims(rank,dims);
    z=malloc(sizeof*z
            + (rank+rank+datasz)*sizeof(int));

    z->type = normal;
    z->rank = rank;
    z->dims = (int*)(z+1);
    z->cons = 0;
    z->weight = z->dims + rank;
    z->data = z->weight + rank;
    memmove(z->dims,dims,rank*sizeof(int));
    for(x=1, i=rank-1; i>=0; i--){
        z->weight[i] = x;
        x *= z->dims[i];
    }

    return z;
}

int *constant(array a,int idx){
    return a->data;
}

int *ret_index(array a,int idx){
    *a->data = idx;
    return a->data;
}

array array_new_function(int rank, int dims[],
        int *data, int datan, int *(*func)(array,int)){
    int i,x;
    array z;
    z=malloc(sizeof*z
            + (rank+rank+datan)*sizeof(int));

    z->type = function;
    z->rank = rank;
    z->dims = (int*)(z+1);
    z->cons = 0;
    z->weight = z->dims + rank;
    z->data = z->weight + rank;
    memmove(z->data, data, datan+sizeof(int));
    z->func = func;
    memmove(z->dims,dims,rank*sizeof(int));
    for(x=1, i=rank-1; i>=0; i--){
        z->weight[i] = x;
        x *= z->dims[i];
    }

    return z;
}

void loaddimsv(int rank, int dims[], va_list ap){
    int i;
    for (i=0; i<rank; i++){
        dims[i]=va_arg(ap,int);
    }
}

array (array_new)(int rank, ...){
    va_list ap;
    int dims[rank];
    int i;
    int x;

    va_start(ap,rank);
    loaddimsv(rank,dims,ap);
    va_end(ap);

    return array_new_dims(rank,dims);
}

array cast_dims(int data[], int rank, int dims[]){
    int i,x;
    array z=malloc(sizeof*z
            + (rank+rank)*sizeof(int));

    z->type = indirect;
    z->rank = rank;
    z->dims = (int*)(z+1);
    z->cons = 0;
    z->weight = z->dims + rank;
    z->data = data;
    memmove(z->dims,dims,rank*sizeof(int));
    for(x=1, i=rank-1; i>=0; i--){
        z->weight[i] = x;
        x *= z->dims[i];
    }

    return z;
}

array (cast)(int data[], int rank, ...){
    va_list ap;
    int dims[rank];
    int i;
    int x;

    va_start(ap,rank);
    loaddimsv(rank,dims,ap);
    va_end(ap);

    return cast_dims(data, rank, dims);
}

array clone(array a){
    array z=malloc(sizeof*z
            + (a->rank+a->rank)*sizeof(int));

    z->type = indirect;
    z->rank = a->rank;
    z->dims = (int*)(z+1);
    z->cons = 0;
    z->weight = z->dims + z->rank;
    z->data = a->data;
    memmove(z->dims,a->dims,z->rank*sizeof(int));
    memmove(z->weight,a->weight,z->rank*sizeof(int));
    return z;
}

int *vector_index(int ind, int dims[], int n, int vec[]){
    int i,t=ind, *z=vec;
    for (i=0; i<n; i++){
        z[n-1-i] = t % dims[n-1-i];
        t /= dims[n-1-i];
    }
    return z;
}

int ravel_index(int vec[], int dims[], int n){
    int i,z=*vec;
    for (i=0; i<n-1; i++){
        z *= dims[i];
        z += vec[i+1];
    }
    return z;
}

array copy(array a){
    int datasz = productdims(a->rank,a->dims);
    array z=malloc(sizeof*z
            + (a->rank+a->rank+datasz)*sizeof(int));
    int i;
    int x;
    int ind[a->rank];

    z->type = normal;
    z->rank = a->rank;
    z->dims = (int*)(z+1);
    z->cons = 0;
    z->weight = z->dims + z->rank;
    z->data = z->weight + z->rank;
    memmove(z->dims,a->dims,z->rank*sizeof(int));
    for (x=1, i=z->rank-1; i>=0; i--){
        z->weight[i] = x;
        x *= z->dims[i];
    }

    for (i=0; i<datasz; i++){
        vector_index(i,z->dims,z->rank,ind);
        z->data[i] = *elema(a,ind);
    }

    return z;
}


int *elema(array a, int ind[]){
    int idx = 0;
    int i;
    for (i=0; i<a->rank; i++){
        idx += ind[i] * a->weight[i];
    }
    idx += a->cons;
    return a->type==function? a->func(a,idx): a->data+idx;
}

int *elemv(array a, va_list ap){
    int idx = 0;
    int i;
    for (i=0; i<a->rank; i++){
        int ind;
        ind = va_arg(ap, int);
        idx += ind * a->weight[i];
    }
    idx += a->cons;
    return a->type==function? a->func(a,idx): a->data+idx;
}

int *elem(array a, ...){
    va_list ap;
    int *z;

    va_start(ap,a);
    z = elemv(a,ap);
    va_end(ap);

    return z;
}


void transpose2(array a){
    int t;
    t = a->dims[0],
        a->dims[0] = a->dims[1],
                     a->dims[1] = t;
    t = a->weight[0],
        a->weight[0] = a->weight[1],
                       a->weight[1] = t;
}

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

void transposea(array a, int spec[]){
    int dims[a->rank];
    int weight[a->rank];
    int i;
    for (i=0; i<a->rank; i++){
        dims[i] = a->dims[spec[i]];
        weight[i] = a->weight[spec[i]];
    }
    memcpy(a->dims, dims, a->rank*sizeof(int));
    memcpy(a->weight, weight, a->rank*sizeof(int));
}

array slice(array a, int i){
    int rank = a->rank-1;
    array z=malloc(sizeof(struct ar)
            + (rank+rank)*sizeof(int));
    z->rank = rank;
    z->dims = (int *)(z+1);
    z->weight = z->dims + z->rank;
    memcpy(z->dims, a->dims+1, z->rank*sizeof(int));
    memcpy(z->weight, a->weight+1, z->rank*sizeof(int));
    z->cons = i*a->weight[0];
    z->data = a->data;
    return z;
}

array slicea(array a, int spec[]){
    int i,j;
    int rank;
    for (i=0, rank=0; i<a->rank; i++)
        rank += spec[i]==-1;
    int dims[rank];
    int weight[rank];
    for (i=0,j=0; i<rank; i++,j++){
        while (spec[j]!=-1) j++;
        if (j>=a->rank) break;
        dims[i] = a->dims[i];
        weight[i] = a->weight[j];
    }
    array z = cast_dims(a->data, rank, dims);
    memcpy(z->weight,weight,rank*sizeof(int));
    for (j=0; j<a->rank; j++){
        if (spec[j]!=-1)
            z->cons += spec[j] * a->weight[j];
    }
    return z;
}

array slices(array a, int s[], int f[]){
    int rank = 0;
    int i;
    for (i=0; i<a->rank; i++){
        rank += s[i] != f[i];
    }
    int dims[rank];
    int weight[rank];
    int j=0;
    for (i=0; i<rank; i++){
        while (s[j]==f[j]) ++j;
        dims[i] = 1 + (s[j]<f[j] ? f[j]-s[j] : s[j]-f[j] );
        weight[i] =    s[j]<f[j] ? a->weight[j] : -a->weight[j];
        ++j;
    }
    array z = cast_dims(a->data, rank, dims);
    memcpy(z->weight, weight, rank*sizeof(int));
    for (i=0; i<a->rank; i++){
        z->cons += s[i] * a->weight[i];
    }
    return z;
}

array extend(array a, int extra){
    int rank = a->rank + extra;
    int dims[rank];
    int i;
    for (i=0; i<extra; i++)
        dims[i] = 1;
    memcpy(dims+extra, a->dims, a->rank*sizeof(int));
    return cast_dims(a->data, rank, dims);
}


array cat(array x, array y){
    int xsz = productdims(x->rank,x->dims);
    int ysz = productdims(y->rank,y->dims);
    int datasz = xsz + ysz;
    array z=array_new(datasz);
    int scratch[x->rank+y->rank];
    int i;
    for (i=0; i<xsz; i++)
        *elem(z,i) = *elema(x,vector_index(i,x->dims,x->rank,scratch));
    for (i=0; i<ysz; i++)
        *elem(z,xsz+i) = *elema(y,vector_index(i,y->dims,y->rank,scratch));
    return z;
}

/* use ret_index instead */
array iota(int n){
    array z = array_new(n);
    int i;
    for (i=0; i<n; i++)
        *elem(z,i) = i;
    return z;
}

array scalar(int n){
    array z = array_new(1);
    *elem(z,0) = n;
    return z;
}

array (vector)(int n, ...){
    va_list ap;
    array z = array_new(n);
    int i;

    va_start(ap,n);
    for (i=0; i<n; i++)
        *elem(z,i) = va_arg(ap, int);
    va_end(ap);
    return z;
}


#ifdef TESTMODULE
#include <stdlib.h>
#include <string.h>
#include "minunit.h"
int tests_run = 0;

static char *test_basic(){
    array a = array_new_dims(1, (int[]){4});
    *elem(a,3) = 12;
    test_case(*elem(a,3)!=12);

    array b = array_new(4,5);
    *elem(b,3,4) = 5;
    test_case(*elem(b,3,4)!=5);

    array c = iota(4);
    test_case(*elem(c,3)!=3);

    array d = iota(64);
    array e = cast(d->data, 2,2,2,2,2,2);
    test_case(*elem(e, 1,1,1,1,1,1) != 63);

    array f = cast(d->data, 4,4,4);
    test_case(*elem(f, 3,3,3) != 63);

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
