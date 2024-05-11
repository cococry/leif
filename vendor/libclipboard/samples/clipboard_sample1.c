/**
 *  \file clipboard_sample1.c
 *  \brief Test application for clipboard
 *
 *  \copyright Copyright (C) 2016 Jeremy Tan.
 *             This file is released under the MIT license.
 *             See LICENSE for details.
 */

#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libclipboard.h>

typedef struct app_opts {
    bool opt_set;
    bool interactive;
    bool dump_clipboard;
    bool clear_clipboard;
    bool set_clipboard;
    clipboard_mode mode;
} app_opts;

void interactive_help() {
    printf("help       Prints this help message\n");
    printf("set <text> Sets clipboard text\n");
    printf("print      Prints current clipboard text\n");
    printf("clear      Clears the clipboard\n");
    printf("owned      Prints if clipboard data is owned\n");
    printf("quit       Exits interactive mode\n");
    printf("exit       Exits interactive mode\n");
}

char *strip(char *ptr) {
    if (ptr == NULL) {
        return NULL;
    }

    while (isspace(*ptr)) {
        ptr++;
    }

    if (*ptr == '\0') {
        return NULL;
    }

    /* Guaranteed *ptr isn't the NULL terminator and isn't space */
    char *end = ptr + strlen(ptr) - 1;
    while (end > ptr && isspace(*end)) {
        end--;
    }
    *(end + 1) = '\0';

    return ptr;
}

void do_interactive(clipboard_c *cb, app_opts *opts) {
    char buf[BUFSIZ], prev[BUFSIZ] = {'p'};
    printf("Interactive mode\nEnter \"help\" to see options\n> ");
    fflush(stdout);
    int i = 0;
    bool exit = false;

    while (!exit && fgets(buf, BUFSIZ, stdin)) {
        /* Remember history depth=1 */
        if (isspace(buf[0])) {
            strcpy(buf, prev);
        } else {
            strcpy(prev, buf);
        }

        switch (tolower(buf[0])) {
            case 'h':
                interactive_help();
                break;
            case 'p': {
                char *text = clipboard_text_ex(cb, NULL, opts->mode);
                if (text) {
                    printf("$%d=%s\n", i++, text);
                    free(text);
                }
            }
            break;
            case 'c':
                clipboard_clear(cb, opts->mode);
                printf("Cleared\n");
                break;
            case 'q':
            case 'e':
                exit = true;
                break;
            case 's': {
                char *ptr = strip(strchr(buf, ' '));
                if (ptr) {
                    clipboard_set_text_ex(cb, ptr, -1, opts->mode);
                    printf("Set\n");
                }
            }
            break;
            case 'o':
                if (clipboard_has_ownership(cb, opts->mode)) {
                    printf("$%d=true\n", i++);
                } else {
                    printf("$%d=false\n", i++);
                }
                break;
            default:
                printf("Unknown option: %s\n", buf);
                break;
        }

        printf("> ");
        fflush(stdout);
    }
}

int help(const char *name, bool was_error) {
    printf("Usage: %s [-p[=mode]] [-c[=mode]] [-i[=mode]] [-s[=mode] [text]...]\n", name);
    printf("Sample clipboard application. Library version: %d.%d\n\n", LIBCLIPBOARD_VERSION_MAJOR, LIBCLIPBOARD_VERSION_MINOR);
    printf("With no options, the default is to dump the CLIPBOARD clipboard to stdout.\n");
    printf("Available modes: CLIPBOARD, SELECTION\n");
    printf("Default mode: CLIPBOARD\n\n");
    printf("  -p[=mode]            Prints the contents of the clipboard.\n");
    printf("  -c[=mode]            Clears the contents of the clipboard.\n");
    printf("  -i[=mode]            Enters interactive mode.\n");
    printf("  -s[=mode] [text]...  Sets the clipboard with [text]. If [text] is empty,\n");
    printf("                       input is read from stdin.\n\n");
    printf("Example: %s -p=SELECTION\n", name);
    printf("Example: %s -s some text to put on the clipboard\n", name);
    printf("Example: %s -c=CLIPBOARD\n", name);

    return was_error == true;
}

bool parse_mode(const char *name, const char *arg, app_opts *opts) {
    if (arg[2] == '\0') {
        return true;
    } else if (!strcmp(arg + 2, "=CLIPBOARD")) {
        opts->mode = LCB_CLIPBOARD;
    } else if (!strcmp(arg + 2, "=SELECTION")) {
        opts->mode = LCB_SELECTION;
    } else {
        printf("Error: Unknown mode: %s\n", arg);
        help(name, true);
        return false;
    }
    return true;
}

char *grow_buffer(char *dst, size_t *cur_sz, const char *src, bool add_sep) {
    size_t len = strlen(src);
    dst = realloc(dst, *cur_sz + len + 2);
    if (dst == NULL) {
        return NULL;
    }
    if (*cur_sz == 0) {
        memcpy(dst, src, len);
        *cur_sz += len;
    } else {
        if (add_sep) {
            dst[*cur_sz] = ' ';
            memcpy(dst + (*cur_sz) + 1, src, len);
            *cur_sz += len + 1;
        } else {
            memcpy(dst + (*cur_sz), src, len);
            *cur_sz += len;
        }
    }
    return dst;
}

int main(int argc, char *argv[]) {
    app_opts opts = {0};

    int i;
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'h':
                    return help(argv[0], false);
                    break;
                case 'p':
                    opts.dump_clipboard = !opts.dump_clipboard;
                    opts.opt_set = true;
                    if (!parse_mode(argv[0], argv[i], &opts)) {
                        return 1;
                    }
                    break;
                case 's':
                    opts.set_clipboard = true;
                    opts.opt_set = true;
                    if (!parse_mode(argv[0], argv[i], &opts)) {
                        return 1;
                    }
                    break;
                case 'c':
                    opts.clear_clipboard = !opts.clear_clipboard;
                    opts.opt_set = true;
                    if (!parse_mode(argv[0], argv[i], &opts)) {
                        return 1;
                    }
                    break;
                case 'i':
                    opts.interactive = !opts.interactive;
                    opts.opt_set = true;
                    if (!parse_mode(argv[0], argv[i], &opts)) {
                        return 1;
                    }
                    break;
                default:
                    printf("Error: Unknown option: %s\n\n", argv[i]);
                    return help(argv[0], true);
            }
            if (opts.set_clipboard) {
                i++;
                break;
            }
        } else {
            printf("Error: Value was unexpected at this time: %s\n\n", argv[i]);
            return help(argv[0], true);
        }
    }

    clipboard_c *cb = clipboard_new(NULL);
    if (cb == NULL) {
        printf("Clipboard initialisation failed!\n");
        return 1;
    }

    if (opts.dump_clipboard || !opts.opt_set) {
        char *text = clipboard_text_ex(cb, NULL, opts.mode);
        if (text != NULL) {
            printf("%s\n", text);
            free(text);
        }
    }

    if (opts.clear_clipboard) {
        clipboard_clear(cb, opts.mode);
    }

    if (opts.set_clipboard) {
        char *data = NULL;
        size_t sz = 0;

        for (; i < argc; i++) {
            data = grow_buffer(data, &sz, argv[i], true);
            if (data == NULL) {
                printf("realloc call failed!\n");
                return 1;
            }
        }
        if (data) {
            data[sz] = '\0';
            clipboard_set_text_ex(cb, data, sz, opts.mode);
            free(data);
        } else {
            char buf[BUFSIZ];
            while (fgets(buf, BUFSIZ, stdin)) {
                data = grow_buffer(data, &sz, buf, false);
                if (data == NULL) {
                    printf("realloc call failed!\n");
                    return 1;
                }
            }
            if (data) {
                data[sz] = '\0';
                clipboard_set_text_ex(cb, data, sz, opts.mode);
                free(data);
            }
        }
#ifdef LIBCLIPBOARD_BUILD_X11
        /* On X11, we must stay alive until the other window has copied our data */
        if (!opts.interactive) {
            printf("Press enter to exit.\n");
            fflush(stdout);
            getchar();
        }
#endif
    }

    if (opts.interactive) {
        do_interactive(cb, &opts);
    }

    clipboard_free(cb);
    return 0;
}
