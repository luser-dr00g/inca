#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERRORS(_) _(SETJMP_INIT) _(ABORT) _(RANK) _(RANGE) _(LENGTH) _(FUNCTION) _(OPERATOR) _(TYPE)
#define BARE(_) _ ,
#define STR(x) #x ,
enum errnames { ERRORS(BARE) };
char *errstr[] = { ERRORS(STR) };
jmp_buf mainloop;

enum types { NUL, CHR, INT, DBL, BOX, FUN, OPR, FIL, NTYPES };
/* TPAIR() combines types into a single value that can be switch()ed */
#define TPAIR(a,b) ((a)*NTYPES+(b))
typedef char     C;
typedef intptr_t I;
typedef double   D;
typedef struct a{I x,f,k,t,n,r,d[0];} *ARC; /* array header */
#define AX(a) ((a)->x) /* allocation metadata (gc) */
#define AF(a) ((a)->f) /* flags */
#define AK(a) ((a)->k) /* offset of ravel. sizeof(struct a)+sizeof(I)*AR(a) */
#define AT(a) ((a)->t) /* type */
#define AN(a) ((a)->n) /* # of atoms in ravel */
#define AR(a) ((a)->r) /* rank */
#define AD(a) ((a)->d) /* dims */
/* values float *after* variable-length dims */
#define AV(a) ((I*)(((C*)(a))+AK(a))) /* values (ravel) */
#define AZ(a) (AT(a)==DBL?sizeof(D):AT(a)==CHR?sizeof(C):sizeof(I)) /* element size */
#define FL_ASSN 1 /* ARC was just assigned */

#define SWITCH(x,y) switch(x){ default: y }
#define CASE break;case
#define DO(n,x) {I i=0,_n=(n);for(;i<_n;++i){x;}}
#define R return
#define V1(f) ARC f(ARC w)          /* monadic verb signature */
#define V2(f) ARC f(ARC a,ARC w)    /* dyadic verb signature */
ARC vm(I v, ARC w);                 /* monadic verb handler */ 
ARC vd(I v, ARC a, ARC w);          /* dyadic verb handler */ 
I nommv(I f, I o);                  /*new operator monadic (, yielding) monadic verb */
I noddv(I f, I o, I g);             /*new operator dyadic  (, yielding) dyadic verb */
ARC ex(I *e);
I *wd(C *s);

ARC cfile; // current file

V2(transposed);
ARC jotdot(ARC a, I f, ARC w);
ARC eqop(ARC a,I f,ARC w);
ARC power(ARC a,I f,ARC w);
ARC fog(ARC a,I f,I g,ARC w);
ARC dotop(ARC a, I f, I g, ARC w); /* matrix multiply */
ARC reduce(I f, ARC w);
ARC scan(I f, ARC w);
ARC transposeop(I f, ARC w);
ARC firstaxis(I f, ARC w);
ARC notopm(I f, ARC w);
ARC notopmd(ARC a, I f, ARC w);

I st[52]; /* symbol table */
#define VAR(a) st[isupper(a)?26+a-'A':a-'a'] /* lookup variable */ 

/* allocation list. for garbage collection*/
struct alist { I x; int mark; struct alist *next; } *ahead = NULL;

I apush(struct alist **node, I a) { /* *node is head by reference in root call */
    if(*node) return apush(&(*node)->next, a); /* seek to the end */
    *node = malloc(sizeof(struct alist));
    if (!*node) perror("malloc"),exit(1);
    (*node)->x = a;
    (*node)->mark = 0;
    (*node)->next = NULL;
    ((ARC)a)->x = (I)*node;
    return a;
}
#define asize(a) (sizeof a/sizeof*a)
void mark(I x){
    if (x){
        if (labs(x) < 256) return;
        ARC a = (ARC)x;
        I y;
        int j,n;
        struct alist *node = (struct alist *)(a->x);
        if (node->mark == 0){
            node->mark = 1;
            switch(AT(a)){
            CASE BOX:
                DO(AN(a),mark(AV(a)[i]))
            CASE FUN: // recurse through user functions
                DO(AN(a),mark(AV(a)[i]))
            CASE OPR: // recurse through derived functions
                ;
            }
        }
    }
}
static int discard(struct alist **node){
    struct alist *next;
    ARC x;
    if (labs((I)*node) < 256) return 0;
    next = (*node)->next;
    x = (ARC)((*node)->x);
    free(x);
    free(*node);
    *node = next;
    return 1;
}
I collect(struct alist **node){
    int i,j,n;
    if (*node == NULL) return 0;
    if (*node == ahead) { /* mark live allocations */
        for (i=0; i < asize(st); i++){
            if (st[i]){ 
                mark(st[i]);
                if (labs(st[i]) > 255) AF((ARC)st[i]) &= ~FL_ASSN;
            }
        }
    }
    if (cfile)
        mark((I)cfile);
    i = 0;
    i += collect(&(*node)->next);
    if ((*node)->mark == 0)
        i += discard(node);
    else
        (*node)->mark = 0;
    return i;
}

I *ma(I n){R(I*) apush(&ahead, (I)malloc(n*sizeof(I)));} //make allocation
void mv(C *d,C *s,I n){DO(n,d[i]=s[i]);}                 //move bytes
I tr(I r,I *d){I z=1;DO(r,z=z*d[i]);R z;}                //table rank
ARC ga(I t,I r,I *d){I n =tr(r,d);                       //generate new array
    struct a a = {t};
    ARC z;
    if (r<0)r=0;
    I sz=(sizeof(*z)/sizeof(I))+r+n*(t==DBL?2:1);
    //if (t==CHR) sz=(sz+1)/4;
    z=(ARC)ma(sz);
    AF(z)=0,AT(z)=t,AR(z)=r,AN(z)=n,AK(z)=sizeof(*z)+r*sizeof(I);
    mv((C*)AD(z),(C*)d,r*sizeof(I));
    R z;}
ARC scalarI(I i){ARC z=ga(INT,0,0);*AV(z)=i;R z;}
ARC scalarD(D d){ARC z=ga(DBL,0,0);*(D*)AV(z)=d;R z;}
ARC scalarC(C c){ARC z=ga(CHR,0,0);*(C*)AV(z)=c;R z;}
ARC arrayC(C *s,I n){ARC z=ga(CHR,1,(I[]){n});mv((C*)AV(z),s,n);R z;}
ARC toI(ARC a){ if (AT(a)==INT)R a;
    ARC z=ga(INT,AR(a),AD(a)); DO(AN(a),AV(z)[i]=((D*)AV(a))[i]); R z;}
ARC toD(ARC a){ if (AT(a)==DBL)R a;
    ARC z=ga(DBL,AR(a),AD(a)); DO(AN(a),((D*)AV(z))[i]=AV(a)[i]); R z;}
ARC cp(ARC a){ ARC z=ga(AT(a),AR(a),AD(a)); DO(AN(a),AV(z)[i]=AV(a)[i]) R z;}

V1(iota){ // generate an index vector 0..w-1
    if (AT(w)==CHR) longjmp(mainloop, TYPE);
    I n=AT(w)==DBL?(I)*(D*)AV(w):*AV(w);ARC z=ga(INT,1,&n);DO(n,AV(z)[i]=i);R z;}

V2(rsh){ // reshape
    I r=AR(a)?*AD(a):1,n=tr(r,AV(a)),wn=AN(w);
    ARC z=ga(AT(w),r,AV(a));
    mv((C*)AV(z),(C*)AV(w),(wn=n>wn?wn:n)*AZ(w));
    if((n-=wn)>0)mv(((C*)AV(z))+wn*AZ(w),(C*)AV(z),n*AZ(w));
    R z;
}
V1(sha){ // shape
    ARC z=ga(INT,1,&AR(w));
    mv((C*)AV(z),(C*)AD(w),AR(w)*sizeof(I));R z;}
V1(id){R cp(w);} // monadic +
V1(size){
    ARC z=ga(INT,0,0);
    *AV(z)=AR(w)?*AD(w):1;R z;}

/* check computation for integer overflow */
int subunder(long x, long y); 
int addover(long x, long y) { 
    if (y == LONG_MIN) return 1; 
    if (y < 0) return subunder(x, -y); 
    if (x > LONG_MAX - y) return 1; 
    return 0; } 
int subunder(long x, long y) { 
    if (y == LONG_MIN) return 1; 
    if (y < 0) return addover(x, -y); 
    if (x < LONG_MIN + y) return 1; 
    return 0; } 
int mulover(long x, long y) { 
    if (x == 0||y == 0) return 0; 
    if (x < 0) x = -x; 
    if (y < 0) y = -y; 
    if (x > LONG_MAX / y) return 1; 
    return 0; } 
int divover(long x, long y) { return 1; } /* integer division always overflows */
int boolover(long x, long y){ return 0; } /* boolean operations never do */

#define MATHOP1(op, overflow) \
    I r=AR(w),*d=AD(w),n=AN(w);ARC z; \
restart: \
    switch(AT(w)){ \
    CASE INT: \
        z=ga(INT,r,d); \
        DO(n, if(overflow(0,AV(w)[i])){w=toD(w);goto restart;} AV(z)[i]= op AV(w)[i]); \
    CASE DBL: \
        z=ga(DBL,r,d); DO(n,((D*)AV(z))[i]= op ((D*)AV(w))[i]); \
    } R z;

V1(negate){ MATHOP1(-, subunder) }
V1(not){ MATHOP1(!, boolover) }

#define RANKCOMPAT \
    if (AR(a)==0) a=rsh(sha(w),a); \
    if (AR(w)==0) { w=rsh(sha(a),w); r=AR(w); d=AD(w); n=AN(w); } \
    if (r!=AR(a)) longjmp(mainloop, RANK); \
    if (n!=AN(a)) longjmp(mainloop, LENGTH); \

#define MATHOP2(op, overflow, ident) \
    I r=AR(w),*d=AD(w),n=AN(w);ARC z; \
    RANKCOMPAT \
restart ## ident: \
    switch(TPAIR(AT(a),AT(w))) { \
    default: longjmp(mainloop, TYPE); \
    CASE TPAIR(INT,INT): \
        z=ga(INT,r,d); \
        DO(n, if(overflow(AV(a)[i],AV(w)[i])){w=toD(w);goto restart ## ident;} \
                AV(z)[i]=AV(a)[i] op AV(w)[i]) \
    CASE TPAIR(INT,DBL): \
        z=ga(DBL,r,d); DO(n,((D*)AV(z))[i]=(D)AV(a)[i] op ((D*)AV(w))[i]) \
    CASE TPAIR(DBL,INT): \
        z=ga(DBL,r,d); DO(n,((D*)AV(z))[i]=((D*)AV(a))[i] op (D)AV(w)[i]) \
    CASE TPAIR(DBL,DBL): \
        z=ga(DBL,r,d); DO(n,((D*)AV(z))[i]=((D*)AV(a))[i] op ((D*)AV(w))[i]) \
    } R z;

#define MATHOPF1(ifunc,dfunc) \
    I r=AR(w),*d=AD(w),n=AN(w);ARC z; \
    switch(AT(w)){ \
    default: longjmp(mainloop, TYPE); \
        CASE INT: z=ga(INT,r,d); DO(n,AV(z)[i]=ifunc(AV(w)[i])) \
        CASE DBL: z=ga(DBL,r,d); DO(n,((D*)AV(z))[i]=dfunc(((D*)AV(w))[i])) \
    } R z;

#define MATHOPF2(ifunc,dfunc) \
    I r=AR(w),*d=AD(w),n=AN(w);ARC z; \
    RANKCOMPAT \
    switch(TPAIR(AT(a),AT(w))){ \
    default: longjmp(mainloop, TYPE); \
    CASE TPAIR(INT,INT): \
        z=ga(INT,r,d); DO(n,AV(z)[i]=(I)ifunc(AV(a)[i], AV(w)[i])) \
    CASE TPAIR(INT,DBL): \
        z=ga(DBL,r,d); DO(n,((D*)AV(z))[i]=dfunc(AV(a)[i], ((D*)AV(w))[i])) \
    CASE TPAIR(DBL,INT): \
        z=ga(DBL,r,d); DO(n,((D*)AV(z))[i]=dfunc(((D*)AV(a))[i], AV(w)[i])) \
    CASE TPAIR(DBL,DBL): \
        z=ga(DBL,r,d); DO(n,((D*)AV(z))[i]=dfunc(((D*)AV(a))[i], ((D*)AV(w))[i])) \
    } R z;

V2(plus){    MATHOP2(+, addover, plus) }
V2(minus){   MATHOP2(-, subunder, minus) }
V2(times){   MATHOP2(*, mulover, times) }
V2(divide){  MATHOP2(/, divover, divide) }
V2(modulus){ ARC t=a;a=w;w=t;
    I r=AR(w),*d=AD(w),n=AN(w);ARC z; \
    RANKCOMPAT \
    switch(TPAIR(AT(a),AT(w))) { \
    CASE TPAIR(INT,INT): \
        z=ga(INT,r,d); \
        DO(n, AV(z)[i]=AV(a)[i] % AV(w)[i]); \
    CASE TPAIR(INT,DBL): \
        z=ga(INT,r,d); DO(n,AV(z)[i]=AV(a)[i] % (I)((D*)AV(w))[i]); \
    CASE TPAIR(DBL,INT): \
        z=ga(INT,r,d); DO(n,AV(z)[i]=(I)((D*)AV(a))[i] % AV(w)[i]); \
    CASE TPAIR(DBL,DBL): \
        z=ga(INT,r,d); DO(n,AV(z)[i]=(I)((D*)AV(a))[i] % (I)((D*)AV(w))[i]); \
    } R z;
}
V2(equal){   MATHOP2(==, boolover, equal) }
V2(notequal){MATHOP2(!=, boolover, notequal)}
V2(and){     MATHOP2(&&, boolover, and) }
V2(or){      MATHOP2(||, boolover, or) }
V2(less){    MATHOP2(<, boolover, less) }
V2(greater){ MATHOP2(>, boolover, greater) }

V2(powerf){ MATHOPF2(pow,pow) }

I imin(I a,I w) { R a<w?a:w;}
D dmin(D a,D w) { R a<w?a:w;}
V2(minimum){ MATHOPF2(imin,dmin) }

I imax(I a,I w) { R a>w?a:w;}
D dmax(D a,D w) { R a>w?a:w;}
V2(maximum){ MATHOPF2(imax,dmax) }

V1(signum){
    ARC z=ga(INT,AR(w),AD(w));
    DO(AN(z),
        switch(AT(w)){
        CASE INT: AV(z)[i] = ((AV(w)[i]>0) - (AV(w)[i]<0));
        CASE DBL: AV(z)[i] = ((((D*)AV(w))[i]>0.0) - (((D*)AV(w))[i]<0.0)); }
      )
    R z;
}
V1(flr){
    ARC z;
    switch(AT(w)){
    CASE INT: z=w;
    CASE DBL: z=ga(INT,AR(w),AD(w)); DO(AN(z),AV(z)[i] = (I)floor(((D*)AV(w))[i]))
    }
    R z;
}
V1(ceiling){
    ARC z;
    switch(AT(w)){
    CASE INT: z=w;
    CASE DBL: z=ga(INT,AR(w),AD(w)); DO(AN(z),AV(z)[i] = (I)ceil(((D*)AV(w))[i]))
    }
    R z;
}
V1(absolute){ MATHOPF1(labs,fabs) }

V2(from){
    I r=AR(w)-1,*d=AD(w)+1,n=tr(r,d);
    ARC z;
    switch(AR(a)){
    default: longjmp(mainloop, RANK);
    CASE 0: z=ga(AT(w),r,d);mv((C*)AV(z),((C*)AV(w))+*AV(a)*n*AZ(w),n*AZ(w));
    CASE 1:
        d=malloc(AR(w)*sizeof(I));
        d[0]=AN(a);
        DO(AR(w)-1,d[i+1]=AD(w)[i+1])
        z=ga(AT(w),AR(w),d);
        DO(d[0], mv(((C*)AV(z))+i*n*AZ(w), ((C*)AV(w))+AV(a)[i]*n*AZ(w),n*AZ(w)))
        free(d);
    } R z;
}
V1(box){
    ARC z=ga(BOX,0,0);*AV(z)=(I)w;R z;}
V1(unbox){
    if (AT(w)==BOX){ ARC z=(ARC)*AV(w);R z; }
    R w;
}
V1(ravel){
    I n=AN(w);ARC z=ga(AT(w),1,&n);
    switch(AT(w)){
    CASE CHR:
        DO(n,((C*)AV(z))[i] = ((C*)AV(w))[i])
    CASE INT:
        DO(n,AV(z)[i] = AV(w)[i])
    CASE DBL:
        DO(n,((D*)AV(z))[i] = ((D*)AV(w))[i])
    } R z;
}

V2(cat){
    I an=AN(a),wn=AN(w),n=an+wn; ARC z;
    switch(TPAIR(AT(a),AT(w))){
    default: longjmp(mainloop, TYPE);
    CASE TPAIR(CHR,CHR):
        z=ga(CHR,1,&n);
        DO(an,((C*)AV(z))[i]=((C*)AV(a))[i])
        DO(wn,((C*)AV(z))[i+an]=((C*)AV(w))[i])
    CASE TPAIR(BOX,BOX):
        z=ga(BOX,1,&n);
        mv((C*)AV(z),(C*)AV(a),an*AZ(a));
        mv((C*)(AV(z)+an),(C*)AV(w),wn*AZ(w));
    CASE TPAIR(INT,INT):
        z=ga(INT,1,&n);
        mv((C*)AV(z),(C*)AV(a),an*AZ(a));
        mv((C*)(AV(z)+an),(C*)AV(w),wn*AZ(w));
    CASE TPAIR(INT,DBL):
        z=ga(DBL,1,&n);
        DO(an,((D*)AV(z))[i]=(D)AV(a)[i]);
        mv(((C*)AV(z))+an*AZ(w),(C*)AV(w),wn*AZ(w));
    CASE TPAIR(DBL,INT):
        z=ga(DBL,1,&n);
        mv((C*)AV(z),(C*)AV(a),an*AZ(w));
        DO(wn,((D*)AV(z))[an+i]=(D)AV(w)[i])
    CASE TPAIR(DBL,DBL):
        z=ga(DBL,1,&n);
        mv((C*)AV(z),(C*)AV(a),an*AZ(w));
        mv(((C*)AV(z))+an*AZ(w),(C*)AV(w),wn*AZ(w));
    }
    R z;
}

V2(find){ /* dyadic iota: find index of w in a */
    if(AR(a)!=1)longjmp(mainloop,RANK);
    ARC z=ga(INT,AR(w),AD(w));
    int j;
    DO(AN(z),j=i;DO(AN(a),if(AV(w)[j]==AV(a)[i]){AV(z)[j]=i;break;}AV(z)[j]=i+1;))
    R z;
}

V2(rotate){
    I t,n=*AV(a);
    if (n<0) n+=AN(w);
    w=cp(w);
    DO(n,t=*AV(w);mv((C*)AV(w),(C*)(AV(w)+1),(AN(w)-1)*AZ(w));AV(w)[AN(w)-1]=t;)
    R w;
}

V1(reverse){ /* reverse along primary axis (rows in 2D) */
    if(AR(w)==0)R w;
    I n = tr(AR(w)-1,AD(w)+1), m = (AT(w)==DBL?2:1);
    //printf("n=%d\n", n);
    ARC z=ga(AT(w),AR(w),AD(w));
    DO(AD(z)[0], mv((C*)(AV(z)+i*n*m), (C*)(AV(w)+(AD(z)[0]-1-i)*n*m), n*m*AZ(z)))
    R z;
}

V1(transposem){ /* reverse axes (row/col in 2D) */
    R transposed(reverse(iota(scalarI(AR(w)))),w);
}

V1(reverselast){ /* reverse along last axis (cols in 2D) */
    ARC z=w;
    if(AR(w)==0)R w;
    z=transposed(rotate(scalarI(-1),iota(scalarI(AR(z)))),z);
    z=reverse(z);
    z=transposed(rotate(scalarI(1),iota(scalarI(AR(z)))),z);
    R z;
}

V2(dotf); /* dot function: shortcut for  "plus dot times" */

V2(compress){
    I n=0,j=0;
    if(AT(a)!=INT){longjmp(mainloop, TYPE);}
    DO(AN(a),n+=AV(a)[i]!=0)
    ARC z=ga(AT(w),1,&n);
    switch(AT(w)){
    default: longjmp(mainloop, TYPE);
    CASE CHR: DO(AN(a),if(AV(a)[i])((C*)AV(z))[j++]=((C*)AV(w))[i])
    CASE INT: DO(AN(a),if(AV(a)[i])AV(z)[j++]=AV(w)[i])
    CASE DBL: DO(AN(a),if(AV(a)[i])((D*)AV(z))[j++]=((D*)AV(w))[i])
    } R z;
}

V2(expand){
    if(AT(a)!=INT){longjmp(mainloop, TYPE);}
    ARC z=ga(AT(w),1,&AN(a));
    switch(AT(w)){
    default: longjmp(mainloop, TYPE);
    CASE CHR: DO(AN(a), ((C*)AV(z))[i]=AV(a)[i]?((C*)AV(w))[i]:' ')
    CASE INT: DO(AN(a), AV(z)[i]=AV(a)[i]?AV(w)[i]:0)
    CASE DBL: DO(AN(a), ((D*)AV(z))[i]=AV(a)[i]?((D*)AV(w))[i]:0.0)
    } R z;
}

V1(commentm){
    //printf("commentm\n");
    longjmp(mainloop, ABORT);
}
V2(commentd){
    //printf("commentd\n");
    //pr(a,stdout);
    R a;
}

V1(execute){
    if (AT(w)==CHR){
        C *d = (C*)ma(AN(w)+1); mv(d,(C*)AV(w),AN(w)); ((C*)d)[AN(w)] = 0; /* make a copy + '\0' */
        R ex(wd(d));
    }
    R w;
}

V2(filed){
    if (AT(a)!=INT) longjmp(mainloop, TYPE);
    switch(AT(w)){
    default: longjmp(mainloop, TYPE);
    CASE CHR: {
        ARC z=ga(FIL,0,0);
        C *fn = (C*)ma(AN(w)+1); mv(fn,(C*)AV(w),AN(w)); ((C*)fn)[AN(w)] = 0; /* make a copy + '\0' */
        C *mode;
        mode = *AV(a)==1?"r":
               *AV(a)==2?"w":
               *AV(a)==3?"rw":0;
        *AV(z) = (I)fopen(fn,mode);
        R cfile = z;
    }
    }
}
V1(filem){
    switch(AT(w)){
    default: longjmp(mainloop, TYPE);
    CASE CHR: {
        ARC z=ga(FIL,0,0);
        //C fn[AN(w)+1]; mv(fn,(C*)AV(w),AN(w)); fn[AN(w)] = '\0'; /* make a copy + '\0' */
        C *fn = (C*)ma(AN(w)+1); mv(fn,(C*)AV(w),AN(w)); ((C*)fn)[AN(w)] = 0; /* make a copy + '\0' */
        *AV(z) = (I)fopen(fn,"r");
        R cfile = z;
    }
    CASE INT:
        if (cfile==0) longjmp(mainloop, TYPE);
        switch(*AV(w)){
        default:
            longjmp(mainloop, RANGE);
        CASE 0: {
            C c;
            fread(&c, sizeof c, 1, (FILE*)*AV(cfile));
            R scalarC(c);
        }
        CASE 1: {
            C *buf=NULL,*tmp;
            buf = (C*)ma(256);
            if (!fgets(buf,255,(FILE*)*AV(cfile)))
                longjmp(mainloop, RANGE);
            R arrayC(buf, strlen(buf)-1);
        }
        }
    }
}

V1(wfile){
    switch(AT(w)){
    default: longjmp(mainloop, TYPE);
    CASE CHR:
        pr(w,(FILE*)*AV(cfile));
        R w;
    }
}

#define FUNCNAME(name,      c,    id,  vd,         vm,         odd,   omm,         omd) name,
#define FUNCINFO(name,      c,    id,  vd,         vm,         odd,   omm,         omd) \
                {           c,    id,  vd,         vm,         odd,   omm,         omd},
#define FTAB(_) \
                _(NOP,      0,    0.0, 0,          0,          0,     0,           0) \
                _(EXCL,    '!',   0.0, notequal,   not,        0,     notopm,      notopmd) \
                _(DBLQUOTE,'"',   0.0, 0,          0,          0,     0,           0) \
                _(HASH,    '#',   0.0, rsh,        sha,        0,     0,           0) \
                _(DOLLAR,  '$',   0.0, or,         0,          0,     0,           0) \
                _(PERCENT, '%',   1.0, divide,     0,          0,     0,           0) \
                _(AND,     '&',   1.0, and,        0,          fog,   0,           0) \
                _(QUOTE,   '\'',  0.0, 0,          0,          0,     0,           0) \
                _(STAR,    '*',   1.0, times,      signum,     0,     0,           power) \
                _(PLUS,    '+',   0.0, plus,       id,         0,     0,           0) \
                _(COMMA,   ',',   0.0, cat,        ravel,      0,     0,           0) \
                _(MINUS,   '-',   0.0, minus,      negate,     0,     0,           0) \
                _(DOT,     '.',   0.0, dotf,       0,          dotop, 0,           jotdot) \
                _(SLASH,   '/',   0.0, compress,   0,          0,     reduce,      0) \
                _(COLON,   ':',   0.0, 0,          0,          0,     0,           0) \
                _(SEMI,    ';',   0.0, 0,          execute,    0,     0,           0) \
                _(LANG,    '<',   0.0, less,       box,        0,     0,           0) \
                _(EQUAL,   '=',   0.0, equal,      0,          0,     0,           eqop) \
                _(RANG,    '>',   0.0, greater,    unbox,      0,     0,           0) \
                _(QUEST,   '?',   0.0, 0,          0,          0,     0,           0) \
                _(AT,      '@',   0.0, rotate,     reverse,    0,     transposeop, 0) \
                _(LBRAC,   '[',   0.0, minimum,    flr,        0,     0,           0) \
                _(BKSLASH, '\\',  0.0, expand,     0,          0,     scan,        0) \
                _(RBRAC,   ']',   0.0, maximum,    ceiling,    0,     0,           0) \
                _(CARET,   '^',   M_E, powerf,     0,          0,     0,           0) \
                _(HBAR,    '_',   0.0, filed,      filem,      0,     firstaxis,   0) \
                _(BKQUOTE, '`',   0.0, transposed, transposem, 0,     0,           0) \
                _(LCURL,   '{',   0.0, from,       size,       0,     0,           0) \
                _(VBAR,    '|',   0.0, modulus,    absolute,   0,     0,           0) \
                _(RCURL,   '}',   0.0, commentd,   commentm,   0,     0,           0) \
                _(TILDE,   '~',   0.0, find,       iota,       0,     0,           0) \
                _(NFUNC,   0,     0.0, 0,          0,          0,     0,           0) \
/* END FTAB */
enum{FTAB(FUNCNAME)};
struct{             C c; D id;
           ARC(*vd)(ARC,        ARC);
           ARC(*vm)(            ARC);
          ARC(*odd)(ARC, I,  I, ARC);
          ARC(*omm)(     I,     ARC);
          ARC(*omd)(ARC, I,     ARC);
}ftab[]={ FTAB(FUNCINFO) };

ARC vid(I f){ R qd(f)?  vid(AV((ARC)f)[1]) : scalarD(ftab[f].id); }

I pi(I i, FILE *f){R fprintf(f,"%d ",i);}
I pc(I c, FILE *f){R fprintf(f,"%c",c);}
I pd(D d, FILE *f){R fprintf(f,"%f ",d);}
I po(I c, FILE *f){R fprintf(f,"%c", ftab[c].c);}
I pf(I i, FILE *f){
    if (labs(i) < 256){
        if (qp(i)){R fprintf(f,"%c",i);}
        if (qv(i)){R po(i,f);}
        R pc(i,f);
    }
    R pr(i,f);
}
I nl(FILE *f){R fprintf(f,"\n");}

/* FIXME generalize this shit, bro */
I pr(ARC w, FILE *f){
    if(labs((I)w)<255){ uintptr_t x=(intptr_t)w;
        if (qp(x))
            fprintf(f,"%c", (int)x);
        else if (qv(x))
            fprintf(f,"%c", ftab[x].c);
        else if (qd(x)) {
            DO(AN(w)-1,)
        }
        R nl(f);
    }
    I r=AR(w),*d=AD(w),n=AN(w);
    int j,k,l,(*p)();
    //printf("%d:",AT(w)); DO(r,pi(d[i])); nl(f);
    switch(AT(w)){
    default: R fprintf(f,"unprintable type\n");
    CASE FUN: p=pf;
    CASE OPR: p=po;
    CASE BOX: p=pr;
    CASE INT: p=pi;
    CASE CHR: p=pc;
        switch(r){
        case 0:case 1:
            DO(n,p(((C*)AV(w))[i],f)) nl(f);
        CASE 2:
            DO(d[0], j=i; DO(d[1],p(((C*)AV(w))[j*d[1]+i],f)) nl(f) )
        CASE 3:
            DO(d[0], k=i; DO(d[1], j=i; DO(d[2],p(((C*)AV(w))[(k*d[1]+j)*d[2]+i],f)) nl(f)
                        ) nl(f);nl(f))
        CASE 4:
            DO(d[0], l=i; DO(d[1], k=i; DO(d[2], j=i;
                            DO(d[3],p(((C*)AV(w))[((l*d[1]+k)*d[2]+j)*d[3]+i],f)) nl(f)
                            ) nl(f);nl(f)) nl(f);nl(f))
        }
        R 0;
    CASE DBL: p=pd;
        switch(r){
        case 0:case 1:
            DO(n,p(((D*)AV(w))[i],f)) nl(f);
        CASE 2:
            DO(d[0], j=i; DO(d[1],p(((D*)AV(w))[j*d[1]+i],f)) nl(f) )
        CASE 3:
            DO(d[0], k=i; DO(d[1], j=i; DO(d[2],p(((D*)AV(w))[(k*d[1]+j)*d[2]+i],f)) nl(f)
                        ) nl(f);nl(f))
        CASE 4:
            DO(d[0], l=i; DO(d[1], k=i; DO(d[2], j=i;
                            DO(d[3],p(((D*)AV(w))[((l*d[1]+k)*d[2]+j)*d[3]+i],f)) nl(f)
                            ) nl(f);nl(f)) nl(f);nl(f))
        }
        R 0;
    }
    switch(r){
    case 0:case 1:
        DO(n,p(AV(w)[i],f)) nl(f);
    CASE 2:
        DO(d[0], j=i; DO(d[1],p(AV(w)[j*d[1]+i],f)) nl(f) )
    CASE 3:
        DO(d[0], k=i; DO(d[1], j=i; DO(d[2],p(AV(w)[(k*d[1]+j)*d[2]+i],f)) nl(f)
                    ) nl(f);nl(f))
    CASE 4:
        DO(d[0], l=i; DO(d[1], k=i; DO(d[2], j=i;
                        DO(d[3],p(AV(w)[((l*d[1]+k)*d[2]+j)*d[3]+i],f)) nl(f)
                        ) nl(f);nl(f)) nl(f);nl(f))
    }
    R 0;
}

qp(unsigned a){R (a<255) && isalpha(a);} /* question: is a variable? */
qd(unsigned a){R (a>255) && (AT((ARC)a)==OPR || AT((ARC)a)==FUN || AT((ARC)a)==FIL);} /* derived function */
qv(unsigned a){R qd(a) /* verb */
              || ((a<NFUNC) && (ftab[a].vd || ftab[a].vm));}
qo(unsigned a){R (a<NFUNC) && (ftab[a].odd || ftab[a].omm || ftab[a].omd);} /* operator */
qf(unsigned a){R a>255 && AT((ARC)a)==FIL;} /* is a file */
qomm(unsigned a){R (a<NFUNC) && (ftab[a].omm);} /* operator-monadic : dyadic */
qodd(unsigned a){R (a<NFUNC) && (ftab[a].odd);} /* operator-dyadic : dyadic */
qomd(unsigned a){R (a<NFUNC) && (ftab[a].omd);} /* operator-monadic : dyadic */

ARC dotdot(ARC a, I f, ARC w){ /* iota shortcut */
    R plus(a,iota(plus(scalarI(1),minus(w,a))));
}

I idx(I*vec,I*dims,I n){ I z=*vec; DO(n-1,z*=dims[i+1];z+=vec[i+1]) R z; }  // index
I *vdx(I ind,I*dims,I n, I*vec){ // vector-dex. vec is a passed-in tmp array, size of dims
    I t=ind,*z=vec; DO(n,z[n-1-i]=t%dims[n-1-i];t/=dims[n-1-i]) R z; }

V2(transposed){ /* dyadic transpose. vector a selects axes of w */
    //printf("transposed\n"); printf("AR(a)=%d\n",AR(a));
    if (AR(a)==0) a=rotate(a,iota(scalarI(AR(w))));
    if (AR(a)>1) longjmp(mainloop, RANK);
    ARC d,dims=from(a,d=sha(w));
    ARC z=ga(AT(w),AN(dims),AV(dims));
    I j;
    DO(AN(w),
            vdx(i,AD(w),AR(w),AV(d));
            d=from(a,d);
            j=idx(AV(d),AD(z),AR(z));
            AV(z)[j]=AV(w)[i];
            )
    R z;
}

ARC jotdot(ARC a, I f, ARC w){ /* Outer Product wrt f */
    if (f==DOT){ R dotdot(a,f,w); }
    I *d=malloc((AR(a)+AR(w))*sizeof(I));
    mv((C*)d,(C*)AD(a),AR(a)*sizeof(I));
    mv((C*)(d+AR(a)),(C*)AD(w),AR(w)*sizeof(I));
    ARC z=ga(AT(w),AR(a)+AR(w),d);
    ARC sa,sw,sz;
    switch(AT(z)){
    CASE INT: sa=scalarI(0); sw=scalarI(0);
        DO(AN(z),
            *AV(sa)=AV(a)[idx(vdx(i,AD(z),AR(z),d),AD(a),AR(a))];
            *AV(sw)=AV(w)[idx(d+AR(a),AD(w),AR(w))];
            sz=vd(f,sa,sw);
            AV(z)[i]=*AV(sz); )
    CASE DBL: sa=scalarD(0); sw=scalarD(0);
        a=toD(a);
        DO(AN(z),
            *(D*)AV(sa)=((D*)AV(a))[idx(vdx(i,AD(z),AR(z),d),AD(a),AR(a))];
            *(D*)AV(sw)=((D*)AV(w))[idx(d+AR(a),AD(w),AR(w))];
            sz=vd(f,sa,sw);
            ((D*)AV(z))[i]=*(D*)AV(sz); )
    }
    free(d);
    R z;
}

ARC dotop(ARC a, I f, I g, ARC w){ /* Inner Product wrt f and g */
    if(AT(a)==CHR||AT(w)==CHR) longjmp(mainloop, TYPE);
    if(AR(a)==0&&AR(w)==0)R vd(g,a,w);
    I *d=malloc((AR(a)+AR(w)-2+2)*sizeof(I)); 
    mv((C*)d,(C*)AD(a),(AR(a)-1)*sizeof(I));
    mv((C*)(d+AR(a)-1),(C*)(AD(w)+1),(AR(w)-1)*sizeof(I));
    //ARC wdims=sha(w),adims=sha(a);
    w=transposed(rotate(scalarI(1),iota(scalarI(AR(w)))),w); /* take rows of transpose(w) */
    ARC va,vw,vz;
    ARC z;
    va=ga(AT(a),1,AD(a)+AR(a)-1);
retry:
    vw=ga(AT(w),1,AD(w)+AR(w)-1);
    z=ga(AT(w),AR(a)+AR(w)-2,d);
    DO(AN(z),
            vdx(i,AD(z),AR(z),d); /* generate index vector from z */
            mv((C*)(d+AR(a)),(C*)(d+AR(a)-1),AR(w)*sizeof(I)); /* scootch over the w part */

            d[AR(a)-1]=0;                  /* 0 to index the whole "row" of a */
            mv((C*)AV(va),(C*)(AV(a))+idx(d,AD(a),AR(a))*AZ(a),AN(va)*AZ(va)); /* copy "row" of a */

            (d+AR(a))[AR(w)-1]=0;          /* 0 to index the whole "row" of `w */
            mv((C*)AV(vw),(C*)(AV(w))+idx(d+AR(a),AD(w),AR(w))*AZ(w),AN(vw)*AZ(va)); /* copy "row" of `w */

            vz=reduce(f,vd(g,va,vw));   /* perform functions */

            //DO(AR(a)+AR(w),printf("%d",d[i])) printf("\n");
            //pr(va,stdout); pr(vw,stdout); pr(vz,stdout);
            switch(TPAIR(AT(z),AT(vz))){
            CASE TPAIR(INT,INT): AV(z)[i]=*AV(vz); /* extract (scalar) result */
            CASE TPAIR(INT,DBL): w=toD(w); goto retry;
            CASE TPAIR(DBL,INT): ((D*)AV(z))[i]=(D)*AV(vz);
            CASE TPAIR(DBL,DBL): ((D*)AV(z))[i]=*((D*)AV(vz));
            }
            )
    free(d);
    R z;
}

ARC firstaxis(I f, ARC w){
    ARC z;
    w=transposed(rotate(scalarI(1),iota(scalarI(AR(w)))),w); 
    z = vm(f, w);
    z=transposed(rotate(scalarI(-1),iota(scalarI(AR(z)))),z); 
    R z;
}

ARC eqop(ARC a,I f,ARC w){
    switch(f){
        default: longjmp(mainloop, OPERATOR);
        CASE LANG: { MATHOP2(<=, boolover, lesseq) }
        CASE RANG: { MATHOP2(>=, boolover, greatereq) }
        CASE EQUAL: { MATHOP2(==, boolover, eqeq) }
        CASE EXCL: { MATHOP2(!=, boolover, notequal) }
    }
}

ARC notopm(I f, ARC w){
    R not(vm(f,w));
}

ARC notopmd(ARC a, I f, ARC w){
    R not(vd(f,a,w));
}

ARC power(ARC a,I f,ARC w){
    I n=*AV(w);
    ARC z=vid(f);
    if(n)z=a;
    DO(n-1,z=vd(f,a,z))
    R z;
}
ARC fog(ARC a,I f,I g,ARC w){
    R vm(f,vd(g,a,w));
}

V2(dotf){dotop(a,PLUS,STAR,w);}

ARC reduce(I f, ARC w){
    I r=AR(w),*d=AD(w),n=AN(w);ARC z=vid(f);
    if (r){
        if (d[0]){
            n=d[0];
            ARC ind=scalarI(n-1);
            z=from(ind,w);
            DO(n-1,*AV(ind)=n-2-i; z=vd(f,from(ind,w),z))
        }
    } else
        z=w;
    R z;
}
ARC scan(I f, ARC w){
    I r=AR(w),*d=AD(w),n=AN(w);ARC z=vid(f),x=z;
    if (r){
        if (d[0]){
            n=d[0];
            ARC ind=scalarI(0);
            z=x=from(ind,w);
            DO(n-1,*AV(ind)=i+1;z=cat(z,x=vd(f,x,from(ind,w))))
        }
    } else
        z=w;
    R z;
}

ARC transposeop(I f, ARC w){
    ARC z;
    if (AR(w)==0)R w;
    if (AR(w)==1){
        z=ga(AT(w),2,(I[]){1,AD(w)[0]});
        mv((C*)AV(z),(C*)AV(w),AN(w)*AZ(w));
        w=z;
    }
    switch(f){
    default: longjmp(mainloop, OPERATOR);
    CASE DOT: z=cp(w);
    CASE MINUS: z=reverse(w);
    CASE VBAR: z=reverselast(w);
    CASE SLASH: z=transposem(reverse(reverselast(w)));
    CASE BKSLASH: z=transposem(w);
    CASE PLUS: z=reverselast(                    reverse(w));
    CASE LANG: z=transposem(reverse(reverselast( reverse(w))));
    CASE RANG: z=transposem(                     reverse(w));
    }
    R z;
}

I nfun(I *e){ /* new user function */
    I n;
    for(n=0;e[n];n++);
    ARC z=ga(FUN,1,(I[]){++n});
    DO(n,AV(z)[i]=e[i])
    R (I)z;
}
I nommv(I f, I o){        /* new derived monadic verb  arity in [0] */ 
    ARC z=ga(OPR,1,(I[]){3}); AV(z)[0]=1; AV(z)[1]=f; AV(z)[2]=o;
    R (I)z; }
I nomdv(I f, I o){        /* derived dyadic verb of 1 function */
    ARC z=ga(OPR,1,(I[]){3}); AV(z)[0]=2; AV(z)[1]=f; AV(z)[2]=o;
    R (I)z; }
I noddv(I f, I o, I g){ /* new derived dyadic verb of 2 functions */ 
    ARC z=ga(OPR,1,(I[]){4}); AV(z)[0]=2; AV(z)[1]=f; AV(z)[2]=o; AV(z)[3]=g;
    R (I)z; }

ARC vm(I v, ARC w){         /* monadic verb handler */ 
    if (qd(v)){ARC d=(ARC)v; /* derived verb */
        if (AT(d)==FUN){   /* user-defined function */
            d = cp(d);
            ARC z, y;
            y = (ARC)VAR('y');
            VAR('y') = (I)w;
            z = ex(AV(d)); 
            VAR('y') = (I)y;
            R z;
        }
        if (AT(d)==OPR && AV(d)[0]==1){  /* operator */
            R ftab[ AV(d)[2] ].omm(AV(d)[1], w);
        }
        if (AT(d)==FIL){ /* file token used as function */
            cfile = d;
            R filem(w);
        }
        longjmp(mainloop, OPERATOR);
    }
    //printf("vm\n");
    if (labs(v) >= NFUNC) longjmp(mainloop, FUNCTION);
    if (!ftab[v].vm)  /* no monadic function defined: call dyadic function with a=id(f) */
        R ftab[v].vd(scalarD(ftab[v].id), w);
    R ftab[v].vm(w);  /* basic monadic function */
}
ARC vd(I v, ARC a, ARC w){  /* dyadic verb handler */ 
    if (qd(v)){ARC d=(ARC)v; /* derived verb */
        if (AT(d)==FUN){    /* user-defined function */
            d = cp(d);
            ARC z, x, y;
            y = (ARC)VAR('y');
            x = (ARC)VAR('x');
            VAR('y') = (I)w;
            VAR('x') = (I)a;
            z = ex(AV(d)); 
            VAR('y') = (I)y;
            VAR('x') = (I)x;
            R z;
        }
        if (AT(d)==OPR && ftab[ AV(d)[2] ].odd && AN(d)==4)  /* dyadic operator */
            R ftab[ AV(d)[2] ].odd(a, AV(d)[1], AV(d)[3], w);
        if (AT(d)==OPR && ftab[ AV(d)[2] ].omd && AN(d)==3) /* monadic operator/dyadic function */
            R ftab[ AV(d)[2] ].omd(a, AV(d)[1], w);
        longjmp(mainloop, OPERATOR);
    }
    //printf("vd\n");
    if (labs(v) >= NFUNC) longjmp(mainloop, FUNCTION);
    R ftab[v].vd(a,w);  /* basic function */
}

#define ADV ++e; /* advance expression pointer */ 
#define BB b=e[1]; /* update b */ 
#define CC c=e[2]; /* update c */ 
#define DD d=e[3]; /* update d */ 
#define ABCD ADV BB CC DD 
#define AACD ADV ADV CC DD 
#define PAREN(x,off) \
    if ((x)=='('){ int i,n,*p; \
        for(i=1,n=1;e[i+off] && n;i++) \
            n+=e[i+off]=='('?1:e[i+off]==')'?-1:0; \
        p=malloc((i+off)*sizeof(I)); \
        mv((C*)p,(C*)(e+1+off),(i-2)*sizeof(I)); \
        p[i-2]=0; \
        (x)=(I)ex(p); \
        free(p); \
        e+=i-1; \
    }

ARC ex(I *e){ I a=*e,b,c,d; BB CC DD 
#ifdef TRACE
    printf("%d %d %d %d: %d %d\n", (int)a, (int)b, (int)c, (int)d, (int)e[0], (int)e[1]);
#endif
    if(!*e)R (ARC)0;
    I bspace=0;
    while(a==' '){a=*ABCD} 
    PAREN(a,0) BB CC DD
    if(qp(a)&&b==LANG){ ARC z;
assign:
        z = (ARC)(VAR(a)=(I)ex(e+2));
        if (labs((I)z) > 255) AF(z) |= FL_ASSN;
        R z;
    }
    if(a==COLON) R (ARC)nfun(e+1);
    if(qp(a)&&b==COLON){
        --e; /* pretend there was a '<' */
        goto assign;
    }
mon_verb: 
    if(qf(a)&&b==LANG){
        R wfile(ex(e+2));
    }
    if(qv(a)){ 
        while(b==' '){bspace=1; ABCD} 
        if (b){
            if (qo(b)){
                if(qomm(b)){ a=nommv(a,b); ABCD goto mon_verb; } 
                if (c && qodd(b) && qv(c)){ a=noddv(a,b,c); ADV ABCD
                    printf("%d %c\n", *e, (*e)& 127);
                    printf("%d %c\n", b, (b)& 127);
                    goto mon_verb; }
                if (qomd(b)){ a=nomdv(a,b); ABCD goto mon_verb; }
            }
            R vm(a,ex(e+1));
        }
        R (ARC)a;
    } 
dy_verb: 
    while(b==' '){bspace=1; ABCD} 
    PAREN(b,1) CC DD
    if(qp(b) && VAR(b))b=VAR(b);
    if(b){ 
        if(qv(b)){ 
            if (labs(b) < NFUNC && ftab[b].vd == commentd) R (ARC)a;
            if(qo(c)){ 
                while(d==' '){ADV DD}
                if(qp(d) && VAR(d))d=VAR(d);
                PAREN(d,3)
                e[3]=d;
                if (qodd(c) && qv(d)){ b=noddv(b,c,d); AACD goto dy_verb; }
                if (qomd(c)){ b=nomdv(b,c); ADV CC DD goto dy_verb; }
                longjmp(mainloop, OPERATOR);
            }
            while(c==' '){ADV CC DD}
            PAREN(c,2) else {
                if(qp(c) && VAR(c)) c=VAR(c);
                e[2]=c;
                c=(I)ex(e+2);
            }

            if(qp(a) && VAR(a))a=VAR(a); 
            R vd(b,(ARC)a,(ARC)c);
        } 
        if(qp(a) && VAR(a))a=VAR(a);
        if(qv(a)) { BB CC DD goto mon_verb; } //discard any lookup on b
        if(bspace){  // space-delimited vector?
            bspace=0;
            if(qv(b)){ABCD goto dy_verb;} 
            if( (  (AT((ARC)b)==INT || AT((ARC)b)==DBL)
                && (AT((ARC)a)==INT || AT((ARC)a)==DBL) )
            || (AT((ARC)a)==CHR && AT((ARC)b)==CHR)){ 
                a=(I)cat((ARC)a,(ARC)b); ABCD goto dy_verb;
            }
        }
        R (ARC)a;
    } 
    if(qp(a))a=VAR(a);
    R (ARC)a;
}


noun(c){ARC z;if(!isdigit(c))R 0;z=ga(INT,0,0);*AV(z)=c-'0';R (I)z;}
verb(c){I i=1;for(;ftab[i].c;)if(ftab[i++].c==c)R i-1;R 0;}

I *wd(C *s){
    I a,n=strlen(s),*e=ma(n+1),i,j;C c,*rem;long ll;
    for(i=0,j=0;c=s[i];i++,j++){
        if(c=='}') { e[j]=0; break; }
        if(s[i]=='\''){
            //a=(I)scalarC(s[++i]); //previous, single-char version
            ++i;
            int k,l;
            for (k=0; k < 1; k++)
                for (l=0; ;l++)
                    if (s[i+l]=='\'')
                        break;
            a=(I)arrayC(s+i, l);
            i+=l;
            //printf("%c", s[i]);
        } else if(isdigit(c)){
            ll=strtol(s+i,&rem,10); //printf("%d:%s\n", ll, rem);
            if(*rem=='.'){D d;
                d=strtod(s+i,&rem); //printf("%f:%s\n", d, rem-1);
                if (rem[-1]=='.') { //no decimal
                    --rem;
                    goto integer;
                }
                a=(I)scalarD(d);
                s=rem;i=-1; //adjust pointer+index
            } else {
integer:
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

C *lib[] = {
    "F<:;>(y!0){(<'1'),<'y*Fy-1'",  //factorial, cf. http://www.jsoftware.com/papers/DirectDef.htm
    "P<:+/x*y^~(:+/y=y)x",          //polynomial function, ibid.
    "T<:y*x%+/y",                   //tips distribution. distribute x among y "shares"
    "B:((~#y)<.(~#y))", //base encode
};

int main(){C s[999];
    int err;
    int i;
    if (err = setjmp(mainloop)){
        printf("%s %s\n", errstr[err], err==ABORT?"":"ERROR");
    }
    for (i=0;i < asize(lib); i++){
        ex(wd(lib[i]));
    }
    printf("<@%d>\n", collect(&ahead));
    while(printf("\t"),fgets(s, -1 + sizeof s, stdin) && ! (s[strlen(s)-1]='\0') ) {
        int gc_count;
        ARC z = ex(wd(s));
        if (labs((I)z)>255 && !(AF(z)&FL_ASSN))
            pr(z,stdout);
        gc_count = collect(&ahead);
#ifdef GCREPORT
        printf("<@%d>\n", gc_count);
#endif
    }

    R 0;
}

#if 0
// i thought these would be useful for matrix multiply, but needed something different
// see idx() and vdx() which operate on int[]s.
// these functions would have required packing the dims into new arrays and for
// what?? extra effort simply because these pretty functions operator on the 
// wrong data types.
//
// at the same time. this code is unapollogitically terse and dense.
// so read these first. I know, I know, I said they're not even used.
// but, it would be good practice for trying to read any of the above.

// ARC is our "archetype" data structure for arrays of all sorts and sizes.
// this function performs "Horner's Rule" using a as an array of dimensions
// and w as an array of indices. All arguments assumed to be integers.
// It computes the result as an integer (I z),
// uses scalarI() to construct an array result.
// AV() macro yields the "array values", a `*` to the (ravel of) the data.
// AN() is the total number of elements in the ravel.
// DO() macro implements a counted-loop control structure with i = 0 .. n-1
ARC rdx(ARC a, ARC w){
    I z=*AV(w);
    DO(AN(w)-1,z*=AV(a)[1+i];z+=AV(w)[1+i])
    R scalarI(z);
}

// ddx performs the inverse operation, a div/mod loop to peel-off
// indices corresponding to the scalar (integer) w's ravel-position
// in the space with dimensions a.
ARC ddx(ARC a, ARC w){
    I t=*AV(w);
    ARC z=ga(INT,AR(a),AD(a));
    DO(AN(z),AV(z)[AN(z)-1-i]=t%AV(a)[AN(a)-1-i];t/=AV(a)[AN(a)-1-i])
    R z;
}
#endif
