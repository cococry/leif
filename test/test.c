#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <leif.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

static void resize_callback(GLFWwindow* window, int32_t width, int32_t height) {
    lf_resize_display(width, height);
    glViewport(0, 0, width, height);
}
int main(int argc, char* argv[]) {
       if(!glfwInit()) {
        printf("Failed to initialize GLFW.\n");
    }

    int32_t win_w = 755;
    int32_t win_h = 550;
    GLFWwindow* window = glfwCreateWindow(win_w, win_h, "Hello Leif",NULL, NULL);
    if(!window) {
        printf("Failed to create GLFW window.\n");
    }
    glfwMakeContextCurrent(window);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize Glad.\n");
    }

    LfTheme theme = lf_default_theme("../test/fonts/poppins.ttf", 28);
    lf_init_glfw(win_w, win_h, "../test/fonts/poppins.ttf", &theme, window);   
    lf_set_text_wrap(true);
    glfwSetFramebufferSizeCallback(window, resize_callback);

    LfTexture tick = lf_tex_create("../test/textures/tick.png", false, LF_TEX_FILTER_LINEAR);   

    while(!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

        lf_div_begin((vec2s){0, 0}, (vec2s){win_w, win_h}); 
        for(uint32_t i = 0; i < 100; i++) {
            lf_button("Hello");
        }
        lf_update();
        lf_div_end();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
} 
