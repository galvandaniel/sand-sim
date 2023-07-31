.PHONY: clean

test: sandbox.h sandbox.c test.c
	clang -Wall -gdwarf-4 -o test sandbox.c test.c

clean:
	rm -rf a.out test
