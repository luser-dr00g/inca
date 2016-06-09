#include <errno.h>

typedef int token;
typedef int state;
typedef int state_and_action_code;
#define state_from(s_a_a_c) ((s_a_a_c)/10)
#define action_from(s_a_a_c) ((s_a_a_c)%10)

/*
 * The transition table and state set
 *
 * Each state embodies a certain amount of "knowledge" 
 * about what sort of token has been encountered. 
 * The dot character '.' causes a great deal of trouble
 * since it is heavily overloaded. If the dot has a digit
 * on either or both sides, then it is considered a decimal
 * point separating the integer and fractional parts of a
 * floating-point number. 
 * Otherwise, the dot is considered part of an identifier.
 *
 * Note, the state enum codes are 10* the corresponding table index.
 */
enum state {
    ini=0,  //indeterminate
    min=10, //initial minus                          -
    dot=20, //initial dot                            . 
    num=30, //integer                                0 
    dit=40, //medial dot                             0.
    fra=50, //fraction                               0.0
    str=60, //initial quote                          '    ''
    quo=70, //end or escape quote                    'aaaa'
    oth=80, //identifier or other symbol             a+
    dut=90, //trailing dot                           +.
    tra=100, //trailing minus                         q-
    sng=110, //copula or other self-delimiting symbol ()
};

#define NUM_CLASSES \
  sizeof((unsigned char[]) \
       { 0,     '-',   '0',   '.',   '\'',  '(',   '?',   ' ',  0x2190, '\r'})
state_and_action_code wdtab[][NUM_CLASSES] = {
/* state*/
/* | *//*character class*/
/* V *//*none   minus  0-9    .      '      ()     oth    sp     <-     \r    */
/* 0 */{ oth+2, min+2, num+2, dot+2, str+2, sng+2, sng+2, ini+0, sng+2, ini+4 },
/* 10*/{ oth+0, min+1, num+0, dot+0, str+1, sng+1, sng+1, ini+1, sng+1, ini+4 },
/* 20*/{ oth+0, min+1, fra+0, oth+0, str+1, sng+1, sng+1, ini+1, sng+1, ini+4 },
/* 30*/{ oth+1, min+1, num+0, dit+0, str+1, sng+1, sng+1, ini+1, sng+1, ini+4 },
/* 40*/{ oth+0, min+1, num+0, dut+1, str+1, sng+1, sng+1, ini+1, sng+1, ini+4 },
/* 50*/{ oth+1, min+1, fra+0, dot+1, str+1, sng+1, sng+1, ini+1, sng+1, ini+4 },
/* 60*/{ str+0, str+0, str+0, str+0, quo+0, str+0, str+0, str+0, str+0, ini+4 },
/* 70*/{ oth+1, min+1, num+1, dot+1, str+0, sng+1, sng+1, ini+1, sng+1, ini+4 },
/* 80*/{ oth+0, tra+0, num+1, dut+0, str+1, sng+1, sng+1, ini+1, sng+1, ini+4 },
/* 90*/{ oth+0, tra+0, num+3, dut+0, str+1, sng+1, sng+1, ini+1, sng+1, ini+4 },
/*100*/{ oth+0, tra+0, num+3, dut+0, str+1, sng+1, sng+1, ini+1, sng+1, ini+4 },
/*110*/{ oth+1, min+1, num+1, dot+1, str+1, sng+1, sng+1, ini+1, sng+1, ini+4 },
};

static unsigned char cctab[128] = {
    ['0']=2, ['1']=2, ['2']=2, ['3']=2, ['4']=2,
    ['5']=2, ['6']=2, ['7']=2, ['8']=2, ['9']=2,
    ['.']=3,
    ['\'']=4,
    ['(']=5, [')']=5,
    ['[']=6, [']']=6, [';']=6, [':']=6,
    [' ']=7, ['\t']=7,
    [0x0D]=9,
};

static inline unsigned char qminus(int ch){
    return ch == (quadneg? '-': 0x00af);
}

static inline unsigned char character_class(int ch){
    return
           qminus(ch)? 1:
           ch<sizeof cctab? cctab[ch]:
           ch==0x2190? 8:
           ch==0x22c4? 9:
           0;
}

void check_quadneg(symtab st);
token newobj(int *s, int n, int state);
token *collapse_adjacent_numbers_if_needed(token *p);
