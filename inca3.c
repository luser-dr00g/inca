#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<termios.h>
#include<unistd.h>

typedef char C;
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

#define MODE1(x) (x|1<<7)
#define ALPHATAB(_) \
    _( PLUS,            '+',  0, "+", "+" ) \
    _( PLUSMINUS, MODE1('g'), 1, "g", ESC(n)"g"ESC(o) ) \
    _( NULLCHAR, 0, 0, 0, 0 )
#define ALPHATAB_ENT(a,...) {__VA_ARGS__},
struct alpha {
    int base; int ext; char *input; char *output;
} alphatable[] = { ALPHATAB(ALPHATAB_ENT) };
#define ALPHATAB_NAME(a,...) ALPHA_ ## a ,
enum alphaname { ALPHATAB(ALPHATAB_NAME) };

int inputtobase (int c, int mode){
    return mode? MODE1(c): c;
    //return c | mode << 7;
}
char *basetooutput (int c, int mode){
    int i;
    for (i=0;i<(sizeof alphatable/sizeof*alphatable);i++)
        if (c==alphatable[i].base && mode==alphatable[i].ext)
            return alphatable[i].output;
    return "";
}

struct termios tm;
void specialtty(){ tcgetattr(0,&tm);
//https://web.archive.org/web/20060117034503/http://www.cs.utk.edu/~shuford/terminal/xterm_codes_news.txt
    //fputs("\x1B""*0\n",stdout);
#if 0
    fputs(ESC(*0\n),stdout);
    fputs(ESC(n)
            "lqqqqqk\n"
            "x"
      ESC(o)"a box"ESC(n)
                  "x\n"
            "mqqqqqj\n"
            ESC(o)"\n", stdout);
#endif
    //fputs("\x1B*0\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    //fputs("\x1B*1\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n"); //these 2 are not interesting
    //fputs("\x1B*2\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n"); //
    //fputs("\x1B*A\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    //fputs("\x1B*B\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    fputs(ESC(*0)ESC(n),stdout);
    fputs( "~!@#$%^&*()_+" "\n" "`1234567890-="  "\n"
           "QWERTYUIOP{}|" "\n" "qwertyuiop[]\\" "\n"
           "ASDFGHJKL:\""  "\n" "asdfghjkl;'"    "\n"
           "ZXCVBNM<>?"    "\n" "zxcvbnm,./"     "\n" , stdout);
        fputs(ESC(o),stdout);
    fputs(ESC(*0),stdout);

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

#define CTL(x) (x-64)
#define EOT 004
#define DEL 127
char * getln(char **s, int *len){
    int mode = 0;
    char *p;
    if (!*s) *s = malloc(*len=256);
    p = *s;
    while(1){
        int c;
        c = fgetc(stdin);
        switch(c){
        case EOF:
        case EOT: *p = 0; goto err;
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
                 if (mode)
                     fputs(ESC(n),stdout),
                         fputc(c,stdout),
                         fputs(ESC(o),stdout);
                 else
                     fputc(c,stdout);
                 *p++ = inputtobase(c,mode);
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

#define VERBTABLE(_) \
        _( ZEROFUNC,         0,  0,    0 ) \
        _( PLUS,            '+', id,   plus ) \
        _( PLUSMINUS, MODE1('g'), neg, plusminus ) \
        _( RBRACE,          '{', size, from ) \
        _( TILDE,           '~', iota, find ) \
        _( RANG,            '<', box,  0 ) \
        _( HASH,            '#', sha,  rsh ) \
        _( COMMA,           ',', 0,    cat ) \
        _( NULLFUNC,         0,  0,    0 ) 
#define VERBTABLE_ENT(a, ...) { __VA_ARGS__ },
struct {
    C c; A (*vm)(); A (*vd)();
} op[] = { VERBTABLE(VERBTABLE_ENT) };

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
        if(op[i].c==c)
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
