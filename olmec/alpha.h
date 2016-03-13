

/*
   The Alphabet table defines the input and output of character data
   (including non-ascii APL symbols).
 */
/* ALPHA_##NAME  base  ext input  output
   ALPHA_##NAME indexes the table
   base is the internal representation, ideally the UCS4 code
   ext corresponds to 'mode' in get_line 0==normal_keyboard 1==alt_keyboard
   input and output are defined as strings so they can extend to multichar
   input- and output- translations.
   currently only output uses strings. only the first char of *input
   is currently matched.
 */
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
    /*_( PLUSMINUS, MODE1('g'), 1, "g", ESC(n)"g""\xE" ) */\
    /* xterm alt graphics chars */ \
    _( IBEAM,     0x2336,     1, "!", "\xe2\x8c\xb6" ) \
    _( DELTILDE,  0x236b,     1, "@", "\xe2\x8d\xab" ) \
    _( DELTASTIL, 0x234b,     1, "#", "\xe2\x8d\x8b" ) \
    _( DELSTIL,   0x2352,     1, "$", "\xe2\x8d\x92" ) \
    _( CIRCSTIL,  0x233d,     1, "%", "\xe2\x8c\xbd" ) \
    _( CIRCBACK,  0x2349,     1, "^", "\xe2\x8d\x89" ) \
    _( CIRCBAR,   0x2296,     1, "&", "\xe2\x8a\x96" ) \
    _( CIRCSTAR,  0x235F,     1, "*", "\xe2\x8d\x9f" ) \
    _( DNCRTIL,   0x2371,     1, "(", "\xe2\x8d\xb1" ) \
    _( UPCRTIL,   0x2372,     1, ")", "\xe2\x8d\xb2" ) \
    _( QEQ,       0x2338,     1, "_", "\xe2\x8c\xb8" ) \
    _( BANG,      '!',        1, "_", "!" ) \
    _( DOMINO,    0x2339,     1, "+", "\xe2\x8c\xb9" ) \
    _( SLASHBAR,  0x233F,     1, "/", "\xE2\x8c\xbf" ) \
    _( BACKBAR,   0x2340,     1, "\\", "\xE2\x8d\x80" ) \
    _( DELTUND,   0x2359,     1, "|", "\xe2\x8d\x99" ) \
    _( TWODOTS,   0x00a8,     1, "1", ESC(o)"(""\xE" ) \
    _( DIAERESIS, MODE1('!'), 1, "!", /*U+00a8*/ /*"\xc2"*/"\xa8" ) \
    _( HIMINUS,   0x00af,     1, "2", ESC(o)"/""\xE" ) \
    _( HIMIN,     0x00af,     1, "2", "\xc2\xaf" ) \
    _( HIMN,      0x00af,     1, "2", "\xaf" ) \
    _( MACRON,    MODE1('@'), 1, "@", /*U+00af*/ /*"\xc2"*/"\xaf" ) \
    _( NOTEQUAL,  MODE1('|'), 1, "|", ESC(n)"|""\xE" ) \
    _( LESS,            '<',  1, "3", "<" ) \
    _( LESSEQ,    0x2264,     1, "4", /*U+2264*/ "\xe2\x89\xa4") \
    _( LESSEQUAL, MODE1('$'), 1, "$", ESC(n)"y""\xE" ) \
    _( EQALT,           '=',  1, "5", "=" ) \
    _( MOREEQ,    0x2265,     1, "6", /*U+2265*/ "\xe2\x89\xa5") \
    _( MOREEQUAL, MODE1('^'), 1, "^", ESC(n)"z""\xE" ) \
    _( MORE,            '>',  1, "7", ">" ) \
    _( EQSLASH,   MODE1('*'), 1, "8", ESC(n)"|""\xE" ) \
    _( OR,        0x2228,     1, "9", "\xe2\x88\xa8" ) \
    _( AND,       0x2227,     1, "0", "\xe2\x88\xa7" ) \
    _( TURTLE,    0x2361,     1, "q", "\xe2\x8d\xa1" ) \
    _( QUESTBOX,  0x2370,     1, "q", "\xe2\x8d\xb0" ) \
    _( QUEST,           '?',  1, "q", "?" ) \
    _( INVQUEST,  MODE1('Q'), 1, "Q", ESC(o)"?""\xE" ) \
    _( OMEGA,     0x2375,     1, "w", /*U+2375*/ "\xe2\x8d\xb5" ) \
    _( OMEGAUND,  0x2379,     1, "W", "\xe2\x8d\xb9" ) \
    _( EPSILON,   0x2208,     1, "e", /*U+2208*/ "\xe2\x88\x88" ) \
    _( EPSILUND,  0x2377,     1, "E", "\xe2\x8d\xb7" ) \
    _( RHO,       0x2374,     1, "r", /*U+2374*/ "\xe2\x8d\xb4" ) \
    _( DOUBR,     0x211d,     1, "R", "\xe2\x84\x9d" ) \
    _( CIRCJOT,   0x233e,     1, "t", "\xe2\x8c\xbe" ) \
    _( TILDOP,    '~',        1, "t", "~" ) \
    _( TILDEOP,   0x223c,     1, "t", /*U+223c*/ "\xe2\x88\xbe" ) \
    _( STLTILDE,  0x236d,     1, "T", "\xe2\x8d\xad" ) \
    _( UPARROW,   0x2191,     1, "y", /*U+2191*/ "\xe2\x86\x91" ) \
    _( YEN,       MODE1('Y'), 1, "Y", ESC(o)"%""\xE" ) \
    _( DNARROW,   0x2193,     1, "u", /*U+2193*/ "\xe2\x86\x93" ) \
    _( NULL,      0x2300,     1, "U", "\xe2\x8c\x80" ) \
    _( ZIT,       0x2311,     1, "U", "\xe2\x8c\x91" ) \
    _( RUDDER,    0x2388,     1, "U", "\xe2\x8e\x88" ) \
    _( ZILDE,     0x236c,     1, "U", "\xe2\x8d\xac" ) \
    _( POSITION,  0x2316,     1, "U", "\xe2\x8c\x96" ) \
    _( IOTA,      0x2373,     1, "i", /*U+2373*/ "\xe2\x8d\xb3" ) \
    _( IOTAUND,   0x2378,     1, "I", "\xe2\x8d\xb8" ) \
    _( CIRCLE,    0x25cb,     1, "o", /*U+25cb*/ "\xe2\x97\x8b" ) \
    _( DIARCIRC,  0x2365,     1, "O", "\xe2\x8d\xa5" ) \
    _( STAROP,    0x22c6,     1, "p", /*U+22c6*/ "\xe2\x8b\x86" ) \
    _( POUND,     MODE1('}'), 1, "P", ESC(n)"}""\xE" ) \
    _( LTTACK,    0x22a3,     1, "{", "\xe2\x8a\xa3" ) \
    _( RTTACK,    0x22a2,     1, "}", "\xe2\x8a\xa2" ) \
    _( LTARROW,   0x2190,     1, "[", /*U+2190*/ "\xe2\x86\x90" ) \
    _( RTARROW,   0x2192,     1, "]", /*U+2192*/ "\xe2\x86\x92" ) \
    _( ALPHA,     0x237a,     1, "a", /*U+237a*/ "\xe2\x8d\xba" ) \
    _( ALPHAUND,  0x2376,     1, "A", "\xe2\x8d\xb6" ) \
    _( LEFTCEIL,  0x2308,     1, "s", /*U+2308*/ "\xe2\x8c\x88" ) \
    _( SQUISHQ,   0x2337,     1, "S", "\xe2\x8c\xb7" ) \
    _( LEFTFLOOR, 0x230a,     1, "d", /*U+230a*/ "\xe2\x8c\x8a" ) \
    _( QMORE,     0x2344,     1, "D", "\xe2\x8d\x84" ) \
    _( SPOOK,     0x2363,     1, "f", "\xe2\x8d\xa3" ) \
    _( UNDBAR2,         '_',  1, "f", "_" ) \
    _( SAME,      0x2261,     1, "F", "\xe2\x89\xa1" ) \
    _( NABLA,     0x2207,     1, "g", /*U+2207*/ "\xe2\x88\x87" ) \
    _( DELTASTL,  0x234b,     1, "G", "\xe2\x8d\x8b" ) \
    _( INCREMENT, 0x2206,     1, "h", /*U+2206*/ "\xe2\x88\x86" ) \
    _( DELSTL,    0x2352,     1, "H", "\xe2\x8d\x92" ) \
    _( JOT,      0x2218,     1, "j", /*U+2218*/ "\xe2\x88\x98" ) \
    _( DIARJOT,   0x2364,     1, "J", "\xe2\x8d\xa4" ) \
    _( DIARDOT,   0x2235,     1, ".", "\xe2\x88\xb5" ) \
    _( MOUSE,     0x2362,     1, "k", "\xe2\x8d\xa2" ) \
    _( KWOTE,          '\'',  1, "k", "'" ) \
    _( QLESS,     0x2343,     1, "K", "\xe2\x8d\x83" ) \
    _( QUAD,      0x2395,     1, "l", /*U+2395*/ "\xe2\x8e\x95" ) \
    _( QQUAD,     0x235e,     1, "L", "\xe2\x8d\x9e" ) \
    _( QSLASH,    0x2341,     1, ";", "\xe2\x8d\x81" ) \
    _( QBACKSL,   0x2342,     1, ":", "\xe2\x8d\x82" ) \
    _( QJOT,      0x233b,     1, "\"", "\xe2\x8c\xbb" ) \
    _( SUBSET,    0x2282,     1, "z", /*U+2282*/ "\xe2\x8a\x82" ) \
    _( LTSHOESTL, 0x2367,     1, "Z", "\xe2\x8d\xa7" ) \
    _( SUPERSET,  0x2283,     1, "x", /*U+2283*/ "\xe2\x8a\x83" ) \
    _( QDIAMOND,  0x233a,     1, "X", "\xe2\x8c\xba" ) \
    _( CAP,       0x2229,     1, "c", /*U+2229*/ "\xe2\x88\xa9" ) \
    _( UPSHOEJOT, 0x235d,     1, "C", "\xe2\x8d\x9d" ) \
    _( CUP,       0x222a,     1, "v", /*U+222a*/ "\xe2\x88\xaa" ) \
    _( DNSHOESTL, 0x2366,     1, "V", "\xe2\x8d\xa6" ) \
    _( UPTACK,    0x22a5,     1, "b", /*U+22a5*/ "\xe2\x8a\xa5" ) \
    _( UPTAKJOT,  0x234e,     1, "B", "\xe2\x8d\x8e" ) \
    _( DNTACK,    0x22a4,     1, "n", /*U+22a4*/ "\xe2\x8a\xa4" ) \
    _( DNTAKJOT,  0x2355,     1, "N", "\xe2\x8d\x95" ) \
    _( QDELTA,    0x234d,     1, "m", "\xe2\x8d\x8d" ) \
    _( QDEL,      0x2354,     1, "M", "\xe2\x8d\x94" ) \
    _( PIPEA,     '|',        1, "m", "|" ) \
    _( DIVIDES,   0x2223,     1, "m", /*U+2223*/ "\xe2\x88\xa3" ) \
    _( DOT,       MODE1('~'), 1, "~", ESC(n)"~""\xE" ) \
    _( DIAM,      0x22c4,     1, "`", "\xe2\x8b\x84" ) \
    _( COMMABAR,  0x236a,     1, ",", "\xe2\x8d\xaa" ) \
    _( DIAMOND,   MODE1('`'), 1, "`", ESC(n)"`""\xE" ) \
    _( PI,        MODE1('{'), 1, "{", ESC(n)"{""\xE" ) \
    _( EURO,      MODE1('e'), 1, "e", "\xe2\x82\xac" ) \
    _( CENT,      MODE1('e'), 1, "e", ESC(o)"\"""\xE" ) \
    _( HBAR0,     MODE1('o'), 1, "o", ESC(n)"o""\xE" ) \
    _( HBAR1,     MODE1('p'), 1, "p", ESC(n)"p""\xE" ) \
    _( HBAR3,     MODE1('q'), 1, "q", ESC(n)"q""\xE" ) \
    _( HBAR4,     MODE1('r'), 1, "r", ESC(n)"r""\xE" ) \
    _( HBAR5,     MODE1('s'), 1, "s", ESC(n)"s""\xE" ) \
    _( GRAYBOX,   MODE1('a'), 1, "a", ESC(n)"a""\xE" ) \
    _( DEGREE,    MODE1('f'), 1, "f", ESC(n)"f""\xE" ) \
    _( HT,        '\x9',      0, "\t", ESC(n)"b""\xE" ) \
    _( NL,        '\xa',      0, "\n", ESC(n)"h""\xE" ) \
    _( LF,        '\xa',      0, "\n", ESC(n)"e""\xE" ) \
    _( VT,        '\xb',      0, "\v", ESC(n)"i""\xE" ) \
    _( FF,        '\xc',      0, "\f", ESC(n)"c""\xE" ) \
    _( CR,        '\xd',      0, "\r", ESC(n)"d""\xE" ) \
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
    _( GUILLEFT,  MODE1('<'), 1, "<", ESC(o)"+""\xE" ) \
    _( GUILRIGHT, MODE1('>'), 1, ">", ESC(o)";""\xE" ) \
    _( COMPL,     MODE1('^'), 1, "^", ESC(o)",""\xE" ) \
    _( TIMES,     MODE1('='), 1, "-", ESC(o)"W""\xE" ) \
    _( DIVIDE,    MODE1('/'), 1, "/", ESC(o)"w""\xE" ) \
    _( PARAGRAPH, MODE1(','), 1, "?", ESC(o)"6""\xE" ) \
    _( CDOT,      MODE1('.'), 1, ".", ESC(o)"7""\xE" ) \
    _( HYPHEN,    MODE1('-'), 1, "-", ESC(o)"-""\xE" ) \
    _( BUTTON,    MODE1('i'), 1, "i", ESC(o)"$""\xE" ) \
    _( SECTION,   MODE1('h'), 1, "h", ESC(o)"'""\xE" ) \
    _( PRIME,     MODE1('\''), 1, "'", ESC(o)"4""\xE" ) \
    _( CIRCC,     MODE1('c'), 1, "c", ESC(o)")""\xE" ) \
    _( ZEROSLASH, MODE1('v'), 1, "v", ESC(o)"X""\xE" ) \
    _( OBAR,      MODE1(';'), 1, ";", ESC(o)":""\xE" ) \
    /* ALPHA_NAME base       ext input output */ \
    _( BARA,      MODE1('@'), 1, "@", ESC(o)"*""\xE" ) \
    _( CIRCR,     MODE1('#'), 1, "#", ESC(o)".""\xE" ) \
    _( MU,        MODE1('$'), 1, "$", ESC(o)"5""\xE" ) \
    _( COLONBAR,  MODE1('+'), 1, "=", ESC(o)"w""\xE" ) \
    _( COLONBR,   0x00f7, 1, "+", "\xC3\xb7" ) \
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
    _( ONE1,            '1',  1, "1", "1" ) /* fallback: digits in alt mode */ \
    _( TWO1,            '2',  1, "2", "2" ) \
    _( THREE1,          '3',  1, "3", "3" ) \
    _( FOUR1,           '4',  1, "4", "4" ) \
    _( FIVE1,           '5',  1, "5", "5" ) \
    _( SIX1,            '6',  1, "6", "6" ) \
    _( SEVEN1,          '7',  1, "7", "7" ) \
    _( EIGHT1,          '8',  1, "8", "8" ) \
    _( NINE1,           '9',  1, "9", "9" ) \
    _( ZERO1,           '0',  1, "0", "0" ) \
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
    _( NULLCHAR, 0, 0, "", "" )
#define ALPHATAB_ENT(a,...) {__VA_ARGS__},
struct alpha{
    int base;int ext;char*input;char*output;
}alphatab[]={ALPHATAB(ALPHATAB_ENT)};
#define ALPHATAB_NAME(a,...) ALPHA_ ## a ,
enum alphaname { ALPHATAB(ALPHATAB_NAME) };
    /* NB. ALPHA_NAME!=alphatab[ALPHA_NAME].base */
