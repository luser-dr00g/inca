#ifndef NUMBER_H
#define NUMBER_H

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpfr.h>
#include <gmp.h>

#include "common.h"
#include "encoding.h"
#include "symtab.h"

enum numtag { NONE, Z, FR };

typedef union {
    enum numtag tag;
    struct z {
        enum numtag tag;
        mpz_t z;
    } z;
    struct fr {
        enum numtag tag;
        mpfr_t fr;
    } fr;
} u_number;
typedef u_number number[1];
typedef u_number *number_ptr;

extern object neginf;
extern object inf;

void init_number(symtab env);

void init_z(number_ptr z);
void init_fr(number_ptr fr);
number_ptr new_number_z(char *str);
number_ptr new_number_fr(char *str);
number_ptr new_number_lit(int lit);

number_ptr number_add(number_ptr x, number_ptr y);
number_ptr number_sub(number_ptr x, number_ptr y);
number_ptr number_mul(number_ptr x, number_ptr y);
number_ptr number_div(number_ptr x, number_ptr y);
number_ptr number_mod(number_ptr x, number_ptr y);

void number_promote(number_ptr n);
int number_print_width(number_ptr num);
char *number_get_str(number_ptr num);

#endif
