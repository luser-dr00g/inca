#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum types { NUL, CHR, INT, DBL, BOX, FUN };
typedef char C;
typedef intptr_t I;
typedef double D;
typedef struct a{I k,t,n,r,d[0];}*ARC;
#define AK(a) ((a)->k) /*offset of ravel*/
#define AT(a) ((a)->t) /*type*/
#define AN(a) ((a)->n) /*# of atoms in ravel*/
#define AR(a) ((a)->r) /*rank*/
#define AD(a) ((a)->d) /*dims*/
#define AV(a) ((I*)(((C*)(a))+AK(a))) /* values (ravel)*/

#define R return
#define V1(f) ARC f(ARC w)
#define V2(f) ARC f(ARC a,ARC w)
#define DO(n,x) {I i=0,_n=(n);for(;i<_n;++i){x;}}
ARC vm(I v, ARC w);         /* monadic verb handler */ 
ARC vd(I v, ARC a, ARC w);  /* dyadic verb handler */ 
I nommv(I f, I o);        /* new derived monadic verb */ 
I noddv(I f, I o, I g); /* new derived dyadic verb */ 

ARC power(ARC a,I f,ARC w);
ARC fog(ARC a,I f,I g,ARC w);

I *ma(n){R(I*)malloc(n*sizeof(I));}
mv(d,s,n)I *d,*s;{DO(n,d[i]=s[i]);}
tr(r,d)I *d;{I z=1;DO(r,z=z*d[i]);R z;}
ARC ga(t,r,d)I *d;{I n;
    ARC z=(ARC)ma((sizeof(*z)/sizeof(I))+r+(n=tr(r,d))*(t==DBL?2:1));
    AT(z)=t,AR(z)=r,AN(z)=n,AK(z)=sizeof(*z)+r*sizeof(I);
    mv(AD(z),d,r);
    R z;}
ARC scalarI(I i){ARC z=ga(INT,0,0);*AV(z)=i;R z;}
ARC scalarD(D d){ARC z=ga(DBL,0,0);*(D*)AV(z)=d;R z;}

V1(iota){
    I n=AT(w)==DBL?(I)*(D*)AV(w):*AV(w);ARC z=ga(INT,1,&n);DO(n,AV(z)[i]=i);R z;}


#define MATHOP(op) \
    I r=AR(w),*d=AD(w),n=AN(w); ARC z; \
    switch(AT(a)){ \
    case INT: switch(AT(w)){ \
        case INT: z=ga(INT,r,d); DO(n,AV(z)[i]=AV(a)[i] op AV(w)[i]); break; \
        case DBL: z=ga(DBL,r,d); DO(n,((D*)AV(z))[i]=AV(a)[i] op ((D*)AV(w))[i]); break; \
        } break;\
    case DBL: switch(AT(w)){ \
        case INT: z=ga(INT,r,d); DO(n,AV(z)[i]=((D*)AV(a))[i] op AV(w)[i]); break; \
        case DBL: z=ga(DBL,r,d); DO(n,((D*)AV(z))[i]=((D*)AV(a))[i] op ((D*)AV(w))[i]); break; \
        } break; \
    } \
    R z;

V2(plus){    MATHOP(+) }
V2(minus){   MATHOP(-) }
V2(times){   MATHOP(*) }
V2(divide){  MATHOP(/) }
V2(equal){   MATHOP(==) }
V2(and){     MATHOP(&&) }
V2(or){      MATHOP(||) }
V2(less){    MATHOP(<) }
V2(greater){ MATHOP(>) }

V2(from){
    I r=AR(w)-1,*d=AD(w)+1,n=tr(r,d);
    ARC z=ga(AT(w),r,d);mv(AV(z),AV(w)+(n**AV(a)),n*(AT(a)==DBL?2:1));R z;}
V1(box){
    ARC z=ga(BOX,0,0);*AV(z)=(I)w;R z;}
V2(cat){
    I an=AN(a),wn=AN(w),n=an+wn;
    ARC z;
    switch(AT(w)){
    case INT: switch(AT(a)){
        case INT: z=ga(AT(w),1,&n); mv(AV(z),AV(a),an);mv(AV(z)+an,AV(w),wn);R z;
        case DBL: z=ga(AT(a),1,&n);
        mv(AV(z),AV(a),an*2);//mv(AV(z)+an*2,AV(w),wn);R z;
        DO(wn,((D*)AV(z))[an+i]=(D)AV(w)[i])
        R z;
        }
    case DBL: switch(AT(a)){
        case INT: z=ga(AT(w),1,&n);
        //mv(AV(z),AV(a),an*2);
        DO(an,((D*)AV(z))[i]=(D)AV(a)[i]);
        mv(AV(z)+an*2,AV(w),wn*2);R z;
        case DBL:
        z=ga(AT(w),1,&n);
        mv(AV(z),AV(a),an*2);mv(AV(z)+an,AV(w),wn*2);R z;
        }
    }
}
V2(find){}
V2(rsh){
    I r=AR(a)?*AD(a):1,n=tr(r,AV(a)),wn=AN(w);
    ARC z=ga(AT(w),r,AV(a));
    mv(AV(z),AV(w),(wn=n>wn?wn:n)*(AT(w)==DBL?2:1));
    if(n-=wn)mv(AV(z)+wn,AV(z),n*(AT(w)==DBL?2:1));R z;}
V1(sha){
    ARC z=ga(INT,1,&AR(w));
    mv(AV(z),AD(w),AR(w));R z;}
V1(id){R w;}
V1(size){
    ARC z=ga(INT,0,0);
    *AV(z)=AR(w)?*AD(w):1;R z;}

pi(i){printf("%d ",i);}
pd(D d){printf("%f ",d);}
nl(){printf("\n");}
pr(w)ARC w;{
    I r=AR(w),*d=AD(w),n=AN(w);
    DO(r,pi(d[i]));
    nl();
    if(AT(w)==BOX)DO(n,printf("< ");pr(AV(w)[i]))
    else if(AT(w)==DBL)DO(n,pd(((D*)AV(w))[i]))
    else DO(n,pi(AV(w)[i]));
    nl();}

#define FUNCNAME(name,      c,    id,  vd,     vm,     odd, omm,    omd) name,
#define FUNCINFO(name,      c,    id,  vd,     vm,     odd, omm,    omd) \
                {           c,    id,  vd,     vm,     odd, omm,    omd},
#define FTAB(_) \
                _(NOP,      0,    0.0, 0,      0,      0,   0,      0) \
                _(PLUS,    '+',   0.0, plus,   id,     0,   0,      0) \
                _(MINUS,   '-',   0.0, minus,  0,      0,   0,      0) \
                _(TIMES,   '*',   1.0, times,  0,      0,   0,      power) \
                _(PERCENT, '%',   1.0, divide, 0,      0,   0,      0) \
                _(LCURL,   '{',   0.0, from,   size,   0,   0,      0) \
                _(TILDE,   '~',   0.0, find,   iota,   0,   0,      0) \
                _(LANG,    '<',   0.0, less,   box,    0,   0,      0) \
                _(RANG,    '>',   0.0, greater,0,      0,   0,      0) \
                _(HASH,    '#',   0.0, rsh,    sha,    0,   0,      0) \
                _(COMMA,   ',',   0.0, cat,    0,      0,   0,      0) \
                _(AND,     '&',   0.0, and,    0,      fog, 0,      0) \
                _(DOLLAR,  '$',   0.0, or,     0,      0,   0,      0) \
                _(EQUAL,   '=',   0.0, equal,  0,      0,   0,      0) \
/* END FTAB */
enum{FTAB(FUNCNAME) NFUNC};
struct{
        C c; D id;
            ARC(*vd)(ARC,ARC);
                ARC(*vm)(ARC);
                    ARC(*odd)(ARC,I,I,ARC);
                        ARC(*omm)(I,ARC);
                            ARC(*omd)(ARC,I,ARC);
}ftab[]={ FTAB(FUNCINFO) };

ARC power(ARC a,I f,ARC w){
    I n=*AV(w);
    ARC z=scalarD(ftab[f].id);
    DO(n,z=ftab[f].vd(a,z));
    R z;
}
ARC fog(ARC a,I f,I g,ARC w){
    R vm(f,vd(g,a,w));
}

I st[26];
qp(unsigned a){R (a<255) && islower(a);}
qd(unsigned a){R (a>255) && AT((ARC)a)==FUN;}
qv(unsigned a){R qd(a) || ((a<NFUNC) && (ftab[a].vd || ftab[a].vm));}
qo(unsigned a){R (a<NFUNC) && (ftab[a].odd || ftab[a].omm || ftab[a].omd);}
qomm(unsigned a){R (a<NFUNC) && (ftab[a].omm);}
qodd(unsigned a){R (a<NFUNC) && (ftab[a].odd);}
qomd(unsigned a){R (a<NFUNC) && (ftab[a].omd);}

I nommv(I f, I o){        /* new derived monadic verb */ 
    ARC z=ga(FUN,1,(I[]){3});
         /*arity*/
    AV(z)[0]=1; AV(z)[1]=f; AV(z)[2]=o;
    R (I)z;
}
I nomdv(I f, I o){        /* derived dyadic verb of 1 function */
    ARC z=ga(FUN,1,(I[]){3});
         /*arity*/
    AV(z)[0]=2; AV(z)[1]=f; AV(z)[2]=o;
    R (I)z;
}
I noddv(I f, I o, I g){ /* new derived dyadic verb */ 
    ARC z=ga(FUN,1,(I[]){4});
         /*arity*/
    AV(z)[0]=2; AV(z)[1]=f; AV(z)[2]=o; AV(z)[3]=g;
    R (I)z;
}

ARC vm(I v, ARC w){         /* monadic verb handler */ 
    if (qd(v)){ARC d=(ARC)v;
        R ftab[ AV(d)[2] ].omm(AV(d)[1], w);
    }
    if (!ftab[v].vm)
        R ftab[v].vd(scalarD(ftab[v].id), w);
    R ftab[v].vm(w);
}
ARC vd(I v, ARC a, ARC w){  /* dyadic verb handler */ 
    if (qd(v)){ARC d=(ARC)v;
        if (ftab[ AV(d)[2] ].omd)
            R ftab[ AV(d)[2] ].omd(a, AV(d)[1], w);
        R ftab[ AV(d)[2] ].odd(a, AV(d)[1], AV(d)[3], w);
    }
    R ftab[v].vd(a,w);
}

#define VAR(a) (st[a-'a']) /* lookup variable */ 
#define ADV ++e; /* advance expression pointer */ 
#define BB b=e[1]; /* update b */ 
#define CC c=e[2]; /* update c */ 
#define DD d=e[3]; /* update d */ 
#define ABCD ADV BB CC DD 
#define AACD ADV ADV CC DD 

ARC ex(I *e){ I a=*e,b,c,d; BB CC DD 
    while(a==' '){a=*ABCD} 
    if(qp(a)&&b==LANG)R (ARC)(VAR(a)=(I)ex(e+2)); 
mon_verb: 
    if(qv(a)){ 
        while(b==' '){ABCD} 
        if(qo(b)){ a=nommv(a,b); ABCD goto mon_verb; } 
        R vm(a,ex(e+1));
    } 
dy_verb: 
    if(b){ 
        if(qv(b)){ 
            while(c==' '){ADV CC DD}
            if(qo(c)){ 
                while(d==' '){ADV DD}
                if (qomd(c)){
                    b=nomdv(b,c); ADV CC DD goto dy_verb;
                }
                b=noddv(b,c,d); AACD goto dy_verb;
            }
            c=(I)ex(e+2); 
            if(qp(a))a=VAR(a); 
            R vd(b,(ARC)a,(ARC)c);
        } 
        if(qp(a))a=VAR(a);
        if(b==' '){  // space-delimited vector
            if(qv(c)){ABCD goto dy_verb;} 
            if(AT((ARC)c)==INT||AT((ARC)c)==DBL){ 
                a=(I)cat((ARC)a,(ARC)c); ADV ABCD goto dy_verb;
            }
        }
        R (ARC)a;
    } 
    if(qp(a))a=VAR(a);
    R (ARC)a;
}


noun(c){ARC z;if(!isdigit(c))R 0;z=ga(INT,0,0);*AV(z)=c-'0';R (I)z;}
verb(c){I i=1;for(;ftab[i].c;)if(ftab[i++].c==c)R i-1;R 0;}
//I *wd(s)C *s;{I a,n=strlen(s),*e=ma(n+1);C c; DO(n,e[i]=(a=noun(c=s[i]))?a:(a=verb(c))?a:c);e[n]=0;R e;}
I *wd(C *s){
    I a,n=strlen(s),*e=ma(n+1),i,j;C c,*rem;long ll;
    for(i=0,j=0;c=s[i];i++,j++){
        if(isdigit(c)){
            ll=strtol(s+i,&rem,10); //printf("%d:%s\n", ll, rem);
            if(*rem=='.'){D d;
                d=strtod(s+i,&rem); //printf("%f:%s\n", d, rem);
                a=(I)scalarD(d);
                s=rem;i=-1; //adjust pointer+index
            } else {
                a=(I)scalarI(ll);
                s=rem;i=-1;
            }
        } else {
            if(!(a=verb(c)))
                a=c;
        }
        e[j]=a;
    }
    e[j]=0;
    R e;
}

main(){C s[99];while(gets(s))pr(ex(wd(s)));}
