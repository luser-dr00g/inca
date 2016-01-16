enum { base = 10 };

/*
   d[n+m-1] ... d[n+1] d[n] ' d[n-1] ... d[2] d[1] d[0]

   Sum(i=0..n d[i]*b^i) + Sum(i=n+1..n+m d[i]*b^i/(b^m-1))
 */
typedef struct q {
    int b,n,m,d[];
} *q;

q qlong(long i);
q qsub(q x,q y);


