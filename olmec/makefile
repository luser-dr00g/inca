testprogs= $(notdir $(wildcard ./*_test.c))
unitprogs= $(subst _test,,$(testprogs))
tests= $(basename $(testprogs))
units= $(basename $(unitprogs))
unitobjs= $(patsubst %,%.o,$(units))

all:olmec test

test:all_tests $(tests)
	@./all_tests
all_tests.c:all_tests.m4 makefile $(unitprogs)
	m4 -D UNITS="$(units)" $< >$@
all_tests:all_tests.c
	$(CC) $(CFLAGS) -o $@ $^ encoding.o $(LDLIBS)
symtab_test:symtab_test.c
	$(CC) $(CFLAGS) -o $@ $^ encoding.o $(LDLIBS)

olmec:main.o editor.o encoding.o lex.o exec.o verbs.o adverbs.o xverb.o $(unitobjs)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

