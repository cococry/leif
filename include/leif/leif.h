#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <cglm/cglm.h>
#include <cglm/struct.h>
#include <wchar.h>
#include <libclipboard.h>
#include <stdio.h>


#define LF_PRIMARY_ITEM_COLOR (LfColor){133, 138, 148, 255} 
#define LF_SECONDARY_ITEM_COLOR (LfColor){96, 100, 107, 255}

#define LF_NO_COLOR (LfColor){0, 0, 0, 0}
#define LF_WHITE (LfColor){255, 255, 255, 255}
#define LF_BLACK (LfColor){0, 0, 0, 255}
#define LF_RED (LfColor){255, 0, 0, 255}
#define LF_GREEN (LfColor){0, 255, 0, 255}
#define LF_BLUE (LfColor){0, 0, 255, 255}

#ifdef LF_GLFW
typedef struct GLFWwindow GLFWwindow;
typedef void (*GLFWkeyfun)(GLFWwindow* window, int key, int scancode, int action, int mods);
typedef void (*GLFWmousebuttonfun)(GLFWwindow* window, int button, int action, int mods);
typedef void (*GLFWscrollfun)(GLFWwindow* window, double xoffset, double yoffset);
typedef void (*GLFWcursorposfun)(GLFWwindow* window, double xpos, double yoffset);
#define LF_MAX_KEYS 348 // GLFW_KEY_LAST  
#define LF_MAX_MOUSE_BUTTONS 7 // GLFW_MOUSE_BUTTON_LAST 
#define LF_KEY_CALLBACK_t GLFWkeyfun
#define LF_MOUSE_BUTTON_CALLBACK_t GLFWmousebuttonfun
#define LF_SCROLL_CALLBACK_t GLFWscrollfun
#define LF_CURSOR_CALLBACK_t GLFWcursorposfun
#define LF_MAX_KEY_CALLBACKS 4
#define LF_MAX_MOUSE_BTTUON_CALLBACKS 4
#define LF_MAX_SCROLL_CALLBACKS 4
#define LF_MAX_CURSOR_POS_CALLBACKS 4
#else 

#define LF_MAX_KEYS 0
#define LF_MAX_MOUSE_BUTTONS 0
#define LF_KEY_CALLBACK_t void*
#define LF_MOUSE_BUTTON_CALLBACK_t void*
#define LF_SCROLL_CALLBACK_t void*
#define LF_CURSOR_CALLBACK_t void*
#define LF_MAX_KEY_CALLBACKS 0
#define LF_MAX_MOUSE_BTTUON_CALLBACKS 0
#define LF_MAX_SCROLL_CALLBACKS 0
#define LF_MAX_CURSOR_POS_CALLBACKS 0
#endif


#define LF_MAX_RENDER_BATCH 10000
#define LF_MAX_TEX_COUNT_BATCH 32

// -- Struct Defines ---
typedef struct {
  uint32_t id;
} LfShader;

typedef struct {
    uint32_t id;
    uint32_t width, height;
} LfTexture;

typedef struct {
  vec2 pos; // 8 Bytes
  vec4 border_color; // 16 Bytes
  float border_width; // 4 Bytes 
  vec4 color; // 16 Bytes
  vec2 texcoord; // 8 Bytes
  float tex_index; // 4 Bytes
  vec2 scale; // 8 Bytes
  vec2 pos_px; // 8 Bytes
  float corner_radius; // 4 Bytes
  vec2 min_coord, max_coord; // 16 Bytes
} LfVertex; // 88 Bytes per vertex

// State of the batch renderer
typedef struct {
  LfShader shader;
  uint32_t vao, vbo, ibo;
  uint32_t vert_count;
  LfVertex* verts;
  vec4s vert_pos[4];
  LfTexture textures[LF_MAX_TEX_COUNT_BATCH];
  uint32_t tex_index, tex_count,index_count;
} LfRenderState;


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
    void (*insert_override_callback)(void*);
    void (*key_callback)(void*);

    bool retain_height;
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
    float div_scroll_acceleration, div_scroll_max_velocity;
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

typedef struct {
    bool is_dragging;
    vec2s start_cursor_pos;
    float start_scroll;
} LfDragState;

typedef struct {
  bool keys[LF_MAX_KEYS];
  bool keys_changed[LF_MAX_KEYS];
} LfKeyboard;

typedef struct {
  bool buttons_current[LF_MAX_MOUSE_BUTTONS];
  bool buttons_last[LF_MAX_MOUSE_BUTTONS];

  double xpos, ypos, xpos_last, ypos_last, xpos_delta, ypos_delta;
  bool first_mouse_press; 
  double xscroll_delta, yscroll_delta;
} LfMouse;


// State of input 
typedef struct {
  LfKeyboard keyboard;
  LfMouse mouse;

  // List of callbacks (user defined)
  LF_KEY_CALLBACK_t key_cbs[LF_MAX_KEY_CALLBACKS];
  LF_MOUSE_BUTTON_CALLBACK_t mouse_button_cbs[LF_MAX_MOUSE_BTTUON_CALLBACKS];
  LF_SCROLL_CALLBACK_t scroll_cbs[LF_MAX_SCROLL_CALLBACKS];
  LF_CURSOR_CALLBACK_t cursor_pos_cbs[LF_MAX_CURSOR_POS_CALLBACKS];

  uint32_t key_cb_count, mouse_button_cb_count, scroll_cb_count, cursor_pos_cb_count;

  // Event references 
  LfKeyEvent key_ev;
  LfMouseButtonEvent mb_ev;
  LfCursorPosEvent cp_ev;
  LfScrollEvent scr_ev;
  LfCharEvent ch_ev;
} LfInputState;


typedef struct {
  LfUIElementProps* data;
  uint32_t count, cap;
} LfPropsStack;

typedef struct {
  bool init;

  // Window
  uint32_t dsp_w, dsp_h;
  void* window_handle;

  LfRenderState render;
  LfTheme theme;

  LfDiv current_div, prev_div;
  int32_t current_line_height, prev_line_height;
  vec2s pos_ptr, prev_pos_ptr; 

  // Pushable variables
  LfFont* font_stack, *prev_font_stack;
  LfUIElementProps div_props, prev_props_stack;
  LfColor image_color_stack;
  int64_t element_id_stack;

  LfPropsStack props_stack;

  vec2s cull_start, cull_end;

  LfTexture tex_arrow_down, tex_tick;

  bool text_wrap, line_overflow, div_hoverable, input_grabbed;

  uint64_t active_element_id;

  float* scroll_velocity_ptr;
  float* scroll_ptr;

  LfDiv selected_div, selected_div_tmp, scrollbar_div, grabbed_div;

  uint32_t drawcalls;

  bool entered_div;

  bool div_velocity_accelerating;

  float last_time, delta_time;
  clipboard_c* clipboard;

  bool renderer_render; 

  LfDragState drag_state;
} LfState;


typedef void (*LfMenuItemCallback)(uint32_t*);

LfState lf_init_glfw(uint32_t display_width, uint32_t display_height, void* glfw_window);

LfState lf_init_x11(uint32_t display_width, uint32_t display_height);

void lf_terminate(LfState* state);

LfTheme lf_default_theme();

LfTheme lf_get_theme(LfState* state);

void lf_set_theme(LfState* state, LfTheme theme);

void lf_resize_display(LfState* state, uint32_t display_width, uint32_t display_height);

LfFont lf_load_font(const char* filepath, uint32_t size);

LfFont lf_load_font_ex(const char* filepath, uint32_t size, uint32_t bitmap_w, uint32_t bitmap_h);

LfTexture lf_load_texture(const char* filepath, bool flip, LfTextureFiltering filter);

LfTexture lf_load_texture_resized(const char* filepath, bool flip, LfTextureFiltering filter, uint32_t w, uint32_t h);

LfTexture lf_load_texture_resized_factor(const char* filepath, bool flip, LfTextureFiltering filter, float wfactor, float hfactor);

LfTexture lf_load_texture_from_memory(const void* data, size_t size, bool flip, LfTextureFiltering filter);

LfTexture lf_load_texture_from_memory_resized(const void* data, size_t size, bool flip, LfTextureFiltering filter, uint32_t w, uint32_t h);

LfTexture lf_load_texture_from_memory_resized_factor(const void* data, size_t size, bool flip, LfTextureFiltering filter, float wfactor, float hfactor);

LfTexture lf_load_texture_from_memory_resized_to_fit(const void* data, size_t size, bool flip, LfTextureFiltering filter, int32_t container_w, int32_t container_h);

unsigned char* lf_load_texture_data(const char* filepath, int32_t* width, int32_t* height, int32_t* channels, bool flip);

unsigned char* lf_load_texture_data_resized(const char* filepath, int32_t w, int32_t h, int32_t* channels, bool flip);

unsigned char* lf_load_texture_data_resized_factor(const char* filepath, int32_t wfactor, int32_t hfactor, int32_t* width, int32_t* height, int32_t* channels, bool flip);

unsigned char* lf_load_texture_data_from_memory(const void* data, size_t size, int32_t* width, int32_t* height, int32_t* channels, bool flip);

unsigned char* lf_load_texture_data_from_memory_resized(const void* data, size_t size, int32_t* channels, int32_t* o_w, int32_t* o_h, bool flip, uint32_t w, uint32_t h);

unsigned char* lf_load_texture_data_from_memory_resized_to_fit_ex(const void* data, size_t size, int32_t* o_width, int32_t* o_height, int32_t i_channels, 
    int32_t i_width, int32_t i_height, bool flip, int32_t container_w, int32_t container_h);

unsigned char* lf_load_texture_data_from_memory_resized_to_fit(const void* data, size_t size, int32_t* o_width, int32_t* o_height, int32_t* o_channels,
    bool flip, int32_t container_w, int32_t container_h);

unsigned char* lf_load_texture_data_from_memory_resized_factor(const void* data, size_t size, int32_t* width, int32_t* height, int32_t* channels, bool flip, float wfactor, float hfactor);

void lf_create_texture_from_image_data(LfTextureFiltering filter, uint32_t* id, int32_t width, int32_t height, int32_t channels, unsigned char* data); 

void lf_free_texture(LfTexture* tex);

void lf_free_font(LfFont* font);

LfFont lf_load_font_asset(const char* asset_name, const char* file_extension, uint32_t font_size);

LfTexture lf_load_texture_asset(const char* asset_name, const char* file_extension); 

void lf_add_key_callback(LfState* state, void* cb);

void lf_add_mouse_button_callback(LfState* state, void* cb);

void lf_add_scroll_callback(LfState* state, void* cb);

void lf_add_cursor_pos_callback(LfState* state, void* cb);

bool lf_key_went_down(LfState* state, uint32_t key);

bool lf_key_is_down(LfState* state, uint32_t key);

bool lf_key_is_released(LfState* state, uint32_t key);

bool lf_key_changed(LfState* state, uint32_t key);

bool lf_mouse_button_went_down(LfState* state, uint32_t button);

bool lf_mouse_button_is_down(LfState* state, uint32_t button);

bool lf_mouse_button_is_released(LfState* state, uint32_t button);

bool lf_mouse_button_changed(LfState* state, uint32_t button);

bool lf_mouse_button_went_down_on_div(LfState* state, uint32_t button);

bool lf_mouse_button_is_released_on_div(LfState* state, uint32_t button);

bool lf_mouse_button_changed_on_div(LfState* state, uint32_t button);

double lf_get_mouse_x(LfState* state);

double lf_get_mouse_y(LfState* state);

double lf_get_mouse_x_delta(LfState* state);

double lf_get_mouse_y_delta(LfState* state);

double lf_get_mouse_scroll_x(LfState* state);

double lf_get_mouse_scroll_y(LfState* state);

#define lf_div_begin(state, pos, size, scrollable) {\
    static float scroll = 0.0f; \
    static float scroll_velocity = 0.0f; \
    _lf_div_begin_loc(state, pos, size, scrollable, &scroll, &scroll_velocity, __FILE__, __LINE__);\
}

#define lf_div_begin_ex(state, pos, size, scrollable, scroll_ptr, scroll_velocity_ptr) _lf_div_begin_loc(state, pos, size, scrollable, scroll_ptr, scroll_velocity_ptr, __FILE__, __LINE__);

LfDiv* _lf_div_begin_loc(LfState* state, vec2s pos, vec2s size, bool scrollable, float* scroll, 
        float* scroll_velocity, const char* file, int32_t line);

void lf_div_end(LfState* state);

LfClickableItemState _lf_item_loc(LfState* state, vec2s size,  const char* file, int32_t line);
#define lf_item(state, size) _lf_item_loc(state, size, __FILE__, __LINE__)

#define lf_button(state, text) _lf_button_loc(state, text, __FILE__, __LINE__)
LfClickableItemState _lf_button_loc(LfState* state, const char* text, const char* file, int32_t line);

#define lf_button_wide(state, text) _lf_button_wide_loc(state, text, __FILE__, __LINE__)
LfClickableItemState _lf_button_wide_loc(LfState* state, const wchar_t* text, const char* file, int32_t line);

#define lf_image_button(state, img) _lf_image_button_loc(state, img, __FILE__, __LINE__)
LfClickableItemState _lf_image_button_loc(LfState* state, LfTexture img, const char* file, int32_t line);

#define lf_image_button_fixed(state, img, width, height) _lf_image_button_fixed_loc(state, img, width, height, __FILE__, __LINE__)
LfClickableItemState _lf_image_button_fixed_loc(LfState* state, LfTexture img, float width, float height, const char* file, int32_t line);

#define lf_button_fixed(state, text, width, height) _lf_button_fixed_loc(state, text, width, height, __FILE__, __LINE__)
LfClickableItemState _lf_button_fixed_loc(LfState* state, const char* text, float width, float height, const char* file, int32_t line);

#define lf_button_fixed_wide(state, text, width, height) _lf_button_fixed_loc_wide(state, text, width, height, __FILE__, __LINE__)
LfClickableItemState _lf_button_fixed_wide_loc(LfState* state, const wchar_t* text, float width, float height, const char* file, int32_t line);

#define lf_slider_int(state, slider) _lf_slider_int_loc(state, slider, __FILE__, __LINE__)
LfClickableItemState _lf_slider_int_loc(LfState* state, LfSlider* slider, const char* file, int32_t line);

#define lf_slider_int_inl_ex(lf_state, slider_val, slider_min, slider_max, slider_width, slider_height, slider_handle_size, state) { \
  static LfSlider slider = { \
    .val = slider_val, \
    .handle_pos = 0, \
    .min = slider_min, \
    .max = slider_max, \
    .width = slider_width, \
    .height = slider_height, \
    .handle_size = slider_handle_size \
  }; \
  state = lf_slider_int(lf_state, &slider); \
} \

#define lf_slider_int_inl(lf_state, slider_val, slider_min, slider_max, state) lf_slider_int_inl_ex(lf_state, slider_val, slider_min, slider_max, lf_get_current_div().aabb.size.x / 2.0f, 5, 0, state)

#define lf_progress_bar_val(state, width, height, min, max, val) _lf_progress_bar_val_loc(state, width, height, min, max, val, __FILE__, __LINE__)
LfClickableItemState _lf_progress_bar_val_loc(LfState* state, float width, float height, int32_t min, int32_t max, int32_t val, const char* file, int32_t line);

#define lf_progress_bar_int(state, val, min, max, width, height) _lf_progress_bar_int_loc(val, min, max, width, height, __FILE__, __LINE__)
LfClickableItemState _lf_progress_bar_int_loc(LfState* state, float val, float min, float max, float width, float height, const char* file, int32_t line);

#define lf_progress_stripe_int(state, slider) _lf_progresss_stripe_int_loc(state, slider , __FILE__, __LINE__)
LfClickableItemState _lf_progress_stripe_int_loc(LfState* state, LfSlider* slider, const char* file, int32_t line);

#define lf_checkbox(state, text, val, tick_color, tex_color) _lf_checkbox_loc(state, text, val, tick_color, tex_color, __FILE__, __LINE__)
LfClickableItemState _lf_checkbox_loc(LfState* state, const char* text, bool* val, LfColor tick_color, LfColor tex_color, const char* file, int32_t line);

#define lf_checkbox_wide(state, text, val, tick_color, tex_color) _lf_checkbox_wide_loc(state, text, val, tick_color, tex_color, __FILE__, __LINE__)
LfClickableItemState _lf_checkbox_wide_loc(LfState* state, const wchar_t* text, bool* val, LfColor tick_color, LfColor tex_color, const char* file, int32_t line);

#define lf_menu_item_list(state, items, item_count, selected_index, per_cb, vertical) _lf_menu_item_list_loc(state, __FILE__, __LINE__, items, item_count, selected_index, per_cb, vertical)
int32_t _lf_menu_item_list_loc(LfState* state, const char** items, uint32_t item_count, int32_t selected_index, LfMenuItemCallback per_cb, bool vertical, const char* file, int32_t line);

#define lf_menu_item_list_wide(state, items, item_count, selected_index, per_cb, vertical) _lf_menu_item_list_loc_wide(state, __FILE__, __LINE__, items, item_count, selected_index, per_cb, vertical)
int32_t _lf_menu_item_list_loc_wide(LfState* state, const wchar_t** items, uint32_t item_count, int32_t selected_index, LfMenuItemCallback per_cb, bool vertical, const char* file, int32_t line);

#define lf_dropdown_menu(state, items, placeholder, item_count, width, height, selected_index, opened) _lf_dropdown_menu_loc(state, items, placeholder, item_count, width, height, selected_index, opened, __FILE__, __LINE__)
void _lf_dropdown_menu_loc(LfState* state, const char** items, const char* placeholder, uint32_t item_count, float width, float height, int32_t* selected_index, bool* opened, const char* file, int32_t line);

#define lf_dropdown_menu_wide(state, items, placeholder, item_count, width, height, selected_index, opened) _lf_dropdown_menu_loc_widestate, (items, placeholder, item_count, width, height, selected_index, opened, __FILE__, __LINE__)
void _lf_dropdown_menu_loc_wide(LfState* state, const wchar_t** items, const wchar_t* placeholder, uint32_t item_count, float width, float height, int32_t* selected_index, bool* opened, const char* file, int32_t line);

#define lf_input_text_inl_ex(state, buffer, buffer_size, input_width, placeholder_str)  \
    {                                                                                   \
    static LfInputField input = {                                                       \
        .cursor_index = 0,                                                              \
        .width = input_width,                                                           \
        .buf = buffer,                                                                  \
        .buf_size = buffer_size,                                                        \
        .placeholder = (char*)placeholder_str,                                          \
        .selected = false                                                               \
    };                                                                                  \
    _lf_input_text_loc(state, &input, __FILE__, __LINE__);                              \
    }                                                                                   \

#define lf_input_text_inl(state, buffer, buffer_size) lf_input_text_inl_ex(state, buffer, buffer_size, (int32_t)(lf_get_current_div(state).aabb.size.x / 2), "")

#define lf_input_text(state, input) _lf_input_text_loc(state, input, __FILE__, __LINE__)
void _lf_input_text_loc(LfState* state, LfInputField* input, const char* file, int32_t line);

#define lf_input_int(state, input) _lf_input_int_loc(state, input, __FILE__, __LINE__)
void _lf_input_int_loc(LfState* state, LfInputField* input, const char* file, int32_t line);

#define lf_input_float(state, input) _lf_input_float_loc(state, input, __FILE__, __LINE__)
void _lf_input_float_loc(LfState* state, LfInputField* input, const char* file, int32_t line);

void lf_input_insert_char_idx(LfInputField* input, char c, uint32_t idx);

void lf_input_insert_str_idx(LfInputField* input, const char* insert, uint32_t len, uint32_t idx);

void lf_input_field_unselect_all(LfInputField* input);

bool lf_input_grabbed(LfState* state);

void lf_div_grab(LfState* state, LfDiv div);

void lf_div_ungrab(LfState* state);

bool lf_div_grabbed(LfState* state);

LfDiv lf_get_grabbed_div(LfState* state);

#define lf_begin(state) _lf_begin_loc(state, __FILE__, __LINE__)
void _lf_begin_loc(LfState* state, const char* file, int32_t line);

void lf_end(LfState* state);

void lf_next_line(LfState* state);

vec2s lf_text_dimension(LfState* state, const char* str);

vec2s lf_text_dimension_ex(LfState* state, const char* str, float wrap_point);

vec2s lf_text_dimension_wide(LfState* state, const wchar_t* str);

vec2s lf_text_dimension_wide_ex(LfState* state, const wchar_t* str, float wrap_point);

vec2s lf_button_dimension(LfState* state, const char* text);

float lf_get_text_end(LfState* state, const char* str, float start_x);

void lf_text(LfState* state, const char* text);

void lf_text_wide(LfState* state, const wchar_t* text);

void lf_set_text_wrap(LfState* state, bool wrap);

LfDiv lf_get_current_div(LfState* state);

LfDiv lf_get_selected_div(LfState* state);

LfDiv* lf_get_current_div_ptr(LfState* state);

LfDiv* lf_get_selected_div_ptr(LfState* state);

void lf_set_ptr_x(LfState* state, float x);

void lf_set_ptr_y(LfState* state, float y);

void lf_set_ptr_x_absolute(LfState* state, float x);

void lf_set_ptr_y_absolute(LfState* state, float y);

float lf_get_ptr_x(LfState* state);

float lf_get_ptr_y(LfState* state);

void lf_push_font(LfState* state, LfFont* font);

void lf_pop_font();

LfTextProps lf_text_render(LfState* state, vec2s pos, const char* str, LfFont font, LfColor color, 
        int32_t wrap_point, vec2s stop_point, bool no_render, bool render_solid, int32_t start_index, int32_t end_index);

LfTextProps lf_text_render_wchar(LfState* state, vec2s pos, const wchar_t* str, LfFont font, LfColor color, 
                           int32_t wrap_point, vec2s stop_point, bool no_render, bool render_solid, int32_t start_index, int32_t end_index);

void lf_rect_render(LfState* state, vec2s pos, vec2s size, LfColor color, LfColor border_color, float border_width, float corner_radius);

void lf_image_render(LfState* state, vec2s pos, LfColor color, LfTexture tex, LfColor border_color, float border_width, float corner_radius);

bool lf_point_intersects_aabb(vec2s p, LfAABB aabb);

bool lf_aabb_intersects_aabb(LfAABB a, LfAABB b);

void lf_push_style_props(LfState* state, LfUIElementProps props);

void lf_pop_style_props(LfState* state);

bool lf_hovered(LfState* state, vec2s pos, vec2s size);

bool lf_area_hovered(LfState* state, vec2s pos, vec2s size);

LfCursorPosEvent lf_mouse_move_event(LfState* state);

LfMouseButtonEvent lf_mouse_button_event(LfState* state);

LfScrollEvent lf_mouse_scroll_event(LfState* state);

LfKeyEvent lf_key_event(LfState* state);

LfCharEvent lf_char_event(LfState* state);

void lf_set_cull_start_x(LfState* state, float x);

void lf_set_cull_start_y(LfState* state, float y);

void lf_set_cull_end_x(LfState* state, float x);

void lf_set_cull_end_y(LfState* state, float y);  

void lf_unset_cull_start_x(LfState* state);

void lf_unset_cull_start_y(LfState* state);

void lf_unset_cull_end_x(LfState* state);

void lf_unset_cull_end_y(LfState* state);

void lf_set_image_color(LfState* state, LfColor color);

void lf_unset_image_color(LfState* state);

void lf_set_current_div_scroll(LfState* state, float scroll); 

float lf_get_current_div_scroll(LfState* state); 

void lf_set_current_div_scroll_velocity(LfState* state, float scroll_velocity);

float lf_get_current_div_scroll_velocity(LfState* state);

void lf_set_line_height(LfState* state, uint32_t line_height);

uint32_t lf_get_line_height(LfState* state);

void lf_set_line_should_overflow(LfState* state, bool overflow);

void lf_set_div_hoverable(LfState* state, bool hoverable);

void lf_push_element_id(LfState* state, int64_t id);

void lf_pop_element_id(LfState* state);

LfColor lf_color_brightness(LfColor color, float brightness);

LfColor lf_color_alpha(LfColor color, uint8_t a);

vec4s lf_color_to_zto(LfColor color);

LfColor lf_color_from_hex(uint32_t hex);

LfColor lf_color_from_zto(vec4s zto);

void lf_image(LfState* state, LfTexture tex);

void lf_rect(LfState* state, float width, float height, LfColor color, float corner_radius);

void lf_seperator(LfState* state);

void lf_set_clipboard_text(LfState* state, const char* text);

char* lf_get_clipboard_text(LfState* state);

void lf_set_no_render(LfState* state, bool no_render);

void lf_set_input_state(LfInputState input, LfState ui);
