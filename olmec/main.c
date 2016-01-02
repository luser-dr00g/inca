#include <stdio.h>
#include <unistd.h>

#include "ed.h"
#include "io.h"

int main() {
    char *buf = NULL;
    char *prompt = "\t";
    int n;

    if (isatty(fileno(stdin))) specialtty();

    while(get_line(prompt, &buf, &n)){
        puts(buf);
    }

    if (isatty(fileno(stdin))) restoretty();
    return 0;
}
