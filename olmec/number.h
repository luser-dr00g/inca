#ifndef NUMBER_H
#define NUMBER_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <mpfr.h>
#include <gmp.h>

#include "common.h"

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

number_ptr new_number_z(char *str);
number_ptr new_number_fr(char *str);
int number_print_width(number_ptr num);
char *number_get_str(number_ptr num);

#endif
