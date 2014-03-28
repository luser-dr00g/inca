#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char C;
typedef intptr_t I;
typedef struct a{I t,r,d[3],p[2];} *A;
//t (type): t=0:regular array, t=1:boxed
//r (rank): significant dims in d
//d (dims): dimensions of p
//p ("physical" data)is a flexible array member. why [2]? ??!

#define V1(f) A f(A w)
#define V2(f) A f(A a,A w)
#define DO(n,x) {I i=0,_n=(n);for(;i<_n;++i){x;}}

I ma(I n){return (I)malloc(n*sizeof(I));} //malloc an array
mv(I*d,I*s,I n){DO(n,d[i]=s[i]);} //copy n ints from s to d
I tr(I r,I*d){I z=1;DO(r,z=z*d[i]);return z;} //table rank (total # of elements in array)
A ga(I t,I r,I*d){A z=(A)ma(5+tr(r,d));z->t=t,z->r=r,mv(z->d,d,r);return z;} //construct(malloc) array with dims
A cp(A w){I n=tr(w->r,w->d); //copy an array structure and contents
    A z=(A)ma(5+ (n?n:1));z->t=w->t;z->r=0;*z->p=*w->p;
    DO(n,z->p[i]=w->p[i]);
    return z;
}

V1(box){A z=ga(1,0,0);*z->p=(I)w;return z;}
V1(unbox){return (A)*w->p;}

//Perform C operation "op" upon left arg 'a' and right arg 'w' (alpha and wmega)
//recursing to the calling function func for boxed args
//allow a or w to be scalar
//nb. ->t is the type field (1==boxed,0==normal)
//    ->r is the "rank" field, how many dimensions of data (0..3)?
//for this macro, the actual value of rank is not important just zero/non-zero.
//zero rank means arg is a scalar and ->p[0] is the value.
//non-zero rank means arg is an array and ->p[0..n] contain the values
//    where n is the total size of the array (Product of dims 0..r)
#define OP(op,func) \
    if(a->t && w->t) \
        z = (A)box(func(unbox(a),unbox(w))); \
    else if(a->t) \
        z = (A)func(unbox(a),w); \
    else if(w->t) \
        z = (A)func(a,unbox(w)); \
    else if(a->r && w->r) \
        DO(n,z->p[i]=(a->p[i]) op (w->p[i])) \
    else if(w->r) \
        DO(n,z->p[i]=(*a->p) op (w->p[i])) \
    else if(a->r) /* w is scalar: allocate z with dims from a */ \
        { n=tr(a->r,a->d); z=ga(0,a->r,a->d); DO(n,z->p[i]=(a->p[i]) op (*w->p)) } \
    else *z->p = (*a->p) op (*w->p); \
    return z;

//Perform C math function "op" upon args a and w
//recursing for boxed args
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
    return z;

/* iota: generate j=0 index vector */
V1(iota){I n=*w->p;A z=ga(0,1,&n);DO(n,z->p[i]=i);return z;}

/* arithmetic/logical functions */
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
    DO(n,z->p[i]=abs(w->p[i]));return z;
}
V2(and){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OP(&&,and)
}
V2(or){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OP(||,or)
}
V1(not){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    DO(n,z->p[i]=!w->p[i]);return z;}
V2(equal){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
    OP(=,equal)
}

/* extract row from matrix or scalar from vector */
V2(from){I r=w->r-1,*d=w->d+1,n=tr(r,d);n=n?n:1;
    A z=ga(w->t,r,d);mv(z->p,w->p+(n**a->p),n);return z;}
/* catenate two arrays (yields a rank=1 vector) */
V2(cat){I an=tr(a->r,a->d),wn=tr(w->r,w->d),n=an+wn;
    A z=ga(w->t,1,&n);mv(z->p,a->p,an);mv(z->p+an,w->p,wn);return z;}
/* catenate two "rows" FIXME: doesn't work if w is rank>1 */
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
    return z;
}
/* use data in a as new dims for array containing data from w */
V2(reshape){I r=a->r?*a->d:1,n=tr(r,a->p),wn=tr(w->r,w->d);
    A z=ga(w->t,r,a->p);mv(z->p,w->p,wn=n>wn?wn:n);    //wn=min(wn,n) 
    if(n-=wn)mv(z->p+wn,z->p,n);return z;}
/* return the dims of w */
V1(shape){A z=ga(0,1,&w->r);mv(z->p,w->d,w->r);return z;}
/* return w unmolested */
V1(identity){return w;}
/* suspiciously similar to shape */
V1(size){A z=ga(0,1,&w->r);mv(z->p,w->d,w->r);return z;}
/* catenate elements of w selected by nonzero elements of a */
V2(compress){I an=tr(a->r,a->d),n=0,j=0;
    DO(an,if(a->p[i])++n);
    A z=ga(0,1,&n);
    DO(an,if(a->p[i])z->p[j++]=w->p[i]);
    return z;
}
/* fill array with 0 or element from w by nonzero elements of a */
V2(expand){
    I an=tr(a->r,a->d);
    A z=ga(0,a->r,a->d);
    DO(an,z->p[i]=a->p[i]?w->p[i]:0);
    return z;
}

/* dyadic iota: find index of a in w */
V2(find){I wn=tr(w->r,w->d);A z=0; I i;
    if(a->r==0){
        for(i=0;i<wn;i++){
            if(*a->p==w->p[i]){
                z=ga(0,0,0);
                *z->p=i;
                return z;
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
    return z;
}
/* exchange dims 0 and 1, possibly promoting vector to row-matrix first */
V1(transpose){I r=w->r,d[3],t;A z;
    if(r==0)return w;
    DO(r,d[i]=w->d[i]);
    if(r==1){r=2;d[1]=d[0];d[0]=1;}              //vector->row matrix
    t=d[0]; d[0]=d[1]; d[1]=t;
    //printf("r=%d,d[0]=%d,d[1]=%d\n",r,d[0],d[1]);
    z=ga(0,r,d);
    int i,j;
    for(i=0;i<d[0];i++)
        for(j=0;j<d[1];j++)
            z->p[i*d[1]+j]=w->p[j*d[0]+i];
    return z;
}
/* reverse elements in w */
V1(reverse){I n=tr(w->r,w->d);A z=ga(0,w->r,w->d);
    DO(n,z->p[i]=w->p[n-1-i]);
    return z;
}

A reduce(A w,I f);
A dot(A a,I f,I g,A w);

pi(i){printf("%d ",i);}
nl(){printf("\n");}
pr(A w){I r=w->r,*d=w->d,n=tr(r,d);I j,k;
    DO(r,pi(d[i]));nl();
    if(w->t)DO(n,printf("< ");pr((A)w->p[i]))else
    switch(r){
    case 0: pi(*w->p);nl();break;
    case 1: DO(n,pi(w->p[i]));nl(); break;
    case 2: DO(d[0], j=i;DO(d[1],pi(w->p[j*d[1]+i]));nl();); break;
    case 3: DO(d[0], k=i;DO(d[1], j=i;DO(d[2],pi(w->p[(k*d[1]+j)*d[2] +i]));nl();)nl();nl();); break;
    }
}

I st[28];
qp(a){return a>='_'&&a<='z';}

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
I vid[]={0,      '0',    0,      0,     0,    0,       '0',   0,    '0',   '2',   '1',    0,
                 '1', '0',    0,    0,        '0', 0,         0,         0,   0,     0,    0};
qv(unsigned a){return a<'_'&&a<NV&&(vd[a]||vm[a]);}
qo(unsigned a){return a<'_'&&a<NV&&(od[a]||om[a]);}

/* perform reduction over array w using function f */
A reduce(A w,I f){
    I r=w->r,*d=w->d,n=tr(r,d);
    //printf("reduce(%c): %u %u\n",vt[f-1],w,f); pr(w);
    A z=(A)noun(vid[f]);
    //printf("n = %d, z = %d, *w->p = %d\n", n, *z->p, *w->p); fflush(0);
    if (w->r){
        A ind = (A)noun('0');;
        *ind->p = n-1;
        //printf("*ind->p=%d\n",*ind->p);
        z = from(ind,w);
        //printf("z=%d,*z->p=%d\n",z,*z->p);
        //printf("w->p[n-1]=%d\n",w->p[n-1]);
        //z = w->p[n-1];
        DO(n-1,*ind->p=n-2-i;z=(*vd[f])(from(ind,w),z);/*printf("z = %d\n", *z->p);*/);
    } else {
        z=(*vd[f])(z,w);
    }
    return z;
}
/* perform general matrix multiplication Af.gW ::== f/Ag'W
   f-reduce rows of A g-function transpose of W
 */
A dot(A a,I f,I g,A w){
    return reduce((*vd[g])(a,transpose(w)),f);
    //return reduce((*vd[g])(a,w),f);
}

/* execute an encoded expression, an "int" string, terminated by a zero int.
 */
A ex(I*e){I a=*e,w=e[1]; I d=w;
    //{int i;for(i=0;e[i];i++)printf("%d ",e[i]);printf("\n");} // dump command-"string"
EX:
    /* if a is a variable followed by Left ANGle bracket, assign to variable result of ex(remainder) */
    if(qp(a)&&w==LANG)return (A)(st[a-'_']=(I)ex(e+2)); //use '<' for assignment

    if (qv(a)){I m=a;  /* if a is a verb, it is in monadic position, so call it 'm' */
        if (qo(w)){    /* if w is an operator, */
            return (*om[w])(ex(e+2),a);  /* call operator function with function a on result of ex(rem)*/
        }
        if (vm[m]==0&&vid[m]){  /* if no monadic verb, but there is a verb "identity" element, */
            a=noun(vid[m]);     /* load the identity element */
            return (*vd[m])((A)a,ex(e+1));  /* call dyadic verb with w=ex(rem) */
        }
        return (*vm[m])(ex(e+1));  /* call monadic verb with w=ex(rem) */
    }

    if (w){  /* both ifs have failed. so w is not assignment and a is not a function */
        if (qv(w)){  /* if w is a verb, */

            if (qo(e[2])){  /* if followed by an operator */
                w=(I)ex(e+4);   /* w=ex(rem) */
                if (qp(a)) a=st[a-'_'];   /* w may have assigned to variable a; if so, load it */
                return (*od[e[2]])((A)a,d,e[3],(A)w);  /* call dyadic operator */
            }
            w=(I)ex(e+2);   /* ::not followed by an operator, w=ex(rem) */
            //printf("lookup a\n");
            if (qp(a)) a=st[a-'_'];  /* if a is a var, load it now */
            return (*vd[d])((A)a,(A)w);  /* call dyadic function */
        }
        if (qp(a)) a=st[a-'_'];  /* a is not function, w is not a function */

        if (w==' '){ //e:"a bc..." A=cat(a,b) concatenate space-delimited integer vector
            A _d,_w;
            w=e[2];
            if (w){
                if (qp(w)) w=(I)cp((A)st[w-'_']);
                e+=2;                  //e:"Ac..."
                d=e[1];
                while (d&&!qv(d)&&d!=' '){  // accumulate integer
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
                a=(I)cat((A)a,(A)w);  // cat new integer w into integer vector a
                d=w=e[1];
                goto EX;
            }
        } else {  // not verb, not a space
            A _a=(A)a, _w=(A)w;  // accumulate integer
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
    if (qp(a)) a=st[a-'_'];  /* a not a function, w is zero (end of string). load var a if a is a var */
    if (a==0)return (A)noun('0');  /* if somehow a is zero (the end of string), return scalar zero */
    return (A)a;             /* return a, whatever it is now, a non-null "something", hopefully */
}

/* construct an integer string from from command string */
noun(c){A z;if(c<'0'||c>'9')return 0;z=ga(0,0,0);*z->p=c-'0';return (I)z;}
verb(c){I i=0;for(;vt[i];)if(vt[i++]==c)return i;return 0;}
I*wd(C*s){I a,n=strlen(s),*e=(I*)ma(n+1);C c;
    DO(n,e[i]=(a=noun(c=s[i]))?a:(a=verb(c))?a:c);
    e[n]=0;return e;}

/* var('_')=REPL */
main(){C s[999];
    //printf("sizeof(intptr_t)=%u\n",sizeof(intptr_t));
    while(putchar('\t'),gets(s))pr((A)(st[0]=(I)ex(wd(s))));}  // st['_'-'_']
