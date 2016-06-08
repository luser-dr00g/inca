testprogs= $(notdir $(wildcard ./*_test.c))
unitprogs= $(subst _test,,$(testprogs))
tests= $(basename $(testprogs))
units= $(basename $(unitprogs))
unitobjs= $(patsubst %,%.o,$(units))
testexes= $(patsubst %,%.exe,$(tests))
LDLIBS = -lmpfr -lgmp

extraobjs= encoding.o print.o editor.o

all:olmec test

clean:
	rm -f *.o all_tests.c all_tests.exe $(testexes)

test:all_tests $(tests)
	@./all_tests
all_tests.c:all_tests.m4 makefile $(unitprogs)
	m4 -D UNITS="$(units)" $< >$@
all_tests:all_tests.c
	$(CC) $(CFLAGS) -o $@ $^ $(extraobjs) $(LDLIBS)
array_test:array_test.c
	$(CC) $(CFLAGS) -o $@ $^ number.o symtab.o $(extraobjs) $(LDLIBS)
symtab_test:symtab_test.c
	$(CC) $(CFLAGS) -o $@ $^ number.o array.o $(extraobjs) $(LDLIBS)
number_test:number_test.c
	$(CC) $(CFLAGS) -o $@ $^ array.o symtab.o $(extraobjs) $(LDLIBS)

olmec:main.o lex.o exec.o verbs.o adverbs.o xverb.o $(extraobjs) $(unitobjs)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

