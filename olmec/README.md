## olmec

A work-in-progress APL interpreter intended for code-golfing.

Older than the Inca were the Olmec (according to the
Belgian anime The Mysterious Cities of Gold).

Where the earlier incarnations of the inca interpreter were inspired
by the 'quipu' which emphasizes links and pointers, olmec is guided
by simpler ideas such as abaci, counting-boards and pebbles.
The links are de-emphasized, and the values brought to the
surface. The implementation is composed of thin abstractions. The `int`s
are visible, moving around, representing things.

## Language

The language is built on APL ideas. 
Implementation has begun with the formal descriptions of basic vector
verbs and adverbs from
[A Generalization of APL](http://www.softwarepreservation.org/projects/apl/Books/AGENERALIZATIONOFAPL/view).
rho, iota, cat, compress, expand, reduce, scan, backscan, encode, decode, reverse,
rotate.

One major unique point is that anything not whitespace, a number, a paren,
a left-arrow, or a quote (beginning a quoted-string) is an identifier.
Any contiguous sequence of letters and symbols is an identifier.
The longest-defined prefix of an identifer is partitioned-off and the
remaining characters are scanned again and again for the next
longest-defined prefix until the string is exhausted.
So if you're defining a variable in the middle of an expression, make sure
it butts-up against a paren, number or add a space. Otherwise, you'll be
assigning a value to the entire identifer sequence.

This treatment of identifiers naturally supports quad-variables and
redefinition of functions and operators. It may also be possible to use
this feature for idiom-replacement.

The Quad-Minus variable `⎕-` swaps the meaning of minus and hi-minus. When set
to 0 (the default) you get the APL behavior that hi-minus is a decoration
for numbers designating a negative number, but regular minus is the
subtraction or negation *function*. When Quad-Minus is set to 1, then the
regular minus-sign is treated as the the decoration for numbers designating
a negative number, but hi-minus is now the *function*. This should
facilitate reading and interpreting of more common comma-separated
number-vector notations.

The Quad-k variable `⎕k` displays the APL characters accessible by holding
the ALT-key. The related Quad-a variable `⎕a` shows what olmec considers
to be the "normal" keyboard layout against which the characters are selected.

Consult the file `tables.md` for a listing of implemented adverbs and verbs.

## Running Environment/Building

The program should compile simply with `make`, assuming GNU make and a
working C99 compiler.

The program assumes it is running in a VT220-compatible terminal emulator
such as XTerm or equivalent (AFAIK all modern terminals are ports of XTerm)
with Unicode support and a US-ASCII keyboard layout.

Currently, the program only operates in an interactive mode, relying upon
the ALT-key to type APL characters. CTRL-n can also be used as an APL-lock
key to toggle the need to hold the ALT key. Type `⎕k` to print out the APL
keyboard.

Output can be selected and copied-out from the terminal, but utf-8 input is
not currently accepted, so you cannot paste expressions into the terminal
for the interpreter to execute.

## Design

A major simplifying assumption in the design of this project is:
"Everything is an `int`".
The encoding module packs any data type into an integer handle.
So `int` rules.

There are 4 major abstractions which are not ints,
each of which is a typedef of a pointer to the respective structure type:
array, verb, symtab, number. Each of these is constructed by functions or
macros to properly initialize all the members.

All the innards of these structures are exposed by their header files,
so there is encapsulation, but no information-hiding.

The atomic types defined in the encoding module all overlay normal (assumed
32bit) `int`s. So `int` is used as the external interface type. But various 
modules define typedefs to indicate the sematics that a variable is an 
encoded-int and not (necessarily) just a number. The scanner code calls
it a `typedef int token;`. The parser code calls it a `typedef int
object;`. The encoding code calls it a
`typedef union integer { datum data; int32_t int32; } integer;`.

A spiritual predecessor of sorts is
[sexp.c](https://github.com/luser-dr00g/sexp.c/blob/master/sexp.c)
which employs a similar integer encoding, permitting all functions
over compound data structures to be declared with integer parameter
and return types. Thus an object *represents* a value, but does not
necessarily *contain* its respective value(s). From the APL perspective,
this means all objects are boxed at the function-call interface.
Verbs are currently defined over narrow-ranged integers (24bit+sign)
or 1-D arrays, ie. vectors. 

Another ancestor is this toy "forth machine" interpreter: 
https://groups.google.com/forum/#!topic/comp.lang.c/WGSl7ERMu1U/discussion

Both of these build upon the unifying notion that all data is composed
of *cells* of a certain fixed size, motivating the use of `int` as a 
*unified* or `union` type. 

But much of the code comes from
[inca3](https://github.com/luser-dr00g/inca/blob/master/inca3.c)
which (ab)uses the same style as the incunabulum.

The `array.h` file defines the multidimensional array structure
and helper functions, storing `int`s. The `symtab.h` file defines
the symbol table structure and helper functions, mapping `int`
sequences to `int`. The `encoding.h` file defines the tag/value
partitioning of the `int` and the tables of pointers which map
`int` to `void *`, be it an `array` pointer or `verb` or `symtab`
or `xverb`. The xverb structure is used for those symbols which
may be verbs or adverbs depending upon context, such as `/` and `\`
which are verbs if preceded by a noun, but are adverbs if preceded
by a(n other) verb.

The "abstractions" as I keep calling them, are essentially *classes* in the OO
sense, but with somewhat less discipline. The `array.h` defines the
class structure and member-functions for the array type. Ditto for `symtab`
and `verb`. `adverb` is a derived type, distinct from verbs, but sharing
exactly the same implementation, And `xverb` is multiply-inherited
from both `verb` and `adverb`, although it is aware that they share the
same representation. 

An array is an object, with an internal `struct` representation and
*semi-atomic* `array` representation which is implicitly a pointer to the
struct, and a *fully-atomic* representation as an encoded integer.
The array structure and supporting functions use a dope-vector for
constructing complicated slices or shared arrays with re-ordered or
selected indices.

A verb is an object, with an internal `struct` representation and
*semi-atomic* `verb` representation which is implicitly a pointer to the
struct, and a *fully-atomic* representation as an encoded integer.
Verbs are all loaded into the default symbol table as single-character
identifiers, then adverbs are loaded, then xverbs (using the verb and
adverb definitions) are loaded.

An adverb is an object, with an internal `struct` representation and
*semi-atomic* `adverb` representation which is implicitly a pointer to the
struct, and a *fully-atomic* representation as an encoded integer.
Adverbs include conjunctions as *dyadic adverbs*, although the two
are distinguished as separate types by the grammar rules of the parser.

A symbol table is an object, with an internal `struct` representation and
*semi-atomic* `symtab` representation which is implicitly a pointer to the
struct, and a *fully-atomic* representation as an encoded integer.
The symbol table contains associations, or key/value pairs.
The 'key' is a sequence of encoded integers, which should usually have
of the PCHAR tag. The 'value' is a single encoded integer, although of course
almost anything can be packed-up into one of these abstraction and `cache`d
to produce the requisite encoded integer.

The number type allows arbitrary-precision floating-point and integer values,
The operations are handled by the GMP and MPFR libraries.

The top-level of execution then procedes with the stereotypical
*READ-EVAL-PRINT* loop, augmented with APL's behavior that definitions
are not printed by default. The *READ* stage calls the `editor.h` functions.
The *PRINT* stage calls the `print.h` functions. The *EVAL* stage calls
`lex.h` functions and `exec.h` functions. The `qn.h` functions are 
obsolescent. But it's sad to see them go.

The `io.h` functions implement utf-8 to/from ucs4 conversions, but
they are not currently used. When patched in, it should allow pasting
of expressions *into* the terminal window (copying out already works)
as well as execution of saved scripts. Currently only keyboard input
is accepted to select special APL characters, with hand-coded conversions
defined in `alpha.h` and used by the editor functions. So the full 
enormity of Unicode characters are not yet unleashed for the identifier
set, although the implementation has support (mostly) available for this.

## Notes, links, updates

The problem I ran into with inca3 appears to be lack of overall design.
This was pointed out to me when my question on Programmers.SE gave a code smell.
http://programmers.stackexchange.com/questions/286898/are-there-any-problems-with-defining-a-single-source-module-in-c-using-conditi

Another enormous problem with the **inca** series is the use of `intptr_t` as a
unified type, making assumptions about the ranges represented by pointers which
are empirically not workable on 64bit systems.

So, starting over (yet again), inca4 (aka `olmec`) will start with design. Simple,
flexible pieces designed and built separately, in separate files. With unit testing.
The very start is UTF-8, the basis of the I/O module.
It has been reviewed extensively
(https://codereview.stackexchange.com/questions/98838/utf-8-encoding-decoding).


In fact, these first pieces shall be part of a library from which the
application will draw. This was a crowning achievement for my xpost Postscript interpreter.

I had already been moving towards
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

The unit-tesing framework is described in this thread: 
https://groups.google.com/d/topic/comp.lang.c/ih2bq_w10Gw/discussion
and this SO Q/A pair:
http://stackoverflow.com/questions/32163935/compose-a-combined-test-suite-program-from-a-collection-of-unit-tests

I also have a thread about implementing the rational arithmetic functions:
https://groups.google.com/d/topic/comp.lang.c/GVCXmz_3KOA/discussion  
And a SO question: http://stackoverflow.com/questions/32529118/incorporate-repetition-detection-in-my-p-adic-arithmetic-loops
And a Math question: http://math.stackexchange.com/questions/1615290/is-there-a-better-representation-than-p-adics-for-exact-computer-arithmetic


--

Update. Got the editor working. You can type APL chars by holding the ALT key.


Code Reviews:  
http://codereview.stackexchange.com/questions/98838/utf-8-encoding-decoding  
http://codereview.stackexchange.com/questions/114243/unicode-capable-symbol-table-n-way-search-tree-with-hash-buckets  
http://codereview.stackexchange.com/questions/115748/termios-xterm-line-editor-for-apl-interpreter  
https://groups.google.com/d/topic/comp.lang.c/7NeTMCbl9zM/discussion int-code module
https://groups.google.com/d/topic/comp.lang.c/X7BzyIKpogg/discussion scanner and parser
http://codereview.stackexchange.com/questions/122038/ndarrays-in-c-slicing-transposing-polynomials

--

With the addition of the array and encoding modules, the design has 
deviated somewhat from the goal stated above. There are multiple types.
Also, all array elements have a homogeneous fixed-size representation.
This is done by treating a 32bit integer "token" as an 8bit tag
followed by a 24bit payload. An all-bits-zero or all-bits-one tag 
indicates an immediate 24bit integer so common integer values simply
encode to themselves.

So all arrays can be "mixed" arrays and we will be targeting 
"Generalized APL".
http://www.softwarepreservation.org/projects/apl/Books/AGENERALIZATIONOFAPL/view


--

Scanner and Parser both written and working. REPL is able to assign variables,
lookup variables, and resolve parentheses. Next up: verbs!


--

Added the `plus` verb (for immediate literals only: no overflow checking) 
and enough support to execute it. Current debugging output from the REPL 
(with `<- annotations`):

    $ ./olmec
            2+2                      <- user input
    0032 002b 0032 000d 0000         <- input bytes
    number                           <- scanner analysis
    other
    number

    1                                <- rank of resulting array
    3                                <- length of dim[0]
    2(0,2) 16777259(1,2b) 2(0,2)     <- encoded values
    0x0                              <- a hex zero for some reason
    ->2(0,2)                         <- push 2 to right stack
    ->16777259(1,2b)                 <- push + to right stack
    lookup
    ==117440512(7,0)                 <- lookup + yielding verb plus
    ->2(0,2)                         <- push 2 to right stack
    ->150994944(9,0)                 <- push start marker
    match 3                          <- grammar production matched
    dyad                             <- handler function
    4(0,4)                           <- result


-- 

Adverb and conjunction support is implemented. So now I'm exploring the
implementation of verb rank for all verbs.
[An Implementation of J](http://sblom.github.io/openj-core/iojVerb.htm)
describes the behavior in terms of a "model" in J code. So, I merely
have to implement the functions and operators (verbs and adverbs) described
there and detailed
[in the dictionary](http://www.jsoftware.com/help/dictionary/vocabul.htm),
and rank can be implemented meta-circularly, by calling other 
implemented verbs. Of course these supplementary uses of verbs must
be for simpler cases than *normal* usage of verbs, to avoid an infinite
recursion. 

--

Added 'quad-minus' variable which flip-flops the semantics of high-minus
and regular minus.

    ./olmec
             ⎕-←1
             ¯-2-3-4
    2 3 4
             ⎕-←0
             -¯2¯3¯4
    2 3 4

For some reason hi-minus is displaying here as slash. Looks fine from
github.com.


-- 

The next addition, I think, needs to be chaining or stacking of environments
for variable scoping. This will mean that the symbol lookup might get a little
more complicated. 

-- 

Belated Updates: bracket-indexing (including axis for verbs and adverbs, and
even applying bracket-axis indexing to the left-bracket itself in a bracket-indexing
operation. For each semicolon-separated dimension, there may be 0, 1, or more 
indices listed in arbitrary order and even repeated. Assignment to such a
complex-indexed array may write cells more than once. The copy proceeds along the
row-major ravelled representation of the re-shape of source array (re-shaped to 
match the target shape).
https://groups.google.com/d/msg/comp.lang.apl/dZZmWVGH_VY/L9OFVK6JBAAJ 

dfns! eg.

    josh@LAPTOP-ILO10OOF ~/inca/olmec 
    $ ./olmec 
            f:2+3 
            f 
    5 
            g:2+⍵ 
            g3 
    5 
            h:⍺+⍵ 
            2h3 
    5 

Thread: https://groups.google.com/d/topic/comp.lang.apl/Q329hQDXQZA/discussion

-- 

Added del definitions for multi-line functions, and the long-awaited number type.
It uses gmp and mpfr for multiprecision ints and floats.

https://groups.google.com/d/topic/comp.lang.apl/SiiSUxkrKFk/discussion


It's a struggle to keep the number handling from getting too complicated.

