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
static bool check_win(char* board[3][3], char* player) {
    for (int i = 0; i < 3; i++) {
        if (strcmp(board[i][0], player) == 0 && strcmp(board[i][1], player) == 0 && strcmp(board[i][2], player) == 0)  {
            return true;
        }
    }
    for (int i = 0; i < 3; i++) {
        if (strcmp(board[0][i], player) == 0 && strcmp(board[1][i], player) == 0 && strcmp(board[2][i], player) == 0) {
            return true;
        }
    }
    if (strcmp(board[0][0], player) == 0 && strcmp(board[1][1], player) == 0 && strcmp(board[2][2], player) == 0){
        return true;
    }
    if (strcmp(board[0][2], player) == 0 && strcmp(board[1][1], player) == 0 && strcmp(board[2][0], player) == 0) {
        return true;
    }
    
    return false;
}
static bool board_full(char* board[3][3]) {
    for(uint32_t i = 0; i < 3; i++) {
        for(uint32_t j = 0; j < 3; j++) {
            if(strcmp(board[i][j], " ") == 0) return false;
        }
    }
    return true;
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
    LfTheme theme = lf_default_theme("../test/fonts/poppins.ttf", 36);
    lf_init_glfw(win_w, win_h, "../test/fonts/poppins.ttf", &theme, window);   
    glfwSetFramebufferSizeCallback(window, resize_callback);
    float lastTime = 0.0f;
    float deltaTime = 0.0f;
        
    LfTexture tex = lf_tex_create("../test/textures/sweden.jpg", false, LF_TEX_FILTER_LINEAR);
    const char* heading = "TikTakToe";
    char* table[3][3] = {
        {" ", " ",  " "}, 
        {" ", " ", " "}, 
        {" ", " ", " "}
    };
    bool x_turn = true;
    bool won = false;
    bool draw = false;

    while(!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        
        lf_div_begin((LfVec2f){0, 0}, (LfVec2i){win_w, win_h});
       
        if(lf_button("Exit") == LF_CLICKED) {
            glfwSetWindowShouldClose(window,true);
        }
        lf_set_ptr_x(((lf_get_div_size().values[0] - lf_text_dimension(heading).values[0]) * 0.5f) - theme.text_props.margin_left);
        lf_text(heading);
        lf_next_line();
        for(uint32_t i = 0; i < 3; i++) {
            lf_set_ptr_x(((lf_get_div_size().values[0] - ((150) * 3)) * 0.5f) - theme.button_props.margin_left * 6);
            for(uint32_t j = 0; j < 3; j++) {
                if(lf_button_fixed(table[i][j], 150, 150) == LF_CLICKED) {
                    if(!won) {
                        if(strcmp(table[i][j], " ") == 0) { 
                            table[i][j] = x_turn ? "X" : "O";
                            if(check_win(table, x_turn ? "X" : "O")) {
                                won = true;
                            }
                            x_turn = !x_turn;
                        }
                    }
                }
            }
            lf_next_line();
        }
        if(won || board_full(table)) {
            char text[96] = {0};
            if(won) {
                if(!x_turn) {
                    strcpy(text, "X Won!");
                } else {
                    strcpy(text, "O Won!");
                }
            } else {
                strcpy(text, "Draw!");
            }
            lf_next_line();
            lf_set_ptr_x(((lf_get_div_size().values[0] - lf_text_dimension(text).values[0]) * 0.5f));
            lf_text(text);

            lf_next_line();
            lf_set_ptr_x(((lf_get_div_size().values[0] - 150) * 0.5f));

            if(lf_button_fixed("Reset", 150, -1) == LF_CLICKED) {
                won = false;
                for(uint32_t i = 0; i < 3; i++) {
                    for(uint32_t j = 0; j < 3; j++) {
                        table[i][j] = " ";
                    }
                }
                x_turn = true;
            }
        } else {
            char turn_text[96] = {0};
            if(x_turn) 
                strcpy(turn_text, "X Turn");
            else 
                strcpy(turn_text, "O Turn");
            lf_next_line();
            lf_set_ptr_x(((lf_get_div_size().values[0] - lf_text_dimension(turn_text).values[0]) * 0.5f));
            lf_text(turn_text);
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
