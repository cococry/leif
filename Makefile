leif:
	gcc -c leif.c -o bin/leif.o -lX11 -I/usr/include/X11
	ar rcs bin/leif.a bin/leif.o

.PHONY: all test clean
