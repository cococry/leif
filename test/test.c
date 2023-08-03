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
    glfwSetFramebufferSizeCallback(window, resize_callback);

    float lastTime = 0.0f;
    float deltaTime = 0.0f;
    char name_buf[512] = {0};
    char age_buf[32] = {0};
    char height_buf[32] = {0};
    int32_t age = 0;
    float height = 0;
    LfInputField input_age = (LfInputField){.buf = age_buf, .val = &age, .width = 600};
    LfInputField input_name = (LfInputField){.buf = name_buf, .width = 600};

    LfInputField input_height = (LfInputField){.buf = height_buf, .width = 600, .val = &height};
    bool submitted_name = false, submitted_age = false, submitted_height = false, generated = false;

    lf_set_text_wrap(true);
    while(!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        
        lf_div_begin((LfVec2f){0, 0}, (LfVec2i){win_w, win_h});
        lf_text("Greetings Generator");
        lf_next_line();
        lf_text("Your age:");
        lf_input_int(&input_age);
        if(lf_button_fixed("Submit", -1, theme.font.font_size) == LF_CLICKED) {
            submitted_age = true;
        }
        lf_next_line();
        lf_text("Your name:");
        lf_input_text(&input_name);
        if(lf_button_fixed("Submit", -1, theme.font.font_size) == LF_CLICKED) {
            submitted_name = true;
        }
        lf_next_line();
        lf_text("Your height:");
        lf_input_float(&input_height);
        if(lf_button_fixed("Submit", -1, theme.font.font_size) == LF_CLICKED) {
            submitted_height = true;
        }
        lf_next_line();

        if(lf_button_fixed("Generate\n Greeting", 150, 150) == LF_CLICKED) {
            generated = true;
        }
        if(lf_button_fixed("Close App", 150, 150) == LF_CLICKED) {
            glfwSetWindowShouldClose(window, true);
        }
        if(submitted_age) {
            lf_next_line();
            lf_text("Submitted age: %i", age);
        }
        if(submitted_name) {
            lf_next_line();
            lf_text("Submitted name: %s", name_buf);
        }
        if(submitted_height) {
            lf_next_line();
            lf_text("Submitted height: %.2f", height);
        }
        if(submitted_age && submitted_name && generated) {
            lf_next_line();
            lf_text("Your greeting: \"Hey There, my name is %s, i'm %i years old and %.2fm tall. Nice to meet you! How are you? What did you do today?\"", name_buf, height, age);
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
