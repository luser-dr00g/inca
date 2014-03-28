inca
====

based on the J-incunabulum,
      http://www.jsoftware.com/jwiki/Essays/Incunabulum
lightly extended to allow propagating specifications "a+2+a<3",
new functions minus,times,unbox. multi-digit integers.
identity element for monadic use of minus,times,cat.

The file inca.c compiles for me with cygwin32 gcc.
The code does not consistently adhere to any particular
version of the C standard, and may fail to compile with -pedantic,
or any other options to enforce standards-conformance.

I first found the J incunabulum through this SO question:
http://stackoverflow.com/questions/13827096/how-can-i-compile-and-run-this-1989-written-c-program
And I've added links to various explanatory pages as comments to that question
(click "add/show more comments" under the question).

Implements monadic functions m  mW

     + identity 
     { size 
     ~ iota 
     < box 
     # shape 
     > unbox 
     | absolute 
     ! not 
     ' transpose
     @ reverse

dyadic functions d  AdW

     + plus   (monadic: a=0)
     { from 
     ~ find
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
     / compress
     \ expand

monadic operator 

     / reduce  f/X  => x0 f (x1 f (x2 f ( ... xn-2 f xn-1)))

dyadic operator

     . matrix product Af.gW => f/Ag'W
        eg. plus over times:  +..
            plus over plus:   +.+

over multidigit numbers and variables
     '_'(underscore), '`'(backtick), and a-z 
`'_'`(underscore) is set to the result of the previous line. 

The interpreter also implements a non-greedy "cat" for 
number vectors separated by spaces. Hence `1 2 3+~3` => `1 3 5`
where `~` is the zero-based iota. Spaces must only be used between
numbers. You may not pad operators with extra space or it will be
misinterpreted.

If the length of the command string exceeds 998 characters,
the behavior is undefined.

If array operands have incompatible sizes, the behavior
is undefined.

Example session: monadic functions.

    josh@Z1 ~/inca
    $ ./inca
            +5

    5 
            {1 2 3
    1 
    3 
            ~9
    9 
    0 1 2 3 4 5 6 7 8 
            <12

    < 
    12 
            #1 2 3;4 5 6
    2 
    2 3 
            ><12

    12 
            |-25

    25 
            !4

    0 
            !0

    1 
            '1 2 3;4 5 6;7 8 9;10 11 12
    3 4 
    1 4 7 10 
    2 5 8 11 
    3 6 9 12 
            @_
    3 4 
    12 9 6 3 
    11 8 5 2 
    10 7 4 1 


Example session: dyadic functions and operators.

    josh@Z1 ~/inca
    $ ./inca
            2 3+7 6
    2 
    9 9 
            1{52 53 54

    53 
            5~4+~9

    1 
            4+~9
    9 
    4 5 6 7 8 9 10 11 12 
            a<10

    10 
            b+a+b<6

    22 
            b

    6 
            ~64
    64 
    0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 
            8 8#_
    8 8 
    0 1 2 3 4 5 6 7 
    8 9 10 11 12 13 14 15 
    16 17 18 19 20 21 22 23 
    24 25 26 27 28 29 30 31 
    32 33 34 35 36 37 38 39 
    40 41 42 43 44 45 46 47 
    48 49 50 51 52 53 54 55 
    56 57 58 59 60 61 62 63 
            _,~9
    73 
    0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 0 1 2 3 4 5 6 7 8 
            12 6#_
    12 6 
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

    -10 
            2*3

    8 
            3*2

    9 
            *3

    8 
            **3

    256 
            2*2*3

    256 
            +/~9

    36 
            ./~9

    0 
            ./1+~9

    362880 
            ./1+~4 

    24 
            1.2.3.4

    24 
            1 2 3 4+..5 6 7 8
    1 
    70 
            1.5

    5 
            _+2.6

    17 
            _+3.7

    38 
            _+4.8

    70 


