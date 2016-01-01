testprogs= $(notdir $(wildcard ./*_test.c))
unitprogs= $(subst _test,,$(testprogs))
tests= $(basename $(testprogs))
units= $(basename $(unitprogs))
unitobjs= $(patsubst %,%.o,$(units))

all:test olmec

test:all_tests $(tests)
	./all_tests
all_tests.c:all_tests.m4 makefile $(unitprogs)
	m4 -D UNITS="$(units)" $< >$@

olmec:main.o $(unitobjs) ed.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

