/**
 * @file test_bparser.c
 * @brief Unit tests for the bparser module.
 */
#include "test_framework.h"
#include "../modules/bparser/bparser.h"

#define TEST_ELF_64 "examples/64bit_x86_64_gcc"

TEST(test_bparser_load_null) {
    bparser *bp = bparser_load(NULL);
    ASSERT_NULL(bp);
}

TEST(test_bparser_load_valid) {
    baseer_target_t *t = baseer_open(TEST_ELF_64, BASEER_MODE_BOTH);
    if (!t) {
        printf("(skipped - no test binary) ");
        return;
    }
    bparser *bp = bparser_load(t);
    ASSERT_NOT_NULL(bp);
    ASSERT_EQ(bp->mode, BASEER_MODE_BOTH);
    ASSERT_NOT_NULL(bp->block);
    ASSERT_NOT_NULL(bp->fp);
    ASSERT(bp->size > 0);
    ASSERT_EQ(bp->size, t->size);
    free(bp);
    baseer_close(t);
}

TEST(test_bparser_read_memory) {
    baseer_target_t *t = baseer_open(TEST_ELF_64, BASEER_MODE_MEMORY);
    if (!t) {
        printf("(skipped - no test binary) ");
        return;
    }
    bparser *bp = bparser_load(t);
    ASSERT_NOT_NULL(bp);

    unsigned char buf[4] = {0};
    size_t n = bparser_read(bp, buf, 0, 4);
    ASSERT_EQ(n, 4);
    /* ELF magic */
    ASSERT_EQ(buf[0], 0x7f);
    ASSERT_EQ(buf[1], 'E');
    ASSERT_EQ(buf[2], 'L');
    ASSERT_EQ(buf[3], 'F');

    free(bp);
    baseer_close(t);
}

TEST(test_bparser_read_null_params) {
    size_t n = bparser_read(NULL, NULL, 0, 4);
    ASSERT_EQ(n, 0);
}

TEST(test_bparser_read_beyond_bounds) {
    baseer_target_t *t = baseer_open(TEST_ELF_64, BASEER_MODE_MEMORY);
    if (!t) {
        printf("(skipped - no test binary) ");
        return;
    }
    bparser *bp = bparser_load(t);

    unsigned char buf[16] = {0};
    /* Read from beyond the file size */
    size_t n = bparser_read(bp, buf, bp->size + 100, 16);
    /* Should return the file size (bounds check) */
    ASSERT_EQ(n, bp->size);

    free(bp);
    baseer_close(t);
}

static bool test_callback_ran = false;
static bool test_cb(bparser *p, void *arg) {
    test_callback_ran = true;
    return true;
}

TEST(test_bparser_apply) {
    baseer_target_t *t = baseer_open(TEST_ELF_64, BASEER_MODE_MEMORY);
    if (!t) {
        printf("(skipped - no test binary) ");
        return;
    }
    bparser *bp = bparser_load(t);
    test_callback_ran = false;
    bool result = bparser_apply(bp, test_cb, NULL);
    ASSERT_EQ(result, 1);
    ASSERT_EQ(test_callback_ran, true);
    free(bp);
    baseer_close(t);
}

TEST(test_bparser_apply_null) {
    bool result = bparser_apply(NULL, test_cb, NULL);
    ASSERT_EQ(result, 0);

    baseer_target_t *t = baseer_open(TEST_ELF_64, BASEER_MODE_MEMORY);
    if (!t) {
        printf("(skipped - no test binary) ");
        return;
    }
    bparser *bp = bparser_load(t);
    result = bparser_apply(bp, NULL, NULL);
    ASSERT_EQ(result, 0);
    free(bp);
    baseer_close(t);
}

int main(void) {
    printf("\n=== BParser Tests ===\n");
    RUN_TEST(test_bparser_load_null);
    RUN_TEST(test_bparser_load_valid);
    RUN_TEST(test_bparser_read_memory);
    RUN_TEST(test_bparser_read_null_params);
    RUN_TEST(test_bparser_read_beyond_bounds);
    RUN_TEST(test_bparser_apply);
    RUN_TEST(test_bparser_apply_null);
    TEST_REPORT();
}
