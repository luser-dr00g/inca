#ifndef VERBS_H_
#define VERBS_H_
#include "common.h"

#define VERBS_FOREACH(_) \
/*name base                   nilad, monad     dyad        f  g  h  mr lr rr*/ \
_(PLUS,'+',                   nnone, vid,      vplus,      0, 0, 0, 0, 0, 0 ) \
_(SUB, '-',                   nnone, vneg,     vminus,     0, 0, 0, 0, 0, 0 ) \
_(SUB2,0x00af,                nnone, vneg,     vminus,     0, 0, 0, 0, 0, 0 ) \
_(MUL, MODE1('='),            nnone, vsignum,  vtimes,     0, 0, 0, 0, 0, 0 ) \
_(MUL2,'*',                   nnone, vsignum,  vtimes,     0, 0, 0, 0, 0, 0 ) \
_(DIV, MODE1('+'),            nnone, vrecip,   vdivide,    0, 0, 0, 0, 0, 0 ) \
_(MOD, '|',                   nnone, vabs,     vresidue,   0, 0, 0, 0, 0, 0 ) \
_(RHO, 0x2374/*rho alt-r*/,   nnone, vshapeof, vreshape,   0, 0, 0, 0, 0, 0 ) \
_(RHO2,'$',                   nnone, vshapeof, vreshape,   0, 0, 0, 0, 0, 0 ) \
_(TAL, '#',                   nnone, vtally,   dnone,      0, 0, 0, 0, 0, 0 ) \
_(IOTA,0x2373/*iota alt-i*/,  nnone, viota,    dnone,      0, 0, 0, 0, 0, 0 ) \
_(TAKE,'{',                   nnone, vhead,    vtake,      0, 0, 0, 0, 1, 0 ) \
_(DROP,'}',                   nnone, vbehead,  vdrop,      0, 0, 0, 0, 0, 0 ) \
_(CAT, ',',                   nnone, vravel,   vcat,       0, 0, 0, 0, 0, 0 ) \
_(LINK,';',                   nnone, vprenul,  vlink,      0, 0, 0, 0, 0, 0 ) \
_(INDR,'[',                   nnone, mnone,    vindexright,0, 0, 0, 0, 0, 0 ) \
_(INDL,']',                   nnone, mnone,    vindexleft, 0, 0, 0, 0, 0, 0 ) \
_(TAK2,0x2191/*up alt-y*/,    nnone, mnone,    vtake,      0, 0, 0, 0, 0, 0 ) \
_(DRO2,0x2193/*down alt-u*/,  nnone, mnone,    vdrop,      0, 0, 0, 0, 0, 0 ) \
_(COMP,0x001f,                nnone, mnone,    vcompress,  0, 0, 0, 0, 0, 0 ) \
_(EXP, 0x001e,                nnone, mnone,    vexpand,    0, 0, 0, 0, 0, 0 ) \
_(BASE,0x22a5/*alt-b*/,       nnone, mnone,    vbase,      0, 0, 0, 0, 0, 0 ) \
_(ENC, 0x22a4/*alt-n*/,       nnone, mnone,    vencode,    0, 0, 0, 0, 0, 0 ) \
_(ROT, 0x233d/*alt-%*/,       nnone, vreverse, vrotate,    0, 0, 0, 0, 0, 0 ) \
_(CONC,0x2282/*alt-z*/,       nnone, vconceal, dnone,      0, 0, 0, 0, 0, 0 ) \
_(REVL,0x2283/*alt-x*/,       nnone, vreveal,  dnone,      0, 0, 0, 0, 0, 0 ) \
_(NONE,0x2361/*alt-q*/,       nnone, vnoresult, vnoresultd,0, 0, 0, 0, 0, 0 ) \
_(NIL, 0x2300/*alt-U*/,       vnil,  mnone,    dnone,      0, 0, 0, 0, 0, 0 ) \
/**/

struct verb {
    object id;
    nilad *nilad;
    monad *monad;
    dyad *dyad;
    object f,g,h; /* operator arguments */
    int mr,lr,rr; /* monadic,left,right rank*/
};

#define nnone vnil
#define mnone vid
#define dnone vplus
#define DECLARE_VERB_FUNCTIONS(name, base, fnilad, fmonad, fdyad, ...) \
    nilad fnilad; \
    monad fmonad; \
    dyad fdyad;
VERBS_FOREACH(DECLARE_VERB_FUNCTIONS)
#undef nnone
#undef mnone
#undef dnone
#undef DECLARE_VERB_FUNCTIONS

extern object vtab[];

void init_vb(symtab st);

#endif
