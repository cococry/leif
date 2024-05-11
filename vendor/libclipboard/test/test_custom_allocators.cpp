/**
 *  \file test_custom_allocators.cpp
 *  \brief Custom allocators test
 *
 *  \copyright Copyright (C) 2016 Jeremy Tan.
 *             This file is released under the MIT license.
 *             See LICENSE for details.
 */

/*
 *  I thought about using gmock and came to the conclusion that it's easier
 *  to make my own mocks. Tests should *not* be run in parallel.
 */
#include <gtest/gtest.h>
#include <libclipboard.h>
#include <atomic>
#include <assert.h>

#include "libclipboard-test-private.h"

struct {
    std::atomic<int> alloc_fail_count;
    std::atomic<int> malloc_count;
    std::atomic<int> calloc_count;
    std::atomic<int> realloc_count;
    std::atomic<int> free_count;
} g_alloc_counts;

struct {
    std::atomic<clipboard_malloc_fn> malloc_ptr;
    std::atomic<clipboard_calloc_fn> calloc_ptr;
    std::atomic<clipboard_realloc_fn> realloc_ptr;
    std::atomic<clipboard_free_fn> free_ptr;
} g_alloc_ptrs;

static void *mock_malloc(size_t size) {
    g_alloc_counts.malloc_count++;
    if (g_alloc_ptrs.malloc_ptr.load()) {
        void *ret;
        ret = g_alloc_ptrs.malloc_ptr.load()(size);
        assert(ret != NULL);
        return ret;
    }
    g_alloc_counts.alloc_fail_count++;
    return NULL;
}

static void *mock_calloc(size_t nmemb, size_t size) {
    g_alloc_counts.calloc_count++;
    if (g_alloc_ptrs.calloc_ptr.load()) {
        void *ret;
        ret = g_alloc_ptrs.calloc_ptr.load()(nmemb, size);
        assert(ret != NULL);
        return ret;
    }
    g_alloc_counts.alloc_fail_count++;
    return NULL;
}

static void *mock_realloc(void *ptr, size_t size) {
    g_alloc_counts.realloc_count++;
    if (ptr == NULL) {
        g_alloc_counts.malloc_count++;
    }
    if (g_alloc_ptrs.realloc_ptr.load()) {
        void *ret;
        ret = g_alloc_ptrs.realloc_ptr.load()(ptr, size);
        assert(ret != NULL);
        return ret;
    }
    g_alloc_counts.alloc_fail_count++;
    return NULL;
}

static void mock_free(void *ptr) {
    g_alloc_counts.free_count++;
    if (g_alloc_ptrs.free_ptr.load()) {
        g_alloc_ptrs.free_ptr.load()(ptr);
    }
}

class CustomAllocatorsTest : public ::testing::Test {
protected:
    CustomAllocatorsTest() : std_mock{} {
        std_mock.user_malloc_fn = mock_malloc;
        std_mock.user_calloc_fn = mock_calloc;
        std_mock.user_realloc_fn = mock_realloc;
        std_mock.user_free_fn = mock_free;

        g_alloc_counts.alloc_fail_count = 0;
        g_alloc_counts.malloc_count = 0;
        g_alloc_counts.calloc_count = 0;
        g_alloc_counts.realloc_count = 0;
        g_alloc_counts.free_count = 0;

        g_alloc_ptrs.malloc_ptr = malloc;
        g_alloc_ptrs.calloc_ptr = calloc;
        g_alloc_ptrs.realloc_ptr = realloc;
        g_alloc_ptrs.free_ptr = free;
    }

    clipboard_opts std_mock;
};

TEST_F(CustomAllocatorsTest, TestAllocatorsFail) {
    g_alloc_ptrs.malloc_ptr = nullptr;
    g_alloc_ptrs.calloc_ptr = nullptr;
    g_alloc_ptrs.realloc_ptr = nullptr;
    g_alloc_ptrs.free_ptr = nullptr;

    clipboard_c *cb = clipboard_new(&std_mock);
    ASSERT_TRUE(cb == NULL);

    int alloc_counts = g_alloc_counts.malloc_count + g_alloc_counts.calloc_count;
    // Expect at least one alloc call
    ASSERT_GT(alloc_counts, 0);
    // No. of frees should at least match no. of successful allocs.
    ASSERT_GE(g_alloc_counts.free_count, alloc_counts - g_alloc_counts.alloc_fail_count);
}

TEST_F(CustomAllocatorsTest, TestAllocationsMatchesFrees) {
    clipboard_c *cb = clipboard_new(&std_mock);
    ASSERT_TRUE(cb != NULL);

    ASSERT_TRUE(clipboard_set_text(cb, "allocTest"));
    mock_free(clipboard_text(cb));

    clipboard_free(cb);

    int alloc_counts = g_alloc_counts.malloc_count + g_alloc_counts.calloc_count;
    // Expect at least one alloc call
    ASSERT_GT(alloc_counts, 0);
    // No. of frees should at least match no. of successful allocs.
    ASSERT_GE(g_alloc_counts.free_count, alloc_counts - g_alloc_counts.alloc_fail_count);
}

TEST_F(CustomAllocatorsTest, TestAllocationsMatchesFreesEx) {
    clipboard_c *cb1 = clipboard_new(NULL);
    clipboard_c *cb2 = clipboard_new(&std_mock);
    char *text;
    ASSERT_TRUE(cb1 != NULL);
    ASSERT_TRUE(cb2 != NULL);

    ASSERT_TRUE(clipboard_set_text(cb1, "allocTest"));
    TRY_RUN_STRNE(clipboard_text(cb2), "allocTest", text);
    ASSERT_STREQ("allocTest", text);
    mock_free(text);

    clipboard_free(cb1);
    clipboard_free(cb2);

    int alloc_counts = g_alloc_counts.malloc_count + g_alloc_counts.calloc_count;
    // Expect at least one alloc call
    ASSERT_GT(alloc_counts, 0);
    // No. of frees should at least match no. of successful allocs.
    ASSERT_GE(g_alloc_counts.free_count, alloc_counts - g_alloc_counts.alloc_fail_count);
}

TEST_F(CustomAllocatorsTest, TestAllocationFailOnGetText) {
    clipboard_c *cb = clipboard_new(&std_mock);
    ASSERT_TRUE(cb != NULL);

    ASSERT_TRUE(clipboard_set_text(cb, "allocTest2"));
    g_alloc_ptrs.malloc_ptr = nullptr;
    g_alloc_ptrs.calloc_ptr = nullptr;
    g_alloc_ptrs.realloc_ptr = nullptr;
    ASSERT_TRUE(clipboard_text(cb) == NULL);

    clipboard_free(cb);
    int alloc_counts = g_alloc_counts.malloc_count + g_alloc_counts.calloc_count;
    // Expect at least one alloc call
    ASSERT_GT(alloc_counts, 0);
    // No. of frees should at least match no. of successful allocs.
    ASSERT_GE(g_alloc_counts.free_count, alloc_counts - g_alloc_counts.alloc_fail_count);
}
