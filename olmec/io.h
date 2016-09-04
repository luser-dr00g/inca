/* Unicode format conversions */

/*
   <--------  adapters ("apps-"hungarian naming)
   utf8 utf8(ucs4...)
   ucs4 ucs4(utf8...)
 */

enum errinfo {
    no_error = 0,
    invalid_encoding = 1,
    invalid_extended_encoding = 2,
    buffer_alloc_fail = 4,
    bad_following_character = 8,
    over_length_encoding = 16,
    code_point_out_of_range = 32,
};

typedef struct {
    int n;
    unsigned char b[4];
} utfcp;
int32_t to_ucs4(utfcp c);
utfcp to_utf8(int32_t u);
int32_t *ucs4(char *str, int n, int *an, enum errinfo *errinfo);
char *utf8(int32_t *ar, int n, int *an, enum errinfo *errinfo);

#define REPLACEMENT 0xFFFD

