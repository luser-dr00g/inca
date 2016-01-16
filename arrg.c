#include<stdarg.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"ppnarg.h" //https://github.com/luser-dr00g/inca/blob/master/ppnarg.h
typedef char C; typedef int I;
typedef struct a{I r,*d,*w,*p;}*A;
#define R return
#define DO(n,x){I i=0,N=(n);for(;i<N;++i){x;}}                   /*loop i=[0..n)*/
I tr(I r,I*d){ I z=1; DO(r,z*=d[i]) R z; }                       /* productdims */
dw(I r,I*d,I*w){ I x=1; DO(r,w[r-1-i]=x; x*=d[r-1-i]) }          /* calculate weights */
A ah(I r,I*d,I sz){ A z=malloc((sizeof*z)+(sizeof(I)*(r+r+sz))); /* array header + sz extra */
    z->r=r; z->d=(I*)(z+1); z->w=z->d+r; z->p=NULL;
    memcpy(z->d,d,r*sizeof(I)); dw(r,d,z->w); R z; }
A ara(I r,I*d){ I sz=tr(r,d); A z=ah(r,d,sz); z->p=z->w+r; R z; } /* arraya */
dv(I r,I*d,va_list v){ DO(r,d[i]=va_arg(v,I)) } /* loaddimsv */
A ar(I r,...){ va_list v; I d[r]; va_start(v,r); dv(r,d,v); va_end(v); R ara(r,d); } /* array rm */
#define ar(...)ar(PP_NARG(__VA_ARGS__),__VA_ARGS__)
A caa(I*p,I r,I*d){ A z=ah(r,d,0); z->p=p; R z; }                /* casta rm */
A ca(I*p,I r,...){ va_list v; I d[r]; va_start(v,r); dv(r,d,v); va_end(v); R caa((I*)p,r,d); }
#define ca(p,...)ca((I*)p,PP_NARG(__VA_ARGS__),__VA_ARGS__)
A cl(A a){ A z=ah(a->r,a->d,0);
    memcpy(z->d,a->d,z->r*sizeof(I));
    memcpy(z->w,a->w,z->r*sizeof(I)); z->p=a->p; R z; }          /* clone */
I*ela(A a,I*j){ I x=0; DO(a->r, x+=j[i]*a->w[i]) R a->p+x; }     /* elema */
I*elv(A a,va_list v){ I j[a->r]; dv(a->r,j,v); R ela(a,j); }     /* elemv */
I*el(A a,...){ I*z; va_list v; va_start(v,a); z=elv(a,v); va_end(v); R z; } /* elem */
int *vx(I x,I*d,I r,I*v){ DO(r, v[r-1-i]=x%d[r-1-i]; x/=d[r-1-i]) R v; }    /* vector_index */
int rx(I*v,I*d,I r){ I z=*v; DO(r-1, z*=d[i+1]; z+=v[i+1]) R z; }           /* ravel_index */
A cp(A a){ I sz=tr(a->r,a->d); A z=ah(a->r,a->d,sz); z->p=z->w+z->r;
    I j[z->r]; DO(sz, vx(i,z->d,z->r,j); z->p[i]=*ela(a,j)) R z; }          /* copy rm */
#define CASE ;break;case

#define OP(x,y)((((x))*256)+((y)))
#define OPS(_)  _("+", '+', 0, +,  0) \
                _("*", '*', 0, *,  1) \
                _("=", '=', 0, ==, 1) \
                _("==",'=','=',==, 1) \
                /* f   f0  f1  F  id */
#define BINOP(f,f0,f1,F,id) CASE OP(f0,f1): *el(z,i) = *el(x,i) F *el(y,i);
A bin(A x,C*f,A y){ A z=cp(x); DO(x->d[0], switch(OP(*f,f[1])){ OPS(BINOP) }) R z; }
#define bin(X,F,Y) bin(X,#F,Y)
#define REDID(f,f0,f1,F,id) CASE OP(f0,f1): z = id;
#define REDOP(f,f0,f1,F,id) CASE OP(f0,f1): z = *el(y,n-2-i) F z;
int red(C*f,A y){ I z; I n=y->d[0];
    switch(OP(*f,f[1])){ OPS(REDID) }
    if (n){ z=*el(y,n-1);
            if (n-1>0) DO(n-1, switch(OP(*f,f[1])){ OPS(REDOP) }) }
    R z; }
#define red(F,Y) red(#F,Y)

A xt(A a,I x){ I r=a->r+x; I d[r];
    DO(x, d[i]=1) memcpy(d+x,a->d,a->r*sizeof(I)); R caa(a->p,r,d); } /* extend */
A iot(I n){ A z=ar(n); DO(n,*el(z,i)=i) R z; } /* index generator */

A xp(A a,I*j){
    I d[a->r]; I w[a->r];
    DO(a->r, d[i]=a->d[j[i]];
             w[i]=a->w[j[i]])
    A z=caa(a->p,a->r,d);
    memcpy(z->w,w,a->r*sizeof(I)); R z; } /* transpose */

A sl0(A a,I x){ I r=a->r-1; A z=ah(r,a->d+1,0);
    memcpy(z->w,a->w+1,r*sizeof(I));
    z->p=a->p+x*a->w[0]; R z; }

A sl(A a,I*s,I*f){ I r=0;
    DO(a->r, r+=s[i]!=f[i])
    I d[r]; I w[r]; I j=0;
    DO(r,   while(s[j]==f[j])++j;
            d[i]=1+(s[j]<f[j]? f[j]-s[j] : s[j]-f[j]);
            w[i]=   s[j]<f[j]? a->w[j]   : -a->w[j];
            ++j)
    A z=caa(a->p,r,d);
    memcpy(z->w,w,r*sizeof(I)); DO(a->r, z->p += s[i] * a->w[i]) R z; } /* slices [s[i]..f[i]] */

I contig(A a){ I x=1;
    DO(a->r, if(a->w[a->r-1-i]!=x) R 0;
             x*=a->d[a->r-1-i])
    R 1; }

pr(A a,I wid){ I max; I copy=0;
    if (wid){ max=wid; } else {
        I sz=tr(a->r,a->d);
        if (!contig(a)){
            a=cp(a);
            copy=1;
        }
        max=0;
        DO(sz,
            sz=snprintf(NULL,0,"%d",a->p[i]);
            if (sz>max) max=sz)
    }
    switch(a->r){
    case 0: printf("%*d\n", max, *a->p); break;
    case 1: {
                I sep=0;
                DO(a->d[0],
                    if (sep) printf(" ");
                    printf("%*d", max, *el(a,i));
                    sep=1) }
            if (wid==0) printf("\n"); break;
    default:DO(a->d[0], A t=sl0(a,i);
                        pr(t,max);
                        printf("\n");
                        free(t)) break;
    }
    if(copy)free(a); }

dmp(A a){
    printf("%d\n",a->r);
    DO(a->r,printf("%d ",a->d[i])) printf("\n");
    DO(a->r,printf("%d ",a->w[i])) printf("\n");
    printf("%p %p %p %p\n",
            (void*)a, (void*)a->d, (void*)a->w, (void*)a->p); }

int main(){
    A x=ar(3);
    *el(x,0)=5;
    *el(x,1)=6;
    *el(x,2)=7;
    dmp(x);
    pr(x,0);

    A a=iot(24);
    dmp(a);
    pr(a,0);
    A b=ca(a->p,2,3,4);
    dmp(b);
    pr(b,0);
    A c=sl(b,(I[]){0,1,1},(I[]){0,2,3});
    pr(c,0);
    A d=xp(c,(I[]){1,0});
    pr(d,0);
    return 0;
}
