Beginning thoughts on a third rewrite.
Oettinger's paper (http://www.mt-archive.info/Oettinger-1961.pdf)
suggests unifying the representations of variables and
functions by specifying their degree. Simple variables are degree 0.
Degree >= 1 are functions accepting so many parameters.

Iverson also describes unifying the behavior of functions with
regard to the ranks of their actual parameters.
(http://www.softwarepreservation.org/projects/apl/Books/ADICTIONARYOFAPL)

I also want to better coordinate the behavior of the data types.
So I think I need a {quad} symbol for system variables. 
Perhaps it can even handle variable index-origin sensibly.
But for datatypes, I'm imagining a selectable scheme for overflow promotions.
The behavior of inca "1" is:

    integer overflow -> OVERFLOW (invokes "undefined behavior" in the C implementation)

The behavior of inca2 is:

    integer overflow -> promot to double

For the next one, I want to offer several options and several new floating-point types.

    double
    quad-double
    (complex) <-- maybe not right away, but I want an extensible framework that can
                  accomodate this later.
    rational 
    multiprecision integers
    rational/multiprecision

And the system variable will select which type integer overflow will promote to.

As for variables, I'm trying to imagine a way to allow for multicharacter variable
names while still allowing unbroken expressions involving single-letter variables.
I want `a<1` `b<+` `c<2` `abc` to still yield 3, unless there's an "abc" variable defined.
Currently, the symbol table is dead simple. An array of 52 pointers (2 * 26 letters
in the alphabet). But what if it were overlayed with another data structure, like
a trie or search-tree? In inca 1 and 2, all objects are simultaneously considered part
of the symbol table (or not, as the case may be; if it's a temporary or something)
and part of the allocation stack (for garbage-collection). Adding another member to
the archetype struct will allow objects to be organized along yet another parallel
structuring principle.

From the "abc" example, with N-way splits at each level.

    [a] = { 1 }
           [a]
           [b] = {  }
                  [a]
                  [b]
                  [c] = { 5 }
           [c]
           [d]
    [b] = { + }
    [c] = { 2 }
    [d] = {  }
    [e] = {  }

So, descending the tree has precedence over broad pattern matching. Having parsed
a as the node

          { 1 }
           [a]
           [b] = {  }
                  [a]
                  [b]
                  [c] = { 5 }
           [c]
         
if the next char is 'b', it descends the tree to

           [b] = {  }
                  [a]
                  [b]
                  [c] = { 5 }

even though 'ab' itself is not defined.

This may be a twisty and confusing set of rules, but I think it will afford the
maximum flexibility of usage. User functions can still be single characters which
behave more-or-less syntactically like basic functions. But idioms of these functions
can also be overrided.



As a new side-angle/distraction, I've discovered how to access the graphics characters in xterm.
So the new version will also include an extended alphabet and a more powerful line-editor.

