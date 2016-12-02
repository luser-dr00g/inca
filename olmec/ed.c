#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <math.h> // log2
//#include <sys/bitops.h> // ilog2

// type to contain 1 utf-8 "character" up to 4 bytes

typedef struct {
    int n;
    unsigned char b[4];
} utfcp;
uint32_t to_ucs4(utfcp c);
utfcp to_utf8(uint32_t u);

#define REPLACEMENT 0xFFFD

/* number of leading zeros of byte-sized value */
static int leading0s(uint_least32_t x){ return 7 - (x? floor(log2(x)): -1); }

/* number of leading ones of byte-sized value */
#define leading1s(x) leading0s(0xFF^(x))

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
    if (u<0x20) return (utfcp){2, '^', u+'@'};
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
    char buf[4];
    do {
        memset(buf, 0, sizeof buf);
        len = read(fileno(stdin), buf, 4);
    } while(len == -1 && errno == EAGAIN);
    //printf("%d:", len);
    //for (int i=0; i<len; ++i) printf(" %02x", (unsigned)(unsigned char)buf[i]);
    //puts("");

    utfcp u = {len, buf[0], buf[1], buf[2], buf[3]};
    return (character){ len==0 ? EOF : to_ucs4(u), u };
}


typedef struct editor {
    unsigned *bufp;
    int n;
    unsigned *p;
} editor;

void print(editor *ed, character c){
    if (c.bytes.n==1)
        putchar(c.bytes.b[0]);
    else
        printf("%*s", c.bytes.n, c.bytes.b);
    fflush(stdout);
}

void store(editor *ed, character c){
    *ed->p++ = c.unicode;
}

typedef unsigned Decoder(editor*, character);

unsigned ignore(editor *ed, character c){
    return 0;
}

unsigned eot(editor *ed, character c){
    //printf("EOT\n");
    print(ed, c);
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
    print(ed, nl);
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

unsigned escape(editor *ed, character c){
    return c.unicode;
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
    c.bytes = (utfcp){ 2, '^', c.unicode + '@', 0, 0 };
    return controltable[c.unicode](ed, c);
}

unsigned ascii(editor *ed, character c){
    print(ed, c);
    store(ed, c);
    return c.unicode;
}

unsigned extended(editor *ed, character c){
    return 0;
}

unsigned unicode2(editor *ed, character c){
    print(ed, c);
    store(ed, c);
    return c.unicode;
}

unsigned unicode3(editor *ed, character c){
    print(ed, c);
    store(ed, c);
    return c.unicode;
}

unsigned unicode4(editor *ed, character c){
    print(ed, c);
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
    editor ed = { .bufp = p, .n = *lenp, .p = p };
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

