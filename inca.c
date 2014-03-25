#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char C;
typedef intptr_t I;
typedef struct a{I t,r,d[3],p[2];} *A;
//t=0:regular array, t=1:boxed
//r:significant dims in d
//d:dims of p
//p is a flexible array member. why [2]? ??!

#define P printf
#define R return

#define V1(f) A f(A w)
#define V2(f) A f(A a,A w)
#define DO(n,x) {I i=0,_n=(n);for(;i<_n;++i){x;}}

I ma(I n){R (I)malloc(n*sizeof(I));}
mv(I*d,I*s,I n){DO(n,d[i]=s[i]);}
I tr(I r,I*d){I z=1;DO(r,z=z*d[i]);R z;} //table rank
A ga(I t,I r,I*d){A z=(A)ma(5+tr(r,d));z->t=t,z->r=r,mv(z->d,d,r);R z;}
A cp(A w){I n=tr(w->r,w->d);
    A z=(A)ma(5+ (n?n:1));z->t=w->t;z->r=0;*z->p=*w->p;
    DO(n,z->p[i]=w->p[i]);
    R z;
}

//allow a or w to be scalar
#define OP(op) \
    if(a->r && w->r) \
        DO(n,z->p[i]=a->p[i] op w->p[i]) \
    else if(w->r) \
        DO(n,z->p[i]=*a->p op w->p[i]) \
    else if(a->r) \
        { n=tr(a->r,a->d); z=ga(0,a->r,a->d); DO(n,z->p[i]=a->p[i] op *w->p) } \
    else *z->p = *a->p op *w->p; \
    R z;

V1(iota){I n=*w->p;A z=ga(0,1,&n);DO(n,z->p[i]=i);R z;}
V2(plus){
    //printf("plus %d %d\n", *a->p, *w->p);
    I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OP(+)
}
V2(minus){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OP(-)
}
V2(times){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OP(*)
}
V2(divide){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OP(/)
}
V2(modulus){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    A t=a;a=w;w=t; //swap args: w%a
    OP(%)
}
V1(absolute){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    DO(n,z->p[i]=abs(w->p[i]));R z;
}
V2(and){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OP(&&)
}
V2(or){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OP(||)
}
V1(not){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    DO(n,z->p[i]=!w->p[i]);R z;}

V2(from){I r=w->r-1,*d=w->d+1,n=tr(r,d);
    A z=ga(w->t,r,d);mv(z->p,w->p+(n**a->p),n);R z;}
V1(box){A z=ga(1,0,0);*z->p=(I)w;R z;}
V1(unbox){R (A)*w->p;}
V2(cat){I an=tr(a->r,a->d),wn=tr(w->r,w->d),n=an+wn;
    A z=ga(w->t,1,&n);mv(z->p,a->p,an);mv(z->p+an,w->p,wn);R z;}
V2(find){}
V2(reshape){I r=a->r?*a->d:1,n=tr(r,a->p),wn=tr(w->r,w->d);
    A z=ga(w->t,r,a->p);mv(z->p,w->p,wn=n>wn?wn:n); //wn=min(wn,n) 
    if(n-=wn)mv(z->p+wn,z->p,n);R z;}
V1(shape){A z=ga(0,1,&w->r);mv(z->p,w->d,w->r);R z;}
V1(identity){R w;}
V1(size){A z=ga(0,1,&w->r);mv(z->p,w->d,w->r);R z;}

A reduce(A w,I f);
A dot(A a,I f,I g,A w);

pi(i){P("%d ",i);}
nl(){P("\n");}
pr(A w){I r=w->r,*d=w->d,n=tr(r,d); DO(r,pi(d[i]));nl();
    if(w->t)DO(n,P("< ");pr((A)w->p[i]))else DO(n,pi(w->p[i]));nl();}

I st[26];
qp(a){R a>='a'&&a<='z';}

enum   {         PLUS=1, FROM, IOTA, BOX, SHAPE,   CAT, UNBOX, MINUS, TIMES, DIVIDE, BAR,      AND, OR,  NOT, SLASH,   DOT, NV  };
C vt[]={         '+',    '{',  '~', '<', '#',      ',', '>',    '-',   '*',  '%',    '|',     '&',  '^', '`', '/',     '.', 0 };
A(*vd[])(A,A)={0,plus,   from, find, 0,   reshape, cat, 0,     minus, times, divide, modulus,  and, or,  0,    0,      0,   0},
 (*vm[])(A)={0,identity, size, iota, box, shape,   0,   unbox, 0,     0,     0,      absolute, 0,   0,   not,  0,      0,   0};
A(*od[])(A,I,I,A)={0,0,  0,    0,    0,   0,       0,   0,     0,     0,     0,      0,        0,   0,   0,    0,      dot, 0},
 (*om[])(A,I)={0, 0,     0,    0,    0,   0,       0,   0,     0,     0,     0,      0,        0,   0,   0,    reduce, 0,   0};
I vid[]={0,       '0',   0,    0,    0,   0,       0,   0,     0,     '1',   1,      2,        '1', '0',  0,    0,      0,   0};
qv(unsigned a){R a<'a'&&a<NV&&(vd[a]||vm[a]);}
qo(unsigned a){R a<'a'&&a<NV&&(od[a]||om[a]);}

A reduce(A w,I f){
    I r=w->r,*d=w->d,n=tr(r,d);
    //printf("reduce: %u %u\n",w,f); pr(w);
    A z=(A)noun(vid[f]);
    //printf("n = %d, z = %d, *w->p = %d\n", n, *z->p, *w->p); fflush(0);
    if (w->r){
        A ind = noun('0');;
        DO(n,*ind->p=i;z=(*vd[f])(z,from(ind,w));/*printf("z = %d\n", *z->p);*/);
    } else {
        z=(*vd[f])(z,w);
    }
    R z;
}
A dot(A a,I f,I g,A w){
}

A ex(I*e){I a=*e,w=e[1]; I d=w;
EX:
    if(qp(a)&&w==BOX)R st[a-'a']=ex(e+2);
    if (qv(a)){I m=a;
        if (qo(w)){
            R (*om[w])(ex(e+2),a);
        }
        if (vm[m]==0){
            a=noun(vid[m]);
            R (*vd[m])(a,ex(e+1));
        }
        R (*vm[m])(ex(e+1));
    }
    if (w){
        if (qv(w)){

            if (qo(e[2])){
                R (*od[e[2]])(a,w,e[3],ex(e+4));
            }
            w=ex(e+2);
            if (qp(a)) a=st[a-'a'];
            R (*vd[d])(a,w);
        }
        if (qp(a)) a=st[a-'a'];

        if (w==' '){ //e:"a bc..." A=cat(a,b)
            A _d,_w;
            w=e[2];
            if (w){
                if (qp(w)) w=cp(st[w-'a']);
                e+=2;                  //e:"Ac..."
                d=e[1];
                while (d&&!qv(d)&&d!=' '){
                    if (qp(d)) cp(d=st[d-'a']);
                    _d=d;
                    if (_d->r==0){
                        _w=w;
                        *_d->p+=*_w->p*10;
                        w=d;
                        ++e;
                        d=e[1];
                        continue;
                    }
                    break;
                }
                a=cat((A)a,(A)w);
                d=w=e[1];
                goto EX;
            }
        } else {
            A _a=a, _w=w;
            if (qp(w)) cp(w=st[w-'a']);
            if (_a->r==0 && _w->r==0){
                *_w->p+=*_a->p*10;
                a=w;
                ++e;
                d=w=e[1];
                goto EX;
            }
        }
    }
    R (A)a; 
}

noun(c){A z;if(c<'0'||c>'9')R 0;z=ga(0,0,0);*z->p=c-'0';R z;}
verb(c){I i=0;for(;vt[i];)if(vt[i++]==c)R i;R 0;}
I*wd(C*s){I a,n=strlen(s),*e=ma(n+1);C c;
    DO(n,e[i]=(a=noun(c=s[i]))?a:(a=verb(c))?a:c);
    e[n]=0;R e;}

main(){C s[99];
    //vid[1]=vid[6]=vid[8]=noun('0');vid[9]=vid[10]=noun('1');vid[11]=noun('2');vid[12]=noun('1');vid[13]=noun('0');
    while(gets(s))pr(ex(wd(s)));}
