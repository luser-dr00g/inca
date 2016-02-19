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
    dot=10, //initial dot  . 
    num=20, //integer 0 
    dit=30, //medial dot  0.
    fra=40, //fraction 0.0
    str=50, //initial quote '
    quo=60, //end or escape quote 'aaa''
    oth=70, //identifier or other symbol a+
    dut=80, //trailing dot +.
    sng=90, //copula or other self-delimiting symbol ()
};

int wdtab[][sizeof cclass/sizeof*cclass] = {
       /*char-class*/
/*state  none   0-9    .      '      (      )      sp     <-     \r  */
/*0*/ { oth+2, num+2, dot+2, str+2, sng+2, sng+2, ini+0, sng+2, ini+0 },
/*10*/{ oth+0, fra+0, oth+0, str+1, ini+1, ini+1, ini+1, sng+1, ini+1 },
/*20*/{ oth+1, num+0, dit+0, str+1, oth+1, oth+1, ini+1, sng+1, ini+1 },
/*30*/{ oth+0, num+1, dut+1, str+1, sng+1, sng+1, ini+1, sng+1, ini+1 },
/*40*/{ oth+1, fra+0, dot+1, str+1, sng+1, sng+1, ini+1, sng+1, ini+1 },
/*50*/{ str+0, str+0, str+0, quo+0, str+0, str+0, str+0, str+0, ini+1 },
/*60*/{ oth+1, num+1, dot+1, str+0, oth+1, oth+1, ini+1, sng+1, ini+1 },
/*70*/{ oth+0, num+1, dut+0, str+1, sng+1, sng+1, ini+1, sng+1, ini+1 },
/*80*/{ oth+0, num+3, dut+0, str+1, sng+1, sng+1, ini+1, sng+1, ini+1 },
/*90*/{ oth+1, num+1, dot+1, str+1, sng+1, sng+1, ini+1, sng+1, ini+1 },
};

#define emit(a,b,c) (*p++=newobj(s+(a),(b)-a,c*10))

array wd(int *s, int n){
    int a,b;
    int i,j,i_,state=0,oldstate=0,oldoldstate=0;
    int c;
    array z = array_new(n+1);
    int *p=z->data;
    //printf("n=%d\n",n);

    state=0;
    for (i=0; i<n; i++){
        printf("i= %d, state = %d, p-s = %d\n", i, state*10, (int)(p-z->data));
        c=s[i];
        a=0;
        for (i_=1; i_<sizeof cclass/sizeof*cclass; i_++){
            if (classint(cclass[i_],c)){
                a=i_;
                break;
            }
        }
        b=wdtab[state][a];
        oldoldstate=oldstate;
        oldstate=state;
        state=b/10;
        switch(b%10){  //encoded actions
            case 0: break;               // do nothing
            case 1: emit(j,i,oldstate);  // generate a token
                    j=i;
                    break;
            case 2: j=i;          // just reset start index
                    break;
            case 3: emit(j,i-1,oldoldstate);
                    j=i-1;
                    break;
        }
        if (p-z->data>1){
            if(gettag(p[-1])==LITERAL && gettag(p[-2])==LITERAL)
                (--p)[-1] = cache(ARRAY, vector(p[-1],p[0]));
            if(gettag(p[-1])==LITERAL && gettag(p[-2])==ARRAY)
                (--p)[-1] = cache(ARRAY, cat(getptr(p[-1]), vector(p[0])));
        }
    }
    z->dims[0] = p-z->data;

    //printf("wd %p\n", getptr(z->data[0]));
    return z;
}

int newobj(int *s, int n, int state){
    switch(state){
        case num:
        case dit:
        /*case fra:*/
            printf("number n=%d\n", n);
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
            printf("string n=%d\n", n);
            {
                array t=copy(cast(s,n));
                int i;
                for (i=0; i<n; i++)
                    t->data[i] = newdata(CHAR, t->data[i]);
                return cache(ARRAY, t);
            }
        case ini:
        case dot:
        case dut:
        case oth:
        case sng:
            printf("other n=%d\n", n);
            if (n==1){
                if (*s == '(') return newdata(LPAROBJ, 0);
                if (*s == ')') return newdata(RPAROBJ, 0);
                return newdata(PCHAR, *s);
            } else {
                array t=copy(cast(s,n));
                //printf("newobj %p\n", (void*)t);
                int i;
                for (i=0; i<n; i++)
                    t->data[i] = newdata(PCHAR, t->data[i]);
                int x = cache(PROG, t);
                //printf("newobj %d(%d,%d) %p\n", x, gettag(x), getval(x), getptr(x));
                return x;
            }
    }
    return newdata(NULLOBJ,0);
}


