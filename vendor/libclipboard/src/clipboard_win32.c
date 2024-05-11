/**
 *  \file clipboard_win32.c
 *  \brief Windows implementation of the clipboard.
 *
 *  \copyright Copyright (C) 2016 Jeremy Tan.
 *             This file is released under the MIT license.
 *             See LICENSE for details.
 */

#include "libclipboard.h"

#ifdef LIBCLIPBOARD_BUILD_WIN32

#include "libclipboard.h"
#include <windows.h>
#include <tchar.h>


/** Win32 Implementation of the clipboard context **/
struct clipboard_c {
    /** Window to be used for associating with clipborad data **/
    HWND hwnd;
    /** Max number of retries to obtain clipboard lock **/
    int max_retries;
    /** Delay (ms) between retries **/
    int retry_delay;

    /** malloc **/
    clipboard_malloc_fn malloc;
    /** calloc **/
    clipboard_calloc_fn calloc;
    /** realloc **/
    clipboard_realloc_fn realloc;
    /** free **/
    clipboard_free_fn free;
};

/**
 *  \brief Window procedure for clipboard context.
 *
 *  \param [in] hwnd The window handle i.e. cb->hwnd
 *  \param [in] msg The window message
 *  \param [in] wParam Window message param1
 *  \param [in] lParam Window message param2
 *  \return Value dependant on message processed
 *
 *  See also: WindowProc on MSDN
 */
static LRESULT CALLBACK clipboard_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/**
 *  \brief Attempt to obtain the clipboard lock
 *
 *  \param [in] cb The clipboard context
 *  \return true iff OpenClipboard succeeded.
 *
 *  \details This function will only attempt a retry if OpenClipboard fails
 *           due to ERROR_ACCESS_DENIED, which indicates that someone else
 *           currently has the lock. It will also only retry so long as
 *           cb->max_retries > 0.
 */
static bool get_clipboard_lock(clipboard_c *cb) {
    int retries = cb->max_retries;
    int last_error = 0;

    do {
        if (OpenClipboard(cb->hwnd)) {
            return true;
        } else if ((last_error = GetLastError()) != ERROR_ACCESS_DENIED) {
            return false;
        }

        Sleep(cb->retry_delay);
    } while (retries-- > 0);

    SetLastError(last_error);
    return false;
}

LCB_API clipboard_c *LCB_CC clipboard_new(clipboard_opts *cb_opts) {
    clipboard_calloc_fn calloc_fn = cb_opts && cb_opts->user_calloc_fn ? cb_opts->user_calloc_fn : calloc;
    clipboard_c *ret = calloc_fn(1, sizeof(clipboard_c));
    if (ret == NULL) {
        return NULL;
    }
    LCB_SET_ALLOCATORS(ret, cb_opts);

    ret->max_retries = LCB_WIN32_MAX_RETRIES_DEFAULT;
    ret->retry_delay = LCB_WIN32_RETRY_DELAY_DEFAULT;

    if (cb_opts) {
        if (cb_opts->win32.max_retries > 0) {
            ret->max_retries = cb_opts->win32.max_retries;
        } else if (cb_opts->win32.max_retries < 0) {
            ret->max_retries = 0;
        }
        if (cb_opts->win32.retry_delay > 0) {
            ret->max_retries = cb_opts->win32.retry_delay;
        } else if (cb_opts->win32.retry_delay < 0) {
            ret->retry_delay = 0;
        }
    }

    WNDCLASSEX wndclass = {0};
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.lpfnWndProc = clipboard_wnd_proc;
    wndclass.lpszClassName = _T("libclipboard");
    if (!RegisterClassEx(&wndclass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        ret->free(ret);
        return NULL;
    }
    ret->hwnd = CreateWindowEx(0, wndclass.lpszClassName, wndclass.lpszClassName,
                               0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
    if (ret->hwnd == NULL) {
        ret->free(ret);
        return NULL;
    }

    return ret;
}

LCB_API void LCB_CC clipboard_free(clipboard_c *cb) {
    if (cb == NULL) {
        return;
    }

    DestroyWindow(cb->hwnd);
    cb->free(cb);
}

LCB_API void LCB_CC clipboard_clear(clipboard_c *cb, clipboard_mode mode) {
    if (cb == NULL) {
        return;
    }

    if (!get_clipboard_lock(cb)) {
        return;
    }

    EmptyClipboard();
    CloseClipboard();
}

LCB_API bool LCB_CC clipboard_has_ownership(clipboard_c *cb, clipboard_mode mode) {
    return cb && (GetClipboardOwner() == cb->hwnd);
}

LCB_API char *LCB_CC clipboard_text_ex(clipboard_c *cb, int *length, clipboard_mode mode) {
    char *ret = NULL;

    if (cb == NULL || !get_clipboard_lock(cb)) {
        return NULL;
    }

    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData == NULL) {
        CloseClipboard();
        return NULL;
    }

    wchar_t *pData = (wchar_t *)GlobalLock(hData);
    if (pData == NULL) {
        CloseClipboard();
        return NULL;
    }

    int len_required =
        WideCharToMultiByte(CP_UTF8, 0, pData, -1, NULL, 0, NULL, NULL);
    if (len_required != 0 && (ret = cb->calloc(len_required, sizeof(char))) != NULL) {
        int len_actual =
            WideCharToMultiByte(CP_UTF8, 0, pData, -1, ret, len_required,
                                NULL, NULL);

        if (len_actual == 0) {
            cb->free(ret);
            ret = NULL;
        } else if (length) {
            /* Length excluding the NULL terminator */
            *length = len_actual - 1;
        }
    }

    GlobalUnlock(hData);
    CloseClipboard();
    return ret;
}

LCB_API bool LCB_CC clipboard_set_text_ex(clipboard_c *cb, const char *src, int length, clipboard_mode mode) {
    if (cb == NULL || src == NULL || length == 0) {
        return false;
    }

    if (length > 0) {
        length += 1;
    }

    int len_required = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src,
                                           length, NULL, 0);
    if (len_required == 0) {
        return false;
    }

    HGLOBAL buf = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t) * len_required);
    if (buf == NULL) {
        return false;
    }

    wchar_t *locked;
    if ((locked = (wchar_t *)GlobalLock(buf)) == NULL) {
        GlobalFree(buf);
        return false;
    }

    int ret = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src, length,
                                  locked, len_required);
    /* NULL-terminate */
    locked[len_required - 1] = 0;
    GlobalUnlock(buf);

    if (ret == 0 || !get_clipboard_lock(cb)) {
        GlobalFree(buf);
        return false;
    } else {
        /* EmptyClipboard must be called to properly update clipboard ownership */
        EmptyClipboard();
        if (SetClipboardData(CF_UNICODETEXT, buf) == NULL) {
            CloseClipboard();
            GlobalFree(buf);
            return false;
        }
    }

    /* CloseClipboard appears to change the sequence number... */
    CloseClipboard();
    return true;
}

#endif /* LIBCLIPBOARD_BUILD_WIN32 */
