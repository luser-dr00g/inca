
/* predicate functions are instantiated according to the table
   defined in the ex.h and are also exported.
   the q[] function array is used by classify to apply all 
   predicate functions yielding a sum of all applicable codes
   defined in the table. Specific qualities or combinations 
   may then be determined easily by masking.
 */
#define DEFINE_PREDICATE_FUNCTION(enum_def,fname,...) \
    int fname(object x){ return __VA_ARGS__; }
PREDICATES_FOREACH(DEFINE_PREDICATE_FUNCTION)
#undef DEFINE_PREDICATE_FUNCTION

static int (*q[])(object) = {
#define PREDICATE_FUNCTION_NAME(enum_def,fname,...) fname,
    PREDICATES_FOREACH(PREDICATE_FUNCTION_NAME)
#undef PREDICATE_FUNCTION_NAME
};

// declare predicate enums and composed patterns
enum predicate {
#define PREDICATE_ENUMERATION(enum_def,...) enum_def,
    PREDICATES_FOREACH(PREDICATE_ENUMERATION) 
#undef PREDICATE_ENUMERATION
    EDGE = MARK+ASSN+LPAR,
    AVN = VRB+NOUN+ADV
};

/* encode predicate applications into a binary number
   which can be compared to a pattern with a mask */
static inline int classify(object x){
    int i,v,r;
    for (i=0, v=1, r=0; i<sizeof q/sizeof*q; i++, v*=2)
        if (q[i](x))
            r |= v;
    return r;
}

// the Parse Table defines the grammar of the language
// At each stack move, the top four elements of the right stack
// are checked against each of these patterns. A matching pattern
// returns element t[pre] from the temp area to the right stack
// then calls func(t[x],t[y],t[z]) and pushes the result to the
// right stack, then pushes t[post] and t[post2]. 
// A -1 in any of these positions means do nothing, or do not
// bother to pass anything meaningful. That is, any of the x,y,z
// parameters marked -1 correspond to a "dummy" argument of 
// the function which is there in order that all handler
// functions have the same signature.
//
// The table itself is transformed via macro-expansion into
// specific cases of a switch statement.
#define PARSE_PRODUCTIONS_FOREACH(_) \
/*    p[0]      p[1]      p[2]      p[3]      func     start finish */\
/*-->items[3]   it[2]     it[1]     it[0]                           */\
_(L0, EDGE,     MON,      NOUN,     ANY,      monadic,  2,    1 ) \
_(L1, EDGE+AVN, VRB,      MON,      NOUN,     monadic,  1,    0 ) \
_(L2, ANY,      NOUN,     DEX,      ANY,      monadic,  1,    2 ) \
_(L3, EDGE+AVN, NOUN,     DYA,      NOUN,     dyadic,   2,    0 ) \
_(L4, EDGE+AVN, NOUN+VRB, ADV,      ANY,      adv,      1,    2 ) \
_(L5, ANY,      LEV,      NOUN+VRB, ANY,      adv,      2,    1 ) \
_(L6, EDGE+AVN, NOUN+VRB, CONJ,     NOUN+VRB, conj_,    2,    0 ) \
_(L7, VAR,      ASSN,     AVN,      ANY,      spec,     3,    1 ) \
_(L8, LPAR,     ANY,      RPAR,     ANY,      punc,     3,    1 ) \
_(L9, MARK,     ANY,      RPAR,     ANY,      punc,     1,    2 ) \
_(L10,ANY,      LPAR,     ANY,      NUL,      punc,     2,    1 ) \
/**/

// generate labels to coordinate table and execution
enum {
#define PRODUCTION_LABEL(label, ...) label,
    PARSE_PRODUCTIONS_FOREACH(PRODUCTION_LABEL)
#undef PRODUCTION_LABEL
};

// create parsetab array of structs containing the patterns
static
struct parsetab { int c[4]; } ptab[] = {
#define PRODUCTION_PATTERNS(label, pat1, pat2, pat3, pat4, ...) \
    {pat1, pat2, pat3, pat4},
    PARSE_PRODUCTIONS_FOREACH(PRODUCTION_PATTERNS)
#undef PRODUCTION_PATTERNS
};

static int min(int x, int y){
    return x<y? x: y;
}

#define PRODUCTION_ELSEIFS(label, p1,p2,p3,p4, func, s, f) \
    else if (matches_ptab_pattern(items, label)) { \
        stack_prune(right, 4); \
        int dir = f-s>0 ? 1 : -1; \
        int n = 1+abs(f-s); \
        int minfs = min(f,s); \
        int excess = 4 - n - minfs; \
        DEBUG(0, "s=%d f=%d dir=%d, n=%d, minfs=%d, excess=%d\n", \
                s, f, dir, n, minfs, excess); \
        items[minfs] = \
            datum_to_stack_element( \
                func(items[s].datum, \
                    n>=2? items[s+dir].datum: 0, \
                    n>=3? items[s+2*dir].datum: 0, \
                    env) \
                ); \
        for (int i=0; i<excess; ++i){ \
            items[minfs+1+i] = items[minfs+i+n]; \
        } \
        stack_reclaim(right, excess+1+minfs); \
    }


object monadic(object f, object y, object dummy, symtab st);
object dyadic(object x, object f, object y, symtab st);
object adv(object f, object g, object dummy, symtab st);
object conj_(object f, object g, object h, symtab st);
object spec(object name, object v, object dummy, symtab st);
object punc(object x, object dummy, object dummy2, symtab st);


/* stack type
   the size is generously pre-calculated
   and so we can skip all bounds checking.
 */

typedef struct stack_element {
    int datum;
    int code;
} stack_element;

typedef struct stack {
    unsigned next;
    unsigned limit;
    stack_element elements[];
} *stack;

static
stack new_stack (unsigned n) {
    DEBUG(0, "new_stack(%u)\n", n);
    stack r = malloc(sizeof *r + n * sizeof r->elements[0]);
    return r->next = 0, r->limit = n, r;
}

static
void stack_release (stack s){
    free(s);
}

static
unsigned stack_capacity (stack s){
    return s->limit;
}

static
int stack_is_empty (stack s){
    return s->next == 0;
}

static
void stack_push (stack s, stack_element e){
    s->elements[ s->next++ ] = e;
}

static
stack_element datum_to_stack_element (int d){
    return (stack_element){ d, classify(d) };
}

static
void stack_push_datum (stack s, int d){
    stack_push(s, datum_to_stack_element(d));
}

static
stack_element stack_pop (stack s){
    return s->elements[ --s->next ];
}

static
unsigned stack_element_count (stack s){
    return s->next;
}

static
void stack_prune (stack s, unsigned n){
    s->next -= n;
}

static
void stack_reclaim (stack s, unsigned n){
    s->next += n;
}

static
stack_element *stack_top_elements_address (stack s, unsigned n){
    return s->elements + s->next - n;
}

static int is_pronoun(stack_element x);
static int is_assn(stack_element x);
static size_t sum_symbol_lengths(array e, int n);
static int parse_and_lookup_name(stack left, stack right, stack_element x, symtab st);
static stack new_left_stack_for (array expr);
static int matches_ptab_pattern (stack_element items[4], int i);
static int penultimate_prereleased_value (stack s);

