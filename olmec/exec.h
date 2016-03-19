
// predicate table contains predicate functions
// and associated enum values
#define PREDICATES_FOREACH(_) \
_( ANY  =     1, qany, 1 ) \
_( VAR  =     2, qprog, gettag(x)==PROG \
                || (gettag(x)==PCHAR && getval(x)!=0x2190 /*leftarrow*/ ) ) \
_( NOUN =     4, qnoun, gettag(x)==LITERAL \
                 || gettag(x)==CHAR \
                 || gettag(x)==ARRAY ) \
_( MON  =     8, qmon, (gettag(x)==VERB && ((verb)getptr(x))->monad) \
                 || (gettag(x)==XVERB && ((xverb)getptr(x))->verb->monad) ) \
_( DYA  =    16, qdya, (gettag(x)==VERB && ((verb)getptr(x))->dyad) \
                 || (gettag(x)==XVERB && ((xverb)getptr(x))->verb->dyad) ) \
_( VRB  =    32, qverb, qmon(x) || qdya(x) ) \
_( DEX  =    64, qdex, 0 ) /*dextri-monadic verb*/\
_( ADV  =   128, qadv, (gettag(x)==ADVERB && ((verb)getptr(x))->monad) \
                 || (gettag(x)==XVERB && ((xverb)getptr(x))->adverb->monad) ) \
_( LEV  =   256, qlev, 0 ) /*sinister adverb*/\
_( CONJ =   512, qconj, (gettag(x)==ADVERB && ((verb)getptr(x))->dyad) \
                 || (gettag(x)==XVERB && ((xverb)getptr(x))->adverb->dyad) ) \
_( MARK =  1024, qmark, gettag(x)==MARKOBJ ) \
_( ASSN =  2048, qassn, gettag(x)==PCHAR && getval(x) == 0x2190 ) \
_( LPAR =  4096, qlpar, gettag(x)==LPAROBJ ) \
_( RPAR =  8192, qrpar, gettag(x)==RPAROBJ ) \
_( NUL  = 16384, qnull, gettag(x)==NULLOBJ ) \
/**/

// declare predicate functions
#define DECLARE_PREDICATE_FUNCTION(enum_def,fname,...) int fname(object);
PREDICATES_FOREACH(DECLARE_PREDICATE_FUNCTION)

extern int last_was_assn;
// execute an expression e with environment st
object execute_expression(array expr, symtab st);

