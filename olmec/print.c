#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "editor.h"
#include "encoding.h"
#include "symtab.h"
#include "array.h"
#include "verbs.h"
#include "xverb.h"

int printatom(int x, int width, int bprint){
    if (!bprint){
        switch(gettag(x)){
        case NULLOBJ: return strlen("NULL");
        case CHAR:
        case PCHAR:
            return 1;
        case LITERAL:
            return snprintf(NULL, 0, "%d", getval(x));
        default:
            return strlen("00000000(00,0000)");
        }
    } else {
        switch(gettag(x)){
        case NULLOBJ: printf("NULL");
                      break;
        case CHAR:
        case PCHAR:
            if (width)
                printf(" %*s", width, basetooutput(getval(x)));
            else
                printf("%s", basetooutput(getval(x)));
            break;
        case VERB:
            printf("%*s", width,
                    basetooutput(getval( ((verb)getptr(x))->id ))); break;
        case ADVERB:
            printf("%*s", width,
                    basetooutput(getval( ((verb)getptr(x))->id ))); break;
        case XVERB:
            printf("%*s", width,
                    basetooutput(getval( ((xverb)getptr(x))->id ))); break;
        case LITERAL:
            printf(" %*d", width, getval(x)); break;
        default:
            printf(" %08x(%d,%d)", x, gettag(x), getval(x));
        }
    }
    return width;
}

int printarray(array t, int width){
    t = makesolid(t);
    int maxwidth;

    if (width){ maxwidth = width; }
    else {
        int n = productdims(t->rank,t->dims);
        if (n==0) {
            printf("NIL\n");
            return 0;
        }

        maxwidth = 0;
        for (int i=0; i<n; ++i){
            int size;
            size = printatom(t->data[i], 0, 0);
            if (size>maxwidth)
                maxwidth = size;
        }
    }

    switch(t->rank){
    case 0: //printf("%*d\n", maxwidth, *t->data); break;
            printatom(t->data[0], maxwidth, 1);
    case 1: for (int i=0; i<t->dims[0]; ++i)
                //printf("%*d\n", maxwidth, *elem(t,i));
                printatom(*elem(t,i), maxwidth, 1);
            break;
    default:
            for (int i=0; i<t->dims[0]; ++i, printf("\n")){
                array ts = slice(t,i);
                printarray(ts, maxwidth);
                free(ts);
            }
            break;
    }
    return maxwidth;
}


void print(int x, int width){
    //printf("%08x(%d,%d)", x, gettag(x), getval(x));
    switch(gettag(x)){
        default: printatom(x, 0, 1);
                 printf("\n");
                 break;
        case PROG:
        case ARRAY: {
            array t = getptr(x);
            printarray(t, 0);
            printf("\n");

            if (0) {
                printf("\n");
                //printf("%d\n",t->rank);
                for (int i=0; i<t->rank; i++)
                    printf("%d ", t->dims[i]);
                //printf("\n");

                int n = productdims(t->rank,t->dims);
                printf("n=%d\n", n);
                int scratch[t->rank];
                for (int i=0; i<n; i++){
                    int xx = *elema(t,vector_index(i,t->dims,t->rank,scratch));
                    char *app = "";
                    for (int j=0; j<t->rank; j++, app=",")
                        printf("%s%d", app, scratch[j]);
                    printf(": ");
                    printf("%08x(%d,%d)", xx, gettag(xx), getval(xx));
                    switch(gettag(xx)){
                        case CHAR:
                        case PCHAR:
                            printf(" %s", basetooutput(getval(xx))); break;
                        case VERB:
                            printf(" %s",
                                    basetooutput(getval(
                                            ((verb)getptr(xx))->id ))); break;
                        case LITERAL:
                            printf(" %d", getval(xx)); break;
                    }
                    printf("\n");
                }
            }
        } break;
    }
}

