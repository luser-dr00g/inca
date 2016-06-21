#ifndef VERBS_H_
#define VERBS_H_
#include "common.h"

#define VERBS_FOREACH(param,_) \
/*      name base                   nilad, monad     dyad        f  g  h  mr lr rr mdesc ddesc*/ \
_(param,PLUS,'+',                   nnone, vid,      vplus,      0, 0, 0, 0, 0, 0, \
        identity, add) \
_(param,SUB, '-',                   nnone, vneg,     vminus,     0, 0, 0, 0, 0, 0, \
        negate/negative, subtract) \
_(param,SUB2,0x00af,                nnone, vneg,     vminus,     0, 0, 0, 0, 0, 0, \
        negative/negate, subtract) \
_(param,MUL, 0x00d7,                nnone, vsignum,  vtimes,     0, 0, 0, 0, 0, 0, \
        sign of, multiply) \
_(param,MUL2,'*',                   nnone, vsignum,  vtimes,     0, 0, 0, 0, 0, 0, \
        sign of, multiply) \
_(param,DIV, 0x00f7,                nnone, vrecip,   vdivide,    0, 0, 0, 0, 0, 0, \
        reciprocal, divide) \
_(param,POW, 0x22c6/*alt-p*/,       nnone, mnone,    vpow,       0, 0, 0, 0, 0, 0, \
        none, power)\
_(param,MOD, '|',                   nnone, vabs,     vresidue,   0, 0, 0, 0, 0, 0, \
        absolute value, residue) \
_(param,EQ,  '=',                   nnone, mnone,    veq,        0, 0, 0, 0, 0, 0, \
        none, compare for equality) \
_(param,NE,  0x2260,                nnone, mnone,    vne,        0, 0, 0, 0, 0, 0, \
        none, compare for inequality) \
_(param,RHO, 0x2374/*rho alt-r*/,   nnone, vshapeof, vreshape,   0, 0, 0, 0, 0, 0, \
        yield dimension vector, new array with specified dimensions populated by elements from right array) \
_(param,RHO2,'$',                   nnone, vshapeof, vreshape,   0, 0, 0, 0, 0, 0, \
        yield dimension vector, new array with specified dimensions populated by elements from right array) \
_(param,TAL, '#',                   nnone, vtally,   dnone,      0, 0, 0, 0, 0, 0, \
        number of items, none) \
_(param,IOTA,0x2373/*iota alt-i*/,  nnone, viota,    dnone,      0, 0, 0, 0, 0, 0, \
        index generator, none) \
_(param,CAT, ',',                   nnone, vravel,   vcat,       0, 0, 0, 0, 0, 0, \
        row-major-ordered vector of, catenate two arrays into vector) \
_(param,LINK,';',                   nnone, vprenul,  vlink,      0, 0, 0, 0, 0, 0, \
        ?, cat and enclose) \
_(param,INDR,'{',                   nnone, mnone,    vindexright,0, 0, 0, 0, 0, 0, \
        none, right is data and left is indices) \
_(param,INDL,'}',                   nnone, mnone,    vindexleft, 0, 0, 0, 0, 0, 0, \
        none, left is data and right is indices) \
_(param,TAKE,0x2191/*up alt-y*/,    nnone, vhead,    vtake,      0, 0, 0, 0, 1, 0, \
        first element, first n elements) \
_(param,DROP,0x2193/*down alt-u*/,  nnone, vbehead,  vdrop,      0, 0, 0, 0, 0, 0, \
        all but the first, all but first n elements) \
_(param,COMP,0x001f,                nnone, mnone,    vcompress,  0, 0, 0, 0, 0, 0, \
        none, select from right according to bools in left) \
_(param,EXP, 0x001e,                nnone, mnone,    vexpand,    0, 0, 0, 0, 0, 0, \
        none, accumulate from right or zeros according to bools in left) \
_(param,BASE,0x22a5/*alt-b*/,       nnone, mnone,    vbase,      0, 0, 0, 0, 0, 0, \
        none, interpret vector right using base left) \
_(param,ENC, 0x22a4/*alt-n*/,       nnone, mnone,    vencode,    0, 0, 0, 0, 0, 0, \
        none, produce encoded vector of value right according to base left) \
_(param,ROT, 0x233d/*alt-%*/,       nnone, vreverse, vrotate,    0, 0, 0, 0, 0, 0, \
        reverse order of elements, rotate through elements) \
_(param,CONC,0x2282/*alt-z*/,       nnone, vconceal, dnone,      0, 0, 0, 0, 0, 0, \
        encode array into simple scalar, none) \
_(param,REVL,0x2283/*alt-x*/,       nnone, vreveal,  dnone,      0, 0, 0, 0, 0, 0, \
        decode scalar into concealed array, none) \
_(param,NONE,0x2361/*alt-q*/,       nnone, vnoresult, vnoresultd,0, 0, 0, 0, 0, 0, \
        for testing, for testing) \
_(param,BRNC,0x2192/*right alt-{*/, nnone, vbranch,  dnone,      0, 0, 0, 0, 0, 0, \
        in del functions transfer to specified line, none) \
_(param,NIL, 0x2300/*alt-U*/,       vnil,  mnone,    dnone,      0, 0, 0, 0, 0, 0, \
        none, none) \
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


#define VERBTAB_ENUM(param,name, ...) \
    VERB_ ## name,
enum { VERBS_FOREACH(0,VERBTAB_ENUM) VERB_NOOP };
extern object vtab[VERB_NOOP];
// yield verb from verbtab given enum short name
#define VT(x) getptr(vtab[VERB_##x])

object ndel(verb v);
object mdel(object w, verb v);
object ddel(object a, object w, verb v);
object ndfn(verb v);
object mdfn(object w, verb v);
object ddfn(object a, object w, verb v);

void init_vb(symtab st);

#endif
