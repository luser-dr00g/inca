#include <stdarg.h>
#include <stdlib.h>

#include "ar.h"
#include "en.h"
#include "st.h"

int qp(int x){ return gettag(x)==PROG; }
int qc(int x){ return gettag(x)==CHAR && getval(x) == 0x2190; }

#define PREDTAB(_) \
    _( ANY = 1, qa, 1 ) \
    _( VAR = 2, qp, gettag(x)==PROG ) \
    _( NOUN = 4, qn, 1 ) \
    _( VERB = 8, qv, 1 ) \
    _( ADV = 16, qo, 1 ) \
    _( CONJ = 32, qj, 1 ) \
    _( ASSN = 64, qc, 1 ) \
    /**/

#define PRED_FUNC(X,Y,...) int Y(int x){ return __VA_ARGS__; }
PREDTAB(PRED_FUNC)
#define PRED_ENT(X,Y,...) Y,
int (*q[])(int) = { PREDTAB(PRED_ENT) };
#define PRED_ENUM(X,...) X,
enum predicate { PREDTAB(PRED_ENUM) 
                 EDGE = MARK+ASSN+LPAR,
                 AVN = VERB+NOUN+ADV };

int classify(int x){
    int i,v,r;
    for (i=0, v=1, r=0; i<sizeof q/sizeof*q; i++, v*=2)
        if (q[i](x))
            r |= v;
    return r;
}

typedef struct stack { int top; int a[1];} stack; /* top==0::empty */
#define stackpush(stkp,el) ((stkp)->a[(stkp)->top++]=(el))
#define stackpop(stkp) ((stkp)->a[--(stkp)->top])

array ex(array e){
    int n = e->dims[0];
    int i,j;
    int x;
    stack *lstk,*rstk;
    int docheck;

    for (i=0; i<n; i++) // sum symbol lengths
        if (gettag(e->data[i])==PROG)
            j+=((array*)getptr(e->data[i]))->dims[0];

    // prepare stacks
    lstk=malloc(sizeof*lstk + (n+j+1) * sizeof*lstk->a);
    lstk->top=0;
    stackpush(lstk,mark);
    for (i=0; i<n; i++)
        stackpush(lstk,e->data[i]);
    rstk=malloc(sizeof*rstk + (n+j+1) * sizeof*rstk->a);
    rstk->top=0;
    stackpush(rstk,null);    

    while(lstk->top){
        x=stackpop(lstk);

        if (qp(x)){ //parse and lookup name
            if (rstk->top && qc(rstk->a[rstk->top-1])){
                stackpush(rstk,x);
            } else {
                array a = getptr(x);
                int *s = a->data;
                int *p = s;
                int n = a->dims[0];
                symtab tab = findsym(&st,&p,&n);

                if (tab->a == null) printf("error undefined\n");
                while (n){ //while name
                    stackpush(lstk,newobj(s,p-s,50)); //pushback prefix
                    tab = findsym(&st,&p,50);         //parse name
                    if (tab->a == null) printf("error undefined\n");
                }
                //replace name with defined value
                stackpush(rstk,tab->a);
            }
        } else { stackpush(rstk,x); }

        //check rstk with patterns and reduce
        docheck = 1;
        while (docheck){
            docheck = 0;
            if (rstk->top>=4){ //enough elements to check?
                int c[4];
                for (j=0; j<4; j++)
                    c[j] = classify(rstk->a[rstk->top-1-j]);
                for (i=0; i<sizeof parsetab/sizeof*parsetab; i++){
                    if ( c[0] & parsetab[i].c[0] &&
                         c[1] & parsetab[i].c[1] &&
                         c[2] & parsetab[i].c[2] &&
                         c[3] & parsetab[i].c[3] ) {
                        t[0] = stackpop(rstk);
                        t[1] = stackpop(rstk);
                        t[2] = stackpop(rstk);
                        t[3] = stackpop(rstk);
                        switch(i){
                            PARSETAB(PARSETAB_ACTION)
                        }
                        docheck = 1;
                        break;
                    }
                }
            }
        }
    }
    //assemble results and return
}

