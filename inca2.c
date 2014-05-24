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
typedef struct a{I k,t,n,r,d[3],p[1];}*ARC;
#define AK(a) (a->k) /*offset of ravel*/
#define AT(a) (a->t) /*type*/
#define AN(a) (a->n) /*# of atoms in ravel*/
#define AR(a) (a->r) /*rank*/
#define AD(a) (a->d) /*dims*/
#define AV(a) (a->p) /*values (ravel)*/

#define R return
#define V1(f) ARC f(ARC w)
#define V2(f) ARC f(ARC a,ARC w)
#define DO(n,x) {I i=0,_n=(n);for(;i<_n;++i){x;}}

I *ma(n){R(I*)malloc(n*sizeof(I));}
mv(d,s,n)I *d,*s;{DO(n,d[i]=s[i]);}
tr(r,d)I *d;{I z=1;DO(r,z=z*d[i]);R z;}
ARC ga(t,r,d)I *d;{I n;
    ARC z=(ARC)ma((sizeof(*z)/sizeof(I))+(n=tr(r,d)));
    AT(z)=t,AR(z)=r,AN(z)=n,mv(AD(z),d,r);
    R z;}
V1(iota){
    I n=*AV(w);ARC z=ga(INT,1,&n);DO(n,AV(z)[i]=i);R z;}
V2(plus){
    I r=AR(w),*d=AD(w),n=AN(w);ARC z=ga(INT,r,d);
    DO(n,AV(z)[i]=AV(a)[i]+AV(w)[i]);R z;}
V2(from){
    I r=AR(w)-1,*d=AD(w)+1,n=tr(r,d);
    ARC z=ga(AT(w),r,d);mv(AV(z),AV(w)+(n**AV(a)),n);R z;}
V1(box){
    ARC z=ga(BOX,0,0);*AV(z)=(I)w;R z;}
V2(cat){
    I an=AN(a),wn=AN(w),n=an+wn;
    ARC z=ga(AT(w),1,&n);
    mv(AV(z),AV(a),an);mv(AV(z)+an,AV(w),wn);R z;}
V2(find){}
V2(rsh){
    I r=AR(a)?*AD(a):1,n=tr(r,AV(a)),wn=AN(w);
    ARC z=ga(AT(w),r,AV(a));
    mv(AV(z),AV(w),wn=n>wn?wn:n);
    if(n-=wn)mv(AV(z)+wn,AV(z),n);R z;}
V1(sha){
    ARC z=ga(INT,1,&AR(w));
    mv(AV(z),AD(w),AR(w));R z;}
V1(id){R w;}
V1(size){
    ARC z=ga(INT,0,0);*AV(z)=AR(w)?*AD(w):1;R z;}

pi(i){printf("%d ",i);}
nl(){printf("\n");}
pr(w)ARC w;{
    I r=AR(w),*d=AD(w),n=AN(w);
    DO(r,pi(d[i]));
    nl();
    if(AT(w)==BOX)DO(n,printf("< ");pr(AV(w)[i]))
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
qp(unsigned a){R  islower(a&255);}
qv(unsigned a){R a<'a';}
ARC ex(e)I *e;{I a=*e;
 if(qp(a)){if(e[1]=='=')R (ARC)(st[a-'a']=(I)ex(e+2));a= st[ a-'a'];}
 R qv(a)?(ftab[a].vm)(ex(e+1)):e[1]?(ftab[e[1]].vd)((ARC)a,ex(e+2)):(ARC)a;}
noun(c){ARC z;if(!isdigit(c))R 0;z=ga(INT,0,0);*AV(z)=c-'0';R (I)z;}
verb(c){I i=1;for(;ftab[i].c;)if(ftab[i++].c==c)R i-1;R 0;}
I *wd(s)C *s;{I a,n=strlen(s),*e=ma(n+1);C c;
 DO(n,e[i]=(a=noun(c=s[i]))?a:(a=verb(c))?a:c);e[n]=0;R e;}

main(){C s[99];while(gets(s))pr(ex(wd(s)));}
