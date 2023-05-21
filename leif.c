#include "leif.h"
#include <string.h>

static LfState state;

bool lf_init() {
    state.xdisplay = XOpenDisplay(NULL);
    if(!state.xdisplay)
        return false;

    state.xscreen = DefaultScreen(state.xdisplay);
    if(state.xscreen == None) 
        return false;

    state.xroot = DefaultRootWindow(state.xdisplay);
    if(state.xroot == None)
        return false;

    memset(&state.win_hints, -1, sizeof(state.win_hints));
    return true;
}

void lf_win_hint(LfWinHint hint, int32_t value) {
    switch (hint) {
        case LF_RESIZABLE:
            state.win_hints.resizable = (bool)value;
            break;
        case LF_BORDER_COLOR:
            state.win_hints.border_color = value;
            break;
        case LF_BORDER_WIDTH:
            state.win_hints.border_width = value;
            break;
    }    
}

LfWin lf_win(const char* name, uint32_t width, uint32_t height, int32_t x, int32_t y,uint32_t color) {
    LfWin win;
    
    int32_t border_width = (state.win_hints.border_width == -1) ? 3 : state.win_hints.border_width;
    int32_t border_color = (state.win_hints.border_color == -1) ? 0x000000 : state.win_hints.border_color;
    int32_t win_bg_color = (state.win_hints.win_bg_color == -1) ? 0xffffff : state.win_hints.win_bg_color;

    win.xwin = XCreateSimpleWindow(state.xdisplay, state.xroot, 
                                   width, height, x, y, border_width, border_color, win_bg_color);
    XMapWindow(state.xdisplay, win.xwin);
    XRaiseWindow(state.xdisplay, win.xwin);

    XStoreName(state.xdisplay, win.xwin, name);
    win.x = x;
    win.y = y;
    win.width = width;
    win.height = height;
    win.name = name;
    win.open = true;

    return win;
}

void lf_win_close(LfWin* win) {
    win->open = false;
    XUnmapWindow(state.xdisplay, win->xwin);
    XDestroyWindow(state.xdisplay, win->xwin);
}

void lf_terminate() {
    XCloseDisplay(state.xdisplay);
}
