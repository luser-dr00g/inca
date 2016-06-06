#ifndef VERBS_H_
#define VERBS_H_
#include "common.h"

#define VERBS_FOREACH(param,_) \
/*name base                   nilad, monad     dyad        f  g  h  mr lr rr*/ \
_(param,PLUS,'+',                   nnone, vid,      vplus,      0, 0, 0, 0, 0, 0 ) \
_(param,SUB, '-',                   nnone, vneg,     vminus,     0, 0, 0, 0, 0, 0 ) \
_(param,SUB2,0x00af,                nnone, vneg,     vminus,     0, 0, 0, 0, 0, 0 ) \
_(param,MUL, MODE1('='),            nnone, vsignum,  vtimes,     0, 0, 0, 0, 0, 0 ) \
_(param,MUL2,'*',                   nnone, vsignum,  vtimes,     0, 0, 0, 0, 0, 0 ) \
_(param,DIV, MODE1('+'),            nnone, vrecip,   vdivide,    0, 0, 0, 0, 0, 0 ) \
_(param,MOD, '|',                   nnone, vabs,     vresidue,   0, 0, 0, 0, 0, 0 ) \
_(param,RHO, 0x2374/*rho alt-r*/,   nnone, vshapeof, vreshape,   0, 0, 0, 0, 0, 0 ) \
_(param,RHO2,'$',                   nnone, vshapeof, vreshape,   0, 0, 0, 0, 0, 0 ) \
_(param,TAL, '#',                   nnone, vtally,   dnone,      0, 0, 0, 0, 0, 0 ) \
_(param,IOTA,0x2373/*iota alt-i*/,  nnone, viota,    dnone,      0, 0, 0, 0, 0, 0 ) \
_(param,CAT, ',',                   nnone, vravel,   vcat,       0, 0, 0, 0, 0, 0 ) \
_(param,LINK,';',                   nnone, vprenul,  vlink,      0, 0, 0, 0, 0, 0 ) \
_(param,INDR,'{',                   nnone, mnone,    vindexright,0, 0, 0, 0, 0, 0 ) \
_(param,INDL,'}',                   nnone, mnone,    vindexleft, 0, 0, 0, 0, 0, 0 ) \
_(param,TAKE,0x2191/*up alt-y*/,    nnone, vhead,    vtake,      0, 0, 0, 0, 1, 0 ) \
_(param,DROP,0x2193/*down alt-u*/,  nnone, vbehead,  vdrop,      0, 0, 0, 0, 0, 0 ) \
_(param,COMP,0x001f,                nnone, mnone,    vcompress,  0, 0, 0, 0, 0, 0 ) \
_(param,EXP, 0x001e,                nnone, mnone,    vexpand,    0, 0, 0, 0, 0, 0 ) \
_(param,BASE,0x22a5/*alt-b*/,       nnone, mnone,    vbase,      0, 0, 0, 0, 0, 0 ) \
_(param,ENC, 0x22a4/*alt-n*/,       nnone, mnone,    vencode,    0, 0, 0, 0, 0, 0 ) \
_(param,ROT, 0x233d/*alt-%*/,       nnone, vreverse, vrotate,    0, 0, 0, 0, 0, 0 ) \
_(param,CONC,0x2282/*alt-z*/,       nnone, vconceal, dnone,      0, 0, 0, 0, 0, 0 ) \
_(param,REVL,0x2283/*alt-x*/,       nnone, vreveal,  dnone,      0, 0, 0, 0, 0, 0 ) \
_(param,NONE,0x2361/*alt-q*/,       nnone, vnoresult, vnoresultd,0, 0, 0, 0, 0, 0 ) \
_(param,BRNC,0x2192/*right alt-{*/, nnone, vbranch,  dnone,      0, 0, 0, 0, 0, 0 ) \
_(param,NIL, 0x2300/*alt-U*/,       vnil,  mnone,    dnone,      0, 0, 0, 0, 0, 0 ) \
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
#define DECLARE_VERB_FUNCTIONS(param,name, base, fnilad, fmonad, fdyad, ...) \
    nilad fnilad; \
    monad fmonad; \
    dyad fdyad;
VERBS_FOREACH(0,DECLARE_VERB_FUNCTIONS)
#undef nnone
#undef mnone
#undef dnone
#undef DECLARE_VERB_FUNCTIONS

extern object vtab[];
object ndel(verb v);
object mdel(object w, verb v);
object ddel(object a, object w, verb v);
object ndfn(verb v);
object mdfn(object w, verb v);
object ddfn(object a, object w, verb v);

void init_vb(symtab st);

#endif
