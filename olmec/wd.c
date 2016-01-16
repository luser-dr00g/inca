#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ar.h"
#include "en.h"

#define ALPHALOWER "abcdefghijklmnopqrstuvwxyz"
#define ALPHAUPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define DIGIT "0123456789"
#define DOT "."
#define SPACE " \t"
#define ZEROCL ""

char *cclass[] = {0, ALPHAUPPER ALPHALOWER, DIGIT, DOT, SPACE};
int wdtab[][sizeof cclass/sizeof*cclass] = {
    /*char-class*/
    /*null  aA    0-9   .     sp   */ /*    state  */
    { 30+2, 20+2, 10+2, 10+2, 0+0  }, /*  0 init   */
    { 30+1, 20+1, 10+0, 10+0, 10+0 }, /* 10 number */
    { 30+1, 20+0, 10+1, 30+1, 0+1  }, /* 20 name   */
    { 30+1, 20+1, 10+1, 30+1, 0+1  }, /* 30 other  */
};

int newobj(int *s, int n, int state){
    int t;
    switch(state){
        case 1: //number
            {
                char buf[n+1];
                for (int i=0; i<n; i++)
                    buf[i] = s[i];
                buf[n] = 0;
                return newdata(LITERAL,strtol(buf,NULL,10));
            }
        case 2: //name
            break;
        case 3: //other
            break;
    }
    return newdata(NULLOBJ,0);
}

#define emit(a,b,c) (*p++=newobj(s+(a),(b)-a,c))

array wd(int *s, int n){
    int a,b;
    int i,j,i_,state,oldstate;
    int c;
    char *cp;
    array z = array_new(n+1);
    int *p=z->data;

    state=0;
    for (i=0; i<n; i++){
        c=s[i];
        a=0;
        for (i_=1; i_<sizeof cclass/sizeof*cclass; i_++){
            if (cp=strchr(cclass[i_],c)){
                a=i_;
                break;
            }
        }
        b=wdtab[state][a];
        oldstate=state;
        state=b/10;
        switch(b%10){  //encoded actions
            case 0: break;
            case 1: emit(j,i,oldstate);  // generate a token (and)
                    /*fallthrough*/
            case 2: j=i; break;          // reset start index
        }
    }
    *p++=0;
    return z;
}

