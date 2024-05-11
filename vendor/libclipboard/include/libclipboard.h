/**
 *  \file  libclipboard.h
 *  \brief Main header file
 *
 *  \copyright Copyright (C) 2016 Jeremy Tan.
 *             This file is released under the MIT license.
 *             See LICENSE for details.
 */

#ifndef _LIBCLIPBOARD_H_
#define _LIBCLIPBOARD_H_

#include "libclipboard-config.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* Symbol exports and calling convention logic */
#if defined(_WIN32) || defined(__CYGWIN__)
#  define LIBCLIPBOARD_DLL_IMPORT __declspec(dllimport)
#  define LIBCLIPBOARD_DLL_EXPORT __declspec(dllexport)
#  define LIBCLIPBOARD_DLL_LOCAL
#  if defined(LIBCLIPBOARD_USE_STDCALL) && !defined(_WIN64)
#    define LCB_CC __stdcall
#  else
#    define LCB_CC
#  endif
#else
#  if __GNUC__ >= 4
#    define LIBCLIPBOARD_DLL_IMPORT __attribute__ ((visibility ("default")))
#    define LIBCLIPBOARD_DLL_EXPORT __attribute__ ((visibility ("default")))
#    define LIBCLIPBOARD_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#  else
#    define LIBCLIPBOARD_DLL_IMPORT
#    define LIBCLIPBOARD_DLL_EXPORT
#    define LIBCLIPBOARD_DLL_LOCAL
#  endif
#  define LCB_CC
#endif

#ifdef LIBCLIPBOARD_BUILD_SHARED
#  ifdef clipboard_EXPORTS /* Defined by CMake when we're compiling the shared library */
#    define LCB_API LIBCLIPBOARD_DLL_EXPORT
#  else
#    define LCB_API LIBCLIPBOARD_DLL_IMPORT
#  endif
#  define LCB_LOCAL LIBCLIPBOARD_DLL_LOCAL
#else
#  define LCB_API
#  define LCB_LOCAL
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Default action timeout of 1500ms **/
#define LCB_X11_ACTION_TIMEOUT_DEFAULT 1500
/** Default transfer size (X11 only), default 1MB (must be multiple of 4) **/
#define LCB_X11_TRANSFER_SIZE_DEFAULT  1048576
/** Default max number of retries to try to obtain clipboard lock **/
#define LCB_WIN32_MAX_RETRIES_DEFAULT 5
/** Default delay in ms between retries to obtain clipboard lock **/
#define LCB_WIN32_RETRY_DELAY_DEFAULT 5

/**
 *  \brief For internal use only. Initialises custom allocators.
 *
 *  \param [out] cb Clipboard context
 *  \param [in] opts Clipboard options
 */
#define LCB_SET_ALLOCATORS(cb, opts) do { \
    (cb)->malloc = (opts) && (opts)->user_malloc_fn ? (opts)->user_malloc_fn : malloc; \
    (cb)->calloc = (opts) && (opts)->user_calloc_fn ? (opts)->user_calloc_fn : calloc; \
    (cb)->realloc = (opts) && (opts)->user_realloc_fn  ? (opts)->user_realloc_fn : realloc; \
    (cb)->free = (opts) && (opts)->user_free_fn ? (opts)->user_free_fn : free; \
} while(0)

/** Custom malloc function signature **/
typedef void *(*clipboard_malloc_fn)(size_t size);
/** Custom calloc function signature **/
typedef void *(*clipboard_calloc_fn)(size_t nmemb, size_t size);
/** Custom realloc function signature **/
typedef void *(*clipboard_realloc_fn)(void *ptr, size_t size);
/** Custom free function signature **/
typedef void (*clipboard_free_fn)(void *ptr);

/**
 *  Determines which clipboard is used in called functions.
 */
typedef enum clipboard_mode {
    /** The primary (global) clipboard **/
    LCB_CLIPBOARD = 0,
    /** The (global) mouse selection clipboard **/
    LCB_PRIMARY,
    /** The (global) mouse selection clipboard; for backwards compatibility **/
    LCB_SELECTION = LCB_PRIMARY,
    /** The largely unused (global) secondary selection clipboard **/
    LCB_SECONDARY,
    /** Sentinel value for end of clipboard modes **/
    LCB_MODE_END
} clipboard_mode;

/**
 *  Options to be passed on instantiation.
 */
typedef struct clipboard_opts {
    /* I would put the OS specific opts in a union, but anonymous unions are non-standard */
    /* Typing out union names is too much effort */

    /** X11 specific options **/
    struct clipboard_opts_x11 {
        /** Max time (ms) to wait for action to complete **/
        int action_timeout;
        /** Transfer size, in bytes. Must be a multiple of 4. **/
        uint32_t transfer_size;
        /** The name of the X11 display (NULL for default - DISPLAY env. var.) **/
        const char *display_name;
    } x11;

    /** Win32 specific options **/
    struct clipboard_opts_win32 {
        /**
         *  Max number of retries to try to obtain clipboard lock.
         *  If max_retries is zero, the default value will be used.
         *  Specify a negative value for zero retries.
         */
        int max_retries;
        /**
         *  Delay in ms between retries to obtain clipboard lock.
         *  If retry_delay is zero, the default value will be used.
         *  Specify a negative value for no (zero) delay.
         */
        int retry_delay;
    } win32;

    /** User specified malloc (NULL for default) **/
    clipboard_malloc_fn user_malloc_fn;
    /** User specified calloc (NULL for default) **/
    clipboard_calloc_fn user_calloc_fn;
    /** User specified realloc (NULL for default) **/
    clipboard_realloc_fn user_realloc_fn;
    /** User specified free (NULL for default) **/
    clipboard_free_fn user_free_fn;
} clipboard_opts;

/** Opaque data structure for a clipboard context/instance **/
typedef struct clipboard_c clipboard_c;

/**
 *  \brief Instantiates a new clipboard instance of the given type.
 *
 *  \param [in] cb_opts Implementation specific options (optional).
 *  \return The new clipboard instance, or NULL on failure.
 */
LCB_API clipboard_c *LCB_CC clipboard_new(clipboard_opts *cb_opts);

/**
 *  \brief Frees associated clipboard data from the provided structure.
 *
 *  \param [in] cb The clipboard to be freed.
 */
LCB_API void LCB_CC clipboard_free(clipboard_c *cb);

/**
 *  \brief Clears the contents of the given clipboard.
 *
 *  \param [in] cb The clipboard to clear.
 *  \param [in] mode Which clipboard to clear (platform dependent)
 */
LCB_API void LCB_CC clipboard_clear(clipboard_c *cb, clipboard_mode mode);

/**
 *  \brief Determines if the clipboard is currently owned
 *
 *  \param [in] cb The clipboard to check
 *  \param [in] mode Which clipboard to clear (platform dependent)
 *  \return true iff the clipboard data is owned by the provided instance.
 */
LCB_API bool LCB_CC clipboard_has_ownership(clipboard_c *cb, clipboard_mode mode);

/**
 *  \brief Retrieves the text currently held on the clipboard.
 *
 *  \param [in] cb The clipboard to retrieve from
 *  \param [out] length Returns the length of the retrieved data, excluding
 *                      the NULL terminator (optional).
 *  \param [in] mode Which clipboard to clear (platform dependent)
 *  \return A copy to the retrieved text. This must be free()'d by the user.
 *          Note that the text is encoded in UTF-8 format.
 */
LCB_API char *LCB_CC clipboard_text_ex(clipboard_c *cb, int *length, clipboard_mode mode);

/**
 *  \brief Simplified version of clipboard_text_ex
 *
 *  \param [in] cb The clipboard to retrieve from
 *  \return As per clipboard_text_ex.
 *
 *  \details This function assumes LCB_CLIPBOARD as the clipboard mode.
 */
LCB_API char *LCB_CC clipboard_text(clipboard_c *cb);

/**
 *  \brief Sets the text for the provided clipboard.
 *
 *  \param [in] cb The clipboard to set the text.
 *  \param [in] src The UTF-8 encoded text to be set in the clipboard.
 *  \param [in] length The length of text to be set (excluding the NULL
 *                     terminator).
 *  \param [in] mode Which clipboard to clear (platform dependent)
 *  \return true iff the clipboard was set (false on error)
 *
 *  \details If the length parameter is -1, src is treated as a NULL-terminated
 *           string and its length will be determined automatically.
 */
LCB_API bool LCB_CC clipboard_set_text_ex(clipboard_c *cb, const char *src, int length, clipboard_mode mode);

/**
 *  \brief Simplified version of clipboard_set_text_ex
 *
 *  \param [in] cb The clipboard to set the text.
 *  \param [in] src The UTF-8 encoded NULL terminated string to be set.
 *  \return true iff the clipboard was set (false on error)
 *
 *  \details This function assumes LCB_CLIPBOARD as the clipboard mode.
 */
LCB_API bool LCB_CC clipboard_set_text(clipboard_c *cb, const char *src);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCLIPBOARD_H_ */
