#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char C;
typedef intptr_t INT;
typedef struct a{INT t,r,d[3],p[2];}*ARC;

#define R return
#define V1(f) ARC f(ARC w)
#define V2(f) ARC f(ARC a,ARC w)
#define DO(n,x) {INT i=0,_n=(n);for(;i<_n;++i){x;}}

INT *ma(n){R(INT*)malloc(n*4);}
mv(d,s,n)INT *d,*s;{DO(n,d[i]=s[i]);}
tr(r,d)INT *d;{INT z=1;DO(r,z=z*d[i]);R z;}
ARC ga(t,r,d)INT *d;{ARC z=(ARC)ma(5+tr(r,d));z->t=t,z->r=r,mv(z->d,d,r);
 R z;}
V1(iota){INT n=*w->p;ARC z=ga(0,1,&n);DO(n,z->p[i]=i);R z;}
V2(plus){INT r=w->r,*d=w->d,n=tr(r,d);ARC z=ga(0,r,d);
 DO(n,z->p[i]=a->p[i]+w->p[i]);R z;}
V2(from){INT r=w->r-1,*d=w->d+1,n=tr(r,d);
 ARC z=ga(w->t,r,d);mv(z->p,w->p+(n**a->p),n);R z;}
V1(box){ARC z=ga(1,0,0);*z->p=(INT)w;R z;}
V2(cat){INT an=tr(a->r,a->d),wn=tr(w->r,w->d),n=an+wn;
 ARC z=ga(w->t,1,&n);mv(z->p,a->p,an);mv(z->p+an,w->p,wn);R z;}
V2(find){}
V2(rsh){INT r=a->r?*a->d:1,n=tr(r,a->p),wn=tr(w->r,w->d);
 ARC z=ga(w->t,r,a->p);mv(z->p,w->p,wn=n>wn?wn:n);
 if(n-=wn)mv(z->p+wn,z->p,n);R z;}
V1(sha){ARC z=ga(0,1,&w->r);mv(z->p,w->d,w->r);R z;}
V1(id){R w;}
V1(size){ARC z=ga(0,0,0);*z->p=w->r?*w->d:1;R z;}
pi(i){printf("%d ",i);}
nl(){printf("\n");}
pr(w)ARC w;{INT r=w->r,*d=w->d,n=tr(r,d);DO(r,pi(d[i]));nl();
 if(w->t)DO(n,printf("< ");pr(w->p[i]))else DO(n,pi(w->p[i]));nl();}

C vt[]="+{~<#,";
ARC(*vd[])()={0,plus,from,find,0,rsh,cat},
 (*vm[])()={0,id,size,iota,box,sha,0};
INT st[26];
qp(unsigned a){R  a>='a'&&a<='z';}
qv(unsigned a){R a<'a';}
ARC ex(e)INT *e;{INT a=*e;
 if(qp(a)){if(e[1]=='=')R (ARC)(st[a-'a']=(INT)ex(e+2));a= st[ a-'a'];}
 R qv(a)?(*vm[a])(ex(e+1)):e[1]?(*vd[e[1]])(a,ex(e+2)):(ARC)a;}
noun(c){ARC z;if(c<'0'||c>'9')R 0;z=ga(0,0,0);*z->p=c-'0';R (INT)z;}
verb(c){INT i=0;for(;vt[i];)if(vt[i++]==c)R i;R 0;}
INT *wd(s)C *s;{INT a,n=strlen(s),*e=ma(n+1);C c;
 DO(n,e[i]=(a=noun(c=s[i]))?a:(a=verb(c))?a:c);e[n]=0;R e;}

main(){C s[99];while(gets(s))pr(ex(wd(s)));}
