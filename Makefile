CC=cc
INCS=`pkg-config --cflags glfw3 cglm` -Ivendor/glad/include -Ivendor/stb_image/ -Ivendor/stb_truetype
CFLAGS+=${INCS} -DLF_GLFW -O3 -ffast-math 
all: bin/leif.a

bin/leif.a: bin/leif.o
	ar rcs bin/libleif.a bin/*.o
bin/leif.o: bin
	${CC} -c ${CFLAGS} leif.c -o bin/leif.o
bin:
	mkdir bin
clean:
	rm -r ./bin

.PHONY: all test clean
