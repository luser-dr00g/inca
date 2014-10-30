inca
====

This document describes the interpreter implemented in inca2.c.

Additional relevant material has been posted to nntp:comp.lang.apl in various threads:

inca at a turning point
https://groups.google.com/d/topic/comp.lang.apl/7yN-7yGUFiY/discussion

automatic conversion to floating-point on integer overflow
https://groups.google.com/d/topic/comp.lang.apl/Lu2tgqanK5Q/discussion

jot-dot again
https://groups.google.com/d/topic/comp.lang.apl/zCPJ8KaZyWg/discussion

Inca2 implements homogenous arrays of arbitrary dimension in several datatypes

    CHR
    INT
    DBL
    BOX
    FUN (operators)

and automatically promotes integer arrays to double in response to arithmetic overflow.
But it does not (yet) implement any form of "user function call" which got me into
so much trouble with the first version. But you can assign functions to variables,
and interpolate them in a command. eg.

    $./inca2
            a<3
    3 
            b<+
    +
            c<4
    4 
            abc
    7 

This works by "type-punning" the function as described further below regarding
operator compositions. It is parsed as a monadic function, and upon finding no
right-argument, is simply returned.  In the 'abc' command, however, it is
interpolated, discovered to be -- in fact -- a dyadic function in correct
position, and called.


All numbers must begin with a digit in order to be recognized by the scanner. ie.

    0.5

is a number, but

    .5

attempts to call the DOT function monadically (which does not exist, and so uses
the dyadic function with its "identity element", currently 0.0) upon the integer 5.
Whitespace may be introduced to disambiguate numbers from functions and operators,
in particular the 'dot' function or operator following an integer should include a space
to prevent a bad parse (as just illustrated).

A variable immediately followed by < left angle bracket is 
considered an assignment to the variable; but with an intervening space, the angle
bracket will be considered the less-than function.


The command characters are defined by this table:


    #define FUNCNAME(name,      c,    id,  vd,         vm,         odd,   omm,         omd) name,
    #define FUNCINFO(name,      c,    id,  vd,         vm,         odd,   omm,         omd) \
                    {           c,    id,  vd,         vm,         odd,   omm,         omd},
    #define FTAB(_) \
                    _(NOP,      0,    0.0, 0,          0,          0,     0,           0) \
                    _(EXCL,    '!',   0.0, notequal,   not,        0,     notopm,      notopmd) \
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
                    _(RANG,    '>',   0.0, greater,    unbox,      0,     0,           0) \
                    _(QUEST,   '?',   0.0, 0,          0,          0,     0,           0) \
                    _(AT,      '@',   0.0, rotate,     reverse,    0,     transposeop, 0) \
                    _(LBRAC,   '[',   0.0, minimum,    flr,        0,     0,           0) \
                    _(BKSLASH, '\\',  0.0, expand,     0,          0,     scan,        0) \
                    _(RBRAC,   ']',   0.0, maximum,    ceiling,    0,     0,           0) \
                    _(CARET,   '^',   M_E, powerf,     0,          0,     0,           0) \
                    _(HBAR,    '_',   0.0, 0,          0,          0,     firstaxis,   0) \
                    _(BKQUOTE, '`',   0.0, transposed, transposem, 0,     0,           0) \
                    _(LCURL,   '{',   0.0, from,       size,       0,     0,           0) \
                    _(VBAR,    '|',   0.0, modulus,    absolute,   0,     0,           0) \
                    _(RCURL,   '}',   0.0, commentd,   commentm,   0,     0,           0) \
                    _(TILDE,   '~',   0.0, find,       iota,       0,     0,           0) \
                    _(NFUNC,   0,     0.0, 0,          0,          0,     0,           0) \
    /* END FTAB */
    enum{FTAB(FUNCNAME)};
    struct{             C c; D id;
               ARC(*vd)(ARC,        ARC);
               ARC(*vm)(            ARC);
              ARC(*odd)(ARC, I,  I, ARC);
              ARC(*omm)(     I,     ARC);
              ARC(*omd)(ARC, I,     ARC);
    }ftab[]={ FTAB(FUNCINFO) };


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

    (function)(operator)(data)

it builds a monadic operator (considering the function to be 'dyadic' even though
it was considered monadic a moment earlier.

If it finds

    (function)(operator)(function)

where operator has a defined dyadic form, it will build the dyadic operator and
reduce to a single function. If there is no defined dyadic operator, but there is
a defined monadic operator, it will build the monadic operator and reduce.

If the leftmost object is not a function, it begins building a dyadic function call.
It scans ahead to see if "c" is an operator (so it can reduce) or a monadic function
or data (so it can recurse).

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
9 parties with integer-valued "shares" ranging from 10 to 18.

    $ ./inca2
            a*550%+/a<10+~9
    43.650794 48.015873 52.380952 56.746032 61.111111 65.476190 69.841270 74.206349 78.571429 

Integer division always "overflows" and triggers promotion to floating-point.
But the floor function (underscore) will yield integers (after truncating).

*Update:* I've just discovered a comment feature that's already implemented. The exec() function
will only build space-delimited vectors of number-typed objects. So you can write comments as
strings immediately following the expression. It's simply discarded.


    $ ./inca2
            12'a number'
    12 

    <@3>
            12 'a number'
    12 

    <@3>

The `<@...>` output is the garbage collector report of how many temporary arrays
have been discarded. It runs every time through the main loop.

*Update:* Right curly bracket `}` is now the comment character. Called dyadically
it yields the left arg and does not evaluate the right arg. Called monadically,
it ABORTs back to the main loop.

*Update:* Changed minimum/floor function from underscore to left square bracket,
so maximum/ceiling can have the corresponding right square bracket. Underscore 
is now the 'last-axis' operator, equivalent to an overtyped hyphen in APL.

*Update:* Can now build dyadic operators in the leftmost position!

    $ ./inca2
            a<+.*
    "+.*

    <@1>
            (~3)a~3
    5 

    <@34>
            ~3
    0 1 2 

    <@3>
            (1*1)+(2*2)
    5 

    <@20>

*Update:* At long last, user functions. The new `;` (execute) monadic function
takes a char array argument.

Factorial function:

    josh@Z1 ~/inca
    $ ./inca2
            f<:;>(y!0){(<'1'),<'y*fy-1'
    ;>(y!0 

    ){(<1

    ),<y*fy-1



    <@1>
            f0
    1 

    <@15>
            f1
    1 

    <@38>
            f2
    2 

    <@61>
            f3
    6 

    <@84>
            f4
    24 

    <@107>
            f5
    120 

    <@130>

The tips formula can now be expressed as a tips-function.

    josh@Z1 ~/inca
    $ ./inca2
            a*550%+/a<15+2*~12
    26.442308 29.967949 33.493590 37.019231 40.544872 44.070513 47.596154 51.121795 54.647436 58.173077 61.698718 65.224359 

    <@91>
            a
    15 17 19 21 23 25 27 29 31 33 35 37 

    <@1>
            a*550%+/a
    26.442308 29.967949 33.493590 37.019231 40.544872 44.070513 47.596154 51.121795 54.647436 58.173077 61.698718 65.224359 

    <@82>
            t<:y*x%+/y
    y*x%+/y

    <@1>
            550ta
    26.442308 29.967949 33.493590 37.019231 40.544872 44.070513 47.596154 51.121795 54.647436 58.173077 61.698718 65.224359 

    <@83>
            550 t 15 17 19 21 23 25 27 29 31 33 35 37   
    26.442308 29.967949 33.493590 37.019231 40.544872 44.070513 47.596154 51.121795 54.647436 58.173077 61.698718 65.224359 

    <@106>

*Update:* Added file function. Monadic underscore with char argument opens a file. 
With numeric argument, reads from the file. 0: read single char. 1: read line.

    ./inca2
    <@3>
            _'lib.inca'
    unprintable type
    <@3>
            _1
    i<:0!1+~y


    <@4>
            _1
    o<:0=1+~y


    <@4>
            _1
    u<:('ox),'iy


    <@4>
            _1
    r<:(40.'iy);(1+~<y);((0{:").'iy);((0{:u).'iy);(@~<y);(41.'iy);((0{:;).'iy-1),0


    <@4>
            _1
    s<:;$,\@'ry


    <@4>
            _1
    t<:(y.(x.1000)%+/y)%1000


    <@4>
            _1
    :lib_loaded


    <@4>
            _1
    RANGE ERROR
    <@12>


