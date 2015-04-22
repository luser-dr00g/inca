Inca3
-- 

**Inca3** is the third re-write of the inca language interpreter (which is more or less
defined by its implementation if/where the behavior disagrees with the documentation).

Each rewrite has begun with the J incunabulum to which I then apply edits.
Not only that, but I retype I from my handwritten notebook copy. This passes the
code through my brain and fingers even though I'm merely "copying".

Each additional feature or change is made compilable (and usually tested and apparently
correct) before committing. Thus the commit log provides a time-wise view of the entire
source. I also employ a select few "methodologies" which I try to apply consistently.
These include DRY or Don't Repeat Yourself, which means that any two places in the source
which "do the same thing" ought to be factored to maintain that condition. The practice 
of starting from a working, simple interpreter and extending is an application of the
Tracer-Bullet strategy, where the first order of business is build a column connecting
the top-down and bottom-up designs so the basic functionality can be directly tested and
debugged, and each new feature can be directly tested and debugged.

The features:
--- 

The symbol table interacts nicely with the parsing to analyze variable names at execution
time. Many of the features of inca 1's *interpolation* abilities are now made available with
a more natural behavior. A symbol is interpreted when passing from the left-stack to the
right-stack by splitting off the longest defined prefix (which is pushed back onto the
left-stack) and then repeating until the symbol is exhausted. Thus symbols can be directly
adjacent to one-another with or without intervening whitespace.

True APL characters. (Some are fake.)

True APL shift-reduce parsing.

Modern APL/J handling of verb rank as it applies to the frame/cell handling of its arguments.



Above this line is topical.
<hr>
Below this line is chronological.


Beginning thoughts on a third rewrite.
Oettinger's paper (http://www.mt-archive.info/Oettinger-1961.pdf)
suggests unifying the representations of variables and
functions by specifying their degree. Simple variables are degree 0.
Degree >= 1 are functions accepting so many parameters.

Iverson also describes unifying the behavior of functions with
regard to the ranks of their actual parameters.
(http://www.softwarepreservation.org/projects/apl/Books/ADICTIONARYOFAPL)
Other ridiculously useful books and papers: 
[An Implementation of J](http://sblom.github.io/openj-core/ioj.htm), 
[IOJ presentation notes](http://archive.vector.org.uk/trad/v094/hui094_85.pdf)
[Rationalized APL](http://www.jsoftware.com/papers/RationalizedAPL.htm), 
[A Dictionary of APL](http://www.softwarepreservation.org/projects/apl/Books/ADICTIONARYOFAPL) 
[Practical Uses of a Model of APL](http://www.jsoftware.com/papers/APLModel.htm) 
[Kona wiki](https://github.com/kevinlawler/kona/wiki)

I also want to better coordinate the behavior of the data types.
So I think I need a {quad} symbol for system variables. 
Perhaps it can even handle variable index-origin sensibly.
But for datatypes, I'm imagining a selectable scheme for overflow promotions.
The behavior of inca "1" is:

    integer overflow -> OVERFLOW (invokes "undefined behavior" in the C implementation)

The behavior of inca2 is:

    integer overflow -> promote to double

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

The basics of the extended character set is implemented. Switch to the alternate characters with ctrl-N `^N` and back to normal with ctrl-O `^O`. There is one function available fromthe extended set: plusminus. Monadic plusminus performs a negation of the argument. Dyadic plus minus creates an array and returns both the sum and difference of the left and right arguments.

There is a third set of characters available that I may incorporate into inca's alternate set. But I intend inca to maintain only 2 input modes. So the third set will only be a bank to draw from, not the basis of its own defined set. This process has begun and the alternate keyboard no longer has the silly `LF` `CR` `VT` etc chars, but these may be useful in displaying strings.

The extensible symbol table is implemented. Very much as described above, it has 52-way branching at each node. The searching function has a defining mode where it allocates new nodes for each char in the symbol, and a separate prefix search mode where it returns the longest match.

I'm currently studying the J documents concerning the parsing. Hopefully I can replace the procedural parsing with a table-driven setup.

-- 

3/11/2015

To sum up so far, beginning (yet again) with the incunabulum
I've added a custom
line-editor using vt220 commands compatible with xterm, setting the
mode using the terminal-indepedent (and POSIX-portable) termios library.
Since it works on with Cygwin's xorg server on Windows 8, I feel safe
in assuming that this will be fairly-easily portable among modern
unix/linux systems.

The editor itself is guided by [The Craft of Text Editing](http://www.finseth.com/craft/craft.pdf) (4.98MB pdf).

Again with vt220 codes, I've added an *alternate keyboard* full of
crazy shapes and doodles collected from the vt220 line-drawing set
and "uk" set. These have yet to be sensibly organized, but I have
added one non-ascii doodle command from the alternate keyboard as
a *tracer-bullet* implementation strategy. This is the plus/minus
function which performs negation monadically, and dyadically it
returns an array of the sum and difference of the left and right
arguments (a sensible interpretation of plus/minus, I think).

Next, I've added a trie structure for an extensible symbol table.
For lookups, it uses a prefix search: returning the longest defined
prefix of the requested key. This should permit the simultaneous
use of of long names and space-free short names in a tight expression.
Thus, this is no longer just a golfing language.

Next, I've reimplemented the `wd()` function which scans the 
expression to form words. It now uses a table-driven approach
similar to the description in *An Implementation of J* (I do not
understand the diagram of the table there). To cooperate with 
the symbol-lookup mechanism, it consumes any run of alphabetic
characters and collects it as a single symbol-typed object
which contains a zero-terminated "C" string. This is distinct
from the char-typed objects which are "pascal-style" with a
separate count and no terminator. Or rather "will be", as the 
char-typed objects aren't really implemented yet.

Next up, I'm going to take a hard look at the `ex()` function
and the stack-based algorithms described in *An Implementation of J* 
and *A Dictionary of APL*. So that one will be reimplemented to be
table-based as well and probably to use true right-to-left semantics now.

-- 

Table-driven parser is written and working, including an extra
*implicit-parens* feature. The assignment character has been changed to
left-guillemot, accessed from the alternate keyboard `^n` `<` `^o`.

-- 

The alt keyboard is now accessible with the ALT key. Ctrl-N is now an
"ALT-lock" toggle key.

The code has been reworked to be very table-driven using X-Macros to 
construct symbolically-indexed tables.

The alt keyboard is being reworked to produce true Unicode APL characters
(which xterm handles just fine).

-- 

Implemented scalar agreement (scalar extension) for the plus() function.
Minus, times, and divide do not yet do this.

Shifting thoughts toward the implementation of multiple numeric types.

-- 

I had a great idea for numeric types. The thing I most want to avoid is
different sizes of atoms in the array data. Because then all access to
the data has to be cast to the appropriate pointer type, and we need to
switch on the type even just to iterate through it.

So everything needs to be the same size. Drawing from the Lisp book I just
finished reading, this is the exact same problem that early Lisp
implementations faced when dealing with numeric types. So we can borrow 
some of the same solutions. I don't want to add a type word to every atom.
I like having the basis as `int`, passing the issue off to the C implementation
to determine what `int` is most appropriate for a given machine. 

So the idea is to steal bits off the top of the `int` to use as a type 
indicator. So we'll lose some precision for atomic integers. But full-width
integers will also be accessible in a separate table. Floating-point numbers
will also be stored in a separate table.

I haven't decided exactly how many bits to use, so I hope to make the
code adaptable to different choices of partition. But for the first draft,
the `int` will divide right down the middle. So atomic integers will
be in the range -32768..32767 .

    0x00112233 
      0000nnnn  small atomic integer nnnn
      bbbbiiii  indirect number at table[bbbb][iiii]
                stored as (bank+1,index)

There will be a master table of pointers to tables for each type. These
should be `struct` so each level can maintain its own bookkeeping fields.

-- 

Instead of structs, the master table is an abstract BOX array. For 
bookkeeping, element[0] is treated as a top-of-list counter/cursor, 
and the remaining elements treated as a 1-index-origin array.

The fixnum and flonum tables also use element[0] as a counter 
and the remaining elements are the data.

Presumably, to extend this to rationals, element[0] of ratnum will
be count/1. :)


-- 

Some big updates. The Rank conjunction and generalized rank:frame/cell behavior
for verbs, all wrapped in macros for easy inclusion in verb functions.

Overflow detection for integer arithmetic. Currently this triggers a promotion
to floating-point (double) which is the only larger type. But I'm carefully 
considering how to add an arbitrary precision integer. And then it will need
a configuration option, perhaps accessed by a conjunction like rank.

Only a very few functions are implemented so far. And only the math functions
use the rank behavior. So plus, minus, times, divide. Iota and rho are partially
implemented. And the reduce adverb. And the conjunctions rank and curry/compose.

This iteration of the code is more "depth"-first than earlier gos.

Assignment is now the left-arrow (ALT-[).

