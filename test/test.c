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
    char buf_email[512] ={0};
    LfInputField input_email = (LfInputField){.buf = buf_email, .width = 450, .selected = false, .placeholder = "Email..."};
    char buf_password[512] ={0};
    LfInputField input_password = (LfInputField){.buf = buf_password, .width = 450, .selected = false, .placeholder = "Password..."};
    bool show_text = false;

    vec2s dim_password = lf_text_dimension("Password");

    char submitted_password[512] = {0};
    char submitted_email[512] = {0};
    bool submitted = false;
    while(!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

        lf_div_begin((vec2s){0, 0}, (vec2s){win_w, win_h}); 
        lf_text("Email");
        lf_set_ptr_x(dim_password.x + theme.text_props.margin_left + theme.text_props.margin_right);
        lf_input_text(&input_email);
        lf_set_item_color((vec4s){LF_RGBA(189, 34, 88, 255)});
        lf_button_fixed("Clear", -1, theme.font.font_size);
        lf_unset_item_color();

        lf_next_line();

        lf_text("Password");
        lf_input_text(&input_password);
        lf_set_item_color((vec4s){LF_RGBA(189, 34, 88, 255)});
        lf_button_fixed("Clear", -1, theme.font.font_size);
        lf_unset_item_color();

        lf_next_line();
        lf_set_ptr_x(dim_password.x + theme.text_props.margin_left + theme.text_props.margin_right);
        lf_set_item_color((vec4s){LF_RGBA(34, 189, 88, 255)});
        if(lf_button_fixed("Submit", 150,60) == LF_CLICKED) {
            memcpy(submitted_password, buf_password, 512);
            memcpy(submitted_email, buf_email, 512);
            submitted = true;
        }
        lf_checkbox("Show text", &show_text, tick.id);
        if(submitted && show_text){
            lf_text("Hey there, you submitted a new user with the email %s and the password %s. Welcome!", submitted_email, submitted_password);
        }
        lf_unset_item_color();
        lf_div_end();

        lf_update();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
} 
