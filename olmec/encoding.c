/*  Encoding
 *
 *  this file defines the sub-typing of data atoms.
 *  All data are packed into integer handles. The benefit for
 *  array operations is all data atoms will have a uniform
 *  size no matter what the content actually is. This replaces
 *  the intptr_t hackery (ab)used in earlier versions 
 *  (not portable to 64bit build). 
 *
 *  the array data are always just straight 32bit integers.
 *  but we treat as a 7bit tag and 24bit integer value.
 *  An immediate integer value is indicated by a negative
 *  sign-bit or all-zero tag. In essence, a 25bit sign/magnitude
 *  rep with no -0. This also means that we're not really using
 *  up all the available bits. Depending upon the final suite
 *  of distinct types and the desired "word size", this arrangement
 *  might be optimized further.
 *
 *  Composite objects (boxed or reference objects) have
 *  an associated pointer stored in an array associated
 *  with the tag. Thus an array object can be enclosed
 *  into a scalar (integer handle) with
 *
 *      int x;
 *      x = cache(ARRAY, array_new_dims(3,3)); //3x3 matrix
 *
 *  To better convey the abstract use of this integer type,
 *  we will make use of this typedef to designate such int-handles.
 *
 *  commont.h:
 *      typedef int object;
 *
 *  the array data structure (which is implicitly a pointer
 *  to its struct) can be retrived from the handle
 *  with
 *
 *      array a;
 *      a = getptr(x);
 *
 *  Most functions will need to check the types of their 
 *  arguments in order to determine how to proceed.
 *  This can be accomplished with `gettag()`.
 *
 *      switch(gettag(x)){
 *      case LITERAL: // handle atomic integer
 *          break;
 *      case ARRAY: {
 *          array X = getptr(x); 
 *      }
 *      }
 */

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "encoding.h"
#include "array.h"

int gettag(object d){
    if (d<0) return 0; /* negatives are literals */
    integer int32;
    int32.int32 = d;
    
    return int32.uint32 >> 24;
}

int getval(object d){
    if (d<0) return d;
    integer int32;
    int32.int32 = d;
    return int32.uint32 & ((1<<24)-1);
}

object newdata(int tag, int val){
    if (tag==LITERAL && val<0) return val;
    integer int32;
    int32.uint32 = ((unsigned)tag << 24) | ((unsigned)val & ((1<<24)-1));
    int x = int32.int32;
    DEBUG(3,"newdata %x(%d %d)\n", x, tag, val);
    return x;
}

integer nulldata;// = { .data = { .tag = NULLOBJ, .val = 0 } };
object null /* = nulldata.int32 */;
integer markdata;// = { .data = { .tag = MARKOBJ, .val = 0 } };
object mark /* = markdata.int32 */;
object nil;
object blank;

void init_en(void){
    nulldata.uint32 = newdata(NULLOBJ, 0);
    null = nulldata.int32;
    markdata.uint32 = newdata(MARKOBJ, 0);
    mark = markdata.int32;
    cache(LBRACOBJ, array_new_rank_dims(0));
    blank = newdata(CHAR, ' ');
}

int addnewtocache(size_t *used, size_t *max, void ***data, void *ptr){
    if (*used == *max){
        *max = *max * 7 + 11;
        void *tmp = realloc(*data, *max * sizeof(void*));
        if (!tmp) return null;
        *data = tmp;
    }
    int z = (*used)++;
    (*data)[z] = ptr;
    DEBUG(3,"addnew %d %p %p\n", z, ptr, (*data)[z]);
    return z;
}


struct memory_bank {
    size_t used, max;
    void **tab;
} memory_bank[LAST_INDEXED_TYPE - FIRST_INDEXED_TYPE + 1];

object cache(int tag, void *ptr){
    if (tag < FIRST_INDEXED_TYPE || tag > LAST_INDEXED_TYPE)
        return null;
    int idx = tag - FIRST_INDEXED_TYPE;
    return newdata(tag,
            addnewtocache(&memory_bank[idx].used,
                          &memory_bank[idx].max,
                          &memory_bank[idx].tab,
                          ptr));
}

void *getptr(object d){
    if (d<0) return NULL;
    int tag = gettag(d);
    if (tag < FIRST_INDEXED_TYPE || tag > LAST_INDEXED_TYPE)
        return NULL;
    int idx = tag - FIRST_INDEXED_TYPE;
    return memory_bank[idx].tab[getval(d)];
}


// fill returns a "blank" value for any type
// and identity elements for verbs
object getfill(object d){
    switch(gettag(d)){
        case PCHAR:
            switch(getval(d)){
            case '+':
	                      return 0;
            case MODE1('='):  // Times
            case MODE1('+'):  // Divided-By
            case '*':
                 return 1;
            } /*fallthru*/
        default:
        case LITERAL:
            return newdata(CHAR, 0x2300); //null
            return newdata(CHAR, 0x2316); //position
            return newdata(CHAR, 0x2218); //jot
            //return newdata(LITERAL, (1<<24)-1);
        case CHAR: return newdata(CHAR, ' ');
    }
}

