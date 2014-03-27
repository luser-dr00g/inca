#include <math.h>
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

V1(box){A z=ga(1,0,0);*z->p=(I)w;R z;}
V1(unbox){R (A)*w->p;}

//allow a or w to be scalar
#define OP(op,func) \
    if(a->t && w->t) \
        z = (A)box(func(unbox(a),unbox(w))); \
    else if(a->t) \
        z = (A)func(unbox(a),w); \
    else if(w->t) \
        z = (A)func(a,unbox(w)); \
    else if(a->r && w->r) \
        DO(n,z->p[i]=a->p[i] op w->p[i]) \
    else if(w->r) \
        DO(n,z->p[i]=*a->p op w->p[i]) \
    else if(a->r) \
        { n=tr(a->r,a->d); z=ga(0,a->r,a->d); DO(n,z->p[i]=a->p[i] op *w->p) } \
    else *z->p = *a->p op *w->p; \
    R z;

#define OPF(op,func) \
    if(a->t && w->t) \
        z = (A)box(func(unbox(a),unbox(w))); \
    else if(a->t) \
        z = (A)func(unbox(a),w); \
    else if(w->t) \
        z = (A)func(a,unbox(w)); \
    else if(a->r && w->r) \
        DO(n,z->p[i]=op(a->p[i], w->p[i])) \
    else if(w->r) \
        DO(n,z->p[i]=op(*a->p, w->p[i])) \
    else if(a->r) \
        { n=tr(a->r,a->d); z=ga(0,a->r,a->d); DO(n,z->p[i]=op(a->p[i], *w->p)) } \
    else *z->p =op( *a->p, *w->p); \
    R z;

V1(iota){I n=*w->p;A z=ga(0,1,&n);DO(n,z->p[i]=i);R z;}

V2(plus){
    //printf("plus %d %d\n", *a->p, *w->p);
    I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OP(+,plus)
}
V2(minus){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OP(-,minus)
}
V2(times){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OP(*,times)
}
V2(power){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OPF(pow,power)
}
V2(divide){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OP(/,divide)
}
V2(modulus){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    A t=a;a=w;w=t; //swap args: w%a
    OP(%,modulus)
}
V1(absolute){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    DO(n,z->p[i]=abs(w->p[i]));R z;
}
V2(and){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OP(&&,and)
}
V2(or){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OP(||,or)
}
V1(not){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    DO(n,z->p[i]=!w->p[i]);R z;}
V2(equal){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OP(=,equal)
}

V2(from){I r=w->r-1,*d=w->d+1,n=tr(r,d);
    A z=ga(w->t,r,d);mv(z->p,w->p+(n**a->p),n);R z;}
V2(cat){I an=tr(a->r,a->d),wn=tr(w->r,w->d),n=an+wn;
    A z=ga(w->t,1,&n);mv(z->p,a->p,an);mv(z->p+an,w->p,wn);R z;}
V2(rowcat){A z;I an;
    switch(w->r){
    case 0:z=ga(0,1,(I[]){1});*z->p=(I)w;w=z;
    case 1:z=ga(0,2,(I[]){2,w->d[0]});
           mv(z->p,a->p,an=tr(a->r,a->d));
           mv(z->p+an,w->p,tr(w->r,w->d)); break;
    default:z=ga(0,w->r,(I[]){w->d[0]+1,w->d[1],w->d[2]});
            mv(z->p,a->p,an=tr(a->r,a->d));
            mv(z->p+an,w->p,tr(w->r,w->d)); break;
    }
    R z;
}
V2(reshape){I r=a->r?*a->d:1,n=tr(r,a->p),wn=tr(w->r,w->d);
    A z=ga(w->t,r,a->p);mv(z->p,w->p,wn=n>wn?wn:n); //wn=min(wn,n) 
    if(n-=wn)mv(z->p+wn,z->p,n);R z;}
V1(shape){A z=ga(0,1,&w->r);mv(z->p,w->d,w->r);R z;}
V1(identity){R w;}
V1(size){A z=ga(0,1,&w->r);mv(z->p,w->d,w->r);R z;}
V2(compress){I an=tr(a->r,a->d),n=0,j=0;
    DO(an,if(a->p[i])++n);
    A z=ga(0,1,&n);
    DO(an,if(a->p[i])z->p[j++]=w->p[i]);
    R z;
}
V2(expand){
    I an=tr(a->r,a->d);
    A z=ga(0,a->r,a->d);
    DO(an,z->p[i]=a->p[i]?w->p[i]:0);
    R z;
}

V2(find){I wn=tr(w->r,w->d);A z=0; I i;
    if(a->r==0){
        for(i=0;i<wn;i++){
            if(*a->p==w->p[i]){
                z=ga(0,0,0);
                *z->p=i;
                R z;
            }
        }
    }else{
        I an=tr(a->r,a->d);
        A ind=(A)noun('0');
        z=ga(0,a->r,a->d);
        for(i=0;i<an;i++){
            *ind->p=i;
            ind=find(from(ind,a),w);
            z->p[i]=*ind->p;
        }
    }
    R z;
}
V1(transpose){I r=w->r,d[3],t;A z;
    if(r==0)R w;
    DO(r,d[i]=w->d[i]);
    if(r==1){r=2;d[1]=1;}              //vector->row matrix
    t=d[0]; d[0]=d[1]; d[1]=t;
    //printf("r=%d,d[0]=%d,d[1]=%d\n",r,d[0],d[1]);
    z=ga(0,r,d);
    int i,j;
    for(i=0;i<d[0];i++)
        for(j=0;j<d[1];j++)
            z->p[i*d[1]+j]=w->p[j*d[0]+i];
    R z;
}
V1(reverse){I n=tr(w->r,w->d);A z=ga(0,w->r,w->d);
    DO(n,z->p[i]=w->p[n-1-i]);
    R z;
}

A reduce(A w,I f);
A dot(A a,I f,I g,A w);

pi(i){P("%d ",i);}
nl(){P("\n");}
pr(A w){I r=w->r,*d=w->d,n=tr(r,d);I j,k;
    DO(r,pi(d[i]));nl();
    if(w->t)DO(n,P("< ");pr((A)w->p[i]))else
    switch(r){
    case 0: pi(*w->p);nl();break;
    case 1: DO(n,pi(w->p[i]));nl(); break;
    case 2: DO(d[0], j=i;DO(d[1],pi(w->p[j*d[1]+i]));nl();); break;
    case 3: DO(d[0], k=i;DO(d[1], j=i;DO(d[2],pi(w->p[(k*d[1]+j)*d[2] +i]));nl();)nl();nl();); break;
    }
}

I st[28];
qp(a){R a>='_'&&a<='z';}

enum   {         PLUS=1, LBRACE, TILDE, LANG, HASH,    COMMA, RANG, MINUS, STAR, PERCENT, BAR,
                 AND, CARET,  BANG, SLASH,    DOT, BACKSLASH, QUOTE,     AT,  EQUAL, SEMI, NV};
C vt[]={         '+',    '{',    '~',   '<',  '#',     ',',   '>',  '-',   '*',  '%',    '|',
                 '&', '^',    '!',  '/',      '.', '\\',      '\'',      '@', '=',   ';',  0};
A(*vd[])(A,A)={0,plus,   from,   find,  0,    reshape, cat,   0,    minus, power, divide, modulus,
                 and, or,     0,    compress, times, expand,  0,         0,   equal, rowcat, 0},
 (*vm[])(A)={0,identity, size,   iota,  box,  shape,   0,     unbox, 0,     0,     0,      absolute,
                 0,   0,      not,  0,        0,   0,         transpose, reverse, 0, 0,    0};
A(*od[])(A,I,I,A)={0,0,  0,      0,     0,    0,       0,     0,    0,     0,     0,      0,
                 0,   0,      0,    0,        dot, 0,         0,         0,   0,     0,    0},
 (*om[])(A,I)={0, 0,     0,      0,     0,    0,       0,     0,    0,     0,     0,      0,
                 0,   0,      0,    reduce,   0,   0,         0,         0,   0,     0,    0};
I vid[]={0,       '0',   0,      0,     0,    0,       '0',   0,    '0',   '2',   '1',    0,
                 '1', '0',    0,    0,        '0', 0,         0,         0,   0,     0,    0};
qv(unsigned a){R a<'_'&&a<NV&&(vd[a]||vm[a]);}
qo(unsigned a){R a<'_'&&a<NV&&(od[a]||om[a]);}

A reduce(A w,I f){
    I r=w->r,*d=w->d,n=tr(r,d);
    //printf("reduce: %u %u\n",w,f); pr(w);
    A z=(A)noun(vid[f]);
    //printf("n = %d, z = %d, *w->p = %d\n", n, *z->p, *w->p); fflush(0);
    if (w->r){
        A ind = (A)noun('0');;
        *ind->p = n-1;
        z = from(ind,w);
        DO(n-1,*ind->p=n-2-i;z=(*vd[f])(from(ind,w),z);/*printf("z = %d\n", *z->p);*/);
    } else {
        z=(*vd[f])(z,w);
    }
    R z;
}
A dot(A a,I f,I g,A w){
    R reduce((*vd[g])(a,transpose(w)),f);
}

A ex(I*e){I a=*e,w=e[1]; I d=w;
    //if (a==0){A z=ga(0,0,0);*z->p=0;R z;}
    {int i;for(i=0;e[i];i++)printf("%d ",e[i]);printf("\n");} // dump command-"string"
EX:
    if(qp(a)&&w==LANG)R (A)(st[a-'_']=(I)ex(e+2)); //use '<' for assignment
    if (qv(a)){I m=a;
        if (qo(w)){
            R (*om[w])(ex(e+2),a);
        }
        if (vm[m]==0&&vid[m]){
            a=noun(vid[m]);
            R (*vd[m])((A)a,ex(e+1));
        }
        R (*vm[m])(ex(e+1));
    }
    if (w){
        if (qv(w)){

            if (qo(e[2])){
                R (*od[e[2]])((A)a,w,e[3],(A)ex(e+4));
            }
            w=(I)ex(e+2);
            if (qp(a)) a=st[a-'_'];
            R (*vd[d])((A)a,(A)w);
        }
        if (qp(a)) a=st[a-'_'];

        if (w==' '){ //e:"a bc..." A=cat(a,b)
            A _d,_w;
            w=e[2];
            if (w){
                if (qp(w)) w=(I)cp((A)st[w-'_']);
                e+=2;                  //e:"Ac..."
                d=e[1];
                while (d&&!qv(d)&&d!=' '){
                    if (qp(d)) cp((A)(d=st[d-'_']));
                    _d=(A)d;
                    if (_d->r==0){
                        _w=(A)w;
                        *_d->p+=*_w->p*10;
                        w=d;
                        ++e;
                        d=e[1];
                        continue;
                    }
                    break;
                }
                a=(I)cat((A)a,(A)w);
                d=w=e[1];
                goto EX;
            }
        } else {
            A _a=(A)a, _w=(A)w;
            if (qp(w)) cp((A)(w=st[w-'_']));
            if (_a->r==0 && _w->r==0){
                *_w->p+=*_a->p*10;
                a=w;
                ++e;
                d=w=e[1];
                goto EX;
            }
        }
    }
    if (qp(a)) a=st[a-'_'];
    if (a==0)R noun('0');
    R (A)a; 
}

noun(c){A z;if(c<'0'||c>'9')R 0;z=ga(0,0,0);*z->p=c-'0';R (I)z;}
verb(c){I i=0;for(;vt[i];)if(vt[i++]==c)R i;R 0;}
I*wd(C*s){I a,n=strlen(s),*e=(I*)ma(n+1);C c;
    DO(n,e[i]=(a=noun(c=s[i]))?a:(a=verb(c))?a:c);
    e[n]=0;R e;}

main(){C s[99];
    while(putchar('\t'),gets(s))pr((A)(st[0]=(I)ex(wd(s))));}  // st['_'-'_']
