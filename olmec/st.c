#include <stdlib.h>

typedef struct st {
    int key;
    int val;
    int n;
    struct st **tab /*[n]*/ ;
} *ST;

int hash(int x){
    //return 0;
    return x^(x<<4)^(x<<15);
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
    int n = st->n;
    //TODO
}

ST findsymb(ST st, int **sp, int *n, int mode){
    ST last = st;
    int *lasp = *sp;
    ST *t = NULL;
    int nn = *n;
    int lasn = nn;

    while(nn--){
        t = hashlookup(st, **sp);
        if (!t) {
            rehash(st);
            t = hashlookup(st, **sp);
        }
        // t is now a pointer to a slot
        if (*t) { // slot not empty
            st = *t;
            (*sp)++;
            if ((*t)->key==**sp){ // match
                last = st;
                lasp = *sp;
                lasn = nn;
            }
        }
        else switch(mode){ // slot empty
        case 0: // prefix search : return last match
            *sp = lasp;
            *n = lasn;
            return last;
        case 1: // defining search
            *t = calloc(1, sizeof(struct st));
            (*t)->tab = calloc((*t)->n = 11, sizeof(struct st));
            st = *t;
            (*sp)++;
            break;
        }
    }

    return st;
}

#ifdef TESTMODULE
#include "minunit.h"
int tests_run = 0;

struct st st = { .key = 0, .val = 0, .n = 10, .tab=(struct st *[10]){0} };

static char *test_put_get(){
    int array[] = {48,49,50};
    int *symb;
    int n = 3;
    ST t;

    symb = array;
    t = findsymb(&st,&symb,&n,1);
    t->key = 42;

    symb = array;
    t = findsymb(&st,&symb,&n,0);
    test_case(t->key != 42);

    return 0;
}

static char *all_tests(){
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

