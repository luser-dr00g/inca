
## Adverbs and Conjunctions:
ie. monadic and dyadic operators.

symbol | adverb | desc | conjunction | desc 
--- | --- | --- | --- | --- 
& | _ | none | amp | compose functions or curry argument 
@ | _ | none | atop | compose functions 
/ | areduce | reduce using verb | _ | none 
\ | ascan | scan using verb | _ | none 
&#x2340; | abackscan | scan right-to-left using verb | _ | none 
&#x00a8; | _ | none | rank | derive new verb with specified or borrowed rank 



## monadic and dyadic Verbs:
ie. unary and binary functions

symbol | monadic | desc | dyadic | desc 
--- | --- | --- | --- | --- 
+ | vid | identity | vplus | add 
- | vneg | negate/negative | vminus | subtract 
&#x00af; | vneg | negative/negate | vminus | subtract 
&#x00d7; | vsignum | sign of | vtimes | multiply 
* | vsignum | sign of | vtimes | multiply 
&#x00f7; | vrecip | reciprocal | vdivide | divide 
&#x22c6; | _ | none | vpow | power 
&#x2223; | vabs | absolute value | vresidue | residue 
= | _ | none | veq | compare for equality 
&#x2260; | _ | none | vne | compare for inequality 
&#x2374; | vshapeof | yield dimension vector | vreshape | new array with specified dimensions populated by elements from right array 
$ | vshapeof | yield dimension vector | vreshape | new array with specified dimensions populated by elements from right array 
# | vtally | number of items | _ | none 
&#x2373; | viota | index generator | _ | none 
, | vravel | row-major-ordered vector of | vcat | catenate two arrays into vector 
; | vprenul | ? | vlink | cat and enclose 
{ | _ | none | vindexright | right is data and left is indices 
} | _ | none | vindexleft | left is data and right is indices 
&#x2191; | vhead | first element | vtake | first n elements 
&#x2193; | vbehead | all but the first | vdrop | all but first n elements 
/ | _ | none | vcompress | select from right according to bools in left 
\ | _ | none | vexpand | accumulate from right or zeros according to bools in left 
&#x22a5; | _ | none | vbase | interpret vector right using base left 
&#x22a4; | _ | none | vencode | produce encoded vector of value right according to base left 
&#x233d; | vreverse | reverse order of elements | vrotate | rotate through elements 
&#x2282; | vconceal | encode array into simple scalar | _ | none 
&#x2283; | vreveal | decode scalar into concealed array | _ | none 
&#x2361; | vnoresult | for testing | vnoresultd | for testing 
&#x2192; | vbranch | in del functions transfer to specified line | _ | none 
&#x2300; | _ | none | _ | none 


