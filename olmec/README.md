## Design

The problem I ran into with inca3 appears to be lack of overall design.
So, starting over (yet again), inca4 will start with design. Simple,
flexible pieces. The very start is UTF-8, the basis of the I/O module.
It has been reviewed extensively
(https://codereview.stackexchange.com/questions/98838/utf-8-encoding-decoding).

In fact, these first pieces shall be part of a library from which the
application will draw. Older than the Inca were the Olmec (according to the
Belgian anime The Lost Cities of Gold).

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

