#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<termios.h>
#include<unistd.h>

typedef char C;
typedef long I;
typedef struct a{I t,
    r,d[3],
    p[2];}*A;

#define P printf
#define R return
#define V1(f) A f(A w)
#define V2(f) A f(A a, A w)
#define DO(n,x) {I i=0,_n=(n);for(;i<_n;++i){x;}}
#define ESC(x) "\x1B" #x

struct termios tm;
void specialtty(){ tcgetattr(0,&tm);
//https://web.archive.org/web/20060117034503/http://www.cs.utk.edu/~shuford/terminal/xterm_codes_news.txt
    //fputs("\x1B""*0\n",stdout);
    fputs(ESC(*0\n),stdout);
    fputs(ESC(n)
            "lqqqqqk\n"
            "x"
      ESC(o)"a box"ESC(n)
                  "x\n"
            "mqqqqqj\n"
            ESC(o)"\n", stdout);
    fputs("\x1B*0\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    fputs("\x1B*1\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    fputs("\x1B*2\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    fputs("\x1B*A\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    fputs("\x1B*B\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    fputs(ESC(*0\n),stdout);

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
char * getln(char **s){
    int mode = 0;
    char *p;
    if (!*s) *s = malloc(256);
    p = *s;
    while(1){
        int c;
        c = fgetc(stdin);
        switch(c){
        case EOF: goto err;
        case EOT: goto err;
        case '\n': goto breakwhile;
        case CTL('N'): mode = 1;
                       //fputc('N',stdout);
                       break;
        case CTL('O'): mode = 0;
                       //fputc('O',stdout);
                       break;
        case CTL('U'): 
                       while(p>=*s){
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
                 *p++ = c;
                 break;
        }
    }
breakwhile:
    *p++ = 0;
err:
    //if (*p==004) return NULL;
    return p==*s?NULL:*s;
    //return gets(*s);
}

I *ma(n){R(I*)malloc(n*4);}
mv(d,s,n)I *d,*s;{DO(n,d[i]=s[i]);}
tr(r,d)I *d;{I z=1;DO(r,z=z*d[i]);R z;}
A ga(t,r,d)I *d;{A z=(A)ma(5+tr(r,d));z->t=t,z->r=r,mv(z->d,d,r);
 R z;}

V1(iota){I n=*w->p;A z=ga(0,1,&n);DO(n,z->p[i]=i);R z;}
V2(plus){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
 DO(n,z->p[i]=a->p[i]+w->p[i]);R z;}
V2(from){I r=w->r-1,*d=w->d+1,n=tr(r,d);
 A z=ga(w->t,r,d);mv(z->p,w->p+(n**a->p),n);R z;}
V1(box){A z=ga(1,0,0);*z->p=(I)w;R z;}
V2(cat){I an=tr(a->r,a->d),wn=tr(w->r,w->d),n=an+wn;
 A z=ga(w->t,1,&n);mv(z->p,a->p,an);mv(z->p+an,w->p,wn);R z;}
V2(find){}
V2(rsh){I r=a->r?*a->d:1,n=tr(r,a->p),wn=tr(w->r,w->d);
 A z=ga(w->t,r,a->p);mv(z->p,w->p,wn=n>wn?wn:n);
 if(n-=wn)mv(z->p+wn,z->p,n);R z;}
V1(sha){A z=ga(0,1,&w->r);mv(z->p,w->d,w->r);R z;}
V1(id){R w;}
V1(size){A z=ga(0,0,0);*z->p=w->r?*w->d:1;R z;}

pi(i){P("%d ",i);}
nl(){P("\n");}
pr(w)A w;{I r=w->r,*d=w->d,n=tr(r,d);
    DO(r,pi(d[i]));
    nl();
    if(w->t)
        DO(n,P("< ");pr(w->p[i]))
    else
        DO(n,pi(w->p[i]));
    nl();}

struct {
    C c; A (*vm)(); A (*vd)();
} op[] = {
    { 0, 0, 0 },
    { '+', id,   plus },
    { '{', size, from },
    { '~', iota, find },
    { '<', box,  0 },
    { '#', sha,  rsh },
    { ',', 0,    cat },
    { 0, 0, 0 }
};
I st[26];
qp(a){R  abs(a)>='a' && abs(a)<='z';}
qv(a){R 0<=abs(a) && abs(a)<'a';}

struct a nullob = {.r=1};
A null = &nullob;
A ex(e)I *e;{I a=*e;
    if(!a)R null;
 if(qp(a)){
     if(e[1]=='=')
         R (A)(st[a-'a']=(I)ex(e+2));
     a= st[ a-'a'];}
 R qv(a)?(op[a].vm)(ex(e+1)):e[1]?(op[e[1]].vd)(a,ex(e+2)):(A)a;}
noun(c){A z;
    if(c<'0'||c>'9')R 0;
    z=ga(0,0,0);
    *z->p=c-'0';
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

main(){C *s = NULL;
    specialtty();
    while(getln(&s))
        pr(ex(wd(s)));
    restoretty();
}
