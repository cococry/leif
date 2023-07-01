#pragma once 
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    float values[2];
} LfVec2f;

typedef struct {
    int32_t values[2];
} LfVec2i;

typedef struct {
    float values[3];
}LfVec3f;

typedef struct {
    float values[4];
}LfVec4f;

typedef struct {
    float values[16];
} LfMat4;

typedef struct {
    uint32_t id;
    const char* filepath;
} LfTexture;
typedef struct {
    void* cdata;
    void* font_info;
    uint32_t tex_width, tex_height;
    uint32_t line_gap_add, font_size;
    LfTexture bitmap;
} LfFont;


typedef enum {
    LF_TEX_FILTER_LINEAR = 0,
    LF_TEX_FILTER_NEAREST
} LfTextureFiltering;

typedef enum {
    LF_BUTTON_STATE_IDLE = 0,
    LF_BUTTON_STATE_HOVERED,
    LF_BUTTON_STATE_CLICKED,
    LF_BUTTON_STATE_HELD
} LfButtonState;

void lf_init_glfw(uint32_t display_width, uint32_t display_height, void* glfw_window);

void lf_resize_display(uint32_t display_width, uint32_t display_height);

void lf_rect(LfVec2i pos, LfVec2i size, LfVec4f color);

void lf_image(LfVec2i pos, LfVec2i size, LfVec4f color, LfTexture tex);

void lf_text(LfVec2i pos, const char* str, LfFont font, LfVec4f color);

LfFont lf_load_font(const char* filepath, uint32_t pixelsize, uint32_t tex_width, uint32_t tex_height, uint32_t num_glyphs, uint32_t line_gap_add);

LfTexture lf_tex_create(const char* filepath, bool flip, LfTextureFiltering filter); 

void lf_free_font(LfFont* font);  

void lf_flush();


void lf_add_key_callback(void* cb);

void lf_add_mouse_button_callback(void* cb);

void lf_add_scroll_button_callback(void* cb);

void lf_add_cursor_pos_button_callback(void* cb);

bool lf_key_went_down(uint32_t key);

bool lf_key_is_down(uint32_t key);

bool lf_key_is_released(uint32_t key);

bool lf_key_changed(uint32_t key);

bool lf_mouse_button_went_down(uint32_t button);

bool lf_mouse_button_is_down(uint32_t button);

bool lf_mouse_button_is_released(uint32_t button);

bool lf_mouse_button_changed(uint32_t button);

double lf_get_mouse_x();

double lf_get_mouse_y();

double lf_get_mouse_x_delta();

double lf_get_mouse_y_delta();

double lf_get_mouse_scroll_x();

double lf_get_mouse_scroll_y();


typedef struct {
    LfVec4f idle_color, hover_color, clicked_color, held_color;
} LfButtonProps;
LfButtonState lf_button(LfVec2i pos, LfVec2i size, LfButtonProps props);
