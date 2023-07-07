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

    lf_init_glfw(1000, 1080, NULL, window);    
    glfwSetFramebufferSizeCallback(window, resize_callback);

    float lastTime = 0.0f;
    float deltaTime = 0.0f;
    float div_size = 600;

    bool next = false;
    while(!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        if(!next) {
            lf_div_begin((LfVec2i){100, 100}, (LfVec2i){div_size, div_size});
            if(lf_button("add size") == LF_CLICKABLE_ITEM_STATE_CLICKED) {
                if(div_size + 100 < 1000)
                    div_size += 100; 
            }
            if(lf_button("decrease size") == LF_CLICKABLE_ITEM_STATE_CLICKED) {
                if(div_size - 100 > 100)
                    div_size -= 100; 
            }
            if(lf_button("next") == LF_CLICKABLE_ITEM_STATE_CLICKED) {
                next = !next;
            }
            lf_new_line();
            lf_text("This is a demonstration of the current state of the leif library.\n");
        } else {  
            LfTextProps text_props = lf_get_text_props("Hello World");
            lf_div_begin((LfVec2i){100, 100}, (LfVec2i){600, 600});
            if(lf_button("previous") == LF_CLICKABLE_ITEM_STATE_CLICKED) {
                next = !next;
            }
            lf_set_ptr_x(lf_get_div_size().values[0] / 2.0f - text_props.width / 2.0f);
            lf_set_ptr_y(lf_get_div_size().values[1] / 2.0f - text_props.height);
            lf_text("Hello World");
            lf_div_end();
        }
        lf_flush();
        lf_update_input();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}


