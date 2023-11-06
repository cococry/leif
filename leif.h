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

// --- Events ---
typedef struct {
    int32_t keycode;
    bool happened, pressed;
} LfKeyEvent;

typedef struct {
    int32_t button_code;
    bool happened, pressed;
} LfMouseButtonEvent;

typedef struct {
    int32_t x, y;
    bool happened;
} LfCursorPosEvent;

typedef struct {
    int32_t xoffset, yoffset;
    bool happened;
} LfScrollEvent;

typedef struct {
    int32_t charcode;
    bool happened;
} LfCharEvent;

typedef struct {
    bool happened;
} LfGuiReEstablishEvent;

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
    char* placeholder;
    bool selected;
    int32_t initial_width;
} LfInputField;

typedef struct {
    void* val;
    int32_t handle_pos;
    bool _init;
    float min, max;
    bool held, selcted;
    uint32_t width;
    uint32_t height;
} LfSlider;

typedef enum {
    LF_RELEASED = -1,
    LF_IDLE = 0,
    LF_HOVERED = 1,
    LF_CLICKED = 2, 
    LF_HELD =32,
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
    float corner_radius;
} LfUIElementProps;

typedef struct {
    vec2s pos, size;
} LfAABB;

typedef struct {
    LfUIElementProps button_props, div_props, text_props, image_props, 
                     inputfield_props, checkbox_props, slider_props;
    LfFont font;
} LfTheme;

typedef void (*LfMenuItemCallback)(uint32_t*);

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

LfClickableItemState lf_image_button(LfTexture img);

LfClickableItemState lf_button_fixed(const char* text, int32_t width, int32_t height);
 
void lf_next_line();

vec2s lf_text_dimension(const char* str);

float lf_get_text_end(const char* str, float start_x);

void lf_text(const char* text);

vec2s lf_get_div_size();

void lf_set_ptr_x(float x);

void lf_set_ptr_y(float y);

float lf_get_ptr_x();

float lf_get_ptr_y();

void lf_image(LfTexture tex);

LfTheme* lf_theme();

void lf_begin();

void lf_end();

void lf_input_text(LfInputField* input);

void lf_input_int(LfInputField* input);

void lf_input_float(LfInputField* input);

void lf_set_text_wrap(bool wrap);

void lf_push_font(LfFont* font);

void lf_pop_font();

void lf_checkbox(const char* text, bool* val, uint32_t tex);

void lf_rect(float width, float height, vec4s color, float corner_radius);

LfClickableItemState lf_slider_int(LfSlider* slider);

LfClickableItemState lf_progress_bar_int(LfSlider* slider);

LfClickableItemState lf_progress_stripe_int(LfSlider* slider);

int32_t lf_menu_item_list(const char** items, uint32_t item_count, int32_t selected_index, vec4s selected_color, LfMenuItemCallback per_cb, bool vertical);

LfTextProps lf_text_render(vec2s pos, const char* str, LfFont font, int32_t wrap_point, 
        int32_t stop_point_x, int32_t start_point_x, int32_t stop_point_y, int32_t start_point_y, bool no_render, vec4s color);

void lf_rect_render(vec2s pos, vec2s size, vec4s color, vec4s border_color, float border_width, float corner_radius);

void lf_image_render(vec2s pos, vec4s color, LfTexture tex, vec4s border_color, float border_width, float corner_radius);

bool lf_point_intersects_aabb(vec2s p, LfAABB aabb);

bool lf_aabb_intersects_aabb(LfAABB a, LfAABB b);

void lf_push_style_props(LfUIElementProps props);

void lf_pop_style_props();

void lf_push_text_start_x(int32_t start_x);

void lf_push_text_stop_x(int32_t stop_x);

void lf_push_text_start_y(int32_t start_y);

void lf_push_text_stop_y(int32_t stop_y);

void lf_pop_text_start_x();

void lf_pop_text_stop_x();

void lf_pop_text_start_y();

void lf_pop_text_stop_y();

bool lf_hovered(vec2s pos, vec2s size);

void lf_flush();

void lf_renderer_begin();

LfCursorPosEvent lf_mouse_move_event();

LfMouseButtonEvent lf_mouse_button_event();

LfScrollEvent lf_mouse_scroll_event();

LfKeyEvent lf_key_event();

LfCharEvent lf_char_event();

LfGuiReEstablishEvent lf_gui_reastablish_event();
