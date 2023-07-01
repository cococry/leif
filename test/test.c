#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <leif.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

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


    lf_init_glfw(1000, 1080, window);
    
    glfwSetFramebufferSizeCallback(window, resize_callback);

    LfFont font_arial = lf_load_font("../test/fonts/impact.ttf", 36.0f, 1024, 1024, 256, 10);
    LfFont font = lf_load_font("../test/fonts/arial.ttf", 36.0f, 1024, 1024, 256, 10);

    float lastTime = 0.0f;
    float deltaTime = 0.0f;
    float scale = 15;
    float offset = 25;
    char filepath[64];
    LfTexture tex;
    LfTexture leif = lf_tex_create("../test/textures/leif.png", false, LF_TEX_FILTER_NEAREST); 
    LfTexture ragnar = lf_tex_create("../test/textures/ragnar.png", false, LF_TEX_FILTER_LINEAR);
    LfTexture tiger = lf_tex_create("../test/textures/tiger.jpg", false, LF_TEX_FILTER_LINEAR);
    LfVec4f bg_color = (LfVec4f){0.3f, 0.3f, 0.3f, 1.0f};
    tex = leif;
    strcpy(filepath, "../test/textures/logo.png");
    while(!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        lf_image((LfVec2i){600.0f, 250.0f}, (LfVec2i){strcmp(tex.filepath, ragnar.filepath) == 0 ? 450 : 300, 300}, (LfVec4f){1.0f, 1.0f, 1.0f, 1.0f}, tex);
        lf_image((LfVec2i){1200.0f, 250.0f}, (LfVec2i){600, 300}, (LfVec4f){1.0f, 1.0f, 1.0f, 1.0f}, tiger);
        if(lf_button((LfVec2i){605, 630}, (LfVec2i){1050, 300}, (LfButtonProps){
                                        .idle_color = (LfVec4f){0.3f, 0.3f, 0.3f, 1.0f}, 
                                      .hover_color = (LfVec4f){0.6f, 0.6f, 0.6f, 1.0f},
                                       .clicked_color = (LfVec4f){0.3f, 0.3f, 0.3f, 1.0f}, 
                                        .held_color = (LfVec4f){0.3f, 0.3f, 0.3f, 1.0f}}) == LF_BUTTON_STATE_HOVERED) {
            bg_color = (LfVec4f){0.6f, 0.6f, 0.6f, 1.0f};
        } else {
            bg_color = (LfVec4f){0.3f, 0.3f, 0.3f, 1.0f};
        }
        lf_text((LfVec2i){130, 570},  "Welcome to the leif library. Leif is a minimalistic GUI library\nwritten in C & OpenGL.\nLeif's focus is on minimalism, usability"
                ",features & modernism.\nLeif currently supports rendering text, textures &\nrectengular geometry with a inhouse batch renderer.\n", font_arial, 
                    (LfVec4f){1.0f, 1.0f, 1.0f, 1.0f});
        for(uint32_t y = 0; y < 16; y++) {
            for(uint32_t x = 0; x < 16; x++) {
                lf_rect((LfVec2i){x * offset + 450, y * offset + 900}, (LfVec2i){scale, scale}, (LfVec4f){(double)rand() / RAND_MAX, (double)rand() / RAND_MAX, (double)rand() / RAND_MAX, 1.0f});
            }
        }
        if(lf_button((LfVec2i){100, 100}, (LfVec2i){50, 50}, 
                     (LfButtonProps){.idle_color = (LfVec4f){0.0f, 1.0f, 0.0f, 1.0f}, 
                                      .hover_color = (LfVec4f){0.0f, 0.8f, 0.0f, 1.0f},
                                       .clicked_color = (LfVec4f){1.0f, 0.0f, 0.0f, 1.0f}, 
                                        .held_color = (LfVec4f){0.0f, 1.0f, 0.0f, 1.0f}}) == LF_BUTTON_STATE_HELD) {
            scale += deltaTime * 8;
            offset += deltaTime * 8;
        }
        if(lf_button((LfVec2i){200, 100}, (LfVec2i){50, 50}, 
                     (LfButtonProps){.idle_color = (LfVec4f){1.0f, 0.0f, 0.0f, 1.0f}, 
                                      .hover_color = (LfVec4f){0.0f, 0.8f, 0.0f, 1.0f},
                                       .clicked_color = (LfVec4f){1.0f, 0.0f, 0.0f, 1.0f}, 
                                        .held_color = (LfVec4f){0.0f, 1.0f, 0.0f, 1.0f}}) == LF_BUTTON_STATE_HELD) {
            scale -= deltaTime * 8;
            offset -= deltaTime * 8;
        }
        if(lf_button((LfVec2i){300, 100}, (LfVec2i){50, 50}, 
                     (LfButtonProps){.idle_color = (LfVec4f){0.0f, 0.0f, 1.0f, 1.0f}, 
                                      .hover_color = (LfVec4f){0.0f, 0.8f, 0.0f, 1.0f},
                                       .clicked_color = (LfVec4f){1.0f, 0.0f, 0.0f, 1.0f}, 
                                        .held_color = (LfVec4f){0.0f, 1.0f, 0.0f, 1.0f}}) == LF_BUTTON_STATE_CLICKED) {
            if(strcmp(tex.filepath, leif.filepath) == 0) {
                tex = ragnar;
            } else {
                tex = leif;
            }
        }
        lf_flush();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}


