leif:
	gcc -c leif.c -o bin/leif.o
	ar rcs bin/leif.a bin/leif.o

.PHONY: all test clean
