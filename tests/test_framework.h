/**
 * @file test_framework.h
 * @brief Minimal single-header unit test framework for Baseer.
 *
 * Usage:
 *   #include "test_framework.h"
 *   TEST(test_name) { ASSERT(1 == 1); }
 *   int main(void) { RUN_TEST(test_name); TEST_REPORT(); }
 */
#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int _tf_pass = 0;
static int _tf_fail = 0;
static int _tf_total = 0;

#define TEST(name) static void name(void)

#define RUN_TEST(name) do { \
    _tf_total++; \
    printf("  [RUN ] %s ... ", #name); \
    fflush(stdout); \
    name(); \
    _tf_pass++; \
    printf("\033[32mPASS\033[0m\n"); \
} while(0)

/* If an assertion fails, longjmp is overkill; just print and exit the test */
#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("\033[31mFAIL\033[0m\n"); \
        fprintf(stderr, "    Assertion failed: %s\n    at %s:%d\n", #cond, __FILE__, __LINE__); \
        _tf_fail++; \
        _tf_pass--; /* undo the pre-increment in RUN_TEST */ \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NEQ(a, b) ASSERT((a) != (b))
#define ASSERT_NULL(p) ASSERT((p) == NULL)
#define ASSERT_NOT_NULL(p) ASSERT((p) != NULL)
#define ASSERT_STR_EQ(a, b) ASSERT(strcmp((a), (b)) == 0)

#define TEST_REPORT() do { \
    printf("\n  ================================\n"); \
    printf("  Results: %d/%d passed", _tf_pass, _tf_total); \
    if (_tf_fail > 0) printf(", \033[31m%d failed\033[0m", _tf_fail); \
    printf("\n  ================================\n"); \
    return (_tf_fail > 0) ? 1 : 0; \
} while(0)

#endif /* TEST_FRAMEWORK_H */
