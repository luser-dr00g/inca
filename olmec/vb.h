#define MODE1(x) (x|1<<7)

#define VERBTAB(_) \
/*base             monad     dyad        f  g  h  mr lr rr*/ \
_('+',             vid,      vplus,      0, 0, 0, 0, 0, 0 ) \
_('-',             vneg,     vminus,     0, 0, 0, 0, 0, 0 ) \
_('*',             vsignum,  vtimes,     0, 0, 0, 0, 0, 0 ) \
_(MODE1('+'),      vrecip,   vdivide,    0, 0, 0, 0, 0, 0 ) \
_(0x2374/*rho*/,   vshapeof, vreshape,   0, 0, 0, 0, 0, 0 ) \
_('#',             vtally,   0,          0, 0, 0, 0, 0, 0 ) \
_(0x2373/*iota*/,  viota,    0,          0, 0, 0, 0, 0, 0 ) \
_('{',             vhead,    vtake,      0, 0, 0, 0, 1, 0 ) \
_('}',             vbehead,  vdrop,      0, 0, 0, 0, 0, 0 ) \
_(',',             vravel,   vcat,       0, 0, 0, 0, 0, 0 ) \
_(';',             vprenul,  vlink,      0, 0, 0, 0, 0, 0 ) \
_('[',             0,        vindexright,0, 0, 0, 0, 0, 0 ) \
_(']',             0,        vindexleft, 0, 0, 0, 0, 0, 0 ) \
/**/
typedef struct verb {
    int id;
    int (*monad)(int,struct verb*);
    int (*dyad)(int,int,struct verb*);
    int f,g,h; /* operator arguments */
    int mr,lr,rr; /* monadic,left,right rank*/
} *verb;

void init_vb(symtab st);

