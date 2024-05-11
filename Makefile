CC=gcc
INCS=`pkg-config --cflags glfw3` -Ivendor/glad/include -Ivendor/stb_image/ -Ivendor/stb_truetype -Ivendor/stb_image_resize
CFLAGS+=${INCS} -DLF_GLFW -O3 -ffast-math 
all: lib/leif.a

build-deps:
	cd vendor/cglm/ && ./build.sh && cd ../libclipboard && ./build.sh

lib/leif.a: lib/leif.o
	ar cr lib/libleif.a lib/*.o

lib/leif.o: lib build-deps
	${CC} ${CFLAGS} -c leif.c -o lib/leif.o
	${CC} -c vendor/glad/src/glad.c -o lib/glad.o

lib:
	mkdir lib
clean:
	rm -r ./lib

install:
	cp lib/libleif.a /usr/local/lib/ 
	cp -r include/leif /usr/local/include/ 
	cp -r .leif ~/

uninstall:
	rm -f /usr/local/lib/libleif.a
	rm -rf /usr/local/include/leif/
	rm -rf ~/.leif/

.PHONY: all test clean
