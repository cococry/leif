#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <leif.h>
#include <stdio.h>

static void resize_callback(GLFWwindow* window, int32_t width, int32_t height) {
    lf_resize_display(width, height);
    glViewport(0, 0, width, height);
}
int main() {
    if(!glfwInit()) {
        printf("Failed to initialize GLFW.\n");
    }

    GLFWwindow* window = glfwCreateWindow(1000, 1000, "Hello Leif",NULL, NULL);

    if(!window) {
        printf("Failed to create GLFW window.\n");
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize Glad.\n");
    }

    glfwSetFramebufferSizeCallback(window, resize_callback);

    lf_init(1000, 1000);

    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        lf_draw_rect((LfVec2i){400.0f, 400.0f}, (LfVec2i){200, 200}, (LfVec4f){1.0f, 0.0f, 0.0f, 1.0f});
        lf_draw_rect((LfVec2i){0.0f, 0.0f}, (LfVec2i){100, 100}, (LfVec4f){0.0f, 1.0f, 0.0f, 1.0f});
        lf_flush();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
