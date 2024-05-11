/**
 *  \file clipboard_x11.c
 *  \brief X11 implementation of the clipboard.
 *
 *  \copyright Copyright (C) 2016 Jeremy Tan.
 *             This file is released under the MIT license.
 *             See LICENSE for details.
 */

#define _POSIX_C_SOURCE 199309L

#include "libclipboard.h"

#ifdef LIBCLIPBOARD_BUILD_X11

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

#include <xcb/xcb.h>
#include <pthread.h>

/* X11 headers are borked */
#if defined(__MINGW32__) && defined(gettimeofday)
#  undef gettimeofday
#endif

#define VALID_MODE(x) ((x) >= LCB_CLIPBOARD && (x) < LCB_MODE_END)

/**
 *  Enumeration of standard X11 atom identifiers
 */
typedef enum std_x_atoms {
    /** The TARGETS atom identifier **/
    X_ATOM_TARGETS = 0,
    /** The MULTIPLE atom identifier **/
    X_ATOM_MULTIPLE,
    /** The TIMESTAMP atom identifier **/
    X_ATOM_TIMESTAMP,
    /** The INCR atom identifier **/
    X_ATOM_INCR,
    /** The CLIPBOARD atom identifier **/
    X_ATOM_CLIPBOARD,
    /** The UTF8_STRING atom identifier **/
    X_ATOM_UTF8_STRING,
    /** End marker sentinel **/
    X_ATOM_END
} std_x_atoms;

/**
 *  Union to simplify getting interned atoms from XCB
 */
typedef union atom_c {
    /** The atom **/
    xcb_atom_t atom;
    /** The cookie returned by xcb_intern_atom **/
    xcb_intern_atom_cookie_t cookie;
} atom_c;

/**
 *  Contains selection data
 */
typedef struct selection_c {
    /** Determines if we currently own the given selection **/
    bool has_ownership;
    /** The pointer to the data of the selection **/
    unsigned char *data;
    /** The length (in bytes) of the selection data **/
    size_t length;
    /** The type of data held in this selection **/
    xcb_atom_t target;
    /** The X11 atom for the selection mode e.g. XA_PRIMARY **/
    xcb_atom_t xmode;
} selection_c;

/** X11 Implementation of the clipboard context **/
struct clipboard_c {
    /** XCB Display connection **/
    xcb_connection_t *xc;
    /** XCB Default screen **/
    xcb_screen_t *xs;
    /** Standard atoms **/
    atom_c std_atoms[X_ATOM_END];
    /** Our window to use for messages **/
    xcb_window_t xw;
    /** Action timeout (ms) **/
    int action_timeout;
    /** Transfer size (bytes) **/
    uint32_t transfer_size;

    /** Event loop thread **/
    pthread_t event_loop;
    /** Indicates true iff event_loop is initted **/
    bool event_loop_initted;
    /** Mutex for access to context data **/
    pthread_mutex_t mu;
    /** Indicates true iff mu is initted **/
    bool mu_initted;
    /** Condition variable to notify when action is complete **/
    pthread_cond_t cond;
    /** Indicates true iff cond is initted **/
    bool cond_initted;

    /** Selection data **/
    selection_c selections[LCB_MODE_END];

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
 *  The standard atom names. These values should match the
 *  std_x_atoms enumeration.
 */
const char * const g_std_atom_names[X_ATOM_END] = {
    "TARGETS", "MULTIPLE", "TIMESTAMP", "INCR",
    "CLIPBOARD", "UTF8_STRING",
};

/**
 *  \brief Interns the list of atoms
 *
 *  \param [in] xc The XCB connection.
 *  \param [out] atoms The location to store interned atoms.
 *  \param [in] atom_names The names of the atoms to intern.
 *  \param [in] number The number of atoms to intern.
 *  \return true iff all atoms were interned.
 */
static bool x11_intern_atoms(xcb_connection_t *xc, atom_c *atoms, const char * const *atom_names, int number) {
    for (int i = 0; i < number; i++) {
        atoms[i].cookie = xcb_intern_atom(xc, 0,
                                          strlen(atom_names[i]), atom_names[i]);
    }

    for (int i  = 0; i < number; i++) {
        xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(xc,
                                         atoms[i].cookie, NULL);
        if (reply == NULL) {
            return false;
        }

        atoms[i].atom = reply->atom;
        free(reply); /* XCB: Do not use custom allocators */
    }

    return true;
}

/**
 *  \brief Retrieves the screen from the given connection.
 *
 *  \param [in] xc The XCB connection.
 *  \param [in] screen The screen number to retrieve.
 *  \return The screen, or NULL on error.
 *
 *  Based on https://xcb.freedesktop.org/xlibtoxcbtranslationguide/
 */
static xcb_screen_t *x11_get_screen(xcb_connection_t *xc, int screen) {
    xcb_screen_iterator_t iter;

    iter = xcb_setup_roots_iterator(xcb_get_setup(xc));
    for (; iter.rem; screen--, xcb_screen_next(&iter)) {
        if (screen == 0) {
            return iter.data;
        }
    }

    return NULL;
}

/**
 *  \brief Clears the selection data held in our cache on SelectionClear.
 *
 *  \param [in] cb The clipboard context.
 *  \param [in] e The selection clear event.
 */
static void x11_clear_selection(clipboard_c *cb, xcb_selection_clear_event_t *e) {
    if (e->owner != cb->xw) {
        return;
    }

    for (int i = 0; i < LCB_MODE_END; i++) {
        selection_c *sel = &cb->selections[i];
        if (sel->xmode == e->selection && (pthread_mutex_lock(&cb->mu) == 0)) {
            cb->free(sel->data);
            sel->data = NULL;
            sel->length = 0;
            sel->has_ownership = false;
            sel->target = XCB_NONE;
            pthread_mutex_unlock(&cb->mu);
            break;
        }
    }
}

/**
 *  \brief Retrieves the selection into our cache on SelectionNotify
 *
 *  \param [in] cb The clipboard context.
 *  \param [in] e The selection notify event.
 *
 *  The INCR format is not supported.
 */
static void x11_retrieve_selection(clipboard_c *cb, xcb_selection_notify_event_t *e) {
    unsigned char *buf = NULL;
    size_t bufsiz = 0, bytes_after = 1;
    xcb_get_property_reply_t *reply = NULL;
    xcb_atom_t actual_type;
    uint8_t actual_format;

    if (e->property != XCB_ATOM_PRIMARY && e->property != XCB_ATOM_SECONDARY && e->property != cb->std_atoms[X_ATOM_CLIPBOARD].atom) {
        fprintf(stderr, "x11_retrieve_selection: [Warn] Unknown selection property returned: %d\n", e->property);
        return;
    }

    while (bytes_after > 0) {
        free(reply); /* XCB: Do not use custom allocators */
        xcb_get_property_cookie_t ck = xcb_get_property(cb->xc, true, cb->xw,
                                       e->property, XCB_ATOM_ANY,
                                       bufsiz / 4, cb->transfer_size / 4);
        reply = xcb_get_property_reply(cb->xc, ck, NULL);
        /* reply->format should be 8, 16 or 32. */
        if (reply == NULL || (bufsiz > 0 && (reply->format != actual_format || reply->type != actual_type)) || ((reply->format % 8) != 0)) {
            fprintf(stderr, "x11_retrieve_selection: [Err] Invalid return value from xcb_get_property_reply\n");
            cb->free(buf);
            buf = NULL;
            break;
        }

        if (bufsiz == 0) {
            actual_type = reply->type;
            actual_format = reply->format;
        }

        /* Todo: Check for INCR */

        int nitems = xcb_get_property_value_length(reply);
        if (nitems > 0) {
            if ((bufsiz % 4) != 0) {
                fprintf(stderr, "x11_retrieve_selection: [Err] Got more data but read data size is not a multiple of 4\n");
                cb->free(buf);
                buf = NULL;
                break;
            }

            size_t unit_size = (reply->format / 8);
            buf = cb->realloc(buf, unit_size * (bufsiz + nitems));
            if (buf == NULL) {
                fprintf(stderr, "x11_retrieve_selection: [Err] realloc failed\n");
                break;
            }

            memcpy(buf + bufsiz, xcb_get_property_value(reply), nitems * unit_size);
            bufsiz += nitems * unit_size;
        }

        bytes_after = reply->bytes_after;
    }
    free(reply); /* XCB: Do not use custom allocators */

    if (buf != NULL && (pthread_mutex_lock(&cb->mu) == 0)) {
        selection_c *sel = NULL;
        for (int i = 0; i < LCB_MODE_END; i++) {
            if (cb->selections[i].xmode == e->property) {
                sel = &cb->selections[i];
                break;
            }
        }

        if (sel != NULL && sel->target == actual_type) {
            cb->free(sel->data);
            sel->data = buf;
            sel->length = bufsiz;
            buf = NULL;
        } else {
            fprintf(stderr, "x11_retrieve_selection: [Warn] Mismatched selection: actual_type=%d\n", actual_type);
        }

        pthread_cond_broadcast(&cb->cond);
        pthread_mutex_unlock(&cb->mu);
    }
    cb->free(buf);
}

/**
 *  \brief Sends the selection data to the requestor on SelectionRequest
 *
 *  \param [in] cb The clipboard context.
 *  \param [in] e The selection request event.
 *  \return true iff the data was sent (requestor's property was changed)
 *
 *  Not currently ICCCM compliant as MULTIPLE target is unsupported. Also
 *  not compliant because we're not supplying a proper TIMESTAMP value.
 */
static bool x11_transmit_selection(clipboard_c *cb, xcb_selection_request_event_t *e) {
    /* Default location to store data if none specified */
    if (e->property == XCB_NONE) {
        e->property = e->target;
    }

    if (e->target == cb->std_atoms[X_ATOM_TARGETS].atom) {
        xcb_atom_t targets[] = {
            cb->std_atoms[X_ATOM_TIMESTAMP].atom,
            cb->std_atoms[X_ATOM_TARGETS].atom,
            cb->std_atoms[X_ATOM_UTF8_STRING].atom
        };
        xcb_change_property(cb->xc, XCB_PROP_MODE_REPLACE, e->requestor,
                            e->property, XCB_ATOM_ATOM,
                            sizeof(xcb_atom_t) * 8,
                            sizeof(targets) / sizeof(xcb_atom_t), targets);
    } else if (e->target == cb->std_atoms[X_ATOM_TIMESTAMP].atom) {
        xcb_timestamp_t cur = XCB_CURRENT_TIME;
        xcb_change_property(cb->xc, XCB_PROP_MODE_REPLACE, e->requestor,
                            e->property, XCB_ATOM_INTEGER, sizeof(cur) * 8,
                            1, &cur);
    } else if (e->target == cb->std_atoms[X_ATOM_UTF8_STRING].atom) {
        selection_c *sel = NULL;
        if (pthread_mutex_lock(&cb->mu) != 0) {
            return false;
        }

        for (int i = 0; i < LCB_MODE_END; i++) {
            if (cb->selections[i].xmode == e->selection) {
                sel = &cb->selections[i];
                break;
            }
        }

        if (sel == NULL || !sel->has_ownership || sel->data == NULL || sel->target != e->target) {
            pthread_mutex_unlock(&cb->mu);
            return false;
        }

        xcb_change_property(cb->xc, XCB_PROP_MODE_REPLACE, e->requestor,
                            e->property, e->target, 8, sel->length, sel->data);
        pthread_mutex_unlock(&cb->mu);
    } else {
        /* Unknown target */
        return false;
    }

    return true;
}

/**
 *  \brief The main event loop to process window messages.
 *
 *  \param [in] arg The clipboard context.
 *
 *  This thread will run indefinitely until the clipboard context's window
 *  is destroyed. It *must* receive a DestroyNotify message to end.
 */
static void *x11_event_loop(void *arg) {
    clipboard_c *cb = (clipboard_c *)arg;
    xcb_generic_event_t *e;

    while ((e = xcb_wait_for_event(cb->xc))) {
        if (e->response_type == 0) {
            /* I think this cast is appropriate... */
            xcb_generic_error_t *err = (xcb_generic_error_t *) e;
            fprintf(stderr, "x11_event_loop: [Warn] Received X11 error: %d\n", err->error_code);
            free(e); /* XCB: Do not use custom allocators */
            continue;
        }

        switch (e->response_type & ~0x80) {
            case XCB_DESTROY_NOTIFY: {
                xcb_destroy_notify_event_t *evt = (xcb_destroy_notify_event_t *)e;
                if (evt->window == cb->xw) {
                    free(e); /* XCB: Do not use custom allocators */
                    return NULL;
                }
            }
            break;
            case XCB_SELECTION_CLEAR: {
                x11_clear_selection(cb, (xcb_selection_clear_event_t *)e);
            }
            break;
            case XCB_SELECTION_NOTIFY: {
                x11_retrieve_selection(cb, (xcb_selection_notify_event_t *)e);
            }
            break;
            case XCB_SELECTION_REQUEST: {
                xcb_selection_request_event_t *req = (xcb_selection_request_event_t *)e;
                xcb_selection_notify_event_t notify = {0};
                notify.response_type = XCB_SELECTION_NOTIFY;
                notify.time = XCB_CURRENT_TIME;
                notify.requestor = req->requestor;
                notify.selection = req->selection;
                notify.target = req->target;
                notify.property = x11_transmit_selection(cb, req) ? req->property : XCB_NONE;
                xcb_send_event(cb->xc, false, req->requestor, XCB_EVENT_MASK_PROPERTY_CHANGE, (char *)&notify);
                xcb_flush(cb->xc);
            }
            break;
            case XCB_PROPERTY_NOTIFY: {
            }
            break;
            default: {
                /* Ignore unknown messages */
            }
        }
        free(e); /* XCB: Do not use custom allocators */
    }

    fprintf(stderr, "x11_event_loop: [Warn] xcb_wait_for_event returned NULL\n");
    return NULL;
}

LCB_API clipboard_c *LCB_CC clipboard_new(clipboard_opts *cb_opts) {
    clipboard_opts defaults = {
        .x11.display_name = NULL,
        .x11.action_timeout = LCB_X11_ACTION_TIMEOUT_DEFAULT,
        .x11.transfer_size = LCB_X11_TRANSFER_SIZE_DEFAULT,
    };

    if (cb_opts == NULL) {
        cb_opts = &defaults;
    }

    clipboard_calloc_fn calloc_fn = cb_opts->user_calloc_fn ? cb_opts->user_calloc_fn : calloc;
    clipboard_c *cb = calloc_fn(1, sizeof(clipboard_c));
    if (cb == NULL) {
        return NULL;
    }
    LCB_SET_ALLOCATORS(cb, cb_opts);

    cb->action_timeout = cb_opts->x11.action_timeout > 0 ?
                         cb_opts->x11.action_timeout : LCB_X11_ACTION_TIMEOUT_DEFAULT;
    /* Round down to nearest multiple of 4 */
    cb->transfer_size = (cb_opts->x11.transfer_size / 4) * 4;
    if (cb->transfer_size == 0) {
        cb->transfer_size = LCB_X11_TRANSFER_SIZE_DEFAULT;
    }

    cb->mu_initted = pthread_mutex_init(&cb->mu, NULL) == 0;
    if (!cb->mu_initted) {
        clipboard_free(cb);
        return NULL;
    }

    cb->cond_initted = pthread_cond_init(&cb->cond, NULL) == 0;
    if (!cb->cond_initted) {
        clipboard_free(cb);
        return NULL;
    }

    int preferred_screen;
    cb->xc = xcb_connect(cb_opts->x11.display_name, &preferred_screen);
    assert(cb->xc != NULL); /* Docs say return is never NULL */
    if (xcb_connection_has_error(cb->xc) != 0) {
        clipboard_free(cb);
        return NULL;
    }
    cb->xs = x11_get_screen(cb->xc, preferred_screen);
    assert(cb->xs != NULL);

    if (!x11_intern_atoms(cb->xc, cb->std_atoms, g_std_atom_names, X_ATOM_END)) {
        clipboard_free(cb);
        return NULL;
    }

    cb->selections[LCB_CLIPBOARD].xmode = cb->std_atoms[X_ATOM_CLIPBOARD].atom;
    cb->selections[LCB_PRIMARY].xmode   = XCB_ATOM_PRIMARY;
    cb->selections[LCB_SECONDARY].xmode = XCB_ATOM_SECONDARY;

    /* Structure notify mask to get DestroyNotify messages */
    /* Property change mask for PropertyChange messages */
    uint32_t event_mask = XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_PROPERTY_CHANGE;
    cb->xw = xcb_generate_id(cb->xc);
    xcb_generic_error_t *err = xcb_request_check(cb->xc,
                               xcb_create_window_checked(cb->xc,
                                       XCB_COPY_FROM_PARENT, cb->xw, cb->xs->root,
                                       0, 0, 10, 10, 0,  XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                       cb->xs->root_visual,
                                       XCB_CW_EVENT_MASK, &event_mask));
    if (err != NULL) {
        cb->xw = 0;
        /* Am I meant to free this? */
        free(err); /* XCB: Do not use custom allocators */
        clipboard_free(cb);
        return NULL;
    }

    cb->event_loop_initted = pthread_create(&cb->event_loop, NULL,
                                            x11_event_loop, (void *)cb) == 0;
    if (!cb->event_loop_initted) {
        clipboard_free(cb);
        return NULL;
    }

    return cb;
}

LCB_API void LCB_CC clipboard_free(clipboard_c *cb) {
    if (cb == NULL) {
        return;
    }

    if (cb->event_loop_initted) {
        /* This should send a DestroyNotify message as the termination condition */
        xcb_destroy_window(cb->xc, cb->xw);
        xcb_flush(cb->xc);
        pthread_join(cb->event_loop, NULL);
    } else if (cb->xw != 0) {
        xcb_destroy_window(cb->xc, cb->xw);
    }

    if (cb->xc != NULL) {
        xcb_disconnect(cb->xc);
    }

    if (cb->cond_initted) {
        pthread_cond_destroy(&cb->cond);
    }
    if (cb->mu_initted) {
        pthread_mutex_destroy(&cb->mu);
    }

    /* Free selection data */
    for (int i = 0; i < LCB_MODE_END; i++) {
        if (cb->selections[i].data != NULL) {
            cb->free(cb->selections[i].data);
        }
    }

    cb->free(cb);
}

LCB_API void LCB_CC clipboard_clear(clipboard_c *cb, clipboard_mode mode) {
    if (cb == NULL || cb->xc == NULL) {
        return;
    }

    xcb_atom_t sel;

    switch (mode) {
        case LCB_CLIPBOARD:
            sel = cb->std_atoms[X_ATOM_CLIPBOARD].atom;
            break;
        case LCB_PRIMARY:
            sel = XCB_ATOM_PRIMARY;
            break;
        case LCB_SECONDARY:
            sel = XCB_ATOM_SECONDARY;
            break;
        default:
            return;
    }

    xcb_set_selection_owner(cb->xc, XCB_NONE, sel, XCB_CURRENT_TIME);
    xcb_flush(cb->xc);
}

LCB_API bool LCB_CC clipboard_has_ownership(clipboard_c *cb, clipboard_mode mode) {
    bool ret = false;

    if (!VALID_MODE(mode)) {
        return false;
    }

    if (cb && (pthread_mutex_lock(&cb->mu) == 0)) {
        ret = cb->selections[mode].has_ownership;
        pthread_mutex_unlock(&cb->mu);
    }
    return ret;
}

/**
 *  \brief Copies the selection data into a newly allocated buffer
 *
 *  \param [in] cb The clipboard context
 *  \param [in] sel The selection context
 *  \param [out] ret The return location
 *  \param [out] length The length of the returned data (optional)
 */
static void retrieve_text_selection(clipboard_c *cb, selection_c *sel, char **ret, int *length) {
    if (sel->data != NULL && sel->target == cb->std_atoms[X_ATOM_UTF8_STRING].atom) {
        *ret = cb->malloc(sizeof(char) * (sel->length + 1));
        if (*ret != NULL) {
            memcpy(*ret, sel->data, sel->length);
            (*ret)[sel->length] = '\0';

            if (length != NULL) {
                *length = sel->length;
            }
        }
    }
}

LCB_API char LCB_CC *clipboard_text_ex(clipboard_c *cb, int *length, clipboard_mode mode) {
    char *ret = NULL;

    if (cb == NULL || !VALID_MODE(mode)) {
        return NULL;
    }

    if (pthread_mutex_lock(&cb->mu) == 0) {
        selection_c *sel = &cb->selections[mode];
        if (sel->has_ownership) {
            retrieve_text_selection(cb, sel, &ret, length);
        } else {
            /* Convert selection & wait for reply */
            struct timeval now;
            struct timespec timeout;
            int pret = 0;

            xcb_get_selection_owner_reply_t *owner = xcb_get_selection_owner_reply(cb->xc,
                    xcb_get_selection_owner(cb->xc, sel->xmode), NULL);
            if (owner == NULL || owner->owner == 0) {
                /* No selection owner; no data available */
                pthread_mutex_unlock(&cb->mu);
                free(owner); /* XCB: Do not use custom allocators */
                return NULL;
            }
            free(owner); /* XCB: Do not use custom allocators */

            /* Unset any old value */
            cb->free(sel->data);
            sel->data = NULL;
            sel->length = 0;

            sel->target = cb->std_atoms[X_ATOM_UTF8_STRING].atom;
            xcb_convert_selection(cb->xc, cb->xw, sel->xmode,
                                  sel->target, sel->xmode, XCB_CURRENT_TIME);
            xcb_flush(cb->xc);

            /* Calculate timeout */
            gettimeofday(&now, NULL);
            timeout.tv_sec = now.tv_sec + (cb->action_timeout / 1000);
            timeout.tv_nsec = (now.tv_usec * 1000UL) + ((cb->action_timeout % 1000) * 1000000UL);
            if (timeout.tv_nsec >= 1000000000UL) {
                timeout.tv_sec += timeout.tv_nsec / 1000000000UL;
                timeout.tv_nsec = timeout.tv_nsec % 1000000000UL;
            }

            while (pret == 0 && sel->data == NULL) {
                pret = pthread_cond_timedwait(&cb->cond, &cb->mu, &timeout);
            }

            retrieve_text_selection(cb, sel, &ret, length);
        }

        pthread_mutex_unlock(&cb->mu);
    }

    return ret;
}

LCB_API bool LCB_CC clipboard_set_text_ex(clipboard_c *cb, const char *src, int length, clipboard_mode mode) {
    bool ret = false;

    if (cb == NULL || src == NULL || length == 0 || !VALID_MODE(mode)) {
        return false;
    }

    if (pthread_mutex_lock(&cb->mu) == 0) {
        selection_c *sel = &cb->selections[mode];
        if (sel->data != NULL) {
            cb->free(sel->data);
        }
        if (length < 0) {
            length = strlen(src);
        }

        sel->data = cb->malloc(sizeof(char) * (length + 1));
        if (sel->data != NULL) {
            memcpy(sel->data, src, length);
            sel->data[length] = '\0';
            sel->length = length;
            sel->has_ownership = true;
            sel->target = cb->std_atoms[X_ATOM_UTF8_STRING].atom;
            xcb_set_selection_owner(cb->xc, cb->xw, sel->xmode, XCB_CURRENT_TIME);
            xcb_flush(cb->xc);
            ret = true;
        }

        pthread_mutex_unlock(&cb->mu);
    }

    return ret;
}

#endif /* LIBCLIPBOARD_BUILD_X11 */
