#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "editor.h"
#include "encoding.h"
#include "symtab.h"
#include "array.h"
#include "verbs.h"
#include "xverb.h"
#include "number.h"
#include "print.h"


int printarray(array t, int width);

/* return 1 if element is nonscalar */
int checkatom(object x, int *pwidth){
    switch(gettag(x)){
    case NULLOBJ:
        *pwidth = strlen("NULL");
        return 0;
    case CHAR:
    case PCHAR:
        *pwidth = 1;
        return 0;
    case LITERAL:
        *pwidth = snprintf(NULL, 0, "%d", getval(x));
        return 0;
    case NUMBER:
        *pwidth = number_print_width(getptr(x));
        return 0;
    case PROG:
        *pwidth = 0;
        return 0;
    case EXPR:
        *pwidth = 1;
        return 1;
    case ARRAY:
        *pwidth = 2;
        return 1;
    default:
        *pwidth = strlen("00000000(00,0000)");
        return 1;
    }
}

int printatom(object x, int width){
    switch(gettag(x)){
    case NULLOBJ: printf("NULL");
                  break;
    case MARKOBJ: printf("%s", basetooutput(0x22c4));
                  break;
    case CHAR:
    case PCHAR:
        if (width)
            printf(" %*s", width, basetooutput(getval(x)));
        else
            printf("%s", basetooutput(getval(x)));
        break;
    case VERB: {
        verb v = getptr(x);
        if (v->f) print(v->f, width, 1);
        printf("%*s", width, basetooutput(getval(v->id)));
        if (v->g) print(v->g, width, 1);
        break;
    }
    case ADVERB: {
        verb v = getptr(x);
        if (v->f) print(v->f, width, 1);
        printf("%*s", width, basetooutput(getval(v->id)));
        if (v->g) print(v->g, width, 1);
        break;
    }
    case XVERB:
        printf("%*s", width,
                basetooutput(getval( ((xverb)getptr(x))->id ))); break;
    case LITERAL:
        printf(" %*d", width, getval(x)); break;
    case NUMBER:
        printf(" %s", number_get_str(getptr(x))); break;
    default:
        printf(" %08x(%d,%d)", x, gettag(x), getval(x));
    }
    return width;
}

void printindexdisplay(array t){
    //printf("\n");
    DEBUG(3,"%d\n",t->rank);
    printf("%s", basetooutput(0x2374)); // rho
    for (int i=0; i<t->rank; i++)
        printf("%d ", t->dims[i]);
    //printf("\n");

    int n = productdims(t->rank,t->dims);
    DEBUG(3,"n=%d", n);
    printf("\n");
    int scratch[t->rank];
    for (int i=0; i<n; i++){
        int xx = *elema(t,vector_index(i,t->dims,t->rank,scratch));
        char *app = "";
        for (int j=0; j<t->rank; j++, app=",")
            printf("%s%d", app, scratch[j]);
        printf(": ");
        DEBUG(3,"%08x(%d,%d)", xx, gettag(xx), getval(xx));
        //printf("\n");
        switch(gettag(xx)){
            case CHAR:
            case PCHAR:
                printf(" %s", basetooutput(getval(xx)));
                printf("\n");
                break;
            case ADVERB:
            case VERB:
                printf(" %s",
                        basetooutput(getval(
                                ((verb)getptr(xx))->id )));
                printf("\n");
                break;
            case LITERAL:
                printf(" %d", getval(xx));
                printf("\n");
                break;
            case PROG:
                print(xx, 1, 1);
                break;
            case EXPR:
            case BLOCK:
            case ARRAY:
                print(xx, 0, 1);
                break;
        }
    }
    printf("\n");
}


int printarray(array t, int width){
    IFDEBUG(3, printindexdisplay(t));
    t = makesolid(t);
    int maxwidth;
    int nonscalar = 0;

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
            if (nonscalar = checkatom(t->data[i], &size))
                break;
            if (size>maxwidth)
                maxwidth = size;
        }
    }

    if (nonscalar)
        printindexdisplay(t);
    else
        switch(t->rank){
        case 0: //DEBUG(1,"%*d\n", maxwidth, *t->data);
                printatom(t->data[t->cons], maxwidth);
                break;
        case 1: for (int i=0; i<t->dims[0]; ++i) {
                    //DEBUG(1,"%*d\n", maxwidth, *elem(t,i));
                    printatom(*elem(t,i), maxwidth);
                }
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


void print(object x, int width, int newline){
    DEBUG(3,"%08x(%d,%d)", x, gettag(x), getval(x));
    switch(gettag(x)){
        default: printatom(x, width);
                 if (newline) printf("\n");
                 break;
        case EXPR:
        case BLOCK:
        case PROG:
        case ARRAY: {
            array t = getptr(x);
            printarray(t, width);
            if (newline) printf("\n");

        } break;
    }
}

