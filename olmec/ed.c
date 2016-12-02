#define _POSIX_SOURCE
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <math.h> // log2
//#include <sys/bitops.h> // ilog2


///////////////////////////////////////////////////////////////////////////////
// 
// UTF-8 <-> UCS-4 processing
//
///////////////////////////////////////////////////////////////////////////////

// type to contain 1 utf-8 "character" up to 4 bytes
// if b[4] is 0, then b is a string
//
typedef struct {
    int n;
    unsigned char b[5];
} utfcp;
uint32_t to_ucs4(utfcp c);
utfcp to_utf8(uint32_t u);

// Unicode-defined replacement for miscoded chars
#define REPLACEMENT 0xFFFD

/* number of leading zeros of byte-sized value */
static int leading0s(uint_least32_t x){ return 7 - (x? floor(log2(x)): -1); }

/* number of leading ones of byte-sized value */
#define leading1s(x) leading0s(0xFF^(x))

// rather than signal an error,
// we pass this through to allow for a special encoding
uint32_t expand_shortcut(unsigned char b){
    return b;
}

uint32_t to_ucs4(utfcp c){
    int prefix = leading1s(c.b[0]);
    int n = prefix? prefix: 1;
    uint32_t u;
    //printf("prefix:%d\n",n);
    //if (n != c.n)
    switch(prefix){
    case 0: u = c.b[0]; break;
    case 1: return u = expand_shortcut(c.b[0]);
    case 2: u = c.b[0] & 0x1f; break;
    case 3: u = c.b[0] & 0x0f; break;
    case 4: u = c.b[0] & 0x07; break;
    }
    //printf("%04x\n", u);
    for(int i=1; i<n; ++i){
        u = (u << 6) | (c.b[i] & 0x3f);
        //printf("%04x\n", u);
    }

    if (u < ((int[]){0,0,0x80,0x800,0x10000})[prefix]) {
        //error |= over_length_encoding;
        u=REPLACEMENT;
    }
    return u;
}

utfcp to_utf8(uint32_t u){
    if (u<0x20) return (utfcp){2, '^', u+'@'}; // sanitize control codes
    if (u<0x80) return (utfcp){1,u};
    if (u<0x800) return (utfcp){2,0xC0|(u>>6),
		     0x80|(u&0x3f)};
    if (u<0x10000) return (utfcp){3,0xE0|(u>>12),
		       0x80|((u>>6)&0x3f),0x80|(u&0x3f)};
    if (u<0x110000) return (utfcp){4,0xF0|(u>>18),
			0x80|((u>>12)&0x3f),0x80|((u>>6)&0x3f),0x80|(u&0x3f)};
    //(else) error RANGE
    return (utfcp){0,0};
}


///////////////////////////////////////////////////////////////////////////////
//
// Terminal handling
//
///////////////////////////////////////////////////////////////////////////////


struct termios saved_settings;

void restore_terminal(void){
    tcsetattr(0, TCSANOW, &saved_settings);
}

void init_terminal(void){
    tcgetattr(0, &saved_settings);
    atexit(restore_terminal);

    struct termios raw_mode = saved_settings;

    raw_mode.c_iflag |= IGNPAR; //ignore parity errors
    raw_mode.c_iflag &=  //non-canon, no echo, no kill
        ~(IGNBRK | PARMRK | ISTRIP | ICRNL | IXON | IXANY | IXOFF);

    raw_mode.c_lflag &=
        ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON);

    raw_mode.c_cflag &= ~(CSIZE | PARENB);
    raw_mode.c_cflag |= CS8;

    raw_mode.c_oflag |= OPOST;  //special output processing

    raw_mode.c_cc[VMIN] = 4;  //min chars to read
    raw_mode.c_cc[VTIME] = 1;  //timeout

    if (tcsetattr(0, TCSANOW, &raw_mode) == -1)
        perror("init_terminal");
}

typedef struct {
    unsigned unicode;
    utfcp bytes;
} character;

// read up to 4 bytes from keyboard/stdin
// and attempt to decode it as a utf-8 encoding
//
character read_character(void){
    int len;
    char buf[5];
    do {
        memset(buf, 0, sizeof buf);
        len = read(fileno(stdin), buf, 4);
    } while(len == -1 && errno == EAGAIN);
    //printf("%d:", len);
    //for (int i=0; i<len; ++i) printf(" %02x", (unsigned)(unsigned char)buf[i]);
    //puts("");

    utfcp u = {len, buf[0], buf[1], buf[2], buf[3], buf[4]};
    return (character){ len==0 ? EOF : to_ucs4(u), u };
}



///////////////////////////////////////////////////////////////////////////////
//
// The Line Editor
//
///////////////////////////////////////////////////////////////////////////////


typedef struct editor {
    unsigned *bufp;
    int n;
    unsigned *p;
    int mode;
} editor;

typedef unsigned Decoder(editor*, character);


void print(character c){
    if (c.bytes.n==1)
        putchar(c.bytes.b[0]);
    else
        printf("%*s", c.bytes.n, c.bytes.b);
    fflush(stdout);
}

void printbytes(character c){
    printf("%d:",c.bytes.n);
    for (int i=0; i<c.bytes.n; ++i) printf("%04x ", c.bytes.b[i]);
    fflush(stdout);
}

void store(editor *ed, character c){
    *ed->p++ = c.unicode;
}



///////////////////////////////////////////////////////////////////////////////
//
// Key Handlers (Decoders)
//
///////////////////////////////////////////////////////////////////////////////


unsigned ignore(editor *ed, character c){
    return 0;
}

unsigned eot(editor *ed, character c){
    //printf("EOT\n");
    print(c);
    character eod = { .unicode = 0x4, .bytes = { 1, 0x4 }};
    store(ed, eod);
    return EOF;
}

unsigned bell(editor *ed, character c){
    printf("ding!\n");
    return c.unicode;
}

unsigned backspace(editor *ed, character c){
    if (ed->p > ed->bufp){
        printf("\b \b"), fflush(stdout);
        ed->p--;
    }
    return c.unicode;
}

unsigned tab(editor *ed, character c){
    return c.unicode;
}

unsigned linefeed(editor *ed, character c){
    printf("linefeed\n");
    return c.unicode;
}

unsigned vtab(editor *ed, character c){
    return c.unicode;
}

unsigned formfeed(editor *ed, character c){
    return c.unicode;
}

unsigned carriage(editor *ed, character c){
    //printf("carriage\n");
    character nl = { .unicode = '\n', .bytes = { 1, '\n' }};
    print(nl);
    store(ed, nl);
    return '\n';
}

unsigned shiftout(editor *ed, character c){
    return c.unicode;
}

unsigned shiftin(editor *ed, character c){
    return c.unicode;
}

unsigned nak(editor *ed, character c){
    return c.unicode;
}

//
// The special APL keys accessed with ALT- or ESC+
//
unsigned apl_alphabet[96] = {
    //SP      !       "       #        $       %       &    '
    //    IBEAM DELTILD DELTASTIL DELSTIL CIRCSTIL CIRCBAR 
    ' ', 0x2336, 0x236b, 0x234b,  0x2352, 0x233d, 0x2296, '\'',

    //  (        )       *       +       ,     -       .     /
    //NOR     NAND CIRCSTAR DOMINO COMMABAR TIMES   ERGO SLASHBAR
    0x2371, 0x2372, 0x235f, 0x2339, 0x236a, 0xd7, 0x2235, 0x233f,

    //   0      1       2    3       4    5       6   7   
    // AND DIAERESIS MACRON      LT|EQ        GT|EQ
    0x2227,  0xa8,   0xaf, '<', 0x2264, '=', 0x2265, '>',

    //   8       9   :    ;      <     =     >     ?
    //NOTEQ     OR              << DIVIDES  >> PILCROW
    0x2260, 0x2228, ':', ';', 0xab, 0xf7, 0xbb, 0xb6,

    //   @       A       B       C   D        E       F   G
    //DELTIL _ALPHA_  EXEC    LAMP        _EPS_    SAME  DELTASTIL
    0x236b, 0x2376, 0x234e, 0x235d, 'D', 0x2377, 0x2261, 0x234b,

    //   H       I       J   K        L   M        N       O
    //DELSTL   _I_  DIAJOT        'QUAD       FORMAT DIACIRC
    0x2352, 0x2378, 0x2364, 'K', 0x235e, 'M', 0x2355, 0x2365,

    // P      Q       R       S       T       U       V      W
    //POUND inv?   REAL  SQUISH  TILSTL    NULL     PHI _OMEGA_
    0xa3,  0xbf, 0x211d, 0x2337, 0x236d, 0x2300, 0x2366, 0x2379,

    //X     Y       Z       [       \       ]   ^       _
    //    YEN  SUBSTIL     <- BACKBAR      ->  BACKCIRC
    'X', 0xa5, 0x2367, 0x2190, 0x2340, 0x2192, 0x2349, '_',

    //   `       a       b       c       d       e   f       g
    //DIAMOND ALPHA   BASE     CAP   FLOOR EPSILON        NABLA
    0x22c4, 0x237a, 0x22a5, 0x2229, 0x230a, 0x2208, 'f', 0x2207,

    //   h       i       j   k        l   m        n       o
    //INCR    IOTA     JOT         QUAD       ENCODE    CIRC
    0x2206, 0x2373, 0x2218, 'k', 0x2395, 'm', 0x22a4, 0x25cb,

    //   p   q        r       s   t        u       v     w
    //STAR          RHO    CEIL         DOWN     CUP OMEGA
    0x22c6, '?', 0x2374, 0x2308, '~', 0x2193, 0x222a, 0x2375,

    //   x       y       z       {   |        }   ~   DEL
    //SUPER     UP     SUB    LEFT        RIGHT
    0x2283, 0x2191, 0x2282, 0x22a3, '|', 0x22a2, '~', 0
};

unsigned alpha(editor *ed, character c){
    c.unicode = apl_alphabet[c.bytes.b[1] - ' '];
    c.bytes = to_utf8(c.unicode);
    print(c);
    store(ed, c);
    return c.unicode;
}

Decoder *metatable[256] = {
    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,
    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,
    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,
    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,

    alpha, alpha, alpha, alpha, alpha, alpha, alpha, alpha,
    alpha, alpha, alpha, alpha, alpha, alpha, alpha, alpha,
    alpha, alpha, alpha, alpha, alpha, alpha, alpha, alpha,
    alpha, alpha, alpha, alpha, alpha, alpha, alpha, alpha,

    alpha, alpha, alpha, alpha, alpha, alpha, alpha, alpha,
    alpha, alpha, alpha, alpha, alpha, alpha, alpha, alpha,
    alpha, alpha, alpha, alpha, alpha, alpha, alpha, alpha,
    alpha, alpha, alpha, alpha, alpha, alpha, alpha, alpha,

    alpha, alpha, alpha, alpha, alpha, alpha, alpha, alpha,
    alpha, alpha, alpha, alpha, alpha, alpha, alpha, alpha,
    alpha, alpha, alpha, alpha, alpha, alpha, alpha, alpha,
    alpha, alpha, alpha, alpha, alpha, alpha, alpha, alpha,

    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,
    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,
    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,
    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,

    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,
    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,
    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,
    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,

    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,
    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,
    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,
    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,

    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,
    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,
    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,
    ignore, ignore, ignore, ignore, ignore, ignore, ignore, ignore,

};

unsigned escape(editor *ed, character c){
    //printbytes(c);
    switch(c.bytes.n){
        case 1: ed->mode = 1 - ed->mode; break;
        case 2: ed->mode = 0;
                return metatable[c.bytes.b[1]](ed, c);
        case 3: ed->mode = 0;
                // TODO
    }
    return 0;
}

Decoder *controltable[32] = {
    //^@    ^A      ^B      ^C      ^D   ^E      ^F      ^G
    ignore, ignore, ignore, ignore, eot, ignore, ignore, bell,
    //^H       ^I   ^J        ^K    ^L        ^M        ^N        ^O
    backspace, tab, linefeed, vtab, formfeed, carriage, shiftout, shiftin,
    //^P    ^Q      ^R      ^S      ^T      ^U   ^V      ^W
    ignore, ignore, ignore, ignore, ignore, nak, ignore, ignore,
    //^X    ^Y      ^Z      ^[      ^\      ^]      ^^      ^_
    ignore, ignore, ignore, escape, ignore, ignore, ignore, ignore,
};

unsigned control(editor *ed, character c){
    //printf("control character\n");
    //c.bytes = (utfcp){ 2, '^', c.unicode + '@', 0, 0 };
    return controltable[c.bytes.b[0]](ed, c);
}

unsigned ascii(editor *ed, character c){
    if (ed->mode){
        c.bytes.n = 2;
        c.bytes.b[1] = c.bytes.b[0];
        c.bytes.b[0] = 27;
        return escape(ed, c);
    }
    print(c);
    store(ed, c);
    return c.unicode;
}

unsigned extended(editor *ed, character c){
    return 0;
}

unsigned unicode2(editor *ed, character c){
    print(c);
    store(ed, c);
    return c.unicode;
}

unsigned unicode3(editor *ed, character c){
    print(c);
    store(ed, c);
    return c.unicode;
}

unsigned unicode4(editor *ed, character c){
    print(c);
    store(ed, c);
    return c.unicode;
}

Decoder *chartable[256] = {
control, control, control, control, control, control, control, control, 
control, control, control, control, control, control, control, control, 
control, control, control, control, control, control, control, control, 
control, control, control, control, control, control, control, control, 

ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii,
ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii,
ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii,
ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii,

ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii,
ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii,
ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii,
ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii,

ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii,
ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii,
ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii,
ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii,

extended, extended, extended, extended, extended, extended, extended, extended,
extended, extended, extended, extended, extended, extended, extended, extended,
extended, extended, extended, extended, extended, extended, extended, extended,
extended, extended, extended, extended, extended, extended, extended, extended,

extended, extended, extended, extended, extended, extended, extended, extended,
extended, extended, extended, extended, extended, extended, extended, extended,
extended, extended, extended, extended, extended, extended, extended, extended,
extended, extended, extended, extended, extended, extended, extended, extended,

unicode2, unicode2, unicode2, unicode2, unicode2, unicode2, unicode2, unicode2,
unicode2, unicode2, unicode2, unicode2, unicode2, unicode2, unicode2, unicode2,
unicode2, unicode2, unicode2, unicode2, unicode2, unicode2, unicode2, unicode2,
unicode2, unicode2, unicode2, unicode2, unicode2, unicode2, unicode2, unicode2,

unicode3, unicode3, unicode3, unicode3, unicode3, unicode3, unicode3, unicode3,
unicode3, unicode3, unicode3, unicode3, unicode3, unicode3, unicode3, unicode3,

unicode4, unicode4, unicode4, unicode4, unicode4, unicode4, unicode4, unicode4,
unicode4, unicode4, unicode4, unicode4, unicode4, unicode4, unicode4, unicode4,
};


unsigned *read_line(char *prompt, unsigned **bufp, int *lenp){
    if (prompt) fputs(prompt, stdout), fflush(stdout);
    if (!*bufp) *bufp = malloc( (sizeof**bufp) * (*lenp = 256));
    unsigned *p = *bufp;

    character c;
    utfcp u;
    editor ed = { .bufp = p, .n = *lenp, .p = p, .mode = 0 };
    unsigned x;
    do {
        c = read_character();
        //printf("U%04x\n", c.unicode);
        //printf("%*s", u.n, u.b);
        x = chartable[c.bytes.b[0]](&ed, c);
        u = to_utf8(x);
        //printf("U%04x\n", x);
        //if (x) printf("%*s", u.n, u.b), fflush(stdout);
        //if (x) *p++ = x;
    } while (x != (unsigned)'\n' && x != (unsigned)EOF);
    *bufp = ed.bufp;
    *lenp = ed.n;
    p = ed.p;

    if (p[-1] == EOF) p[-1] = '\n';
    if (p == (*bufp+1) && x == EOF){
        return NULL;
    }
    return *bufp;
}


///////////////////////////////////////////////////////////////////////////////
//
// main()
//
///////////////////////////////////////////////////////////////////////////////

int main(void){
    init_terminal();

    //printf("%u\n", (unsigned)'\n');

    char *prompt = "> ";
    unsigned *buf = NULL;
    int len;
    while (read_line(prompt, &buf, &len)){
        for (int i = 0; buf[i]!='\n'; ++i)
            printf("%04x ", buf[i]);
        puts("");
    }

    return 0;
}

