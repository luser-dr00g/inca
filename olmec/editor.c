/*
 * The editor functions handle the *READ* part of the REPL.
 * It defines a large character-translation table to coordinate the
 * input-form/internal-rep/output-form for "normal" keyboards and an
 * "alternate" keyboard accessed with the alt key (also toggle-able
 * with ctrln). The alternate keyboard is patterned after the classic
 * APL keyboards I've seen. The ⎕a and ⎕k variables illustrate the
 * two keyboards, normal and ALT respectively.

$ ./olmec
        ⎕a
 ~ ! @ # $ % ^ & * ( ) _ +
 ` 1 2 3 4 5 6 7 8 9 0 - =
 Q W E R T Y U I O P { } |
 q w e r t y u i o p [ ] \
 A S D F G H J K L : "    
 a s d f g h j k l ; '    
 Z X C V B N M < > ?      
 z x c v b n m , . /      

        ⎕k
 · ⌶ ⍫ ⍋ ⍒ ⌽ ⍉ ⊖ ⍟ ⍱ ⍲ ⌸ ⌹
 ⋄ ¨ ¯ < ≤ = ≥ > ≠ ∨ ∧ × ÷
 ¿ ⍹ ⍷ ℝ ⍭ ¥ ⌀ ⍸ ⍥ £ ⊣ ⊢ ⍙
 ⍡ ⍵ ∈ ⍴ ⌾ ↑ ↓ ⍳ ○ ⋆ ← → ⍀
 ⍶ ⌷ ⍄ ≡ ⍋ ⍒ ⍤ ⍃ ⍞ ⍂ ⌻    
 ⍺ ⌈ ⌊ ⍣ ∇ ∆ ∘ ⍢ ⎕ ⍁ ´    
 ⍧ ⌺ ⍝ ⍦ ⍎ ⍕ ⍔ « » ¶      
 ⊂ ⊃ ∩ ∪ ⊥ ⊤ ⍍ ⍪ ∵ ⌿      


 *
 * It sets up a specialtty() mode using termios settings and VT220
 * charset codes. The VT codes are largely a crutch until I better
 * understand how to interface Unicode more directly with Xterm.
 * In particular, diaeresis and macron and a few others do not 
 * copy correctly with mouse selection. They display wrongly pasted
 * here in the source in vim.
 * 
 */
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


#include "common.h"
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

void restoretty(){
    tcsetattr(0,TCSANOW,&tm);
}

void specialtty(){

    // is the use of these causing my problems
    // outputing macron as \xc2\xaf or \xaf ?
    fputs(ESC()")B",stdout); // set G1 charset to B:usascii
    fputs(ESC(*0),stdout); // set G2 to 0:line drawing ESC(n)
    fputs(ESC(+A),stdout); // set G3 to A:"uk" accented ESC(o) (HI_MINUS)
    fputc(CTL('N'),stdout); // select G1 charset
                            // ESC(n): select G2
                            // ESC(o): select G3

    tcgetattr(0,&tm);

    struct termios tt=tm;
    tt.c_iflag |= IGNPAR; //ignore parity errors
    tt.c_iflag &= ~(IGNBRK | PARMRK | ISTRIP | ICRNL |INLCR |IGNCR
                    | IXON | IXANY | IXOFF); //ignore special characters
    tt.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL
                    | ICANON
                    /*| ISIG*/ ); // non-canonical mode, no echo, no kill
    //tt.c_lflag &= ~IEXTEN;
    tt.c_cflag &= ~(CSIZE | PARENB);
    tt.c_cflag |= CS8;
    tt.c_oflag &= ~(/*OPOST |*/ ONLCR | OCRNL | ONOCR); // disable special output processing
    tt.c_oflag |= (/*ONOCR |*/ OPOST | ONLCR );
    tt.c_cc[VMIN] = 3; // min chars to read
    tt.c_cc[VTIME] = 1; // timeout
    //cfmakeraw(&tt);
    if (tcsetattr(0,TCSANOW,&tt) == -1)
        perror("tcsetattr");

    atexit(restoretty);
#if 0
#define DO(n,x)  {int i=0,_n=(n);for(;i<_n;++i){x;}}
    fputs("\x1B*0\x1Bn",stdout); DO('~'-' ',printf("%c",' '+i))printf("\x1Bo\n");
    fputs("\x1B*A\x1Bn",stdout); DO('~'-' ',printf("%c",' '+i))printf("\x1Bo\n");
    fputs("\x1B*B\x1Bn",stdout); DO('~'-' ',printf("%c",' '+i))printf("\x1Bo\n");
#endif
}

void beep(){
    fputc(CTL('G'), stdout);
    fflush(stdout);
}

void tostartofline(){
    //fputc(0x0D, stdout);
    fputc('\r', stdout);
}

void clearline(){
    fputs(ESC([0J), stdout);
    //fputc(CTL('U'), stdout);
    //fflush(stdout);
}

void linefeed(){
    fputc('\n', stdout);
}

int *get_line(char *prompt, int **bufref, int *len, int *expn){
    int mode = 0;
    int tmpmode = 0;
    int *p;

    if (!*bufref) {
        *bufref = malloc((sizeof**bufref) * (*len=256));
        p = *bufref;
    } else {
        for (p = *bufref; *p; ++p)
            ;
    }

    while(1){
        int c;

        tostartofline();
        clearline();
        if (prompt) fputs(prompt,stdout);
        for (int *t=*bufref; t<p; ++t){
            fputs(basetooutput(*t), stdout);
        }
        fflush(stdout);

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
                case 1:  // bare ESC key
                    tmpmode = 1;
                    break;
                case 2:
                    c = key[1];  // ESC-$(c)
                    switch(c){
                    default:
                        tmpmode = 1;
                        goto storechar;
                        break;
                    }
                case 3:
                    c = key[2];  // 3-char ESC sequence
                    //printf("%02x%c%c",key[0],key[1],key[2]);
                    //fflush(stdout);
                    switch(c){
                        case 'A': //up-arrow
                        case 'B': //down-arrow
                        case 'C': //right-arrow
                        case 'D': //left-arrow
                            ;
                    }
                    break;
                }
                break;
        case '\r':
        case '\n':
                tostartofline();
                linefeed();
                *p++ = c;
                goto breakwhile;
        case CTL('N'):
                beep();
                mode = !mode;
                tmpmode = 0;
                break;
        case CTL('U'):
                /*
                while(p>*bufref){
                    fputs("\b \b", stdout);
                    fflush(stdout);
                    --p;
                }
                */
                p = *bufref;
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
                c = inputtobase(c,mode^tmpmode);
                *p++ = c;
                tmpmode = 0;
                //fputs(basetooutput(c), stdout);
                //fflush(stdout);
                break;
        }
    }
breakwhile:
    *p++ = 0;
    *expn = p-*bufref;
err:
    return p==*bufref?NULL:*bufref;
}

