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
	$(CC) $(CFLAGS) -o $@ $^ en.o $(LDLIBS)
st_test:st_test.c
	$(CC) $(CFLAGS) -o $@ $^ en.o $(LDLIBS)

olmec:main.o ed.o en.o wd.o ex.o vb.o adverbs.o $(unitobjs)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

