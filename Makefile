
leif:
	gcc -c leif.c -o bin/leif.o -Ivendor/glad/include -Ivendor/stb_image/ -lm -ffast-math -O3 -Wextra
	ar rcs bin/libleif.a bin/leif.o bin/glad.o bin/stb_image.o  

.PHONY: all test clean
