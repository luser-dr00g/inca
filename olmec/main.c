#include <string.h>
#define _BSD_SOURCE
#include <stdio.h>
#include <unistd.h>

#include "common.h"
#include "array.h"
#include "editor.h"
#include "encoding.h"
#include "io.h"
#include "symtab.h"
#include "exec.h"
#include "lex.h"
#include "verbs.h"
#include "adverbs.h"
#include "xverb.h"
#include "print.h"
#include "number.h"

// the global symbol table
symtab env;

// quad-neg variable controls minus/hi-minus semantics in
// the lexical analysis
void init_quad_neg(symtab st){
    define_symbol(st, newdata(PCHAR, 0x2395),newdata(PCHAR, '-'), 0);
}

// define quad-k variable illustrating alt-keybaord layout
// type quad with alt-l
void init_quad_k(symtab st){
    //alt-keyboard
    //
    //-> iterate over string
    char *rows[] = {
        "~!@#$%^&*()_+",
        "`1234567890-=",
        "QWERTYUIOP{}|",
        "qwertyuiop[]\\",
        "ASDFGHJKL:\"",
        "asdfghjkl;'",
        "ZXCVBNM<>?",
        "zxcvbnm,./",
    };
    array qk = array_new_dims(8,13);
    for (int i=0,j; i<8; ++i){
        for (j=0; j<13; ++j){
            if (!rows[i][j]) break;
            *elem(qk,i,j) = newdata(PCHAR, inputtobase(rows[i][j],1));
        }
        for (; j<13; ++j){
            *elem(qk,i,j) = newdata(PCHAR, inputtobase(' ',0));
        }
    }
    define_symbol(st,newdata(PCHAR, 0x2395),newdata(PCHAR, 'k'), cache(ARRAY, qk));


    //normal keyboard
    array qa = array_new_dims(8,13);
    for (int i=0,j; i<8; ++i){
        for (j=0; j<13; ++j){
            if (!rows[i][j]) break;
            *elem(qa, i, j) = newdata(PCHAR, inputtobase(rows[i][j],0));
        }
        for (; j<13; ++j){
            *elem(qa, i, j) = newdata(PCHAR, inputtobase(' ',0));
        }
    }
    define_symbol(st,newdata(PCHAR, 0x2395),newdata(PCHAR, 'a'), cache(ARRAY, qa));
}

int mainloop(){
    static int *buf = NULL;
    static int buflen;
    int expn;
    char *prompt = "        ";
    int last_was_assn;

    while((buf?buf[0]=0:0), get_line(prompt, &buf, &buflen, &expn)){

        IFDEBUG(2,
            for (int i=0;i<expn;i++)
                DEBUG(2,"%04x ", buf[i]);
            DEBUG(2,"\n");
            );

        array expr = array_new_dims(expn);
        memcpy(expr->data,buf,expn*sizeof(int));

        object e = scan_expression(expr, env);

        object x = execute(e, env, &last_was_assn);
        //object x = execute_expression(a, env, &last_was_assn);
        DEBUG(2, "last_was_assn = %d\n", last_was_assn);
        IFDEBUG(2, print(x, 10, 1));

        if (!last_was_assn && x!=mark)
            print(x, 0, 1);
    }
    return 0;
}

void init_all(){
    init_en();
    init_array();
    env = makesymtab(10);
    env->value = null; // set root-node value
    init_vb(env);
    init_av(env);
    init_xverb(env);
    init_quad_neg(env);
    init_quad_k(env);
    init_number(env);
    //print(inf, 0, 1);
    //print(neginf, 0, 1);
}

int main() {
    int do_tty = isatty(fileno(stdin));
    init_all();

    if (do_tty) specialtty();

        mainloop();

    if (do_tty) restoretty();
    return 0;
}

