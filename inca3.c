#include<limits.h>
#include<math.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<termios.h>
#include<unistd.h>


/* Basic types */
typedef unsigned char C;
typedef intptr_t      I;
typedef uintptr_t     U;
typedef double        D;
typedef struct a{I t, r, n, k, d[1];}*A; /* The abstract array header */
#define AT(a) ((a)->t)                   /* Type (takes values from enum type) */
#define AR(a) ((a)->r)                   /* Rank (size of Dims) */
#define AN(a) ((a)->n)                   /* Number of values in ravel */
#define AK(a) ((a)->k)                   /* Offset of ravel */
#define AD(a) ((a)->d)                   /* Dims */
#define AV(a) ((I*)(((C*)(a))+AK(a)))    /* Values in ravelled order */
enum type { NLL, INT, BOX, SYMB, CHAR, NUM, DBL, MRK, VRB, NTYPES };
#define TYPEPAIR(a,b) ((a)*NTYPES+(b))

/* "singleton" objects */
struct a nullob = { NLL };
A null = &nullob;
struct a markob = { MRK };
A mark = &markob;
I infinite; /* result for function arguments not in function domain */

A newsymb(C *s,I n,I state);  /* symbol constructor */
struct st *findsymb(struct st *st, char **s, int mode); /* symbol lookup */

/* Idioms */
#define P printf
#define R return
#define V1(f) A f(A w,      A self)  /* monadic verb signature */
#define V2(f) A f(A a, A w, A self)  /* dyadic verb signature */
#define DO(n,x)  {I i=0,_n=(n);for(;i<_n;++i){x;}}
#define DO2(n,x) {I j=0,_o=(n);for(;j<_o;++j){x;}}
#define DO3(n,x) {I k=0,_p=(n);for(;k<_p;++k){x;}}


/* Special ascii control-code macros
   MODE1() defines the internal "base" representation for non-ascii APL symbols,
   which just sets the top bit.
 */
#define ESC(x) "\x1b" #x
#define ESCCHR '\x1b'
#define CTL(x) (x-64)
#define EOT 004
#define DEL 127
#define MODE1(x) (x|1<<7)
#define MODE2(x) (x-32)

#include "ialf3.h" /* define character translation table alphatab[] */

/* convert input character to internal representation */
int inputtobase (int c, int mode){ int i;
    for (i=0;i<(sizeof alphatab/sizeof*alphatab);i++)
        if (c==*alphatab[i].input && mode==alphatab[i].ext)
            return alphatab[i].base;
    return mode? MODE1(c): c;
    //return c | mode << 7;
}
/* convert internal representation to output representation */
char *basetooutput (int c){ int i;
    for (i=0;i<(sizeof alphatab/sizeof*alphatab);i++)
        if (c==alphatab[i].base)
            return alphatab[i].output;
    return "";
}


struct termios tm; /* terminal settings struct to save default settings */

/* setup special terminal mode for line editor:
   turn off canonical mode for char-at-a-time processing
   select vt220 G2 and G3 character sets for display of extra glyphs
 */
void specialtty(){
    tcgetattr(0,&tm);

/*https://web.archive.org/web/20060117034503/\
http://www.cs.utk.edu/~shuford/terminal/xterm_codes_news.txt*/
    //fputs("\x1B""*0\n",stdout);
#if 0
    //experiment with line-drawing chars
    fputs(ESC(*0),stdout);
    fputs(ESC(n)
            "lqqqqqk\n"
            "x"
      ESC(o)"a box"ESC(n)
                  "x\n"
            "mqqqqqj\n"
            ESC(o)"\n", stdout);
#endif

#if 0
    //show the various alternate charsets available in xterm vt220 mode
    fputs("\x1B*0\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    //these 2 are not interesting:
    //fputs("\x1B*1\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    //fputs("\x1B*2\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    fputs("\x1B*A\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
    fputs("\x1B*B\x1Bn",stdout); DO('~'-' ',P("%c",' '+i))P("\x1Bo\n");
#endif

#if 0
    //show special char and line-drawing set as keyboard layout
    fputs(ESC(*0)ESC(n),stdout);
    fputs( "~!@#$%^&*()_+" "\n" "`1234567890-="  "\n"
           "QWERTYUIOP{}|" "\n" "qwertyuiop[]\\" "\n"
           "ASDFGHJKL:\""  "\n" "asdfghjkl;'"    "\n"
           "ZXCVBNM<>?"    "\n" "zxcvbnm,./"     "\n" , stdout);
        fputs(ESC(o),stdout);
#endif

/* Select vt220 character sets */

    fputs(ESC()")B",stdout); //set G1 charset to B : usascii
    fputs(ESC(*0),stdout); //set G2 charset to 0 : special char and line drawing set ESC(n)
    fputs(ESC(+A),stdout); //set G3 charset to A : "uk" accented and special chars ESC(o)
    fputc(CTL('N'),stdout); //select G1 charset  ESC(n):select G2  ESC(o):select G3

#if 1
    { /* print ALT-keyboard layout */
        int i,j;
        char *keys[] = {
           "~!@#$%^&*()_+",
           "`1234567890-=",
           "QWERTYUIOP{}|",
           "qwertyuiop[]\\",
           "ASDFGHJKL:\"",
           "asdfghjkl;'",
           "ZXCVBNM<>?",
           "zxcvbnm,./"
        };
        for (i=0;i<(sizeof keys/sizeof*keys);i++){
            int n = strlen(keys[i]);
            for (j=0;j<n;j++)                          /* V- mode=1 */
                fputs(basetooutput(inputtobase(keys[i][j],1)),stdout);
            fputc('\n',stdout);
        }
    }
#endif

/*
   Set special terminal mode.
   * No echo. (we'll control what you see)
   * No echo of newline. (we'll control what you say)
   * Ignore XON/XOFF semantics. (so ctrl-s and ctrl-q are available)
   */
    { struct termios tt=tm; //man termios
        //cfmakeraw(&tt);
        tt.c_iflag &= ~(IGNBRK | /*BRKINT |*/
                PARMRK | ISTRIP | /*INLCR | IGNCR | ICRNL |*/ IXON);
        /*tt.c_oflag &= ~OPOST;*/
        tt.c_lflag &= ~(ECHO | /*ECHONL |*/ ICANON /*| ISIG | IEXTEN*/);
        tt.c_cflag &= ~(CSIZE | PARENB);
        tt.c_cflag |= CS8;
        tcsetattr(0,TCSANOW,&tt); }
}
/* restore saved default terminal settings */
void restoretty(){ tcsetattr(0,TCSANOW,&tm); }



/* read expression from terminal into buffer
   (re)allocate buffer as necessary
   process backspaces
   process mode changes and alt key.

   pass characters through the translation table,
       storing the "base" form, echoing the "output" form.
 */
C * getln(C *prompt, C **s, int *len){
    int mode = 0;
    int tmpmode = 0;
    C *p;
    if (prompt) fputs(prompt,stdout);
    if (!*s) *s = malloc(*len=256);
    p = *s;
    while(1){
        int c;
        if (p-*s>*len){
            C *t = realloc(*s,*len*=2);
            if (t) *s=t;
            else { *len/=2; R NULL; }
        }
        c = fgetc(stdin);
        switch(c){
        case EOF:
        case EOT: if (p==*s) goto err;
                  break;
        case ESCCHR:
                  c = fgetc(stdin);
                  switch(c){
                  default:
                      tmpmode = 1;
                      goto storechar;
                      break;
#if 0
/*
TODO: learn how the arrow keys can be distinguished from alt-[
 */
                  case '[':
                      c = fgetc(stdin);
                      switch(c){
                      default:
                          *p++ = MODE1('[');
                          fputs(basetooutput(MODE1('[')),stdout);
                          ungetc(c,stdin);
                          break;
                      case 'Z':
                          c = '\v';       //convert shift-TAB to vertical tab
                          *p++ = c;                    // save base in string
                          fputs(basetooutput(c),stdout);  // echo output form
                          break;
                      }
                      break;
#endif
                  }
                  break;
        case '\n':
                  fputc('\n',stdout);
                  *p++ = c;
                  goto breakwhile;
        case CTL('N'): mode = !mode; tmpmode = 0; break;
        case CTL('U'): 
                       while(p>*s){
                           fputs("\b \b",stdout);
                           --p;
                       }
                       tmpmode = 0;
                       break;
        case '\b':
        case DEL:
                   fputs("\b \b",stdout);
                   if (p!=*s) --p;
                   break;
        default:
storechar:
                 c = inputtobase(c,mode|tmpmode);    // convert to internal "base" form
                 *p++ = c;                               // save base in string
                 tmpmode = 0;
                 fputs(basetooutput(c),stdout);             // echo output form
                 break;
        }
    }
breakwhile:
    *p++ = 0;
err:
    return p==*s?NULL:*s;
}



/* allocate integer array */
I *ma(I n){R(I*)calloc(n,sizeof(I));}

/* move integers */
void mv(I*d,I*s,I n){DO(n,d[i]=s[i]);}

/* table rank, product of dimensions d[0..r-1] */
I tr(I r,I*d){I z=1;DO(r,z=z*d[i]);R z;}

/* generate (allocate and initialize) new abstract array 
   of type t */
A ga(I t,I r,I*d){I n;A z=(A)ma(sizeof*z+r+(n=tr(r,d)));
    AT(z)=t;AR(z)=r;AN(z)=n;AK(z)=sizeof*z+(-1+AR(z))*sizeof(I);
    mv(AD(z),d,r);R z;}

/* integer scalar */
A i0(I i){A z=ga(INT,0,0);*AV(z)=i;R z;}

/* integer vector */
A i1(I n,I*v){A z=ga(INT,1,&n);mv(AV(z),v,n);R z;}



/* integer-encoded numbers in the NUM type */
enum {
    IMM_BIT = 16,
    IMM_MASK = (1<<IMM_BIT)-1,
    IMM_SIGN = 1<<(IMM_BIT-1),
    BANK_BIT = sizeof(I)*CHAR_BIT - IMM_BIT,
    BANK_MASK = ((1<<BANK_BIT)-1) << IMM_BIT,
};
#define encodenum(bnk,idx) ((bnk<<IMM_BIT)|idx)

A bank; /* the top-level number bank */
#define BANK_INIT (bank=ga(BOX,1,(I[]){1<<BANK_BIT})), \
                  (AV(bank)[0]=0)
A fixnum; /* current full-width integer allocation block */
#define FIXNUM_INIT (fixnum=(A) (AV(bank)[ ++AV(bank)[0] ]=(I)ga(INT,1,(I[]){1<<IMM_BIT}))), \
                    (AV(fixnum)[0]=0)
A flonum; /* current floating point number allocation block */
#define FLONUM_INIT (flonum=(A) (AV(bank)[ ++AV(bank)[0] ]=(I)ga(DBL,1,(I[]){1<<IMM_BIT}))), \
                    (((D*)AV(flonum))[0]=0.0)

/* search full-width integer in bank[FIXNUM]
   allocate if not found
   return encoded (bank,index) */
I fix(I i){
    int j,n;
    for(j=0,n=AV(fixnum)[0];++j<=n;)
        if (AV(fixnum)[j]==i)
            R encodenum(1,j);
    //TODO: check for full table
    // allocate new fixnum table in bank,
    // update global fixnum pointer
    // use a cursor variable here instead of constant 1
    AV(fixnum)[j]=i;
    ++AV(fixnum)[0];
    R encodenum(1,j);
}

/* search floating number in bank[FLONUM]
   allocate if not found
   return encoded(bank,index) */
I flo(D d){
    int j,n;
    for(j=0,n=(I)(((D*)AV(flonum))[0]);++j<=n;)
        if (((D*)AV(flonum))[j]==d)
            R encodenum(2,j);
    //TODO: check for full table as in fix()
    ((D*)AV(flonum))[j]=d;
    ++((D*)AV(flonum))[0];
    R encodenum(2,j);
}

/* "number"-encoded integer */
I num(I i){
    if ((i)&(BANK_MASK|IMM_SIGN)) {
        if (((i)&(BANK_MASK|IMM_SIGN))==(BANK_MASK|IMM_SIGN)) {
            R i&IMM_MASK;           //small negative number
        }
        else
            R fix(i);
    } else 
        R i;
}

/* number scalar */
A num0(I i){ A z=i0(num(i)); AT(z)=NUM; R z; }
/* "raw" number scalar. don't overcook. */
A num0r(I i){ A z=i0(i); AT(z)=NUM; R z;}
/* floating number scalar */
A num0f(D d){ A z=i0(flo(d)); AT(z)=NUM; R z; }

/* convert immediate num to full-width integer */
I numimm(I n){ R n&IMM_SIGN?n|BANK_MASK:n; }

/* fetch full-width integer from bank */
I numint(I n){
    if ((n&BANK_MASK)==0) R numimm(n);
    R AV((A)(AV(bank)[((unsigned)n&BANK_MASK)>>IMM_BIT]))[n&IMM_MASK];
}

/* fetch floating-point number from bank */
D numdbl(I n){
    R ((D*)AV((A)(AV(bank)[((unsigned)n&BANK_MASK)>>IMM_BIT])))[n&IMM_MASK];
}


/* Multiprecision Integers
   MPIs are a subtype of NUM (as are FIX FLO and IMM).
   They are allocated in the mpint bank which is a BOX array.
   Each element (the mpi data) is an INT array.
   Numbers are stored in a sign-magnitude representation.
   The top bit of the first int is the sign bit. 
   The base must therefore be <= 2^31.
   The implementation can use either power-of-2 or power-of-10 bases.
   Until a general radix-conversion is written for the output, 
   the base 10000 is used so the output routine can take shortcuts.
 */

#define MPI_SIGN_BIT (1U<<31)
#ifdef MPI_POWER_OF_2
#   define MPI_BITS 31
#   define MPI_BASE (1U<<MPI_BITS)
#else
#   define MPI_BASE 10000
#endif

#define MPI_MAX (MPI_BASE-1)
#define MPI_SIGN(x) (!!(AV(x)[0]&MPI_SIGN_BIT))
#define SET_MPI_SIGN(x) (AV(x)[0]|=MPI_SIGN_BIT)
#define CLEAR_MPI_SIGN(x) (AV(x)[0]&=~MPI_SIGN_BIT) /*(AV(x)[0]&=MPI_MAX)*/

A mpint;
I mpzero;
#define MPINT_INIT (mpint=(A) (AV(bank)[ ++AV(bank)[0] ]=(I)ga(BOX,1,(I[]){1<<IMM_BIT}))), \
                    (AV(mpint)[0]=0), \
                    (mpzero=mpi(0))

/* construct new multiprecision int of length n.
   return encoded bank:index.
TODO: check for full table as in fix()
 */
I newmpint(I n){
    I j = ++AV(mpint)[0];
    AV(mpint)[j]=(I)ga(INT,1,(I[]){n});
    R encodenum(3,j);
}

/* yield the array representation of mpi designated n */
A numbox(I n){
    R (A)AV((A)(AV(bank)[((unsigned)n&BANK_MASK)>>IMM_BIT]))[n&IMM_MASK];
}

/* allocate a larger array, left-padded with 0, containing the same value as x,
   assumed unsigned.
note: does not allocate a slot in the mpint bank.
 */
A promote(A x,I n){
    A z=ga(INT,1,(I[]){n});
    DO(n-AN(x),AV(z)[i]=0)
    DO(AN(x),AV(z)[i+(n-AN(x))]=AV(x)[i])
    R z;
}

/*
   construct a multiprecision int with (signed) value u.
   return encoded bank:index.
 */
I mpi(U u){
    I sign = !!(u&MPI_SIGN_BIT);
    if (sign) u=(~u)+1; //unsigned 2's-comp negate
#ifdef MPI_POWER_OF_2
    I r=newmpint(2);
    A z=numbox(r);
    AV(z)[0]=u>>MPI_BITS;
    AV(z)[1]=u&MPI_MAX;
#else
    I n=u?ceil(ceil(log10((D)u))/floor(log10((D)MPI_MAX))):1;
    if(!n)n=1;
    I r=newmpint(n);
    A z=numbox(r);
    I t;
    DO(n, AV(z)[(AN(z)-1)-i]=u%MPI_BASE; u/=MPI_BASE)
    //AV(z)[0]=i/MPI_BASE;
    //AV(z)[1]=i%MPI_BASE;
#endif
    if (sign) SET_MPI_SIGN(z);
    R r;
}

I mpi_lt_mag(A a, A w){ // is a smaller than w?
    if(AN(a)<AN(w)) R 1;
    if(AN(a)>AN(w)) R 0;
#ifdef MPI_POWER_OF_2
    DO(AN(w),if((AV(a)[i]&MPI_MAX)>(AV(w)[i]&MPI_MAX))R 0)
#else
    DO(AN(w),if((AV(a)[i]&~MPI_SIGN_BIT)>(AV(w)[i]&~MPI_SIGN_BIT))R 0)
#endif
    R 1;
}

/* perform math function op on mpi x and mpi y, yielding new mpi result.
   allocates a new mpi value in the mpint bank.
 */
I mpop(I x,C op,I y){
    A a,w,z;
    I r;
    I b=MPI_BASE;
    a=numbox(x);
    w=numbox(y);

    I n=AN(a);
    switch(op){ // handle sizes
    case '+': case '-':
        if (n<AN(w)){ a=promote(a,AN(w)); n=AN(w); }
        if (n>AN(w)){ w=promote(w,n); }
    }

    I signa=MPI_SIGN(a);  // save and remove signs from data representation
    CLEAR_MPI_SIGN(a);
    I signw=MPI_SIGN(w);
    CLEAR_MPI_SIGN(w);
    I signz=signa;

    switch(op){ // handle signs
    case '+':
        if (signa!=signw){
            op='-';
            if (mpi_lt_mag(a,w)){
                A t=w;w=a;a=t;
                I tsign=signw;signw=signa;signa=tsign;
                signz=signa;
            }
        }
        break;
    case '-':
        if (signa!=signw){
            signz=!signw;
            op='+';
        } else if (mpi_lt_mag(a,w)){
            A t=w;w=a;a=t;
            I tsign=signw;signw=signa;signa=tsign;
            signz=!signz;
        }
        break;
    case '*':
    case '/':
        signz=signa!=signw;
        break;
    }

    switch(op){           // unsigned multiprecision arithmetic
    case '+':
        r=newmpint(n+1);
        z=numbox(r);
        {
            int j=n,k=0;
            while(j){
                U t= (U)AV(a)[j-1] + (U)AV(w)[j-1] + (U)k;
#ifdef MPI_POWER_OF_2
                AV(z)[j]= t&MPI_MAX;
                k= !!(t&MPI_BASE);
#else
                AV(z)[j]= t%MPI_BASE;
                k= t/MPI_BASE;
#endif
                --j;
            }
            if (k) AV(z)[0]=k;
            else {
                AK(z)+=sizeof(I); //don't keep adding zeros on the left
                --AN(z);
                --AD(z)[0];
            }
        }
        break;

    case '-':
        r=newmpint(n);
        z=numbox(r);
        {
            int j=n,k=0;
            while(j){
                U t= ((U)AV(a)[j-1] - (U)AV(w)[j-1]) - (U)k;
#ifdef MPI_POWER_OF_2
                AV(z)[j-1]= t&MPI_MAX;
                k= !!(t&MPI_BASE);
#else
                AV(z)[j-1]= t%MPI_BASE;
                k= t/MPI_BASE;
#endif
                --j;
            }
        }
        break;

    case '*':
        r=newmpint(n+AN(w)); /* m+n*/
        z=numbox(r);
        DO(AN(z),AV(z)[i]=0)
        {
            I j=n; /* m ==AN(a) */
            while(j){
                if (AV(w)[j-1]==0){ AV(z)[j-1]=0; } else {
                    U i=AN(w), /* n */
                      k=0;
                    while (i>0){
                        uint64_t t = AV(a)[i-1] * AV(w)[j-1] + AV(z)[(i+j)-1] + k;
#ifdef MPI_POWER_OF_2
                        AV(z)[(i+j)-1]= t&MPI_MAX;
                        k= t>>MPI_BITS;
#else
                        AV(z)[(i+j)-1]= t%MPI_BASE;
                        k= t/MPI_BASE;
#endif
                        --i;
                    }
                }
                --j;
            }
        }
        break;

    case '/':
        {
            I allzero=1;
            DO(AN(a),if((AV(a)[i]&~MPI_SIGN_BIT)==0)continue;allzero=0;)
            if (allzero) 
                R 0;
            allzero=1;
            DO(AN(w),if((AV(w)[i]&~MPI_SIGN_BIT)==0)continue;allzero=0;)
            if (allzero)
                R infinite;
        }
        break;
    }

    if (signa) SET_MPI_SIGN(a);  // restore signs to the data representation
    if (signw) SET_MPI_SIGN(w);
    if (signz) SET_MPI_SIGN(z);
    R r;
}



/* verb function declarations */
V1(copy);
V1(iota); V2(find);
V2(match);
V1(id); V2(plus);
V1(neg); V2(minus); V2(plusminus);
V2(times);
V2(quotient);
V1(size); V2(from);
V1(sha); V2(rsh);
V1(box);
V2(cat);
V2(boxcat);

V1(reduce);

/*
   The verb table. The VERBNAME symbolically indexes a
   single functional symbol which has an associated 
   ALPHATAB name and associated functions for monadic
   (single right argument) or dyadic (left and right args) uses.
   Verbs are recognized by the wd() function by being non-whitespace
   non-alphanumeric and then refined by verb() called by newsymb().
   The verb's A representation is a small integer which indexes
   this table or an array of type VRB whose value is a (possibly
   modified) copy of the verb record.
 */
/*         VERBNAME   ALPHA_NAME       vm       vd         f  g  k  m  n  id */
#define VERBTAB(_) \
        _( ZEROFUNC,  0,               0,       0,         0, 0, 0, 0, 0,  0 ) \
        _( PLUS,      ALPHA_PLUS,      id,      plus,      0, 0, 0, 0, 0,  0 ) \
        _( MINUS,     ALPHA_MINUS,     neg,     minus,     0, 0, 0, 0, 0,  0 ) \
        _( TIMES,     ALPHA_TIMES,     0,       times,     0, 0, 0, 0, 0,  1 ) \
        _( DIVIDE,    ALPHA_COLONBAR,  0,       quotient,  0, 0, 0, 0, 0,  1 ) \
        _( PLUSMINUS, ALPHA_PLUSMINUS, neg,     plusminus, 0, 0, 0, 0, 0,  0 ) \
        _( RBRACE,    ALPHA_RBRACE,    size,    from,      0, 0, 0, 0, 0,  0 ) \
        _( IOTA,      ALPHA_IOTA,      iota,    find,      0, 0, 0, 0, 0,  0 ) \
        _( BOXF,      ALPHA_LANG,      box,     0,         0, 0, 0, 0, 0,  0 ) \
        _( RHO,       ALPHA_RHO,       sha,     rsh,       0, 0, 0, 0, 0,  0 ) \
        _( COMMA,     ALPHA_COMMA,     0,       cat,       0, 0, 0, 0, 0,  0 ) \
        _( SEMICOLON, ALPHA_SEMICOLON, box,     boxcat,    0, 0, 0, 0, 0,  0 ) \
        _( SLASH,     0,               0,       0,         0, 0, 0, 0, 0,  0 ) \
        _( NULLFUNC,  0,               0,       0,         0, 0, 0, 0, 0,  0 ) 
struct v { I c; A (*vm)(); A (*vd)(); I f,g,k,m,n; I id; };
typedef struct v *V; //dynamic verb type
#define VERBTAB_NAME(a, ...) a ,
enum { VERBTAB(VERBTAB_NAME) };     //generate verb symbols

#define VERBTAB_ENT(a, ...) { __VA_ARGS__ },
struct v vt[] = { VERBTAB(VERBTAB_ENT) };  //generate verb table array


/* adverb function declarations */
V1(withl);
V1(withr);
V1(on1);
V2(on2);
V2(amp);
V2(rank);
V1(areduce);

/*
   The adverb table uses the same struct as a verb but is 
   separated to better distinguish the two classes of object.
   The adverb's A representation is a small integer, biased by
   the range of verb codes, which indexes this table. (Adverbs it
   seems do not exist as dynamic entities, since the application
   of an adverb produces a derived *verb*; thus there is no such
   thing as a derived adverb (as defined in J and APL87).)
 */
/*   ADVNAME  ALPHA_NAME       vm       vd    f  g  k  m  n id) */
#define ADVTAB(_) \
    _(ZEROOP=NULLFUNC+1, 0,    0,       0,    0, 0, 0, 0, 0, 0) \
    _(WITH,   ALPHA_AMPERSAND, 0,       amp,  0, 0, 0, 0, 0, 0) \
    _(RANK,   ALPHA_TWODOTS,   0,       rank, 0, 0, 0, 0, 0, 0) \
    _(ASLASH, ALPHA_SLASH,     areduce, 0,    0, 0, 0, 0, 0, 0) \
    _(NULLOP, 0)
#define ADVTAB_NAME(a, ...) a ,
enum { ADVTAB(ADVTAB_NAME) };

struct v ot[] = { ADVTAB(VERBTAB_ENT) };  //generate adverb table array

/* produce a new derived verb with specified fields. must have A z;
   declared and available for scratch. */
#define DERIV(...) \
    ((z=ga(VRB,1,(I[]){sizeof(struct v)/sizeof(int)})), \
    (*((V)AV(z))=(struct v){__VA_ARGS__}), \
    z)

/* adverb/conjunction macros */
I qv(); /* declare the verb predicate */
enum { ANOUN = 1, AVERB, N_A,
    NN=ANOUN*N_A+ANOUN,
    NV=ANOUN*N_A+AVERB,
    VN=AVERB*N_A+ANOUN,
    VV=AVERB*N_A+AVERB,
};
#define VERBNOUN(x) \
    (qv(x)?AVERB:ANOUN)

#define CONJCASE(a,w) \
    (VERBNOUN(a)*N_A+VERBNOUN(w))

/* load verb data from code in x.
   save new derived verb in x. */
#define LOADV(x) \
    abs((I)x)<(sizeof vt/sizeof*vt)? \
        (x=DERIV(vt[(I)x].c, vt[(I)x].vm, vt[(I)x].vd, vt[(I)x].f, vt[(I)x].g, \
                vt[(I)x].k, vt[(I)x].m, vt[(I)x].n, vt[(I)x].id)) \
        :x

/* & conjunction */
V2(amp){
    A z;
    switch(CONJCASE(a,w)){
    case NN: R 0;
    case NV: R DERIV(ALPHA_AMPERSAND, withl, NULL, (I)a, (I)w, 0, 0, 0, 0);
    case VN: R DERIV(ALPHA_AMPERSAND, withr, NULL, (I)a, (I)w, 0, 0, 0, 0);
    case VV: R DERIV(ALPHA_AMPERSAND, on1,   on2,  (I)a, (I)w, 0, 0, 0, 0);
    }
}

/* rank conjunction */
V2(rank){
    A z;
    switch(CONJCASE(a,w)){
    case NN: R 0;
    case NV: R 0;
    case VN: LOADV(a);
             switch(AN(w)){
             case 0: R 0;
             case 1: R DERIV(((V)AV(a))->c, ((V)AV(a))->vm, ((V)AV(a))->vd, 0, 0,
                             numint(*AV(w)), numint(*AV(w)), numint(*AV(w)), ((V)AV(a))->id);
             case 2: R DERIV(((V)AV(a))->c, ((V)AV(a))->vm, ((V)AV(a))->vd, 0, 0,
                             numint(AV(w)[1]), numint(AV(w)[0]), numint(AV(w)[1]), ((V)AV(a))->id);
             case 3: R DERIV(((V)AV(a))->c, ((V)AV(a))->vm, ((V)AV(a))->vd, 0, 0,
                             numint(AV(w)[0]), numint(AV(w)[1]), numint(AV(w)[2]), ((V)AV(a))->id);
             default: R 0;
             }
    case VV: LOADV(a);
             LOADV(w);
             R DERIV(((V)AV(a))->c, ((V)AV(a))->vm, ((V)AV(a))->vd, 0, 0,
                     ((V)AV(w))->k, ((V)AV(w))->m, ((V)AV(w))->n, ((V)AV(a))->id);
    }
}

/* reduce adverb */
V1(areduce){
    A z;
    R DERIV(ALPHA_SLASH, reduce, NULL, (I)w, 0, 0, 0, 0, 0);
}

/* Verb macros */

/* create v pointer to access verb properties
   if self is zero, load base verb from verb table
 */
#define LOADVSELF(base) \
    V v=self? \
        (I)self>0&&(I)self<(sizeof vt/sizeof*vt)? \
            vt+(I)self \
            :(V)AV(self) \
        :vt+base; 

/* fill-in array f with rk elements of ar's shape vector */
#define LOADFRAME(f,ar,rk) \
    /*P("rk=%d\n",rk);*/ \
    if (AR(ar)-(rk)>=0) { \
        /*f = ga(0,AR(ar)?AR(ar)==1?0:1:0,(I[]){AR(ar)-(rk)?AR(ar)-(rk):1});*/ \
        /*mv(AV(f),AR(ar)-(rk)?AD(ar):(I[]){0},AR(ar)-(rk)?AR(ar)-(rk):1);*/ \
        AR(f)=AR(ar)-(rk)>0?1:0; \
        AN(f)=AD(f)[0]=AR(ar)-(rk); \
        AK(f)=((C*)AD(ar))-((C*)f); /*make "indirect" array of ar's frame shape*/ \
    } \
    /*P("AR(f)=%d\n", AR(f));*/ 

/* load the left frame */
#define LFRAME(rk) \
    /*A lf = 0;*/ \
    A lf = &(struct a){.t=INT,.n=0,.r=0}; \
    if ((rk)<0) { LOADFRAME(lf,a,AR(a)+(rk)) } \
    else { LOADFRAME(lf,a,rk) }

/* load the right frame */
#define RFRAME(rk) \
    /*A rf = 0;*/ \
    A rf = &(struct a){.t=INT,.n=0,.r=0}; \
    if ((rk)<0) { LOADFRAME(rf,w,AR(w)+(rk)) } \
    else { LOADFRAME(rf,w,rk) } 

/* fill-in array c with rk elements from the tail of ar's shape vector */
#define LOADCELL(c,ar,rk) \
    if ((rk)>0 && (rk)<AR(ar)) { \
        AR(c)=(rk)>0?1:0; \
        AN(c)=AD(c)[0]=rk; \
        AK(c)=((C*)(AD(ar)+AR(ar)-(rk)))-((C*)c); /* indirect array of ar's cell shape */ \
    }

/* load the left cell */
#define LCELL(rk) \
    A lc = &(struct a){.t=INT,.n=0,.r=0}; \
    if ((rk)<0) { LOADCELL(lc,a,AR(a)+(rk)) } \
    else { LOADCELL(lc,a,rk) }

/* load the right cell */
#define RCELL(rk) \
    A rc = &(struct a){.t=INT,.n=0,.r=0}; \
    if ((rk)<0) { LOADCELL(rc,w,AR(w)+(rk)) } \
    else { LOADCELL(rc,w,rk) }

/* reshape arguments to make frames agree */
#define FRAME_AGREE \
    /*P("frame agree\n");*/ \
    if (!*AV(match(lf,rf,0))) { /* Frame Agreement */ \
        /*P("no match\n");*/ \
        if (AN(lf)==0) /* Scalar extension on left frame */ \
        { \
            /*P("reshape_a\n");*/ \
            /*pr(rf); pr(lc);*/ \
            if (AN(lc)>0) \
                a=rsh(cat(rf,lc,0),a,0); \
            else \
                a=rsh(rf,a,0); \
            /*pr(a);*/ \
        } else \
        if (AN(rf)==0) /* Scalar extension on right frame */ \
        { \
            /*P("reshape_w\n");*/ \
            /*pr(lf); pr(rc);*/ \
            if (AN(rc)>0) \
                w=rsh(cat(lf,rc,0),w,0); \
            else \
                w=rsh(lf,w,0); \
            /*pr(w);*/ \
        } \
    }

/* header size */
#define HSZ(rc) ((AR(rc)?AR(rc):0)+sizeof(struct a))

/*
   create an box array of frame dimensions, 
   each element of which is an indirect array 
   accessing a cell-sized slice of the argument array.
 */
#define BOX_CELLS(base,csz,lf,lc,a,ba,bam) \
        csz=tr(AN(lc),AV(lc)); /* cell size */ \
        A ba=ga(BOX,AN(lf),AV(lf)); /* boxed a */ \
        A bam=(A)ma(HSZ(lc)*tr(AN(lf),AV(lf)));  /* boxed a memory (for array headers) */ \
        DO(AN(ba), \
            AV(ba)[i]=(I)(((I*)bam)+HSZ(lc)*i);   /* set pointer to header in box array */ \
            AT((A)(((I*)bam)+HSZ(lc)*i))=AT(a);   /* type of header is type of a  */ \
            AN((A)(((I*)bam)+HSZ(lc)*i))=csz;     /* num elements is cell size    */ \
            AR((A)(((I*)bam)+HSZ(lc)*i))=AN(lc);  /* rank is length of cell shape */ \
            mv(AD((A)(((I*)bam)+HSZ(lc)*i)),AV(lc),AN(lc)); /* dims are cell data */ \
            AK((A)(((I*)bam)+HSZ(lc)*i))=               /* array data is (ptr to) */ \
                ((C*)(AV(w)+csz*i))-((C*)(((I*)bam)+HSZ(lc)*i));    /* slice of a */ \
            /*pr(AV(ba)[i]);*/ \
          ) \

/*
   unpack and expand the boxed results from the recursive call to the verb
   */
#define ASSEMBLE_RESULTS(ba,bam) \
        /*P("assemble results\n");*/ \
        A ms = ga(INT,1,(I[]){0}); \
        DO(AN(bz), /* find max shape */ \
                if ( (AR((A)(AV(bz)[i]))>AN(ms)) \
                  || (tr(AR((A)(AV(bz)[i])),AD((A)(AV(bz)[i]))) > tr(AR(ms),AD(ms))) \
                   ) \
                { \
                    free(ms); \
                    ms = ga(INT,1,&AR((A)(AV(bz)[i]))); \
                    mv(AV(ms),AD((A)(AV(bz)[i])),AR((A)(AV(bz)[i]))); \
                } \
          ) \
        /*pr(ms);*/ \
        DO(AN(bz), /* pad to max shape */ \
                if ( (AR((A)(AV(bz)[i]))<AN(ms)) \
                  || (tr(AR((A)(AV(bz)[i])),AD((A)(AV(bz)[i])))<tr(AN(ms),AV(ms))) \
                   ) \
                { \
                    AV(bz)[i]=(I)rsh(ms,(A)AV(bz)[i],0); \
                } \
          ) \
        rf=cat(rf,ms,0); \
        /*pr(rf);*/ \
        A z=ga(AT(w),AN(rf),AV(rf)); \
        A zslice=ga(AT(w),1,&AN(ms)); \
        AR(zslice)=AN(ms); \
        mv(AD(zslice),AV(ms),AN(ms)); \
        AK(zslice)=((C*)AV(z))-((C*)zslice); \
        AN(zslice)=tr(AR(zslice),AD(zslice)); \
        DO(AN(bz), \
                mv(AV(zslice),AV((A)AV(bz)[i]),AN(zslice)); \
                /*pr(zslice);*/ \
                AK(zslice)+=AN(zslice)*sizeof(I); \
          ) \
        free(ms); free(zslice); free(ba); free(bam); R z; \

/*
   monadic verb behavior for handling non-base cells
   */
#define CELL_HANDLE1(base) \
    if (self&& (vt[base].k != v->k)) { \
        /* requested cell is not base cell */ \
        I csz; \
        BOX_CELLS(base,csz,rf,rc,w,bw,bwm) \
        A bz=v->vm(bw,base); \
        ASSEMBLE_RESULTS(bw,bwm) \
    }

/*
   dyadic verb behavior for handling non-base cells
   */
#define CELL_HANDLE2(base) \
    /*P("cell handle\n");*/ \
    if (self&& (vt[base].m != v->m && vt[base].n != v->n)) { \
        /*P(" requested cells are not base cells\n");*/ \
        I csz; \
        BOX_CELLS(base,csz,lf,lc,a,ba,bam) \
        BOX_CELLS(base,csz,rf,rc,w,bw,bwm) \
        A bz=v->vd(ba,bw,base); \
        /*pr(bz);*/ \
        free(ba); free(bam); \
        ASSEMBLE_RESULTS(bw,bwm) \
    } else if (self&& vt[base].m != v->m) { \
        /*P(" left cell is not base cell\n");*/ \
        I csz; \
        BOX_CELLS(base,csz,lf,lc,a,ba,bam) \
        A bz=v->vd(ba,w,base); /* call self recursively with base ranks */ \
        /*pr(bz);*/ \
        /* assemble results */ \
        ASSEMBLE_RESULTS(ba,bam) \
    } else if (self&& vt[base].n != v->n) { \
        /*P(" right cell is not base cell\n");*/ \
        I csz; \
        BOX_CELLS(base,csz,rf,rc,w,bw,bwm) \
        A bz=v->vd(a,bw,base); /* call self recursively with base ranks */ \
        /*pr(bz);*/ \
        ASSEMBLE_RESULTS(bw,bwm) \
    }

/*
   monadic behavior for handling a boxed argument:
   call verb recursively upon each element
   */
#define BOX_HANDLE1(base) \
    if (AT(w)==BOX) { \
        A z=ga(BOX,AN(rf),AV(rf)); \
        DO(AN(z), \
                AV(z)[i]=(I)v->vm(AV(w)[i],base); \
          ) \
        R z; \
    }

/*
   dyadic behavior for handling boxed arguments:
   call verb recursively upon boxed elements or slices
   */
#define BOX_HANDLE2(base) \
    if (AT(a)==BOX&&AT(w)==BOX){ \
        /*P("BOXaw\n");*/ \
        A z=ga(BOX,AN(lf),AV(lf)); \
        DO(AN(z), \
                AV(z)[i]=(I)v->vd(AV(a)[i],AV(w)[i],base); \
          ) \
        R z; \
    } else if (AT(a)==BOX) { \
        /*P("BOXa\n");*/ \
        A wslice=ga(AT(w),1,&AN(rc)); \
        AR(wslice)=AN(rc); \
        mv(AD(wslice),AV(rc),AN(rc)); \
        AK(wslice)=((C*)AV(w))-((C*)wslice); \
        AN(wslice)=tr(AR(wslice),AD(wslice)); \
        A z=ga(BOX,AN(lf),AV(lf)); \
        DO(AN(z), \
                AV(z)[i]=(I)v->vd(AV(a)[i],wslice,base); \
                AK(wslice)+=AN(wslice)*sizeof(I); \
          ) \
        free(wslice); \
        R z; \
    } else if (AT(w)==BOX) { \
        /*P("BOXw\n");*/ \
        A aslice=ga(AT(a),1,&AN(lc)); \
        AR(aslice)=AN(lc); \
        mv(AD(aslice),AV(lc),AN(lc)); \
        AK(aslice)=((C*)AV(a))-((C*)aslice); \
        AN(aslice)=tr(AR(aslice),AD(aslice)); \
        A z=ga(BOX,AN(rf),AV(rf)); \
        DO(AN(z), \
                /*P("slice=%d %d %d %d\n", AR(aslice), *AD(aslice), AN(aslice), AK(aslice));*/ \
                /*pr(aslice);*/ \
                /*pr(AV(w)[i]);*/ \
                /*P("recurse(slice,box) %d\n",i);*/ \
                AV(z)[i]=(I)v->vd(aslice,AV(w)[i],base); \
                AK(aslice)+=AN(aslice)*sizeof(I); \
          ) \
        free(aslice); \
        R z; \
    }

/* general rank/frame/cell behavior for monadic verbs */
#define RANK1(base) \
    LOADVSELF(base) \
    RFRAME(v->k) \
    RCELL(v->k) \
    CELL_HANDLE1(base) \
    BOX_HANDLE1(base)

/* general rank/frame/cell behavior for dyadic verbs */
#define RANK2(base) \
    LOADVSELF(base) \
    /*pr(a); pr(w);*/ \
    LFRAME(v->m) \
    RFRAME(v->n) \
    LCELL(v->m) \
    RCELL(v->n) \
    /*P("%d_%d\n",v->m,v->n);*/ \
    /*P("%d_%d\n",AR(a),AR(w));*/ \
    /*P("%d_%d\n",lf?AR(lf):0,rf?AR(rf):0);*/ \
    /*P("%d_%d\n",lc?AN(lc):0,rc?AN(rc):0);*/ \
    /*pr(lf); pr(lc);*/ \
    /*pr(rf); pr(rc);*/ \
    FRAME_AGREE \
    CELL_HANDLE2(base) \
    BOX_HANDLE2(base) \

/* derived verb data cracker for derived verb actions */
#define DECLFG \
    A z; \
    LOADV(self); \
    V v=(V)AV(self); \
    A fs = (A)v->f; LOADV(fs); \
    A gs = (A)v->g; LOADV(gs); \
    V1((*f1)) = ((V)AV(fs))->vm; \
    V1((*g1)) = ((V)AV(gs))->vm; \
    V2((*f2)) = ((V)AV(fs))->vd; \
    V2((*g2)) = ((V)AV(gs))->vd;

/* derived verb actions */
V1(withl){ DECLFG; R g2(fs,w,gs); }
V1(withr){ DECLFG; R f2(w,gs,fs); }
V1(on1){ DECLFG; R f1(g1(w,gs),fs); }
V2(on2){ DECLFG; R f2(g1(a,gs),g1(w,gs),fs); }

/* derived action for reduce */
V1(reduce){
    DECLFG; 
    //pr(self);
    switch(AR(w)){
    case 0: z=w; break;
    case 1: z=num0(((V)AV(fs))->id);
            switch(AN(w)){
            case 0: break;
            //case 1: z=num0r(*AV(w)); break;
            default:  z=num0r(AV(w)[AN(w)-1]);
                    DO(AN(w)-1, z=f2(num0r(AV(w)[AN(w)-2-i]),z,fs));
            }
    default:
            ;
    }
    R z;
}

/* (internal) match verb. used in rank agreement by all verbs. */
V2(match){
    if(a==w) R num0(1);
    if(a && w) {
        if(AR(a)!=AR(w)
        || AN(a)!=AN(w)
          ) {
            R num0(0);
        }
        DO(AN(a),if(AV(a)[i]!=AV(w)[i])R num0(0);)
    } else {
        R num0(0);
    }
    R num0(1);
}

/* numeric type crackers for math function verbs */
enum { IMM = 1, FIX, FLO, MPI, NUM_TYPES };
#define NUMTYPEPAIR(a,b) \
    ((a)*NUM_TYPES+(b))

/* yield the type of a num */
#define TYPENUM(a) \
        ((a)&BANK_MASK? \
            (AT(((A)AV(bank)[((a)&BANK_MASK)>>IMM_BIT]))==DBL? \
                 FLO \
                 : (AT(((A)AV(bank)[((a)&BANK_MASK)>>IMM_BIT]))==BOX? \
                     MPI \
                     :FIX) \
             ) \
            :IMM)

/* compose types of 2 arguments into a value */
#define NUMERIC_TYPES(a,b) \
    NUMTYPEPAIR(TYPENUM(a), TYPENUM(b))

/* apply unary math op to num, yielding num */
#define MON_MATH_FUNC(func,z,y,overflow) \
    switch(TYPENUM(y)){ \
    case IMM: if (overflow(numimm(y))) z=flo(func (D)numimm(y)); \
              else z=num(func numimm(y)); break; \
    case FIX: if (overflow(numint(y))) z=flo(func (D)numint(y)); \
              else z=num(func numint(y)); break; \
    case FLO: z=flo(func numdbl(y)); break; \
    case MPI: z=mpop(mpzero, *#func, y); break; \
    }

/* perform the domain-checking case in BIN_MATH_FUNC macro */
#define DOM(d,z,x,y)    if (!d(x,y)){ z=infinite; } else

/* select overflow promotion behavior. 0=double 1=mpi */
static int overflow_mpi = 1;

/* apply binary math op to nums x and y, assigning result as num z.
TODO: additional numeric types.
    configurable overflow promotion handling. <-- partially done.
 */
#define BIN_MATH_FUNC(func,z,x,y,overflow,domainI,domainD) \
     switch(NUMERIC_TYPES(x,y)){ \
     case NUMTYPEPAIR(IMM,IMM): DOM(domainI,z,numimm(x),numimm(y)) \
                             if (overflow(numimm(x),numimm(y))) \
                                 if (overflow_mpi) \
                                     z=mpop(mpi(numimm(x)),*#func,mpi(numimm(y))); \
                                 else \
                                     z=flo((D)numimm(x) func (D)numimm(y)); \
                             else z=num(numimm(x) func numimm(y)); break; \
     case NUMTYPEPAIR(IMM,FIX): DOM(domainI,z,numimm(x),numint(y)) \
                             if (overflow(numimm(x),numint(y))) \
                                 if (overflow_mpi) \
                                     z=mpop(mpi(numimm(x)),*#func,mpi(numint(y))); \
                                 else \
                                     z=flo((D)numimm(x) func (D)numint(y)); \
                             else z=num(numimm(x) func numint(y)); break; \
     case NUMTYPEPAIR(IMM,FLO): DOM(domainD,z,numimm(x),numdbl(y)) \
                             z=flo(numimm(x) func numdbl(y)); break; \
     case NUMTYPEPAIR(FIX,IMM): DOM(domainI,z,numint(x),numimm(y)) \
                             if (overflow(numint(x),numimm(y))) \
                                 if (overflow_mpi) \
                                     z=mpop(mpi(numint(x)),*#func,mpi(numimm(y))); \
                                 else \
                                     z=flo((D)numint(x) func (D)numimm(y)); \
                             else z=num(numint(x) func numimm(y)); break; \
     case NUMTYPEPAIR(FIX,FIX): DOM(domainI,z,numint(x),numint(y)) \
                             if (overflow(numint(x),numint(y))) \
                                 if (overflow_mpi) \
                                     z=mpop(mpi(numint(x)),*#func,mpi(numint(y))); \
                                 else \
                                     z=flo((D)numint(x) func (D)numint(y)); \
                             else z=num(numint(x) func numint(y)); break; \
     case NUMTYPEPAIR(FIX,FLO): DOM(domainD,z,numint(x),numdbl(y)) \
                             z=flo(numint(x) func numdbl(y)); break; \
     case NUMTYPEPAIR(FLO,IMM): DOM(domainD,z,numdbl(x),numimm(y)) \
                             z=flo(numdbl(x) func numimm(y)); break; \
     case NUMTYPEPAIR(FLO,FIX): DOM(domainD,z,numdbl(x),numint(y)) \
                             z=flo(numdbl(x) func numint(y)); break; \
     case NUMTYPEPAIR(FLO,FLO): DOM(domainD,z,numdbl(x),numdbl(y)) \
                             z=flo(numdbl(x) func numdbl(y)); break; \
     case NUMTYPEPAIR(IMM,MPI): z=mpop(mpi(numimm(x)), *#func, y); break; \
     case NUMTYPEPAIR(FIX,MPI): z=mpop(mpi(numint(x)), *#func, y); break; \
     case NUMTYPEPAIR(MPI,IMM): z=mpop(x, *#func, mpi(numimm(y))); break; \
     case NUMTYPEPAIR(MPI,FIX): z=mpop(x, *#func, mpi(numint(y))); break; \
     case NUMTYPEPAIR(MPI,MPI): z=mpop(x, *#func, y); break; \
     }

#define BIN_BOOL_FUNC()

//#define TEST_MPI  /* define TEST_MPI to lower the overflow promotion thresholds */
#ifdef TEST_MPI
I plusover(I x,I y){ R (((x>0)&&(y>0)&&(x>(MPI_MAX-y))) || ((x<0)&&(y<0)&&(x<(MPI_MAX-y)))); }
#else
I plusover(I x,I y){ R (((x>0)&&(y>0)&&(x>(INT_MAX-y))) || ((x<0)&&(y<0)&&(x<(INT_MAX-y)))); }
#endif
I plusdomainI(I x,I y){ R 1; }
I plusdomainD(D x,D y){ R 1; }

/* return w */
V1(id){R w;}
/* add */
V2(plus){
    /*P("plus!\n");*/
    RANK2(PLUS)
    A z=ga(NUM,AR(w),AD(w));
    //P("%d\n",v->id);
    DO(AN(z), BIN_MATH_FUNC(+,AV(z)[i],AV(a)[i],AV(w)[i],plusover,plusdomainI,plusdomainD) )
    R z;}

I minusover(I x,I y){ R (y==INT_MIN&&x==0)?1:plusover(x,-y); }
I negover(I y){ R minusover(0,y); }

/* negate w */
V1(neg){
    RANK1(MINUS)
    A z=copy(w,0);
    DO(AN(z), MON_MATH_FUNC(-,AV(z)[i],AV(z)[i],negover))
    R z;}
V2(minus){
    RANK2(MINUS)
    A z=ga(NUM,AR(w),AD(w));
    DO(AN(z), BIN_MATH_FUNC(-,AV(z)[i],AV(a)[i],AV(w)[i],minusover,plusdomainI,plusdomainD))
    R z;}

/* return sum and difference */
V2(plusminus){ w=cat(w,neg(w,0),0); a=cat(a,a,0); R plus(a,w,0);}

#ifdef TEST_MPI
I timesover(I x,I y){
    if(x==0||y==0)R 0;
    if(x<0){x=-x;}
    if(y<0){y=-y;}
    R x>((MPI_MAX)/(y));
}
#else
I timesover(I x,I y){
    if(x==0||y==0)R 0;
    if(x==INT_MIN||y==INT_MIN)R 1;
    if(x<0){ x=-x; }
    if(y<0){ y=-y; }
    R x>((INT_MAX)/(y));
}
#endif


V1(signum){
    RANK1(TIMES)
    A z=ga(NUM,AR(w),AD(w));
    DO(AN(z), BIN_BOOL_FUNC())
    R z;}

V2(times){
    RANK2(TIMES)
    A z=ga(NUM,AR(w),AD(w));
    DO(AN(z), BIN_MATH_FUNC(*,AV(z)[i],AV(a)[i],AV(w)[i],timesover,plusdomainI,plusdomainD))
    R z;}

I divover(I x,I y){ R y?x%y:1; }
I divdomainI(I x,I y){ R y; }
I divdomainD(D x,D y){ R y!=0.0; }

V2(quotient){
    RANK2(DIVIDE)
    A z=ga(NUM,AR(w),AD(w));
    DO(AN(w), BIN_MATH_FUNC(/,AV(z)[i],AV(a)[i],AV(w)[i],divover,divdomainI,divdomainD))
    R z;}

/* make a copy */
V1(copy){I n=AN(w); A z=ga(AT(w),AR(w),AD(w)); mv(AV(z),AV(w),n); R z;}
/* generate index vector */
V1(iota){I n=*AV(w);A z=ga(INT,1,&n);DO(n,AV(z)[i]=i);R z;}
/* not implemented */
V2(find){}

/* length of first dimension */
V1(size){A z=ga(INT,0,0);*AV(z)=AR(w)?*AD(w):1;R z;}
/* index */
V2(from){
    if (AT(a)==BOX){
        DO(AN(a),w=from((A)AV(a)[i],w,0))
        R w;
    } else {
        I r=AR(w)-1,*d=AD(w)+1,n=tr(r,d);
        A z=ga(AT(w),r,d);
        mv(AV(z),AV(w)+(n**AV(a)),n);
        R z;
    }
}

/* pack array into a scalar */
V1(box){A z=ga(BOX,0,0);*AV(z)=(I)w;R z;}
/* catenate two arrays */
V2(cat){
     //P("cat:\n"); pr(a); pr(w);
     I an=AN(a),wn=AN(w),n=an+wn;
     A z=ga(AT(w),1,&n);mv(AV(z),AV(a),an);mv(AV(z)+an,AV(w),wn);
     //pr(z);
     R z;}

V2(boxcat){
    I at = AT(a)==BOX, wt = AT(w)==BOX;
    switch (at*2+wt){
        case 0*2+0: R cat(box(a,0),box(w,0),0);
        case 0*2+1: R cat(box(a,0),w,0);
        case 1*2+0: R cat(a,box(w,0),0);
        case 1*2+1: R cat(a,w,0);
    }
}

/* return the shape of w */
V1(sha){A z=ga(INT,1,&AR(w));mv(AV(z),AD(w),AR(w));R z;}
/* reshape w to dimensions a */
V2(rsh){I r=AR(a)?AN(a):1,n=tr(r,AV(a)),wn=AN(w);
    //P("rsh:\n"); pr(a); pr(w);
    A z=ga(AT(w),r,AV(a));
    mv(AV(z),AV(w),wn=n>wn?wn:n);  /* copy */
    if((n-=wn)>0)mv(AV(z)+wn,AV(z),n); /* move */
    //P("#:");pr(z);
    R z;}


/* Printing */

/* print symbol */
ps(A i){P("%s",(char*)AV(i));}
/* print verb */
pv(i){
    if (i>0 && i<sizeof vt/sizeof*vt)
        P("%s",basetooutput(alphatab[vt[i].c].base));
    else if (i>0 && i<sizeof vt/sizeof*vt+sizeof ot/sizeof*ot)
        P("%s",basetooutput(alphatab[ot[i-ZEROOP].c].base));
    else
        pc(i);
}
/* print derived verb */
pdv(i){
    A w=(A)i;
    V v=(V)AV(w);
    P("derived ");
    pv(v->c);
}
/* print character */
pc(i){qv(i)?pv(i):P("%s",basetooutput(i));}
pi(i){
    P("%d ",i);
}
/* print number */
pn(i){
    switch(TYPENUM(i)){
    case IMM: P("%d ",numimm(i)); break;
    case FIX: P("%d ",numint(i)); break;
    case FLO: P("%f ",numdbl(i)); break;
    case MPI: P("mpi:");
              {
                  A b=numbox(i);
                  I allzero=1;
                  DO(AN(b),if((AV(b)[i]&~MPI_SIGN_BIT)==0)continue;allzero=0;)
                  if (allzero) 
                      P("0");
                  else {
                      if (MPI_SIGN(b)) P("-");
                      if (AN(b)>1 && AV(b)[0]&~MPI_SIGN_BIT)
                          P("%d",AV(b)[0]&~MPI_SIGN_BIT);
                      DO(AN(b)-1, P("%04d",AV(b)[i+1]))
                  }
              }
              P(" ");
              break;
    }
}
/* print newline */
nl(){P("\n");}
/* print any array */
pr(A w){
    if (abs((I)w)<256) /* "scalar" verb repr */
        pv(w);
    else {
        I r=AR(w),*d=AD(w),n=AN(w);
        if(w==null)R 0;
        DO(r,pi(d[i])); P("#"); nl();
        switch(AT(w)){
        case BOX: DO(n,P("< ");pr((A)(AV(w)[i]))) break;
        case SYMB: ps(w); break;
        case NUM: DO(n,pn(AV(w)[i])) break;
        case INT: DO(n,pi(AV(w)[i])) break;
        case VRB: pdv(w); break;
        default: P("unprintable ");
        }
    }
    nl();
}


/* Symbol Table */


/*
   Character classes used by the symbol table function
   findsymb() and by the scanning function wd().
 */
#define ALPHALOWER "abcdefghijklmnopqrstuvwxyz"
#define ALPHAUPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define HIMINUS ((char[]){MODE1('@'), 0})
#define DIGIT "0123456789"
#define DOT "."
#define SPACE " \t"
#define ZEROCL ""

/*
   The symbol table is implemented as a trie data structure
   with 52-way branching, allowing strict alphabetic symbols.
 */
struct st { A a; struct st *tab[52]; } st; /* symbol-table */
#define ST_INIT (st.a = null) /* initialize symbol table root value */

char *alph=ALPHAUPPER ALPHALOWER; /* symbol-table collation-ordered set */
/*
   find symbol in symbol table.
    mode 0: search trie for longest-prefix match.
        ret root on fail (val=null).
        update input string point to remainder (no change on fail).
    mode 1: search and allocate.
        ret new leaf node (val=null).
        update input string.  */
struct st *findsymb(struct st *st, char **s, int mode) {
    int code;
    struct st *last = st;
    char *lasp = *s;
    while(isalpha(**s)){
        code = strchr(alph,**s)-alph;
        if (st->tab[code]){
            st=st->tab[code];
            (*s)++;
            if (st->a!=null){
                last = st;
                lasp = *s;
            }
        } else
            switch(mode){
            case 0:             //prefix search
                    *s = lasp;
                    return last;
            case 1:             //defining search
                    st->tab[code] = calloc(1,sizeof(struct st));
                    st->a=null;
                    st=st->tab[code];
                    (*s)++;
                    break;
            }

    }
    return st; 
}


/* Execution */

/*
   Predicate table for pattern matching in the parser ex().
 */
#define PREDTAB(_) \
    _( ANY = 1,    qa, 1                                )                       \
    _( VAR = 2,    qp, abs(a)>256                                               \
                       && AT((A)a)==SYMB                )                       \
    _(NOUN = 4,    qn, abs(a)>256                                               \
                       && AT((A)a)!=SYMB                                        \
                       && AT((A)a)!=VRB                                         \
                       && AT((A)a)!=MRK                                         \
                       && AT((A)a)!=NLL                                         \
                       && ((A)a)!=null                                          \
                       && ((A)a)!=&nullob               )                       \
    _(VERB = 8,    qv, (a>0                                                     \
                           && a<(sizeof vt/sizeof*vt)                           \
                           && (vt[a].vm || vt[a].vd))                           \
                       || (abs(a)>256 && AT((A)a)==VRB) )                       \
    _(ADV  = 16,   qo, a>0                                                      \
                       && a>(sizeof vt/sizeof*vt)                               \
                       && a<(sizeof vt/sizeof*vt+sizeof ot/sizeof*ot)           \
                       && (ot[a-ZEROOP].vm)             )                       \
    _(CONJ = 32,   qj, a>0                                                      \
                       && a>(sizeof vt/sizeof*vt)                               \
                       && a<(sizeof vt/sizeof*vt+sizeof ot/sizeof*ot)           \
                       && (ot[a-ZEROOP].vd)             )                       \
    _(ASSN = 64,   qc, a==MODE1('[')                    )                       \
    _(MARK = 128,  qm, ((A)a)==mark                     )                       \
    _(LPAR = 256,  ql, a=='('                           )                       \
    _(RPAR = 512,  qr, a==')'                           )                       \
    _(NULP = 1024, qu, ((A)a)==null                     )
#define PRED_FUNC(X,Y,...) I Y(I a){R __VA_ARGS__;}
PREDTAB(PRED_FUNC)                      //generate predicate functions
#define PRED_ENT(a,b,...) b ,
I(*q[])(I) = { PREDTAB(PRED_ENT) };      //generate predicate function table
#define PRED_ENUM(a,...) a ,
enum predicate { PREDTAB(PRED_ENUM)     //generate predicate symbols
                 EDGE = MARK+ASSN+LPAR,
                 AVN = VERB+NOUN+ADV };
/* encode predicate applications into a binary number,
   a bitset represented as a bit mask */
int classify(A a){ int i,v,r;
    for(i=0,v=1,r=0;i<(sizeof q/sizeof*q);i++,v*=2)if(q[i]((I)a))r|=v;
    //P("%d ", r); pr(a);
    R r;}

/*
   Parse table for processing expressions on top of the right-stack
 */
#define PARSETAB(_) \
    /*INDEX   PAT1          PAT2       PAT3  PAT4       ACTION*/            \
    /*      =>t[0]          t[1]       t[2]  t[3]             */            \
    _(MONA,   EDGE,         VERB,      NOUN, ANY,                           \
            {stackpush(rstk,t[3]);                                          \
             if(abs((I)t[1])>256) {                                                         \
                 if(((V)AV(t[1]))->vm) stackpush(rstk,((V)AV(t[1]))->vm(t[2],t[1]));        \
                 else stackpush(rstk,((V)AV(t[1]))->vd(num0(((V)AV(t[1]))->id),t[2],t[1])); \
             } else {                                                                       \
                 if(vt[(I)t[1]].vm) stackpush(rstk,vt[(I)t[1]].vm(t[2],t[1]));              \
                 else stackpush(rstk,vt[(I)t[1]].vd(num0(vt[(I)t[1]].id),t[2],t[1]));       \
             }                                                                              \
             stackpush(rstk,t[0]);} )                                                       \
    \
    _(MONB,   EDGE+AVN,     VERB,      VERB, NOUN,                                          \
            {if(abs((I)t[2])>256) {                                                         \
                 if (((V)AV(t[2]))->vm) stackpush(rstk,((V)AV(t[2]))->vm(t[3],t[2]));       \
                 else stackpush(rstk,((V)AV(t[2]))->vd(num0(((V)AV(t[2]))->id),t[3],t[2])); \
             } else {                                                                       \
                 if (vt[(I)t[2]].vm) stackpush(rstk,vt[(I)t[2]].vm(t[3],t[2]));             \
                 else stackpush(rstk,vt[(I)t[2]].vd(num0(vt[(I)t[2]].id),t[3],t[2]));       \
             }                                                                              \
             stackpush(rstk,t[1]);                                                          \
             stackpush(rstk,t[0]);} )                                                       \
    \
    _(DYAD,   EDGE+AVN,     NOUN,      VERB, NOUN,                                  \
            {if(abs((I)t[2])>256)                                                   \
                 stackpush(rstk,((V)AV(t[2]))->vd(t[1],t[3],t[2]));                 \
             else stackpush(rstk,vt[(I)t[2]].vd(t[1],t[3],t[2]));                   \
             stackpush(rstk,t[0]);} )                                               \
    \
    _(ADVB,   EDGE+AVN,     NOUN+VERB, ADV,  ANY,                                   \
            {stackpush(rstk,t[3]);                                                  \
             stackpush(rstk,ot[((I)t[2])-ZEROOP].vm(t[1],t[2]));                    \
             stackpush(rstk,t[0]);} )                                               \
    \
    _(FRMJ,   EDGE+AVN,     NOUN+VERB, CONJ, NOUN+VERB,                     \
            {stackpush(rstk,ot[((I)t[2])-ZEROOP].vd(t[1],t[3],t[2]));       \
             stackpush(rstk,t[0]);} )                                       \
    \
    _(SPEC,   VAR,          ASSN,      AVN,  ANY,                           \
            {char *s=(char*)AV(t[0]);                                       \
             struct st *slot = findsymb(&st,&s,1);                          \
             stackpush(rstk,t[3]);                                          \
             stackpush(rstk,slot->a=t[2]);} )                               \
    \
    _(PUNC,   LPAR,         ANY,       RPAR, ANY,                           \
            {stackpush(rstk,t[3]);                                          \
             stackpush(rstk,t[1]);} )                                       \
    \
    _(FAKL,   MARK,         ANY,       RPAR, ANY,                           \
            {stackpush(rstk,t[3]);                                          \
             stackpush(rstk,t[1]);                                          \
             stackpush(rstk,t[0]);} )                                       \
    \
    _(FAKR,   EDGE+AVN,     LPAR,      ANY,  NULP,                          \
            {stackpush(rstk,t[3]);                                          \
             stackpush(rstk,t[2]);                                          \
             stackpush(rstk,t[0]);} )                                       \
    \
    _(INDX,  EDGE+VERB+ADV, NOUN,      NOUN, ANY,                           \
            {stackpush(rstk,t[3]);                                          \
             stackpush(rstk,from(t[2],t[1],0));                             \
             stackpush(rstk,t[0]); })                                       \
    \
    _(NOACT, 0,        0,    0,    0,    0;)
#define PARSETAB_PAT(name, pat1, pat2, pat3, pat4, ...) { pat1, pat2, pat3, pat4 },
struct parsetab { I c[4]; } parsetab[] = { PARSETAB(PARSETAB_PAT) };
#define PARSETAB_INDEX(name, ...) name,
enum { PARSETAB(PARSETAB_INDEX) };
#define PARSETAB_ACTION(name, pat1, pat2, pat3, pat4, ...) case name: __VA_ARGS__ break;

/*
   Stack data structure.
   if stkp->top!=0 then top element is stkp->a[stkp->top-1]
 */
typedef struct stack { int top; A a[1]; } stack; /* top==0::empty */
#define stackpush(stkp,el) ((stkp)->a[(stkp)->top++]=(el))
#define stackpop(stkp) ((stkp)->a[--(stkp)->top])


/*
   Evaluate an expression according to APL rules.
   execute right-to-left.
   The arguments to a function are the single element to the left
   and the entire expression to the right.
   Push expression to left-stack.
   The loop:
       move element to right stack, looking up variables.
       check right stack for patterns
             and execute appropriate actions, reducing the stack.
   Terminate loop when left-stack is depleted and no further patterns
   apply to the right stack.
   Return remaining element.
   Print error if expression has not fully reduced (unrecognized function).
 */
A ex(I *e){I a=*e;
    int i,j,n,docheck;
    stack *lstk,*rstk;
    A t[4];

    //for(i=0;e[i];i++)pr((A)e[i]);
    for(i=0,j=0;e[i];i++)if(qp(e[i]))j+=AD(((A)e[i]))[0];
    n=i;
    lstk=malloc(sizeof*lstk + (1+i+j)*sizeof(A));
    lstk->top = 0;
    stackpush(lstk,mark);
    for(i=0;i<n;i++) stackpush(lstk,((A)e[i])); //push to lstk
    rstk=malloc(sizeof*rstk + (1+i+j)*sizeof(A));
    rstk->top = 0;
    stackpush(rstk,null);
    while(lstk->top){ //push to rstk
        //for(i=0;i<lstk->top;i++)pr(lstk->a[i]); fflush(0);
        //for(i=0;i<rstk->top;i++)pr(rstk->a[i]); fflush(0);
        a=(I)stackpop(lstk);
        if(qp(a)){                              //parse defined names now
            if (rstk->top && qc((I)rstk->a[rstk->top-1])){
                stackpush(rstk,((A)a));
            } else { char *s,*p; struct st *tab; A sa=(A)a;
                s = p = (char*)AV(sa);
                tab=findsymb(&st,&p,0);
                while(*p){
                    if (tab==&st){
                        P("Error %s undefined\n",p);
                        R (A)a;
                    }
                    if (tab->a!=null){          // found a defined prefix
                        stackpush(lstk,newsymb(s,p-s,2)); //pushback prefix
                        s=p;
                    }
                    tab=findsymb(&st,&p,0);
                }
                stackpush(rstk,tab->a);
            }
        }else{stackpush(rstk,((A)a));}

        //niladic function?

        docheck=1;
        while(docheck) {
            docheck=0;
            if(rstk->top>=4){ I c[4];  //enough elements to check?
                for(j=0;j<4;j++)
                    c[j] = classify(rstk->a[rstk->top-1-j]);  //summarize attributes
                for(i=0;i<(sizeof parsetab/sizeof*parsetab);i++) { //match against table
                    if( (c[0]&parsetab[i].c[0])
                      &&(c[1]&parsetab[i].c[1])
                      &&(c[2]&parsetab[i].c[2])
                      &&(c[3]&parsetab[i].c[3]) ) {
                        docheck=1;
                        t[0]=stackpop(rstk);
                        t[1]=stackpop(rstk);
                        t[2]=stackpop(rstk);
                        t[3]=stackpop(rstk);
                        switch(i){
                            PARSETAB(PARSETAB_ACTION)

                        }
                    }
                }
            }
        }

    }

    //for(i=0;i<rstk->top;i++)pr(rstk->a[i]); fflush(0);
    stackpop(rstk); //mark
    a = (I)stackpop(rstk);
    if (rstk->top && rstk->a[rstk->top-1]!=null){
        P("Error extra elements on stack:\n");
        while ((i=(I)stackpop(rstk))!=(I)null)
            pr((A)i);
        P("\n");
    }
    free(rstk);
    free(lstk);
    R (A)a;
}


/* Scanning */


/* lookup character (in internal representation) in verb table */
verb(c){I i=0;
    for(;vt[++i].c;)
        if(alphatab[vt[i].c].base==c)
            R i;
    for(i=0;ot[++i].c;)
        if(alphatab[ot[i].c].base==c)
            R i+ZEROOP; //operators take the values after the last verb
    R 0;}

/*
   construct a single scanned token given a string pointer, length and type.
 */
A newsymb(C *s,I n,I state){
    I t;
    //P("%d\n",n);DO(n,P("%c",s[i]))P("\n");
    //if(strchr(DIGIT,*s) || strchr(HIMINUS,*s)) {
    switch(state){
    case 1: {
        char *end;
        s=strndup(s,n);
        while(isspace(s[n-1])) s[--n]=0;
        DO(n,if(s[i]==alphatab[ALPHA_MACRON].base)s[i]='-')
        //A z=ga(INT,0,0); *AV(z)=strtol(s,&end,10);
        A z;
        {
            long n=strtol(s,&end,10);
            if(*end=='.') { z=num0f(strtod(s,&end)); }
            else {
                if (abs(n)/MPI_BASE) z=num0r(mpi(n));
                else z=num0(n);
            }
        }
        while(((C*)end-s) < n){
            //A r=ga(INT,0,0); *AV(r)=strtol(end,&end,10);
            A r;
            {
                char *sp=end;
                long n=strtol(sp,&end,10);
                if(*end=='.') { r=num0f(strtod(sp,&end)); }
                else {
                    if (abs(n)/MPI_BASE) z=num0r(mpi(n));
                    else r=num0(n);
                }
            }
            z=cat(z,r,0);
        }
        free(s);
        R z; }
    //} else if(strchr(ALPHAUPPER ALPHALOWER,*s)) {
    case 2: {
        A z=ga(SYMB,1,(I[]){n+1});
        mv(AV(z),(I*)s,n+3/4);
        ((C*)AV(z))[n] = 0;
        R z; }
    //} else {
    case 3: {
        I c=verb(*s);
        R (A)(c?c:(I)*s); }
    }
}


/*
   scanner table for word formation
   columns are indexed by matched character classes
   rows are indexed by the current state of the scanner automaton
   element returned yields newstate when divided by ten
                and yields action code when remainder from division by ten is taken.
 */
char *cclass[] = {0, ALPHAUPPER ALPHALOWER, HIMINUS, DIGIT, DOT, SPACE};
int wdtab[][sizeof cclass/sizeof*cclass] = {
    /*char-class*/
    /*0     a     -     d     .     s    */ /*    state  */
    { 30+2, 20+2, 10+2, 10+2, 10+2, 0+0  }, /*  0 init   */
    { 30+1, 20+1, 10+0, 10+0, 10+0, 10+0 }, /* 10 number */
    { 30+1, 20+0, 10+1, 10+1, 30+1, 0+1  }, /* 20 name   */
    { 30+1, 20+1, 10+1, 10+1, 30+1, 0+1  }, /* 30 other  */
    /*{newstate+action,...}
      action=0:do nothing
      action=1:generate a token and reset start index
      action=2:reset start index
     */
};

/* construct new object giving start and end string positions */
#define emit(a,b,c) (*z++=(I)newsymb(s+(a),(b)-a,c)); 

/*
   scan expression for alphabetic and numeric parts 
   delimited by whitespace
 */
I *wd(C *s){
    I a,b,n=strlen(s),*e=ma(n+1),*z=e;
    int i,j,i_,state,oldstate;
    C c, *cp;
    state=0;
    for(i=0;i<n;i++){
        c=s[i];
        //P("'%c'\n",c);
        a=0;
        for(i_=1;i_<(sizeof cclass/sizeof*cclass);i_++) {
            if (cp=strchr(cclass[i_],c)) {
                a=i_;
                break;
            }
        }
        b=wdtab[state][a];
        //P("%d\n",b);
        oldstate=state;
        state=b/10;
        switch(b%10){ //encoded actions
        case 0: break;
        case 1: emit(j,i,oldstate);  // generate a token (and)
                /*fallthrough*/
        case 2: j=i; break;          // reset start index
        }
    }
    *z++=0;
    R e;}


/* main{REPL} */


/*
   setup special terminal mode if connected to a terminal
   perform read-eval-print loop
 */
int main(){
    C *s=NULL;
    int n=0;
    C *prompt="\t";
    ST_INIT; /* initialize symbol table root value */
    BANK_INIT;
    FIXNUM_INIT;
    FLONUM_INIT;
    MPINT_INIT;
    infinite = flo(strtod("inf", NULL));

    if (isatty(fileno(stdin))) specialtty();
    while(getln(prompt,&s,&n))
        pr(ex(wd(s)));
    if (isatty(fileno(stdin))) restoretty();

    R 0;
}

/* eof */
