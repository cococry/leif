#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <cglm/cglm.h>
#include <cglm/struct.h>
#include <wchar.h>
#include <stdio.h>

#define LF_PRIMARY_ITEM_COLOR (LfColor){133, 138, 148, 255} 
#define LF_SECONDARY_ITEM_COLOR (LfColor){96, 100, 107, 255}

#define LF_NO_COLOR (LfColor){0, 0, 0, 0}
#define LF_WHITE (LfColor){255, 255, 255, 255}
#define LF_BLACK (LfColor){0, 0, 0, 255}
#define LF_RED (LfColor){255, 0, 0, 255}
#define LF_GREEN (LfColor){0, 255, 0, 255}
#define LF_BLUE (LfColor){0, 0, 255, 255}

typedef struct {
    uint8_t r, g, b, a;
} LfColor;

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
    uint32_t id;
    uint32_t width, height;
} LfTexture;
typedef struct {
    void* cdata;
    void* font_info;
    uint32_t tex_width, tex_height;
    uint32_t line_gap_add, font_size;
    LfTexture bitmap;

    uint32_t num_glyphs;
} LfFont;

typedef enum {
    LF_TEX_FILTER_LINEAR = 0,
    LF_TEX_FILTER_NEAREST
} LfTextureFiltering;

typedef struct {
    float width, height;
    int32_t end_x, end_y;
    uint32_t rendered_count;
} LfTextProps;

typedef struct {
    int32_t cursor_index, width, height, start_height;
    char* buf;
    uint32_t buf_size;
    char* placeholder;
    bool selected;

    uint32_t max_chars;

    int32_t selection_start, selection_end, 
            mouse_selection_start, mouse_selection_end;
    int32_t selection_dir, mouse_dir;

    bool _init;


    void (*char_callback)(char);
    
} LfInputField;

typedef struct {
    void* val;
    int32_t handle_pos;
    bool _init;
    float min, max;
    bool held, selcted;
    float width;
    float height;
    uint32_t handle_size;
    LfColor handle_color;
} LfSlider;

typedef enum {
    LF_RELEASED = -1,
    LF_IDLE = 0,
    LF_HOVERED = 1,
    LF_CLICKED = 2,
    LF_HELD = 3,
} LfClickableItemState;


typedef struct {
    LfColor color, hover_color;
    LfColor text_color, hover_text_color;
    LfColor border_color;
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
                     inputfield_props, checkbox_props, slider_props, scrollbar_props;
    LfFont font;
    bool div_smooth_scroll;
    float div_scroll_acceleration, div_scroll_max_veclocity;
    float div_scroll_amount_px;
    float div_scroll_velocity_deceleration;

    float scrollbar_width;
} LfTheme;

typedef struct {
    int64_t id;
    LfAABB aabb;
    LfClickableItemState interact_state;

    bool scrollable;
    
    vec2s total_area;
} LfDiv;

typedef void (*LfMenuItemCallback)(uint32_t*);

void lf_init_glfw(uint32_t display_width, uint32_t display_height, void* glfw_window);

void lf_terminate();

LfTheme lf_default_theme();

LfTheme lf_get_theme();

void lf_set_theme(LfTheme theme);

void lf_resize_display(uint32_t display_width, uint32_t display_height);

LfFont lf_load_font(const char* filepath, uint32_t size);

LfFont lf_load_font_ex(const char* filepath, uint32_t size, uint32_t bitmap_w, uint32_t bitmap_h, uint32_t num_glyphs);

LfTexture lf_load_texture(const char* filepath, bool flip, LfTextureFiltering filter);

LfTexture lf_load_texture_resized(const char* filepath, bool flip, LfTextureFiltering filter, uint32_t w, uint32_t h);

LfTexture lf_load_texture_resized_factor(const char* filepath, bool flip, LfTextureFiltering filter, float wfactor, float hfactor);

LfTexture lf_load_texture_from_memory(const void* data, size_t size, bool flip, LfTextureFiltering filter);

LfTexture lf_load_texture_from_memory_resized(const void* data, size_t size, bool flip, LfTextureFiltering filter, uint32_t w, uint32_t h);

LfTexture lf_load_texture_from_memory_resized_factor(const void* data, size_t size, bool flip, LfTextureFiltering filter, float wfactor, float hfactor);

unsigned char* lf_load_texture_data(const char* filepath, int32_t* width, int32_t* height, int32_t* channels, bool flip);

unsigned char* lf_load_texture_data_resized(const char* filepath, int32_t w, int32_t h, int32_t* channels, bool flip);

unsigned char* lf_load_texture_data_resized_factor(const char* filepath, int32_t wfactor, int32_t hfactor, int32_t* width, int32_t* height, int32_t* channels, bool flip);

unsigned char* lf_load_texture_data_from_memory(const void* data, size_t size, int32_t* width, int32_t* height, int32_t* channels, bool flip);

unsigned char* lf_load_texture_data_from_memory_resized(const void* data, size_t size, int32_t* channels, bool flip, uint32_t w, uint32_t h);

unsigned char* lf_load_texture_data_from_memory_resized_to_fit(const void* data, size_t size, int32_t* width, int32_t* height, int32_t* channels, bool flip, uint32_t w, uint32_t h);


void lf_create_texture_from_image_data(LfTextureFiltering filter, uint32_t* id, int32_t width, int32_t height, int32_t channels, unsigned char* data); 

void lf_free_texture(LfTexture* tex);

void lf_free_font(LfFont* font);

LfFont lf_load_font_asset(const char* asset_name, const char* file_extension, uint32_t font_size);

LfTexture lf_load_texture_asset(const char* asset_name, const char* file_extension); 

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

bool lf_mouse_button_went_down_on_div(uint32_t button);

bool lf_mouse_button_is_released_on_div(uint32_t button);

bool lf_mouse_button_changed_on_div(uint32_t button);

double lf_get_mouse_x();

double lf_get_mouse_y();

double lf_get_mouse_x_delta();

double lf_get_mouse_y_delta();

double lf_get_mouse_scroll_x();

double lf_get_mouse_scroll_y();

#define lf_div_begin(pos, size, scrollable) {\
    static float scroll = 0.0f; \
    static float scroll_velocity = 0.0f; \
    _lf_div_begin_loc(pos, size, scrollable, &scroll, &scroll_velocity, __FILE__, __LINE__);\
}

#define lf_div_begin_ex(pos, size, scrollable, scroll_ptr, scroll_velocity_ptr) _lf_div_begin_loc(pos, size, scrollable, scroll_ptr, scroll_velocity_ptr, __FILE__, __LINE__);

LfDiv _lf_div_begin_loc(vec2s pos, vec2s size, bool scrollable, float* scroll, 
        float* scroll_velocity, const char* file, int32_t line);

void lf_div_end();

LfClickableItemState _lf_item_loc(vec2s size,  const char* file, int32_t line);
#define lf_item(size) _lf_item_loc(size, __FILE__, __LINE__)

#define lf_button(text) _lf_button_loc(text, __FILE__, __LINE__)
LfClickableItemState _lf_button_loc(const char* text, const char* file, int32_t line);

#define lf_button_wide(text) _lf_button_loc_wide(text, __FILE__, __LINE__)
LfClickableItemState _lf_button_wide_loc(const wchar_t* text, const char* file, int32_t line);

#define lf_image_button(img) _lf_image_button_loc(img, __FILE__, __LINE__)
LfClickableItemState _lf_image_button_loc(LfTexture img, const char* file, int32_t line);

#define lf_image_button_fixed(img, width, height) _lf_image_button_fixed_loc(img, width, height, __FILE__, __LINE__)
LfClickableItemState _lf_image_button_fixed_loc(LfTexture img, float width, float height, const char* file, int32_t line);

#define lf_button_fixed(text, width, height) _lf_button_fixed_loc(text, width, height, __FILE__, __LINE__)
LfClickableItemState _lf_button_fixed_loc(const char* text, float width, float height, const char* file, int32_t line);

#define lf_button_fixed_wide(text, width, height) _lf_button_fixed_loc_wide(text, width, height, __FILE__, __LINE__)
LfClickableItemState _lf_button_fixed_wide_loc(const wchar_t* text, float width, float height, const char* file, int32_t line);

#define lf_slider_int(slider) _lf_slider_int_loc(slider, __FILE__, __LINE__)
LfClickableItemState _lf_slider_int_loc(LfSlider* slider, const char* file, int32_t line);

#define lf_slider_int_inl_ex(slider_val, slider_min, slider_max, slider_width, slider_height, slider_handle_size, state) { \
  static LfSlider slider = { \
    .val = slider_val, \
    .handle_pos = 0, \
    .min = slider_min, \
    .max = slider_max, \
    .width = slider_width, \
    .height = slider_height, \
    .handle_size = slider_handle_size \
  }; \
  state = lf_slider_int(&slider); \
} \

#define lf_slider_int_inl(slider_val, slider_min, slider_max, state) lf_slider_int_inl_ex(slider_val, slider_min, slider_max, lf_get_current_div().aabb.size.x / 2.0f, 5, 0, state)

#define lf_progress_bar_val(width, height, min, max, val) _lf_progress_bar_val_loc(width, height, min, max, val, __FILE__, __LINE__)
LfClickableItemState _lf_progress_bar_val_loc(float width, float height, int32_t min, int32_t max, int32_t val, const char* file, int32_t line);

#define lf_progress_bar_int(val, min, max, width, height) _lf_progress_bar_int_loc(val, min, max, width, height, __FILE__, __LINE__)
LfClickableItemState _lf_progress_bar_int_loc(float val, float min, float max, float width, float height, const char* file, int32_t line);

#define lf_progress_stripe_int(slider) _lf_progresss_stripe_int_loc(slider , __FILE__, __LINE__)
LfClickableItemState _lf_progress_stripe_int_loc(LfSlider* slider, const char* file, int32_t line);

#define lf_checkbox(text, val, tick_color, tex_color) _lf_checkbox_loc(text, val, tick_color, tex_color, __FILE__, __LINE__)
LfClickableItemState _lf_checkbox_loc(const char* text, bool* val, LfColor tick_color, LfColor tex_color, const char* file, int32_t line);

#define lf_checkbox_wide(text, val, tick_color, tex_color) _lf_checkbox_wide_loc(text, val, tick_color, tex_color, __FILE__, __LINE__)
LfClickableItemState _lf_checkbox_wide_loc(const wchar_t* text, bool* val, LfColor tick_color, LfColor tex_color, const char* file, int32_t line);

#define lf_menu_item_list(items, item_count, selected_index, per_cb, vertical) _lf_menu_item_list_loc(__FILE__, __LINE__, items, item_count, selected_index, per_cb, vertical)
int32_t _lf_menu_item_list_loc(const char** items, uint32_t item_count, int32_t selected_index, LfMenuItemCallback per_cb, bool vertical, const char* file, int32_t line);

#define lf_menu_item_list_wide(items, item_count, selected_index, per_cb, vertical) _lf_menu_item_list_loc_wide(__FILE__, __LINE__, items, item_count, selected_index, per_cb, vertical)
int32_t _lf_menu_item_list_loc_wide(const wchar_t** items, uint32_t item_count, int32_t selected_index, LfMenuItemCallback per_cb, bool vertical, const char* file, int32_t line);

#define lf_dropdown_menu(items, placeholder, item_count, width, height, selected_index, opened) _lf_dropdown_menu_loc(items, placeholder, item_count, width, height, selected_index, opened, __FILE__, __LINE__)
void _lf_dropdown_menu_loc(const char** items, const char* placeholder, uint32_t item_count, float width, float height, int32_t* selected_index, bool* opened, const char* file, int32_t line);

#define lf_dropdown_menu_wide(items, placeholder, item_count, width, height, selected_index, opened) _lf_dropdown_menu_loc_wide(items, placeholder, item_count, width, height, selected_index, opened, __FILE__, __LINE__)
void _lf_dropdown_menu_loc_wide(const wchar_t** items, const wchar_t* placeholder, uint32_t item_count, float width, float height, int32_t* selected_index, bool* opened, const char* file, int32_t line);

#define lf_input_text_inl_ex(buffer, buffer_size, input_width, placeholder_str)         \
    {                                                                                   \
    static LfInputField input = {                                                       \
        .cursor_index = 0,                                                              \
        .width = input_width,                                                           \
        .buf = buffer,                                                                  \
        .buf_size = buffer_size,                                                        \
        .placeholder = (char*)placeholder_str,                                          \
        .selected = false                                                               \
    };                                                                                  \
    _lf_input_text_loc(&input, __FILE__, __LINE__);                                     \
    }                                                                                   \

#define lf_input_text_inl(buffer, buffer_size) lf_input_text_inl_ex(buffer, buffer_size, (int32_t)(lf_get_current_div().aabb.size.x / 2), "")

#define lf_input_text(input) _lf_input_text_loc(input, __FILE__, __LINE__)
void _lf_input_text_loc(LfInputField* input, const char* file, int32_t line);

#define lf_input_int(input) _lf_input_int_loc(input, __FILE__, __LINE__)
void _lf_input_int_loc(LfInputField* input, const char* file, int32_t line);

#define lf_input_float(input) _lf_input_float_loc(input, __FILE__, __LINE__)
void _lf_input_float_loc(LfInputField* input, const char* file, int32_t line);

bool lf_input_grabbed();

void lf_div_grab(LfDiv div);

void lf_div_ungrab();

bool lf_div_grabbed();

LfDiv lf_get_grabbed_div();

#define lf_begin() _lf_begin_loc(__FILE__, __LINE__)
void _lf_begin_loc(const char* file, int32_t line);

void lf_end();

void lf_next_line();

vec2s lf_text_dimension(const char* str);

vec2s lf_text_dimension_ex(const char* str, float wrap_point);

vec2s lf_text_dimension_wide(const wchar_t* str);

vec2s lf_text_dimension_wide_ex(const wchar_t* str, float wrap_point);

vec2s lf_button_dimension(const char* text);

float lf_get_text_end(const char* str, float start_x);

void lf_text(const char* text);

void lf_text_wide(const wchar_t* text);

void lf_set_text_wrap(bool wrap);

LfDiv lf_get_current_div();

LfDiv lf_get_selected_div();

LfDiv* lf_get_current_div_ptr();

LfDiv* lf_get_selected_div_ptr();

void lf_set_ptr_x(float x);

void lf_set_ptr_y(float y);

void lf_set_ptr_x_absolute(float x);

void lf_set_ptr_y_absolute(float y);

float lf_get_ptr_x();

float lf_get_ptr_y();

uint32_t lf_get_display_width();

uint32_t lf_get_display_height();

void lf_push_font(LfFont* font);

void lf_pop_font();

LfTextProps lf_text_render(vec2s pos, const char* str, LfFont font, LfColor color, 
        int32_t wrap_point, vec2s stop_point, bool no_render, bool render_solid, int32_t start_index, int32_t end_index);

LfTextProps lf_text_render_wchar(vec2s pos, const wchar_t* str, LfFont font, int32_t wrap_point, bool no_render, LfColor color);

void lf_rect_render(vec2s pos, vec2s size, LfColor color, LfColor border_color, float border_width, float corner_radius);

void lf_image_render(vec2s pos, LfColor color, LfTexture tex, LfColor border_color, float border_width, float corner_radius);

bool lf_point_intersects_aabb(vec2s p, LfAABB aabb);

bool lf_aabb_intersects_aabb(LfAABB a, LfAABB b);

void lf_push_style_props(LfUIElementProps props);

void lf_pop_style_props();

bool lf_hovered(vec2s pos, vec2s size);

LfCursorPosEvent lf_mouse_move_event();

LfMouseButtonEvent lf_mouse_button_event();

LfScrollEvent lf_mouse_scroll_event();

LfKeyEvent lf_key_event();

LfCharEvent lf_char_event();

void lf_set_cull_start_x(float x);

void lf_set_cull_start_y(float y);

void lf_set_cull_end_x(float x);

void lf_set_cull_end_y(float y);  

void lf_unset_cull_start_x();

void lf_unset_cull_start_y();

void lf_unset_cull_end_x();

void lf_unset_cull_end_y();

void lf_set_image_color(LfColor color);

void lf_unset_image_color();

void lf_set_current_div_scroll(float scroll); 

void lf_set_current_div_scroll_velocity(float scroll_velocity);

void lf_set_line_height(uint32_t line_height);

uint32_t lf_get_line_height();

void lf_set_line_should_overflow(bool overflow);

void lf_set_div_hoverable(bool hoverable);

void lf_push_element_id(int64_t id);

void lf_pop_element_id();

LfColor lf_color_brightness(LfColor color, float brightness);

LfColor lf_color_alpha(LfColor color, uint8_t a);

vec4s lf_color_to_zto(LfColor color);

LfColor lf_color_from_hex(uint32_t hex);

LfColor lf_color_from_zto(vec4s zto);

void lf_image(LfTexture tex);

void lf_rect(float width, float height, LfColor color, float corner_radius);

void lf_seperator();

