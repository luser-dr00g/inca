#include<limits.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<termios.h>
#include<unistd.h>


typedef unsigned char C;
typedef intptr_t I;
typedef uintptr_t U;
typedef double D;
typedef struct a{I t, r, n, k, d[1];}*A; /* The abstract array header */
#define AT(a) ((a)->t)                   /* Type */
#define AR(a) ((a)->r)                   /* Rank (size of Dims) */
#define AN(a) ((a)->n)                   /* Number of values in ravel */
#define AK(a) ((a)->k)                   /* Offset of ravel */
#define AD(a) ((a)->d)                   /* Dims */
#define AV(a) ((I*)(((C*)a)+AK(a)))      /* Values in ravelled order */
enum type { INT, BOX, SYMB, CHAR, NUM, DBL, MRK, NLL, VRB, NTYPES };
struct a nullob = { NLL };
A null = &nullob;           //two "singular" objects
struct a markob = { MRK };
A mark = &markob;

A newsymb(C *s,I n,I state);
struct st *findsymb(struct st *st, char **s, int mode);

#define P printf
#define R return
#define V1(f) A f(A w,      A self)
#define V2(f) A f(A a, A w, A self)
#define DO(n,x)  {I i=0,_n=(n);for(;i<_n;++i){x;}}
#define DO2(n,x) {I j=0,_o=(n);for(;j<_o;++j){x;}}
#define DO3(n,x) {I k=0,_p=(n);for(;k<_p;++k){x;}}



#define ESC(x) "\x1b" #x
#define ESCCHR '\x1b'
#define CTL(x) (x-64)
#define EOT 004
#define DEL 127
#define MODE1(x) (x|1<<7)
#define MODE2(x) (x-32)

/* ALPHA_##NAME  base  ext input  output   (ext corresponds to 'mode' in getln)*/
#define ALPHATAB(_) \
    _( SPACE, ' ', 0, " ", " " ) \
    _( a, 'a', 0, "a", "a" ) /* basic latin alphabet */ \
    _( b, 'b', 0, "b", "b" ) \
    _( c, 'c', 0, "c", "c" ) \
    _( d, 'd', 0, "d", "d" ) \
    _( e, 'e', 0, "e", "e" ) \
    _( f, 'f', 0, "f", "f" ) \
    _( g, 'g', 0, "g", "g" ) \
    _( h, 'h', 0, "h", "h" ) \
    _( i, 'i', 0, "i", "i" ) \
    _( j, 'j', 0, "j", "j" ) \
    _( k, 'k', 0, "k", "k" ) \
    _( l, 'l', 0, "l", "l" ) \
    _( m, 'm', 0, "m", "m" ) \
    _( n, 'n', 0, "n", "n" ) \
    _( o, 'o', 0, "o", "o" ) \
    _( p, 'p', 0, "p", "p" ) \
    _( q, 'q', 0, "q", "q" ) \
    _( r, 'r', 0, "r", "r" ) \
    _( s, 's', 0, "s", "s" ) \
    _( t, 't', 0, "t", "t" ) \
    _( u, 'u', 0, "u", "u" ) \
    _( v, 'v', 0, "v", "v" ) \
    _( w, 'w', 0, "w", "w" ) \
    _( x, 'x', 0, "x", "x" ) \
    _( y, 'y', 0, "y", "y" ) \
    _( z, 'z', 0, "z", "z" ) \
    _( A, 'A', 0, "A", "A" ) \
    _( B, 'B', 0, "B", "B" ) \
    _( C, 'C', 0, "C", "C" ) \
    _( D, 'D', 0, "D", "D" ) \
    _( E, 'E', 0, "E", "E" ) \
    _( F, 'F', 0, "F", "F" ) \
    _( G, 'G', 0, "G", "G" ) \
    _( H, 'H', 0, "H", "H" ) \
    _( I, 'I', 0, "I", "I" ) \
    _( J, 'J', 0, "J", "J" ) \
    _( K, 'K', 0, "K", "K" ) \
    _( L, 'L', 0, "L", "L" ) \
    _( M, 'M', 0, "M", "M" ) \
    _( N, 'N', 0, "N", "N" ) \
    _( O, 'O', 0, "O", "O" ) \
    _( P, 'P', 0, "P", "P" ) \
    _( Q, 'Q', 0, "Q", "Q" ) \
    _( R, 'R', 0, "R", "R" ) \
    _( S, 'S', 0, "S", "S" ) \
    _( T, 'T', 0, "T", "T" ) \
    _( U, 'U', 0, "U", "U" ) \
    _( V, 'V', 0, "V", "V" ) \
    _( W, 'W', 0, "W", "W" ) \
    _( X, 'X', 0, "X", "X" ) \
    _( Y, 'Y', 0, "Y", "Y" ) \
    _( Z, 'Z', 0, "Z", "Z" ) \
    /* ALPHA_NAME      base ext input output */ \
    _( ONE,             '1',  0, "1", "1" ) /* ascii digits */ \
    _( TWO,             '2',  0, "2", "2" ) \
    _( THREE,           '3',  0, "3", "3" ) \
    _( FOUR,            '4',  0, "4", "4" ) \
    _( FIVE,            '5',  0, "5", "5" ) \
    _( SIX,             '6',  0, "6", "6" ) \
    _( SEVEN,           '7',  0, "7", "7" ) \
    _( EIGHT,           '8',  0, "8", "8" ) \
    _( NINE,            '9',  0, "9", "9" ) \
    _( ZERO,            '0',  0, "0", "0" ) \
    _( ONE1,            '1',  1, "1", "1" ) /* accept same digits in alt mode */ \
    _( TWO1,            '2',  1, "2", "2" ) \
    _( THREE1,          '3',  1, "3", "3" ) \
    _( FOUR1,           '4',  1, "4", "4" ) \
    _( FIVE1,           '5',  1, "5", "5" ) \
    _( SIX1,            '6',  1, "6", "6" ) \
    _( SEVEN1,          '7',  1, "7", "7" ) \
    _( EIGHT1,          '8',  1, "8", "8" ) \
    _( NINE1,           '9',  1, "9", "9" ) \
    _( ZERO1,           '0',  1, "0", "0" ) \
    _( PLUS,            '+',  0, "+", "+" ) /* ascii punctuation */ \
    _( MINUS,           '-',  0, "-", "-" ) \
    _( EQUAL,           '=',  0, "=", "=" ) \
    _( UNDERSCORE,      '_',  0, "_", "_" ) \
    _( LBRACE,          '{',  0, "{", "{" ) \
    _( RBRACE,          '}',  0, "}", "}" ) \
    _( PIPE,            '|',  0, "|", "|" ) \
    _( LBRACKET,        '[',  0, "[", "[" ) \
    _( RBRACKET,        ']',  0, "]", "]" ) \
    _( BACKSLASH,       '\\', 0, "\\", "\\" ) \
    _( COLON,           ':',  0, ":", ":" ) \
    _( SEMICOLON,       ';',  0, ";", ";" ) \
    _( QUOTE,           '\'', 0, "'", "'" ) \
    _( DBLQUOTE,        '"',  0, "\"", "\"" ) \
    _( COMMA,           ',',  0, ",", "," ) \
    _( PERIOD,          '.',  0, ".", "." ) \
    _( SLASH,           '/',  0, "/", "/" ) \
    _( LANG,            '<',  0, "<", "<" ) \
    _( RANG,            '>',  0, ">", ">" ) \
    _( QUESTION,        '?',  0, "?", "?" ) \
    _( TILDE,           '~',  0, "~", "~" ) \
    _( BACKQUOTE,       '`',  0, "`", "`" ) \
    _( EXCL,            '!',  0, "!", "!" ) \
    _( AT,              '@',  0, "@", "@" ) \
    _( HASH,            '#',  0, "#", "#" ) \
    _( DOLLAR,          '$',  0, "$", "$" ) \
    _( PERCENT,         '%',  0, "%", "%" ) \
    _( CARET,           '^',  0, "^", "^" ) \
    _( AMPERSAND,       '&',  0, "&", "&" ) \
    _( STAR,            '*',  0, "*", "*" ) \
    _( LPAREN,          '(',  0, "(", "(" ) \
    _( RPAREN,          ')',  0, ")", ")" ) \
    /* ALPHA_NAME base      ext input output */ \
    _( PLUSMINUS, MODE1('g'), 1, "g", ESC(n)"g""\xE" ) /* xterm alt graphics chars */ \
    _( TWODOTS,   MODE1('!'), 1, "!", ESC(o)"(""\xE" ) \
    _( DIAERESIS, MODE1('!'), 1, "!", /*U+00a8*/ /*"\xc2"*/"\xa8" ) \
    _( HIMINUS,   MODE1('@'), 1, "@", ESC(o)"/""\xE" ) \
    _( MACRON,    MODE1('@'), 1, "@", /*U+00af*/ /*"\xc2"*/"\xaf" ) \
    _( NOTEQUAL,  MODE1('|'), 1, "|", ESC(n)"|""\xE" ) \
    _( LESS,            '<',  1, "#", "<" ) \
    _( LESSEQUAL, MODE1('$'), 1, "$", ESC(n)"y""\xE" ) \
    _( EQALT,           '=',  1, "%", "=" ) \
    _( MOREEQUAL, MODE1('^'), 1, "^", ESC(n)"z""\xE" ) \
    _( MORE,            '>',  1, "&", ">" ) \
    _( EQSLASH,   MODE1('*'), 1, "*", ESC(n)"|""\xE" ) \
    _( QUEST,           '?',  1, "q", "?" ) \
    _( OMEGA,     MODE1('w'), 1, "w", /*U+2375*/ "\xe2\x8d\xb5" ) \
    _( EPSILON,   MODE1('e'), 1, "e", /*U+2208*/ "\xe2\x88\x88" ) \
    _( RHO,       MODE1('r'), 1, "r", /*U+2374*/ "\xe2\x8d\xb4" ) \
    _( TILDEOP,   MODE1('t'), 1, "t", /*U+223c*/ "\xe2\x88\xbe" ) \
    _( UPARROW,   MODE1('y'), 1, "y", /*U+2191*/ "\xe2\x86\x91" ) \
    _( DNARROW,   MODE1('u'), 1, "u", /*U+2193*/ "\xe2\x86\x93" ) \
    _( IOTA,      MODE1('i'), 1, "i", /*U+2373*/ "\xe2\x8d\xb3" ) \
    _( CIRCLE,    MODE1('o'), 1, "o", /*U+25cb*/ "\xe2\x97\x8b" ) \
    _( STAROP,    MODE1('p'), 1, "p", /*U+22c6*/ "\xe2\x8b\x86" ) \
    _( LTARROW,   MODE1('['), 1, "[", /*U+2190*/ "\xe2\x86\x90" ) \
    _( RTARROW,   MODE1('{'), 1, "{", /*U+2192*/ "\xe2\x86\x92" ) \
    _( ALPHA,     MODE1('a'), 1, "a", /*U+237a*/ "\xe2\x8d\xba" ) \
    _( LEFTCEIL,  MODE1('s'), 1, "s", /*U+2308*/ "\xe2\x8c\x88" ) \
    _( LEFTFLOOR, MODE1('d'), 1, "d", /*U+230a*/ "\xe2\x8c\x8a" ) \
    _( UNDBAR2,         '_',  1, "f", "_" ) \
    _( NABLA,     MODE1('g'), 1, "g", /*U+2207*/ "\xe2\x88\x87" ) \
    _( INCREMENT, MODE1('h'), 1, "h", /*U+2206*/ "\xe2\x88\x86" ) \
    _( RING,      MODE1('j'), 1, "j", /*U+2218*/ "\xe2\x88\x98" ) \
    _( KWOTE,          '\'',  1, "k", "'" ) \
    _( QUAD,      MODE1('l'), 1, "l", /*U+2395*/ "\xe2\x8e\x95" ) \
    _( SUBSET,    MODE1('z'), 1, "z", /*U+2282*/ "\xe2\x8a\x82" ) \
    _( SUPERSET,  MODE1('x'), 1, "x", /*U+2283*/ "\xe2\x8a\x83" ) \
    _( CAP,       MODE1('c'), 1, "c", /*U+2229*/ "\xe2\x88\xa9" ) \
    _( CUP,       MODE1('v'), 1, "v", /*U+222a*/ "\xe2\x88\xaa" ) \
    _( UPTACK,    MODE1('b'), 1, "b", /*U+22a5*/ "\xe2\x8a\xa5" ) \
    _( DNTACK,    MODE1('n'), 1, "n", /*U+22a4*/ "\xe2\x8a\xa4" ) \
    _( DIVIDES,   MODE1('m'), 1, "m", /*U+2223*/ "\xe2\x88\xa3" ) \
    _( DOT,       MODE1('~'), 1, "~", ESC(n)"~""\xE" ) \
    _( DIAMOND,   MODE1('`'), 1, "`", ESC(n)"`""\xE" ) \
    _( PI,        MODE1('{'), 1, "{", ESC(n)"{""\xE" ) \
    _( POUND,     MODE1('}'), 1, "}", ESC(n)"}""\xE" ) \
    _( EURO,      MODE1('e'), 1, "e", "\xe2\x82\xac" ) \
    _( CENT,      MODE1('e'), 1, "e", ESC(o)"\"""\xE" ) \
    _( YEN,       MODE1('d'), 1, "d", ESC(o)"%""\xE" ) \
    _( HBAR0,     MODE1('o'), 1, "o", ESC(n)"o""\xE" ) \
    _( HBAR1,     MODE1('p'), 1, "p", ESC(n)"p""\xE" ) \
    _( HBAR3,     MODE1('q'), 1, "q", ESC(n)"q""\xE" ) \
    _( HBAR4,     MODE1('r'), 1, "r", ESC(n)"r""\xE" ) \
    _( HBAR5,     MODE1('s'), 1, "s", ESC(n)"s""\xE" ) \
    _( GRAYBOX,   MODE1('a'), 1, "a", ESC(n)"a""\xE" ) \
    _( DEGREE,    MODE1('f'), 1, "f", ESC(n)"f""\xE" ) \
    _( HT,        '\x9', 0, "\t", ESC(n)"b""\xE" ) \
    _( NL,        '\xa', 0, "\n", ESC(n)"h""\xE" ) \
    _( LF,        '\xa', 0, "\n", ESC(n)"e""\xE" ) \
    _( VT,        '\xb', 0, "\v", ESC(n)"i""\xE" ) \
    _( FF,        '\xc', 0, "\f", ESC(n)"c""\xE" ) \
    _( CR,        '\xd', 0, "\r", ESC(n)"d""\xE" ) \
    _( JUNCL,     MODE1('m'), 1, "m", ESC(n)"m""\xE" ) \
    _( JUNCJ,     MODE1('j'), 1, "j", ESC(n)"j""\xE" ) \
    _( JUNCK,     MODE1('k'), 1, "k", ESC(n)"k""\xE" ) \
    _( JUNCR,     MODE1('l'), 1, "l", ESC(n)"l""\xE" ) \
    _( VBAR,      MODE1('x'), 1, "x", ESC(n)"x""\xE" ) \
    _( JUNCF,     MODE1('t'), 1, "t", ESC(n)"t""\xE" ) \
    _( JUNC3,     MODE1('u'), 1, "u", ESC(n)"u""\xE" ) \
    _( JUNCT,     MODE1('w'), 1, "w", ESC(n)"n""\xE" ) \
    _( JUNCM,     MODE1('n'), 1, "n", ESC(n)"w""\xE" ) \
    _( JUNCW,     MODE1('b'), 1, "b", ESC(n)"v""\xE" ) \
    /* ALPHA_NAME base       ext input output */ \
    _( INVEXCL,   MODE1('!'), 1, "!", ESC(o)"!""\xE" ) /* "uk" chars patch */ \
    _( INVQUEST,  MODE1('?'), 1, "?", ESC(o)"?""\xE" ) \
    _( GUILLEFT,  MODE1('<'), 1, "<", ESC(o)"+""\xE" ) \
    _( GUILRIGHT, MODE1('>'), 1, ">", ESC(o)";""\xE" ) \
    _( COMPL,     MODE1('^'), 1, "^", ESC(o)",""\xE" ) \
    _( TIMES,     MODE1('='), 1, "=", ESC(o)"W""\xE" ) \
    _( DIVIDE,    MODE1('/'), 1, "/", ESC(o)"w""\xE" ) \
    _( CDOT,      MODE1('.'), 1, ".", ESC(o)"7""\xE" ) \
    _( HYPHEN,    MODE1('-'), 1, "-", ESC(o)"-""\xE" ) \
    _( BUTTON,    MODE1('i'), 1, "i", ESC(o)"$""\xE" ) \
    _( SECTION,   MODE1('h'), 1, "h", ESC(o)"'""\xE" ) \
    _( PRIME,     MODE1('\''), 1, "'", ESC(o)"4""\xE" ) \
    _( CIRCC,     MODE1('c'), 1, "c", ESC(o)")""\xE" ) \
    _( ZEROSLASH, MODE1('v'), 1, "v", ESC(o)"X""\xE" ) \
    _( OBAR,      MODE1(';'), 1, ";", ESC(o)":""\xE" ) \
    /* ALPHA_NAME base       ext input output */ \
    _( PARAGRAPH, MODE1(','), 1, ",", ESC(o)"6""\xE" ) \
    _( BARA,      MODE1('@'), 1, "@", ESC(o)"*""\xE" ) \
    _( CIRCR,     MODE1('#'), 1, "#", ESC(o)".""\xE" ) \
    _( MU,        MODE1('$'), 1, "$", ESC(o)"5""\xE" ) \
    _( COLONBAR,  MODE1('+'), 1, "+", ESC(o)"w""\xE" ) \
    _( DEL,       MODE1('&'), 1, "&", ESC(o)"P""\xE" ) \
    _( SUPONE,    MODE1('('), 1, "(", ESC(o)"9""\xE" ) \
    _( SUPTWO,    MODE1(')'), 1, ")", ESC(o)"2""\xE" ) \
    /*_( SUPTHREE,  MODE1('+'), 1, "+", ESC(o)"1""\xE" )*/ \
    _( a1, 'a', 1, "a", "a" ) /* fallback: basic latin alphabet */ \
    _( b1, 'b', 1, "b", "b" ) \
    _( c1, 'c', 1, "c", "c" ) \
    _( d1, 'd', 1, "d", "d" ) \
    _( e1, 'e', 1, "e", "e" ) \
    _( f1, 'f', 1, "f", "f" ) \
    _( g1, 'g', 1, "g", "g" ) \
    _( h1, 'h', 1, "h", "h" ) \
    _( i1, 'i', 1, "i", "i" ) \
    _( j1, 'j', 1, "j", "j" ) \
    _( k1, 'k', 1, "k", "k" ) \
    _( l1, 'l', 1, "l", "l" ) \
    _( m1, 'm', 1, "m", "m" ) \
    _( n1, 'n', 1, "n", "n" ) \
    _( o1, 'o', 1, "o", "o" ) \
    _( p1, 'p', 1, "p", "p" ) \
    _( q1, 'q', 1, "q", "q" ) \
    _( r1, 'r', 1, "r", "r" ) \
    _( s1, 's', 1, "s", "s" ) \
    _( t1, 't', 1, "t", "t" ) \
    _( u1, 'u', 1, "u", "u" ) \
    _( v1, 'v', 1, "v", "v" ) \
    _( w1, 'w', 1, "w", "w" ) \
    _( x1, 'x', 1, "x", "x" ) \
    _( y1, 'y', 1, "y", "y" ) \
    _( z1, 'z', 1, "z", "z" ) \
    _( A1, 'A', 1, "A", "A" ) \
    _( B1, 'B', 1, "B", "B" ) \
    _( C1, 'C', 1, "C", "C" ) \
    _( D1, 'D', 1, "D", "D" ) \
    _( E1, 'E', 1, "E", "E" ) \
    _( F1, 'F', 1, "F", "F" ) \
    _( G1, 'G', 1, "G", "G" ) \
    _( H1, 'H', 1, "H", "H" ) \
    _( I1, 'I', 1, "I", "I" ) \
    _( J1, 'J', 1, "J", "J" ) \
    _( K1, 'K', 1, "K", "K" ) \
    _( L1, 'L', 1, "L", "L" ) \
    _( M1, 'M', 1, "M", "M" ) \
    _( N1, 'N', 1, "N", "N" ) \
    _( O1, 'O', 1, "O", "O" ) \
    _( P1, 'P', 1, "P", "P" ) \
    _( Q1, 'Q', 1, "Q", "Q" ) \
    _( R1, 'R', 1, "R", "R" ) \
    _( S1, 'S', 1, "S", "S" ) \
    _( T1, 'T', 1, "T", "T" ) \
    _( U1, 'U', 1, "U", "U" ) \
    _( V1, 'V', 1, "V", "V" ) \
    _( W1, 'W', 1, "W", "W" ) \
    _( X1, 'X', 1, "X", "X" ) \
    _( Y1, 'Y', 1, "Y", "Y" ) \
    _( Z1, 'Z', 1, "Z", "Z" ) \
    _( PLUS1,            '+',  1, "+", "+" ) /* fallback: ascii punctuation */ \
    _( MINUS1,           '-',  1, "-", "-" ) \
    _( EQUAL1,           '=',  1, "=", "=" ) \
    _( UNDERSCORE1,      '_',  1, "_", "_" ) \
    _( LBRACE1,          '{',  1, "{", "{" ) \
    _( RBRACE1,          '}',  1, "}", "}" ) \
    _( PIPE1,            '|',  1, "|", "|" ) \
    _( LBRACKET1,        '[',  1, "[", "[" ) \
    _( RBRACKET1,        ']',  1, "]", "]" ) \
    _( BACKSLASH1,       '\\', 1, "\\", "\\" ) \
    _( COLON1,           ':',  1, ":", ":" ) \
    _( SEMICOLON1,       ';',  1, ";", ";" ) \
    _( QUOTE1,           '\'', 1, "'", "'" ) \
    _( DBLQUOTE1,        '"',  1, "\"", "\"" ) \
    _( COMMA1,           ',',  1, ",", "," ) \
    _( PERIOD1,          '.',  1, ".", "." ) \
    _( SLASH1,           '/',  1, "/", "/" ) \
    _( LANG1,            '<',  1, "<", "<" ) \
    _( RANG1,            '>',  1, ">", ">" ) \
    _( QUESTION1,        '?',  1, "?", "?" ) \
    _( TILDE1,           '~',  1, "~", "~" ) \
    _( BACKQUOTE1,       '`',  1, "`", "`" ) \
    _( EXCL1,            '!',  1, "!", "!" ) \
    _( AT1,              '@',  1, "@", "@" ) \
    _( HASH1,            '#',  1, "#", "#" ) \
    _( DOLLAR1,          '$',  1, "$", "$" ) \
    _( PERCENT1,         '%',  1, "%", "%" ) \
    _( CARET1,           '^',  1, "^", "^" ) \
    _( AMPERSAND1,       '&',  1, "&", "&" ) \
    _( STAR1,            '*',  1, "*", "*" ) \
    _( LPAREN1,          '(',  1, "(", "(" ) \
    _( RPAREN1,          ')',  1, ")", ")" ) \
    /* ALPHA_NAME base      ext input output */ \
    _( NULLCHAR, 0, 0, 0, 0 )
#define ALPHATAB_ENT(a,...) {__VA_ARGS__},
struct alpha{int base;int ext;char*input;char*output;}alphatab[]={ALPHATAB(ALPHATAB_ENT)};
#define ALPHATAB_NAME(a,...) ALPHA_ ## a ,
enum alphaname { ALPHATAB(ALPHATAB_NAME) }; /* NB. ALPHA_NAME!=alphatab[ALPHA_NAME].base */

/* convert input character to internal representation */
int inputtobase (int c, int mode){ int i;
    for (i=0;i<(sizeof alphatab/sizeof*alphatab);i++)
        if (c==*alphatab[i].input && mode==alphatab[i].ext)
            return alphatab[i].base;
    return mode? MODE1(c): c;
    //return c | mode << 7;
}
/* convert internal representation to output representation */
char *basetooutput (int c){ int i;
    for (i=0;i<(sizeof alphatab/sizeof*alphatab);i++)
        if (c==alphatab[i].base)
            return alphatab[i].output;
    return "";
}


struct termios tm; /* terminal settings struct to save default settings */

/* setup special terminal mode for line editor:
   turn off canonical mode for char-at-a-time processing
   select vt220 G2 and G3 character sets for display of extra glyphs
 */
void specialtty(){
    tcgetattr(0,&tm);

//https://web.archive.org/web/20060117034503/http://www.cs.utk.edu/~shuford/terminal/xterm_codes_news.txt
    //fputs("\x1B""*0\n",stdout);
#if 0
    //experiment with line-drawing chars
    fputs(ESC(*0),stdout);
    fputs(ESC(n)
            "lqqqqqk\n"
            "x"
      ESC(o)"a box"ESC(n)
                  "x\n"
            "mqqqqqj\n"
            ESC(o)"\n", stdout);
#endif

#if 0
    //show the various alternate charsets available in xterm vt220 mode
    fputs("\x1B*0\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    //fputs("\x1B*1\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n"); //these 2 are not interesting
    //fputs("\x1B*2\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n"); //
    fputs("\x1B*A\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    fputs("\x1B*B\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
#endif

#if 0
    //show special char and line-drawing set as keyboard layout
    fputs(ESC(*0)ESC(n),stdout);
    fputs( "~!@#$%^&*()_+" "\n" "`1234567890-="  "\n"
           "QWERTYUIOP{}|" "\n" "qwertyuiop[]\\" "\n"
           "ASDFGHJKL:\""  "\n" "asdfghjkl;'"    "\n"
           "ZXCVBNM<>?"    "\n" "zxcvbnm,./"     "\n" , stdout);
        fputs(ESC(o),stdout);
#endif

    fputs(ESC()")B",stdout); //set G1 charset to B : usascii
    fputs(ESC(*0),stdout); //set G2 charset to 0 : special char and line drawing set ESC(n)
    fputs(ESC(+A),stdout); //set G3 charset to A : "uk" accented and special chars ESC(o)
    fputc(CTL('N'),stdout); //select G1 charset  ESC(n):select G2  ESC(o):select G3

#if 1
    { /* print ALT-keyboard layout */
        int i,j;
        char *keys[] = {
           "~!@#$%^&*()_+",
           "`1234567890-=",
           "QWERTYUIOP{}|",
           "qwertyuiop[]\\",
           "ASDFGHJKL:\"",
           "asdfghjkl;'",
           "ZXCVBNM<>?",
           "zxcvbnm,./"
        };
        for (i=0;i<(sizeof keys/sizeof*keys);i++){
            int n = strlen(keys[i]);
            for (j=0;j<n;j++)                          /* V- mode=1 */
                fputs(basetooutput(inputtobase(keys[i][j],1)),stdout);
            fputc('\n',stdout);
        }
    }
#endif

    { struct termios tt=tm; //man termios
        //cfmakeraw(&tt);
        tt.c_iflag &= ~(IGNBRK | /*BRKINT |*/
                PARMRK | ISTRIP | /*INLCR | IGNCR | ICRNL |*/ IXON);
        /*tt.c_oflag &= ~OPOST;*/
        tt.c_lflag &= ~(ECHO | /*ECHONL |*/ ICANON /*| ISIG | IEXTEN*/);
        tt.c_cflag &= ~(CSIZE | PARENB);
        tt.c_cflag |= CS8;
        tcsetattr(0,TCSANOW,&tt); }
}
/* restore saved default terminal settings */
void restoretty(){ tcsetattr(0,TCSANOW,&tm); }



/* read expression from terminal into buffer
   (re)allocate buffer as necessary
   process backspaces
   process mode changes and alt key.
 */
C * getln(C *prompt, C **s, int *len){
    int mode = 0;
    int tmpmode = 0;
    C *p;
    if (prompt) fputs(prompt,stdout);
    if (!*s) *s = malloc(*len=256);
    p = *s;
    while(1){
        int c;
        c = fgetc(stdin);
        switch(c){
        case EOF:
        case EOT: if (p==*s) goto err;
                  break;
        case ESCCHR:
                  c = fgetc(stdin);
                  switch(c){
                  default:
                      tmpmode = 1;
                      goto storechar;
                      break;
#if 0
                  case '[':
                      c = fgetc(stdin);
                      switch(c){
                      default:
                          *p++ = MODE1('[');
                          fputs(basetooutput(MODE1('[')),stdout);
                          ungetc(c,stdin);
                          break;
                      case 'Z':
                          c = '\v';       //convert shift-TAB to vertical tab
                          *p++ = c;                    // save base in string
                          fputs(basetooutput(c),stdout);  // echo output form
                          break;
                      }
                      break;
#endif
                  }
                  break;
        case '\n':
                  fputc('\n',stdout);
                  *p++ = c;
                  goto breakwhile;
        case CTL('N'): mode = !mode; tmpmode = 0; break;
        case CTL('U'): 
                       while(p>*s){
                           fputs("\b \b",stdout);
                           --p;
                       }
                       tmpmode = 0;
                       break;
        case '\b':
        case DEL:
                   fputs("\b \b",stdout);
                   if (p!=*s) --p;
                   break;
        default:
storechar:
                 c = inputtobase(c,mode|tmpmode);    // convert to internal "base" form
                 *p++ = c;                               // save base in string
                 tmpmode = 0;
                 fputs(basetooutput(c),stdout);             // echo output form
                 break;
        }
    }
breakwhile:
    *p++ = 0;
err:
    return p==*s?NULL:*s;
}



/* allocate integer array */
I *ma(I n){R(I*)malloc(n*sizeof(I));}

/* move integers */
void mv(I*d,I*s,I n){DO(n,d[i]=s[i]);}

/* table rank, product of dimensions d[0..r-1] */
I tr(I r,I*d){I z=1;DO(r,z=z*d[i]);R z;}

/* generate (allocate and initialize) new abstract array 
   of type t */
A ga(I t,I r,I*d){I n;A z=(A)ma(sizeof*z+r+(n=tr(r,d)));
    AT(z)=t;AR(z)=r;AN(z)=n;AK(z)=sizeof*z+(-1+AR(z))*sizeof(I);
    mv(AD(z),d,r);R z;}

/* integer scalar */
A i0(I i){A z=ga(INT,0,0);*AV(z)=i;R z;}

/* integer vector */
A i1(I n,I*v){A z=ga(INT,1,(I[]){n});mv(AV(z),v,n);R z;}



/* integer-encoded numbers in the NUM type */

enum {
    IMM_BIT = 16,
    IMM_MASK = (1<<IMM_BIT)-1,
    IMM_SIGN = 1<<(IMM_BIT-1),
    BANK_BIT = sizeof(I)*CHAR_BIT - IMM_BIT,
    BANK_MASK = ((1<<BANK_BIT)-1) << IMM_BIT,
};
#define encodenum(bnk,idx) ((bnk<<IMM_BIT)|idx)

A bank;
#define BANK_INIT (bank=ga(BOX,1,(I[]){1<<BANK_BIT})), \
                  (AV(bank)[0]=0)
A fixnum;
#define FIXNUM_INIT (fixnum=(A) (AV(bank)[1]=(I)ga(INT,1,(I[]){1<<IMM_BIT}))), \
                    (AV(fixnum)[0]=0)
A flonum;
#define FLONUM_INIT (flonum=(A) (AV(bank)[2]=(I)ga(DBL,1,(I[]){1<<IMM_BIT}))), \
                    (((D*)AV(flonum))[0]=0.0)

/* "number"-encoded integer */

I num(I i){
    if ((unsigned)i&BANK_MASK) {
        if (((unsigned)i&(BANK_MASK|IMM_SIGN))==(BANK_MASK|IMM_SIGN)) { //small negative number
            R i&IMM_MASK;
        }
        else
            R fix(i);
    } else 
        R i;
}

/* number scalar */
A num0(I i){
    A z=i0(num(i));
    AT(z)=NUM;
    R z;
}

/* search full-width integer in bank[FIXNUM]
   allocate if not found
   return encoded (bank,index) */
I fix(I i){
    int j,n;
    for(j=0,n=AV(fixnum)[0];++j<=n;)
        if (AV(fixnum)[j]==i)
            R encodenum(1,j);
    //TODO: check for full table
    // allocate new fixnum table in bank,
    // update global fixnum pointer
    // use a cursor variable here instead of constant 1
    AV(fixnum)[j]=i;
    ++AV(fixnum)[0];
    R encodenum(1,j);
}

/* search floating number in bank[FLONUM]
   allocate if not found
   return encoded(bank,index) */
I flo(D d){
    int j,n;
    for(j=0,n=(I)(((D*)AV(flonum))[0]);++j<=n;)
        if (((D*)AV(flonum))[j]==d)
            R encodenum(2,j);
    //TODO: check for full table as in fix()
    ((D*)AV(flonum))[j]=d;
    ++((D*)AV(flonum))[0];
    R encodenum(2,j);
}

I numimm(I n){
    R n&IMM_SIGN?n|BANK_MASK:n;
}

I numint(I n){
    R AV((A)(AV(bank)[((unsigned)n&BANK_MASK)>>IMM_BIT]))[n&IMM_MASK];
}

D numdbl(I n){
    R ((D*)AV((A)(AV(bank)[((unsigned)n&BANK_MASK)>>IMM_BIT])))[n&IMM_MASK];
}



/* verb function declarations */

V1(copy);
V1(iota); V2(find);
V2(match);
V1(id); V2(plus);
V1(neg); V2(minus); V2(plusminus);
V2(times);
V2(quotient);
V1(size); V2(from);
V1(sha); V2(rsh);
V1(box);
V2(cat);



/*
   The verb table. The VERBNAME symbolically indexes a
   single functional symbol which has an associated 
   ALPHATAB name and associated functions for monadic
   (single right argument) or dyadic (left and right args) uses.
   Verbs are recognized by the wd() function by being non-whitespace
   non-alphanumeric and then refined by verb() called by newsymb().
   The verb's A representation is a small integer which indexes
   this table or an array of type VRB whose value is a (possibly
   modified) copy of the verb record.
 */
/*         VERBNAME   ALPHA_NAME       vm    vd         f  g  mr lr rr  id */
#define VERBTAB(_) \
        _( ZEROFUNC,  0,               0,    0,         0, 0, 0, 0, 0,  0 ) \
        _( PLUS,      ALPHA_PLUS,      id,   plus,      0, 0, 0, 0, 0,  0 ) \
        _( MINUS,     ALPHA_MINUS,     neg,  minus,     0, 0, 0, 0, 0,  0 ) \
        _( TIMES,     ALPHA_TIMES,     0,    times,     0, 0, 0, 0, 0,  1 ) \
        _( DIVIDE,    ALPHA_COLONBAR,  0,    quotient,  0, 0, 0, 0, 0,  1 ) \
        _( PLUSMINUS, ALPHA_PLUSMINUS, neg,  plusminus, 0, 0, 0, 0, 0,  0 ) \
        _( RBRACE,    ALPHA_RBRACE,    size, from,      0, 0, 0, 0, 0,  0 ) \
        _( IOTA,      ALPHA_IOTA,      iota, find,      0, 0, 0, 0, 0,  0 ) \
        _( BOXF,      ALPHA_RANG,      box,  0,         0, 0, 0, 0, 0,  0 ) \
        _( RHO,       ALPHA_RHO,       sha,  rsh,       0, 0, 0, 0, 0,  0 ) \
        _( COMMA,     ALPHA_COMMA,     0,    cat,       0, 0, 0, 0, 0,  0 ) \
        _( NULLFUNC,         0,        0,    0,         0, 0, 0, 0, 0,  0 ) 
struct v { I c; A (*vm)(); A (*vd)(); I f,g,mr,lr,rr; I id; };
typedef struct v *V; //dynamic verb type
#define VERBTAB_NAME(a, ...) a ,
enum { VERBTAB(VERBTAB_NAME) };     //generate verb symbols

#define VERBTAB_ENT(a, ...) { __VA_ARGS__ },
struct v vt[] = { VERBTAB(VERBTAB_ENT) };  //generate verb table array


/* adverb function declarations */
V1(withl);
V1(withr);
V1(on1);
V2(on2);
V2(amp);
V2(rank);

/*
   The adverb table uses the same struct as a verb but is 
   separated to better distinguish the two classes of object.
 */
/*   ADVNAME  ALPHA_NAME       vm vd    f  g  mr lr rr id) */
#define ADVTAB(_) \
    _(ZEROOP=NULLFUNC, 0,               0, 0,    0, 0, 0, 0, 0, 0) \
    _(WITH,   ALPHA_AMPERSAND, 0, amp,  0, 0, 0, 0, 0, 0) \
    _(RANK,   ALPHA_TWODOTS,   0, rank, 0, 0, 0, 0, 0, 0) \
    _(NULLOP, 0)
#define ADVTAB_NAME(a, ...) a ,
enum { ADVTAB(ADVTAB_NAME) };

struct v ot[] = { ADVTAB(VERBTAB_ENT) };  //generate adverb table array

#define DERIV(...) \
    ((z=ga(VRB,1,(I[]){sizeof(struct v)})), \
    (*((V)AV(z))=(struct v){__VA_ARGS__}), \
    z)

I qv();
enum { ANOUN = 1, AVERB, N_A,
    NN=ANOUN*N_A+ANOUN,
    NV=ANOUN*N_A+AVERB,
    VN=AVERB*N_A+ANOUN,
    VV=AVERB*N_A+AVERB,
};
#define VERBNOUN(x) \
    (qv(x)?AVERB:ANOUN)

#define CONJCASE(a,w) \
    (VERBNOUN(a)*N_A+VERBNOUN(w))

#define LOADV(x) \
    abs((I)x)<(sizeof vt/sizeof*vt)? \
        (x=DERIV(vt[(I)x].c, vt[(I)x].vm, vt[(I)x].vd, vt[(I)x].f, vt[(I)x].g, \
                vt[(I)x].mr, vt[(I)x].lr, vt[(I)x].rr, vt[(I)x].id)) \
        :0

V2(amp){
    A z;
    switch(CONJCASE(a,w)){
    case NN: R 0;
    case NV: R DERIV(ALPHA_AMPERSAND, withl, NULL, 0, 0, 0, 0, 0, 0);
    case VN: R DERIV(ALPHA_AMPERSAND, withr, NULL, 0, 0, 0, 0, 0, 0);
    case VV: R DERIV(ALPHA_AMPERSAND, on1,   on2,  0, 0, 0, 0, 0, 0);
    }
}

V2(rank){
    A z;
    switch(CONJCASE(a,w)){
    case NN: R 0;
    case NV: R 0;
    case VN: LOADV(a);
             switch(AN(w)){
             case 0: R 0;
             case 1: R DERIV(((V)AV(a))->c, ((V)AV(a))->vm, ((V)AV(a))->vd, 0, 0,
                             *AV(w), *AV(w), *AV(w), ((V)AV(a))->id);
             case 2: R DERIV(((V)AV(a))->c, ((V)AV(a))->vm, ((V)AV(a))->vd, 0, 0,
                             AV(w)[0], AV(w)[1], AV(w)[0], ((V)AV(a))->id);
             case 3: R DERIV(((V)AV(a))->c, ((V)AV(a))->vm, ((V)AV(a))->vd, 0, 0,
                             AV(w)[0], AV(w)[1], AV(w)[2], ((V)AV(a))->id);
             default: R 0;
             }
    case VV: LOADV(a);
             LOADV(w);
             R DERIV(((V)AV(a))->c, ((V)AV(a))->vm, ((V)AV(a))->vd, 0, 0,
                     ((V)AV(w))->mr, ((V)AV(w))->lr, ((V)AV(w))->rr, ((V)AV(a))->id);
    }
}

/* Verb macros */

/* create v pointer to access verb properties */
#define LOADVSELF(base) \
    V v=self? \
        abs((I)self)<(sizeof vt/sizeof*vt)? \
            vt+(I)self \
            :(V)AV(self) \
        :vt+base; 

#define LOADFRAME(f,ar,rk) \
    /*P("rk=%d\n",rk);*/ \
    if (AR(ar)-(rk)>0) { \
        /*f = ga(0,AR(ar)?AR(ar)==1?0:1:0,(I[]){AR(ar)-(rk)?AR(ar)-(rk):1});*/ \
        /*mv(AV(f),AR(ar)-(rk)?AD(ar):(I[]){0},AR(ar)-(rk)?AR(ar)-(rk):1);*/ \
        AR(f)=AR(ar)-(rk)>1?1:0; \
        AN(f)=AD(f)[0]=AR(ar)-(rk); \
        AK(f)=((C*)AD(ar))-((C*)f); /*make "indirect" array of ar's frame shape*/ \
    } \
    /*P("AR(f)=%d\n", AR(f));*/ 

#define LFRAME(rk) \
    /*A lf = 0;*/ \
    A lf = &(struct a){.t=INT,.n=0,.r=0}; \
    if ((rk)<0) { LOADFRAME(lf,a,AR(a)+(rk)) } \
    else { LOADFRAME(lf,a,rk) }

#define RFRAME(rk) \
    /*A rf = 0;*/ \
    A rf = &(struct a){.t=INT,.n=0,.r=0}; \
    if ((rk)<0) { LOADFRAME(rf,w,AR(w)+(rk)) } \
    else { LOADFRAME(rf,w,rk) } 

#define LOADCELL(c,ar,rk) \
    if ((rk)>0) { \
        AR(c)=(rk)>1?1:0; \
        AN(c)=AD(c)[0]=rk; \
        AK(c)=((C*)(AD(ar)+AR(ar)-rk))-((C*)c); /* indirect array of ar's cell shape */ \
    }

#define LCELL(rk) \
    A lc = &(struct a){.t=INT,.n=0,.r=0}; \
    if ((rk)<0) { LOADCELL(lc,a,AR(a)-(rk)) } \
    else { LOADCELL(lc,a,rk) }

#define RCELL(rk) \
    A rc = &(struct a){.t=INT,.n=0,.r=0}; \
    if ((rk)<0) { LOADCELL(rc,w,AR(w)-(rk)) } \
    else { LOADCELL(rc,w,rk) }

#define RANK1(base) \
    LOADVSELF(base) \
    RFRAME(v->mr) \
    RCELL(v->mr) \
    if (self&& (vt[base].mr != v->mr)) { \
        /* requested cell is not base cell */ \
    }

#define RANK2(base) \
    LOADVSELF(base) \
    /*pr(a); pr(w);*/ \
    LFRAME(v->lr) \
    RFRAME(v->rr) \
    LCELL(v->lr) \
    RCELL(v->rr) \
    /*P("%d_%d\n",AR(a),AR(w));*/ \
    /*P("%d_%d\n",lf?AR(lf):0,rf?AR(rf):0);*/ \
    /*P("%d_%d\n",lf?AN(lf):0,rf?AN(rf):0);*/ \
    /*pr(lf); pr(rf);*/ \
    /*pr(lc); pr(rc);*/ \
    if (!*AV(match(lf,rf,0))) { /* Frame Agreement */ \
        /*P("no match\n");*/ \
        if (AN(lf)==0) \
        { \
            /*P("reshape_a\n");*/ \
            /*pr(rf); pr(lc);*/ \
            if (AN(lc)>0) \
                rf=cat(rf,lc,0); \
            a=rsh(rf,a,0); \
            /*pr(a);*/ \
        } else \
        if (AN(rf)==0) \
        { \
            /*P("reshape_w\n");*/ \
            /*pr(lf); pr(rc);*/ \
            if (AN(rc)>0) \
                lf=cat(lf,rc,0); \
            w=rsh(lf,w,0); \
            /*pr(w);*/ \
        } \
    } \
    if (self&& (vt[base].lr != v->lr && vt[base].rr != v->rr)) { \
        /* requested cells are not base cells */ \
    } else if (self&& vt[base].lr != v->lr) { \
        /* left cell is not base cell */ \
    } else if (self&& vt[base].rr != v->rr) { \
        /* right cell is not base cell */ \
    }

#define DECLFG(base) \
    LOADVSELF(base) \
    A fs = (A)v->f; \
    A gs = (A)v->g; \
    V1((*f1)) = (A(*)())v->f; \
    V1((*g1)) = (A(*)())v->g; \
    V2((*f2)) = (A(*)())v->f; \
    V2((*g2)) = (A(*)())v->g; \

V1(withl){ DECLFG(WITH); R g2(fs,w,gs); }
V1(withr){ DECLFG(WITH); R f2(w,gs,fs); }
V1(on1){ DECLFG(WITH); R f1(g1(w,gs),fs); }
V2(on2){ DECLFG(WITH); R f2(g1(a,gs),g1(w,gs),fs); }

V2(match){
    if(a==w) R num0(1);
    if(a && w) {
        if(AR(a)!=AR(w)
        || AN(a)!=AN(w)
          ) {
            R num0(0);
        }
        DO(AN(a),if(AV(a)[i]!=AV(w)[i])R 0;)
    } else {
        R num0(0);
    }
    R num0(1);
}

enum { IMM = 1, FIX, FLO, NUM_TYPES };
#define TYPEPAIR(a,b) \
    ((a)*NUM_TYPES+(b))

#define TYPENUM(a) ((a)&BANK_MASK? \
        (AT(((A)AV(bank)[((a)&BANK_MASK)>>IMM_BIT]))==DBL? \
             FLO \
             :FIX) \
        :IMM)

#define NUMERIC_TYPES(a,b) \
    TYPEPAIR(TYPENUM(a), TYPENUM(b))

#define MON_MATH_FUNC(func,z,y) \
    switch(TYPENUM(y)){ \
    case IMM: z=num(func numimm(y)); break; \
    case FIX: z=num(func numint(y)); break; \
    case FLO: z=flo(func numdbl(y)); break; \
    }

#define BIN_MATH_FUNC(func,z,x,y) \
     switch(NUMERIC_TYPES(x,y)){ \
     case TYPEPAIR(IMM,IMM): z=num(numimm(x) func numimm(y)); break; \
     case TYPEPAIR(IMM,FIX): z=num(numimm(x) func numint(y)); break; \
     case TYPEPAIR(IMM,FLO): z=flo(numimm(x) func numdbl(y)); break; \
     case TYPEPAIR(FIX,IMM): z=num(numint(x) func numimm(y)); break; \
     case TYPEPAIR(FIX,FIX): z=num(numint(x) func numint(y)); break; \
     case TYPEPAIR(FIX,FLO): z=flo(numint(x) func numdbl(y)); break; \
     case TYPEPAIR(FLO,IMM): z=flo(numdbl(x) func numimm(y)); break; \
     case TYPEPAIR(FLO,FIX): z=flo(numdbl(x) func numint(y)); break; \
     case TYPEPAIR(FLO,FLO): z=flo(numdbl(x) func numdbl(y)); break; \
     }

/* return w */
V1(id){R w;}
/* add */
V2(plus){
    RANK2(PLUS)
    A z=ga(NUM,AR(w),AD(w));
    //P("%d\n",v->id);
    DO(AN(z), BIN_MATH_FUNC(+,AV(z)[i],AV(a)[i],AV(w)[i]) )
    R z;}

/* negate w */
V1(neg){ RANK1(MINUS)
    A z=copy(w,0);
    DO(AN(z), MON_MATH_FUNC(-,AV(z)[i],AV(z)[i]))
    R z;}
V2(minus){
    RANK2(MINUS)
    A z=ga(0,AR(w),AD(w));
    DO(AN(z), BIN_MATH_FUNC(-,AV(z)[i],AV(a)[i],AV(w)[i]))
    R z;}

/* return sum and difference */
V2(plusminus){ w=cat(w,neg(w,0),0); a=cat(a,a,0); R plus(a,w,0);}

V2(times){
    RANK2(TIMES)
    A z=ga(0,AR(w),AD(w));
    DO(AN(z), BIN_MATH_FUNC(*,AV(z)[i],AV(a)[i],AV(w)[i]))
    R z;}

V2(quotient){
    RANK2(DIVIDE)
    A z=ga(0,AR(w),AD(w));
    DO(AN(w),
            if (AV(w)[i]==0 || AV(w)[i]==flo(0.0)) { AV(z)[i]=0; }
            else //BIN_MATH_FUNC(/,AV(z)[i],AV(a)[i],AV(w)[i])
                switch(NUMERIC_TYPES(AV(a)[i],AV(w)[i])){ \
                case TYPEPAIR(IMM,IMM): AV(z)[i]=flo((D)numimm(AV(a)[i]) / numimm(AV(w)[i])); break; \
                case TYPEPAIR(IMM,FIX): AV(z)[i]=flo((D)numimm(AV(a)[i]) / numint(AV(w)[i])); break; \
                case TYPEPAIR(IMM,FLO): AV(z)[i]=flo((D)numimm(AV(a)[i]) / numdbl(AV(w)[i])); break; \
                case TYPEPAIR(FIX,IMM): AV(z)[i]=flo((D)numint(AV(a)[i]) / numimm(AV(w)[i])); break; \
                case TYPEPAIR(FIX,FIX): AV(z)[i]=flo((D)numint(AV(a)[i]) / numint(AV(w)[i])); break; \
                case TYPEPAIR(FIX,FLO): AV(z)[i]=flo((D)numint(AV(a)[i]) / numdbl(AV(w)[i])); break; \
                case TYPEPAIR(FLO,IMM): AV(z)[i]=flo(   numdbl(AV(a)[i]) / numimm(AV(w)[i])); break; \
                case TYPEPAIR(FLO,FIX): AV(z)[i]=flo(   numdbl(AV(a)[i]) / numint(AV(w)[i])); break; \
                case TYPEPAIR(FLO,FLO): AV(z)[i]=flo(   numdbl(AV(a)[i]) / numdbl(AV(w)[i])); break; \
                }

            //AV(z)[i]=AV(w)[i]?AV(a)[i]/AV(w)[i]:AV(a)[i]?INT_MIN:0
        )
    R z;}

/* make a copy */
V1(copy){I n=AN(w); A z=ga(AT(w),AR(w),AD(w)); mv(AV(z),AV(w),n); R z;}
/* generate index vector */
V1(iota){I n=*AV(w);A z=ga(0,1,&n);DO(n,AV(z)[i]=i);R z;}
/* not implemented */
V2(find){}

/* length of first dimension */
V1(size){A z=ga(0,0,0);*AV(z)=AR(w)?*AD(w):1;R z;}
/* index */
V2(from){I r=AR(w)-1,*d=AD(w)+1,n=tr(r,d);
 A z=ga(AT(w),r,d);mv(AV(z),AV(w)+(n**AV(a)),n);R z;}

/* pack array into a scalar */
V1(box){A z=ga(1,0,0);*AV(z)=(I)w;R z;}
/* catenate two arrays */
V2(cat){
     //P("cat:\n"); pr(a); pr(w);
     I an=AN(a),wn=AN(w),n=an+wn;
     A z=ga(AT(w),1,&n);mv(AV(z),AV(a),an);mv(AV(z)+an,AV(w),wn);
     //pr(z);
     R z;}

/* return the shape of w */
V1(sha){A z=ga(0,1,&AR(w));mv(AV(z),AD(w),AR(w));R z;}
/* reshape w to dimensions a */
V2(rsh){I r=AR(a)?*AD(w):1,n=tr(r,AV(a)),wn=AN(w);
 /*P("rsh:\n"); pr(a); pr(w);*/
 A z=ga(AT(w),r,AV(a));
 mv(AV(z),AV(w),wn=n>wn?wn:n);
 if(n-=wn)mv(AV(z)+wn,AV(z),n);
 /*P("#");pr(z);*/
 R z;}



/* print symbol */
ps(A i){P("%s",(char*)AV(i));}
/* print verb */
pv(i){P("%s",basetooutput(alphatab[vt[i].c].base));}
/* print character */
pc(i){qv(i)?pv(i):P("%s",basetooutput(i));}
/* print number */
pi(i){
    switch(TYPENUM(i)){
    case IMM: P("%d ",numimm(i)); break;
    case FIX: P("%d ",numint(i)); break;
    case FLO: P("%f ",numdbl(i)); break;
    }
}
/* print newline */
nl(){P("\n");}
/* print any array */
pr(A w){
    if (abs((I)w)<256)
        pc(w);
    else {
        I r=AR(w),*d=AD(w),n=AN(w);
        if(w==null)R 0;
        DO(r,pi(d[i])); P("#"); nl();
        if(AT(w)==1)
            DO(n,P("< ");pr((A)(AV(w)[i])))
        else if(AT(w)==SYMB)
            ps(w);
        else
            DO(n,pi(AV(w)[i]));
    }
    nl();
}

/*
   Character classes used by the symbol table function
   findsymb() and by the scanning function wd().
 */
#define ALPHALOWER "abcdefghijklmnopqrstuvwxyz"
#define ALPHAUPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define HIMINUS ((char[]){MODE1('@'), 0})
#define DIGIT "0123456789"
#define SPACE " \t"
#define ZEROCL ""

/*
   The symbol table is implemented as a trie data structure
   with 52-way branching, allowing strict alphabetic symbols.
 */
struct st { A a; struct st *tab[52]; } st; /* symbol-table */
#define ST_INIT (st.a = null) /* initialize symbol table root value */

char *alph=ALPHAUPPER ALPHALOWER; /* symbol-table collation-ordered set */
/*
   find symbol in symbol table.
    mode 0: search trie for longest-prefix match.
        ret root on fail (val=null).
        update input string point to remainder (no change on fail).
    mode 1: search and allocate.
        ret new leaf node (val=null).
        update input string.  */
struct st *findsymb(struct st *st, char **s, int mode) {
    int code;
    struct st *last = st;
    char *lasp = *s;
    while(isalpha(**s)){
        code = strchr(alph,**s)-alph;
        if (st->tab[code]){
            st=st->tab[code];
            (*s)++;
            if (st->a!=null){
                last = st;
                lasp = *s;
            }
        } else
            switch(mode){
            case 0:             //prefix search
                    *s = lasp;
                    return last;
            case 1:             //defining search
                    st->tab[code] = calloc(1,sizeof(struct st));
                    st->a=null;
                    st=st->tab[code];
                    (*s)++;
                    break;
            }

    }
    return st; 
}


/*
   Predicate table for pattern matching in the parser ex().
 */
#define PREDTAB(_) \
    _( ANY = 1,   qa, 1                              )                    \
    _( VAR = 2,   qp, abs(a)>256 && AT(((A)a))==SYMB )                    \
    _(NOUN = 4,   qn, abs(a)>256 && AT(((A)a))!=SYMB )                    \
    _(VERB = 8,   qv, /*0<=abs(a)                                         \
                      && abs(a)<'a'                                       \
                      &&*/ abs(a)<(sizeof vt/sizeof*vt)                   \
                      && (vt[a].vm || vt[a].vd)      )                    \
    _(ADV  = 16,  qo, abs(a)>(sizeof vt/sizeof*vt)                        \
                      && abs(a)<(sizeof vt/sizeof*vt+sizeof ot/sizeof*ot) \
                      && (ot[a-NULLFUNC].vm || ot[a-NULLFUNC].vd) )       \
    _(ASSN = 32,  qc, a==MODE1('<')                  )                    \
    _(MARK = 64,  qm, ((A)a)==mark                   )                    \
    _(LPAR = 128, ql, a=='('                         )                    \
    _(RPAR = 256, qr, a==')'                         )                    \
    _(NULP = 512, qu, ((A)a)==null                   )
#define PRED_FUNC(X,Y,...) Y(a){R __VA_ARGS__;}
PREDTAB(PRED_FUNC)                      //generate predicate functions
#define PRED_ENT(a,b,...) b ,
I(*q[])() = { PREDTAB(PRED_ENT) };      //generate predicate function table
#define PRED_ENUM(a,...) a ,
enum predicate { PREDTAB(PRED_ENUM)     //generate predicate symbols
                 EDGE = MARK+ASSN+LPAR,
                 AVN = VERB+NOUN };
/* encode predicate applications into a binary number,
   a bitset represented as a bit mask */
int classify(A a){ int i,v,r;
    for(i=0,v=1,r=0;i<(sizeof q/sizeof*q);i++,v*=2)if(q[i](a))r|=v; R r;}

/*
   Parse table for processing expressions on top of the right-stack
 */
#define PARSETAB(_) \
    /*INDEX  PAT1      PAT2  PAT3  PAT4  ACTION*/                                           \
    /*     =>t[0]      t[1]  t[2]  t[3]        */                                           \
    _(MONA,  EDGE,     VERB, NOUN, ANY,  {stackpush(rstk,t[3]);                             \
                                          stackpush(rstk,vt[(I)t[1]].vm(t[2],t[1]));        \
                                          stackpush(rstk,t[0]);} )                          \
    _(MONB,  EDGE+AVN, VERB, VERB, NOUN, {stackpush(rstk,vt[(I)t[2]].vm(t[3],t[2]));        \
                                          stackpush(rstk,t[1]);                             \
                                          stackpush(rstk,t[0]);} )                          \
    _(DYAD,  EDGE+AVN, NOUN, VERB, NOUN, {stackpush(rstk,vt[(I)t[2]].vd(t[1],t[3],t[2]));   \
                                          stackpush(rstk,t[0]);} )                          \
    _(SPEC,  VAR,      ASSN, AVN,  ANY,  {char *s=(char*)AV(t[0]);                          \
                                          struct st *slot = findsymb(&st,&s,1);             \
                                          stackpush(rstk,t[3]);                             \
                                          stackpush(rstk,slot->a=t[2]);} )                  \
    _(PUNC,  LPAR,     ANY,  RPAR, ANY,  {stackpush(rstk,t[3]);                             \
                                          stackpush(rstk,t[1]);} )                          \
    _(FAKL,  MARK,     ANY,  RPAR, ANY,  {stackpush(rstk,t[3]);                             \
                                          stackpush(rstk,t[1]);                             \
                                          stackpush(rstk,t[0]);} )                          \
    _(FAKR,  EDGE+AVN, LPAR, ANY,  NULP, {stackpush(rstk,t[3]);                             \
                                          stackpush(rstk,t[2]);                             \
                                          stackpush(rstk,t[0]);} )                          \
    _(NOACT, 0,        0,    0,    0,    0;)
#define PARSETAB_PAT(name, pat1, pat2, pat3, pat4, ...) { pat1, pat2, pat3, pat4 },
struct parsetab { I c[4]; } parsetab[] = { PARSETAB(PARSETAB_PAT) };
#define PARSETAB_INDEX(name, ...) name,
enum { PARSETAB(PARSETAB_INDEX) };
#define PARSETAB_ACTION(name, pat1, pat2, pat3, pat4, ...) case name: __VA_ARGS__ break;

/*
   Stack data structure.
   if stkp->top!=0 then top element is stkp->a[stkp->top-1]
 */
typedef struct stack { int top; A a[1]; } stack; /* top==0::empty */
#define stackpush(stkp,el) ((stkp)->a[(stkp)->top++]=(el))
#define stackpop(stkp) ((stkp)->a[--(stkp)->top])


/*
   Evaluate an expression according to APL rules.
   execute right-to-left.
   The arguments to a function are the single element to the left
   and the entire expression to the right.
   Push expression to left-stack.
   The loop:
       move element to right stack, looking up variables.
       check right stack for patterns
             and execute appropriate actions, reducing the stack.
   Terminate loop when left-stack is depleted and no further patterns
   apply to the right stack.
   Return remaining element.
   Print error if expression has not fully reduced (unrecognized function).
 */
A ex(I *e){I a=*e;

    int i,j,n,docheck;
    stack *lstk,*rstk;
    A t[4];
    //for(i=0;e[i];i++)pr((A)e[i]);
    for(i=0,j=0;e[i];i++)if(qp(e[i]))j+=AD(((A)e[i]))[0];
    n=i;
    lstk=malloc(sizeof*lstk + (1+i+j)*sizeof(A));
    lstk->top = 0;
    stackpush(lstk,mark);
    for(i=0;i<n;i++) stackpush(lstk,((A)e[i])); //push to lstk
    rstk=malloc(sizeof*rstk + (1+i+j)*sizeof(A));
    rstk->top = 0;
    stackpush(rstk,null);
    while(lstk->top){ //push to rstk
        //for(i=0;i<lstk->top;i++)pr(lstk->a[i]); fflush(0);
        //for(i=0;i<rstk->top;i++)pr(rstk->a[i]); fflush(0);
        a=(I)stackpop(lstk);
        if(qp(a)){                              //parse defined names now
            if (rstk->top && qc((I)rstk->a[rstk->top-1])){
                stackpush(rstk,((A)a));
            } else { char *s,*p; struct st *tab; A sa=(A)a;
                s = p = (char*)AV(sa);
                tab=findsymb(&st,&p,0);
                while(*p){
                    if (tab==&st){
                        P("Error %s undefined\n",p);
                        R (A)a;
                    }
                    if (tab->a!=null){          // found a defined prefix
                        stackpush(lstk,newsymb(s,p-s,2)); //pushback prefix
                        s=p;
                    }
                    tab=findsymb(&st,&p,0);
                }
                stackpush(rstk,tab->a);
            }
        }else{stackpush(rstk,((A)a));}

        docheck=1;
        while(docheck) {
            docheck=0;
            if(rstk->top>=4){ I c[4];  //enough elements to check?
                for(j=0;j<4;j++)
                    c[j] = classify(rstk->a[rstk->top-1-j]);  //summarize attributes
                for(i=0;i<(sizeof parsetab/sizeof*parsetab);i++) { //match against table
                    if( (c[0]&parsetab[i].c[0])
                      &&(c[1]&parsetab[i].c[1])
                      &&(c[2]&parsetab[i].c[2])
                      &&(c[3]&parsetab[i].c[3]) ) {
                        docheck=1;
                        t[0]=stackpop(rstk);
                        t[1]=stackpop(rstk);
                        t[2]=stackpop(rstk);
                        t[3]=stackpop(rstk);
                        switch(i){
                            PARSETAB(PARSETAB_ACTION)

                        }
                    }
                }
            }
        }

    }

    //for(i=0;i<rstk->top;i++)pr(rstk->a[i]); fflush(0);
    stackpop(rstk); //mark
    a = (I)stackpop(rstk);
    if (rstk->top && rstk->a[rstk->top-1]!=null){
        P("Error extra elements on stack:\n");
        while ((i=(I)stackpop(rstk))!=(I)null)
            pr((A)i);
        P("\n");
    }
    free(rstk);
    free(lstk);
    R (A)a;
}


/* lookup character (in internal representation) in verb table */
verb(c){I i=0;
    for(;vt[++i].c;)
        if(alphatab[vt[i].c].base==c)
            R i;
    for(i=0;ot[++i].c;)
        if(alphatab[ot[i].c].base==c)
            R NULLFUNC+i; //operators take the values after the last verb
    R 0;}

/*
   construct a single scanned token given a string pointer and length.
 */
A newsymb(C *s,I n,I state){
    I t;
    //P("%d\n",n);DO(n,P("%c",s[i]))P("\n");
    //if(strchr(DIGIT,*s) || strchr(HIMINUS,*s)) {
    switch(state){
    case 1:
        {
        char *end;
        s=strndup(s,n);
        DO(n,if(s[i]==alphatab[ALPHA_MACRON].base)s[i]='-')
        //A z=ga(INT,0,0); *AV(z)=strtol(s,&end,10);
        A z=num0(strtol(s,&end,10));
        while(((C*)end-s) < n){
            //A r=ga(INT,0,0); *AV(r)=strtol(end,&end,10);
            A r=num0(strtol(end,&end,10));
            z=cat(z,r,0);
        }
        free(s);
        R z;
        }
    //} else if(strchr(ALPHAUPPER ALPHALOWER,*s)) {
    case 2:
        {
        A z=ga(SYMB,1,(I[]){n+1});
        mv(AV(z),(I*)s,n+3/4);
        ((C*)AV(z))[n] = 0;
        R z;
        }
    //} else {
    case 3:
        {
        I c=verb(*s);
        R (A)(c?c:(I)*s);
        }
    }
}


/*
   scanner table for word formation
   columns are indexed by matched character classes
   rows are indexed by the current state of the scanner automaton
   element returned yields newstate when divided by ten
                and yields action code when remainder from division by ten is taken.
 */
char *cclass[] = {0, ALPHAUPPER ALPHALOWER, HIMINUS, DIGIT, SPACE};
int wdtab[][5] = {
    /*char-class*/
    /*0     a     -     d      s*/    /* state  */
    { 30+2, 20+2, 10+2, 10+2,  0+0 }, /*  0 init   */
    { 30+1, 20+1, 10+0, 10+0, 10+0 }, /* 10 number */
    { 30+1, 20+0, 10+1, 10+1,  0+1 }, /* 20 name   */
    { 30+1, 20+1, 10+1, 10+1,  0+1 }, /* 30 other  */
    /*{newstate+action,...}*/
};

/* construct new object giving start and end string positions */
#define emit(a,b,c) (*z++=(I)newsymb(s+(a),(b)-a,c)); 
/*
   scan expression for alphabetic and numeric parts 
   delimited by whitespace
 */
I *wd(C *s){
    I a,b,n=strlen(s),*e=ma(n+1),*z=e;
    int i,j,i_,state,oldstate;
    C c, *cp;
    state=0;
    for(i=0;i<n;i++){
        c=s[i];
        //P("'%c'\n",c);
        a=0;
        for(i_=1;i_<(sizeof cclass/sizeof*cclass);i_++) {
            if (cp=strchr(cclass[i_],c)) {
                a=i_;
                break;
            }
        }
        b=wdtab[state][a];
        //P("%d\n",b);
        oldstate=state;
        state=b/10;
        switch(b%10){ //encoded actions
        case 0: break;
        case 1: emit(j,i,oldstate);
        case 2: j=i; break;
        }
    }
    *z++=0;
    R e;}


/*
   setup special terminal mode if connected to a terminal
   perform read-eval-print loop
 */
int main(){C *s=NULL;int n=0;C *prompt="\t";
    ST_INIT; /* initialize symbol table root value */
    BANK_INIT;
    FIXNUM_INIT;
    FLONUM_INIT;
    if (isatty(fileno(stdin))) specialtty();
    while(getln(prompt,&s,&n))
        pr(ex(wd(s)));
    if (isatty(fileno(stdin))) restoretty();
    R 0;
}

