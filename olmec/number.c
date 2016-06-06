//number.c
//$make number LDLIBS='-lmpfr -lgmp'

#include "number.h"

void init_z(number_ptr z){
    z->z.tag = Z;
    mpz_init(z->z.z);
}

void init_fr(number_ptr fr){
    fr->fr.tag = FR;
    mpfr_init(fr->fr.fr);
}

number_ptr new_number_z(char *str){
    number_ptr num = malloc(sizeof *num);
    num->z.tag = Z;
    mpz_init(num->z.z);
    mpz_set_str(num->z.z, str, 10);
    return num;
}

number_ptr new_number_fr(char *str){
    number_ptr num = malloc(sizeof *num);
    num->fr.tag = FR;
    mpfr_init_set_str(num->fr.fr, str, 10, MPFR_RNDN);
    return num;
}

char *number_get_str(number_ptr num){
    char *str;
    switch(num->tag){
    case Z: str = mpz_get_str(NULL, 10, num->z.z);
            break;
    case FR: {
         mpfr_exp_t exp;
         str = mpfr_get_str(NULL, &exp, 10, 0, num->fr.fr, MPFR_RNDN);
         DEBUG(2, "exp = %lld\n", (long long)exp);
         int n = strlen(str);
         char *tmp = malloc(n + 2);
         int i=0;
         exp += str[0]=='-';
         for (;i<exp;++i)
             tmp[i] = str[i];
         tmp[i]='.';
         for (;i<n;++i)
             tmp[i+1] = str[i];
         tmp[n] = 0;
         mpfr_free_str(str);
         str = tmp;
         break;
     }
    }
    return str;
}

int number_print_width(number_ptr num){
    char *str = number_get_str(num);
    int len = strlen(str);
    return len;
}

void promote(number_ptr n){
    mpz_t t;
    memcpy(&t, &n->z.z, sizeof t);
    init_fr(n);
    mpfr_set_z(n->fr.fr, t, MPFR_RNDN);
    mpz_clear(t);
}

#ifdef TESTMODULE
int tests_run;

#define op(func, C, A, B) \
    if ((A)->tag==Z && (B)->tag==Z) { \
        if (!strcmp(#func,"div")) { \
            init_fr(C); \
            promote(A); \
            promote(B); \
            mpfr_##func((C)->fr.fr, (A)->fr.fr, (B)->fr.fr, MPFR_RNDN); \
        } else { \
            init_z(C); \
            mpz_##func((C)->z.z, (A)->z.z, (B)->z.z); \
        } \
    } else if ((A)->tag==FR && (B)->tag==FR) { \
        init_fr(C); \
        mpfr_##func((C)->fr.fr, (A)->fr.fr, (B)->fr.fr, MPFR_RNDN); \
    }

void mpz_nothing(mpz_t c, const mpz_t a, const mpz_t b){
    mpz_set_ui(c,0);
}

void mpfr_nothing(mpfr_t c, const mpfr_t a, const mpfr_t b, mpfr_rnd_t rnd){
    mpfr_set_ui(c,0,rnd);
}

int main(){
    number a, b, c;
    char op[2];
    init_z(a);
    init_z(b);

    return 0;
    gmp_scanf("%Zd %1s %Zd", a->z.z, op, b->z.z);
    switch(*op){
    case '+': op(add, c, a, b); break;
    case '*': op(mul, c, a, b); break;
    case '-': op(sub, c, a, b); break;
    case '/': op(div, c, a, b); break;
    default: op(nothing, c, a, b);
    }
    switch(c->tag){
    case Z: gmp_printf("%Zd\n", c->z.z); break;
    case FR: mpfr_printf("%Rf\n", c->fr.fr); break;
    }
}

#endif
