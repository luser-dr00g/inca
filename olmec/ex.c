#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ar.h"
#include "en.h"
#include "st.h"
#include "wd.h"
#include "vb.h"
#include "ex.h"

int parse_and_lookup_name(stack *lstk, stack *rstk, int x, symtab st);

/* stack type
   the size is generously pre-calculated
   and so we can skip all bounds checking.
   stkp->top is the size (index of next empty slot for next push)
   stkp->top-1 is the topmost element
 */
typedef struct stack { int top; int a[1];} stack; /* top==0::empty */
#define stackpush(stkp,el) ((stkp)->a[(stkp)->top++]=(el))
#define stackpop(stkp) ((stkp)->a[--((stkp)->top)])
#define stacktop(stkp) ((stkp)->a[(stkp)->top-1])

/* predicate functions are instantiated according to the table
   defined in the header.
   the q[] function array is used by classify to apply all 
   predicate functions yielding a sum of all applicable codes
   defined in the table. Specific qualities or combinations 
   may then be determined easily by masking.
 */
#define PRED_FUNC(X,Y,...) int Y(int x){ return __VA_ARGS__; }
PREDTAB(PRED_FUNC)
#define PRED_ENT(X,Y,...) Y,
int (*q[])(int) = { PREDTAB(PRED_ENT) };

/* encode predicate applications into a binary number
   which can be compared to a pattern with a mask */
int classify(int x){
    int i,v,r;
    for (i=0, v=1, r=0; i<sizeof q/sizeof*q; i++, v*=2)
        if (q[i](x))
            r |= v;
    return r;
}

/* Parser Actions,
   each function is called with x y z parameters defined in PARSETAB 
 */
int monad(int f, int y, int dummy, symtab st){
    printf("monad\n");
    verb v = getptr(f);
    return v->monad(y,v);
}

int dyad(int x, int f, int y, symtab st){
    printf("dyad\n");
    verb v = getptr(f);
    return v->dyad(x,y,v);
}

int adv(int f, int g, int dummy, symtab st){
    printf("adverb\n");
    verb v = getptr(g);
    return v->monad(f,v);
}

int conj_(int f, int g, int h, symtab st){
    printf("conj\n");
    verb v = getptr(g);
    return v->dyad(f,h,v);
}

//specification
int spec(int name, int v, int dummy, symtab st){
    def(st, name, v);
    return v;
}

int punc(int x, int dummy, int dummy2, symtab st){
    return x;
}

// the Parse Table defines the grammar of the language
// At each stack move, the top four elements of the right stack
// are checked against each of these patterns. A matching pattern
// returns element t[pre] from the temp area to the right stack
// then calls func(t[x],t[y],t[z]) and pushes the result to the
// right stack, then pushes t[post] and t[post2]. 
#define PARSETAB(_) \
/*    p[0]      p[1]      p[2]      p[3]      func   pre x y z   post,2*/\
_(L0, EDGE,     VRB,      NOUN,     ANY,      monad,  3, 1,2,-1,   0,-1) \
_(L1, EDGE+AVN, VRB,      VRB,      NOUN,     monad, -1, 2,3,-1,   1, 0) \
_(L2, ANY,      NOUN,     DEX,      ANY,      monad,  3, 2,1,-1,   0,-1) \
_(L3, EDGE+AVN, NOUN,     VRB,      NOUN,     dyad,  -1, 1,2,3,    0,-1) \
_(L4, EDGE+AVN, NOUN+VRB, ADV,      ANY,      adv,    3, 1,2,-1,   0,-1) \
_(L5, ANY,      LEV,      NOUN+VRB, ANY,      adv,    3, 2,1,-1,   0,-1) \
_(L6, EDGE+AVN, NOUN+VRB, CONJ,     NOUN+VRB, conj_, -1, 1,2,3,    0,-1) \
_(L7, VAR,      ASSN,     AVN,      ANY,      spec,   3, 0,2,-1,  -1,-1) \
_(L8, LPAR,     ANY,      RPAR,     ANY,      punc,   3, 1,-1,-1, -1,-1) \
_(L9, MARK,     ANY,      RPAR,     ANY,      punc,   3, 1,-1,-1,  0,-1) \
_(L10,ANY,      LPAR,     ANY,      NUL,      punc,   3, 2,-1,-1,  0,-1) \
/**/

// create parsetab array of structs containing the patterns
#define PARSETAB_PAT(label, pat1, pat2, pat3, pat4, ...) \
    {pat1, pat2, pat3, pat4},
struct parsetab { int c[4]; } parsetab[] = { PARSETAB(PARSETAB_PAT) };

// generate labels to coordinate table and execution
#define PARSETAB_INDEX(label, ...) label,
enum { PARSETAB(PARSETAB_INDEX) };

// perform the grammar production, transforming the stack
#define PARSETAB_ACTION(label,p1,p2,p3,p4, func, pre,x,y,z,post,post2) \
    case label: { \
        if (pre>=0) stackpush(rstk,t[pre]); \
        stackpush(rstk,func(x>=0?t[x]:0,y>=0?t[y]:0,z>=0?t[z]:0,st)); \
        if (post>=0) stackpush(rstk,t[post]); \
        if (post2>=0) stackpush(rstk,t[post2]); \
    } break;

// execute expression e using environment st and yield result
int execute_expression(array e, symtab st){
    int n = e->dims[0];
    int i,j;
    int x;
    stack *lstk,*rstk;
    int docheck;

    for (i=0; i<n; i++) { // sum symbol lengths 
        if (gettag(e->data[i])==PROG) {
            //printf("%p\n", getptr(e->data[i]));
            j+=((array)getptr(e->data[i]))->dims[0];
        }
    }

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
        printf("->%d(%d,%x)\n", x, gettag(x), getval(x));

        if (qp(x)){ // x is a pronoun?
            if (parse_and_lookup_name(lstk, rstk, x, st) == null)
                return null;
        } else {
            stackpush(rstk,x);
        }

        docheck = 1;
        while (docheck){ //check rstk with patterns and reduce
            docheck = 0;
            if (rstk->top>=4){ //enough elements to check?
                int c[4];

                for (j=0; j<4; j++)
                    c[j] = classify(rstk->a[rstk->top-1-j]);
                //printf("%d %d %d %d\n", c[0], c[1], c[2], c[3]);

                for (i=0; i<sizeof parsetab/sizeof*parsetab; i++){
                    if ( c[0] & parsetab[i].c[0]
                      && c[1] & parsetab[i].c[1]
                      && c[2] & parsetab[i].c[2]
                      && c[3] & parsetab[i].c[3] ) {
                        int t[4];

                        printf("match %d\n", i);
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

// lookup name in environment unless to the left of assignment
// if the full name is not found, but a defined prefix is found,
// push the prefix back to the left stack and continue lookup
// with remainder. push value to right stack.
int parse_and_lookup_name(stack *lstk, stack *rstk, int x, symtab st){
    if (rstk->top && qc(stacktop(rstk))){ //assignment: no lookup
        stackpush(rstk,x);
    } else {
        printf("lookup\n");
        int *s;
        int n;
        switch(gettag(x)){
            case PCHAR: {  // single char
                s = &x;
                n = 1;
                } break;
            case PROG: {   // longer name
                array a = getptr(x);
                s = a->data;
                n = a->dims[0];
                } break;
        }
        int *p = s;
        symtab tab = findsym(st,&p,&n,0);

        if (tab->val == null) {
            printf("error undefined prefix\n");
            return null;
        }
        while (n){ //while name
            printf("%d\n", n);
            //stackpush(lstk,newobj(s,p-s,70)); //pushback prefix name
            stackpush(lstk,tab->val);           //pushback value
            s = p;
            tab = findsym(st,&p,&n,0);         //lookup remaining name
            if (tab->val == null) {
                printf("error undefined internal\n");
                return null;
            }
        }
        //replace name with defined value
        printf("==%d(%d,%x)\n", tab->val, gettag(tab->val), getval(tab->val));
        stackpush(rstk,tab->val);
    }
    return 0;
}

