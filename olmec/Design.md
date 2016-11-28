
    int(tag) int(val)  <-Encoding->  int(object)

    object ... object   <-Array->  object

    object(key) *  Symtab->  object(val)

    object(a) object(w)  Verb->  object(z)

    object(a) object(w)  Adverb->  verb(z)


Numeric types
--

    integer
    GMP multiprecision integer
    MPFR multiprecision floating point

The math functions should seamlessly promote results into the larger
types as necessary. There are no syntactic decorations for numbers
to indicate or request the multiprecision handling. It should be
automatic.


Object types
--

The data structures of the interpreter are built out of a fairly
low-level view of data. The `object` type is defined as a 32bit
integer. We select a few bits from the top to treat as a `tag` and
restrict our language's integer type to the remaining value bits,
excluding the tag.

The tag indicates the meaning of the value bits. Tags such as 
LITERAL, CHAR, PCHAR (executable char), LABEL, use the integer
value in the obvious way.

Other tags are "indexed" and map to a pointer through an array
of expanding arrays of `void*`. This is wrapped by a function, 
`void *getptr(object d)` but proceeds very simply:

    memory_bank[gettag(d) - FIRST_INDEXED_TYPE].tab[getval(d)];

For interoperability, all array extents should be limited to 
the range of an integer literal, or 2^24.
