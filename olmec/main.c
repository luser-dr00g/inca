#include <stdio.h>
#include <unistd.h>

#include "ed.h"
#include "io.h"
#include "st.h"

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
    }

    if (isatty(fileno(stdin))) restoretty();
    return 0;
}
