# Leif

<img src="https://github.com/cococry/Leif/blob/main/branding/leif-branding.png" width="800"  /> 

## Overview
Leif is a minimalist but yet powerful
and configurable immediate GUI library
written in C. Leif uses Modern OpenGL
for rendering. It is a single file 
library with one C file and one Header.

## Notes
### Early stage library
This library is very early stage so 
don't expect everything to be perfect.
If you track a bug or want to contribute
to the library, feel free to submit an 
issue or a pull request. 

### No Windows Building support
The library only builds on unix systems
for now and windows is not yet supported
by the build. 

### Only GLFW Windowing for now
Leif depends on GLFW at the current state
because it is the system used
to handle inputs and windowing. 
Eventually Leif will support multiple
windowing systems like SDL, Win32 etc.

## Dependencies
- (GLFW)
- stb_image
- stb_truetype
- glad

## Installation
There are two ways to use Leif in your
project. You can either include the C file
into your build and handle the dependencies
yourself or you can link the static library
to your project.

### Using the static library
Using the static library is as easy as
running: 

```console
./install
./build_all
```
This will populate the bin directory
with the libleif.a file which you can link
with. 

*Example with GCC*
```console
gcc -o program program.c -Ileif -Lbin -lleif -lfglfw
```

### Including the C file 
First, add leif.c to your build and
add an include path fof leif.h. 
Then add the dependencies of leif to
your build.(stb_image, stb_truetype
and glad are in the vendor folder).

*Example With GCC*
```console
gcc -o program program.c leif.c -Ileif -lglfw
- vendor/stb_image/stb_image.c vendor/stb_truetype/stb_truetype.c vendor/glad/glad.c
```
