#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

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
#include "debug.h"
#include "print.h"

symtab env;

// define quad-k variable illustrating alt-keybaord layout
// type quad with alt-l
void init_qk(symtab st){
    //keyboard
    array qkname = array_new_dims(2);
    qkname->data[0] = newdata(PCHAR, 0x2395);
    qkname->data[1] = newdata(PCHAR, 'k');
    array qk = array_new_dims(8,13);
#define P(_) newdata(PCHAR, _)
    int keys[] = {
        P(MODE1('~')), P(0x00a8), P(0x00af), P('<'), P(0x2264),
            P('='), P(0x2265), P('>'), P(MODE1('*')),
            P(MODE1('(')), P(MODE1(')')),
            P('_'), P(MODE1('+')),
        P(MODE1('`')), P('1'), P('2'), P('3'), P('4'), P('5'),
            P('6'), P('7'), P('8'), P('9'), P('0'),
            P('-'), P(MODE1('=')),
        P('Q'), P('W'), P('E'), P('R'), P('T'),
            P('Y'), P('U'), P('I'), P('O'), P('P'),
            P(0x2192), P(MODE1('}')), P(MODE1('|')),
        P('?'), P(0x2375), P(0x2208), P(0x2374), P(0x223c),
            P(0x2191), P(0x2193), P(0x2373), P(0x25cb), P(0x22c6), P(0x2190),
            P(']'), P(0x2340),
        P('A'), P('S'), P('D'), P('F'), P('G'),
            P('H'), P('J'), P('K'), P('L'), P(':'), P('"'),
            P(32), P(32),
        P(0x237a), P(0x2308), P(0x230a), P('_'), P(0x2207),
            P(0x2206), P(0x2218), P('\''), P(0x2395),
            P(MODE1(';')), P(MODE1('\'')),
            P(32), P(32),
        P('Z'), P('X'), P('C'), P('V'),
            P('B'), P('N'), P('M'), P(MODE1('<')), P(MODE1('>')), P(MODE1('?')),
            P(32), P(32), P(32),
        P(0x2282), P(0x2283), P(0x2229), P(0x222a), P(0x22a5), P(0x22a4), P('|'),
            P(MODE1(',')), P(MODE1('.')), P(0x233f),
            P(32), P(32), P(32),
    };
    memcpy(qk->data, keys, sizeof keys);

    def(st, cache(PROG, qkname), cache(ARRAY, qk));
}

int main() {
    int *buf = NULL;
    char *prompt = "        ";
    int n;

    init_en();
    env = makesymtab(10);
    env->val = null;
    init_vb(env);
    init_av(env);
    init_xverb(env);
    init_qk(env);

    if (isatty(fileno(stdin))) specialtty();

    while(get_line(prompt, &buf, &n)){
        int i;

        //puts(buf);
        for (i=0;i<n;i++)
            DEBUG(1,"%04x ", buf[i]);
        DEBUG(1,"\n");

        array a = scan_expression(buf, n);
        DEBUG(1,"\n");

        DEBUG(1,"%d\n", a->rank);
        for (i=0;i<a->rank;i++)
            DEBUG(1,"%d ", a->dims[i]);
        DEBUG(1,"\n");

        for (i=0;i<a->dims[0];i++)
            DEBUG(0,"%08x(%d,%d) ", a->data[i],
                    gettag(a->data[i]), getval(a->data[i]));
        DEBUG(0,"\n");

        int x = execute_expression(a,env);

        print(x, 0);
    }

    if (isatty(fileno(stdin))) restoretty();
    return 0;
}

