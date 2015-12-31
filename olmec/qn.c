#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static inline int max(int x, int y){ return x>y?x:y; }
static inline int abv(int x){ return x<0?-x:x; }
static inline int gcd(int x, int y){
    return x==0&&y==0?1
        : y==0?x:gcd(y,x%y);
}
static inline int lcm(int x, int y){
    return x==0||y==0?1
        : (abv(x)/gcd(x,y))*abv(y);
}
static inline int totient(int x){ int i,z=0;
    if(x<2)return 1;
    for(i=1;i<x;i++) z+=gcd(i,x)==1; return z;
}

void qprint(q x){
    int i;
    printf("%d %d %d (", x->b, x->n, x->m);
    if (x->n+x->m)
        printf("%d",x->d[0]);
    for (i=1; i<x->n+x->m; i++)
        printf(", %d",x->d[i]);
    printf(")\n");
}

q qneg(q x){
    return qsub(qlong(0),x);
}

/* nb. not defined for LONG_MIN */
q qlong(long i){
    if (i<0) return qneg(qlong(-i));
    int m=0;
    int n=i?ceil(log(i)/log(base)):0;
    q z=malloc(sizeof*z + (n+m)*sizeof*z->d);
        z->b=base; z->n=n; z->m=m;
    int j;
    for(j=0; j<n; j++){
        z->d[j] = i % base;
        i /= base;
    }
    return z;
}

long longq(q x){
    return 1;
}

int qsign(q x){
    switch(((x->n==0))<<1+(x->m==0)){
    case (1<<1)+1: return 0;
    case 1<<1: return -1;
    case 1: return 1;
    case 0: return x->d[x->n-1] - x->d[x->n+x->m-1];
    }
}

int qdig(q x,int k){
    return k < x->n ? x->d[k] :
           x->m ? x->d[x->n + (k - x->n) % x->m]
           : 0;
}

void findrepetition(q z){
    int i;
    int j;
    qprint(z);
    for(i=1; i<z->n+z->m-1; i*=2) {
        for(j=i+1; j<2*i; j++) {
            if (z->d[j-1]==z->d[i-1]) {
                z->n=i;
                z->m=j-i;
                return;
            }
        }
    }
}

q qadd(q x,q y){
    int m=lcm(x->m,y->m);
    int n=max(x->n,y->n)+m+2;
    int t,c=0;
    q z=malloc(sizeof*z + (n+m)*sizeof*z->d);
        z->b=base; z->n=n; z->m=m;
    int k;
    for(k=0; k<n+m; k++){
        t = qdig(x,k) + qdig(y,k) + c;
        z->d[k] = t % base;
        c = t / base;
    }

    findrepetition(z);
    return z;
}

q qsub(q x,q y){
    int m=lcm(x->m,y->m);
    int n=max(x->n,y->n)+m+2;
    int t,c=0;
    q z=malloc(sizeof*z + (n+m)*sizeof*z->d);
        z->b=base; z->n=n; z->m=m;
    int k;
    for(k=0; k<n+m; k++){
        t = qdig(x,k) - (qdig(y,k) + c);
        if (t < 0){
            c = 1;
            t += base;
        } else {
            c = 0;
        }
        z->d[k] = t % base;
    }

    findrepetition(z);
    return z;
}

q qmul(q x,q y){
    int m=lcm(x->m,y->m);
    int n=x->n+y->n+m+1;
    q z=malloc(sizeof*z + (n+m)*sizeof*z->d);
        z->b=base; z->n=n; z->m=m;
}

q qdiv(q x,q y){
    int m=lcm(x->m,totient(longq(y)*(pow(base,y->m)-1)));
    int n= y->n<=y->m ? x->n + y->n + m + 1 :
           y->m<y->n && y->n<=x->n ? x->n - y->n + m + 1:
           m+1;
    q z=malloc(sizeof*z + (n+m)*sizeof*z->d);
        z->b=base; z->n=n; z->m=m;
}

#ifdef TESTMODULE
#include "minunit.h"
int tests_run = 0;

static char *test_qadd(){
    q x = qlong(12);
    qprint(x);
    q y = qlong(97);
    qprint(y);
    q z = qadd(x,y);
    qprint(z);
    return 0;
}

static char *test_qsub(){
    q x = qlong(0);
    qprint(x);
    q y = qlong(33);
    qprint(y);
    q z = qsub(x,y);
    qprint(z);
    return 0;
}

static char *all_tests(){
    mu_run_test(test_qadd);
    mu_run_test(test_qsub);
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

#endif
