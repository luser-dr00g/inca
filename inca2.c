#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum types { NUL, CHR, INT, DBL, BOX };
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

I *ma(n){R(I*)malloc(n*sizeof(I));}
mv(d,s,n)I *d,*s;{DO(n,d[i]=s[i]);}
tr(r,d)I *d;{I z=1;DO(r,z=z*d[i]);R z;}
ARC ga(t,r,d)I *d;{I n;
    ARC z=(ARC)ma((sizeof(*z)/sizeof(I))+r+(n=tr(r,d))*(t==DBL?2:1));
    AT(z)=t,AR(z)=r,AN(z)=n,AK(z)=sizeof(*z)+r*sizeof(I);
    mv(AD(z),d,r);
    R z;}
V1(iota){
    I n=AT(w)==DBL?(I)*(D*)AV(w):*AV(w);ARC z=ga(INT,1,&n);DO(n,AV(z)[i]=i);R z;}
V2(plus){
    I r=AR(w),*d=AD(w),n=AN(w);
    ARC z;
    switch(AT(a)){
    case INT: switch(AT(w)){
        case INT: z=ga(INT,r,d); DO(n,AV(z)[i]=AV(a)[i]+AV(w)[i]);R z;
        case DBL: z=ga(DBL,r,d); DO(n,((D*)AV(z))[i]=AV(a)[i]+((D*)AV(w))[i]);R z;
        }
    case DBL: switch(AT(w)){
        case INT: z=ga(INT,r,d); DO(n,AV(z)[i]=((D*)AV(a))[i]+AV(w)[i]);R z;
        case DBL: z=ga(DBL,r,d); DO(n,((D*)AV(z))[i]=((D*)AV(a))[i]+((D*)AV(w))[i]);R z;
        }
    }
}
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

struct{ C c; ARC(*vd)(ARC,ARC); ARC(*vm)(ARC); }ftab[]={
    { 0, 0, 0},
    { '+', plus, id },
    { '{', from, size },
    { '~', find, iota },
    { '<', 0, box },
    { '#', rsh, sha },
    { ',', cat, 0 }
};
I st[26];
qp(unsigned a){R a<255&&islower(a);}
qv(unsigned a){
    //R a<'a';
    R a < (sizeof ftab/sizeof*ftab);
}
qo(unsigned a){R 0;}
/*
ARC ex(e)I *e;{I a=*e;
 if(qp(a)){if(e[1]=='=')R (ARC)(st[a-'a']=(I)ex(e+2));a= st[ a-'a'];}
 R qv(a)?(ftab[a].vm)(ex(e+1)):e[1]?(ftab[e[1]].vd)((ARC)a,ex(e+2)):(ARC)a;}
 */
#define VAR(a) (st[a-'a']) /* lookup variable */ 
#define ADV ++e; /* advance expression pointer */ 
#define BB b=e[1]; /* update b */ 
#define CC c=e[2]; /* update c */ 
#define DD d=e[3]; /* update d */ 
#define ABCD ADV BB CC DD 
#define AACD ADV ADV CC DD 

I nmv(I f, I o){        /* new derived monadic verb */ 
}
I ndv(I f, I o, I g){ /* new derived dyadic verb */ 
}
ARC vm(I v, I w){         /* monadic verb handler */ 
    R ftab[v].vm((ARC)w);
}
ARC vd(I v, I a, I w){  /* dyadic verb handler */ 
    R ftab[v].vd((ARC)a,(ARC)w);
}

ARC ex(I *e){ I a=*e,b,c,d; BB CC DD 
    while(a==' '){a=*ABCD} 
    if(qp(a)&&b=='=')R (ARC)(VAR(a)=(I)ex(e+2)); 
mon_verb: 
    if(qv(a)){ 
        while(b==' '){ABCD} 
        if(qo(b)){ 
            a=nmv(a,b); ABCD goto mon_verb; 
        } 
        R vm(a,(I)ex(e+1)); 
    } 
dy_verb: 
    if(b){ 
        if(qv(b)){ 
            while(c==' '){ADV CC DD}
            if(qo(c)){ 
                while(d==' '){ADV DD}
                b=ndv(b,c,d); AACD goto dy_verb; 
            } 
            c=(I)ex(e+2); 
            if(qp(a))a=VAR(a); 
            R vd(b,a,c); 
        } 
        if(qp(a))a=VAR(a);
        if(b==' '){ 
            if(qv(c)){ABCD goto dy_verb;} 
            if(AT((ARC)c)==INT||AT((ARC)c)==DBL){ 
                a=(I)cat((ARC)a,(ARC)c); ADV ABCD goto dy_verb; 
            } 
        } 
    } 
    R (ARC)a; 
} 


ARC scalarI(I i){ARC z=ga(INT,0,0);*AV(z)=i;R z;}
ARC scalarD(D d){ARC z=ga(DBL,0,0);*(D*)AV(z)=d;R z;}
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
