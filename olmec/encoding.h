#define MODE1(x) (x|1<<7)

typedef struct datum {  // these two should be reversed for Big-Endian
    unsigned int val:24;
    unsigned int tag:8; // hi-bit of tag should overlay the sign bit
} datum;

typedef union integer {
    datum data;
    int32_t int32;
} integer;

enum tag {
    LITERAL, /* val is a 24-bit 2's comp integer */
    NUMBER, /* val is an index in the number table */
    CHAR, /* val is a 21-bit Unicode code point padded with zeros */
    PCHAR, /* val is a an executable char */
    PROG, /* val is an (index to an) executable code fragment (ARRAY of PCHAR)*/
    ARRAY, /* val is a(n index to a) boxed array */
    SYMTAB, /* val is a(n index to a) symbol table */
    NULLOBJ, /* val is irrelevant (s.b. 0) */
    VERB, /* val is a(n index to a) verb object */
    ADVERB, /* val is a(n index to a) verb object */
    XVERB, /* val is a verb or adverb */
    MARKOBJ, /* val is irrelevant (s.b. 0) */
    LPAROBJ,
    RPAROBJ,
};

extern int null;
extern int mark;

void init_en();

int gettag(int d);
int getval(int d);
int newdata(int tag, int val);

int cache(int tag, void *ptr);
void *getptr(int d);
int getfill(int d);

