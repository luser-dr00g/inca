#include <stdarg.h>
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
    _( NOUN = 4, qn, 1 ) \
    _( VERB = 8, qv, 1 ) \
    _( ADV = 16, qo, 1 ) \
    _( CONJ = 32, qj, 1 ) \
    _( ASSN = 64, qc, gettag(x)==CHAR && getval(x) == 0x2190 ) \
    /**/
#define PRED_FUNC(X,Y,...) int Y(int x){ return __VA_ARGS__; }
PREDTAB(PRED_FUNC)
#define PRED_ENT(X,Y,...) Y,
int (*q[])(int) = { PREDTAB(PRED_ENT) };
#define PRED_ENUM(X,...) X,
enum predicate { PREDTAB(PRED_ENUM) 
                 EDGE = MARK+ASSN+LPAR,
                 AVN = VERB+NOUN+ADV };
/* encode predicate applications into a binary number */
int classify(int x){
    int i,v,r;
    for (i=0, v=1, r=0; i<sizeof q/sizeof*q; i++, v*=2)
        if (q[i](x))
            r |= v;
    return r;
}

#define PARSETAB(_) \
    _(L0, EDGE,     VERB,      NOUN, ANY,  ) /*monadic func*/\
    _(L1, EDGE+AVN, VERB,      VERB, NOUN, ) /*monadic func*/\
    _(L2, EDGE+AVN, NOUN,      VERB, NOUN, ) /*dyadic func*/\
    _(L3, EDGE+AVN, NOUN+VERB, ADV,  ANY, ) /*adverb*/\
    _(L4, EDGE+AVN, NOUN+VERB, CONJ, NOUN+VERB, ) /*conjunction*/\
    _(L5, VAR,      ASSN,      AVN,  ANY, ) /*specification*/\
    _(L6, LPAR,     ANY,       RPAR, ANY, ) /*punctuation*/\
    /**/
#define PARSETAB_PAT(label, pat1, pat2, pat3, pat4, ...) \
    {pat1, pat2, pat3, pat4},
struct parsetab { int c[4]; } parsetab[] = { PARSETAB(PARSETAB_PAT) };
#define PARSETAB_INDEX(label, ...) label,
enum { PARSETAB(PARSETAB_INDEX) };
#define PARSETAB_ACTION(label, pat1, pat2, pat3, pat4, ...) \
    case label: {__VA_ARGS__;} break;

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
            if (rstk->top && qc(stacktop(rstk))){ //assignment: no lookup
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

        docheck = 1;
        while (docheck){ //check rstk with patterns and reduce
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
                        docheck = 1; //stack changed: check again
                        break;
                    }
                }
            }
        }
    }
    //assemble results and return
}

