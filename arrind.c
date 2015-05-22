//arrind.c - Arrays, indirect.

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ppnarg.h" //https://github.com/luser-dr00g/inca/blob/master/ppnarg.h

typedef struct arr {
    int rank;
    int *dims;
    int *weight;
    int *data;
} *arr;

int productdims(int rank, int *dims);               /* multiply together rank integers in dims array */
void loaddimsv(int rank, int dims[], va_list ap);   /* load rank integers from va_list into int[] dims */

#define array(...) (array)(PP_NARG(__VA_ARGS__),__VA_ARGS__) /* create a new array with specified dimensions */
arr (array)(int rank, ...);             /* create a new array with specified rank and dimensions */
arr arraya(int rank, int dims[]);        /* create array given rank and int[] dims */
arr casta(int *data, int rank, int dims[]);
arr cast(int *data, int rank, ...); /* create an array header to access existing data in multidimensional layout */
arr clone(arr a);               /* create a new array which shares the data of an existing array */
arr copy(arr a);                /* create a (contiguous) copy of a (not necessarily contiguous) existing array */

void transpose2(arr a);         /* exchange the leftmost two dimensions (only two in 2D) */
void transpose(int shift, arr a); /* rotate dims and weights according to sign and magnitude of shift
                                       transpose(1,a)==transpose(-1,a)==transpose2(a) for 2D */
arr slice(arr a, int i);        /* take a (row) slice (in 2D) */
arr slicea(arr a, int spec[]);  /* take a computed slice of a following spec[] instructions */
arr extend(arr a, int extra);   /* prepend extra unit dimensions to a */

int *elem(arr a, ...);          /* access element of a indexed by integer arguments */
int *elema(arr a, int *ind);    /* access element of a indexed by int[] */
int *elemv(arr a, va_list ap);  /* access element of a indexed by va_list */

int *vector_index(int ind, int *dims, int n, int *vec); /* compute vector index list for ravel index ind */
int ravel_index(int *vec, int *dims, int n);            /* compute ravel index for vector index list */

arr catv(arr x, arr y);         /* create a vector of all elements of x followed by all elements of y */
arr iota(int n);                /* generate an index vector 0..n-1 */

#define OPERATORS(_) \
    /* f  F id */ \
    _('+',+,0) \
    _('*',*,1) \
    _('=',==,1) \
    /**/

#define binop(X,F,Y) (binop)(X,*#F,Y)
arr (binop)(arr x, char f, arr y); /* perform binary operation F upon corresponding elements of vectors X and Y */

#define reduce(F,X) (reduce)(*#F,X)
int (reduce)(char f, arr a); /* perform binary operation F upon adjacent elements of vector X, right to left,
                                   reducing vector to a single value */

#define matmul(X,F,G,Y) (matmul)(X,*#F,*#G,Y)
arr (matmul)(arr x, char f, char g, arr y);
        /* perform a (2D) matrix multiplication upon rows of x and columns of y using operations F and G.
           more generally, perform an inner product on arguments of compatible dimension.  */


/* multiply together rank integers in dims array */
int productdims(int rank, int *dims){
    int i,z=1;
    for(i=0; i<rank; i++)
        z *= dims[i];
    return z;
}

/* create array given rank and int[] dims */
arr arraya(int rank, int dims[]){
    int datasz;
    int i;
    int x;
    arr z;
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

    return z;
}

/* load rank integers from va_list into int[] dims */
void loaddimsv(int rank, int dims[], va_list ap){
    int i;
    for (i=0; i<rank; i++){
        dims[i]=va_arg(ap,int);
    }
}

/* create a new array with specified rank and dimensions */
arr (array)(int rank, ...){
    va_list ap;
    //int *dims=calloc(rank,sizeof(int));
    int dims[rank];
    int i;
    int x;
    arr z;

    va_start(ap,rank);
    loaddimsv(rank,dims,ap);
    va_end(ap);

    z = arraya(rank,dims);
    //free(dims);
    return z;
}

/* create an array header to access existing data in multidimensional layout */
arr casta(int *data, int rank, int dims[]){
    int i,x;
    arr z=malloc(sizeof(struct arr)
            + (rank+rank)*sizeof(int));

    z->rank = rank;
    z->dims = (int *)(((char *)z) + sizeof(struct arr));
    z->weight = z->dims + rank;
    z->data = data;
    memmove(z->dims,dims,rank*sizeof(int));
    for(x=1, i=rank-1; i>=0; i--){
        z->weight[i] = x;
        x *= z->dims[i];
    }

    return z;
}

/* create an array header to access existing data in multidimensional layout */
arr cast(int *data, int rank, ...){
    va_list ap;
    int dims[rank];
    int i;
    int x;
    arr z;

    va_start(ap,rank);
    loaddimsv(rank,dims,ap);
    va_end(ap);

    return casta(data, rank, dims);
}

/* create a new array which shares the data of an existing array */
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

/* exchange the leftmost two dimensions (only two in 2D) */
void transpose2(arr a){
    int t;
    //if (a->rank != 2) error();
    t = a->dims[0]; a->dims[0] = a->dims[1]; a->dims[1] = t;
    t = a->weight[0]; a->weight[0] = a->weight[1]; a->weight[1] = t;
}

/* rotate dims and weights according to sign and magnitude of shift
   transpose(1,a)==transpose(-1,a)==transpose2(a) for 2D */
void transpose(int shift, arr a){
    int i;
    int t;
    while(shift){
        if (shift>0){
            t=a->dims[0];
            for (i=1; i<a->rank; i++)
                a->dims[i-1]=a->dims[i];
            a->dims[a->rank-1]=t;
            t=a->weight[0];
            for (i=1; i<a->rank; i++)
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

/* take a computed slice of a following spec[] instructions
   if spec[i] >= 0 and spec[i] < a->rank, then spec[i] selects
      that index from dimension i.
   if spec[i] == -1, then spec[i] selects the entire dimension i.
 */
arr slicea(arr a, int spec[]){
    int i,j;
    int rank;
    for (i=0,rank=0; i<a->rank; i++)
        rank+=spec[i]==-1;
    int dims[rank];
    int weight[rank];
    for (i=0,j=0; i<rank; i++,j++){
        while (spec[j]!=-1) j++;
        if (j>=a->rank) break;
        dims[i] = a->dims[j];
        weight[i] = a->weight[j];
    }
    arr z = casta(a->data, rank, dims);
    memcpy(z->weight,weight,rank*sizeof(int));
    for (j=0; j<a->rank; j++){
        if (spec[j]!=-1)
            z->data += spec[j] * a->weight[j];
    }
    return z;
}

/* prepend extra unit dimensions to a */
arr extend(arr a, int extra){
    int rank = a->rank + extra;
    int dims[rank];
    int i;
    for (i=0; i<extra; i++)
        dims[i] = 1;
    memmove(dims+extra, a->dims, a->rank*sizeof(int));
    return casta(a->data, rank, dims);
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

/* compute vector index list for ravel index ind */
int *vector_index(int ind, int *dims, int n, int *vec){
    int i,t=ind, *z=vec;
    for (i=0; i<n; i++){
        z[n-1-i] = t % dims[n-1-i];
        t /= dims[n-1-i];
    }
    return z;
}

/* compute ravel index for vector index list */
int ravel_index(int *vec, int *dims, int n){
    int i,z=*vec;
    for (i=0; i<n-1;i++){
        z*=dims[i+1];
        z+=vec[i+1];
    }
    return z;
}

/* create a vector of all elements of x followed by all elements of y */
arr catv(arr x, arr y){
    int xsz = productdims(x->rank,x->dims);
    int ysz = productdims(y->rank,y->dims);
    int datasz = xsz + ysz;
    arr z=array(datasz);
    int scratch[x->rank+y->rank];
    int i;
    for (i=0; i<xsz; i++)
        *elem(z,i) = *elema(x,vector_index(i,x->dims,x->rank,scratch));
    for (i=0; i<ysz; i++)
        *elem(z,xsz+i) = *elema(y,vector_index(i,y->dims,y->rank,scratch));
    return z;
}

/* create a (contiguous) copy of a (not necessarily contiguous) existing array */
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
        vector_index(i,z->dims,z->rank,ind);
        z->data[i] = *elema(a,ind);
    }

    return z;
}

/* perform binary operation F upon corresponding elements of vectors X and Y */
#define BINOP(f,F,id) case f: *elem(z,i) = *elem(x,i) F *elem(y,i); break;
arr (binop)(arr x, char f, arr y){
    arr z=copy(x);
    int n=x->dims[0];
    int i;
    for (i=0; i<n; i++){
        switch(f){
            OPERATORS(BINOP)
        }
    }
    return z;
}
#undef BINOP

/* perform binary operation F upon adjacent elements of vector X, right to left,
   reducing vector to a single value */
#define REDID(f,F,id) case f: x = id; break;
#define REDOP(f,F,id) case f: x = *elem(a,i) F x; break;
int (reduce)(char f, arr a){
    int n = a->dims[0];
    int x;
    int i;
    switch(f){
        OPERATORS(REDID)
    }
    if (n) {
        x=*elem(a,n-1);
        for (i=n-2;i>=0;i--){
            switch(f){
                OPERATORS(REDOP)
            }
        }
    }
    return x;
}
#undef REDID
#undef REDOP

/* perform a (2D) matrix multiplication upon rows of x and columns of y
   using operations F and G.
       Z = X F.G Y
       Z[i,j] = F/ X[i,*] G Y'[j,*]

   more generally,
   perform an inner product on arguments of compatible dimension.
       Z = X[A;B;C;D;E;F] +.* Y[G;H;I;J;K]  |(F = G)
       Z[A;B;C;D;E;H;I;J;K] = +/ X[A;B;C;D;E;*] * Y[*;H;I;J;K]
 */
arr (matmul)(arr x, char f, char g, arr y){
    int i,j;
    arr xdims = cast(x->dims,1,x->rank);
    arr ydims = cast(y->dims,1,y->rank);
    xdims->dims[0]--;
    ydims->dims[0]--;
    ydims->data++;
    arr z=arraya(x->rank+y->rank-2,catv(xdims,ydims)->data);
    int datasz = productdims(z->rank,z->dims);
    int k=y->dims[0];
    arr xs = NULL;
    arr ys = NULL;

    for (i=0; i<datasz; i++){
        int idx[x->rank+y->rank];
        vector_index(i,z->dims,z->rank,idx);
        int *xdex=idx;
        int *ydex=idx+x->rank-1;
        memmove(ydex+1,ydex,y->rank);
        xdex[x->rank-1] = -1;
        free(xs);
        free(ys);
        xs = slicea(x,xdex);
        ys = slicea(y,ydex);
        z->data[i] = (reduce)(f,(binop)(xs,g,ys));
    }

    free(xs);
    free(ys);
    free(xdims);
    free(ydims);
    return z;
}

/* generate an index vector 0..n-1 */
arr iota(int n){
    arr z = array(n);
    int i;
    for (i=0;i<n;i++)
        *elem(z,i)=i;
    return z;
}

int main(){

#ifdef TEST_BASIC
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

#ifdef TEST_MATMUL
    {   /* testing reduce() */
        int i,n=3;
        arr a=array(n);
        arr b;
        for (i=0; i<n; i++)
            *elem(a,i) = i+1;
        printf("%2d\n", reduce(*,a));

        free(a);

        n=6;
        a=array(n);
        for (i=0; i<n; i++)
            *elem(a,i) = 5;
        b=binop(a,=,a);
        for (i=0; i<n; i++,printf(" "))
            printf("%2d", *elem(b,i));
        printf("\n%2d\n", reduce(=,b));
        free(a);
        free(b);
    }

    {   /* testing matmul() */
        int i,j,n=3;
        arr a=array(n,n);
        arr b;

        for (i=0;i<n;i++)
            for (j=0;j<n;j++)
                *elem(a,i,j) = ((i*n)+j)+1;
        for (i=0;i<n;i++,printf("\n"))
            for (j=0;j<n;j++,printf(" "))
                printf("%3d",*elem(a,i,j));
        printf("\n");
        b = matmul(a,+,*,a);
        for (i=0;i<n;i++,printf("\n"))
            for (j=0;j<n;j++,printf(" "))
                printf("%3d",*elem(b,i,j));
        printf("\n");

        free(a);
        free(b);
    }

    {   /* testing matmul with higher dimensional arrays */
        int i,j,k,l,n=2;
        arr a=iota(n*n*n);
        arr b=cast(a->data,3,n,n,n);
        for (i=0; i<b->dims[0]; i++,printf("\n"))
            for (j=0; j<b->dims[1]; j++,printf("\n"))
                for (k=0; k<b->dims[2]; k++,printf(" "))
                    printf("%3d", *elem(b,i,j,k));
        printf("\n");
        arr c=matmul(b,+,*,b);
        printf("%d\n",c->rank);
        for (i=0; i<c->rank; i++,printf(" "))
            printf("%d",c->dims[i]);
        printf("\n");
        for (i=0; i<c->dims[0]; i++,printf("\n"))
            for (j=0; j<c->dims[1]; j++,printf("\n"))
                for (k=0; k<c->dims[2]; k++,printf("\n"))
                    for (l=0; l<c->dims[3]; l++,printf(" "))
                        printf("%3d", *elem(c,i,j,k,l));
        printf("\n");
        free(a);
        free(b);
        free(c);
    }
#endif

#ifdef TEST_CAST
    {   /* testing cast() */
        int i,j,k,n=3;
        int q[n][n][n];
        arr a=cast((int *)q,3,n,n,n);

        for (i=0;i<n;i++)
            for (j=0;j<n;j++)
                for (k=0;k<n;k++)
                    *elem(a,i,j,k) = n*n*n - ((i*n+j)*n+k);
        for (i=0;i<n;i++,printf("\n"))
            for (j=0;j<n;j++,printf("\n"))
                for (k=0;k<n;k++,printf(" "))
                    printf("%2d",*elem(a,i,j,k));
        printf("\n");
        for (i=0;i<n;i++,printf("\n"))
            for (j=0;j<n;j++,printf("\n"))
                for (k=0;k<n;k++,printf(" "))
                    printf("%2d", q[i][j][k]);
        printf("\n");

        free(a);
    }
#endif

#ifdef TEST_IOTA
    {
        arr a=iota(12);
        arr b=catv(a,a);
        int i;
        for (i=0;i<b->dims[0];i++,printf(" "))
            printf("%2d", *elem(b,i));
        printf("\n");

        arr c=cast(b->data,3,2,3,4);
        int j,k;
        for (i=0; i<2; i++,printf("\n"))
            for (j=0; j<3; j++,printf("\n"))
                for (k=0; k<4; k++,printf(" "))
                    printf("%2d", *elem(c,i,j,k));

        free(a);
        free(b);
        free(c);
    }
#endif

#ifdef TEST_SLICE
    {
        int n=4;
        arr a=iota(n*n*n);
        arr b=cast(a->data, 3, n,n,n);
        arr c=NULL;
        int i,j,k;

#if 0
        for (i=0; i<n; i++,printf("\n"))
            for (j=0; j<n; j++,printf("\n"))
                for (k=0; k<n; k++,printf(" "))
                    printf("%2d", *elem(b,i,j,k));
        printf("\n");
#endif

        for (k=0; k<n; k++){
            free(c);
            c=slicea(b,(int[]){k,-1,-1});
            printf("%d;-1;-1\n",k);
            for (i=0; i<n; i++,printf("\n"))
                for (j=0; j<n; j++,printf(" "))
                    printf("%2d", *elem(c,i,j));
            printf("\n");
        }

        for (k=0; k<n; k++){
            free(c);
            c=slicea(b,(int[]){-1,0,k});
            printf("-1;0;%d\n",k);
            printf("d:%d w:%d\n", c->dims[0], c->weight[0]);
            for (i=0; i<n; i++,printf(" "))
                printf("%2d", *elem(c,i));
            printf("\n");
        }

        for (k=0; k<n; k++){
            free(c);
            c=slicea(b,(int[]){-1,k,0});
            printf("-1;%d;0\n",k);
            for (i=0; i<n; i++,printf(" "))
                printf("%2d", *elem(c,i));
            printf("\n");
        }

        for (k=0; k<n; k++){
            free(c);
            c=slicea(b,(int[]){k,-1,k});
            printf("%d;-1;%d\n",k,k);
            for (i=0; i<n; i++,printf(" "))
                printf("%2d", *elem(c,i));
            printf("\n");
        }
        for (k=0; k<n; k++){
            free(c);
            c=slicea(b,(int[]){k,k,k});
            printf("%d;%d;%d\n",k,k,k);
            printf("%2d", *elem(c));
            printf("\n");
        }

        free(a);
        free(b);
        free(c);
    }
#endif

    return 0;
}

