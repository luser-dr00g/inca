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
struct a nullob = {.r=1};
A null = &nullob;

#define P printf
#define R return
#define V1(f) A f(A w)
#define V2(f) A f(A a, A w)
#define DO(n,x) {I i=0,_n=(n);for(;i<_n;++i){x;}}
#define ESC(x) "\x1B" #x
#define CTL(x) (x-64)
#define EOT 004
#define DEL 127

#define MODE1(x) (x|1<<7)
#define MODE2(x) (x-32)

#define ALPHATAB(_) \
    _( ONE,             '1',  0, "1", "1" ) \
    _( TWO,             '2',  0, "2", "2" ) \
    _( THREE,           '3',  0, "3", "3" ) \
    _( FOUR,            '4',  0, "4", "4" ) \
    _( FIVE,            '5',  0, "5", "5" ) \
    _( SIX,             '6',  0, "6", "6" ) \
    _( SEVEN,           '7',  0, "7", "7" ) \
    _( EIGHT,           '8',  0, "8", "8" ) \
    _( NINE,            '9',  0, "9", "9" ) \
    _( ZERO,            '0',  0, "0", "0" ) \
    _( ONE1,            '1',  1, "1", "1" ) \
    _( TWO1,            '2',  1, "2", "2" ) \
    _( THREE1,          '3',  1, "3", "3" ) \
    _( FOUR1,           '4',  1, "4", "4" ) \
    _( FIVE1,           '5',  1, "5", "5" ) \
    _( SIX1,            '6',  1, "6", "6" ) \
    _( SEVEN1,          '7',  1, "7", "7" ) \
    _( EIGHT1,          '8',  1, "8", "8" ) \
    _( NINE1,           '9',  1, "9", "9" ) \
    _( ZERO1,           '0',  1, "0", "0" ) \
    _( PLUS,            '+',  0, "+", "+" ) \
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
    _( a, 'a', 0, "a", "a" ) \
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
    _( PLUSMINUS, MODE1('g'), 1, "g", ESC(n)"g""\xE" ) \
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
    _( LF,        MODE1('e'), 1, "e", ESC(n)"e""\xE" ) \
    _( JUNCF,     MODE1('t'), 1, "t", ESC(n)"t""\xE" ) \
    _( LESSEQUAL, MODE1('y'), 1, "y", ESC(n)"y""\xE" ) \
    _( THREEJUNC, MODE1('u'), 1, "u", ESC(n)"u""\xE" ) \
    _( VT,        MODE1('i'), 1, "i", ESC(n)"i""\xE" ) \
    _( GRAYBOX,   MODE1('a'), 1, "a", ESC(n)"a""\xE" ) \
    _( CR,        MODE1('d'), 1, "d", ESC(n)"d""\xE" ) \
    _( DEGREE,    MODE1('f'), 1, "f", ESC(n)"f""\xE" ) \
    _( NL,        MODE1('h'), 1, "h", ESC(n)"h""\xE" ) \
    _( JUNCJ,     MODE1('j'), 1, "j", ESC(n)"j""\xE" ) \
    _( JUNCK,     MODE1('k'), 1, "k", ESC(n)"k""\xE" ) \
    _( JUNCR,     MODE1('l'), 1, "l", ESC(n)"l""\xE" ) \
    _( MOREEQUAL, MODE1('z'), 1, "z", ESC(n)"z""\xE" ) \
    _( VBAR,      MODE1('x'), 1, "x", ESC(n)"x""\xE" ) \
    _( FF,        MODE1('c'), 1, "c", ESC(n)"c""\xE" ) \
    _( JUNCW,     MODE1('v'), 1, "v", ESC(n)"v""\xE" ) \
    _( HT,        MODE1('b'), 1, "b", ESC(n)"b""\xE" ) \
    _( JUNCT,     MODE1('n'), 1, "n", ESC(n)"n""\xE" ) \
    _( JUNCL,     MODE1('m'), 1, "m", ESC(n)"m""\xE" ) \
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
    fputs(ESC(*0),stdout);
    fputs(ESC(n)
            "lqqqqqk\n"
            "x"
      ESC(o)"a box"ESC(n)
                  "x\n"
            "mqqqqqj\n"
            ESC(o)"\n", stdout);
#endif

    fputs("\x1B*0\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    //fputs("\x1B*1\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n"); //these 2 are not interesting
    //fputs("\x1B*2\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n"); //
    fputs("\x1B*A\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    fputs("\x1B*B\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    fputs(ESC(*0)ESC(n),stdout);
    fputs( "~!@#$%^&*()_+" "\n" "`1234567890-="  "\n"
           "QWERTYUIOP{}|" "\n" "qwertyuiop[]\\" "\n"
           "ASDFGHJKL:\""  "\n" "asdfghjkl;'"    "\n"
           "ZXCVBNM<>?"    "\n" "zxcvbnm,./"     "\n" , stdout);
        fputs(ESC(o),stdout);

    fputs(ESC()")B",stdout); //set G2 charset to B : usascii
    fputs(ESC(*0),stdout); //set G2 charset to 0 : special char and line drawing set ESC(n)
    fputs(ESC(+A),stdout); //set G3 charset to A : "uk" accented and special chars ESC(o)
    fputc(CTL('N'),stdout);

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

C * getln(C **s, int *len){
    int mode = 0;
    C *p;
    if (!*s) *s = malloc(*len=256);
    p = *s;
    while(1){
        int c;
        c = fgetc(stdin);
        switch(c){
        case EOF:
        case EOT: if (p==*s) goto err;
                  break;
        case '\n':
                  fputc('\n',stdout);
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
                 //if (mode) fputs(ESC(n),stdout), fputc(c,stdout), fputs((char[]){CTL('N'),0},stdout); else fputc(c,stdout);
                 c = inputtobase(c,mode);
                 *p++ = c;
                 fputs(basetooutput(c),stdout);
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
V1(neg){
    I n=tr(AR(w),AD(w));
    A z=ga(AT(w),AR(w),AD(w));
    mv(AV(z),AV(w),n);
    DO(n,AV(z)[i]=-AV(z)[i])
    R z;}
V1(size){A z=ga(0,0,0);*AV(z)=AR(w)?*AD(w):1;R z;}
V2(plusminus){
    w=cat(w,neg(w));
    a=cat(a,a);
    R plus(a,w);
}

pi(i){P("%d ",i);}
nl(){P("\n");}
pr(w)A w;{I r=AR(w),*d=AD(w),n=tr(r,d);
    if(w==null)R 0;
    DO(r,pi(d[i]));
    nl();
    if(AT(w))
        DO(n,P("< ");pr(AV(w)[i]))
    else
        DO(n,pi(AV(w)[i]));
    nl();}

#define VERBTAB(_) \
        _( ZEROFUNC,  0,               0,    0 ) \
        _( PLUS,      ALPHA_PLUS,      id,   plus ) \
        _( PLUSMINUS, ALPHA_PLUSMINUS, neg,  plusminus ) \
        _( RBRACE,    ALPHA_RBRACE,    size, from ) \
        _( TILDE,     ALPHA_TILDE,     iota, find ) \
        _( RANG,      ALPHA_RANG,      box,  0 ) \
        _( HASH,      ALPHA_HASH,      sha,  rsh ) \
        _( COMMA,     ALPHA_COMMA,     0,    cat ) \
        _( NULLFUNC,         0,        0,    0 ) 
#define VERBTAB_ENT(a, ...) { __VA_ARGS__ },
struct {
    I c; A (*vm)(); A (*vd)();
} op[] = { VERBTAB(VERBTAB_ENT) };
#define VERBTAB_NAME(a, ...) a ,
enum { VERBTAB(VERBTAB_NAME) };

I st[26];
qp(a){R  abs(a)>='a' && abs(a)<='z';}
qv(a){R 0<=abs(a) && abs(a)<'a' && abs(a)<(sizeof op/sizeof*op);}

A ex(e)I *e;{I a=*e;
    if(!a)R null;
 if(qp(a)){
     if(e[1]=='=')
         R (A)(st[a-'a']=(I)ex(e+2));
     a= st[ a-'a'];}
 R qv(a)?(op[a].vm)(ex(e+1)):
     e[1]?(op[e[1]].vd)(a,ex(e+2)):
     (A)a;}
noun(c){A z;
    if(c<'0'||c>'9')R 0;
    z=ga(0,0,0);
    *AV(z)=c-'0';
    R (I)z;}
verb(c){I i=0;
    for(;op[++i].c;)
        if(alphatab[op[i].c].base==c)
            R i;
    R 0;}
I *wd(s)C *s;{I a,n=strlen(s),*e=ma(n+1);C c;
 DO(n,/*P("%c",s[i]);*/e[i]=(a=noun(c=s[i]))?a:(a=verb(c))?a:c);
 /*P("\n");*/
 e[n]=0;
 R e;}

main(){C *s = NULL;int n=0;
    specialtty();
    while(getln(&s,&n))
        pr(ex(wd(s)));
    restoretty();
}
