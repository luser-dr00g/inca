#include <stdio.h>
#include <unistd.h>

#include "ed.h"
#include "io.h"

int main() {
    int *buf = NULL;
    char *prompt = "\t";
    int n;

    if (isatty(fileno(stdin))) specialtty();

    while(get_line(prompt, &buf, &n)){
        int i;
        //puts(buf);
        for (i=0;i<n;i++)
            printf("%02x ", buf[i]);
    }

    if (isatty(fileno(stdin))) restoretty();
    return 0;
}
