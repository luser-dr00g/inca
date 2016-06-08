//number.c
//$make number LDLIBS='-lmpfr -lgmp'

#include "number.h"

object neginf;
object inf;

object getprecision(symtab node){
    return newdata(LITERAL, (int)mpfr_get_default_prec());
}
void setprecision(symtab node, object val){
    switch(gettag(val)){
    case LITERAL: mpfr_set_default_prec(getval(val)); break;
    case NUMBER: {
        number_ptr num = getptr(val);
        switch(num->tag){
        case Z: mpfr_set_default_prec(mpz_get_si(num->z.z)); break;
        case FR: mpfr_set_default_prec(mpfr_get_si(num->fr.fr, MPFR_RNDN)); break;
        }
    } break;
    default: printf("bad type in setprecision()"); break;
    }
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
    char *str;
    switch(num->tag){
    case Z: str = mpz_get_str(NULL, 10, num->z.z);
            break;
    case FR: {
         int n = mpfr_snprintf(NULL, 0, "%Rf", num->fr.fr);
         str = malloc(n+1);
         mpfr_snprintf(str, n+1, "%Rf", num->fr.fr);
         /*
         mpfr_exp_t exp;
         str = mpfr_get_str(NULL, &exp, 10, 0, num->fr.fr, MPFR_RNDN);
         DEBUG(2, "exp = %lld\n", (long long)exp);
         int n = strlen(str);
         if (exp > n) return str;
         char *tmp = malloc(n + 2);
         int i=0;
         exp += str[0]=='-';
         for (;i<exp;++i)
             tmp[i] = str[i];
         tmp[i]='.';
         for (;i<n;++i)
             tmp[i+1] = str[i];
         tmp[n] = 0;
         for (int i=n-1; i>1; --i)
             if (tmp[i]=='0')
                 tmp[i]=0;
             else
                 break;
         mpfr_free_str(str);
         str = tmp;
         break;
         */
     }
    }
    return str;
}

int number_print_width(number_ptr num){
    char *str = number_get_str(num);
    int len = strlen(str);
    return len;
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
}

#endif
