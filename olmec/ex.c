#if 0
/*
 *  Parsing and Execution

 *  Execution in APL proceeds right-to-left and this is 
 *  accomplished with a relatively straightforward algorithm. 
 *  We have 2 stacks (it could also be done with a queue and 
 *  a stack) called the left-stack and the right-stack. 
 *  The left stack starts at the left edge and expands 
 *  on the right. 

        |- 0 1 2 3 top 

 *  The right stack is the opposite, anchored at the right 
 *  and growing to the left. 

                   top 3 2 1 0 -| 

 *  Of course these are just conceptual distinctions: they're 
 *  both just stacks. The left stack is initialized with a 
 *  mark object (illustrated here as ^) to indicate the left
 *  edge, followed by the entire expression. The right stack
 *  has a single null object (illustrated here as $) to indicate
 *  the right edge. 

        |-^2*1+⍳4   $-| 

 *  At each step, we A) move one object to the right stack, 

        |-^2*1+⍳   4$-| 

 *  Until there are at least 4 objects on the right stack, we do
 *  nothing else.

        |-^2*1+   ⍳4$-| 
        |-^2*1   +⍳4$-| 

 *  If there are at least 4 objects on the right stack, then 
 *  we B) classify the top 4 elements with a set of predicate 
 *  functions and then check through the list of grammatical patterns,
 *  but this configuration (VERB VERB NOUN NULLOBJ) doesn't match anything.
 *  Move another object and try again.

        |-^2*   1+⍳4$-| 

 *  Now, the above case (NOUN VERB VERB NOUN) matches this production: 

    /*    p[0]      p[1]      p[2]      p[3]      func   pre x y z   post,2*/\
    _(L1, EDGE+AVN, VRB,      VRB,      NOUN,     monad, -1, 2,3,-1,   1, 0) \

 *  application of a monadic verb. The numbers in the production indicate 
 *  which elements should be preserved, and which should be passed to the 
 *  handler function. The result from the handler function is interleaved 
 *  back onto the right stack. 

        |-^2*   1+A$-|    A←⍳4

 *  where A represents the array object returned from the iota function. 
 *  (Incidentally this is a lazy array, generating its values on-demand.) 

        |-^2   *1+A$-|  dyad
        |-^2   *B$-|      B←1+A
        |-^   2*B$-| 
        |-   ^2*B$-|  dyad
        |-   ^C$-|        C←2*B

 *  Eventually the expression ought to reduce to 3 objects: a mark, 
 *  some result object, and a null. Anything else is an error 
 *  TODO handle this error. 

        |-   ^C$-|
              ^
              |
            result
 */
#endif

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

typedef int object;
#include "ex_private.h"
#include "debug.h"

// execute expression e using environment st and yield result
//TODO check/handle extra elements on stack (interpolate?, enclose and cat?)
int execute_expression(array e, symtab st){
    int i,j,n = e->dims[0];
    stack *lstk,*rstk;
    int docheck;

    init_stacks(&lstk, &rstk, e, n);

    while(lstk->top){ //left stack not empty
        object x = stackpop(lstk);
        DEBUG("->%08x(%d,%d)\n", x, gettag(x), getval(x));

        if (qp(x)){ // x is a pronoun?
            if (parse_and_lookup_name(lstk, rstk, x, st) == null)
                return null;
        } else stackpush(rstk,x);

        docheck = 1;
        while (docheck){ //check rstk with patterns and reduce
            docheck = 0;
            if (rstk->top>=4){
                int c[4];
                for (j=0; j<4; j++)
                    c[j] = classify(rstk->a[rstk->top-1-j]);

                for (i=0; i<sizeof ptab/sizeof*ptab; i++)
                    if (check_pattern(c, ptab, i)) {
                        object t[4];
                        move_top_four_to_temp(t, rstk);
                        switch(i){
                            PARSETAB(PARSETAB_ACTION)
                        }
                        docheck = 1; //stack changed: check again
                        break;
                    }
            }
        }
    }

    return extract_result_and_free_stacks(lstk,rstk);
}

size_t sum_symbol_lengths(array e, int n){
    int i,j;
    for (i=j=0; i<n; i++) { // sum symbol lengths 
        if (gettag(e->data[i])==PROG) {
            //printf("%p\n", getptr(e->data[i]));
            j+=((array)getptr(e->data[i]))->dims[0];
        }
    }
    return j;
}

void init_stacks(stack **lstkp, stack **rstkp, array e, int n){
    int i,j;
#define lstk (*lstkp) /* by-reference */
#define rstk (*rstkp)
    j=sum_symbol_lengths(e,n);
    stackinit(lstk,n+j+1);
    stackpush(lstk,mark);
    for (i=0; i<n; i++) stackpush(lstk,e->data[i]);  // push expression
    stackinit(rstk,n+j+1);
    stackpush(rstk,null);    
#undef lstk
#undef rstk
}

object extract_result_and_free_stacks(stack *lstk, stack *rstk){
    object x;
    stackpop(rstk); // pop mark
    x = stackpop(rstk);
    free(lstk);
    free(rstk);
    return x;
}

int check_pattern(int *c, parsetab *ptab, int i){
    return c[0] & ptab[i].c[0]
        && c[1] & ptab[i].c[1]
        && c[2] & ptab[i].c[2]
        && c[3] & ptab[i].c[3];
}

void move_top_four_to_temp(object *t, stack *rstk){
    t[0] = stackpop(rstk);
    t[1] = stackpop(rstk);
    t[2] = stackpop(rstk);
    t[3] = stackpop(rstk);
}

/* Parser Actions,
   each function is called with x y z parameters defined in PARSETAB 
 */
int monad(int f, int y, int dummy, symtab st){
    DEBUG("monad\n");
    verb v = getptr(f);
    if (!v->monad) {
        printf("monad undefined\n");
        return null;
    }
    return v->monad(y,v);
}

int dyad(int x, int f, int y, symtab st){
    DEBUG("dyad\n");
    verb v = getptr(f);
    if (!v->dyad) {
        printf("dyad undefined\n");
        return null;
    }
    return v->dyad(x,y,v);
}

int adv(int f, int g, int dummy, symtab st){
    DEBUG("adverb\n");
    verb v = getptr(g);
    if (!v->monad) {
        printf("adv undefined\n");
        return null;
    }
    return v->monad(f,v);
}

int conj_(int f, int g, int h, symtab st){
    DEBUG("conj\n");
    verb v = getptr(g);
    if (!v->dyad) {
        printf("conj undefined\n");
        return null;
    }
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


// lookup name in environment unless to the left of assignment
// if the full name is not found, but a defined prefix is found,
// push the prefix back to the left stack and continue lookup
// with remainder. push value to right stack.
int parse_and_lookup_name(stack *lstk, stack *rstk, object x, symtab st){
    if (rstk->top && qc(stacktop(rstk))){ //assignment: no lookup
        stackpush(rstk,x);
    } else {
        DEBUG("lookup\n");
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
            DEBUG("%d\n", n);
            stackpush(lstk,tab->val);           //pushback value
            s = p;
            tab = findsym(st,&p,&n,0);         //lookup remaining name
            if (tab->val == null) {
                printf("error undefined internal\n");
                return null;
            }
        }
        //replace name with defined value
        DEBUG("==%08x(%d,%d)\n", tab->val, gettag(tab->val), getval(tab->val));
        stackpush(rstk,tab->val);
    }
    return 0;
}

