#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <leif.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

static void resize_callback(GLFWwindow* window, int32_t width, int32_t height) {
    lf_resize_display(width, height);
    glViewport(0, 0, width, height);
}
static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    printf("%f, %f\n", xpos, ypos);
}

static int32_t get_file_count_in_folder(const char* filepath) {
    DIR* folder = opendir("/home/cococry/everforest-walls/nature");
    struct dirent* entry; 
    if(!folder) {
        printf("Failed to open directory.\n");
        return 0;
    }
    int32_t file_count = 0;
    while((entry = readdir(folder)) != NULL) {
        if(entry->d_type == DT_REG) {
            char* file_name = entry->d_name;
            int name_len = strlen(file_name);
            if(name_len >= 4 && (strcmp(&file_name[name_len -4], ".jpg") || strcmp(&file_name[name_len -4], ".jpg")))
                file_count++;
        }
    }
    return file_count;
}
int main() {
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

    const char* folder_path = "/home/cococry/dev/leif/test/textures/";
    int32_t file_count_folder = get_file_count_in_folder(folder_path);
    DIR* folder_read = opendir(folder_path);
    struct dirent* entry_read; 
    char** files;
    files = malloc(256 * file_count_folder);
    int32_t file_count_img = 0;
    while((entry_read = readdir(folder_read)) != NULL) {
        if(entry_read->d_type == DT_REG) {
            char* file_name = entry_read->d_name;
            int name_len = strlen(file_name);
            if(name_len >= 4 && (strcmp(&file_name[name_len -4], ".png") == 0) || strcmp(&file_name[name_len -4], ".jpg") == 0) {
                files[file_count_img++] = file_name;
            }
        }
    }
    LfTexture textures[file_count_img];
    for(uint32_t i = 0; i < file_count_img; i++) {
        char str[64];
        strcpy(str, folder_path);
        strcat(str, files[i]);
        textures[i] = lf_tex_create(str, false, LF_TEX_FILTER_NEAREST);
    }
    int32_t texture_i = 0;
    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

        lf_div_begin((LfVec2i){0, 0}, (LfVec2i){win_w, win_h});
        if(lf_button("<") == LF_CLICKABLE_ITEM_STATE_CLICKED) {
            if(texture_i - 1 < 0) {
                texture_i = file_count_img - 1;
            } else {
                texture_i--;
            }
        }
        lf_image(textures[texture_i].id, 640, 360);
        if(lf_button(">") == LF_CLICKABLE_ITEM_STATE_CLICKED) {
            if(texture_i + 1 > file_count_img - 1) {
                texture_i = 0;
            } else {
                texture_i++;
            }
        }
        lf_next_line();
        lf_theme()->text_props.margin_top = 50;
        lf_theme()->text_props.margin_left = 50;
        lf_theme()->text_props.color = (LfVec4f){LF_BLACK};
        lf_theme()->text_props.padding = 10;
        lf_text("%s", files[texture_i]);
        lf_theme()->text_props.color = (LfVec4f){0.0f, 0.0f, 0.0f, 0.0f};
        lf_next_line();
        lf_text("%i", texture_i);
        lf_theme()->text_props.margin_top = 10;
        lf_theme()->text_props.margin_left = 10;
        lf_theme()->text_props.padding = 0;
        lf_div_end();

        lf_flush();
        lf_update_input();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    free(files);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
