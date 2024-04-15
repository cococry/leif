CC=gcc
INCS=`pkg-config --cflags glfw3 cglm` -Ivendor/glad/include -Ivendor/stb_image/ -Ivendor/stb_truetype -Ivendor/stb_image_resize
CFLAGS+=${INCS} -DLF_GLFW -O3 -ffast-math 
all: lib/leif.a

lib/leif.a: lib/leif.o
	ar cr lib/libleif.a lib/*.o
lib/leif.o: lib
	${CC} ${CFLAGS} -c leif.c -o lib/leif.o
	${CC} -c vendor/glad/src/glad.c -o lib/glad.o
lib:
	mkdir lib
clean:
	rm -r ./lib

install:
	sudo cp lib/libleif.a /usr/local/lib/ 
	sudo cp -r include/leif /usr/local/include/ 
	cp -r .leif ~/

.PHONY: all test clean
