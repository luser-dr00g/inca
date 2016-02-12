#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ar.h"
#include "en.h"
#include "st.h"
#include "wd.h"

typedef struct stack { int top; int a[1];} stack; /* top==0::empty */
#define stackpush(stkp,el) ((stkp)->a[(stkp)->top++]=(el))
#define stackpop(stkp) ((stkp)->a[--(stkp)->top])
#define stacktop(stkp) ((stkp)->a[(stkp)->top-1])

#define PREDTAB(_) \
_( ANY = 1, qa, 1 ) \
_( VAR = 2, qp, gettag(x)==PROG ) \
_( NOUN = 4, qn, gettag(x)==LITERAL \
                 || gettag(x)==CHAR \
                 || gettag(x)==ARRAY ) \
_( VRB = 8, qv, gettag(x)==VERB ) \
_( ADV = 16, qo, gettag(x)==ADVERB ) \
_( CONJ = 32, qj, 0 ) \
_( ASSN = 64, qc, gettag(x)==CHAR && getval(x) == 0x2190 ) \
_( LPAR = 128, ql, gettag(x)==CHAR && getval(x) == '(' ) \
_( RPAR = 256, qr, gettag(x)==CHAR && getval(x) == ')' ) \
/**/
#define PRED_FUNC(X,Y,...) int Y(int x){ return __VA_ARGS__; }
PREDTAB(PRED_FUNC)
#define PRED_ENT(X,Y,...) Y,
int (*q[])(int) = { PREDTAB(PRED_ENT) };
#define PRED_ENUM(X,...) X,
enum predicate { PREDTAB(PRED_ENUM) 
                 EDGE = MARK+ASSN+LPAR,
                 AVN = VRB+NOUN+ADV };
/* encode predicate applications into a binary number */
int classify(int x){
    int i,v,r;
    for (i=0, v=1, r=0; i<sizeof q/sizeof*q; i++, v*=2)
        if (q[i](x))
            r |= v;
    return r;
}

int monad(int f, int y, int dummy, symtab st){
}
int dyad(int x, int f, int y, symtab st){
}
int adv(int f, int g, int dummy, symtab st){
}
int conj_(int f, int g, int h, symtab st){
}
int spec(int name, int v, int dummy, symtab st){
    array na = getptr(name);
    int n = na->dims[0];
    int *p = na->data;
    st tab = findsym(st,&p,&n,1);
    tab->val = v;
    return v;
}
int punc(int x, int dummy, int dummy2, symtab st){
    return x;
}

#define PARSETAB(_) \
/*                                               pre x y z   post,2*/\
_(L0, EDGE,     VRB,      NOUN, ANY,      monad,  3, 1,2,-1,   0,-1) \
_(L1, EDGE+AVN, VRB,      VRB,  NOUN,     monad, -1, 2,3,-1,   1, 0) \
_(L2, EDGE+AVN, NOUN,     VRB,  NOUN,     dyad,  -1, 1,2,3,    0,-1) \
_(L3, EDGE+AVN, NOUN+VRB, ADV,  ANY,      adv,    3, 1,2,-1,   0,-1) \
_(L4, EDGE+AVN, NOUN+VRB, CONJ, NOUN+VRB, conj_,  -1, 1,2,3,    0,-1) \
_(L5, VAR,      ASSN,     AVN,  ANY,      spec,   3, 0,2,-1,  -1,-1) \
_(L6, LPAR,     ANY,      RPAR, ANY,      punc,   3, 1,-1,-1, -1.-1) \
/**/
#define PARSETAB_PAT(label, pat1, pat2, pat3, pat4, ...) \
    {pat1, pat2, pat3, pat4},
struct parsetab { int c[4]; } parsetab[] = { PARSETAB(PARSETAB_PAT) };
#define PARSETAB_INDEX(label, ...) label,
enum { PARSETAB(PARSETAB_INDEX) };
#define PARSETAB_ACTION(label,p1,p2,p3,p4, func, pre,x,y,z,post,post2) \
    case label: {
        if (pre>=0) stackpush(rstk,t[pre]); \
        stackpush(rstk,func(x>=0?t[x]:0,y>=0?t[y]:0,z>=0?t[z]:0,st)); \
        if (post>=0) stackpush(rstk,t[post]); \
        if (post2>=0) stackpush(rstk,t[post2]); \
    } break;

array ex(array e, symtab st){
    int n = e->dims[0];
    int i,j;
    int x;
    stack *lstk,*rstk;
    int docheck;

    for (i=0; i<n; i++) // sum symbol lengths
        if (gettag(e->data[i])==PROG)
            j+=((array)getptr(e->data[i]))->dims[0];

    // allocate and prepare stacks
    lstk=malloc(sizeof*lstk + (n+j+1) * sizeof*lstk->a);
    lstk->top=0;
    stackpush(lstk,mark);
    for (i=0; i<n; i++)
        stackpush(lstk,e->data[i]);
    rstk=malloc(sizeof*rstk + (n+j+1) * sizeof*rstk->a);
    rstk->top=0;
    stackpush(rstk,null);    

    while(lstk->top){ //left stack not empty
        x=stackpop(lstk);

        if (qp(x)){ //parse and lookup name
            if (rstk->top && qc(stacktop(rstk))){ //assignment: no lookup
                stackpush(rstk,x);
            } else {
                array a = getptr(x);
                int *s = a->data;
                int *p = s;
                int n = a->dims[0];
                symtab tab = findsym(st,&p,&n,0);

                if (tab->val == null) printf("error undefined\n");
                while (n){ //while name
                    stackpush(lstk,newobj(s,p-s,50)); //pushback prefix
                    tab = findsym(st,&p,&n,0);         //parse name
                    if (tab->val == null) printf("error undefined\n");
                }
                //replace name with defined value
                stackpush(rstk,tab->val);
            }
        } else { stackpush(rstk,x); }

        docheck = 1;
        while (docheck){ //check rstk with patterns and reduce
            docheck = 0;
            if (rstk->top>=4){ //enough elements to check?
                int c[4];
                for (j=0; j<4; j++)
                    c[j] = classify(rstk->a[rstk->top-1-j]);
                for (i=0; i<sizeof parsetab/sizeof*parsetab; i++){
                    if ( c[0] & parsetab[i].c[0]
                      && c[1] & parsetab[i].c[1]
                      && c[2] & parsetab[i].c[2]
                      && c[3] & parsetab[i].c[3] ) {
                        int t[4];
                        t[0] = stackpop(rstk);
                        t[1] = stackpop(rstk);
                        t[2] = stackpop(rstk);
                        t[3] = stackpop(rstk);
                        switch(i){
                            PARSETAB(PARSETAB_ACTION)
                        }
                        docheck = 1; //stack changed: check again
                        break;
                    }
                }
            }
        }
    }
    //assemble results and return
    //TODO check/handle extra elements on stack
    //(interpolate?, enclose and cat?)
    stackpop(rstk); // mark
    return stackpop(rstk);
}

