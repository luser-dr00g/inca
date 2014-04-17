inca
====

Summary:
monadic functions: + id  { size  ~ iota  < box  # shape  > unbox  | abs  ! not  @ rev  
dyadic function: + add  { from  ~ find  < assign  # reshape  , cat  ; rowcat  - minus  . time  
&nbsp;&nbsp;    * pow  % divide  | mod  & and  ^ or  = eq  / compress  \ expand  
mon ops: / reduce  (.-|/\+><)@ transpose    dy op: . matrix product  
variable ` (backtick) is set to result of the previous command. (was underscore)

Based on the J-incunabulum,  
      http://www.jsoftware.com/jwiki/Essays/Incunabulum  
and extended to allow propagating specifications "a+2+a<3",
new functions minus,times,unbox. multi-digit integers.
identity element for monadic use of minus,times,cat.

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




