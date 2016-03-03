/*
 *  Word Formation (scanning)
 *
 *  As shown in en.c and en.h, the "encoding module",
 *  characters are stored in 24bits of a 32bit int, so Unicode
 *  characters are referred-to by their UCS4 code. 
 *
 *  This decision affects the scanner code in that it must deal 
 *  with "int-strings" although the contents are expected to 
 *  primarily be restricted to the ascii domain. One special char 
 *  recognized by the scanner is the left-arrow char ← which is 
 *  used for assignment of values to variables. 
 *
 *  The scanner is also unusual in that it treats most characters 
 *  as identifier characters, even the punctuation chars which 
 *  designate functions. These identifiers are resolved later 
 *  during symbol-lookup using prefix-matching to further scan 
 *  and parse the identifiers. For the current purpose of these 
 *  functions, it is sufficient to distinguish numbers from non- 
 *  numbers and to ensure that certain special characters like 
 *  the left-arrow and the parens are encoded as single tokens
 *  and not parts of identifiers. 
 *
 *  So it's a state-machine that runs through each character 
 *  of the int-string. The character is classified into a
 *  character-class which determines the column of the big table.
 *  The current state (initially 0 or "ini") determines 
 *  the row of the big table. The value in the table encodes 
 *  a new state (the 10s value) and an action (the 1s value). 
 *  The action code adjusts the start-of-token position in 
 *  the strings and can trigger the generation of a new token. 
 *  The new token is packed into an integer handle and simply 
 *  appended to the array structure to be returned. 
 *
 *  The state-machine itself is "programmed" by the table and
 *  enum definitions in wd_private.h.
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ar.h" // array type
#include "en.h" // atomic encoding

#include "wd.h"
#include "debug.h"

typedef int token;
typedef char state;
typedef char state_and_action_code;

#include "wd_private.h"

array scan_expression(int *s, int n){
    array result = array_new(n+1);
    token *p = result->data, *const p1 = p+1;
    state ss, st; /* last state, current state */
    state_and_action_code cc;
    int i,j;

    for (i=j=0, ss=st=0; i < n; i++, ss=st, st=cc/10){
        cc = wdtab[st][ character_class(s[i]) ];
        DEBUG("-%d-\n",cc);

        switch (cc%10){
            case 0: /* do nothing */
                    break;

            case 1: *p++ = newobj(s+j, i-j, st*10);
                    j=i;
                    break;

            case 2: j=i;
                    break;

            case 3: *p++ = newobj(s+j, i-1-j, ss*10);
                    j=i-1;
                    break;
        }

        if (p > p1) p=collapse_adjacent_numbers_if_needed(p);
    }

    result->dims[0] = p - result->data; // set actual encoded length
    return result;
}

token *collapse_adjacent_numbers_if_needed(token *p){
    if (gettag(p[-2])==LITERAL && gettag(p[-1])==LITERAL){
        --p;
        p[-1]=cache(ARRAY, vector(p[-1],p[0]));
    } else if (gettag(p[-2])==ARRAY && gettag(p[-1])==LITERAL){
        --p;
        p[-1]=cache(ARRAY, cat(getptr(p[-1]), scalar(p[0])));
    }
    return p;
}


token new_numeric(int *s, int n){
    DEBUG("num:%d\n", n);
    char buf[n+1];
    for (int i=0; i<n; i++) buf[i] = s[i];
    buf[n] = 0;

    char *p;
    token t = newdata(LITERAL, strtol(buf,&p,10));
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

token new_string(int *s, int n){
    DEBUG("str:%d\n", n);
    array t=array_new(n);
    int i,j,q;
    //for (int i=0; i<n; i++) *elem(t,i) = newdata(CHAR, s[i]);
    for (i=1,j=0,q=0; i<n-1; i++){
        if (q){
            q=0; continue;
        } else if (s[i]=='\'') {
            q=1;
        }
        *elem(t,j++) = newdata(CHAR, s[i]);
    }
    t->dims[0]=j;
    return cache(ARRAY, t);
}

token new_executable(int *s, int n){
    DEBUG("prog:%d\n", n);
    if (n==1){
        if (*s == '(') return newdata(LPAROBJ, 0);
        if (*s == ')') return newdata(RPAROBJ, 0);
        return newdata(PCHAR, *s);
    } else {
        array t=array_new(n);
        for (int i=0; i<n; i++)
            *elem(t,i) = newdata(PCHAR, s[i]);
        return cache(PROG, t);
    }
}

token newobj(int *s, int n, int state){
    switch (state){
        case num:
        case dit: return new_numeric(s, n);

        case quo:
        case str: return new_string(s, n);

        case ini:
        case dot:
        case dut:
        case oth:
        case sng: return new_executable(s, n);

        default:  return null;
    }
}

