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

symtab env;

int main() {
    int *buf = NULL;
    char *prompt = "\t";
    int n;

    init_en();
    env = makesymtab(10);
    env->val = null;

    if (isatty(fileno(stdin))) specialtty();

    while(get_line(prompt, &buf, &n)){
        int i;

        //puts(buf);
        for (i=0;i<n;i++)
            printf("%04x ", buf[i]);
        printf("\n");

        array a = wd(buf, n);
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

        int x = ex(a,env);
        printf("%d(%d,%x)\n", x, gettag(x), getval(x));
    }

    if (isatty(fileno(stdin))) restoretty();
    return 0;
}

