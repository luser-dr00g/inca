#ifndef VERBS_H_
#define VERBS_H_
#include "common.h"

#define VERBS_FOREACH(_) \
/*name base                   monad     dyad        f  g  h  mr lr rr*/ \
_(PLUS,'+',                   vid,      vplus,      0, 0, 0, 0, 0, 0 ) \
_(SUB, '-',                   vneg,     vminus,     0, 0, 0, 0, 0, 0 ) \
_(SUB2,0x00af,                vneg,     vminus,     0, 0, 0, 0, 0, 0 ) \
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
_(BASE,0x22a5/*alt-b*/,       mnone,    vbase,      0, 0, 0, 0, 0, 0 ) \
_(ENC, 0x22a4/*alt-n*/,       mnone,    vencode,    0, 0, 0, 0, 0, 0 ) \
_(ROT, 0x233d/*alt-%*/,       vreverse, vrotate,    0, 0, 0, 0, 0, 0 ) \
_(CONC,0x2282/*alt-z*/,       vconceal, dnone,      0, 0, 0, 0, 0, 0 ) \
_(REVL,0x2283/*alt-x*/,       vreveal,  dnone,      0, 0, 0, 0, 0, 0 ) \
/**/
struct verb {
    int id;
    monad *monad;
    dyad *dyad;
    int f,g,h; /* operator arguments */
    int mr,lr,rr; /* monadic,left,right rank*/
};

#define mnone vid
#define dnone vplus
#define DECLARE_VERB_FUNCTIONS(name, base, fmonad, fdyad, ...) \
    monad fmonad; \
    dyad fdyad;
VERBS_FOREACH(DECLARE_VERB_FUNCTIONS)
#undef mnone
#undef dnone
#undef DECLARE_VERB_FUNCTIONS

extern int vtab[];

void init_vb(symtab st);

#endif
