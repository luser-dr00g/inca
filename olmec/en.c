/* the array data are always just straight 32bit integers.
   but we treat as a 7bit tag and 24bit integer value.

   this file defines the sub-typing of data items.
   */

#include <stdint.h>

#include "en.h"

int gettag(int d){
    if (d<0) return 0; /* negatives are literals */
    integer int32;
    int32.int32 = d;
    datum dat = int32.data;
    return dat.tag;
}

int getval(int d){
    if (d<0) return d;
    integer int32;
    int32.int32 = d;
    data dat = int32.data;
    return dat.val;
}

int newdata(int tag, int val){
    datum dat = { .tag = tag, .val = val };
    integer int32 = { .data = dat };
    return int32.int32;
}
