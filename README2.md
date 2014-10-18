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
Whitespace may be introduced to disambiguate numbers from functions and operators,
in particular the 'dot' function or operator following an integer should include a space
to prevent a bad parse. A variable immediately followed by < left angle bracket is 
considered an assignment to the variable; but with an intervening space, the angle
bracket will be considered the less-than function.


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
omm is monadic operator yielding a monadic function, omd is monadic operator yielding dyadic function.

Having a separate monadic-operator/dyadic-function enables things like the "jot-less" jot-dot,
which follows its function. And the '=' operator which combines sensibly with '!' '=' '<' '>'.

I was side-tracked from this project by (unsuccessfully) attempting to make use of the vt220 
graphical character set with xterm.


Some examples illustrating composition of operators.

    ./inca2
            a=.a<~3
    1 0 0 
    0 1 0 
    0 0 1 

            a<.a<~4
    LENGTH ERROR

Remember, due to our "syntax", a variable immediately followed by left angle bracket, is
an assignment expression. This attempts to perform the dot ('.') function with an implicit
left argument (scalar 0) which is not conformable with iota-4 ('~4'). Hence, we add a
space. This "seals-off" the left-hand-side of the function call we're setting up.

            a <.a<~3
    0 1 1 
    0 0 1 
    0 0 0 

            a <=.a<~3
    1 1 1 
    0 1 1 
    0 0 1 

Only the left angle-bracket used as a comparison function requires this space when 
following a variable name.

            a>=.a<~3
    1 0 0 
    1 1 0 
    1 1 1 

            a+.a
    0 1 2 
    1 2 3 
    2 3 4 

            a*.a
    0 0 0 
    0 1 2 
    0 2 4 

Change the variable mid-stream, and the left-most one will get the new value.

            a*.a<1+a
    1 2 3 
    2 4 6 
    3 6 9 

            a
    1 2 3 

'!' is a comparison function. '!=' is an operator yielding the same comparison function.
'!=.' is an operator performing a jot-dot using the '!=' derived function.

            a!=.a
    0 1 1 
    1 0 1 
    1 1 0 

Operator compositions are somewhat kludged in the implementation, because they
rely upon some degree of "type-punning" between their syntax in the defining 
expression and in the executing expression. This part is very confusing, but
in some cases, adding extra parens will help the interpreter to understand
what you're trying to do. It looks for upto 4 parenthesized expressions 

    (a)(b)(c)(d)

If it finds 

    (function)(operator)

it builds a monadic operator (considering the function to be 'dyadic' even though
it was considered monadic a moment earlier. So you cannot build a "F Op F" dyadic
operator in the left-most position. This "instance" of the exec() must have 
parsed an "a" value that it considers *data* before it will construct a dyadic
operator. So you cannot do "(a)(F Op F)(b)" either, because this is the same problem
again, the "inner" exec that evaluates the parenthesized sub-expression has no
notion of "a" (a left-hand-side operand) and so it will attempt to build a monadic
operator and then get all confused.

If it finds

    (data)(function)(operator)

then it has to consider "d", too, to see if it's a function and determine how
the operator tree should be constructed. If parenthesized, it too will be 
evaluated by a recursive call to exec(), and then its type considered. If it finds

    (data)(function)(operator)(function)

it will build a dyadic operator and reduce to

    (data)(dy-op function)

where function is now our composed operator function or "derived" function, and then
keep scanning for a new "c". Otherwise if "d" is not a function, it will compose a
monadic-operator which yields a dyadic derived function.

    (data)(mon-op function)(data)

Certain monadic operators can "chain". By not having a dyadic form defined.
An example is the 'power' operator.
A single star is the 'times' function.
A star as a monadic operator is the 'power' operator. Thus,


            2*3
    6 

times.


            2**3
    8 

'times' power.


            2***3
    16 

'power' power.


            2****3
    65536 

'"power" power' power.

            2+*3
    6 

'plus' power, ie.  'times'.

            2+**3
    8 

"'plus' power" power, ie. 'times' power.

            2+***3
    16 

'"'plus' power" power' power, ie. 'power' power.

            2+****3
    65536 

and another one.


The full complement of 'figurative' transposes from inca "1" are available.
Where `-@` is a transpose about the horizontal axis and `|@` is a transpose
about the vertical axis, and `\/+><` indicate other axes and axis-compositions,
and `.@` represents the identity transform. `@` is also a monadic function (reverse)
and a dyadic function (rotate).

Inca2 also implements monadic and dyadic transpose functions using backtick.
Dyadic transpose takes a list of axes as left argument and yields a corresponding
rearrangment of the axes of the right argument.

The character data type is not well supported yet. *Update:* add char array syntax,
delimited by single-quotes.

Testing the old motivation, the tips distribution calculation. Divide 550 among
10 parties with integer-valued "shares" ranging from 10 to 19.

    $ ./inca2
            a*550%+/a<10+~9
    43.650794 48.015873 52.380952 56.746032 61.111111 65.476190 69.841270 74.206349 78.571429 

Integer division always "overflows" and triggers promotion to floating-point.
But the floor function (underscore) will yield integers (after truncating).

