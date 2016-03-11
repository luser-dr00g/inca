#define MODE1(x) (x|1<<7)

#define VERBTAB(_) \
/*name base                   monad     dyad        f  g  h  mr lr rr*/ \
_(PLUS,'+',                   vid,      vplus,      0, 0, 0, 0, 0, 0 ) \
_(SUB, '-',                   vneg,     vminus,     0, 0, 0, 0, 0, 0 ) \
_(MUL, MODE1('='),            vsignum,  vtimes,     0, 0, 0, 0, 0, 0 ) \
_(MUL2,'*',                   vsignum,  vtimes,     0, 0, 0, 0, 0, 0 ) \
_(DIV, MODE1('+'),            vrecip,   vdivide,    0, 0, 0, 0, 0, 0 ) \
_(MOD, '|',                   vabs,     vresidue,   0, 0, 0, 0, 0, 0 ) \
_(RHO, 0x2374/*rho alt-r*/,   vshapeof, vreshape,   0, 0, 0, 0, 0, 0 ) \
_(RHO2,'$',                   vshapeof, vreshape,   0, 0, 0, 0, 0, 0 ) \
_(TAL, '#',                   vtally,   dnone,      0, 0, 0, 0, 0, 0 ) \
_(IOTA,0x2373/*iota alt-i*/,  viota,    dnone,      0, 0, 0, 0, 0, 0 ) \
_(TAKE,'{',                   vhead,    vtake,      0, 0, 0, 0, 1, 0 ) \
_(DROP,'}',                   vbehead,  vdrop,      0, 0, 0, 0, 0, 0 ) \
_(CAT, ',',                   vravel,   vcat,       0, 0, 0, 0, 0, 0 ) \
_(LINK,';',                   vprenul,  vlink,      0, 0, 0, 0, 0, 0 ) \
_(INDR,'[',                   mnone,    vindexright,0, 0, 0, 0, 0, 0 ) \
_(INDL,']',                   mnone,    vindexleft, 0, 0, 0, 0, 0, 0 ) \
_(TAK2,0x2191/*up alt-y*/,    mnone,    vtake,      0, 0, 0, 0, 0, 0 ) \
_(DRO2,0x2193/*down alt-u*/,  mnone,    vdrop,      0, 0, 0, 0, 0, 0 ) \
_(COMP,0x001f,                mnone,    vcompress,  0, 0, 0, 0, 0, 0 ) \
_(EXP, 0x001e,                mnone,    vexpand,    0, 0, 0, 0, 0, 0 ) \
_(BASE,0x22a5,                mnone,    vbase,      0, 0, 0, 0, 0, 0 ) \
_(ENC, 0x22a4,                mnone,    vencode,    0, 0, 0, 0, 0, 0 ) \
/**/
typedef struct verb {
    int id;
    int (*monad)(int,struct verb*);
    int (*dyad)(int,int,struct verb*);
    int f,g,h; /* operator arguments */
    int mr,lr,rr; /* monadic,left,right rank*/
} *verb;

#define mnone vid
#define dnone vplus
#define VERBTAB_DECL(name, base, monad, dyad, ...) \
    int monad(int,verb); \
    int dyad(int,int,verb);
VERBTAB(VERBTAB_DECL)
#undef mnone
#undef dnone

extern int vtab[];

void init_vb(symtab st);

