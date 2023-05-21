#pragma once
#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    Window xwin;
    uint32_t width, height;
    int32_t x, y;
    const char* name;
    bool open;
} LfWin;

typedef struct {
    int8_t resizable;
    int32_t border_width;
    int32_t border_color;
    int32_t win_bg_color;
} LfWinHints;

typedef enum {
    LF_RESIZABLE = 0,
    LF_BORDER_WIDTH,
    LF_BORDER_COLOR
} LfWinHint;

typedef struct {
    Display* xdisplay;
    Window xroot;
    int32_t xscreen;
    LfWinHints win_hints;
} LfState;


bool lf_init();

void lf_win_hint(LfWinHint hint, int32_t value);

LfWin lf_win(const char* name, uint32_t width, uint32_t heigt, int32_t x, int32_t y,uint32_t color);

void lf_win_close(LfWin* win);

void lf_terminate();


LfState lf_get_state();
