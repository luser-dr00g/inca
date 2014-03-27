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

Implements monadic functions 

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

dyadic functions 

     + plus 
     { from 
     ~ find
     < assign (not really a function, but an interpreter action) 
     # reshape 
     , cat 
     - minus 
     . times 
     * power
     % divide 
     | modulus 
     & and 
     ^ or 
     = equals?

monadic operator 

     / reduce  f/X  => x0 f (x1 f (x2 f ( ... xn-2 f xn-1)))

dyadic operator

     . matrix product Af.gW => f/Ag'W

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
