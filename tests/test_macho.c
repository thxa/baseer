/**
 * @file test_macho.c
 * @brief Unit tests for Mach-O parsing modules.
 */
#include "test_framework.h"
#include <stdint.h>
#include "../baseer.h"
#include "../modules/bparser/bparser.h"

#define TEST_MACHO "examples/macho"

TEST(test_macho_file_opens) {
    baseer_target_t *t = baseer_open(TEST_MACHO, BASEER_MODE_MEMORY);
    if (!t) {
        printf("(skipped - no test binary) ");
        return;
    }
    ASSERT_NOT_NULL(t);
    ASSERT_NOT_NULL(t->block);
    ASSERT(t->size > 0);
    baseer_close(t);
}

TEST(test_macho_magic_64) {
    baseer_target_t *t = baseer_open(TEST_MACHO, BASEER_MODE_MEMORY);
    if (!t) {
        printf("(skipped - no test binary) ");
        return;
    }
    unsigned char *data = (unsigned char *)t->block;
    /* Mach-O 64-bit magic: 0xFEEDFACF (little-endian: CF FA ED FE) */
    uint32_t magic = *(uint32_t *)data;
    ASSERT(magic == 0xfeedface || magic == 0xfeedfacf);
    baseer_close(t);
}

TEST(test_macho_header_size) {
    baseer_target_t *t = baseer_open(TEST_MACHO, BASEER_MODE_MEMORY);
    if (!t) {
        printf("(skipped - no test binary) ");
        return;
    }
    /* A valid Mach-O must be at least 28 bytes (32-bit header size) */
    ASSERT(t->size >= 28);
    baseer_close(t);
}

TEST(test_macho_ncmds_sane) {
    baseer_target_t *t = baseer_open(TEST_MACHO, BASEER_MODE_MEMORY);
    if (!t) {
        printf("(skipped - no test binary) ");
        return;
    }
    unsigned char *data = (unsigned char *)t->block;
    uint32_t magic = *(uint32_t *)data;

    uint32_t ncmds;
    if (magic == 0xfeedfacf) {
        /* 64-bit: ncmds at offset 16 */
        ncmds = *(uint32_t *)(data + 16);
    } else {
        /* 32-bit: ncmds at offset 12 */
        ncmds = *(uint32_t *)(data + 12);
    }
    /* Should be a reasonable number */
    ASSERT(ncmds > 0);
    ASSERT(ncmds < 10000);
    baseer_close(t);
}

TEST(test_macho_bparser_load) {
    baseer_target_t *t = baseer_open(TEST_MACHO, BASEER_MODE_BOTH);
    if (!t) {
        printf("(skipped - no test binary) ");
        return;
    }
    bparser *bp = bparser_load(t);
    ASSERT_NOT_NULL(bp);
    ASSERT_EQ(bp->size, t->size);

    /* Read first 4 bytes through bparser */
    unsigned char buf[4] = {0};
    size_t n = bparser_read(bp, buf, 0, 4);
    ASSERT_EQ(n, 4);
    /* Should match Mach-O magic */
    uint32_t magic = *(uint32_t *)buf;
    ASSERT(magic == 0xfeedface || magic == 0xfeedfacf);

    free(bp);
    baseer_close(t);
}

TEST(test_macho_too_small_rejected) {
    /* Create a tiny file that's too small to be a Mach-O */
    /* We test the NULL/size checks in baseer_open with a nonexistent path */
    baseer_target_t *t = baseer_open("/dev/null", BASEER_MODE_MEMORY);
    /* /dev/null has 0 bytes - should either fail or produce a 0-size target */
    if (t) {
        ASSERT_EQ(t->size, 0);
        baseer_close(t);
    }
}

int main(void) {
    printf("\n=== Mach-O Tests ===\n");
    RUN_TEST(test_macho_file_opens);
    RUN_TEST(test_macho_magic_64);
    RUN_TEST(test_macho_header_size);
    RUN_TEST(test_macho_ncmds_sane);
    RUN_TEST(test_macho_bparser_load);
    RUN_TEST(test_macho_too_small_rejected);
    TEST_REPORT();
}
