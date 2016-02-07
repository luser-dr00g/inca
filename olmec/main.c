#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "ar.h"
#include "ed.h"
#include "en.h"
#include "io.h"
#include "st.h"
#include "wd.h"

symtab env;

int main() {
    int *buf = NULL;
    char *prompt = "\t";
    int n;

    env = makesymtab(10);

    if (isatty(fileno(stdin))) specialtty();

    while(get_line(prompt, &buf, &n)){
        int i;
        //puts(buf);
        for (i=0;i<n;i++)
            printf("%04x ", buf[i]);
        array a = wd(buf, n);
        printf("\n");
        printf("%d\n", a->rank);
        for (i=0;i<a->rank;i++)
            printf("%d ", a->dims[i]);
        printf("\n");
        for (i=0;i<a->dims[0];i++)
            printf("%d(%d,%d) ", a->data[i], gettag(a->data[i]), getval(a->data[i]));
        printf("\n");
    }

    if (isatty(fileno(stdin))) restoretty();
    return 0;
}
