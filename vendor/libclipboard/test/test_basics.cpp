/**
 *  \file test_basics.cpp
 *  \brief Basic unit tests
 *
 *  \copyright Copyright (C) 2016-2019 Jeremy Tan.
 *             This file is released under the MIT license.
 *             See LICENSE for details.
 */
#include <gtest/gtest.h>
#include <libclipboard.h>

#include "libclipboard-test-private.h"

class BasicsTest : public ::testing::Test {
};

TEST_F(BasicsTest, TestBackwardsCompat) {
    EXPECT_EQ(LCB_SELECTION, LCB_PRIMARY);
    EXPECT_NE(LCB_CLIPBOARD, LCB_PRIMARY);
    EXPECT_NE(LCB_PRIMARY, LCB_SECONDARY);
}

class WithMode : public ::testing::TestWithParam<clipboard_mode> {
protected:
    void SetUp() override {
        mMode = GetParam();
    }

    clipboard_mode mMode;
};


TEST_P(WithMode, TestInstantiation) {
    clipboard_c *ret = clipboard_new(NULL);
    ASSERT_TRUE(ret != NULL);
    clipboard_free(ret);
    // TODO(jtanx): Insert platform specific tests based on clipboard_opts
#ifdef LIBCLIPBOARD_BUILD_WIN32
    clipboard_opts opts = {0};
    ret = clipboard_new(&opts);
    ASSERT_TRUE(ret != NULL);
    clipboard_free(ret);
#endif
}

TEST_P(WithMode, TestMultipleInstantiation) {
    clipboard_c *cb1 = clipboard_new(NULL);
    clipboard_c *cb2 = clipboard_new(NULL);

    ASSERT_TRUE(cb1 != NULL);
    ASSERT_TRUE(cb2 != NULL);

    clipboard_free(cb2);
    clipboard_free(cb1);
}

TEST_P(WithMode, TestClipboardFreeWithNull) {
    /* Just make sure it doesn't segfault */
    clipboard_free(NULL);
}

TEST_P(WithMode, TestClearingClipboard) {
    clipboard_c *cb = clipboard_new(NULL);

    clipboard_set_text_ex(cb, "cleartest", -1, mMode);
    /* Line below should have no effect */
    clipboard_clear(NULL, LCB_CLIPBOARD);

    char *text = clipboard_text_ex(cb, NULL, mMode);
    ASSERT_STREQ("cleartest", text);
    free(text);

    clipboard_clear(cb, mMode);
    TRY_RUN_NE(clipboard_text_ex(cb, NULL, mMode), NULL, text);
    ASSERT_TRUE(text == NULL);

    clipboard_free(cb);
}

TEST_P(WithMode, TestOwnership) {
    clipboard_c *cb1 = clipboard_new(NULL);
    clipboard_c *cb2 = clipboard_new(NULL);
    ASSERT_FALSE(clipboard_has_ownership(cb1, mMode));
    ASSERT_FALSE(clipboard_has_ownership(cb2, mMode));
    ASSERT_FALSE(clipboard_has_ownership(NULL, mMode));

    /* This test is inherently subject to race conditions as any other
       application could obtain the clipboard between setting and assertion. */
    ASSERT_TRUE(clipboard_set_text_ex(cb1, "test", -1, mMode));
    ASSERT_TRUE(clipboard_has_ownership(cb1, mMode));

    char *ret;
    ASSERT_FALSE(clipboard_has_ownership(cb2, mMode));
    /*
       The line below is present only for synchronisation purposes.
       On X11, it may happen that cb2's set text call happens *before*
       cb1's, meaning that the ownership would still belong to cb1.
     */
    TRY_RUN_EQ(clipboard_text_ex(cb2, NULL, mMode), NULL, ret);
    ASSERT_TRUE(ret != NULL);
    free(ret);
    ASSERT_TRUE(clipboard_set_text_ex(cb2, "test2", -1, mMode));

    bool has_ownership;
    TRY_RUN_EQ(clipboard_has_ownership(cb1, mMode), true, has_ownership);
    ASSERT_FALSE(has_ownership);
    ASSERT_TRUE(clipboard_has_ownership(cb2, mMode));

    clipboard_free(cb2);
    clipboard_free(cb1);
}

TEST_P(WithMode, TestSetTextEdgeCases) {
    clipboard_c *cb1 = clipboard_new(NULL);

    ASSERT_FALSE(clipboard_set_text_ex(NULL, NULL, 0, mMode));
    ASSERT_FALSE(clipboard_set_text_ex(NULL, NULL, -1, mMode));
    ASSERT_FALSE(clipboard_set_text_ex(NULL, NULL, 10, mMode));
    ASSERT_FALSE(clipboard_set_text_ex(cb1, NULL, 0, mMode));
    ASSERT_FALSE(clipboard_set_text_ex(cb1, NULL, -1, mMode));
    ASSERT_FALSE(clipboard_set_text_ex(cb1, NULL, 10, mMode));
    ASSERT_FALSE(clipboard_set_text_ex(NULL, "test", 0, mMode));
    ASSERT_FALSE(clipboard_set_text_ex(NULL, "test", -1, mMode));
    ASSERT_FALSE(clipboard_set_text_ex(NULL, "test", 10, mMode));
    ASSERT_FALSE(clipboard_set_text_ex(cb1, "test", 0, mMode));

    clipboard_free(cb1);
}

TEST_P(WithMode, TestSetText) {
    clipboard_c *cb1 = clipboard_new(NULL);
    clipboard_c *cb2 = clipboard_new(NULL);
    char *ret1, *ret2;

    ASSERT_TRUE(clipboard_set_text_ex(cb1, "test", -1, mMode));

    ret1 = clipboard_text_ex(cb1, NULL, mMode);
    TRY_RUN_STRNE(clipboard_text_ex(cb2, NULL, mMode), "test", ret2);
    ASSERT_STREQ("test", ret1);
    ASSERT_STREQ("test", ret2);
    free(ret1);
    free(ret2);

    ASSERT_TRUE(clipboard_set_text_ex(cb2, "string", -1, mMode));
    TRY_RUN_STRNE(clipboard_text_ex(cb1, NULL, mMode), "string", ret1);
    ret2 = clipboard_text_ex(cb2, NULL, mMode);
    ASSERT_STREQ("string", ret1);
    ASSERT_STREQ("string", ret2);
    free(ret1);
    free(ret2);

    ASSERT_TRUE(clipboard_set_text_ex(cb1, "test", 1, mMode));
    ret1 = clipboard_text_ex(cb1, NULL, mMode);
    TRY_RUN_STRNE(clipboard_text_ex(cb2, NULL, mMode), "t", ret2);
    ASSERT_STREQ("t", ret1);
    ASSERT_STREQ("t", ret2);
    free(ret1);
    free(ret2);

    clipboard_free(cb2);
    clipboard_free(cb1);
}

TEST_P(WithMode, TestGetText) {
    clipboard_c *cb1 = clipboard_new(NULL), *cb2 = clipboard_new(NULL);
    char *ret;
    int length;

    ASSERT_TRUE(clipboard_text_ex(NULL, NULL, mMode) == NULL);
    ASSERT_TRUE(clipboard_text_ex(NULL, &length, mMode) == NULL);

    clipboard_set_text_ex(cb1, "test", -1, mMode);
    ret = clipboard_text_ex(cb1, NULL, mMode);
    ASSERT_STREQ("test", ret);
    free(ret);

    ret = clipboard_text_ex(cb1, &length, mMode);
    ASSERT_STREQ("test", ret);
    ASSERT_EQ(static_cast<int>(strlen("test")), length);
    free(ret);

    TRY_RUN_STRNE(clipboard_text_ex(cb2, &length, mMode), "test", ret);
    ASSERT_STREQ("test", ret);
    ASSERT_EQ(static_cast<int>(strlen("test")), length);
    free(ret);

    clipboard_set_text_ex(cb1, "test", 2, mMode);
    ret = clipboard_text_ex(cb1, &length, mMode);
    ASSERT_STREQ("te", ret);
    ASSERT_EQ(static_cast<int>(strlen("te")), length);
    free(ret);

    TRY_RUN_STRNE(clipboard_text_ex(cb2, &length, mMode), "te", ret);
    ASSERT_STREQ("te", ret);
    ASSERT_EQ(static_cast<int>(strlen("te")), length);
    free(ret);

    clipboard_free(cb1);
    clipboard_free(cb2);
}

TEST_P(WithMode, TestUTF8InputOutput) {
    clipboard_c *cb1 = clipboard_new(NULL), *cb2 = clipboard_new(NULL);
    char *ret;

    ASSERT_TRUE(clipboard_set_text_ex(cb1, "\xe6\x9c\xaa\xe6\x9d\xa5", -1, mMode));
    ret = clipboard_text_ex(cb1, NULL, mMode);
    ASSERT_STREQ("\xe6\x9c\xaa\xe6\x9d\xa5", ret);
    free(ret);

    TRY_RUN_STRNE(clipboard_text_ex(cb2, NULL, mMode), "\xe6\x9c\xaa\xe6\x9d\xa5", ret);
    ASSERT_STREQ("\xe6\x9c\xaa\xe6\x9d\xa5", ret);
    free(ret);

    clipboard_free(cb1);
    clipboard_free(cb2);
}

TEST_P(WithMode, TestNewlines) {
    clipboard_c *cb1 = clipboard_new(NULL), *cb2 = clipboard_new(NULL);
    char *ret;

    ASSERT_TRUE(clipboard_set_text_ex(cb1, "a\r\n b\r\n c\r\n", -1, mMode));
    TRY_RUN_STRNE(clipboard_text_ex(cb2, NULL, mMode), "a\r\n b\r\n c\r\n", ret);
    ASSERT_STREQ("a\r\n b\r\n c\r\n", ret);
    free(ret);

    ASSERT_TRUE(clipboard_set_text_ex(cb1, "a\n b\n c\n", -1, mMode));
    TRY_RUN_STRNE(clipboard_text_ex(cb2, NULL, mMode), "a\n b\n c\n", ret);
    ASSERT_STREQ("a\n b\n c\n", ret);
    free(ret);

    ASSERT_TRUE(clipboard_set_text_ex(cb1, "a\r b\r c\r", -1, mMode));
    TRY_RUN_STRNE(clipboard_text_ex(cb2, NULL, mMode), "a\r b\r c\r", ret);
    ASSERT_STREQ("a\r b\r c\r", ret);
    free(ret);

    clipboard_free(cb1);
    clipboard_free(cb2);
}

INSTANTIATE_TEST_CASE_P(ClipboardBasicsTest,
                        WithMode,
                        ::testing::Range(LCB_CLIPBOARD, LCB_MODE_END, 1));
