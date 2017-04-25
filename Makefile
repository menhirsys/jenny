.SUFFIXES:

.PHONY:
run_tests: test
	./test

CFLAGS=-std=c99 -pedantic -Wall -Werror

jenny: main.c
	$(CC) $(CFLAGS) -o $@ $<

%.jenny.c: %.jenny jenny
	./jenny < $< > $@

test: $(shell git ls-files) test.jenny.c
	$(CC) $(CFLAGS) -o $@ test.c jenny.c
