#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <leif.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void resize_callback(GLFWwindow* window, int32_t width, int32_t height) {
    lf_resize_display(width, height);
    glViewport(0, 0, width, height);
}
static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    printf("%f, %f\n", xpos, ypos);
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

    lf_init_glfw(win_w, win_h, NULL, window);    
    glfwSetFramebufferSizeCallback(window, resize_callback);
    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

        lf_div_begin((LfVec2i){0, 0}, (LfVec2i){win_w, win_h});
        lf_button_fixed("Hello", 150, -1);
        lf_button("Hello");
        lf_button_fixed("Hello", 150, -1);
        lf_div_end();

        lf_flush();
        lf_update_input();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
