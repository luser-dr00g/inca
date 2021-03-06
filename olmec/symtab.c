/* Symbol Table
 *
 * As a symbol-table for a Unicode-capable programming language
 * interpreter, I decided to combine the 3 types of associative
 * array that I had implemented before. Xpost's postscript
 * nametype objects are implemented with a ternary search tree,
 * and its dicttype objects are implemented with a hash table.
 * Inca3's symbol table used a trie tree to hold variable-length
 * keys.
 *
 * As a trie, it collapses similar prefixes from the keys.
 * For "abc", "aaa", "abb", and "add", we get the structure:
 * a - a - a
 *   - b - b
 *       - c
 *   - d - d
 *
 * Every key has the same prefix "a" so it is represented
 * exactly once.
 *
 * The Inca3 trie allows only alphabetic characters in symbols,
 * so each node could contain an array of 52 pointers. But to
 * adapt this code for Unicode code points, millions of pointers
 * in each node seems grossly impractical. So the child nodes
 * from each node are organized into a hash table keyed to the
 * single character where they diverge from the tree.
 *
 * In the example above, ('a', 'b', 'd') and ('b', 'c') are
 * collected in hash tables. There are also degenerate hash
 * tables at each of the leaf nodes which are all null.
 *
 * So, each node contains a value or null. Each node also
 * contains a pointer to a table of child nodes which is
 * accessed via a hash lookup on a single char (code-point) of
 * the key string. If the key string is not exhausted, lookup
 * continues on the child nodes of the matched node.
 *
 * Taking advantage of the prefix-collapsing nature of the
 * data-structure, The symbol-lookup mechanism will fallback
 * to returning the longest-defined prefix if the full symbol
 * cannot be found. Assuming this to represent two (or more)
 * juxtaposed symbols, symbol-lookup may then proceed upon
 * the remainder of the key string. see ex.c:parse_and_lookup_name
 *
 * The master lookup function has two search modes:
 * a prefix mode where it returns the first defined value and
 * updates the symbol-string pointer and returns.
 * Or it can be called in the "defining" mode where
 * it follows existing links and creates new nodes until
 * the search string is exhausted.
 *
 * In support of Weizenbaum environment chaining for functions
 * there is now another search option called "bias" which is only
 * relevant for defining searches. If bias is nonzero, search
 * will restart on the prev root before defining new nodes on
 * the current root. This is because defined functions must
 * declare the local variables to be used. Remaining variable 
 * references are to global variables (in the "workspace" or session).
 */
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "encoding.h"
#include "print.h"

#include "symtab.h"

/* construct a new symbol table with n slots */
symtab makesymtab(int n){
    symtab z = malloc(sizeof *z);
    if (z){
        z->key = null;  // key int transitioning into this node
        z->value = null;  // associated value
        z->n = n;    // num slots in table
        z->tab = calloc(n, sizeof *z->tab);  // hashtable of child nodes
        z->prev = NULL;
    }
    return z;
}

symtab makesymtabchain(symtab root, int n){
    symtab z = makesymtab(n);
    z->prev = root;
    return z;
}

int hash(int x){
    return x^(x<<2);
    //return x^(x<<5)^(x<<14); // fill UCS 21bit space with 7bit ascii
}

/* common test clause in hashlookup */
#define RETURN_TAB_I_IF_EQ_K_OR_NULL \
    if (st->tab[i] == NULL || st->tab[i]->key == k) \
        return &st->tab[i]

/* compute hash,
   scan table */
symtab *hashlookup(symtab st, int k){
    int i;
    int h;
    unsigned int sz = st->n;

    h = hash(k) % sz;
    i = h;
    RETURN_TAB_I_IF_EQ_K_OR_NULL;    // test slot h
    for (++i; i < sz; i++)
        RETURN_TAB_I_IF_EQ_K_OR_NULL; // test slots [h+1..sz)
    for (i=0; i < h; i++)
        RETURN_TAB_I_IF_EQ_K_OR_NULL; // test slots [0..h-1]
    return NULL; // :not found
}

/* to rehash, we make a new table of the appropriate size,
   copy all non-null entries to new table
   steal the new table and update n */
void rehash(symtab st){
    int n = st->n * 7 + 11; // large growth to avoid thrashing,
                            // primes to avoid power-of-2 sizes
                            // for better distribution under modulus
                            // (maybe) (that's the idea, anyway)
    int i;
    symtab z=makesymtab(n); // allocate new table z->tab
    symtab *t = NULL;       // temp pointer

    for (i=0; i<st->n; i++){
        if (st->tab[i]){
            t = hashlookup(z, st->tab[i]->key);
            *t = st->tab[i];
        }
    }

    free(st->tab);    // free original table
    st->tab = z->tab; // steal new table
    st->n = n;        // update n
    free(z);          // free new block
}

/* find the associated node for a(n integer) string.
   string is passed by reference in case of prefix match,
   in which case the original string is updated to point
   to the unmatched remainder.
    mode=0: prefix match
    mode=1: defining search

    bias=0: define local
    bias=1: define global unless local def exists
   */
symtab findsym(symtab st, object **spp, int *n, int mode, int bias){
    symtab root = st;
    symtab last = st; // saved last-match value of st
#define sp (*spp)     // sp is an (int*) "by reference"
    int *lasp = sp;   // saved last-match pointer
    symtab *t = NULL; // temp pointer
    int nn = *n;      // working copy of n
    int lasn = nn;    // saved last-match value of n

    IFDEBUG(1, for (int i=0; i<*n; ++i)
                   print(sp[i], 0, 1); );

    while(nn--){
        t = hashlookup(st, *sp);
        if (!t) { // received NULL: table full
            rehash(st);
            t = hashlookup(st, *sp);
        }
        // t is now a pointer to a slot
        if (*t) { // slot not empty
            st = *t;
            sp++;
            if ((*t)->value != null){ // save partial match
                last = st;
                lasp = sp;
                lasn = nn;
            }
        } else { // slot empty
            switch(mode){
            case 0: // prefix search : return last partial match
                sp = lasp;
                *n = lasn;
                goto ret_last;
            case 1: // defining search
                if (bias && root->prev)
                    goto recurse;
                *t = calloc(1, sizeof(struct symtab));
                (*t)->tab = calloc((*t)->n = 11, sizeof(struct symtab));
                st = *t;
                lasn = nn;
                lasp = sp;
                last = st;
                st->key = *sp++;
                st->value = null;
                break;
            }
        }
    }

    //*n = nn+1; // undo nn-- and update n
    sp = lasp;
    *n = lasn;
ret_last:
    if (last == root && root->prev) //not-found::recurse down the chain.
recurse:
        return findsym(root->prev, spp, n, mode, bias);
    return last;  // return last-matched node
}
#undef sp

object getsym(symtab node){
    if (gettag(node->value)==MAGIC){
        magic m = getptr(node->value);
        return m->get(node);
    }
    return node->value;
}

void putsym(symtab node, object val){
    if (gettag(node->value)==MAGIC){
        magic m = getptr(node->value);
        m->put(node, val);
        return;
    }
    node->value = val;
}

void def(symtab st, object name, object v, int bias){
    symtab tab;
    switch(gettag(name)){
    default:
    case CHAR:
    case PCHAR:{
        int n = 1;
        object *p = &name;
        DEBUG(2,"%08x(%d,%d) = %08x(%d,%d)\n",
                name, gettag(name), getval(name),
                v, gettag(v), getval(v));
        tab = findsym(st,&p,&n,1,bias);
        } break;
    case PROG: {
        array na = getptr(name);
        int n = na->dims[0];
        object *p = na->data;
        tab = findsym(st,&p,&n,1,bias);
        } break;
    }
    putsym(tab, v);
}

object find(symtab st, object name){
    symtab tab;
    switch(gettag(name)){
    default:
    case CHAR:
    case PCHAR:{
        int n = 1;
        object *p = &name;
        tab = findsym(st, &p, &n, 0,0);
        } break;
    case PROG: {
        array na = getptr(name);
        int n = na->dims[0];
        object *p = na->data;
        tab = findsym(st, &p, &n, 0,0);
        } break;
    }
    return getsym(tab);
}

void (define_symbol_n)(symtab st, int n, ...){
    va_list ap;
    int key[n-1];
    object *p = key;

    va_start(ap,n);
    for (int i=0; i<n-1; ++i)
        key[i]=va_arg(ap,int);
    va_end(ap);
    --n;

    symtab tab = findsym(st,&p,&n,1,0);
    putsym(tab, va_arg(ap,int));
}

object (symbol_value_n)(symtab st, int n, ...){
    va_list ap;
    int key[n];
    object *p = key;

    va_start(ap,n);
    for (int i=0; i<n; ++i)
        key[i]=va_arg(ap,int);
    va_end(ap);

    symtab tab = findsym(st,&p,&n,0,0);
    return getsym(tab);
}


#ifdef TESTMODULE
#include "minunit.h"
int tests_run = 0;

#include <stdio.h>

struct symtab st = { .key = 0, .value = 0, .n = 10, .tab=(struct symtab *[10]){0} };

static char *test_put_get(){
    int array[] = {48,49,50};
    int *sym;
    int n;
    symtab t;

    sym = array;
    n = 3;
    t = findsym(&st,&sym,&n,1,0);
    //printf("%p\n",(void*)t);
    t->value = 42;

    sym = array;
    n = 3;
    t = findsym(&st,&sym,&n,0,0);
    //printf("%p\n",(void*)t);
    test_case(t->value != 42);
    printf("%d\n", n);
    test_case(n != 0);

    return 0;
}

static char *test_new_functions(){
    symtab st = makesymtab(10);
    define_symbol(st, 's','y','m','b', 42);

    test_case(symbol_value(st, 's','y','m','b') != 42);
    return 0;
}

static char *test_null_all_bits_zero(){
    char **calloc_ed_pointer = calloc(1,sizeof*calloc_ed_pointer);
    test_case(*calloc_ed_pointer!=NULL);
    free(calloc_ed_pointer);
    return 0;
}

static char *all_tests(){
    mu_run_test(test_null_all_bits_zero);
    mu_run_test(test_put_get);
    mu_run_test(test_new_functions);
    return 0;
}

int main() {

    char *result=all_tests();
    if (result != 0) {
        printf("%s\n",result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result != 0;

}
#endif //defined TESTMODULE

