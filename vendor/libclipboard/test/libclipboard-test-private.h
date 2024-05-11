/**
 *  \file libclipboard-test-private.h
 *  \brief Convenience macros to make testing easier
 *
 *  \copyright Copyright (C) 2016 Jeremy Tan.
 *             This file is released under the MIT license.
 *             See LICENSE for details.
 */

#ifndef _LIBCLIPBOARD_TEST_PRIVATE_H
#define _LIBCLIPBOARD_TEST_PRIVATE_H

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

/*
 * The asynchronous nature of X11 means that if we call the
 * clipboard functions too quickly, they may not yet be set correctly.
 * This section defines a macro that attempts a limited number of retries
 * to get the required value.
 */
#ifdef LIBCLIPBOARD_BUILD_X11
#  include <thread>
#  include <chrono>
#  include <iostream>

#  define TRY_RUN(fn, ev, ret, oper) do { \
    ret = fn; \
    for (int i = 0; i < 5 && oper(ret, ev); i++) { \
        std::cout << "Warning(at line:" TO_STRING(__LINE__) "): " TO_STRING(fn) " returned '" << \
            ret << "' trying again!" << std::endl; \
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); \
        ret = fn; \
    } \
} while (0)
#else
#  define TRY_RUN(fn, ev, ret, oper) ret = fn
#endif

#define TRY_RUN_EQ_OPER(actual, expected) (actual == expected)
#define TRY_RUN_NE_OPER(actual, expected) (actual != expected)
#define TRY_RUN_STREQ_OPER(actual, expected) ((actual == NULL) || !strcmp(expected, actual))
#define TRY_RUN_STRNE_OPER(actual, expected) ((actual == NULL) || strcmp(expected, actual))

/** Retry running fn, storing the result in ret, while ret == ev **/
#define TRY_RUN_EQ(fn, ev, ret) TRY_RUN(fn, ev, ret, TRY_RUN_EQ_OPER)
/** Retry running fn, storing the result in ret, while ret != ev **/
#define TRY_RUN_NE(fn, ev, ret) TRY_RUN(fn, ev, ret, TRY_RUN_NE_OPER)
/** Retry running fn, storing the result in ret, while ret == ev (string compare) **/
#define TRY_RUN_STREQ(fn, ev, ret) TRY_RUN(fn, ev, ret, TRY_RUN_STREQ_OPER)
/** Retry running fn, storing the result in ret, while ret != ev (string compare) **/
#define TRY_RUN_STRNE(fn, ev, ret) TRY_RUN(fn, ev, ret, TRY_RUN_STRNE_OPER)

#endif /* _LIBCLIPBOARD_TEST_PRIVATE_H */
