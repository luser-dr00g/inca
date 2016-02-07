#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ar.h"
#include "en.h"

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
        case 2: //string
            {
                array t=copy(cast(s,n));
                return cache(ARRAY, t);
            }
        case 3: //other
            return newdata(CHAR,*s);
    }
    return newdata(NULLOBJ,0);
}

#define DIGIT (int[]){'0','1','2','3','4','5','6','7','8','9', 0}
#define DOT (int[]){'.', 0}
#define LPAR (int[]){'(', 0}
#define RPAR (int[]){')', 0}
#define QUOTE (int[]){'\'', 0}
#define SPACE (int[]){' ','\t', 0}

/* strchr for int strings */
int *classint(int *class, int el){
    for (; *class; class++)
        if (el == *class)
            return class;
    return NULL;
}

int *cclass[] = {0, DIGIT, DOT, QUOTE, LPAR, RPAR, SPACE};
enum state {
    init=0,
    number=10, //numbers and vectors of numbers
    dot=20,    //initial dot
    string=30, //initial quote
    quote=40,  //end or escape quote
    other=50,  //identifier or other symbol
};

int wdtab[][sizeof cclass/sizeof*cclass] = {
    /*char-class*/
    /*none      0-9       .         '         (         )         sp       */
    { other+2,  number+2, dot+2,    string+2, other+2,  other+2,  init+0   },
    { other+1,  number+0, number+0, string+1, other+1,  other+1,  number+0 },
    { otker+0,  number+0, other+0,  string+1, init+1,   init+1,   init+1   },
    { string+0, string+0, string+0, quote+0,  string+0, string+0, string+0 },
    { other+1,  number+1, dot+1,    string+0, other+1,  other+1,  init+1   },
    { other+0,  number+1, other+1,  string+1, string+1, string+1, init+1   },
};

#define emit(a,b,c) (*p++=newobj(s+(a),(b)-a,c))

array wd(int *s, int n){
    int a,b;
    int i,j,i_,state,oldstate;
    int c;
    array z = array_new(n+1);
    int *p=z->data;

    state=0;
    for (i=0; i<n; i++){
        c=s[i];
        a=0;
        for (i_=1; i_<sizeof cclass/sizeof*cclass; i_++){
            if (classint(cclass[i_],c)){
                a=i_;
                break;
            }
        }
        b=wdtab[state][a];
        oldstate=state;
        state=b/10;
        switch(b%10){  //encoded actions
            case 0: break;               // do nothing
            case 1: emit(j,i,oldstate);  // generate a token (and)
                    /*fallthrough*/
            case 2: j=i; break;          // reset start index
        }
    }
    *p++=0;
    return z;
}

