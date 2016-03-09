/* Encoding
 *
 * this file defines the sub-typing of data atoms.
 * All data are packed into integer handles. The benefit for
 * array operations is all data atoms will have a uniform
 * size no matter what the content actually is. This replaces
 * the intptr_t hackery (ab)used in earlier versions 
 * (not portable to 64bit build). 

 * the array data are always just straight 32bit integers.
 * but we treat as a 7bit tag and 24bit integer value.
 * An immediate integer value is indicated by a negative
 * sign-bit or all-zero tag.

 * Composite objects (boxed or reference objects) have
 * an associated pointer stored in an array associated
 * with the tag. Thus an array object can be enclosed
 * into a scalar (integer handle) with

       int x;
       x = cache(ARRAY, array_new_dims(3,3)); //3x3 matrix

 * the array data structure (which is implicitly a pointer
 * to its struct) can be retrived from the handle
 * with

       array a;
       a = getptr(x);
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "encoding.h"
#include "debug.h"

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
    datum dat = int32.data;
    return dat.val;
}

int newdata(int tag, int val){
    if (tag==LITERAL && val<0) return val;
    datum dat = { .tag = tag, .val = val };
    integer int32 = { .data = dat };
    int x = int32.int32;
    DEBUG(2,"newdata %d %d %d\n", tag, val, x);
    return x;
}

integer nulldata = { .data = { .tag = NULLOBJ, .val = 0 } };
int null /* = nulldata.int32 */;
integer markdata = { .data = { .tag = MARKOBJ, .val = 0 } };
int mark /* = markdata.int32 */;

void init_en(){
    null = nulldata.int32;
    mark = markdata.int32;
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
    DEBUG(2,"addnew %d %p %p\n", z, ptr, (*data)[z]);
    return z;
}

size_t numused, nummax;
void **numtab;

size_t progused, progmax;
void **progtab;

size_t arrused, arrmax;
void **arrtab;

size_t symused, symmax;
void **symtabtab;

size_t verbused, verbmax;
void **verbtab;

size_t advused, advmax;
void **advtab;

size_t xvbused, xvbmax;
void **xvbtab;

int cache(int tag, void *ptr){
    DEBUG(2,"cache %p\n", ptr);
    switch(tag){
        default:
        case LITERAL: 
        case CHAR: return null;
        case NUMBER:
            return newdata(tag,
                    addnewtocache(&numused, &nummax, &numtab, ptr));
        case PROG: {
            DEBUG(2,"cache prog\n");
            int x = newdata(tag,
                    addnewtocache(&progused, &progmax, &progtab, ptr));
            DEBUG(2,"cache %d(%d,%d) %p\n", x, gettag(x), getval(x), getptr(x));
            return x;
        }
        case ARRAY:
            DEBUG(2,"cache array\n");
            return newdata(tag,
                    addnewtocache(&arrused, &arrmax, &arrtab, ptr));
        case SYMTAB:
            return newdata(tag,
                    addnewtocache(&symused, &symmax, &symtabtab, ptr));
        case VERB:
            return newdata(tag,
                    addnewtocache(&verbused, &verbmax, &verbtab, ptr));
        case ADVERB:
            return newdata(tag,
                    addnewtocache(&advused, &advmax, &advtab, ptr));
        case XVERB:
            return newdata(tag,
                    addnewtocache(&xvbused, &xvbmax, &xvbtab, ptr));
        case NULLOBJ: return null;
    }
}

void *getptr(int d){
    if (d<0) return NULL;
    switch(gettag(d)){
        default:
        case LITERAL:
        case CHAR: return NULL;
        case NUMBER: return numtab[getval(d)];
        case PROG: return progtab[getval(d)];
        case ARRAY: return arrtab[getval(d)];
        case SYMTAB: return symtabtab[getval(d)];
        case VERB: return verbtab[getval(d)];
        case ADVERB: return advtab[getval(d)];
        case XVERB: return xvbtab[getval(d)];
        case NULLOBJ: return NULL;
    }
}

#define MODE1(x) (x|1<<7)
// fill returns a "blank" value for any type
// and identity elements for verbs
int getfill(int d){
    switch(gettag(d)){
        case PCHAR:
            switch(getval(d)){
            case '+':
                return 0;
            case MODE1('='): 
            case MODE1('+'): 
            case '*':
                 return 1;
            } /*fallthru*/
        default:
        case LITERAL: return newdata(LITERAL, (1<<24)-1);
        case CHAR: return newdata(CHAR, ' ');
    }
}

