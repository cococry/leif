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
} LfTextProps;

typedef enum {
    LF_CLICKABLE_ITEM_STATE_IDLE = 0,
    LF_CLICKABLE_ITEM_STATE_HOVERED = 1,
    LF_CLICKABLE_ITEM_STATE_CLICKED = 2
} LfClickableItemState;

typedef struct {
    LfVec4f color, hover_color, clicked_color, text_color;
    float padding, margin_left, margin_right, margin_top, margin_bottom;
} LfUIElementProps;

typedef struct {
    LfUIElementProps button_props, div_props, text_props;
    LfFont font;
} LfTheme;


void lf_init_glfw(uint32_t display_width, uint32_t display_height, LfTheme* theme, void* glfw_window);

void lf_resize_display(uint32_t display_width, uint32_t display_height);

void lf_rect(LfVec2i pos, LfVec2i size, LfVec4f color);

LfFont lf_load_font(const char* filepath, uint32_t pixelsize, uint32_t tex_width, uint32_t tex_height, uint32_t num_glyphs, uint32_t line_gap_add);

LfTexture lf_tex_create(const char* filepath, bool flip, LfTextureFiltering filter); 

void lf_free_font(LfFont* font);  

void lf_flush();

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

LfClickableItemState lf_div_begin(LfVec2i pos, LfVec2i size);

void lf_div_end();

LfClickableItemState lf_button(const char* text);
 
LfUIElementProps lf_style_color(LfVec4f color);

void lf_new_line();

void lf_update_input();

LfTextProps lf_get_text_props(const char* str);

void lf_text(const char* str);

LfVec2i lf_get_div_size();

void lf_set_ptr_x(float x);

void lf_set_ptr_y(float y);

void lf_image(uint32_t tex_id, uint32_t width, uint32_t height);
