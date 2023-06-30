#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <leif.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

static void resize_callback(GLFWwindow* window, int32_t width, int32_t height) {
    lf_resize_display(width, height);
    glViewport(0, 0, width, height);
}
int main() {
    srand(time(NULL));
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

    lf_init(1000, 1080);

    LfFont font = lf_load_font("../test/fonts/impact.ttf", 36.0f, 1024, 1024, 256, 5);
    LfFont font_arial = lf_load_font("../test/fonts/arial.ttf", 96.0f, 1024, 1024, 256, 5);

    float lastTime = 0.0f;
    float deltaTime = 0.0f;
    while(!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        for(uint32_t y = 0; y < 16; y++) {
            for(uint32_t x = 0; x < 16; x++) {
                lf_draw_rect((LfVec2i){x * 50 + 250, y * 50 + 400}, (LfVec2i){25, 25}, (LfVec4f){(double)rand() / RAND_MAX, (double)rand() / RAND_MAX, (double)rand() / RAND_MAX, 1.0f});
            }
        }
        lf_draw_rect((LfVec2i){650.0f, 900.0f}, (LfVec2i){800.0f, 400.0f}, (LfVec4f){0.2f, 0.3f, 0.8f, 1.0f});
        lf_draw_str((LfVec2i){250.0f, 900.0f}, (LfVec4f){1.0f, 0.0f, 0.0f, 1.0f}, "This is Leif!\n A featurefull (will be) but still minimalistic GUI library written in C.\n", font);
        lf_draw_str((LfVec2i){450.0f, 600.0f}, (LfVec4f){1.0f, 1.0f, 1.0f, 1.0f}, "This is arial!", font_arial);
        lf_draw_image((LfVec2i){650.0f, 200.0f}, (LfVec2i){256, 196}, (LfVec4f){1.0f, 1.0f, 1.0f, 1.0f}, "../test/textures/logo.png", false, TEX_FILTER_LINEAR);
        
        lf_flush();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
