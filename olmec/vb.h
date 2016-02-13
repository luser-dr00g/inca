
#define VERBTAB(_) \
/*base, monad, dyad, f, g, h, mr,lr,rr*/ \
_('+',  id,    plus, 0, 0, 0, 0, 0, 0 ) \
/**/
typedef struct verb {
    int id;
    int (*monad)(int);
    int (*dyad)(int,int);
    int f,g,h; /* operator arguments */
    int mr,lr,rr; /* monadic,left,right rank*/
} *verb;

void init_vb(symtab st);

