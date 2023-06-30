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
    TEX_FILTER_LINEAR = 0,
    TEX_FILTER_NEAREST
} LfTextureFiltering;

void lf_init(uint32_t display_width, uint32_t display_height);

void lf_resize_display(uint32_t display_width, uint32_t display_height);

void lf_draw_rect(LfVec2i pos, LfVec2i size, LfVec4f color);

void lf_draw_image(LfVec2i pos, LfVec2i size, LfVec4f color, const char* filepath, bool flipped, LfTextureFiltering filter);

void lf_draw_str(LfVec2i pos, LfVec4f color, const char* str, LfFont font);

LfFont lf_load_font(const char* filepath, uint32_t pixelsize, uint32_t tex_width, uint32_t tex_height, uint32_t num_glyphs, uint32_t line_gap_add);

void lf_free_font(LfFont* font);

void lf_flush();
