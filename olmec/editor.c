#include<errno.h>
#include<limits.h>
#include<math.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<termios.h>
#include<unistd.h>

/* Special ascii control-code macros
   */
#define ESC(x) "\x1b" #x
#define ESCCHR '\x1b'
#define CTL(x) (x-64)
#define EOT 004
#define DEL 127
#define MODE1(x) (x|1<<7)


#include "alpha.h"


int inputtobase(int c, int mode){
    int i;
    for (i=0;i<(sizeof alphatab/sizeof*alphatab);i++)
        if (c==*alphatab[i].input && mode==alphatab[i].ext)
            return alphatab[i].base;
    return mode? MODE1(c): c;
}

char *basetooutput(int c){
    int i;
    for (i=0;i<(sizeof alphatab/sizeof*alphatab);i++)
        if (c==alphatab[i].base)
            return alphatab[i].output;
    return "";
}

struct termios tm;

void specialtty(){

    fputs(ESC()")B",stdout); // set G1 charset to B:usascii
    fputs(ESC(*0),stdout); // set G2 to 0:line drawing ESC(n)
    fputs(ESC(+A),stdout); // set G3 to A:"uk" accented ESC(o)
    fputc(CTL('N'),stdout); // select G1 charset
                            // ESC(n): select G2
                            // ESC(o): select G3

    tcgetattr(0,&tm);

    struct termios tt=tm;
    tt.c_iflag |= IGNPAR; //ignore parity errors
    tt.c_iflag &= ~(IGNBRK | PARMRK | ISTRIP | ICRNL
                    | IXON | IXANY | IXOFF); //ignore special characters
    tt.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON
                    /*| ISIG*/ ); // non-canonical mode, no echo, no kill
    //tt.c_lflag &= ~IEXTEN;
    tt.c_cflag &= ~(CSIZE | PARENB);
    tt.c_cflag |= CS8;
    //tt.c_oflag &= ~OPOST; // disable special output processing
    tt.c_oflag |= OPOST;
    tt.c_cc[VMIN] = 3; // min chars to read
    tt.c_cc[VTIME] = 1; // timeout
    //cfmakeraw(&tt);
    if (tcsetattr(0,TCSANOW,&tt) == -1)
        perror("tcsetattr");

#if 0
#define DO(n,x)  {int i=0,_n=(n);for(;i<_n;++i){x;}}
    fputs("\x1B*0\x1Bn",stdout); DO('~'-' ',printf("%c",' '+i))printf("\x1Bo\n");
    fputs("\x1B*A\x1Bn",stdout); DO('~'-' ',printf("%c",' '+i))printf("\x1Bo\n");
    fputs("\x1B*B\x1Bn",stdout); DO('~'-' ',printf("%c",' '+i))printf("\x1Bo\n");
#endif
}

void restoretty(){
    tcsetattr(0,TCSANOW,&tm);
}

int *get_line(char *prompt, int **bufref, int *len, int *expn){
    int mode = 0;
    int tmpmode = 0;
    int *p;

    if (prompt) fputs(prompt,stdout);
    fflush(stdout);
    if (!*bufref) *bufref = malloc((sizeof**bufref) * (*len=256));
    p = *bufref;

    while(1){
        int c;
        if (p-*bufref>*len){
            int *t = realloc(*bufref,(sizeof**bufref) * (*len*=2));
            if (t) *bufref = t;
            else { *len/=2; return NULL; }
        }

        char key[3];
        int n;
        n = -1;
        while(n==-1){
            n = read(0, key, 3);
            c = key[0];
        }

        //printf("%d\n", c);
        switch(c){
        case EOF:
        case EOT: if (p==*bufref) goto err;
                  break;
        case ESCCHR:
                switch(n){
                case 1:
                    tmpmode = 1;
                    break;
                case 2:
                    c = key[1];
                    switch(c){
                    default:
                        tmpmode = 1;
                        goto storechar;
                        break;
                    }
                case 3:
                    c = key[1];
                    printf("%02x%c%c",key[0],key[1],key[2]);
                    fflush(stdout);
                    break;
                }
                break;
        case '\r':
        case '\n':
                fputc('\r', stdout);
                fputc('\n', stdout);
                *p++ = c;
                goto breakwhile;
        case CTL('N'):
                mode = !mode;
                tmpmode = 0;
                break;
        case CTL('U'):
                while(p>*bufref){
                    fputs("\b \b", stdout);
                    fflush(stdout);
                    --p;
                }
                tmpmode = 0;
                break;
        case '\b':
        case DEL:
                fputs("\b \b", stdout);
                fflush(stdout);
                if (p!=*bufref) --p;
                break;
        default:
storechar:
                c = inputtobase(c,mode|tmpmode);
                *p++ = c;
                tmpmode = 0;
                fputs(basetooutput(c), stdout);
                fflush(stdout);
                break;
        }
    }
breakwhile:
    *p++ = 0;
    *expn = p-*bufref;
err:
    return p==*bufref?NULL:*bufref;
}

