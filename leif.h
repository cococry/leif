#pragma once 
#include <stdbool.h>
#include <stdint.h>

#define LF_RGBA(r, g, b, a) r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f
#define LF_RED LF_RGBA(255.0f, 0.0f, 0.0f, 255.0f)
#define LF_GREEN LF_RGBA(0.0f, 255.0f, 0.0f, 255.0f)
#define LF_BLUE LF_RGBA(0.0f, 0.0f, 255.0f, 255.0f)
#define LF_WHITE LF_RGBA(255.0f, 255.0f, 255.0f, 255.0f)
#define LF_BLACK LF_RGBA(0.0f, 0.0f, 0.0f, 255.0f)

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
    uint32_t width, height;
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

typedef struct {
    float width, height;
    uint32_t char_count;
    bool reached_stop;
    float first_char_width, last_char_width; 
    char rendered_text[256];
    int32_t end_x, start_x;
} LfTextProps;

typedef enum {
    LF_IDLE = 0,
    LF_HOVERED = 1,
    LF_CLICKED = 2
} LfClickableItemState;

typedef struct {
    LfVec4f color, hover_color, clicked_color, text_color;
    float padding, margin_left, margin_right, margin_top, margin_bottom;
} LfUIElementProps;

typedef struct {
    LfUIElementProps button_props, div_props, text_props, image_props;
    LfFont font;
} LfTheme;


void lf_init_glfw(uint32_t display_width, uint32_t display_height, LfTheme* theme, void* glfw_window);

void lf_resize_display(uint32_t display_width, uint32_t display_height);

void lf_rect(LfVec2f pos, LfVec2i size, LfVec4f color);

LfFont lf_load_font(const char* filepath, uint32_t pixelsize, uint32_t tex_width, uint32_t tex_height, uint32_t num_glyphs, uint32_t line_gap_add);

LfTexture lf_tex_create(const char* filepath, bool flip, LfTextureFiltering filter); 

void lf_free_font(LfFont* font); 

void lf_add_key_callback(void* cb);

void lf_add_mouse_button_callback(void* cb);

void lf_add_scroll_callback(void* cb);

void lf_add_cursor_pos_callback(void* cb);

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

LfClickableItemState lf_div_begin(LfVec2f pos, LfVec2i size);

void lf_div_end();

LfClickableItemState lf_button(const char* text);

LfClickableItemState lf_button_fixed(const char* text, int32_t width, int32_t height);
 
LfUIElementProps lf_style_color(LfVec4f color);

void lf_next_line();

LfTextProps lf_get_text_props(const char* str);

void lf_text(const char* fmt, ...);

LfVec2i lf_get_div_size();

void lf_set_ptr_x(float x);

void lf_set_ptr_y(float y);

void lf_image(uint32_t tex_id, uint32_t width, uint32_t height);

LfTheme* lf_theme();

void lf_update();

void lf_input(char* buf, int32_t width);
