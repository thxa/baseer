/**
 * @file test_baseer_core.c
 * @brief Unit tests for baseer core (open, close, execute).
 */
#include "test_framework.h"
#include "../baseer.h"

/* We need a real file for testing. Use the test binary from examples/ */
#define TEST_ELF_64 "examples/64bit_x86_64_gcc"
#define TEST_ELF_32 "examples/32bit_x86_gcc"

TEST(test_open_null_path) {
    baseer_target_t *t = baseer_open(NULL, BASEER_MODE_MEMORY);
    ASSERT_NULL(t);
}

TEST(test_open_nonexistent_file) {
    baseer_target_t *t = baseer_open("/nonexistent/path/file", BASEER_MODE_MEMORY);
    ASSERT_NULL(t);
}

TEST(test_open_memory_mode) {
    baseer_target_t *t = baseer_open(TEST_ELF_64, BASEER_MODE_MEMORY);
    if (!t) {
        /* Skip if test binary not available */
        printf("(skipped - no test binary) ");
        return;
    }
    ASSERT_NOT_NULL(t);
    ASSERT_NOT_NULL(t->block);
    ASSERT_NULL(t->fp); /* memory mode should not keep fp open */
    ASSERT(t->size > 0);
    ASSERT_EQ(t->mode, BASEER_MODE_MEMORY);
    baseer_close(t);
}

TEST(test_open_stream_mode) {
    baseer_target_t *t = baseer_open(TEST_ELF_64, BASEER_MODE_STREAM);
    if (!t) {
        printf("(skipped - no test binary) ");
        return;
    }
    ASSERT_NOT_NULL(t);
    ASSERT_NOT_NULL(t->fp);
    ASSERT(t->size > 0);
    ASSERT_EQ(t->mode, BASEER_MODE_STREAM);
    baseer_close(t);
}

TEST(test_open_both_mode) {
    baseer_target_t *t = baseer_open(TEST_ELF_64, BASEER_MODE_BOTH);
    if (!t) {
        printf("(skipped - no test binary) ");
        return;
    }
    ASSERT_NOT_NULL(t);
    ASSERT_NOT_NULL(t->block);
    ASSERT_NOT_NULL(t->fp);
    ASSERT(t->size > 0);
    ASSERT_EQ(t->mode, BASEER_MODE_BOTH);
    baseer_close(t);
}

TEST(test_close_null) {
    /* Should not crash */
    baseer_close(NULL);
}

static bool dummy_callback(baseer_target_t *target, void *arg) {
    *(int*)arg = 1;
    return true;
}

static bool failing_callback(baseer_target_t *target, void *arg) {
    return false;
}

TEST(test_execute_null_target) {
    int val = 0;
    bool result = baseer_execute(NULL, dummy_callback, &val);
    ASSERT_EQ(result, false);
    ASSERT_EQ(val, 0); /* callback should not have been called */
}

TEST(test_execute_null_callback) {
    baseer_target_t *t = baseer_open(TEST_ELF_64, BASEER_MODE_MEMORY);
    if (!t) {
        printf("(skipped - no test binary) ");
        return;
    }
    bool result = baseer_execute(t, NULL, NULL);
    ASSERT_EQ(result, false);
    baseer_close(t);
}

TEST(test_execute_success) {
    baseer_target_t *t = baseer_open(TEST_ELF_64, BASEER_MODE_MEMORY);
    if (!t) {
        printf("(skipped - no test binary) ");
        return;
    }
    int val = 0;
    bool result = baseer_execute(t, dummy_callback, &val);
    ASSERT_EQ(result, true);
    ASSERT_EQ(val, 1);
    baseer_close(t);
}

TEST(test_execute_failure) {
    baseer_target_t *t = baseer_open(TEST_ELF_64, BASEER_MODE_MEMORY);
    if (!t) {
        printf("(skipped - no test binary) ");
        return;
    }
    bool result = baseer_execute(t, failing_callback, NULL);
    ASSERT_EQ(result, false);
    baseer_close(t);
}

TEST(test_elf_magic_bytes) {
    baseer_target_t *t = baseer_open(TEST_ELF_64, BASEER_MODE_MEMORY);
    if (!t) {
        printf("(skipped - no test binary) ");
        return;
    }
    unsigned char *data = (unsigned char*)t->block;
    /* ELF magic: 0x7f 'E' 'L' 'F' */
    ASSERT_EQ(data[0], 0x7f);
    ASSERT_EQ(data[1], 'E');
    ASSERT_EQ(data[2], 'L');
    ASSERT_EQ(data[3], 'F');
    baseer_close(t);
}

int main(void) {
    printf("\n=== Baseer Core Tests ===\n");
    RUN_TEST(test_open_null_path);
    RUN_TEST(test_open_nonexistent_file);
    RUN_TEST(test_open_memory_mode);
    RUN_TEST(test_open_stream_mode);
    RUN_TEST(test_open_both_mode);
    RUN_TEST(test_close_null);
    RUN_TEST(test_execute_null_target);
    RUN_TEST(test_execute_null_callback);
    RUN_TEST(test_execute_success);
    RUN_TEST(test_execute_failure);
    RUN_TEST(test_elf_magic_bytes);
    TEST_REPORT();
}
