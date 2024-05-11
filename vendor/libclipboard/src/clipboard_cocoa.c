/**
 *  \file clipboard_cocoa.c
 *  \brief OS X (Cocoa) implementation of the clipboard.
 *
 *  \copyright Copyright (C) 2016 Jeremy Tan.
 *             This file is released under the MIT license.
 *             See LICENSE for details.
 */

#include "libclipboard.h"
#ifdef LIBCLIPBOARD_BUILD_COCOA

#include "libclipboard.h"
#include <stdlib.h>

#include <libkern/OSAtomic.h>
#include <Cocoa/Cocoa.h>

/** Cocoa Implementation of the clipboard context **/
struct clipboard_c {
    /** Handle to the global pasteboard. Really this doesn't need to be here... */
    NSPasteboard *pb;
    /** Pasteboard serial at last check **/
    volatile long last_cb_serial;

    /** malloc **/
    clipboard_malloc_fn malloc;
    /** calloc **/
    clipboard_calloc_fn calloc;
    /** realloc **/
    clipboard_realloc_fn realloc;
    /** free **/
    clipboard_free_fn free;
};

LCB_API clipboard_c *LCB_CC clipboard_new(clipboard_opts *cb_opts) {
    clipboard_calloc_fn calloc_fn = cb_opts && cb_opts->user_calloc_fn ? cb_opts->user_calloc_fn : calloc;
    clipboard_c *cb = calloc_fn(1, sizeof(clipboard_c));
    if (cb == NULL) {
        return NULL;
    }
    LCB_SET_ALLOCATORS(cb, cb_opts);
    cb->pb = [NSPasteboard generalPasteboard];
    return cb;
}

LCB_API void LCB_CC clipboard_free(clipboard_c *cb) {
    if (cb) {
        cb->free(cb);
    }
}

LCB_API void LCB_CC clipboard_clear(clipboard_c *cb, clipboard_mode mode) {
    if (cb != NULL) {
        [cb->pb clearContents];
    }
}

LCB_API bool LCB_CC clipboard_has_ownership(clipboard_c *cb, clipboard_mode mode) {
    if (cb) {
        return [cb->pb changeCount] == cb->last_cb_serial;
    }
    return false;
}

LCB_API char *LCB_CC clipboard_text_ex(clipboard_c *cb, int *length, clipboard_mode mode) {
    NSString *ns_clip;
    const char *utf8_clip;
    char *ret;
    size_t len;

    if (cb == NULL) {
        return NULL;
    }

    /* OS X 10.6 and later: Should use NSPasteboardTypeString */
    ns_clip = [cb->pb stringForType:NSStringPboardType];
    if (ns_clip == nil) {
        return NULL;
    }

    utf8_clip = [ns_clip UTF8String];
    len = strlen(utf8_clip);
    ret = cb->malloc(len + 1);
    if (ret != NULL) {
        memcpy(ret, utf8_clip, len);
        ret[len] = '\0';

        if (length) {
            *length = len;
        }
    }
    return ret;
}

LCB_API bool LCB_CC clipboard_set_text_ex(clipboard_c *cb, const char *src, int length, clipboard_mode mode) {
    if (cb == NULL || src == NULL || length == 0) {
        return false;
    }

    NSString *ns_clip;
    bool ret;

    if (length < 0) {
        ns_clip = [[NSString alloc] initWithUTF8String:src];
    } else {
        ns_clip = [[NSString alloc] initWithBytes:src length:length encoding:NSUTF8StringEncoding];
    }

    [cb->pb declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
    ret = [cb->pb setString:ns_clip forType:NSStringPboardType];
    [ns_clip release];

    long serial = [cb->pb changeCount];
    OSAtomicCompareAndSwapLong(cb->last_cb_serial, serial, &cb->last_cb_serial);
    return ret;
}

#endif /* LIBCLIPBOARD_BUILD_COCOA */
