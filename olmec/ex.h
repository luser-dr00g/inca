
#define PREDTAB(_) \
_( ANY  =    1, qa, 1 ) \
_( VAR  =    2, qp, gettag(x)==PROG \
                || (gettag(x)==PCHAR && getval(x)!=0x2190 /*leftarrow*/ \
                    && getval(x)!='(' && getval(x)!=')' ) ) \
_( NOUN =    4, qn, gettag(x)==LITERAL \
                 || gettag(x)==CHAR \
                 || gettag(x)==ARRAY ) \
_( VRB  =    8, qv, gettag(x)==VERB ) \
_( DEX  =   16, qx, 0 ) /*dextri-monadic verb*/\
_( ADV  =   32, qo, gettag(x)==ADVERB && ((verb)getptr(x))->monad ) \
_( LEV  =   64, qe, 0 ) /*sinister adverb*/\
_( CONJ =  128, qj, gettag(x)==ADVERB && ((verb)getptr(x))->dyad ) \
_( MARK =  256, qm, gettag(x)==MARKOBJ ) \
_( ASSN =  512, qc, gettag(x)==CHAR && getval(x) == 0x2190 ) \
_( LPAR = 1024, ql, gettag(x)==LPAROBJ ) \
_( RPAR = 2048, qr, gettag(x)==RPAROBJ ) \
_( NUL  = 4096, qu, gettag(x)==NULLOBJ ) \
/**/
#define PRED_DECL(X,Y,...) int Y(int);
PREDTAB(PRED_DECL)
#define PRED_ENUM(X,...) X,
enum predicate { PREDTAB(PRED_ENUM) 
                 EDGE = MARK+ASSN+LPAR,
                 AVN = VRB+NOUN+ADV };

int ex(array e, symtab st);

