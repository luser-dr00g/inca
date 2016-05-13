
// predicate table contains predicate functions
// and associated enum values
#define PREDICATES_FOREACH(_) \
_( ANY   =     1, qany, 1 ) \
_( VAR   =     2, qprog, gettag(x)==PROG \
                 || (gettag(x)==PCHAR && getval(x)!=0x2190 /*leftarrow*/ ) ) \
_( NOUN  =     4, qnoun, gettag(x)==LITERAL \
                  || gettag(x)==CHAR \
                  || gettag(x)==ARRAY ) \
_( NIL   =     8, qnil, (gettag(x)==VERB && ((verb)getptr(x))->nilad) ) \
_( MON   =    16, qmon, (gettag(x)==VERB && ((verb)getptr(x))->monad) \
                  || (gettag(x)==XVERB && ((xverb)getptr(x))->verb->monad) ) \
_( DYA   =    32, qdya, (gettag(x)==VERB && ((verb)getptr(x))->dyad) \
                  || (gettag(x)==XVERB && ((xverb)getptr(x))->verb->dyad) ) \
_( VRB   =    64, qverb, qmon(x) || qdya(x) ) \
_( DEX   =   128, qdex, 0 ) /*dextri-monadic verb*/\
_( ADV   =   256, qadv, (gettag(x)==ADVERB && ((verb)getptr(x))->monad) \
                  || (gettag(x)==XVERB && ((xverb)getptr(x))->adverb->monad) ) \
_( LEV   =   512, qlev, 0 ) /*sinister adverb*/\
_( CONJ  =  1024, qconj, (gettag(x)==ADVERB && ((verb)getptr(x))->dyad) \
                  || (gettag(x)==XVERB && ((xverb)getptr(x))->adverb->dyad) ) \
_( MARK  =  2048, qmark, gettag(x)==MARKOBJ ) \
_( ASSN  =  4096, qassn, gettag(x)==PCHAR && getval(x) == 0x2190 ) \
_( LPAR  =  8192, qlpar, gettag(x)==LPAROBJ ) \
_( RPAR  = 16384, qrpar, gettag(x)==RPAROBJ ) \
_( LBRAC = 32768, qlbrac, gettag(x)==LBRACOBJ ) \
_( RBRAC = 65536, qrbrac, gettag(x)==RBRACOBJ ) \
_( NUL  = 131072, qnull, gettag(x)==NULLOBJ ) \
_( SEMI = 262144, qsemi, gettag(x)==PCHAR && getval(x) == ';' ) \
/**/

// declare predicate functions
#define DECLARE_PREDICATE_FUNCTION(enum_def,fname,...) int fname(object);
PREDICATES_FOREACH(DECLARE_PREDICATE_FUNCTION)

// execute an expression e with environment st
object execute_expression(array expr, symtab env, int *plast_was_assn);

