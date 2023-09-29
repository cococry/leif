#pragma once 
#include <stdbool.h>
#include <stdint.h>
#include <cglm/cglm.h>
#include <cglm/struct.h>

#define LF_RGBA(r, g, b, a) r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f
#define LF_RED LF_RGBA(255.0f, 0.0f, 0.0f, 255.0f)
#define LF_GREEN LF_RGBA(0.0f, 255.0f, 0.0f, 255.0f)
#define LF_BLUE LF_RGBA(0.0f, 0.0f, 255.0f, 255.0f)
#define LF_WHITE LF_RGBA(255.0f, 255.0f, 255.0f, 255.0f)
#define LF_BLACK LF_RGBA(0.0f, 0.0f, 0.0f, 255.0f)

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
    int32_t end_x, start_x;
} LfTextProps;

typedef struct {
    int32_t cursor_index, width, height, start_height;
    char* buf;
    void* val;
    vec2s char_positions[512];
    char* placeholder;
    bool selected;
} LfInputField;

typedef struct {
    void* val;
    int32_t handle_pos;
    float min, max;
    bool held, selcted;
} LfSlider;

typedef enum {
    LF_HOVERED = -1,
    LF_IDLE = 0,
    LF_CLICKED = 1, 
    LF_HELD = 2,
} LfClickableItemState;

typedef struct {
    vec4s color;
    vec4s text_color;
    vec4s border_color;
    float padding;
    float margin_left; 
    float margin_right;
    float margin_top; 
    float margin_bottom;
    float border_width;
} LfUIElementProps;

typedef struct {
    vec2s pos, size;
} LfAABB;

typedef struct {
    LfUIElementProps button_props, div_props, text_props, image_props, 
                     inputfield_props, checkbox_props, slider_props;
    LfFont font;
} LfTheme;


void lf_init_glfw(uint32_t display_width, uint32_t display_height, const char* font_path, LfTheme* theme, void* glfw_window);

void lf_terminate();

LfTheme lf_default_theme(const char* font_path, uint32_t font_size);

void lf_resize_display(uint32_t display_width, uint32_t display_height);

LfFont lf_load_font(const char* filepath, uint32_t size);

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

LfClickableItemState lf_div_begin(vec2s pos, vec2s size);

void lf_div_end();

LfClickableItemState lf_button(const char* text);

LfClickableItemState lf_button_fixed(const char* text, int32_t width, int32_t height);
 
void lf_next_line();

vec2s lf_text_dimension(const char* str);

float lf_get_text_end(const char* str, float start_x);

void lf_text(const char* fmt, ...);

vec2s lf_get_div_size();

void lf_set_ptr_x(float x);

void lf_set_ptr_y(float y);

void lf_image(LfTexture tex);

LfTheme* lf_theme();

void lf_update();

void lf_input_text(LfInputField* input);

void lf_input_int(LfInputField* input);

void lf_input_float(LfInputField* input);

void lf_set_text_wrap(bool wrap);

void lf_set_item_color(vec4s color);

void lf_unset_item_color();

void lf_set_text_color(vec4s color);

void lf_unset_text_color();

void lf_push_font(LfFont* font);

void lf_pop_font();

void lf_checkbox(const char* text, bool* val, uint32_t tex);

void lf_rect(float width, float height, vec4s color);

void lf_slider_int(LfSlider* slider);


LfTextProps lf_text_render(vec2s pos, const char* str, LfFont font, int32_t wrap_point, 
        int32_t stop_point, int32_t start_point, bool no_render, vec4s color);

void lf_rect_render(vec2s pos, vec2s size, vec4s color);

void lf_image_render(vec2s pos, vec4s color, LfTexture tex);

bool lf_point_intersects_aabb(vec2s p, LfAABB aabb);

bool lf_aabb_intersects_aabb(LfAABB a, LfAABB b);
