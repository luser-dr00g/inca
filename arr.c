/*
   Example code for SO answer:
http://stackoverflow.com/questions/30023867/how-can-i-work-with-dynamically-allocated-arbitrary-dimensional-arrays/30023868#30023868
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct arr {
    int r;
    int d[1];
} *arr;

int productdims(int rank, int *dims){
    int z=1;
    for(int i=0; i<rank; i++)
        z *= dims[i];
    return z;
}

arr makearr(int rank, int *dims){
    arr z = calloc( (sizeof(struct arr)/sizeof(int)) +
                rank + productdims(rank,dims), sizeof(int));
    z->r = rank;
    memmove(z->d,dims,rank*sizeof(int));
    return z;
}

/*
   idx = i0*w0 + i1*w1 + ... iN*wN
   w0 = d1*d2*d3* ... *dN
   w1 =    d2*d3* ... *dN
   ...
   wN = 1

   idx=0
   idx+=i0

   idx*=d1
   idx+=i1

   idx*=d2
   idx+=i2
   ...
   idx*=dN
   idx+=iN
   idx*=1

 */

int *elem(arr a, ...){
    va_list ap;
    int idx = 0;

    va_start(ap,a);
    if (a->r){
        idx = va_arg(ap,int);
        for(int i=1; i<a->r; i++){
            idx *= a->d[i];
            idx += va_arg(ap,int);
        }
    }
    va_end(ap);

    return &a->d[a->r + idx];
}

#if 0
int *elem(arr a, ...){
    va_list ap;
    int idx = 0;
    int weight = 1;
    int *ind=calloc(a->r, sizeof(int));

    va_start(ap,a);
    for(int i=0; i < a->r; i++){
        ind[i]=va_arg(ap,int);
    }
    va_end(ap);

    for(int i=a->r-1; i>=0; i--){
        idx += ind[i]*weight;
        weight *= a->d[i];
    }

    return &a->d[a->r + idx];
}
#endif


int main() {


    {
        int i,n=6;
        arr m = makearr(1, (int[]){n});
        for (i=0;i<n;i++)
            *elem(m,i) = i;
        for (i=0;i<n;i++,printf(" "))
            printf("%d",*elem(m,i));
    }
    puts("\n");

    {
        int i,j,n=4;
        arr m = makearr(2, (int[]){n,n});
        for (i=0;i<n;i++)
            for (j=0;j<n;j++)
                    *elem(m,i,j) = i*n+j;
        for (i=0;i<n;i++,printf("\n"))
            for (j=0;j<n;j++,printf(" "))
                    printf("%d",*elem(m,i,j));
    }
    puts("\n");

    {
        int i,j,k,n=3;
        arr m = makearr(3, (int[]){n,n,n});
        for (i=0;i<n;i++)
            for (j=0;j<n;j++)
                for (k=0;k<n;k++)
                    *elem(m,i,j,k) = (i*n+j)*n+k;
        for (i=0;i<n;i++,printf("\n"))
            for (j=0;j<n;j++,printf("\n"))
                for (k=0;k<n;k++,printf(" "))
                    printf("%d",*elem(m,i,j,k));
    }

    return 0;
}
