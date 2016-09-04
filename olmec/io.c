//cf. https://codereview.stackexchange.com/questions/98838/utf-8-encoding-decoding
//cf. http://www.ietf.org/rfc/rfc3629.txt p.3
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h> // log2
//#include <sys/bitops.h> // ilog2

#include "io.h"

/* number of leading zeros of byte-sized value */
static int leading0s(uint_least32_t x){ return 7 - (x? floor(log2(x)): -1); }

/* number of leading ones of byte-sized value */
#define leading1s(x) leading0s(0xFF^(x))

/* generate unsigned long mask of x ones in the least-significant position */
#define lomask(x) ((1UL<<(x))-1)

/* generate byte mask of x ones in the most-significant position */
#define himask(x) (0xFF^lomask(8-(x)))

int32_t expand_shortcut(unsigned char b){
    return REPLACEMENT;
}

int32_t to_ucs4(utfcp c){
    int prefix = leading1s(c.b[0]);
    int n = prefix? prefix: 1;
    int32_t u;
    //if (n != c.n)
    switch(prefix){
    case 0: u = c.b[0]; break;
    case 1: return u = expand_shortcut(c.b[0]);
    case 2: u = c.b[0] & 0x1f; break;
    case 3: u = c.b[0] & 0x0f; break;
    case 4: u = c.b[0] & 0x07; break;
    }
    for(int i=1; i<n; ++i){
        u = (u << 6) | (c.b[n] & 0x3f);
    }

    if (u < ((int[]){0,0,0x80,0x800,0x10000})[prefix]) {
        //error |= over_length_encoding;
        u=REPLACEMENT;
    }
    return u;
}

utfcp to_utf8(int32_t u){
    if (u<0x80) return (utfcp){1,u};
    if (u<0x800) return (utfcp){2,0xC0|(u>>6),
		     0x80|(u&0x3f)};
    if (u<0x10000) return (utfcp){3,0xE0|(u>>12),
		       0x80|((u>>6)&0x3f),0x80|(u&0x3f)};
    if (u<0x110000) return (utfcp){4,0xF0|(u>>18),
			0x80|((u>>12)&0x3f),0x80|((u>>6)&0x3f),0x80|(u&0x3f)};
    //(else) error RANGE
    return (utfcp){0,0};
}

int32_t *ucs4(char *str, int n, int *an, enum errinfo *errinfo){
    unsigned char *p=str;
    int32_t *u,*buf;
    uint_least32_t x;
    int prefix;
    int i,j;
    enum errinfo error = no_error;

    buf=u=malloc(n*sizeof*u);
    if (!buf) {
        error |= buffer_alloc_fail;
    }
    else {
        for (i=0; i<n && *p; i++){
            prefix=leading1s(x=*p++);

            switch(prefix){
                case 0: break;
                case 1: error |= invalid_encoding;
                        x=REPLACEMENT;
                        break;
                case 2:
                case 3:
                case 4: x&=lomask(8-prefix);
                        for (j=1; j<prefix; j++){
                            if (leading1s(*p)!=1) {
                                error |= bad_following_character;
                                x=REPLACEMENT;
                                break;
                            }
                            x=(x<<6) | (*p++&lomask(6));
                        }
                        break;
                default: error |= invalid_extended_encoding;
                         x=REPLACEMENT;
                         break;
            }
            if (x < ((int[]){0,0,0x80,0x800,0x10000})[prefix]) {
                error |= over_length_encoding;
                x=REPLACEMENT;
            }
            *u++=x;
	    
        }
    }

    if (an) *an = i;
    if (errinfo) *errinfo = error;
    return buf;
}

char *utf8(int32_t *ar, int n, int *an, enum errinfo *errinfo){
    int i;
    uint_least32_t x;
    char *p,*buf;
    enum errinfo error = no_error;

    buf=p=malloc((n+1)*4);
    if (!buf) {
        error |= buffer_alloc_fail;
    }
    else {
        for (i=0; i<n; i++){
            x=ar[i];
            if (x <= lomask(7)) {
                *p++=           ((x)     & lomask(7));
            }
            else if (x <= lomask(11)) {
                *p++=himask(2)| ((x>>6)  & lomask(5));
                *p++=himask(1)| ((x)     & lomask(6));
            }
            else if (x <= lomask(16)) {
                *p++=himask(3)| ((x>>12) & lomask(4));
                *p++=himask(1)| ((x>>6)  & lomask(6));
                *p++=himask(1)| ((x)     & lomask(6));
            }
            else if (x <= 0x10FFFF) {
                *p++=himask(4)| ((x>>18) & lomask(3));
                *p++=himask(1)| ((x>>12) & lomask(6));
                *p++=himask(1)| ((x>>6)  & lomask(6));
                *p++=himask(1)| ((x)     & lomask(6));
            }
            else {
                error |= code_point_out_of_range;
            }
        }
        *p++=0;
    }

    if (an) *an = p-buf;
    if (errinfo) *errinfo = error;
    return buf;
}

#ifdef TESTMODULE
#include "stdlib.h"
#include "string.h"
#include "minunit.h"
int tests_run = 0;

static char *test_leading0s(){
    //int i;for(i=0;i<256;i++)printf("%d <%x>,nlz %d,nlo %d\n",i,i,leading0s(i),leading0s(i^0xFF));
    test_case(leading0s(0)!=8);
    test_case(leading0s(1)!=7);
    test_case(leading0s(2)!=6);
    test_case(leading0s(4)!=5);
    test_case(leading0s(8)!=4);
    test_case(leading0s(16)!=3);
    test_case(leading0s(32)!=2);
    test_case(leading0s(64)!=1);
    test_case(leading0s(128)!=0);
    //test_case(2!="baloney");
    return 0;
}

#define UNI_EQUS(_) \
    /* str,  ints,              size */ \
    _("abc", ((int[]){97,98,99}), 3) \
/*enddef UNI_EQUS */


static char *test_utf8(){
#define UTF_TEST(str,ints,size) \
    test_case(strcmp(str, \
                     utf8(ints,size,NULL,NULL)));
/*  test_case(strcmp("abc",
                     utf8((int[]){97,98,99},3,NULL,NULL))); */
    UNI_EQUS(UTF_TEST)
    return 0;
}


static char *test_ucs4(){
#define UCS_TEST(str,ints,size) \
    test_case(memcmp(ints, \
                     ucs4(str,size,NULL,NULL), \
                     size*sizeof(int)));
/*  test_case(memcmp((int[]){97,98,99},
                     ucs4("abc",3,NULL,NULL),
                     3*sizeof(int))); */
    UNI_EQUS(UCS_TEST)
    return 0;
}



static char *test_transit(){
#define UTF_UCS_TEST(str,ints,size) \
    test_case(strcmp(str, \
                     utf8(ucs4(str,size,NULL,NULL),size,NULL,NULL)));
/*  test_case(strcmp("abc",
                     utf8(ucs4("abc",3,NULL,NULL),3,NULL,NULL))); */
    UNI_EQUS(UTF_UCS_TEST)

#define UCS_UTF_TEST(str,ints,size) \
    test_case(memcmp(ints, \
                     ucs4(utf8(ints,size,NULL,NULL),size,NULL,NULL), \
                     size*sizeof(int)));
/*  test_case(memcmp((int[]){97,98,99},
                     ucs4(utf8((int[]){97,98,99},3,NULL,NULL),3,NULL,NULL),
                     3*sizeof(int))); */
    UNI_EQUS(UCS_UTF_TEST)
    return 0;
}


static char *all_tests(){
    mu_run_test(test_leading0s);
    mu_run_test(test_utf8);
    mu_run_test(test_ucs4);
    mu_run_test(test_transit);
    return 0;
}

int main() {

    char *result=all_tests();
    if (result != 0) {
        printf("%s\n",result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result != 0;

}

#endif //defined TESTMODULE
