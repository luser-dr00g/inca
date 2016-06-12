#if 0
/*  Parsing and Execution

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

 *  The engine is always trying to examine the top 4 elements
 *  of the right stack. It cannot perform its pattern-match
 *  unless there are at least 4 objects to check.

        |- 0 1 2       2 1 0 -| 
                       | | | |
                       ?+?+?+? <--- 4 pattern slots

 *  Of course these are just conceptual distinctions: they're 
 *  both just stacks. The left stack is initialized with a 
 *  mark object (illustrated here as ^) to indicate the left
 *  edge, followed by the entire expression. The right stack
 *  is initialized with a single mark object (illustrated
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
 *  We now have 4 elements to check.

        |-^2*1   +⍳4$-| 

 *  But this configuration (VERB VERB NOUN NULLOBJ) doesn't match anything.
 *  Move another object and try again.
 
        |-^2*   1+⍳4$-| 

 *  Now, the above case (NOUN VERB VERB NOUN) matches this production: 

//    p[0]      p[1]      p[2]      p[3]      func     start finish //\
//-->items[3]   it[2]     it[1]     it[0]                         hack  //\
_(L1, EDGE+AVN, VRB,      MON,      NOUN,     monadic,  1,    0,  0) \

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
 *  some result object, and a final mark. Anything else is an error 
 *  TODO handle this error. 

        |-   ^C$-|
              ^
              |
            result

 *  One complication of the setup happens when performing the 
 *  "fake left paren" production. Whereas in the real "paren-
 *  punctuation" production there is a left paren which can
 *  simply be eliminated, yielding just the paren contents on top
 *  of the right-stack,... in the "fake left paren", we treat the
 *  beginning-of-line *mark* object as a left paren. But we
 *  cannot simply eliminate it because this would produce a
 *  mal-formed configuration of the stacks, of the whole engine,
 *  a violation of the invariants. But we need it out of the way so the 
 *  remaining productions can effectively reduce the expression.
 *  So, in the fake left paren production, there is this hack:

        stack_push(left,stack_pop(right)) ) \

 *  So, I hope this explains what that's about. Without it, the 
 *  super-parens idea just fails to work with the grammar machine.

 */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "encoding.h"
#include "symtab.h"
#include "lex.h"
#include "verbs.h"
#include "verb_private.h"
#include "adverbs.h"
#include "xverb.h"
#include "exec.h"
#include "print.h"
#include "editor.h"

#include "exec_private.h"

static int last_was_assn;

void dumpstacks(stack left, object x, stack right){
    int n = stack_element_count(left);
    stack_element *ptr = stack_top_elements_address(left, n);
    for (int i=0; i<n; ++i)
        print(ptr[i].datum, 5, 1);
    print(x, 10, 1);
    n = stack_element_count(right);
    ptr = stack_top_elements_address(right, n);
    for (int i=0; i<n; ++i)
        print(ptr[n-1-i].datum, 5, 1);
}

// execute expression e using environment st and yield result
//TODO check/handle extra elements on stack (interpolate?, enclose and cat?)
object execute_expression(array expr, symtab env, int *plast_was_assn){
    DEBUG(2, "execute_expression\n");
    last_was_assn = 0;

    if (is_del_func(expr)){
        *plast_was_assn = 1;
        return read_del_func(expr, env);
    }
    if (is_func_def(expr)) {
        *plast_was_assn = 1;
        return func_def(expr, env);
    }

    stack left = new_left_stack_for(expr);
    stack right = new_stack(1+stack_capacity(left));
    stack_push_datum(right, mark);
    DEBUG(0,"->%08x(%d,%d)\n", null, gettag(null), getval(null));

    while(!stack_is_empty(left)){
        stack_element x = stack_pop(left);
        DEBUG(0,"->%08x(%d,%d)\n", x.datum, gettag(x.datum), getval(x.datum));
        IFDEBUG(1, dumpstacks(left, x.datum, right););

        if (is_pronoun(x)) {
            object y = parse_and_lookup_name(left, right, &x, env);
            if (y != 0) return y;
        }
        if (is_nilad(x)) {
            stack_element y = datum_to_stack_element(niladic(x.datum, 0, 0, 0, env));
            DEBUG(1, "nilad result: ");
            IFDEBUG(1, print(y.datum, 0, 1););
            if (!is_mark(y))
                stack_push(right, y);
        } else
            stack_push(right, x);

        while (stack_element_count(right) >= 4){
            stack_element *items = stack_top_elements_address(right, 4);
            if (0) ;
            PARSE_PRODUCTIONS_FOREACH(PRODUCTION_ELSEIFS)
            else break;
        }
    }
    IFDEBUG(1, puts(""); dumpstacks(left, null, right););

    *plast_was_assn = last_was_assn;
    return stack_release(left), penultimate_prereleased_value(right);
}

object execute_block(array block, symtab env, int *plast_was_assn){
    DEBUG(2, "execute_block %d\n", block->dims[0]);
    object result;
    for (int i=1;i<block->dims[0];++i) {
        DEBUG(2, "i=%d\n",i);
        result = execute_expression(getptr(*elem(block,i)), env, plast_was_assn);
        switch(gettag(result)){
        case LABEL: {
            int label = getval(result);
            printf("label = %d\n", label);
            if (label<1 || label>block->dims[0]-1)
                return mark;
            i = label-1;
            continue;
        }
        }
    }
    return result;
}

int is_block(object exp){
    switch(gettag(exp)){
    case PROG:
    case ARRAY: {
        array expr = getptr(exp);
        switch(expr->rank){
        default: fprintf(stderr, "RANK ERROR\n");
                 return 0;
        case 1:
            if (expr->dims[0]) {
                if (*elem(expr,0)==null)
                    return 1;
            }
        }
    }
    }
    return 0;
}

object execute(object exp, symtab env, int *plast_was_assn){
    DEBUG(2, "execute\n");
    IFDEBUG(2, print(exp, 0, 1););

    if (is_block(exp))
        return execute_block(getptr(exp), env, plast_was_assn);
    else
        return execute_expression(getptr(exp), env, plast_was_assn);
}

int qdel(object x){
    return gettag(x)==PCHAR && getval(x)==0x2207;
}

static
int is_del_func(array expr){
    if (expr->rank && expr->dims[0]>=2){
        if (qdel(*elem(expr,0)))
            return 1;
    }
    return 0;
}

static
int is_func_def(array expr){
    if (expr->rank && expr->dims[0]>=3){
        if (qprog(*elem(expr,0)) && qcolon(*elem(expr,1))){
            return 1;
        }
    }
    return 0;
}

static
int is_label(stack_element x){
    return !!(x.code & LAB);
}

static
int is_pronoun(stack_element x){
    return !!(x.code & VAR);
    return qprog(x.datum);
}

static
int is_assn(stack_element x){
    return !!(x.code & ASSN);
    return qassn(x.datum);
}

static
int is_mark(stack_element x){
    return !!(x.code & MARK);
}

static
int is_nilad(stack_element x){
    return !!(x.code & NIL);
}

static
size_t sum_symbol_lengths(array e, int n){
    int i, j;
    for (i=j=0; i<n; i++) {
        if (gettag(e->data[i])==PROG) {
            j+=((array)getptr(e->data[i]))->dims[0];
        }
    }
    return j;
}

static
stack new_left_stack_for (array expr){
    int n = expr->dims[0];
    stack r = new_stack(n + sum_symbol_lengths(expr, n) + 2);
    stack_push_datum(r, mark);
    for (int i=0; i<n; i++) stack_push_datum(r, expr->data[i]);
    return r;
}

static
int matches_ptab_pattern (stack_element items[4], int i){
    return
        items[3].code & ptab[i].c[0] &&
        items[2].code & ptab[i].c[1] &&
        items[1].code & ptab[i].c[2] &&
        items[0].code & ptab[i].c[3];
}

static
int penultimate_prereleased_value (stack s){
    int result = stack_top_elements_address(s, 2)->datum;
    DEBUG(1, "result: ");
    IFDEBUG(1, print(result, 0, 1));
    return stack_release(s), result;
}


/* Parser Actions,
 * each function is called with items[s..f] parameters defined in
 * PARSE_PRODUCTIONS_FOREACH,
 * except niladic functions which are called at the time the object passes
 * from the left stack to the right stack.
 */
object branchout(stack left, stack right, object label){
    stack_release(left);
    stack_release(right);
    return label;
}

object read_del_func(array header, symtab env){
    DEBUG(1, "del!\n");
    char prompt[9]="";
    static int *buf = NULL;
    static int buflen = 0;
    int expn = 0;
    int i = 1;
    symtab child = makesymtabchain(env, 10);
    array func = scalar(null);
    while(1){
        snprintf(prompt, sizeof prompt, "[%d]      ", i);
        buf?buf[0]=0:0;
        get_line(prompt, &buf, &buflen, &expn);
        int c = buf[0];
        if (c==0x2207)
            break;

        array expr = array_new_dims(expn);
        memcpy(expr->data,buf,expn*sizeof(int));

        array a = scan_expression(expr, env);
        object e = cache(ARRAY, a);
        if (is_func_def(a)){ // label def
            def(child, *elem(a,0), newdata(LITERAL, i),0);
            //a = slices(a, (int[]){2}, (int[]){a->dims[0]-1});
            e = vdrop(2, e, 0);
        }

        if (is_block(e)) {
            i += a->dims[0]-1;
            func = cat(func, slices(a,(int[]){1},(int[]){a->dims[0]-1}));
        } else {
            ++i;
            func = cat(func, scalar(e));
        }
    }
    object ret = del(header, func, env, child);
    last_was_assn = 1;
    return ret;
}

object func_def(array expr, symtab env){
    DEBUG(1, "func_def\n");
    object func = dfn(vdrop(2, cache(ARRAY, expr), 0), env);
    IFDEBUG(2, print(func, 0, 1););
    def(env, *elem(expr, 0), func,0);
    last_was_assn = 1;
    return func;
}


object niladic(object f, object dummy, object dummy2, object dummy3, symtab env){
    (void)(dummy,dummy2,dummy3,env);
    DEBUG(1, "nilad\n");
    verb v = getptr(f);
    if (!v->nilad){
        fprintf(stderr, "nilad undefined\n");
        return null;
    }
    object ret = v->nilad(v);
    last_was_assn = 0;
    return ret;
}

object monadic(object f, object y, object dummy2, object dummy3, symtab env){
    (void)(dummy2,dummy3,env);
    DEBUG(1,"monad %08x(%d,%d) %08x(%d,%d)\n",
            f, gettag(f), getval(f),
            y, gettag(y), getval(y));
    verb v;
    switch(gettag(f)){
    case VERB: v = getptr(f); break;
    case XVERB: {xverb x = getptr(f); v = x->verb;} break;
    }
    if (!v->monad) {
        fprintf(stderr, "monad undefined\n");
        return null;
    }
    object ret = v->monad(y, v);
    last_was_assn = 0;
    return ret;
}

object dyadic(object x, object f, object y, object dummy3, symtab env){
    (void)(dummy3,env);
    DEBUG(1,"dyad %08x(%d,%d) %08x(%d,%d) %08x(%d,%d) \n",
            x, gettag(x), getval(x),
            f, gettag(f), getval(f),
            y, gettag(y), getval(y));
    verb v;
    switch(gettag(f)){
    case VERB: v = getptr(f); break;
    case XVERB: { xverb x = getptr(f);
                    DEBUG(1,"xverb %08x(%d,%d)\n",
                            x->id, gettag(x->id), getval(x->id));
                    v = x->verb; } break;
    }
    if (!v->dyad) {
        fprintf(stderr, "dyad undefined\n");
        return null;
    }
    object ret = v->dyad(x, y, v);
    last_was_assn = 0;
    return ret;
}

object adv(object f, object g, object dummy, object dummy3, symtab env){
    (void)(dummy,dummy3,env);
    DEBUG(1,"adverb\n");
    verb v;
    switch(gettag(g)){
    case ADVERB: v = getptr(g); break;
    case XVERB: { xverb x = getptr(g);
                    DEBUG(1,"xverb %08x(%d,%d)\n",
                            x->id, gettag(x->id), getval(x->id));
                    v = x->adverb; } break;
    }
    if (!v->monad) {
        fprintf(stderr, "adv undefined\n");
        return null;
    }
    object ret = v->monad(f, v);
    last_was_assn = 0;
    return ret;
}

object conj_(object f, object g, object h, object dummy3, symtab env){
    (void)(dummy3,env);
    DEBUG(1,"conj\n");
    verb v;
    switch(gettag(g)){
    case ADVERB: v = getptr(g); break;
    case XVERB: {xverb x = getptr(g); v = x->adverb;} break;
    }
    if (!v->dyad) {
        fprintf(stderr, "conj undefined\n");
        return null;
    }
    object ret = v->dyad(f, h, v);
    last_was_assn = 0;
    return ret;
}

object lcurry (object x,    object f,     object dummy2, object dummy3, symtab env){
    (void)(dummy2,dummy3,env);
    DEBUG(1,"lcurry\n");
    object ret = amp(x,f,0);
    last_was_assn = 0;
    return ret;
}

//specification
object spec(object name, object assn, object v, object dummy3, symtab env){
    (void)(assn,dummy3);
    DEBUG(1,"assn %08x(%d,%d) <- %08x(%d,%d)\n",
            name, gettag(name), getval(name),
            v, gettag(v), getval(v));
    def(env, name, v,1);
    last_was_assn = 1;
    return v;
}

object move   (object nn,    object assn,  object v,      object dummy3, symtab env){
    (void)(assn,dummy3);
    DEBUG(1, "move %08x(%d,%d) <- %08x(%d,%d)\n",
            nn, gettag(nn), getval(nn),
            v, gettag(v), getval(v));
    IFDEBUG(1, print(nn, 0, 1));
    IFDEBUG(1, print(v, 0, 1));
    v = vreshape(vshapeof(nn,0),v,0);
    array narr = getptr(nn);
    array varr = getptr(v);
    int scratch[narr->rank];
    int n = productdims(narr->rank, narr->dims);
    for (int i=0; i<n; ++i){
        vector_index(i,narr->dims,narr->rank,scratch);
        *elema(narr,scratch) = *elema(varr,scratch);
    }
    last_was_assn = 1;
    return nn;
}

//remove parentheses
object punc(object paren, object x, object dummy2, object dummy3, symtab env){
    (void)(paren,dummy2,dummy3,env);
    DEBUG(1,"punc %08x(%d,%d)\n",
            x, gettag(x), getval(x));
    last_was_assn = 0;
    return x;
}


/* The bracket clauses *consume* indices left-to-right into an internal structure
 * associated with the left-bracket object (which remains on the stack), evaluating
 * as needed in order to reduce to a closed (empty) bracket pair.
 *
 * [] getval([)== 0
 *
 * [; getval([)== 0->(-1,_)->(-1,-1,_)  (n)->(n,_)
 */
object brasemi(object lbrac, object semi,  object dummy2, object dummy3, symtab env){
    DEBUG(1, "brasemi\n");
    last_was_assn = 0;
    int idx = getval(lbrac);
    if (idx==0)
        return cache(LBRACOBJ, vector(-1, blank));
    array iarr = getptr(lbrac);
    object *last = elem(iarr,iarr->dims[0]-1);
    if (*last == blank) {
        *last = -1;
    }
    return cache(LBRACOBJ, cat(iarr,scalar(blank)));
}

/*
 * [n; getval([)== 0->(n,_)->(n,n,_)
 */
object branoun(object lbrac, object n,     object semi,   object dummy3, symtab env){
    DEBUG(1, "branoun\n");
    last_was_assn = 0;
    int idx = getval(lbrac);
    if (idx==0)
        return cache(LBRACOBJ, vector(n, blank));
    array iarr = getptr(lbrac);
    object *last = elem(iarr,iarr->dims[0]-1);
    if (*last == blank) {
        *last = n;
        return cache(LBRACOBJ, cat(iarr, scalar(blank)));
    }
    return cache(LBRACOBJ, cat(iarr, scalar(n)));
}

/*
 * [n] getval([)== 0->(n) (?,_)->(?,n)
 */
object bracket(object lbrac, object n,     object dummy2,   object dummy3, symtab env){
    DEBUG(1, "bracket\n");
    last_was_assn = 0;
    int idx = getval(lbrac);
    if (idx==0)
        return cache(LBRACOBJ, scalar(n));
    array iarr = getptr(lbrac);
    object *last = elem(iarr,iarr->dims[0]-1);
    if (*last == blank) {
        *last = n;
        return lbrac;
    }
    return cache(LBRACOBJ, cat(iarr, scalar(n)));
}

/*
 * [[n] getval([)== 0->(-1_0,-1_1,-1_...,-1_n,_)
 */
object bracidx(object lbrac,object inner, object rbrac,  object dummy3, symtab env){
    array iarr = getptr(inner);
    array idx = array_new_dims(iarr->data[iarr->cons]+1);
    for (int i=0; i<idx->dims[0]-1; ++i)
        idx->data[i] = -1;
    idx->data[ idx->dims[0]-1 ] = blank;
    last_was_assn = 0;
    return cache(LBRACOBJ, idx);
}

object funcidx(object f, object lbrac, object rbrac, object dummy3, symtab env){
    object idx = cache(ARRAY, getptr(lbrac));
    DEBUG(1, "funcidx %08x(%d,%d)\n",
            lbrac, gettag(lbrac), getval(lbrac));
    last_was_assn = 0;
    return rank(f, idx, 0);
}

object nounidx(object n, object lbrac, object rbrac, object dummy3, symtab env){
    object idx = cache(ARRAY, getptr(lbrac));
    last_was_assn = 0;
    DEBUG(1, "nounidx %08x(%d,%d) %08x(%d,%d)\n",
            n, gettag(n), getval(n),
            idx, gettag(idx), getval(idx));
    if (getval(lbrac)==0)
        return n;
    array iarr = getptr(idx);
    object *last = elem(iarr,iarr->dims[0]-1);
    if (*last == blank) {
        *last = -1;
    }
    IFDEBUG(1, print(idx,0, 1));
    array narr = getptr(n);
    if (narr->rank == 0)
        return n;
    switch(iarr->rank){
    case 0: 
        DEBUG(1, "index rank=0\n");
        DEBUG(1, "getval(index)=%d\n", getval(*elem(iarr, 0)));
        {
            object el = *elem(iarr,0);
            array elarr = getptr(el);
            DEBUG(1, "index_0 = %d\n", getval(*elem(elarr, 0)));
            return cache(ARRAY,
                        slice(narr, getval(*elem(elarr, 0))));
        }
    case 1:
        DEBUG(1, "index rank=1\n");
        switch(iarr->dims[0]){
        case 0: return n;
        case 1: return cache(ARRAY, slice(narr, getval(*elem(iarr, 0))));
        default:
        if (iarr->dims[0]<narr->rank){
            //TODO WHAT?? VVV (stupid)
#if 0
            DEBUG(1, "iterative slicing\n");
            for(int i=0; i<iarr->dims[0]; i++){
                narr = slice(narr, getval(*elem(iarr, i)));
            }
            return cache(ARRAY, narr);
#endif
            array tmp = array_new_dims(narr->rank);
            int i;
            for (i=0; i<iarr->dims[0]; ++i)
                *elem(tmp,i) = *elem(iarr,i);
            for (; i<narr->rank; ++i)
                *elem(tmp,i) = -1;
            free(iarr);
            iarr = tmp;
        }
        if (iarr->dims[0]==narr->rank){
            DEBUG(1, "index dim == arr rank\n");
            //return cache(ARRAY, slicea(narr, iarr->data));
            array spec[narr->rank];
            for (int i=0; i<narr->rank; ++i) {
                if (gettag(*elem(iarr,i))==ARRAY) {
                    spec[i] = getptr(*elem(iarr,i));
                } else {
                    if (getval(*elem(iarr,i))==-1)
                        spec[i] = NULL;
                    else
                        spec[i] = scalar(getval(*elem(iarr,i)));
                }
            }
            return cache(ARRAY, slicec(narr, spec));
        } else {
            fprintf(stderr, "INDEX LENGTH TOO LONG\n");
        }
        }
    default:
        fprintf(stderr, "INDEX RANK TOO HIGH\n");
    }
    return vectorindexleft(idx, n, VT(INDL));
}

// lookup name in environment unless to the left of assignment
// if the full name is not found, but a defined prefix is found,
// push the prefix back to the left stack and continue lookup
// with remainder. push value to right stack.
static
int parse_and_lookup_name(stack left, stack right, stack_element *x, symtab env){

#if 0
    if (//!stack_is_empty(right) &&  //always NUL at least
            is_assn(*stack_top_elements_address(right, 1))){
        DEBUG(1,"pass name\n");
        stack_push(right, x);   //assignment: no lookup
    } else {
#endif
    if (!is_assn(*stack_top_elements_address(right, 1))) {
        DEBUG(1,"lookup ");
        int *s;
        int n, n0;
        switch(gettag(x->datum)){
            case PCHAR: {  // single char
                s = &x->datum;
                n0 = n = 1;
                } break;
            case PROG: {   // longer name
                array a = getptr(x->datum);
                s = a->data;
                n0 = n = a->dims[0];
                } break;
        }

        object *p = s;
        DEBUG(1,"%d ", n);
        symtab tab = findsym(env,&p,&n, 0,0);
        if (tab->value == null) {
            fprintf(stderr, "error undefined prefix\n");
            return x->datum;
        }

        while (n){ //while name not exhausted
            DEBUG(0,"%d<-%08x(%d,%d)\n", n-n0,
                    tab->value, gettag(tab->value), getval(tab->value));
            DEBUG(1,"%d ", n);
            stack_push_datum(left, getsym(tab));           //pushback value
            s = p;
            n0 = n;
            tab = findsym(env,&p,&n, 0,0);         //lookup remaining name
            if (tab->value == null) {
                fprintf(stderr, "error undefined internal\n");
                return x->datum;
            }
        }

        //replace name with defined value
        object val = getsym(tab);
        DEBUG(0,"==%08x(%d,%d)\n", val, gettag(val), getval(val));
        //stack_push_datum(right, tab->value);
        *x = datum_to_stack_element(val);
    }
    return 0;
}

