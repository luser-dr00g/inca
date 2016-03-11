#if 0
/*
 *  Parsing and Execution

 *  Execution in APL proceeds right-to-left and this is 
 *  accomplished with a relatively straightforward algorithm. 
 *  We have 2 stacks (it could also be done with a queue and 
 *  a stack) called the left-stack and the right-stack. 
 *  The left stack starts at the left edge and expands 
 *  on the right. 

        |- 0 1 2 top 

 *  The right stack is the opposite, anchored at the right 
 *  and growing to the left. 

                   top 2 1 0 -| 

 *  Placing them on one line, the *gap* between them in a
 *  sense represents our *pointer* into the expression.

                   |
                   V
        |- 0 1 2       2 1 0 -| 

 *  Of course these are just conceptual distinctions: they're 
 *  both just stacks. The left stack is initialized with a 
 *  mark object (illustrated here as ^) to indicate the left
 *  edge, followed by the entire expression. The right stack
 *  is initialized with a single null object (illustrated
 *  here as $) to indicate the right edge. 

        |-^2*1+⍳4   $-| 

 *  At each step, we A) move one object to the right stack, 

        |-^2*1+⍳   4$-| 

 *  Until there are at least 4 objects on the right stack, we do
 *  nothing else. The next object is actually an identifier 
 *  sequence '+⍳' which unless the top of the right stack is
 *  the left-arrow (which means assignment) is split into
 *  its defined components, '+' and '⍳' and only the rightmost
 *  one moves to right stack. Defined prefixes are pushed back
 *  to the left stack. At this point these two characters
 *  represent their respective verb objects.

        |-^2*1+   ⍳4$-|     + and ⍳ are now verb objects, not symbols
        |-^2*1   +⍳4$-| 

 *  If there are at least 4 objects on the right stack, then 
 *  we B) classify the top 4 elements with a set of predicate 
 *  functions and then check through the list of grammatical patterns.
 *  The engine is always trying to examine the top 4 elements
 *  of the right stack.

        |- 0 1 2       2 1 0 -| 
                       | | | |
                       ?+?+?+? <--- 4 pattern slots

 *  Back to our expression, we now have 4 elements to check.

        |-^2*1   +⍳4$-| 

 *  But this configuration (VERB VERB NOUN NULLOBJ) doesn't match anything.
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

        |-^2   *1+A$-|  dyad     * is now a verb object
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

#include "array.h"
#include "encoding.h"
#include "symtab.h"
#include "lex.h"
#include "verbs.h"
#include "xverb.h"
#include "exec.h"

typedef int object;
#include "exec_private.h"
#include "debug.h"

// execute expression e using environment st and yield result
//TODO check/handle extra elements on stack (interpolate?, enclose and cat?)
object execute_expression(array e, symtab st){
    int n = e->dims[0];
    stack *lstk,*rstk;

    init_stacks(&lstk, &rstk, e, n);

    while(lstk->top){ //left stack not empty
        object x = stackpop(lstk);
        DEBUG(0,"->%08x(%d,%d)\n", x, gettag(x), getval(x));

        if (qprog(x)){ // x is a pronoun?
            object y;
            if ((y=parse_and_lookup_name(lstk, rstk, x, st)) != 0)
                return y;
        } else stackpush(rstk,x);

        check_rstk_with_patterns_and_reduce(lstk, rstk, st);
    }

    return extract_result_and_free_stacks(lstk, rstk);
}

void check_rstk_with_patterns_and_reduce(stack *lstk, stack *rstk, symtab st){
    int docheck = 1;
    while (docheck){ //check rstk with patterns and reduce
        docheck = 0;
        if (rstk->top>=4){
            int c[4];
            for (int j=0; j<4; j++)
                c[j] = classify(rstk->a[rstk->top-1-j]);
            DEBUG(1, "%08x %08x %08x %08x\n", c[0], c[1], c[2], c[3]);

            for (int i=0; i<sizeof ptab/sizeof*ptab; i++)
                if (check_pattern(c, ptab, i)) {
                    DEBUG(1,"match %d\n",i);
                    DEBUG(1,"%08x %08x %08x %08x\n",
                        ptab[i].c[0], ptab[i].c[1], ptab[i].c[2], ptab[i].c[3]);
                    object t[4];
                    move_top_four_to_temp(t, rstk);
                    switch(i){ PARSETAB(PARSETAB_ACTION) }
                    if (i==L9)  //twiddle the mark for fake left paren
                        stackpush(lstk,stackpop(rstk));
                    docheck = 1; //stack changed: check again
                    break;
                }
        }
    }
}


size_t sum_symbol_lengths(array e, int n){
    int i,j;
    for (i=j=0; i<n; i++) {
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
    DEBUG(1,"rstk->top=%d\n", rstk->top);
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
object monad(object f, object y, object dummy, symtab st){
    DEBUG(0,"monad %08x(%d,%d) %08x(%d,%d)\n",
            f, gettag(f), getval(f),
            y, gettag(y), getval(y));
    verb v;
    switch(gettag(f)){
    case VERB: v = getptr(f); break;
    case XVERB: {xverb x = getptr(f); v = x->verb;} break;
    }
    if (!v->monad) {
        printf("monad undefined\n");
        return null;
    }
    return v->monad(y,v);
}

object dyad(object x, object f, object y, symtab st){
    DEBUG(0,"dyad %08x(%d,%d) %08x(%d,%d) %08x(%d,%d) \n",
            x, gettag(x), getval(x),
            f, gettag(f), getval(f),
            y, gettag(y), getval(y));
    verb v;
    switch(gettag(f)){
    case VERB: v = getptr(f); break;
    case XVERB: { xverb x = getptr(f);
                    DEBUG(0,"xverb %08x(%d,%d)\n",
                            x->id, gettag(x->id), getval(x->id));
                    v = x->verb; } break;
    }
    if (!v->dyad) {
        printf("dyad undefined\n");
        return null;
    }
    return v->dyad(x,y,v);
}

object adv(object f, object g, object dummy, symtab st){
    DEBUG(0,"adverb\n");
    verb v;
    switch(gettag(g)){
    case ADVERB: v = getptr(g); break;
    case XVERB: { xverb x = getptr(g);
                    DEBUG(0,"xverb %08x(%d,%d)\n",
                            x->id, gettag(x->id), getval(x->id));
                    v = x->adverb; } break;
    }
    if (!v->monad) {
        printf("adv undefined\n");
        return null;
    }
    return v->monad(f,v);
}

object conj_(object f, object g, object h, symtab st){
    DEBUG(0,"conj\n");
    verb v;
    switch(gettag(g)){
    case ADVERB: v = getptr(g); break;
    case XVERB: {xverb x = getptr(g); v = x->adverb;} break;
    }
    if (!v->dyad) {
        printf("conj undefined\n");
        return null;
    }
    return v->dyad(f,h,v);
}

//specification
object spec(object name, object v, object dummy, symtab st){
    DEBUG(0,"assn %08x(%d,%d) <- %08x(%d,%d)\n",
            name, gettag(name), getval(name),
            v, gettag(v), getval(v));
    def(st, name, v);
    return v;
}

object punc(object x, object dummy, object dummy2, symtab st){
    DEBUG(0,"punc %08x(%d,%d)\n",
            x, gettag(x), getval(x));
    return x;
}


// lookup name in environment unless to the left of assignment
// if the full name is not found, but a defined prefix is found,
// push the prefix back to the left stack and continue lookup
// with remainder. push value to right stack.
int parse_and_lookup_name(stack *lstk, stack *rstk, object x, symtab st){
    if (rstk->top && qassn(stacktop(rstk))){ //assignment: no lookup
        stackpush(rstk,x);
    } else {
        DEBUG(0,"lookup\n");
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
            return x;
            //return null;
        }
        while (n){ //while name
            DEBUG(0,"%d\n", n);
            DEBUG(0,"<-%08x(%d,%d)\n",tab->val,gettag(tab->val),getval(tab->val));
            stackpush(lstk,tab->val);           //pushback value
            s = p;
            tab = findsym(st,&p,&n,0);         //lookup remaining name
            if (tab->val == null) {
                printf("error undefined internal\n");
                return x;
                //return null;
            }
        }
        //replace name with defined value
        DEBUG(0,"==%08x(%d,%d)\n", tab->val, gettag(tab->val), getval(tab->val));
        stackpush(rstk,tab->val);
    }
    return 0;
}

