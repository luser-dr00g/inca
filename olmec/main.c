#include <stdarg.h>
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
#include "debug.h"

symtab env;

int main() {
    int *buf = NULL;
    char *prompt = "\t";
    int n;

    init_en();
    env = makesymtab(10);
    env->val = null;
    init_vb(env);
    init_av(env);

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
        printf("%08x(%d,%d)\n", x, gettag(x), getval(x));
        switch(gettag(x)){
            case LITERAL:
                printf("%d\n", getval(x));
                break;
            case ARRAY: {
                array t = getptr(x);

                printf("%d\n",t->rank);
                for (i=0; i<t->rank; i++)
                    printf("%d ", t->dims[i]);
                printf("\n");

                int n = productdims(t->rank,t->dims);
                printf("n=%d\n", n);
                int scratch[t->rank];
                for (i=0; i<n; i++){
                    int xx = *elema(t,vector_index(i,t->dims,t->rank,scratch));
                    char *app = "";
                    for (int j=0; j<t->rank; j++, app=",")
                        printf("%s%d", app, scratch[j]);
                    printf(": ");
                    printf("%08x(%d,%d)\n", xx, gettag(xx), getval(xx));
                }
            } break;
        }
    }

    if (isatty(fileno(stdin))) restoretty();
    return 0;
}

