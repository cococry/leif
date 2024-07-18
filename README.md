# leif

## Highlights

Leif is UI in C/C++ made easy. The library is designed to be really easy to setup and provide maximal functionality.
All of that while retaining a concise and readable codebase.

#### Theming & Styling support
The library comes with a fully-featured property system where each UI element can be customized
to your liking. Every visible part of your UI can be configured in no time.

#### Exposed Rendering API
Leif comes with a very performant inhouse batch renderer. The renderers drawing API is fully exposed 
to the user. This expands the scope of application of the library a lot as you can also utilize it
for various non-UI components of your project. 

#### Exposed State
The UI state of leif is nearly completly exposed to the user which, with the rendering system, 
makes it really easy to add new UI components client-side.

#### Complete Input-Handling System
The library contains an input subsystem that can be utilized by the user for various different 
tasks (eg. is_key_down, is_mouse_button_released, etc.) which makes development a whole lot easier.


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


## Installation

Installing the library is made very easy by the use of an install script:
```console
git clone https://github.com/cococry/leif
cd leif
./install.sh
```

## Usage

Initialize Leif before using and after creating a window with your windowing backend:
```c
lf_init_glfw(displayWidth, displayHeight, glfwWindow);
```

Within the main loop of your program, call the lf_begin() and lf_end() functions within which you can render UI elements.

#### Simple hello-world application with GLFW for windowing
```c
#include <GLFW/glfw3.h>
#include <leif/leif.h>

int main() {
  glfwInit();
  GLFWwindow* window = glfwCreateWindow(800, 600, "Hello", NULL, NULL);

  glfwMakeContextCurrent(window);

  lf_init_glfw(800, 600, window);

  while(!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    lf_begin();

    lf_text("Hello, Leif!");

    lf_end();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  lf_terminate();
  glfwDestroyWindow(window);
  glfwTerminate();
}
```
<img src="https://github.com/cococry/Leif/blob/main/branding/ui-elements.png" width=375px/> 

#### Hello-World Plus
```c
#include <GLFW/glfw3.h>
#include <leif/leif.h>

static int winw = 800, winh = 800;

static void resizecb(GLFWwindow* window, int w, int h) {
  winw = w;
  winh = h;
  glViewport(0, 0, w, h);
  // Resize the leif display on window resize
  lf_resize_display(w, h);
}

int main() {
  glfwInit();
  GLFWwindow* window = glfwCreateWindow(winw, winh, "Hello", NULL, NULL);

  glfwMakeContextCurrent(window);

  glfwSetFramebufferSizeCallback(window, resizecb);

  lf_init_glfw(winw, winh, window);

  // Loading a bigger font (replace /home/cococry with your user)
  LfFont bigfont = lf_load_font("/home/cococry/.leif/assets/fonts/inter.ttf", 50);

  while(!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Starting leif context
    lf_begin();

    const char* text = "Hello, Leif!";
    const char* btntext = "Exit";

    // Defining properties of the button
    LfUIElementProps btnprops = lf_get_theme().button_props;
    btnprops.margin_left = 0.0f; btnprops.margin_top = 15.0f; btnprops.border_width = 0.0f; btnprops.corner_radius = 9.0f;
    btnprops.text_color = LF_WHITE;
    btnprops.color = (LfColor){90, 90, 90, 255};

    // Beginnig a new container
    {
      // Styling the container (setting corner radius)
      LfUIElementProps props = lf_get_theme().div_props;
      props.corner_radius = 10.0f;
      lf_push_style_props(props);

      // Positioning the container in the center
      float width = 400.0f, height = 400.0f;
      lf_div_begin(((vec2s){(winw - width) / 2.0f, (winh - height) / 2.0f}), ((vec2s){width, height}), false);

      // Popping the container's props again
      lf_pop_style_props();
    }

    /* Text */
    {
      // Setting big font
      lf_push_font(&bigfont);
      // Center the text horizontally
      lf_set_ptr_x_absolute((winw - lf_text_dimension(text).x) / 2.0f);
      // Center the text (and button) vertically
      lf_set_ptr_y_absolute((winh - (lf_text_dimension(text).y + lf_text_dimension(btntext).y + btnprops.padding * 2.0f + btnprops.margin_top)) / 2.0f);

      // Remove any margin
      LfUIElementProps props = lf_get_theme().text_props;
      props.margin_left = 0.0f; props.margin_right = 0.0f;
      // Push the style props
      lf_push_style_props(props);

      // Render the text
      lf_text(text);

      // Pop the style props
      lf_pop_style_props();

      // Unsetting big font
      lf_pop_font();
    }

    // Advance into the next line
    lf_next_line();

    /* Exit Button */
    {
      const float width = 180.0f;

      lf_push_style_props(btnprops);
      // Center the button horizontally
      lf_set_ptr_x_absolute((winw - (width + btnprops.padding * 2.0f)) / 2.0f);

      // Rendering a button with fixed scale (-1 stands for "use normal height")
      if(lf_button_fixed(btntext, width, -1) == LF_CLICKED) {
        // Closing the window when you pressed the button
        glfwSetWindowShouldClose(window, 1);
      }

      lf_pop_style_props();
    }

    // Ending the container
    lf_div_end();

    // Ending leif context
    lf_end();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  lf_terminate();
  glfwDestroyWindow(window);
  glfwTerminate();
}
```

<img src="https://github.com/cococry/Leif/blob/main/branding/styling-elements.png" width=375px/> 

## Building your project

To build your project, link it with leif and its depdencies.
```console
gcc -o project project.c -lleif -lglfw -lm -lGL -lclipboard
```

## Real world usage

But how does the leif library perform in real applications? I am currently working on a music player called [lyssa](https://github.com/cococry/lyssa). The frontend of the player
is written entirely with leif. You can see a brief look at of the application below.

<img src="https://github.com/cococry/lyssa/blob/main/branding/lyssa-showcase.png" width="1000px"/> 

A [task management app](https://github.com/cococry/todo) that i also wrote with the leif library:
<img src="https://github.com/cococry/todo/blob/main/branding/todo-showcase.png" width="1000px"/> 

## Contributing

You can contribute to Leif by:
  - [Fixing bugs or contributing to features](https://github.com/cococry/lyssa/issues)
  - [Changing features or adding new functionality](https://github.com/cococry/lyssa/pulls)


