/* symbol table */

typedef struct st {
    int key;
    void *val;
    int n;
    struct st **tab /*[n]*/ ;
} *symtab;

symtab makesymtab(int n);
symtab findsym(symtab st, int **spp, int *n, int mode);

