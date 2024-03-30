# Leif

## Overview

Leif is a minimal and powerful immediate UI library/framework written in pure C. The library uses OpenGL for rendering interfaces. 
The focus of this library is to be performant, easy to use and [suckless](https://suckless.org/philosophy/). Leif uses batch rendering for 
displaying GPU accelerated user interfaces. 

### stb-like library

The Leif UI library is contained in only one C Source- and one C Header-file. This is done for providing the easiest possible way to work 
with Leif. But because the library has a few dependencies, it is prepackaged into a static library that the user can then link to.

### Features

Leif comes with a ton of customizablity options and theming. The Element Property System in Leif allows the user to create any UI that 
comes to mind. The library provides lots of UI elements like input fields, buttons text labels, images etc. 

The library provides a complete Div Container System out of the box with which the user can seperate different parts of the UI into contained 
fields. The user can scroll inside every div which creates lots of possiblities to create UI systems.

In the backend, Leif uses a highly efficient batch renderer that can render textures, text and colored geometry in one batch. The rendering backend 
that the library ueses is OpenGL. 


### Dependencies

| Dependency         |  Reason of Usage    |
| ----------------|-------------|
| [glad](https://github.com/Dav1dde/glad) | Loading OpenGL functions |
| [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) | Loading image files into memory |
| [stb_image_resize2](https://github.com/nothings/stb/blob/master/stb_image_resize2.h) | Resizing images |
| [stb_truetype](https://github.com/nothings/stb/blob/master/stb_truetype.h) | Loading font glyphs from font files |
| [cglm](https://github.com/recp/cglm) | Linear Algebra Math | 
| [*GLFW](https://github.com/glfw/glfw) | Handling windowing, input etc. | 

*: This library is an optional library and will be replacable with other libraries


## Building

Building the static library should be as easy as installing the dependencies and executing the Makefile.

#### Install the dependencies (Arch)
```console
sudo pacman -S gcc make glfw cglm
```

#### Install the dependencies (Debian)
```console
sudo apt install gcc make libglfw3 libglfw3-dev libcglm-dev
```

#### Building the library
```console
make
```

## Usage

Initialize Leif before using and after creating a window with your windowing backend:
```c
lf_init_glfw(displayWidth, displayHeight, glfwWindow);
```

Within the main loop of your program, call the lf_begin() and lf_end() functions within which you can render UI elements.

#### A simple example of some UI elements:
```c
lf_begin();
lf_text("Hello Leif!");

lf_next_line();

if(lf_button("Close") == LF_CLICKED) {
  glfwSetWindowShouldClose(window, true);
}
if(lf_button("Show Text") == LF_CLICKED) {
  showText = !showText;
}
if(showText) {
  lf_next_line();
  lf_text_wide(L"Привет");
}
lf_end();
```
<img src="https://github.com/cococry/Leif/blob/main/branding/ui-elements.png" width=375px/> 

#### Styling & Positioning Elements
```c
lf_begin();
for(uint32_t i = 0; i < 4; i++) {
  const char* buttonText = "Start App";

  LfUIElementProps props = lf_theme()->button_props;
  props.margin_left = 0;
  props.margin_top = 20;
  props.color = RGB_COLOR(220, 220, 220);
  props.text_color = LF_BLACK;
  props.corner_radius = 3.5f;
  props.border_width = 0.0f;

  lf_push_style_props(props);

  lf_set_ptr_x_absolute((displayWidth - lf_button_dimension(buttonText).x) / 2.0f);
  lf_button(buttonText);

  lf_pop_style_props();

  lf_next_line();
}
lf_end();
```

<img src="https://github.com/cococry/Leif/blob/main/branding/styling-elements.png" width=375px/> 

#### Working with Div Containers
```c
static LfTexture appleTexture = lf_load_texture("apple.png", false, LF_TEX_FILTER_LINEAR);

lf_begin();

LfUIElementProps div_props = lf_theme()->div_props;
div_props.border_width = 3;
div_props.border_color = LF_BLUE;
div_props.corner_radius = 6;
lf_push_style_props(div_props);

lf_div_begin((vec2s){10, 150}, (vec2s){300, 300}, true);

lf_pop_style_props();

for(uint32_t i = 0; i < 10; i++) {
  if(i % 2 == 0) {
    LfUIElementProps props = lf_theme()->button_props;
    props.border_width = 0;
    props.color = LF_WHITE;
    props.text_color = LF_BLACK;
    props.corner_radius = 3;

    lf_push_style_props(props);

    lf_push_element_id(i);
    lf_button("Button");
    lf_pop_element_id();

    lf_pop_style_props();
  } else {
    lf_image((LfTexture){.id = appleTexture.id, .width = 64, .height = 64});
  }
  lf_next_line();
}
lf_div_end();

div_props.border_color = LF_RED;
lf_push_style_props(div_props)  ;

lf_div_begin((vec2s){350, 150}, (vec2s){200, 200}, true);

lf_pop_style_props();

for(uint32_t i = 0; i < 20; i++) {
  lf_text("Div Test");
  lf_next_line();
}

lf_div_end();

lf_end();
```

<img src="https://github.com/cococry/Leif/blob/main/branding/div-showcase.gif" width="375px"/> 

## Contributing

You can contribue to Lyssa by:
  - [Fixing bugs or contributing to features](https://github.com/cococry/lyssa/issues)
  - [Changing features or adding new functionality](https://github.com/cococry/lyssa/pulls)


