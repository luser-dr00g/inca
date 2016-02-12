/* symbol table */

typedef struct st {
    int key;
    int val;
    int n;
    struct st **tab /*[n]*/ ;
} *symtab;

symtab makesymtab(int n);
/*  mode=0: prefix match
    mode=1: defining search */
symtab findsym(symtab st, int **spp, int *n, int mode);

