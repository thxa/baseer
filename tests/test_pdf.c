/**
 * @file test_pdf.c
 * @brief Unit tests for PDF parsing modules.
 */
#define _GNU_SOURCE
#include "test_framework.h"
#include <stdint.h>
#include <string.h>
#include "../baseer.h"
#include "../modules/bparser/bparser.h"

#define TEST_PDF "examples/test_pdf"

TEST(test_pdf_file_opens) {
    baseer_target_t *t = baseer_open(TEST_PDF, BASEER_MODE_MEMORY);
    ASSERT_NOT_NULL(t);
    ASSERT_NOT_NULL(t->block);
    ASSERT(t->size > 0);
    baseer_close(t);
}

TEST(test_pdf_magic) {
    baseer_target_t *t = baseer_open(TEST_PDF, BASEER_MODE_MEMORY);
    ASSERT_NOT_NULL(t);
    unsigned char *data = (unsigned char *)t->block;
    ASSERT(data[0] == 0x25); /* % */
    ASSERT(data[1] == 0x50); /* P */
    ASSERT(data[2] == 0x44); /* D */
    ASSERT(data[3] == 0x46); /* F */
    ASSERT(data[4] == 0x2D); /* - */
    baseer_close(t);
}

TEST(test_pdf_version) {
    baseer_target_t *t = baseer_open(TEST_PDF, BASEER_MODE_MEMORY);
    ASSERT_NOT_NULL(t);
    unsigned char *data = (unsigned char *)t->block;
    ASSERT(t->size >= 8);
    ASSERT(data[5] >= '0' && data[5] <= '9');
    baseer_close(t);
}

TEST(test_pdf_bparser_load) {
    baseer_target_t *t = baseer_open(TEST_PDF, BASEER_MODE_BOTH);
    ASSERT_NOT_NULL(t);
    bparser *bp = bparser_load(t);
    ASSERT_NOT_NULL(bp);
    ASSERT_EQ(bp->size, t->size);

    unsigned char buf[5] = {0};
    size_t n = bparser_read(bp, buf, 0, 5);
    ASSERT_EQ(n, 5);
    ASSERT(memcmp(buf, "%PDF-", 5) == 0);

    free(bp);
    baseer_close(t);
}

TEST(test_pdf_has_trailer) {
    baseer_target_t *t = baseer_open(TEST_PDF, BASEER_MODE_MEMORY);
    ASSERT_NOT_NULL(t);
    ASSERT(memmem(t->block, t->size, "trailer", 7) != NULL);
    baseer_close(t);
}

TEST(test_pdf_has_eof) {
    baseer_target_t *t = baseer_open(TEST_PDF, BASEER_MODE_MEMORY);
    ASSERT_NOT_NULL(t);
    ASSERT(memmem(t->block, t->size, "%%EOF", 5) != NULL);
    baseer_close(t);
}

int main(void) {
    printf("\n=== PDF Tests ===\n");
    RUN_TEST(test_pdf_file_opens);
    RUN_TEST(test_pdf_magic);
    RUN_TEST(test_pdf_version);
    RUN_TEST(test_pdf_bparser_load);
    RUN_TEST(test_pdf_has_trailer);
    RUN_TEST(test_pdf_has_eof);
    TEST_REPORT();
}
