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
    LfTexture landscape = lf_tex_create("../test/textures/norway.jpg", false, LF_TEX_FILTER_LINEAR);    
    float lastTime = 0.0f;
    float deltaTime = 0.0f;

    bool show_image = false;
    bool show_text = false;
    while(!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        
        lf_div_begin((LfVec2f){0, 0}, (LfVec2i){win_w, win_h});
        if(lf_button("Close App") == LF_CLICKED) {
            glfwSetWindowShouldClose(window, true);
        }
        lf_checkbox("Show Image", &show_image, tick.id);
        if(show_image) {
            lf_next_line();
            lf_image(landscape);
            lf_next_line();
            lf_checkbox("Show Text", &show_text, tick.id);
            if(show_text)
                lf_text("This is an image of the beautiful norway.\nIf you want to learn more about norway, check out https://www.lifeinnorway.net/norway-facts/");
        }
        lf_div_end();

        lf_update();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
