
testprogs= $(notdir $(wildcard ./*_test.c))
tests=$(basename $(testprogs))
unitprogs= $(subst _test,,$(testprogs))
units= $(basename $(unitprogs))

test:all_tests $(tests)
	./all_tests
all_tests.c:all_tests.m4 makefile $(unitprogs)
	m4 -D UNITS="$(units)" $< >$@

