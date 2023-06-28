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

void lf_init(uint32_t display_width, uint32_t display_height);

void lf_resize_display(uint32_t display_width, uint32_t display_height);

void lf_draw_rect(LfVec2i pos, LfVec2i size, LfVec4f color);

void lf_flush();
