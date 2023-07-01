
leif:
	gcc -c -DLF_GLFW leif.c -o bin/leif.o -Ivendor/glad/include -Ivendor/stb_image/ -Ivendor/stb_truetype -lm -ffast-math -O3 -Wextra `pkg-config --cflags glfw3` -lglfw 
	ar rcs bin/libleif.a bin/*.o

.PHONY: all test clean
