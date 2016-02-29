
/* stack type
   the size is generously pre-calculated
   and so we can skip all bounds checking.
   stkp->top is the size (index of next empty slot for next push)
   stkp->top-1 is the topmost element
 */
typedef struct stack { int top; object a[1];} stack; /* top==0::empty */
#define stackinit(stkp,sz) (stkp=malloc(sizeof*stkp + (sz)*sizeof*stkp->a)), \
                           (stkp->top=0)
#define stackpush(stkp,el) ((stkp)->a[(stkp)->top++]=(el))
#define stackpop(stkp) ((stkp)->a[--((stkp)->top)])
#define stacktop(stkp) ((stkp)->a[(stkp)->top-1])

/* predicate functions are instantiated according to the table
   defined in the header.
   the q[] function array is used by classify to apply all 
   predicate functions yielding a sum of all applicable codes
   defined in the table. Specific qualities or combinations 
   may then be determined easily by masking.
 */
#define PRED_FUNC(X,Y,...) int Y(object x){ return __VA_ARGS__; }
PREDTAB(PRED_FUNC)
#define PRED_ENT(X,Y,...) Y,
static int (*q[])(object) = { PREDTAB(PRED_ENT) };

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
#define PARSETAB(_) \
/*    p[0]      p[1]      p[2]      p[3]      func   pre x y z   post,2*/\
_(L0, EDGE,     VRB,      NOUN,     ANY,      monad,  3, 1,2,-1,   0,-1) \
_(L1, EDGE+AVN, VRB,      VRB,      NOUN,     monad, -1, 2,3,-1,   1, 0) \
_(L2, ANY,      NOUN,     DEX,      ANY,      monad,  3, 2,1,-1,   0,-1) \
_(L3, EDGE+AVN, NOUN,     VRB,      NOUN,     dyad,  -1, 1,2,3,    0,-1) \
_(L4, EDGE+AVN, NOUN+VRB, ADV,      ANY,      adv,    3, 1,2,-1,   0,-1) \
_(L5, ANY,      LEV,      NOUN+VRB, ANY,      adv,    3, 2,1,-1,   0,-1) \
_(L6, EDGE+AVN, NOUN+VRB, CONJ,     NOUN+VRB, conj_, -1, 1,2,3,    0,-1) \
_(L7, VAR,      ASSN,     AVN,      ANY,      spec,   3, 0,2,-1,  -1,-1) \
_(L8, LPAR,     ANY,      RPAR,     ANY,      punc,   3, 1,-1,-1, -1,-1) \
_(L9, MARK,     ANY,      RPAR,     ANY,      punc,   3, 1,-1,-1,  0,-1) \
_(L10,ANY,      LPAR,     ANY,      NUL,      punc,   3, 2,-1,-1,  0,-1) \
/**/

// generate labels to coordinate table and execution
#define PARSETAB_INDEX(label, ...) label,
enum { PARSETAB(PARSETAB_INDEX) };

// create parsetab array of structs containing the patterns
#define PARSETAB_PAT(label, pat1, pat2, pat3, pat4, ...) \
    {pat1, pat2, pat3, pat4},
typedef struct parsetab { int c[4]; } parsetab;
static parsetab ptab[] = { PARSETAB(PARSETAB_PAT) };


// perform the grammar production, transforming the stack
#define PARSETAB_ACTION(label,p1,p2,p3,p4, func, pre,x,y,z,post,post2) \
    case label: { \
        if (pre>=0) stackpush(rstk,t[pre]); \
        stackpush(rstk,func(x>=0?t[x]:0,y>=0?t[y]:0,z>=0?t[z]:0,st)); \
        if (post>=0) stackpush(rstk,t[post]); \
        if (post2>=0) stackpush(rstk,t[post2]); \
    } break;


void init_stacks(stack **lstkp, stack **rstkp, array e, int n);
object extract_result_and_free_stacks(stack *lstk, stack *rstk);
int parse_and_lookup_name(stack *lstk, stack *rstk, object x, symtab st);
int check_pattern(int *c, parsetab *ptab, int i);
void move_top_four_to_temp(object *t, stack *rstk);
size_t sum_symbol_lengths(array e, int n);

int monad(int f, int y, int dummy, symtab st);
int dyad(int x, int f, int y, symtab st);
int adv(int f, int g, int dummy, symtab st);
int conj_(int f, int g, int h, symtab st);
int spec(int name, int v, int dummy, symtab st);
int punc(int x, int dummy, int dummy2, symtab st);
