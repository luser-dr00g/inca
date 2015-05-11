#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ppnarg.h"

typedef struct arr {
    int rank;
    int *dims;
    int *weight;
    int *data;
} *arr;

/* multiply together rank integers in dims array
   */
int productdims(int rank, int *dims){
    int i,z=1;
    for(i=0; i<rank; i++)
        z *= dims[i];
    return z;
}

/* create a new array with specified dimensions */
#define array(...) (makearr)(PP_NARG(__VA_ARGS__),__VA_ARGS__)

/* create a new array with specified rank and dimensions
   */
arr (makearr)(int rank, ...){
    va_list ap;
    //int *dims=calloc(rank,sizeof(int));
    int dims[rank];
    int datasz;
    int i;
    int x;
    arr z;

    va_start(ap,rank);
    for (i=0; i<rank; i++){
        dims[i]=va_arg(ap,int);
    }
    va_end(ap);

    datasz=productdims(rank,dims);
    z=malloc(sizeof(struct arr)
            + (rank+rank+datasz)*sizeof(int));

    z->rank = rank;
    z->dims = (int*)(((char*)z) + sizeof(struct arr));
    z->weight = z->dims + rank;
    z->data = z->weight + rank;
    memmove(z->dims,dims,rank*sizeof(int));
    for(x=1, i=rank-1; i>=0; i--){
        z->weight[i] = x;
        x *= z->dims[i];
    }

    //free(dims);
    return z;
}

/* create a new array which shares the data of an existing array
   */
arr clone(arr a){
    arr z=malloc(sizeof(struct arr)
            + (a->rank+a->rank)*sizeof(int));
    z->rank = a->rank;
    z->dims = (int*)(((char*)z) + sizeof(struct arr));
    z->weight = z->dims + z->rank;
    z->data = a->data;
    memmove(z->dims,a->dims,z->rank*sizeof(int));
    memmove(z->weight,a->weight,z->rank*sizeof(int));
    return z;
}

/* exchange the leftmost two dimensions (only two in 2D)
   */
void transpose2(arr a){
    int t;
    //if (a->rank != 2) error();
    t = a->dims[0]; a->dims[0] = a->dims[1]; a->dims[1] = t;
    t = a->weight[0]; a->weight[0] = a->weight[1]; a->weight[1] = t;
}

/* take a (row) slice (in 2D) */
arr slice(arr a, int i){
    int rank = a->rank-1;
    arr z=malloc(sizeof(struct arr)
            + (rank+rank)*sizeof(int));
    z->rank = rank;
    z->dims = (int*)(((char*)z) + sizeof(struct arr));
    z->weight = z->dims + z->rank;
    memmove(z->dims,a->dims+1,z->rank*sizeof(int));
    memmove(z->weight,a->weight+1,z->rank*sizeof(int));
    z->data = a->data + i*a->weight[0];
    return z;
}

/* access element of a indexed by int[] */
int *elema(arr a, int *ind){
    int idx = 0;
    int i;
    for (i=0; i<a->rank; i++){
        idx += ind[i] * a->weight[i];
    }
    return a->data + idx;
}

/* access element of a indexed by va_list */
int *elemv(arr a, va_list ap){
    int idx = 0;
    int i;
    for(i=0; i<a->rank; i++){
        int ind;
        ind = va_arg(ap,int);
        //if (ind > a->dims[i]) error();
        idx += ind * a->weight[i];
    }
    return a->data + idx;
}

/* access element of a indexed by integer arguments */
int *elem(arr a, ...){
    va_list ap;
    int *z;

    va_start(ap,a);
    z = elemv(a,ap);
    va_end(ap);

    return z;
}

/* create a (contiguous) copy of a (not necessarily contiguous)
   existing array
   */
arr copy(arr a){
    int datasz = productdims(a->rank,a->dims);
    arr z=malloc(sizeof(struct arr)
            + (a->rank+a->rank+datasz)*sizeof(int));
    int i;
    int x;
    int ind[a->rank];

    z->rank = a->rank;
    z->dims = (int*)(((char*)z) + sizeof(struct arr));
    z->weight = z->dims + z->rank;
    z->data = z->weight + z->rank;
    memmove(z->dims,a->dims,z->rank*sizeof(int));
    for(x=1, i=z->rank-1; i>=0; i--){
        z->weight[i] = x;
        x *= z->dims[i];
    }

    for (i=0;i<datasz;i++){
        int j;
        int idx = i;
        for (j=z->rank-1; j>=0; j--) { //generate index
            ind[j] = idx % z->dims[j];
            idx /= z->dims[j];
        }
        z->data[i] = *elema(a,ind);
    }

    return z;
}

/* perform binary operation F upon corresponding elements of X and Y */
#define binop(X,F,Y) (binop)(X,*#F,Y)
#define BINOP(f,F) case f: *elem(z,i) = *elem(x,i) F *elem(y,i); break;
arr (binop)(arr x, char f, arr y){
    arr z=copy(x);
    int n=x->dims[0];
    int i;
    for (i=0; i<n; i++){
        switch(f){
        BINOP('+',+)
        BINOP('*',*)
        }
    }
    return z;
}
#undef BINOP

/* perform binary operation F upon adjacent elements, right to left,
   reducing vector to a single value */
#define reduce(F,X) (reduce)(*#F,X)
#define REDID(f,id) case f: x = id; break;
#define REDOP(f,F) case f: x = *elem(a,i) F x; break;
int (reduce)(char f, arr a){
    int n = productdims(a->rank,a->dims);
    int x;
    int i;
    switch(f){
    REDID('+',0)
    REDID('*',1)
    }
    if (n) {
        x=*elem(a,n-1);
        for (i=n-2;i>=0;i--){
            switch(f){
            REDOP('+',+)
            REDOP('*',*)
            }
        }
    }
    return x;
}
#undef REDID
#undef REDOP

/* perform a (2D) matrix multiplication upon x and y */
arr matmul(arr x, arr y){
    int i,j;
    arr z=array(x->dims[0],y->dims[1]);
    y=clone(y);
    transpose2(y);
    for (i=0; i<x->dims[0]; i++){
        for (j=0; j<y->dims[0]; j++){
            arr xs = slice(x,i);
            arr ys = slice(y,j);
            *elem(z,i,j)=reduce(+,binop(xs,*,ys));
            free(xs);
            free(ys);
        }
    }

    free(y);
    return z;
}

int main(){

#if 0
    /* testing basic functionality and copies, transposes, and slices */
    int loop;
    for (loop = 0;
         loop < 1/*00000*/;
         loop++)
    {

        {
            int i,n=12;
            arr a=array(n);

            for (i=0;i<n;i++)
                *elem(a,i) = n-i;
            for (i=0;i<n;i++,printf(" "))
                printf("%2d",*elem(a,i));
            printf("\n\n");

            free(a);
        }

        {
            int i,j,n=6;
            arr a=array(n,n);
            arr b;

            for (i=0;i<n;i++)
                for (j=0;j<n;j++)
                    *elem(a,i,j) = n*n - (i*n+j);
            for (i=0;i<n;i++,printf("\n"))
                for (j=0;j<n;j++,printf(" "))
                    printf("%2d",*elem(a,i,j));
            printf("\n");

            b=clone(a);
            transpose2(b);
            for (i=0;i<n;i++,printf("\n"))
                for (j=0;j<n;j++,printf(" "))
                    printf("%2d",*elem(b,i,j));
            printf("\n");

            free(b);
            free(a);
        }

        {
            int i,j,k,n=3;
            arr a=array(n,n,n);
            arr b;
            arr c;
            arr d;
            for (i=0;i<n;i++)
                for (j=0;j<n;j++)
                    for (k=0;k<n;k++)
                        *elem(a,i,j,k) = n*n*n - ((i*n+j)*n+k);
            for (i=0;i<n;i++,printf("\n"))
                for (j=0;j<n;j++,printf("\n"))
                    for (k=0;k<n;k++,printf(" "))
                        printf("%2d",*elem(a,i,j,k));
            printf("\n");

            /*
            b=slice(a,0);
            transpose2(b);
            for (i=0;i<n;i++)
                for (j=0;j<n;j++)
                    *elem(b,i,j) = n*n - (i*n+j);
            for (i=0;i<n;i++,printf("\n"))
                for (j=0;j<n;j++,printf("\n"))
                    for (k=0;k<n;k++,printf(" "))
                        printf("%2d",*elem(a,i,j,k));
            printf("\n");
            free(b);
            */

            b=clone(a);
            transpose2(b);
            c=slice(b,1);
            transpose2(c);
            for (i=0;i<n;i++)
                for (j=0;j<n;j++)
                    *elem(c,i,j) = n*n - (i*n+j);
            for (i=0;i<n;i++,printf("\n"))
                for (j=0;j<n;j++,printf("\n"))
                    for (k=0;k<n;k++,printf(" "))
                        printf("%2d",*elem(b,i,j,k));
            printf("\n");

            //d = copy(a);
            d = copy(b);
            for (i=0; i<n*n*n; i++){
                printf("%2d", d->data[i]);
                printf(" ");
                if (((i+1)%n)==0) printf("\n");
                if (((i+1)%(n*n))==0) printf("\n");
            }

            free(a);
            free(b);
            free(c);
            free(d);
        }
    }
#endif

    {   /* testing reduce() */
        int i,n=3;
        arr a=array(n);
        for (i=0; i<n; i++)
            *elem(a,i) = i+1;
        printf("%2d\n", reduce(*,a));
    }

    {   /* testing matmul() */
        int i,j,n=3;
        arr a=array(n,n);
        arr b;

        for (i=0;i<n;i++)
            for (j=0;j<n;j++)
                *elem(a,i,j) = ((i*n)+j)+1;
        b = matmul(a,a);
        for (i=0;i<n;i++,printf("\n"))
            for (j=0;j<n;j++,printf(" "))
                printf("%2d",*elem(a,i,j));
        printf("\n");
        for (i=0;i<n;i++,printf("\n"))
            for (j=0;j<n;j++,printf(" "))
                printf("%2d",*elem(b,i,j));
        printf("\n");

    }

    return 0;
}

