#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ar.h"
#include "en.h"

#include "wd.h"

#define DIGIT (int[]){'0','1','2','3','4','5','6','7','8','9', 0}
#define DOT (int[]){'.', 0}
#define LPAR (int[]){'(', 0}
#define RPAR (int[]){')', 0}
#define QUOTE (int[]){'\'', 0}
#define SPACE (int[]){' ','\t', 0}
#define LEFT (int[]){0x2190, 0}
#define CR (int[]){0x0D, 0}

/* strchr for int strings */
int *classint(int *class, int el){
    for (; *class; class++)
        if (el == *class)
            return class;
    return NULL;
}

int *cclass[] = {0, DIGIT, DOT, QUOTE, LPAR, RPAR, SPACE, LEFT, CR};
enum state {
    ini=0,  //indeterminate
    num=10, //numbers and vectors of numbers
    dot=20, //initial dot
    str=30, //initial quote
    quo=40, //end or escape quote
    oth=50, //identifier or other symbol
    sng=60, //copula or other self-delimiting symbol
};

int wdtab[][sizeof cclass/sizeof*cclass] = {
/*char-class*/
/*none   0-9    .      '      (      )      sp     <-     \r  */
{ oth+2, num+2, dot+2, str+2, sng+2, sng+2, ini+0, sng+2, ini+0 },
{ oth+1, num+0, num+0, str+1, oth+1, oth+1, num+0, sng+1, ini+1 },
{ oth+0, num+0, oth+0, str+1, ini+1, ini+1, ini+1, sng+1, ini+1 },
{ str+0, str+0, str+0, quo+0, str+0, str+0, str+0, str+0, ini+1 },
{ oth+1, num+1, dot+1, str+0, oth+1, oth+1, ini+1, sng+1, ini+1 },
{ oth+0, num+1, oth+1, str+1, sng+1, sng+1, ini+1, sng+1, ini+1 },
{ oth+1, num+1, dot+1, str+1, sng+1, sng+1, ini+1, sng+1, ini+1 },
};

#define emit(a,b,c) (*p++=newobj(s+(a),(b)-a,c*10))

array wd(int *s, int n){
    int a,b;
    int i,j,i_,state,oldstate;
    int c;
    array z = array_new(n+1);
    int *p=z->data;
    //printf("n=%d\n",n);

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
                    //printf("wd %p\n", (void*)getptr(p[-1]));
            case 2: j=i; break;          // reset start index
        }
    }
    z->dims[0] = p-z->data;

    //printf("wd %p\n", getptr(z->data[0]));
    return z;
}

int newobj(int *s, int n, int state){
    switch(state){
        case num:
            printf("number\n");
            { //TODO create number vectors
                char buf[n+1];
                char *p;
                for (int i=0; i<n; i++)
                    buf[i] = s[i];
                buf[n] = 0;
                int t = newdata(LITERAL, strtol(buf,&p,10));
                if (*p) {
                    array z = scalar(t);
                    while(*p) {
                        int u = newdata(LITERAL, strtol(p,&p,10));
                        z = cat(z, scalar(u));
                    }
                    t = cache(ARRAY, z);
                }
                return t;
            }
        case quo:
        case str:
            printf("string\n");
            {
                array t=copy(cast(s,n));
                int i;
                for (i=0; i<n; i++)
                    t->data[i] = newdata(CHAR, t->data[i]);
                return cache(ARRAY, t);
            }
        case ini:
        case dot:
        case oth:
        case sng:
            printf("other\n");
            if (n==1){
                return newdata(CHAR, *s);
            } else {
                array t=copy(cast(s,n));
                //printf("newobj %p\n", (void*)t);
                int i;
                for (i=0; i<n; i++)
                    t->data[i] = newdata(CHAR, t->data[i]);
                int x = cache(PROG, t);
                //printf("newobj %d(%d,%d) %p\n", x, gettag(x), getval(x), getptr(x));
                return x;
            }
    }
    return newdata(NULLOBJ,0);
}


