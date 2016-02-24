#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "ar.h"
#include "ed.h"
#include "en.h"
#include "io.h"
#include "st.h"
#include "ex.h"
#include "wd.h"
#include "vb.h"
#include "av.h"

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
            printf("%04x ", buf[i]);
        printf("\n");

        array a = scan_expression(buf, n);
        printf("\n");

        printf("%d\n", a->rank);
        for (i=0;i<a->rank;i++)
            printf("%d ", a->dims[i]);
        printf("\n");

        for (i=0;i<a->dims[0];i++)
            printf("%d(%d,%x) ", a->data[i],
                    gettag(a->data[i]), getval(a->data[i]));
        printf("\n");
        printf("%p\n", getptr(a->data[0]));

        int x = execute_expression(a,env);
        printf("%d(%d,%x)\n", x, gettag(x), getval(x));
        switch(gettag(x)){
            case ARRAY: {
                array t = getptr(x);
                printf("%d\n",t->rank);
                for (i=0; i<t->rank; i++)
                    printf("%d ", t->dims[i]);
                printf("\n");
                for (i=0; i<t->dims[0]; i++){
                    int xx = *elem(t,i);
                    printf("%d: %d(%d,%d)\n", i, xx, gettag(xx), getval(xx));
                }
            } break;
        }
    }

    if (isatty(fileno(stdin))) restoretty();
    return 0;
}

