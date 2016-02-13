typedef struct datum {
    unsigned int val:24;
    unsigned int tag:8;
} datum;

typedef union integer {
    datum data;
    int32_t int32;
} integer;

enum tag {
    LITERAL, /* val is a 24-bit 2's comp integer */
    CHAR, /* val is a 21-bit Unicode code point padded with zeros */
    NUMBER, /* val is an index in the number table */
    PROG, /* val is an (index to an) executable code fragment (ARRAY of CHAR)*/
    ARRAY, /* val is a(n index to a) boxed array */
    SYMTAB, /* val is a(n index to a) symbol table */
    NULLOBJ, /* val is irrelevant (s.b. 0) */
    VERB, /* val is a(n index to a) verb object */
    ADVERB, /* val is a(n index to a) verb object */
    MARKOBJ, /* val is irrelevant (s.b. 0) */
};

extern int null;
extern int mark;

void init_en();

int gettag(int d);
int getval(int d);
int newdata(int tag, int val);

int cache(int tag, void *ptr);
void *getptr(int d);

