/*
 * The transition table and state set
 *
 * Each state embodies a certain amount of "knowledge" 
 * about what sort of token has been encountered. 
 * The dot character '.' causes a great deal of trouble
 * since it is heavily overloaded. If the dot has a digit
 * on either or both sides, then it is considered a decimal
 * point separating the integer and fractional parts of a
 * floating-point number. TODO: add floating-point numbers.
 * Otherwise, the dot is considered part of an identifier.
 *
 * Note, the state enum codes are 10* the corresponding table index.
 */
enum state {
    ini=0,  //indeterminate
    dot=10, //initial dot                            . 
    num=20, //integer                                0 
    dit=30, //medial dot                             0.
    fra=40, //fraction                               0.0
    str=50, //initial quote                          '
    quo=60, //end or escape quote                    'aaa'
    oth=70, //identifier or other symbol             a+
    dut=80, //trailing dot                           +.
    sng=90, //copula or other self-delimiting symbol ()
};

int wdtab[][sizeof "0.'() <r"] = {
/*state*/
/*|*//* character class*/
/*V*//* none   0-9    .      '      (      )      sp     <-     \r    */
/*0*/ { oth+2, num+2, dot+2, str+2, sng+2, sng+2, ini+0, sng+2, ini+0 },
/*10*/{ oth+0, fra+0, oth+0, str+1, ini+1, ini+1, ini+1, sng+1, ini+1 },
/*20*/{ oth+1, num+0, dit+0, str+1, oth+1, oth+1, ini+1, sng+1, ini+1 },
/*30*/{ oth+0, num+1, dut+1, str+1, sng+1, sng+1, ini+1, sng+1, ini+1 },
/*40*/{ oth+1, fra+0, dot+1, str+1, sng+1, sng+1, ini+1, sng+1, ini+1 },
/*50*/{ str+0, str+0, str+0, quo+0, str+0, str+0, str+0, str+0, ini+1 },
/*60*/{ oth+1, num+1, dot+1, str+0, oth+1, oth+1, ini+1, sng+1, ini+1 },
/*70*/{ oth+0, num+1, dut+0, str+1, sng+1, sng+1, ini+1, sng+1, ini+1 },
/*80*/{ oth+0, num+3, dut+0, str+1, sng+1, sng+1, ini+1, sng+1, ini+1 },
/*90*/{ oth+1, num+1, dot+1, str+1, sng+1, sng+1, ini+1, sng+1, ini+1 },
};

static unsigned char cctab[64] = {
    ['0']=1, ['1']=1, ['2']=1, ['3']=1, ['4']=1,
    ['5']=1, ['6']=1, ['7']=1, ['8']=1, ['9']=1,
    ['.']=2,
    ['(']=3,
    [')']=4,
    ['\'']=5,
    [' ']=6, ['\t']=6,
    [0x0D]=8,
};

static inline unsigned char character_class(int ch){
    return ch<64? cctab[ch] :
           ch==0x2190? 7 :
           0;
}


token newobj(int *s, int n, int state);
token *collapse_adjacent_numbers_if_needed(token *p);
