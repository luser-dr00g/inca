/*
 *  The central concept of encoding data is the use of the basic `int` type
 *  for "everything". We chop the 32 bits into an 8 bit tag[*] and 24 bit value.
 *  So we can't deal with literal numbers that are larger than 16.7 million 
 *  or so. 
 *
 *  An `int` which contains one of our encoded-integer values should be 
 *  declared `object` to convey this semantics to the reader.
 *  Conversely, having determined that an object's tag is LITERAL,
 *  code may feel free to treat it as a restricted-range integer value.
 *
 *  [*] since we treat negative numbers as encoding to themselves, in essence
 *  we only have a 7bit tag to play with.
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include "../ppnarg.h"

#define MODE1(x) (x|1<<7) //add hi bit of ascii char

typedef int object;

typedef union integer {
    uint32_t uint32;
    int32_t int32;
} integer;

enum tag {
    LITERAL, /* val is a 24-bit 2's comp integer */
    CHAR, /* val is a 21-bit Unicode code point padded with zeros */
    PCHAR, /* val is a an executable char */

    NUMBER, /* val is an index in the number table */
FIRST_INDEXED_TYPE = NUMBER,
    PROG, /* val is an (index to an) executable code fragment (array of PCHAR)*/

    ARRAY, /* val is a(n index to a) boxed array */
    SYMTAB, /* val is a(n index to a) symbol table */
    LBRACOBJ, /* val is an (index to an) array of the bracket contents */
    ANALYSIS,
    VERB, /* val is a(n index to a) verb object */

    ADVERB, /* val is a(n index to a) verb object */
    XVERB, /* val is a(n index to a) struct containing a verb and adverb */
LAST_INDEXED_TYPE = XVERB,

    MARKOBJ, /* val is irrelevant (s.b. 0) */
    NULLOBJ, /* val is irrelevant (s.b. 0) */
    LPAROBJ,

    RPAROBJ,
    SEMIOBJ,
    RBRACOBJ,
};

typedef struct array *array;

typedef struct verb *verb; // also used for adverbs
typedef object nilad(verb v);
typedef object monad(object w,verb v);
typedef object dyad(object a,object w,verb v);

typedef struct xverb *xverb;

typedef struct symtab *symtab;


#ifdef DEBUGMODE
    #define DEBUG(LVL,...) if (LVL<=DEBUGMODE) fprintf(stderr, __VA_ARGS__)
    #define IFDEBUG(LVL,...) do if (LVL<=DEBUGMODE) { __VA_ARGS__; } while(0)
#else
    #define DEBUG(...)
    #define IFDEBUG(...)
#endif

#endif
