/* the array data are always just straight 32bit integers.
   but we treat as a 7bit tag and 24bit integer value.

   this file defines the sub-typing of data items.
   */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
    datum dat = int32.data;
    return dat.val;
}

int newdata(int tag, int val){
    datum dat = { .tag = tag, .val = val };
    integer int32 = { .data = dat };
    int x = int32.int32;
    printf("newdata %d %d %d\n", tag, val, x);
    return x;
}

integer nulldata = { .data = { .tag = NULLOBJ, .val = 0 } };
int null /* = nulldata.int32 */;
integer markdata = { .data = { .tag = MARK, .val = 0 } };
int mark /* = markdata.int32 */;

void init_en(){
    null = nulldata.int32;
    mark = markdata.int32;
}

size_t numused, nummax;
void **numtab;

size_t progused, progmax;
void **progtab;

size_t arrused, arrmax;
void **arrtab;

size_t symused, symmax;
void **symtabtab;

int addnewtocache(size_t *used, size_t *max, void ***data, void *ptr){
    if (*used == *max){
        *max = *max * 7 + 11;
        void *tmp = realloc(*data, *max * sizeof(void*));
        if (!tmp) return null;
        *data = tmp;
    }
    int z = (*used)++;
    (*data)[z] = ptr;
    printf("addnew %d %p %p\n", z, ptr, (*data)[z]);
    return z;
}

int cache(int tag, void *ptr){
    printf("cache %p\n", ptr);
    switch(tag){
        case LITERAL: return null;
        case CHAR: return null;
        case NUMBER:
            return newdata(tag, addnewtocache(&numused, &nummax, &numtab, ptr));
        case PROG:
            printf("cache prog\n");
            int x = newdata(tag, addnewtocache(&progused, &progmax, &progtab, ptr));
            printf("cache %d(%d,%d) %p\n", x, gettag(x), getval(x), getptr(x));
            return x;
        case ARRAY:
            printf("cache array\n");
            return newdata(tag, addnewtocache(&arrused, &arrmax, &arrtab, ptr));
        case SYMTAB:
            return newdata(tag, addnewtocache(&symused, &symmax, &symtabtab, ptr));
        case NULLOBJ: return null;
    }
}

void *getptr(int d){
    if (d<0) return NULL;
    switch(gettag(d)){
        case LITERAL: return NULL;
        case CHAR: return NULL;
        case NUMBER: return numtab[getval(d)];
        case PROG: return progtab[getval(d)];
        case ARRAY: return arrtab[getval(d)];
        case SYMTAB: return symtabtab[getval(d)];
        case NULLOBJ: return NULL;
    }
}

