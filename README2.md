inca
====

This document describes the interpreter implemented in inca2.c.

Inca2 implements homogenous arrays of arbitrary dimension in several datatypes

    CHR
    INT
    DBL
    BOX
    FUN (operators)

and automatically promotes integer arrays to double in response to arithmetic overflow.
All numbers must begin with a digit in order to be recognized by the scanner. ie.

    0.5

is a number, but

    .5

attempts to call DOT as a monadic function (which does not exist) upon the integer 5.


Various reorganizations in the basic structure of the program enable inca2 to offer
many more features than the first version, using much less code.

The command characters are defined by this table:

    #define FUNCNAME(name,      c,    id,  vd,         vm,         odd,   omm,         omd) name,
    #define FUNCINFO(name,      c,    id,  vd,         vm,         odd,   omm,         omd) \
                    {           c,    id,  vd,         vm,         odd,   omm,         omd},
    #define FTAB(_) \
                    _(NOP,      0,    0.0, 0,          0,          0,     0,           0) \
                    _(EXCL,    '!',   0.0, 0,          not,        0,     0,           0) \
                    _(DBLQUOTE,'"',   0.0, 0,          0,          0,     0,           0) \
                    _(HASH,    '#',   0.0, rsh,        sha,        0,     0,           0) \
                    _(DOLLAR,  '$',   0.0, or,         0,          0,     0,           0) \
                    _(PERCENT, '%',   1.0, divide,     0,          0,     0,           0) \
                    _(AND,     '&',   1.0, and,        0,          fog,   0,           0) \
                    _(QUOTE,   '\'',  0.0, 0,          0,          0,     0,           0) \
                    _(STAR,    '*',   1.0, times,      signum,     0,     0,           power) \
                    _(PLUS,    '+',   0.0, plus,       id,         0,     0,           0) \
                    _(COMMA,   ',',   0.0, cat,        ravel,      0,     0,           0) \
                    _(MINUS,   '-',   0.0, minus,      negate,     0,     0,           0) \
                    _(DOT,     '.',   0.0, dotf,       0,          dotop, 0,           jotdot) \
                    _(SLASH,   '/',   0.0, compress,   0,          0,     reduce,      0) \
                    _(COLON,   ':',   0.0, 0,          0,          0,     0,           0) \
                    _(SEMI,    ';',   0.0, 0,          0,          0,     0,           0) \
                    _(LANG,    '<',   0.0, less,       box,        0,     0,           0) \
                    _(EQUAL,   '=',   0.0, equal,      0,          0,     0,           eqop) \
                    _(RANG,    '>',   0.0, greater,    0,          0,     0,           0) \
                    _(QUEST,   '?',   0.0, 0,          0,          0,     0,           0) \
                    _(AT,      '@',   0.0, rotate,     reverse,    0,     transposeop, 0) \
                    _(LBRAC,   '[',   0.0, 0,          0,          0,     0,           0) \
                    _(BKSLASH, '\\',  0.0, expand,     0,          0,     scan,        0) \
                    _(RBRAC,   ']',   0.0, 0,          0,          0,     0,           0) \
                    _(CARET,   '^',   M_E, powerf,     0,          0,     0,           0) \
                    _(HBAR,    '_',   0.0, minimum,    flr,        0,     0,           0) \
                    _(BKQUOTE, '`',   0.0, transposed, transposem, 0,     0,           0) \
                    _(LCURL,   '{',   0.0, from,       size,       0,     0,           0) \
                    _(VBAR,    '|',   0.0, modulus,    absolute,   0,     0,           0) \
                    _(RCURL,   '}',   0.0, 0,          0,          0,     0,           0) \
                    _(TILDE,   '~',   0.0, find,       iota,       0,     0,           0) \
                    _(NFUNC,   0,     0.0, 0,          0,          0,     0,           0) \
    /* END FTAB */

where vd is dyadic verb, vm is monadic verb, odd is dyadic operator yielding dyadic (derived) function,
omm is monadic operator yielding a monadic function, omd is monadic operator yielding monadic function.

Having a separate monadic-operator/dyadic-function enables things like the "jot-less" jot-dot,
which follows its function. And the '=' operator which combines sensibly with '!' '=' '<' '>'.

I was side-tracked from this project by (unsuccessfully) attempting to make use of the vt220 
graphical character set with xterm.


