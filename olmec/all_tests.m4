divert(`-1')
# http://www.gnu.org/savannah-checkouts/gnu/m4/manual/m4-1.4.17/html_node/Foreach.html#Foreach
# foreach(x, (item_1, item_2, ..., item_n), stmt)
#   parenthesized list, simple version
define(`foreach', `pushdef(`$1')_foreach($@)popdef(`$1')')
define(`_arg1', `$1')
define(`_foreach', `ifelse(`$2', `()', `',
    `define(`$1', _arg1$2)$3`'$0(`$1', (shift$2), `$3')')')

define(`UNITS', (patsubst(UNITS,`\W',`,')))

divert`'dnl
int tests_run;

`#' include <stdio.h>
foreach(`unit', UNITS, `
`#' define main unit`'_main
`#' define tests_run unit`'_tests_run
`#' define all_tests unit`'_all_tests
`#' include "unit`'_test.c"
`#' undef main
`#' undef tests_run
`#' undef all_tests
int unit`'_test(){
    int ret;
    printf("---------------\n");
    printf("running unit`'_test\n");
    ret = unit`'_main();
    tests_run += unit`'_tests_run;
    return ret;
}
')dnl

int main(){
    int ret;
    ret = 0 foreach(`unit', UNITS, ` || unit`'_test() ') ;
    printf("Grand Total tests run: %d\n", tests_run);
    return ret;
}

