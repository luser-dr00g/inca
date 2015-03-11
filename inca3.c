#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<termios.h>
#include<unistd.h>

typedef unsigned char C;
typedef intptr_t I;
typedef struct a{I t,
    r,d[3],
    p[2];}*A;
#define AT(a) (a->t)
#define AR(a) (a->r)
#define AD(a) (a->d)
#define AV(a) (a->p)
enum type {
    INT, BOX, SYMB, CHAR, DBL,
};
struct a nullob = {.r=1};
A null = &nullob;

#define P printf
#define R return
#define V1(f) A f(A w)
#define V2(f) A f(A a, A w)
#define DO(n,x) {I i=0,_n=(n);for(;i<_n;++i){x;}}
#define ESC(x) "\x1b" #x
#define ESCCHR '\x1b'
#define CTL(x) (x-64)
#define EOT 004
#define DEL 127

#define MODE1(x) (x|1<<7)
#define MODE2(x) (x-32)

/* ALPHA_NAME base ext input output */
#define ALPHATAB(_) \
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
    _( DOT,       MODE1('~'), 1, "~", ESC(n)"~""\xE" ) \
    _( DIAMOND,   MODE1('`'), 1, "`", ESC(n)"`""\xE" ) \
    _( PI,        MODE1('{'), 1, "{", ESC(n)"{""\xE" ) \
    _( POUND,     MODE1('}'), 1, "}", ESC(n)"}""\xE" ) \
    _( NOTEQUAL,  MODE1('|'), 1, "|", ESC(n)"|""\xE" ) \
    _( HBAR0,     MODE1('o'), 1, "o", ESC(n)"o""\xE" ) \
    _( HBAR1,     MODE1('p'), 1, "p", ESC(n)"p""\xE" ) \
    _( HBAR3,     MODE1('q'), 1, "q", ESC(n)"q""\xE" ) \
    _( HBAR4,     MODE1('r'), 1, "r", ESC(n)"r""\xE" ) \
    _( HBAR5,     MODE1('s'), 1, "s", ESC(n)"s""\xE" ) \
    _( JUNCM,     MODE1('w'), 1, "w", ESC(n)"w""\xE" ) \
    _( JUNCF,     MODE1('t'), 1, "t", ESC(n)"t""\xE" ) \
    _( LESSEQUAL, MODE1('y'), 1, "y", ESC(n)"y""\xE" ) \
    _( THREEJUNC, MODE1('u'), 1, "u", ESC(n)"u""\xE" ) \
    _( GRAYBOX,   MODE1('a'), 1, "a", ESC(n)"a""\xE" ) \
    _( DEGREE,    MODE1('f'), 1, "f", ESC(n)"f""\xE" ) \
    _( HT,        '\x9', 0, "\t", ESC(n)"b""\xE" ) \
    _( NL,        '\xa', 0, "\n", ESC(n)"h""\xE" ) \
    _( LF,        '\xa', 0, "\n", ESC(n)"e""\xE" ) \
    _( VT,        '\xb', 0, "\v", ESC(n)"i""\xE" ) \
    _( FF,        '\xc', 0, "\f", ESC(n)"c""\xE" ) \
    _( CR,        '\xd', 0, "\r", ESC(n)"d""\xE" ) \
    _( JUNCJ,     MODE1('j'), 1, "j", ESC(n)"j""\xE" ) \
    _( JUNCK,     MODE1('k'), 1, "k", ESC(n)"k""\xE" ) \
    _( JUNCR,     MODE1('l'), 1, "l", ESC(n)"l""\xE" ) \
    _( MOREEQUAL, MODE1('z'), 1, "z", ESC(n)"z""\xE" ) \
    _( VBAR,      MODE1('x'), 1, "x", ESC(n)"x""\xE" ) \
    _( JUNCW,     MODE1('v'), 1, "v", ESC(n)"v""\xE" ) \
    _( JUNCT,     MODE1('n'), 1, "n", ESC(n)"n""\xE" ) \
    _( JUNCL,     MODE1('m'), 1, "m", ESC(n)"m""\xE" ) \
    /* ALPHA_NAME base       ext input output */ \
    _( INVEXCL,   MODE1('!'), 1, "!", ESC(o)"!""\xE" ) /* "uk" chars patch */ \
    _( GUILLEFT,  MODE1('<'), 1, "<", ESC(o)"+""\xE" ) \
    _( GUILRIGHT, MODE1('>'), 1, ">", ESC(o)";""\xE" ) \
    _( COMPL,     MODE1('^'), 1, "^", ESC(o)",""\xE" ) \
    _( HIMINUS,   MODE1('_'), 1, "_", ESC(o)"/""\xE" ) \
    _( TIMES,     MODE1('*'), 1, "*", ESC(o)"W""\xE" ) \
    _( DIVIDE,    MODE1('/'), 1, "/", ESC(o)"w""\xE" ) \
    _( INVQUEST,  MODE1('?'), 1, "?", ESC(o)"?""\xE" ) \
    _( CDOT,      MODE1('.'), 1, ".", ESC(o)"7""\xE" ) \
    _( HYPHEN,    MODE1('-'), 1, "-", ESC(o)"-""\xE" ) \
    _( CENT,      MODE1('e'), 1, "e", ESC(o)"\"""\xE" ) \
    _( BUTTON,    MODE1('i'), 1, "i", ESC(o)"$""\xE" ) \
    _( YEN,       MODE1('d'), 1, "d", ESC(o)"%""\xE" ) \
    _( SECTION,   MODE1('h'), 1, "h", ESC(o)"'""\xE" ) \
    _( PRIME,     MODE1('\''), 1, "'", ESC(o)"4""\xE" ) \
    _( TWODOTS,   MODE1('"'), 1, "\"", ESC(o)"(""\xE" ) \
    _( CIRCC,     MODE1('c'), 1, "c", ESC(o)")""\xE" ) \
    _( ZEROSLASH, MODE1('b'), 1, "b", ESC(o)"X""\xE" ) \
    _( OBAR,      MODE1(';'), 1, ";", ESC(o)":""\xE" ) \
    /* ALPHA_NAME base       ext input output */ \
    _( EQSLASH,   MODE1('='), 1, "=", ESC(n)"|""\xE" ) \
    _( PARAGRAPH, MODE1(','), 1, ",", ESC(o)"6""\xE" ) \
    _( BARA,      MODE1('@'), 1, "@", ESC(o)"*""\xE" ) \
    _( CIRCR,     MODE1('#'), 1, "#", ESC(o)".""\xE" ) \
    _( MU,        MODE1('$'), 1, "$", ESC(o)"5""\xE" ) \
    _( COLONBAR,  MODE1('%'), 1, "%", ESC(o)"w""\xE" ) \
    _( DEL,       MODE1('&'), 1, "&", ESC(o)"P""\xE" ) \
    _( SUPONE,    MODE1('('), 1, "(", ESC(o)"9""\xE" ) \
    _( SUPTWO,    MODE1(')'), 1, ")", ESC(o)"2""\xE" ) \
    _( SUPTHREE,  MODE1('+'), 1, "+", ESC(o)"1""\xE" ) \
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
struct alpha {
    int base; int ext; char *input; char *output;
} alphatab[] = { ALPHATAB(ALPHATAB_ENT) };
#define ALPHATAB_NAME(a,...) ALPHA_ ## a ,
enum alphaname { ALPHATAB(ALPHATAB_NAME) };

int inputtobase (int c, int mode){
    int i;
    for (i=0;i<(sizeof alphatab/sizeof*alphatab);i++)
        if (c==*alphatab[i].input && mode==alphatab[i].ext)
            return alphatab[i].base;
    return mode? MODE1(c): c;
    //return c | mode << 7;
}
char *basetooutput (int c){
    int i;
    for (i=0;i<(sizeof alphatab/sizeof*alphatab);i++)
        if (c==alphatab[i].base)
            return alphatab[i].output;
    return "";
}

struct termios tm;
void specialtty(){ tcgetattr(0,&tm);
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
    {
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
            for (j=0;j<n;j++)
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
void restoretty(){ tcsetattr(0,TCSANOW,&tm); }

C * getln(C *prompt, C **s, int *len){
    int mode = 0;
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
                  case '[':
                      c = fgetc(stdin);
                      switch(c){
                      case 'Z':
                          c = '\v';
                          *p++ = c;                   // save base in string
                          fputs(basetooutput(c),stdout);  // echo output form
                          break;
                      }
                      break;
                  }
                  break;
        case '\n':
                  fputc('\n',stdout);
                  *p++ = c;
                  goto breakwhile;
        case CTL('N'): mode = 1; break;
        case CTL('O'): mode = 0; break;
        case CTL('U'): 
                       while(p>*s){
                           fputs("\b \b",stdout);
                           --p;
                       }
                       break;
        case '\b':
        case DEL:
                   fputs("\b \b",stdout);
                   if (p!=*s) --p;
                   break;
        default:
                 c = inputtobase(c,mode);    // convert to internal "base" form
                 *p++ = c;                   // save base in string
                 fputs(basetooutput(c),stdout);  // echo output form
                 break;
        }
    }
breakwhile:
    *p++ = 0;
err:
    return p==*s?NULL:*s;
}

I *ma(n){R(I*)malloc(n*4);}
mv(d,s,n)I *d,*s;{DO(n,d[i]=s[i]);}
tr(r,d)I *d;{I z=1;DO(r,z=z*d[i]);R z;}
A ga(t,r,d)I *d;{A z=(A)ma(5+tr(r,d));AT(z)=t,AR(z)=r,mv(AD(z),d,r);
 R z;}

ps(A i){P("%s",(char*)AV(i));}
pc(i){P("%s",basetooutput(i));}
pi(i){P("%d ",i);}
nl(){P("\n");}
pr(A w){
    if (abs((I)w)<256)
        pc(w);
    else {
        I r=AR(w),*d=AD(w),n=tr(r,d);
        if(w==null)R 0;
        DO(r,pi(d[i]));
        nl();
        if(AT(w)==1)
            DO(n,P("< ");pr((A)(AV(w)[i])))
        else if(AT(w)==SYMB)
            ps(w);
        else
            DO(n,pi(AV(w)[i]));
        nl();
    }
}

V1(copy){I n=tr(AR(w),AD(w)); A z=ga(AT(w),AR(w),AD(w)); mv(AV(z),AV(w),n); R z;}
V1(iota){I n=*AV(w);A z=ga(0,1,&n);DO(n,AV(z)[i]=i);R z;}
V2(plus){I r=AR(w),*d=AD(w),n=tr(r,d);A z=ga(0,r,d);
 DO(n,AV(z)[i]=AV(a)[i]+AV(w)[i]);R z;}
V2(from){I r=AR(w)-1,*d=AD(w)+1,n=tr(r,d);
 A z=ga(AT(w),r,d);mv(AV(z),AV(w)+(n**AV(a)),n);R z;}
V1(box){A z=ga(1,0,0);*AV(z)=(I)w;R z;}
V2(cat){I an=tr(AR(a),AD(a)),wn=tr(AR(w),AD(w)),n=an+wn;
 A z=ga(AT(w),1,&n);mv(AV(z),AV(a),an);mv(AV(z)+an,AV(w),wn);R z;}
V2(find){}
V2(rsh){I r=AR(a)?*AD(w):1,n=tr(r,AV(a)),wn=tr(AR(w),AD(w));
 A z=ga(AT(w),r,AV(a));mv(AV(z),AV(w),wn=n>wn?wn:n);
 if(n-=wn)mv(AV(z)+wn,AV(z),n);R z;}
V1(sha){A z=ga(0,1,&AR(w));mv(AV(z),AD(w),AR(w));R z;}
V1(id){R w;}
V1(neg){ A z=copy(w); DO(tr(AR(z),AD(z)),AV(z)[i]=-AV(z)[i]) R z;}
V1(size){A z=ga(0,0,0);*AV(z)=AR(w)?*AD(w):1;R z;}
V2(plusminus){ w=cat(w,neg(w)); a=cat(a,a); R plus(a,w);}

/*         FUNCNAME   ALPHA_NAME       vm    vd        */
#define VERBTAB(_) \
        _( ZEROFUNC,  0,               0,    0         ) \
        _( PLUS,      ALPHA_PLUS,      id,   plus      ) \
        _( PLUSMINUS, ALPHA_PLUSMINUS, neg,  plusminus ) \
        _( RBRACE,    ALPHA_RBRACE,    size, from      ) \
        _( TILDE,     ALPHA_TILDE,     iota, find      ) \
        _( RANG,      ALPHA_RANG,      box,  0         ) \
        _( HASH,      ALPHA_HASH,      sha,  rsh       ) \
        _( COMMA,     ALPHA_COMMA,     0,    cat       ) \
        _( NULLFUNC,         0,        0,    0 ) 
#define VERBTAB_ENT(a, ...) { __VA_ARGS__ },
struct {
    I c; A (*vm)(); A (*vd)();
} op[] = { VERBTAB(VERBTAB_ENT) };
#define VERBTAB_NAME(a, ...) a ,
enum { VERBTAB(VERBTAB_NAME) };

struct st {
    A a;
    struct st *tab[52];
} st;

char *alph="ABCDEFGHIJKLMNOPQRSTUVWXYZ""abcdefghijklmnopqrstuvwxyz";
/*
mode 0: search trie for longest-prefix match. ret root on fail. update input string
mode 1: search and allocate. update input string
 */
struct st *findsymb(struct st *st, char **s, int mode) {
    int code;
    while(isalpha(**s)){
        code = strchr(alph,**s)-alph;
        if (st->tab[code]){
            st=st->tab[code];
            (*s)++;
        } else
            switch(mode){
            case 0: goto breakwhile; //prefix search
            case 1: //defining search
                    st->tab[code] = malloc(sizeof(struct st));
                    st=st->tab[code];
                    (*s)++;
                    break;
            }

    }
breakwhile:
    return st;
}

//qp(a){R !(a&(1<<7)) && isalpha(abs(a)&127); }
qp(A a){R AT(a)==SYMB;}
qv(a){R 0<=abs(a) && abs(a)<'a' && abs(a)<(sizeof op/sizeof*op);}
A ex(e)I *e;{I a=*e;
    int i;for(i=0;e[i];i++)pr((A)e[i]);
    if(!a)R null;
 if(qp((A)a)){
     //char *s = (char[]){a, 0};
     A sa = (A)a;
     char *s = (char*)AV(sa);
     //pr(sa);
     if(e[1]=='='){
         R (findsymb(&st, &s, 1)->a)=ex(e+2);
     }
     a=(I)(findsymb(&st, &s, 0)->a);
 }
 R qv(a)?(op[a].vm)(ex(e+1)):
     e[1]?(op[e[1]].vd)(a,ex(e+2)):
     (A)a;}

#define ALPHALOWER "abcdefghijklmnopqrstuvwxyz"
#define ALPHAUPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define DIGIT "0123456789"
#define SPACE " \n"
#define ZEROCL ""
char *cclass[] = {0, ALPHAUPPER ALPHALOWER, DIGIT, SPACE};

verb(c){I i=0;
    for(;op[++i].c;)
        if(alphatab[op[i].c].base==c)
            R i;
    R 0;}

A newsymb(C *s,I n){
    I t;
    //P("%d\n",n);DO(n,P("%c",s[i]))P("\n");
    if(strchr(DIGIT,*s)) {
        A z=ga(INT,0,0);
        *AV(z)=strtol(s,NULL,10);
        R z;
    } else if(strchr(ALPHAUPPER ALPHALOWER,*s)) {
        A z=ga(SYMB,1,(I[]){n+1});
        mv(AV(z),s,n+3/4);
        ((C*)AV(z))[n] = 0;
        R z;
    } else {
        I c=verb(*s);
        R (A)(c?c:(I)*s);
    }
}

int wdtab[][4] = {
    /*0     a     d     s*/
    { 30+2, 20+2, 10+2, 0+0 }, /* init */
    { 30+1, 20+1, 10+0, 0+1 }, /* number */
    { 30+1, 20+0, 10+1, 0+1 }, /* name */
    { 30+1, 20+1, 10+1, 0+1 }, /* other */
};

#define emit(a,b) (*z++=(I)newsymb(s+a,(b)-a)); 
I *wd(C *s){
    I a,b,n=strlen(s),*e=ma(n+1),*z=e;
    int i,j,i_,state;
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
        state=b/10;
        switch(b%10){ //encoded actions
        case 0: break;
        case 1: emit(j,i);
        case 2: j=i; break;
        }
    }
    *z++=0;
    R e;}

#if 0
noun(c){A z;
    if(c<'0'||c>'9')R 0;
    z=ga(0,0,0);
    *AV(z)=c-'0';
    R (I)z;}

I *wd(s)C *s;{I a,n=strlen(s),*e=ma(n+1);C c;
    DO(n,e[i]=
         (a=noun(c=s[i]))?
         a:
         (a=verb(c))?
             a:
             c)
    e[n]=0;
    R e;}
#endif

main(){C *s=NULL;int n=0;C *prompt="\t";
    specialtty();
    while(getln(prompt,&s,&n))
        pr(ex(wd(s)));
    restoretty();
}
