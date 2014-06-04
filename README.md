inca
====

This document describes the interpreter implemented in inca.c,
or inca "1". Current development is inca2.c, but there is
no documentation for it yet.

Summary:
monadic functions: + id  { size  ~ iota  < box  # shape  > unbox  | abs  ! not  @ rev  
dyadic function: + add  { from  ~ find  < assign  # reshape  , cat  ; rowcat  - minus  . time  
&nbsp;&nbsp;    * pow  % divide  | mod  & and  ^ or  = eq  / compress  \ expand  
mon ops: / reduce  (.-|/\+><)@ transpose    dy op: . matrix product  
variable ` (backtick) is set to result of the previous command. (was underscore)

An online version is available courtesy of Thomas Baruchel.
http://baruchel.hd.free.fr/apps/apl/inca/ 
which is awesome and even handles cut+paste.

The program is based on and directly derived from the J-incunabulum,  
      http://www.jsoftware.com/jwiki/Essays/Incunabulum  
and extended to allow propagating specifications "a+2+a<3",
new functions minus,times,unbox. multi-digit integers.
identity element for monadic use of minus,times,cat.
Most extensions have been incorporated "ad-hoc", with
an attempt to maintain consistency of style, balanced 
against a need (demand) for more commentary and more visible
type identifiers.

The name "inca" was chosen for its similarity to "incunabulum",
as well as its obvious (to me) decomposition "In C, A",
as well as the apparent similarity between array-structured data
and the ancient Incan data-storage device, the quipu.
http://en.wikipedia.org/wiki/Quipu

The file inca.c compiles for me with cygwin32 gcc.
The code does not consistently adhere to any particular
version of the C standard, and may fail to compile with -pedantic,
or any other options to enforce standards-conformance.

I first found the J incunabulum through this SO question:
http://stackoverflow.com/questions/13827096/how-can-i-compile-and-run-this-1989-written-c-program
And I've added links to various explanatory pages as comments to that question, and a bugfix 
for the original. Here are the helpful links:
http://www.jsoftware.com/papers/AIOJ/AIOJ.htm  
https://groups.google.com/d/msg/sayeret-lambda/Oxffk3aeUP4/QEuZocgVh5UJ  
http://archive.vector.org.uk/trad/v094/hui094_85.pdf  

Inca has been submitted for critique on comp.lang.c and and comp.lang.apl.
And *some* of the advice given has been followed. It has not been tested
with a 64-bit intptr_t. The code may make 32bit assumptions, although I've
tried to be careful not to do this. I may not have been completely succesful.
The basis of the interpreter is the ability to treat a pointer as an integer
and pack them in the same-sized fields. Additionally, it assumes that pointer
values may be distinguished from character values from their integer 
representations. This assumption is not guaranteed by the standard, but 
appears empirically to be true on my cygwin and ubuntu gnu linux testbeds.
The original code also assumed that these pointer values will be positive,
which is empirically *not* true on cygwin. Not sure about ubuntu, the code
was fixed to use abs(intptr) before the range::type comparison well before
it was ported.

Inca will also accept command-line arguments into the program. These are
available in the 'a' variable as a box-array of command-string arrays.
Also included with the distribution is the small lib.inca file which
accumulates a few functions that arose in postings to comp.lang.apl.
If inca is invoked thusly:

     ./inca `cat lib.inca`

Then the library can be executed by putting the box-array a in a box '<',
making it executeable '$', and executing it ';'.

               ;$<a

Which should respond with the friendly message:

     2:11 
     lib_loaded

With the simple code representation of small integers for operators,
ascii for variables and other non-operator punctuation, and pointers
(boxed arrays, which here are just scalars (array rank=0)), code can 
be generated and executed on the fly. The r function in lib.inca
illustrates this:

     r<:(40.'iy);(1+~<y);((0{:").'iy);((0{:u).'iy);(@~<y);(41.'iy);((0{:;).'iy-1),0

The basic structure here is a series of parenthesized expressions which are 
strung together with ; (rowcat). (40.'iy) makes a vector of y left parens.
(1+~<y) makes a boxed iota vector 1..y. (0{:") is the quote literal, used the
same way the 40 was for left paren. Since double-quote " is a function,
its code representation is not the same as its ascii representation, so we extract
it from a short code sequence which starts at the colon and goes until the closing
paren, so just the double-quote *function*. ((0{:u).'iy) is the same, but with 
the 'u' variable, which could be accessed by ascii, if you want to, or by pulling
it out of a short code sequence. (@~<y) gives a reversed box iota y-1..0. 
(41.'iy) is a vector of closing parens. And the final row is y-1 semicolons, and
a final terminating null.

This produces the transpose of several expresions (j"uk); with j and k varying.
And it can be executed by transposing, ravelling, making executable and then
executing. ;$,\@ which the s function does.

Executing the s function calls r to generate a procedure to produce a triangular
matrices by a run-length encoded representation, built using iotas, directly
into the form of function calls to the function u which takes two numbers 
and produces a row of the matrix.

Functions can also call themselves recursively.

Fibonacci function:

     f<:;(y>1){(<:1);<:('fy-1)+'fy-2

Factorial function:

     f<:;(y>1){(<:1);<:y.fy-1

In the run-length-encoded triangular matrix example, the rows were all the
same length, so rowcat has an easy time to combine rows. But here, the two
code sequences

     :1
     :('fy-1)+'fy-2

are different lengths. So we box them, so rowcat doesn't screw things up.
(FIXME: fix rowcat to pad unequal widths).

     <:1
     <:('fy-1)+'fy-2

rowcat them together `((...);...)`, select one using a boolean expression `(y>1){`, and execute
the resulting expression `;`.

And also *not* executing a command-string prints the string, so:

        :Hello World!
    Hello World!


Recalling the syntax for *executing* code from the command-line.

       ;$<a

This illustrates a means by which "multi-line" functions may
be conveniently constructed, via a confusing arrangement of
vaguely-defined concepts like "box" and "box-array".
The text lines of the file from `cat file` are, again,
defined in the variable a as a box-array, an array of pointers,
of command-strings. To execute the box-array as a whole, it
must be put into a single box. (Errmm. This feels like an 
artificial restriction, and it is with difficulty that I document
the current behavior of the implementation here...)

Following this pattern, one might define a multi-statement 
function in the same manner, as an executable box of a box-array
of command strings.

        f<$<(<:a<1);(<:b<2);(<:c<3)

Executing ;f should then be equivalent to the separate statements.

        a<1
        b<2
        c<3

Implements monadic functions m  mW

     + identity   +1  =>  1
     { size      1 1 1  =>  3
     ~ iota       ~9   =>   0 1 2 3 4 5 6 7 8
     < box        <1 1 1  =>   <1 1 1  (bad example)
     # shape      #~9  =>  9
     > unbox      ><1 1 1  =>   1 1 1
     | absolute    |-12   =>   12
     ! not         !0 1 0   =>   1 0 1
     @ reverse     @~9    =>   8 7 6 5 4 3 2 1 0
     : yield array of remaining command string
     ; execute command string array
     $ convert array to command-string type
     'w  call function w with y as right arg
         function may be a variable containing code (with colon :), eg. square
             s<:y.y
             's6
        36
         or a parenthesized expression, without colon :
             '(y.y)6
        36
                                                            

dyadic functions d  AdW

     + plus
     { from       2 3{@~9   =>   6 5
     ~ find       6 5~@~9   =>   2 3
     < assign if a is a var (not really a function, but an interpreter action) 
     # reshape
     , cat 
     ; rowcat
     - minus   (monadic: a=0)
     . times   (monadic: a=1)
     * power   (monadic: a=2)  <-- this will be e with floating-point
     % divide  (monadic: a=1)  <-- this will make more sense with floating-point
     | modulus   (reverse of C: w%a, divisor on the right)
     & and 
     ^ or 
     = equals?
     ! not-equal?
     : match
     < less-than   (if a is not a var, see assign above)
     / compress
     \ expand
     "w  call function w with x as left arg and y as right arg
         function may be a variable containing code (with colon :), eg x+1-y
             f<:x+1-y
             3"f2
        2
         or a parenthesized expression, without colon :
             3"(x+1-y)2
        2

monadic operators

     / reduce  f/W  => w0 f (w1 f (w2 f ( ... wn-2 f wn-1)))
     \ scan    f\W  => (w0 f w1), ((w0 f w1) f w2), ... ) f wn-2) f wn-1)
     @ transpose   .@  identity transpose
                   -@  vertical transpose
                   |@  horizontal transpose
                   \@  y=x transpose
                   /@  y=-x transpose
                   +@  horz then vert
                   >@  horz then y=x
                   <@  horz then y=-x

dyadic operator

     . matrix product Af.gW => f/Ag\@W
       ( @ for the left function designates "jot-dot", a null-scan over the matrix product )
        eg. plus over times:       +..
            plus over plus:        +.+
            addition table:        @.+
            multiplication table:  @..

over multidigit numbers and variables  
     <pre>`</pre> (backtick), and [a-z]   
<pre>`</pre> (backtick) is set to the result of the previous line.   (was underscore)

The interpreter also implements a non-greedy "cat" for 
number vectors separated by spaces. Hence `1 2 3+~3` => `1 3 5`
where `~` is the zero-based iota. Spaces must only be used between
numbers. You may not pad operators with extra space or it will be
misinterpreted.

If the length of the command string exceeds 998 characters,
the behavior is undefined.

If array operands have incompatible sizes, the behavior
is undefined.

Example sessions: 

monadic functions.

    josh@Z1 ~/inca
    $ !.
    ./inca
            +5

    5 
            {1 2 3
    1 
    3 
            ~9
    9 
    0 1 2 3 4 5 6 7 8 
            ~0
    0 

            <12

    < 
    12 
            #1 2 3;4 5 6
    2 
    2 3 
            |-25

    25 
            !4

    0 
            !0

    1 
            \@1 2 3;4 5 6;7 8 9;10 11 12
    3 4 
    1 4 7 10 
    2 5 8 11 
    3 6 9 12 
            \@4 3#1+~12
    3 4 
    1 4 7 10 
    2 5 8 11 
    3 6 9 12 
            @`
    3 4 
    12 9 6 3 
    11 8 5 2 
    10 7 4 1 

    josh@Z1 ~/inca
    $ 

Example session: dyadic functions and operators.  
(output has been augmented with a type-specifier then colon : then dims)

    $ ./inca
            2 3+7 6
    0:2 
    9 9 
            1{52 53 54
    0:
    53 
            5~4+~9
    0:
    1 
            4+~9
    0:9 
    4 5 6 7 8 9 10 11 12 
            a<10
    0:
    10 
            b+a+b<6
    0:
    22 
            b
    0:
    6 
            ~64
    0:64 
    0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
            8 8#`
    0:8 8 
    0 1 2 3 4 5 6 7 
    8 9 10 11 12 13 14 15 
    16 17 18 19 20 21 22 23 
    24 25 26 27 28 29 30 31 
    32 33 34 35 36 37 38 39 
    40 41 42 43 44 45 46 47 
    48 49 50 51 52 53 54 55 
    56 57 58 59 60 61 62 63 
            `,~9
    0:73 
    0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 0 1 2 3 4 5 6 7 8 
            12 6#`
    0:12 6 
    0 1 2 3 4 5 
    6 7 8 9 10 11 
    12 13 14 15 16 17 
    18 19 20 21 22 23 
    24 25 26 27 28 29 
    30 31 32 33 34 35 
    36 37 38 39 40 41 
    42 43 44 45 46 47 
    48 49 50 51 52 53 
    54 55 56 57 58 59 
    60 61 62 63 0 1 
    2 3 4 5 6 7 
            5.-2
    0:
    -10 
            2*3
    0:
    8 
            3*2
    0:
    9 
            *3
    0:
    8 
            **3
    0:
    256 
            2*2*3
    0:
    256 
            +/~9
    0:
    36 
            ./~9
    0:
    0 
            ./1+~9
    0:
    362880 
            ./1+~4
    0:
    24 
            1.2.3.4
    0:
    24 
            1 2 3 4+..5 6 7 8
    0:
    70 
            1.5
    0:
    5 
            `+2.6
    0:
    17 
            `+3.7
    0:
    38 
            `+4.8
    0:
    70 



Example session: general transpose operator.

    $ ./inca
            3 4#~12
    0:3 4 
    0 1 2 3 
    4 5 6 7 
    8 9 10 11 
            >@`
    0:4 3 
    8 4 0 
    9 5 1 
    10 6 2 
    11 7 3 
            \@`
    0:3 4 
    8 9 10 11 
    4 5 6 7 
    0 1 2 3 
            -@`
    0:3 4 
    0 1 2 3 
    4 5 6 7 
    8 9 10 11 
            <@`
    0:4 3 
    3 7 11 
    2 6 10 
    1 5 9 
    0 4 8 
            /@`
    0:3 4 
    8 9 10 11 
    4 5 6 7 
    0 1 2 3 
            -@`
    0:3 4 
    0 1 2 3 
    4 5 6 7 
    8 9 10 11 
            +@`
    0:3 4 
    11 10 9 8 
    7 6 5 4 
    3 2 1 0 
            |@`
    0:3 4 
    8 9 10 11 
    4 5 6 7 
    0 1 2 3 
            -@`
    0:3 4 
    0 1 2 3 
    4 5 6 7 
    8 9 10 11 
            .@`
    0:3 4 
    0 1 2 3 
    4 5 6 7 
    8 9 10 11 




