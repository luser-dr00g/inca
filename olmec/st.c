#include <stdint.h>
#include <stdlib.h>

#include "st.h"

/* construct a new symbol table with n slots */
symtab makesymtab(int n){
    symtab z = malloc(sizeof *z);
    if (z){
        z->key = 0;  // key int transitioning into this node
        z->val = 0;  // associated value (pointer to struct symtab)
        z->n = n;    // num slots in table
        z->tab = calloc(n, sizeof *z->tab);  // hashtable of child nodes
    }
    return z;
}

int hash(int x){
    return x^(x<<5)^(x<<14); // fill UCS 21bit space with 7bit ascii
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
   string is passed by reference in case a prefix match,
   in which case the original string is updated to pointer
   to the unmatched remainder.
   */
symtab findsym(symtab st, int **spp, int *n, int mode){
    symtab last = st; // saved last-match value of st
#define sp (*spp)     // sp is an (int*) "by reference"
    int *lasp = sp;   // saved last-match pointer
    symtab *t = NULL; // temp pointer
    int nn = *n;      // working copy of n
    int lasn = nn;    // saved last-match value of n

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
            if ((*t)->key == *sp){ // match
                last = st;
                lasp = sp;
                lasn = nn;
            }
        } else {
            switch(mode){ // slot empty
            case 0: // prefix search : return last match
                sp = lasp;
                *n = lasn;
                return last;
            case 1: // defining search
                *t = calloc(1, sizeof(struct st));
                (*t)->tab = calloc((*t)->n = 11, sizeof(struct st));
                st = *t;
                st->key = *sp++;
                break;
            }
        }
    }

    *n = nn+1; // undo nn-- and update n
    sp = lasp;
    return last;  // return last-matched node
}
#undef sp

#ifdef TESTMODULE
#include "minunit.h"
int tests_run = 0;

#include <stdio.h>

struct st st = { .key = 0, .val = 0, .n = 10, .tab=(struct st *[10]){0} };

static char *test_put_get(){
    int array[] = {48,49,50};
    int *sym;
    int n;
    symtab t;

    sym = array;
    n = 3;
    t = findsym(&st,&sym,&n,1);
    //printf("%p\n",(void*)t);
    t->val = (int[]){42};

    sym = array;
    n = 3;
    t = findsym(&st,&sym,&n,0);
    //printf("%p\n",(void*)t);
    test_case(*((int*)t->val) != 42);
    test_case(n != 0);

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

