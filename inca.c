#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char C;
typedef intptr_t INT;
typedef struct a{INT t,r,d[3],p[2];} *ARC;
//t (type): t=0:regular array, t=1:boxed, t=2:captured command
//r (rank): significant dims in d
//d (dims): dimensions of p
//p ("physical" data)is a flexible array member. why [2]? ??!

void pr(ARC w);
ARC ex(INT*e);
ARC reduce(ARC w,INT f);
ARC transpose(ARC w,INT f);
ARC dot(ARC a,INT f,INT g,ARC w);

#define V1(f) ARC f(ARC w)
#define V2(f) ARC f(ARC a,ARC w)
V2(plus);
V2(minus);

#define DO(n,x) {INT i=0,_n=(n);for(;i<_n;++i){x;}}

//malloc an array
INT ma(INT n){return (INT)malloc(n*sizeof(INT));}

//copy n ints from s to d
void mv(INT*d,INT*s,INT n){DO(n,d[i]=s[i]);}

//table rank (total # of elements in array) >= 1
INT tr(INT r,INT*d){INT z=1;DO(r,z=z*d[i]);return z;}

//construct(malloc) array with dims
ARC ga(INT t,INT r,INT*d){ARC z=(ARC)ma(5+tr(r,d));z->t=t,z->r=r,mv(z->d,d,r);return z;}

//dup an array structure and contents
ARC cp(ARC w){INT n=tr(w->r,w->d);
    ARC z=ga(w->t,w->r,w->d);
    mv(z->p,w->p,n);
    return z;
}

// '<' pack an array into a scalar (enclose)
V1(box){ARC z=ga(1,0,0);*z->p=(INT)w;return z;}

// '>' unpack an array from a scalar (disclose)
V1(unbox){
    if (w->t) {
        if (w->r){
            int i,r,n=tr(w->r,w->d);
            ARC t=(ARC)*w->p;
            ARC z=ga(0,2,(INT[]){n,t->d[0],t->d[1]});
            for(i=0,r=0; i<n; i++){
                int tn;
                t=(ARC)w->p[i];
                mv(z->p+r, t->p, tn=tr(t->r,t->d));
                r+=tn;
            }
            return z;
        } else {
            return (ARC)*w->p;
        }
    } else
        return w;
}

/* '{' extract row(s) from matrix or scalar from vector */
V2(from){
    if (a->r){ /* a non-scalar */
        INT an=tr(a->r,a->d),r=w->r,*d=w->d+1,n=tr(r-1,d);
        ARC z=ga(w->t,r,(INT[]){an,d[0],d[1]});
        DO(an,mv(z->p+i,w->p+(n*a->p[i]),n));
        return z;
    } else {
        INT r=w->r-1,*d=w->d+1,n=tr(r,d);
        ARC z=ga(w->t,r,d);
        mv(z->p,w->p+(n**a->p),n);
        return z;
    }
}

//Perform C operation "op" upon left arg 'a' and right arg 'w' (alpha and wmega)
//recursing to the calling function func for boxed args
//allow a or w to be scalar
//nb. ->t is the type field (1==boxed,0==normal)
//    ->r is the "rank" field, how many dimensions of data (0..3)?
//zero rank means arg is a scalar and ->p[0] is the value.
//non-zero rank means arg is an array and ->p[0..n] contain the values
//    where n is the total size of the array (Product of dims 0..r)
#define OP(op,func,prehook) \
    INT r=w->r,*d=w->d,n=tr(r,d);ARC z; \
    ARC ind=(ARC)noun('0'),zero=(ARC)noun('0'); \
    INT wn=tr(w->r-1,w->d+1); \
    INT an=tr(a->r-1,a->d+1); \
    /*printf("%s %d %d\n", #func, *a->p, *w->p);*/ \
    /*pr(a);*/ \
    /*pr(w);*/ \
    prehook; \
    if(a->t && w->t) \
        z = (ARC)box(func(unbox(a),unbox(w))); \
    else if(a->t) \
        z = (ARC)func(unbox(a),w); \
    else if(w->t) \
        z = (ARC)func(a,unbox(w)); \
    else if(a->r>1 && w->r>1) { /* both higher rank */\
        if (a->d[0]>1 && w->d[0]>1) { \
            r=a->r>w->r?a->r:w->r; \
            d=(INT[]){a->d[0],w->d[1],w->d[2]}; \
            n=d[0]; \
            z=ga(0,r,d); \
            DO(n,*ind->p=i;mv(z->p+i*wn,func(from(ind, a), from(ind, w))->p,wn)) \
        } else if (w->d[0]>1) { \
            z=ga(0,r,d); \
            n=d[0]; \
            /*printf("w->d[0]>1\n");*/ \
            DO(n,*ind->p=i;mv(z->p+i*wn,func(from(zero, a), from(ind, w))->p,wn)) \
        } else if (a->d[0]>1) { \
            r=a->r;d=a->d; \
            n=d[0]; \
            z=ga(0,r,(INT[]){d[0],w->d[1],w->d[2]}); \
            /*printf("a->d[0]>1\n");*/ \
            DO(n,*ind->p=i;mv(z->p+i*wn,func(from(ind, a), from(zero, w))->p,wn)) \
        } else { \
            /*z=ga(0,r,d);*/ \
            /*DO(n,z->p[i]=(a->p[i]) op (w->p[i]))*/ \
            z=func(from(zero,a), from(zero,w)); \
        } \
    } else if (w->r>1) { /* w higher but not a */\
        n=d[0]; \
        z=ga(0,r,d); \
        /*printf("w->d[0]>1\n");*/ \
        if (a->r) \
            DO(n,*ind->p=i;mv(z->p+i*wn,func(from(zero, a), from(ind, w))->p,wn)) \
        else \
            DO(n,*ind->p=i;mv(z->p+i*wn,func(a, from(ind, w))->p,wn)) \
    } else if (a->r>1) { /* a higher but not w */\
        r=a->r;d=a->d;n=d[0]; \
        z=ga(0,r,d); \
        /*printf("a->d[0]>1\n");*/ \
        if (w->r) \
            DO(n,*ind->p=i;mv(z->p+i*wn,func(from(ind, a), from(zero, w))->p,wn)) \
        else \
            DO(n,*ind->p=i;mv(z->p+i*wn,func(from(ind, a), w)->p,wn)) \
    } else if(a->r && w->r) { /* both rank 1 */\
        if (a->d[0]>1 && w->d[0]>1){ \
            z=ga(0,r,d); \
            DO(n,z->p[i]=(a->p[i]) op (w->p[i])) \
            /*z=func(from(zero,a), from(zero,w));*/ \
        } else if (w->d[0]>1) { \
            z=ga(0,r,d); \
            DO(n,z->p[i]=(*a->p) op (w->p[i])) \
        } else if (a->d[0]>1) { \
            z=ga(0,r,d); \
            DO(n,z->p[i]=(a->p[i]) op (*w->p)) \
        } else { \
            z=ga(0,r,d); \
            DO(n,*z->p=(*a->p) op (*w->p)) \
        } \
    } else if(w->r) { /* w rank 1 but not a */\
        z=ga(0,r,d); \
        /*DO(n,*ind->p=i;mv(z->p+i*wn,func(a, from(ind, w))->p,wn))*/ \
        DO(n,z->p[i]=(*a->p) op (w->p[i])) \
    } else if(a->r) { /* w is scalar: allocate z with dims from a */ \
        /*{ n=tr(a->r,a->d); z=ga(0,a->r,a->d); DO(n,*ind->p=i;mv(z->p+i*an,func(from(ind, a), w)->p,an)) }*/ \
        { n=tr(a->r,a->d); z=ga(0,a->r,a->d);DO(n,z->p[i]=(a->p[i]) op (*w->p)) } \
    } else { /* both scalar */\
        z=ga(0,0,0); \
        *z->p = (*a->p) op (*w->p); \
    } \
    /*printf("=>\n");*/ \
    /*pr(z);*/ \
    return z;

//Perform C math function "op" upon args a and w
//recursing for boxed args
#define OPF(op,func,prehook) \
    INT r=w->r,*d=w->d,n=tr(r,d);ARC z=ga(0,r,d); \
    prehook; \
    if(a->t && w->t) \
        z = (ARC)box(func(unbox(a),unbox(w))); \
    else if(a->t) \
        z = (ARC)func(unbox(a),w); \
    else if(w->t) \
        z = (ARC)func(a,unbox(w)); \
    else if(a->r && w->r) \
        DO(n,z->p[i]=op(a->p[i], w->p[i])) \
    else if(w->r) \
        DO(n,z->p[i]=op(*a->p, w->p[i])) \
    else if(a->r) \
        { n=tr(a->r,a->d); z=ga(0,a->r,a->d); DO(n,z->p[i]=op(a->p[i], *w->p)) } \
    else *z->p =op( *a->p, *w->p); \
    return z;

/* '~' iota: generate j=0 index vector */
V1(iota){INT n=*w->p;ARC z=ga(0,1,&n);DO(n,z->p[i]=i);return z;}

/* arithmetic/logical functions */

/* '+' */
V2(plus){
    OP(+,plus,)
}

/* '-' */
V2(minus){
    OP(-,minus,)
}

/* '&' */
V2(and){
    OP(&&,and,)
}

/* '^' */
V2(or){
    OP(||,or,)
}

/* '.' */
V2(times){
    OP(*,times,)
}

/* '*' */
V2(power){
    OPF(pow,power,)
}

/* '%' */
V2(divide){
    OP(/,divide,)
}

/* '=' */
V2(equal){
    OP(==,equal,)
}

/* '!' */
V2(unequal){
    OP(!=,unequal,)
}

/* '<' */
V2(less){
    OP(<,less,)
}

/* '>' */
V2(greater){
    OP(>,greater,)
}

/* '|' */
V2(modulus){
    OP(%,modulus, ARC t=a;a=w;w=t;)  //swap args: w%a
}

/* '|' */
V1(absolute){INT r=w->r,*d=w->d,n=tr(r,d);ARC z=ga(0,r,d);
    DO(n,z->p[i]=abs(w->p[i]));return z;
}

/* '!' */
V1(not){INT r=w->r,*d=w->d,n=tr(r,d);ARC z=ga(0,r,d);
    DO(n,z->p[i]=!w->p[i]);return z;}


/* ':' */
V2(match){INT n;
    if (!!a->r | !!w->r){
        if (a->r == w->r){
            if ((n=tr(a->r,a->d)) == tr(w->r,w->d)){
                DO(n,if(a->p[i]!=w->p[i])return (ARC)noun('0'));
                return (ARC)noun('1');
            }
        }
        return (ARC)noun('0');
    } else {
        return (ARC)noun('0' + (*a->p==*w->p));
    }
}

/* ',' reshape w into a vector */
V1(ravel){INT n=tr(w->r,w->d);ARC z=ga(0,1,&n);DO(n,z->p[i]=w->p[i]);return z;}

/* ',' catenate two arrays (yields a rank=1 vector) */
V2(cat){INT an=tr(a->r,a->d),wn=tr(w->r,w->d),n=an+wn;
    ARC z=ga(w->t,1,&n);mv(z->p,a->p,an);mv(z->p+an,w->p,wn);return z;}

/* ';' catenate two "rows", promoting (and padding) scalars and vectors as necessary */
V2(rowcat){ARC z;INT an;ARC b;
    /*
    printf("a->r=%d a->d=%d,%d,%d, w->r=%d w->d=%d,%d,%d\n",
            a->r, a->d[0], a->d[1], a->d[2],
            w->r, w->d[0], w->d[1], w->d[2]);
            */
    switch(a->r){
    case 0:
        switch(w->r){
        case 0: z=ga(w->t,2,(INT[]){2,1}); break;
        case 1: z=ga(w->t,2,(INT[]){2,w->d[0]});
                b=ga(w->t,1,w->d);
                *b->p=*a->p;
                memset(b->p+1,0,tr(b->r,b->d)-1);
                a=b;
                break;
        default: z=ga(w->t,w->r,(INT[]){w->d[0]+1,w->d[1],w->d[2]});
                 b=ga(w->t,w->r-1,(INT[]){w->d[1],w->d[2]});
                 *b->p=*a->p;
                 memset(b->p+1,0,tr(b->r,b->d)-1);
                 a=b;
                 break;
        }
        break;
    case 1:
        switch(w->r){
        case 0: z=ga(w->t,2,(INT[]){2,a->d[0]}); break;
        case 1: z=ga(w->t,2,(INT[]){2,a->d[0]}); break;
        default: z=ga(w->t,w->r,(INT[]){w->d[0]+1,w->d[1],w->d[2]}); break;
        }
        break;
    default:
        switch(w->r){
        case 0: z=ga(w->t,a->r,(INT[]){a->d[0]+1,a->d[1],a->d[2]}); break;
        case 1: z=ga(w->t,a->r,(INT[]){a->d[0]+1,a->d[1],a->d[2]}); break;
        default: z=ga(w->t,a->r,(INT[]){a->d[0]+w->d[0],a->d[1],a->d[2]}); break;
        }
        break;
    }
    mv(z->p,a->p,an=tr(a->r,a->d));
    mv(z->p+an,w->p,tr(w->r,w->d));
    /*
    printf("z->r=%d z->d=%d,%d,%d\n",
            z->r, z->d[0], z->d[1], z->d[2]);
            */
    return z;
}

/* '#' use data in a as new dims for array containing data from w */
V2(reshape){INT r=a->r?*a->d:1,n=tr(r,a->p),wn=tr(w->r,w->d);
    ARC z=ga(w->t,r,a->p);mv(z->p,w->p,wn=n);
    if(n-=wn)mv(z->p+wn,z->p,n);return z;}

/* '#' return the dims of w */
V1(shape){ARC z=ga(0,1,&w->r);mv(z->p,w->d,w->r);return z;}

/* '+' return w unmolested */
V1(identity){return w;}

/* '{' suspiciously similar to shape */
V1(size){ARC z=ga(0,1,&w->r);mv(z->p,w->d,w->r);return z;}

/* '/' catenate elements of w selected by nonzero elements of a */
V2(compress){INT an=tr(a->r,a->d),n=0,j=0;
    DO(an,if(a->p[i])++n);
    ARC z=ga(0,1,&n);
    DO(an,if(a->p[i])z->p[j++]=w->p[i]);
    return z;
}

/* '\' fill array with 0 or element from w by nonzero elements of a */
V2(expand){
    INT an=tr(a->r,a->d);
    ARC z=ga(0,a->r,a->d);
    DO(an,z->p[i]=a->p[i]?w->p[i]:0);
    return z;
}

/* '~' dyadic iota: find index of a in w */
V2(find){INT wn=tr(w->r,w->d);ARC z=0; INT i;
    if(a->r==0){
        for(i=0;i<wn;i++){
            if(*a->p==w->p[i]){
                z=ga(0,0,0);
                *z->p=i;
                return z;
            }
        }
    }else{
        INT an=tr(a->r,a->d);
        ARC ind=(ARC)noun('0');
        z=ga(0,a->r,a->d);
        for(i=0;i<an;i++){
            *ind->p=i;
            ind=find(from(ind,a),w);
            z->p[i]=*ind->p;
        }
    }
    return z;
}
/* '@' reverse elements in w */
V1(reverse){INT n=tr(w->r,w->d);ARC z=ga(0,w->r,w->d);
    DO(n,z->p[i]=w->p[n-1-i]);
    return z;
}

/* '@' rotate array w by scalar a, positive a rotates to the left */
V2(rotate){INT n=tr(w->r,w->d),ap=*a->p;ARC z=ga(0,w->r,w->d);
    if (ap < 0){
        ap = n + ap;
    }
    ap %= n;
    mv(z->p, w->p+ap, n-ap);
    mv(z->p+(n-ap), w->p, ap);
    return z;
}

/*
V2(encode) '['
V2(decode) ']'
V2(max)
V2(min)
remaining symbols '$' '\'' '"' '_'
*/

/* ';' */
V1(execute){
    ARC zero = (ARC)noun('0');
    if (w->t != 2){
        printf("execute requires a command");
        return zero;
    }
    if (w->r>1){ //execute each "row", return result of last row
        INT n=w->d[0];
        ARC z;
        ARC ind = zero;
        DO(n,*ind->p=i;z=ex(from(ind,w)->p));
        return z;
    }
    return ex(w->p);
}

INT st[28];
INT qp(a){return a>='`'&&a<='z';}  /* int a is a variable iff '`' <= a <= 'z'. nb. '`'=='a'-1 */

#define reducemask 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,0
#define dotmask 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,0
#define transposemask PLUS,MINUS,SLASH,BACKSLASH,DOT,BAR,LANG,RANG,0

enum   {ZERO,          PLUS,   LBRACE, TILDE, LANG, HASH,    COMMA, RANG,  MINUS, STAR,  PERCENT, BAR,      RBRACE,
                     AND, CARET,  BANG, SLASH,    DOT,   BACKSLASH, QUOTE, DBLQUOTE,    AT,        EQUAL,  SEMI,   COLON, NV};
C vt[]={               '+',    '{',    '~',   '<',  '#',     ',',   '>',   '-',   '*',   '%',     '|',      '}',
                     '&', '^',    '!',  '/',      '.',   '\\',      '\'',  '\"',        '@',       '=',    ';',    ':',   0};
ARC(*vd[])(ARC,ARC)={0,plus,   from,   find,  less, reshape, cat, greater, minus, power, divide,  modulus,  0,
                     and, or,     unequal, compress, times, expand, 0,     0,           rotate,    equal,  rowcat, match,0},
 (*vm[])(ARC)={0,      identity, size, iota,  box,  shape,   ravel, unbox, 0,     0,     0,       absolute, 0,
                     0,   0,      not,  0,        0,     0,         0,     0,           reverse,   0,      execute,0,   0};
ARC(*od[])(ARC,INT,INT,ARC)={0,0,0,    0,     0,    0,       0,     0,     0,     0,     0,       0,        0,
                     0,   0,      0,    0,        dot,   0,         0,     0,           0,         0,      0,      0,     0},
 (*om[])(ARC,INT)={0,  0,      0,      0,     0,    0,       0,     0,     0,     0,     0,       0,        0,
                     0,   0,      0,    reduce,   0,     0,         0,     0,           transpose, 0,      0,      0,     0};
C odv[NV+1][NV+1]={{0},{0},    {0},   {0},    {0},  {0},     {0},   {0},   {0},   {0},   {0},     {0},     {0},
                     {0}, {0},    {0},  {0},      {dotmask},{0},    {0},   {0},         {0},       {0},    {0},    {0},   {0}};
C omv[NV+1][NV+1]={{0},{0},    {0},   {0},    {0},  {0},     {0},   {0},   {0},   {0},   {0},     {0},     {0},
                     {0}, {0},    {0},  {reducemask},{0},   {0},    {0},   {0},         {transposemask},{0},{0},   {0},   {0}};
INT vid[]={0,          '0',    0,      0,     0,    0,       '0',   0,     '0',   '2',   '1',     0,       0,
                     '1', '0',    0,    0,        '0',   0,         0,     0,           0,         0,      0,      0,     0};
INT qv(unsigned a){return a<'`'&&a<NV&&(vd[a]||vm[a]);}
INT qo(unsigned a){return a<'`'&&a<NV&&(od[a]||om[a]);}

/* operators: monadic operator / (reduce) applies to a function on its left
              monadic operator @ transpose function selected by function on its left
              dyadic operator . (dot) applies to functions on left and right */

/*
   '.@' identity transpose
   '-@' vertical transpose
   '|@' horizontal transpose
   '/@' transpose about the line y=-x 
   '\@' transpose about the line y=x
   '+@' horizontal, then vertical
   '<@' horizontal, then y=-x
   '>@' horizonatl, then y=x
*/
ARC transpose(ARC w,INT f){
    ARC z;INT j;INT t;
    if (w->r == 0){ // w is a scalar, promote to 1x1 matrix
        z=ga(0,2,(INT[]){1,1});
        z->p[0] = w->p[0];
        w = z;
    }
    if (w->r == 1){ // w is a vector, promote to 1-row matrix
        z=ga(0,2,(INT[]){1,w->d[0]});
        mv(z->p,w->p,w->d[0]);
        w = z;
    }
    switch(f){
    default: printf("error bad function arg to operator @: try .-|/\\+<> ie. -@ or >@ \n");
    case DOT:       // . identity
             //z=ga(0,w->r,w->d);DO(tr(w->r,w->d),z->p[i]=w->p[i]); break;
             z=ga(0,w->r,w->d);DO(z->d[1],j=i;DO(z->d[0],z->p[i*w->d[1]+j]=w->p[i*w->d[1]+j])); break;
    case MINUS:     // - vertical 
             z=ga(0,w->r,w->d);DO(z->d[1],j=i;DO(z->d[0],z->p[i*w->d[1]+j]=w->p[(w->d[0]-1-i)*w->d[1]+j])); break;
    case BAR:       // | horizontal
             z=ga(0,w->r,w->d);DO(z->d[1],j=i;DO(z->d[0],z->p[i*w->d[1]+j]=w->p[i*w->d[1]+w->d[1]-1-j])); break;
    case SLASH:     // / y=-x
             z=ga(0,w->r,w->d); t=z->d[0];z->d[0]=z->d[1];z->d[1]=t;
             DO(z->d[0],j=i;DO(z->d[1],z->p[j*w->d[0]+i]=w->p[(w->d[0]-1-i)*w->d[1]+w->d[1]-1-j])); break;
    case BACKSLASH: // \ y=x
             z=ga(0,w->r,w->d); t=z->d[0];z->d[0]=z->d[1];z->d[1]=t;
             DO(z->d[0],j=i;DO(z->d[1],z->p[j*w->d[0]+i]=w->p[i*w->d[1]+j])); break;
    case PLUS:      // + vert+horz
             z=transpose(transpose(w,MINUS),BAR); break;
    case LANG:      // < slash+horz
             z=transpose(transpose(w,MINUS),SLASH); break;
    case RANG:      // > backslash+horz
             z=transpose(transpose(w,MINUS),BACKSLASH); break;
    }
    return z;
}

/* 'f/' perform right-to-left reduction over array w using function f */
ARC reduce(ARC w,INT f){
    INT r=w->r,*d=w->d,n=tr(r,d);
    //printf("reduce(%c): %u %u\n",vt[f-1],w,f); pr(w);
    ARC z=(ARC)noun(vid[f]);   /* default left arg for w=scalar */
    //printf("n = %d, z = %d, *w->p = %d\n", n, *z->p, *w->p); fflush(0);
    if (w->r){             /* w!=scalar */
        ARC ind = (ARC)noun('0'); /* ind is a scalar for use with from() */
        *ind->p = n-1;        /* set payload of ind to last element of w */
        //printf("*ind->p=%d\n",*ind->p);
        z = from(ind,w);      /* get last element of w */
        //printf("z=%d,*z->p=%d\n",z,*z->p);
        //printf("w->p[n-1]=%d\n",w->p[n-1]);
        //z = w->p[n-1];       /* loop right->left through w. f(w[0],f(...f(w[n-3],f(w[n-2],w[n-1])))) */
        DO(n-1,*ind->p=n-2-i;z=(*vd[f])(from(ind,w),z);/*printf("z = %d\n", *z->p);*/);
    } else {
        z=(*vd[f])(z,w);  /* ie. return f(vid[f],w) if w is a scalar */
    }
    return z;
}

/* 'f.g' perform general matrix multiplication Af.gW ::== f/Ag'W
   f-reduce rows of (A {g-function} transpose-of-W) */
ARC dot(ARC a,INT f,INT g,ARC w){
    if (f == AT)
        //return vd[g](transpose(transpose(a,BACKSLASH),BACKSLASH),transpose(w,BACKSLASH));
        return vd[g](transpose(a,BACKSLASH),transpose(transpose(w,BACKSLASH),BACKSLASH));
    else
        return reduce(vd[g](transpose(a,BACKSLASH),transpose(transpose(w,BACKSLASH),BACKSLASH)),f);
    //return reduce((*vd[g])(transpose(transpose(a,BACKSLASH),BACKSLASH),transpose(w,BACKSLASH)),f);
    //return reduce((*vd[g])(a,transpose(w,BACKSLASH)),f);
    //return reduce((*vd[g])(transpose(a,BACKSLASH),w),f);
    //return reduce((*vd[g])(a,w),f);
}

void pi(INT i){printf("%d ",i);}
void nl(){printf("\n");}
void no(){}
void pv(INT i){
    if (i==0)
        printf("\n");
    else if (abs(i) < NV)
        printf("%c", vt[abs(i)-1]);
    else if (abs(i) < 255) {
        if (isprint(i))
            printf("%c", i);
        else
            printf("0%o", i);
    } else
        //pr((ARC)i);
        printf("%d", ((ARC)i)->p[0]);
}
void pr(ARC w){INT r=w->r,*d=w->d,n=tr(r,d);INT j,k;
    void (*p)(INT) = pi;
    void (*eol)() = nl;
    DO(r,p(d[i]));eol();
    if(w->t == 2){
        p = pv;
        eol = no;
    }
    if(w->t % 2)DO(n,printf("< ");pr((ARC)w->p[i]))else
    switch(r){
    case 0: p(*w->p);eol();break;
    case 1: DO(n,p(w->p[i]));eol(); break;
    case 2: DO(d[0], j=i;DO(d[1],p(w->p[j*d[1]+i]));eol();); break;
    case 3: DO(d[0], k=i;DO(d[1], j=i;DO(d[2],p(w->p[(k*d[1]+j)*d[2] +i]));eol();)eol();eol();); break;
    }
}

INT digits(INT w){
    INT r=labs(w)>1?ceil(log10((double)w+1)):1;
    //printf("digits(%d)=%d\n", w, r);
    return r;
}

/* execute an encoded expression, an "int" string, terminated by a zero int.
 */
ARC ex(INT*e){INT a=*e,w=e[1],d=w,o;
    while(a==' '){ /* eat space */
        ++e;
        a=*e;
        d=w=e[1];
    }
EX:
    //{int i;for(i=0;e[i];i++)printf("%d ",e[i]);printf("\n");} // dump command-"string"
    if (a==COLON){ /* monadic ':' denotes capture of remaining command string */
        int i;
        ARC _a;
        ++e;
        for(i=0;e[i];i++) ;
        ++i;
        a=(INT)ga(2,1,&i);
        _a=(ARC)a;
        mv(_a->p,e,i);
        return (ARC)a;
    }
    if (a=='('){ /* parenthesized subexpression */
        int i,p;
        for(i=1,p=1;p&&e[i];i++){ /* find matching close paren */
            switch(e[i]){
            case '(': ++p; break;
            case ')': --p; break;
            }
            //printf("%d(%d) %d ",i, p, e[i]);
        }
        //printf("%d\n", i);
        if (e[i-1] != ')'){
            printf("err: unmatched parens\n");
            return (ARC)noun('0');
        }
        //e[i-1]=0;
        //a=(INT)ex(e+1);
        INT t = ma(i-1); /* copy subexpression and zero-terminate */
        mv((INT*)t,e+1,i-1);
        ((INT*)t)[i-2]=0;
        a=(INT)ex((INT*)t); /* a=ex(subexpr) */
        e+=i-1;
        d=w=e[1];
        goto EX;
    }
    /* if a is a variable followed by Left ANGle bracket, assign to variable result of ex(remainder) */
    if(qp(a)&&w==LANG)return (ARC)(st[a-'`']=(INT)ex(e+2)); //use '<' for assignment

    if (qv(a)){INT m=a;  /* if a is a verb, it is in monadic position, so call it 'm' */
        while (w==' '){ /* eat space */
            ++e;
            d=w=e[1];
        }
        if (qo(w) && om[w] && strchr(omv[w],a)){    /* if w is a monadic operator and a is in omv[m] */
            return (*om[w])(ex(e+2),a);  /* call operator function with function a on result of ex(rem)*/
        }
        if (vm[m]==0&&vid[m]){  /* if no monadic verb, but there is a verb "identity" element, */
            a=noun(vid[m]);     /* load the identity element */
            return (*vd[m])((ARC)a,ex(e+1));  /* call dyadic verb with w=ex(rem) */
        }
        return (*vm[m])(ex(e+1));  /* call monadic verb with w=ex(rem) */
    }

    if (w){  /* both ifs have failed. so w is not assignment and a is not a function */
        if (qv(w)){  /* if w is a verb, */
            while (e[2]==' '){ /* eat space */
                ++e;
            }
            if (qo(o=e[2]) && od[o] && strchr(odv[o],w)){/* if followed by a dyadic operator and v is in odv[d] */
                while (e[3]==' '){ /* eat space */
                    ++e;
                }
                w=(INT)ex(e+4);   /* w=ex(rem) */
                if (qp(a)) a=(INT)cp((ARC)st[a-'`']); /* ex(w) may have assigned to var a; if so, load it */
                return (*od[o])((ARC)a,d,e[3],(ARC)w);/*call dyadic operator e[2] with f=d(=w=e[1]),g=e[3] */
            }
            w=(INT)ex(e+2);   /* ::not followed by an operator, w=ex(rem) */
            //printf("lookup a\n");
            if (qp(a)) a=st[a-'`'];  /* if a is a var, load it now */
            return (*vd[d])((ARC)a,(ARC)w);  /* call dyadic function */
        }
        if (qp(a)) a=(INT)cp((ARC)st[a-'`']);  /* a is not function, w is not a function */

        if (w==' '){ //e:"a bc..." A=cat(a,b) concatenate space-delimited integer vector
            ARC _d,_w;
            w=e[2];  /* read past the space */
            if (w){  /* something there? */
                if (qv(w)){ /* if it's a verb, eat the space */
                    ++e;
                    d=w=e[1];
                    goto EX;
                }
                e+=2;                  /*e:"Ac..."   advance the int-string pointer */
                if (w=='('){ /* parenthesized subexpression */
                    int i,p;
                    for(i=1,p=1;p&&e[i];i++){ /* find matching close paren */
                        switch(e[i]){
                        case '(': ++p; break;
                        case ')': --p; break;
                        }
                        //printf("%d(%d) %d ",i, p, e[i]);
                    }
                    //printf("%d\n", i);
                    if (e[i-1] != ')'){
                        printf("err: unmatched parens\n");
                        return (ARC)noun('0');
                    }
                    //e[i-1]=0;
                    //a=(INT)ex(e+1);
                    INT t = ma(i-1); /* copy subexpression and zero-terminate */
                    mv((INT*)t,e+1,i-1);
                    ((INT*)t)[i-2]=0;
                    w=(INT)ex((INT*)t); /* a=ex(subexpr) */
                    e+=i-1;
                    //pr(a);
                    //pr(w);
                    a=(INT)cat((ARC)a,(ARC)w);  // cat new integer w into integer vector a
                    d=w=e[1];             /* update d and w to next int */
                    goto EX;              /* tail-recurse */
                }

                if (qp(w)) w=(INT)cp((ARC)st[w-'`']); /* if it's a variable, substitute it */

                d=e[1];                /* d is the next int */
                while (d&&!qv(d)&&d!=' '){  /* if nonzero, not a verb, not a space, accumulate integer*/
                    if (qp(d)) d=(INT)cp((ARC)(st[d-'`']));  /* interpolate variable d? */
                    _d=(ARC)d;                          /* treat d like a pointer */
                    if (_d->r==0){                    /* if scalar */
                        _w=(ARC)w;                      /* treat w like a pointer */
                        *_d->p+=*_w->p*pow(10,digits(*_d->p));            /* d = d + w*10^digits(d) */
                        w=d;                          /* w = d */
                        ++e;                          /* advance e (int-string pointer) */
                        d=e[1];                       /* d is the next int */
                        continue;                     /* loop */
                    }
                    break;                            /* break if d not scalar */
                }
                a=(INT)cat((ARC)a,(ARC)w);  // cat new integer w into integer vector a
                d=w=e[1];             /* update d and w to next int */
                goto EX;              /* tail-recurse */
            }
        } else {  // not verb, not a space  // accumulate integer
            ARC _a,_w;
            _a=(ARC)a;
            if (qp(w)) w=(INT)cp((ARC)(st[w-'`']));  /* interpolate variable w? */
            _w=(ARC)w;  /* treat a and w like pointers */
            if (_a->r==0 && _w->r==0){  /* a and w both scalar */
                *_w->p+=*_a->p*pow(10,digits(*_w->p));     /* w = w + a*10^digits(d) */
                a=w;                   /* a = w */
                ++e;                   /* advance e (int-string pointer) */
                d=w=e[1];              /* update d and w to next int */
                goto EX;               /* tail-recurse */
            }
        }
    }
    if (qp(a)) a=(INT)cp((ARC)st[a-'`']); /*a not a function, w is zero (end-string). load var a if a is a var */
    if (a==0)return (ARC)noun('0');  /* if somehow a is zero (the end of string), return scalar zero */
    return (ARC)a;             /* return a, whatever it is now, a non-null "something", hopefully */
}

/* construct an integer string from from command string */
INT noun(c){ARC z;if(c<'0'||c>'9')return 0;z=ga(0,0,0);*z->p=c-'0';return (INT)z;} /* constr (ptr to) scalar */
INT verb(c){INT i=0;for(;vt[i];)if(vt[i++]==c)return i;return 0;}                /* verbs are low integers */
INT*wd(C*s){INT a,n=strlen(s),*e=(INT*)ma(n+1);C c;       /* allocate int-string */
    DO(n,e[i]=(a=noun(c=s[i]))?a:(a=verb(c))?a:c);/* replace numbers with scalars and funcs with ints*/
    e[n]=0;return e;}    /* zero-terminate. nb. variables ('`'-'z') remain as ascii values */

int main(int argc, char **argv){
    C s[999];
    int i;
    int an=0;
    for (i=0; i < sizeof(st)/sizeof*st; i++)
        st[i]=noun('0');
    for (i=1; i < argc; i++)
        an=an<strlen(argv[i])?strlen(argv[i]):an;
    st[1]=(INT)ga(0,1,(INT[]){argc-1});
    for (i=1; i < argc; i++){
        ARC z=ga(2,1,(INT[]){strlen(argv[i])+1});
        mv(z->p,wd(argv[i]),strlen(argv[i])+1);
        z=box(z);
        //pr(z);
        //pr(unbox(z));
        ((ARC)st[1])->p[i-1]=(INT)z;
    }
    //printf("sizeof(intptr_t)=%u\n",sizeof(intptr_t));
    while(putchar('\t'),gets(s))          /* var('`')=REPL */
        pr((ARC)(st[0]=(INT)ex(wd(s))));  /* nb. st['`'-'`'] */
    return 0;
}
