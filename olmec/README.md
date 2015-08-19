## Design

The problem I ran into with inca3 appears to be lack of overall design.
This was pointed out to me when my question on Programmers.SE gave a code smell.
http://programmers.stackexchange.com/questions/286898/are-there-any-problems-with-defining-a-single-source-module-in-c-using-conditi

So, starting over (yet again), inca4 will start with design. Simple,
flexible pieces design and built separately, in separate files. With unit testing.
The very start is UTF-8, the basis of the I/O module.
It has been reviewed extensively
(https://codereview.stackexchange.com/questions/98838/utf-8-encoding-decoding).


In fact, these first pieces shall be part of a library from which the
application will draw. This was a crowning achievement for my xpost Postscript interpreter.

Older than the Inca were the Olmec (according to the
Belgian anime The Mysterious Cities of Gold). I had already been moving towards
a library approach by rewriting the array manipulation functions several times
(both golfed and ungolfed -- golfing helps factoring). See 
https://github.com/luser-dr00g/inca/blob/master/arr.c ,
https://github.com/luser-dr00g/inca/blob/master/arrind.c and
https://github.com/luser-dr00g/inca/blob/master/arrg.c .
See also the write-ups on SO: 
http://stackoverflow.com/questions/30023867/how-can-i-work-with-dynamically-allocated-arbitrary-dimensional-arrays/30023868#300238 and
http://stackoverflow.com/questions/30409991/use-a-dope-vector-to-access-arbitrary-axial-slices-of-a-multidimensional-array . Further background in the threads in comp.lang.c linked in the SO answers.

With this effort, I now (I hope) have the tools to perform the more elaborate
array manipulations needed for APL functions.

I do eventually want to define a more compact encoding, using more 
values from the first byte for common APL symbols. But this will be an
optimization.

    Input/Output
     external | internal
     -------------------
    utf8      | ucs4
    ascii+esc |
    ...       |

The second problem with inca3 is too many types. Ick. So here, we start afresh.
Two types. That's it. Internally, the numeric type will polymorph and extend itself
in convenient ways. But from a semantic (design) point of view, it's a number 
(and thus various math functions are defined over it) or it ain't (and thus only 
'data' operations are defined over it).
We'll use ascii NUL for the null object == UCS4:0x00000000.

    Types
    -----
    char (ucs4)
    number (p-adic rational variable-precision)


In conjunction with the simplified type semantics, the basic scalar functions
will need to be custom-designed for the rational notation I intend to use.
The basic model I'm following is described sufficiently for pen-and-paper use
at https://en.wikipedia.org/wiki/Quote_notation and Eric Hehner's papers 
http://www.cs.toronto.edu/~hehner/ratno.pdf and http://www.cs.toronto.edu/~hehner/arith.pdf .
But a lot of work remains to implement it in C.

As described in the papers, the real trick is going to be efficiently recognizing
a repeated state of the computation.


    Scalar Functions 
    ------
    add
    sub
    mul
    div

Possible thought toward adding integer and boolean as separate datatypes. They're described
in the 1962 APL book.

