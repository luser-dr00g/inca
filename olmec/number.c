//number.c
//$make number LDLIBS='-lmpfr -lgmp'

#include "array.h"
#include "number.h"

object neginf;
object inf;

object getprecision(symtab node){
    //printf("getprecision()\n");
    return newdata(LITERAL, (int)mpfr_get_default_prec());
}
void setprecision(symtab node, object val){
    //printf("setprecision()\n");
retry:
    switch(gettag(val)){
    case LITERAL: mpfr_set_default_prec(getval(val)); break;
    case NUMBER: {
        number_ptr num = getptr(val);
        switch(num->tag){
        case Z: mpfr_set_default_prec(mpz_get_si(num->z.z)); break;
        case FR: mpfr_set_default_prec(mpfr_get_si(num->fr.fr, MPFR_RNDN)); break;
        }
    } break;
    case ARRAY: {
        array a = getptr(val);
        val = *elem(a,0);
        goto retry;
    }
    default: printf("bad type in setprecision()"); break;
    }
}

static int printprec;
static char *printfmt = NULL;

object getprintprec(symtab node){
    //printf("getprintprec()\n");
    return newdata(LITERAL, printprec);
}
void setprintprec(symtab node, object val){
    //printf("setprintprec()\n");
retry:
    switch(gettag(val)){
    case LITERAL: printprec = getval(val); break;
    case NUMBER: {
        number_ptr num = getptr(val);
        switch(num->tag){
        case Z: printprec = mpz_get_si(num->z.z); break;
        case FR: printprec = mpfr_get_si(num->fr.fr, MPFR_RNDN); break;
        } break;
    }
    case ARRAY: {
        array a = getptr(val);
        val = *elem(a,0);
        goto retry;
    }
    default: printf("bad type in setprintprec()"); return;
    }

    int n;
    printfmt = realloc(printfmt, (n = 1+ snprintf(NULL, 0, "%%.%dRf", printprec)));
    snprintf(printfmt, n, "%%.%dRf", printprec);
    //printf("using fmt:%s\n", printfmt);
}

void init_number(symtab env){
    number_ptr num = malloc(sizeof *num);
    double d = strtod("-inf", NULL);
    init_fr(num);
    mpfr_set_d(num->fr.fr, d, MPFR_RNDN);
    neginf = cache(NUMBER, num);

    num = malloc(sizeof *num);
    init_fr(num);
    d = strtod("inf", NULL);
    mpfr_set_d(num->fr.fr, d, MPFR_RNDN);
    inf = cache(NUMBER, num);

    magic m = malloc(sizeof *m);
    m->get = getprecision;
    m->put = setprecision;
    define_symbol(env, newdata(PCHAR, 0x2395),
        newdata(PCHAR, 'F'), newdata(PCHAR, 'P'), newdata(PCHAR, 'C'), 
        cache(MAGIC, m));

    m = malloc(sizeof *m);
    m->get = getprintprec;
    m->put = setprintprec;
    define_symbol(env, newdata(PCHAR, 0x2395),
        newdata(PCHAR, 'P'), newdata(PCHAR, 'P'),
        cache(MAGIC, m));
    setprintprec(NULL, 6);
}

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

number_ptr new_number_lit(int lit){
    number_ptr num = malloc(sizeof *num);
    num->z.tag = Z;
    mpz_init_set_si(num->z.z, lit);
    return num;
}

number_ptr number_add(number_ptr x, number_ptr y){
    number_ptr z = malloc(sizeof *z);
    switch(x->tag){
    case Z:
        switch(y->tag){
        case Z: init_z(z); mpz_add(z->z.z, x->z.z, y->z.z); break;
        case FR: init_fr(z); mpfr_add_z(z->fr.fr, y->fr.fr, x->z.z, MPFR_RNDN); break;
        } break;
    case FR:
        switch(y->tag){
        case Z: init_fr(z); mpfr_add_z(z->fr.fr, x->fr.fr, y->z.z, MPFR_RNDN); break;
        case FR: init_fr(z); mpfr_add(z->fr.fr, x->fr.fr, y->fr.fr, MPFR_RNDN); break;
        } break;
    }
    return z;
}

number_ptr number_sub(number_ptr x, number_ptr y){
    number_ptr z = malloc(sizeof *z);
    switch(x->tag){
    case Z:
        switch(y->tag){
        case Z: init_z(z); mpz_sub(z->z.z, x->z.z, y->z.z); break;
        case FR: init_fr(z); mpfr_z_sub(z->fr.fr, x->z.z, y->fr.fr, MPFR_RNDN); break;
        } break;
    case FR:
        switch(y->tag){
        case Z: init_fr(z); mpfr_sub_z(z->fr.fr, x->fr.fr, y->z.z, MPFR_RNDN); break;
        case FR: init_fr(z); mpfr_sub(z->fr.fr, x->fr.fr, y->fr.fr, MPFR_RNDN); break;
        } break;
    }
    return z;
}

number_ptr number_mul(number_ptr x, number_ptr y){
    number_ptr z = malloc(sizeof *z);
    switch(x->tag){
    case Z:
        switch(y->tag){
        case Z: init_z(z); mpz_mul(z->z.z, x->z.z, y->z.z); break;
        case FR: init_fr(z); mpfr_mul_z(z->fr.fr, y->fr.fr, x->z.z, MPFR_RNDN); break;
        } break;
    case FR:
        switch(y->tag){
        case Z: init_fr(z); mpfr_mul_z(z->fr.fr, x->fr.fr, y->z.z, MPFR_RNDN); break;
        case FR: init_fr(z); mpfr_mul(z->fr.fr, x->fr.fr, y->fr.fr, MPFR_RNDN); break;
        } break;
    }
    return z;
}

number_ptr number_div(number_ptr x, number_ptr y){
    number_ptr z = malloc(sizeof *z);
    switch(x->tag){
    case Z:
        number_promote(x);
        switch(y->tag){
        case Z: init_fr(z); mpfr_div_z(z->fr.fr, x->fr.fr, y->z.z, MPFR_RNDN); break;
        case FR: init_fr(z); mpfr_div(z->fr.fr, x->fr.fr, y->fr.fr, MPFR_RNDN); break;
        } break;
    case FR:
        switch(y->tag){
        case Z: init_fr(z); mpfr_div_z(z->fr.fr, x->fr.fr, y->z.z, MPFR_RNDN); break;
        case FR: init_fr(z); mpfr_div(z->fr.fr, x->fr.fr, y->fr.fr, MPFR_RNDN); break;
        } break;
    }
    return z;
}

number_ptr number_mod(number_ptr x, number_ptr y){
    number_ptr z = malloc(sizeof *z);
    switch(x->tag){
    case Z:
        switch(y->tag){
        case Z: init_z(z); mpz_mod(z->z.z, x->z.z, y->z.z); break;
        case FR: init_fr(z); number_promote(x);
                 mpfr_fmod(z->fr.fr, x->fr.fr, y->fr.fr, MPFR_RNDN); break;
        } break;
    case FR:
        switch(y->tag){
        case Z: number_promote(y); //fall-thru
        case FR: init_fr(z); mpfr_fmod(z->fr.fr, x->fr.fr, y->fr.fr, MPFR_RNDN); break;
        }
    }
    return z;
}

char *number_get_str(number_ptr num){
    char *fmt = printfmt;
    char *str;
    switch(num->tag){
    case Z: str = mpz_get_str(NULL, 10, num->z.z);
            break;
    case FR: {
         int n = mpfr_snprintf(NULL, 0, fmt, num->fr.fr);
         str = malloc(n+1);
         mpfr_snprintf(str, n+1, fmt, num->fr.fr);
     }
    }
    return str;
}

int number_print_width(number_ptr num){
    switch(num->tag){
    case Z:
        return mpz_sizeinbase(num->z.z, 10) + mpz_sgn(num->z.z)==-1;
    case FR:
        return mpfr_snprintf(NULL, 0, printfmt, num->fr.fr);
    }
}

void number_promote(number_ptr n){
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
            number_promote(A); \
            number_promote(B); \
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
    return 0;
}

#endif
