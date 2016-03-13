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

void init_qneg(symtab st){
    array name = array_new_dims(2);
    name->data[0] = newdata(PCHAR, 0x2395);
    name->data[1] = newdata(PCHAR, '-');
    def(st, cache(PROG, name), 0);
}

// define quad-k variable illustrating alt-keybaord layout
// type quad with alt-l
void init_qk(symtab st){
    //keyboard
    array qkname = array_new_dims(2);
    qkname->data[0] = newdata(PCHAR, 0x2395);
    qkname->data[1] = newdata(PCHAR, 'k');
    array qk = array_new_dims(8,13);
#define P(_) newdata(PCHAR, inputtobase(*#_,1)),
#define Q(_) newdata(PCHAR, inputtobase(_,1)),
    int keys[] = {
        P(~) P(!) Q('@') P(#) P($) P(%) P(^) P(&) P(*)Q('(') Q(')') P(_) P(+)
        P(`) P(1) P(2) P(3) P(4) P(5) P(6) P(7) P(8) P(9) P(0) P(-) P(=)
        P(Q) P(W) P(E) P(R) P(T) P(Y) P(U) P(I) P(O) P(P) Q('{') Q('}') P(|)
        P(q) P(w) P(e) P(r) P(t) P(y) P(u) P(i) P(o) P(p) Q('[') Q(']')Q('\\')
        P(A) P(S) P(D) P(F) P(G) P(H) P(J) P(K) P(L) Q(':') Q('"') Q(' ') Q(' ')
        P(a) P(s) P(d) P(f) P(g) P(h) P(j) P(k) P(l) Q(';') Q('\'') Q(' ') Q(' ')
        P(Z) P(X) P(C) P(V) P(B) P(N) P(M) P(<) P(>) Q('?') Q(' ')Q(' ')Q(' ')
        P(z) P(x) P(c) P(v) P(b) P(n) P(m) Q(',') P(.) P(/) Q(' ')Q(' ')Q(' ')
    };
    memcpy(qk->data, keys, sizeof keys);
    def(st, cache(PROG, qkname), cache(ARRAY, qk));
#undef P
#undef Q

    array qaname = array_new_dims(2);
    qaname->data[0] = newdata(PCHAR, 0x2395);
    qaname->data[1] = newdata(PCHAR, 'a');
    array qa = array_new_dims(8,13);
#define P(_) newdata(PCHAR, inputtobase(*#_,0)),
#define Q(_) newdata(PCHAR, inputtobase(_,0)),
    int keysa[] = {
        P(~) P(!) Q('@') P(#) P($) P(%) P(^) P(&) P(*)Q('(') Q(')') P(_) P(+)
        P(`) P(1) P(2) P(3) P(4) P(5) P(6) P(7) P(8) P(9) P(0) P(-) P(=)
        P(Q) P(W) P(E) P(R) P(T) P(Y) P(U) P(I) P(O) P(P) Q('{') Q('}') P(|)
        P(q) P(w) P(e) P(r) P(t) P(y) P(u) P(i) P(o) P(p) Q('[') Q(']')Q('\\')
        P(A) P(S) P(D) P(F) P(G) P(H) P(J) P(K) P(L) Q(':') Q('"') Q(' ') Q(' ')
        P(a) P(s) P(d) P(f) P(g) P(h) P(j) P(k) P(l) Q(';') Q('\'') Q(' ') Q(' ')
        P(Z) P(X) P(C) P(V) P(B) P(N) P(M) P(<) P(>) Q('?') Q(' ')Q(' ')Q(' ')
        P(z) P(x) P(c) P(v) P(b) P(n) P(m) Q(',') P(.) P(/) Q(' ')Q(' ')Q(' ')
    };
    memcpy(qa->data, keysa, sizeof keysa);
    def(st, cache(PROG, qaname), cache(ARRAY, qa));
#undef P
#undef Q

#if 0
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
#endif
}

int main() {
    int *buf = NULL;
    int buflen;
    int expn;
    char *prompt = "        ";

    init_en();
    env = makesymtab(10);
    env->val = null;
    init_vb(env);
    init_av(env);
    init_xverb(env);
    init_qneg(env);
    init_qk(env);

    if (isatty(fileno(stdin))) specialtty();

    while(get_line(prompt, &buf, &buflen, &expn)){
        int i;

        //puts(buf);
        for (i=0;i<buflen;i++)
            DEBUG(2,"%04x ", buf[i]);
        DEBUG(2,"\n");

        array a = scan_expression(buf, expn, env);
        DEBUG(2,"\n");

        DEBUG(2,"%d\n", a->rank);
        for (i=0;i<a->rank;i++)
            DEBUG(2,"%d ", a->dims[i]);
        DEBUG(1,"\n");

        for (i=0;i<a->dims[0];i++)
            DEBUG(1,"%08x(%d,%d) ", a->data[i],
                    gettag(a->data[i]), getval(a->data[i]));
        DEBUG(1,"\n");

        int x = execute_expression(a,env);

        if (!last_was_assn)
            print(x, 0);
    }

    if (isatty(fileno(stdin))) restoretty();
    return 0;
}

