#include <stdint.h>
#include <stdlib.h>

typedef struct st {
    int key;
    int val;
    int n;
    struct st **tab /*[n]*/ ;
} *ST;

ST findsymb(ST st, int **spp, int *n, int mode);

ST makesymbtab(int n){
    ST z = malloc(sizeof *z);
    if (z){
        z->key = 0;
        z->val = 0;
        z->n = n;
        z->tab = calloc(n, sizeof *z->tab);
    }
    return z;
}

int hash(int x){
    return x^(x<<5)^(x<<14);
    return 0;
}

#define RETURN_TAB_I_IF_EQ_K_OR_NULL \
    if (st->tab[i] == NULL || st->tab[i]->key == k) \
        return &st->tab[i];

ST *hashlookup(ST st, int k){
    int i;
    int h;
    unsigned int sz = st->n;

    h = hash(k) % sz;
    i = h;
    RETURN_TAB_I_IF_EQ_K_OR_NULL
    for (++i; i < sz; i++)
        RETURN_TAB_I_IF_EQ_K_OR_NULL
    for (i=0; i < h; i++)
        RETURN_TAB_I_IF_EQ_K_OR_NULL
    return NULL;
}

void rehash(ST st){
    int n = st->n * 7 + 11;
    int i;
    ST z=makesymbtab(n);
    ST *t = NULL;
    for (i=0; i<st->n; i++){
        if (st->tab[i]){
            t = hashlookup(z, st->tab[i]->key);
            *t = st->tab[i];
        }
    }

    t = st->tab;
    st->tab = z->tab;
    st->n = n;
    free(t);
    free(z);
}

ST findsymb(ST st, int **spp, int *n, int mode){
    ST last = st;
#define sp (*spp)
    int *lasp = sp;
    ST *t = NULL;
    int nn = *n;
    int lasn = nn;

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
            if ((*t)->key==*sp){ // match
                last = st;
                lasp = sp;
                lasn = nn;
            }
        }
        else switch(mode){ // slot empty
        case 0: // prefix search : return last match
            sp = lasp;
            *n = lasn;
            return last;
        case 1: // defining search
            *t = calloc(1, sizeof(struct st));
            (*t)->tab = calloc((*t)->n = 11, sizeof(struct st));
            st = *t;
            st->key=*sp++;
            break;
        }
    }

    *n = nn+1;
    return last;
}
#undef sp

#ifdef TESTMODULE
#include "minunit.h"
int tests_run = 0;

#include <stdio.h>

struct st st = { .key = 0, .val = 0, .n = 10, .tab=(struct st *[10]){0} };

static char *test_put_get(){
    int array[] = {48,49,50};
    int *symb;
    int n;
    ST t;

    symb = array;
    n = 3;
    t = findsymb(&st,&symb,&n,1);
    //printf("%p\n",(void*)t);
    t->val = 42;

    symb = array;
    n = 3;
    t = findsymb(&st,&symb,&n,0);
    //printf("%p\n",(void*)t);
    test_case(t->val != 42);
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

