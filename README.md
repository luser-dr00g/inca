inca
====

based on the J-incunabulum,
      http://www.jsoftware.com/jwiki/Essays/Incunabulum
lightly extended to allow propagating specifications "a+2+a=3",
new functions minus,times,unbox.
identity element for monadic use of minus,times,cat.

Implements monadic functions 

     + identity 
     { size 
     ~ iota 
     < box 
     # shape 
     > unbox 
     | absolute 
     ! not 

dyadic functions 

     + plus 
     { from 
     < assign (not really a function, but an interpreter action) 
     # reshape 
     , cat 
     - minus 
     * times 
     % divide 
     | modulus 
     & and 
     ^ or 

monadic operator 

     / reduce 

over multidigit numbers and variables `'_'`, `'`'`, and `"a-z"` 
`'_'` is set to the result of the previous line. 

The interpreter also implements a non-greedy "cat" for 
number vectors separated by spaces. Hence `1 2 3+~3` => `1 3 5`
where `~` is the zero-based iota. 

