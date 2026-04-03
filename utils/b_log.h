/**
 * @file b_log.h
 * @brief Centralized logging macros for Baseer.
 *
 * Provides leveled logging (DEBUG, INFO, WARN, ERROR) with color-coded
 * output and optional verbose mode controlled by the BASEER_VERBOSE
 * environment variable.
 *
 * Usage:
 *   LOG_ERROR("failed to open file: %s", path);
 *   LOG_WARN("section offset out of bounds");
 *   LOG_INFO("loaded %d symbols", count);
 *   LOG_DEBUG("parsing section %d", i);
 */
#ifndef B_LOG_H
#define B_LOG_H

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Log levels
 */
typedef enum {
    BASEER_LOG_ERROR = 0,
    BASEER_LOG_WARN  = 1,
    BASEER_LOG_INFO  = 2,
    BASEER_LOG_DEBUG = 3,
} baseer_log_level_t;

/**
 * @brief Get the current log level from environment.
 *
 * Controlled by BASEER_LOG_LEVEL env var:
 *   0 = errors only
 *   1 = + warnings
 *   2 = + info (default)
 *   3 = + debug
 */
static inline int baseer_log_level(void) {
    const char *env = getenv("BASEER_LOG_LEVEL");
    if (env) {
        int val = atoi(env);
        if (val >= 0 && val <= 3) return val;
    }
    return BASEER_LOG_INFO; /* default: show errors, warnings, info */
}

#define LOG_ERROR(fmt, ...) do { \
    fprintf(stderr, "\033[31m[ERROR]\033[0m " fmt "\n", ##__VA_ARGS__); \
} while(0)

#define LOG_WARN(fmt, ...) do { \
    if (baseer_log_level() >= BASEER_LOG_WARN) \
        fprintf(stderr, "\033[33m[WARN]\033[0m  " fmt "\n", ##__VA_ARGS__); \
} while(0)

#define LOG_INFO(fmt, ...) do { \
    if (baseer_log_level() >= BASEER_LOG_INFO) \
        printf("\033[34m[INFO]\033[0m  " fmt "\n", ##__VA_ARGS__); \
} while(0)

#define LOG_DEBUG(fmt, ...) do { \
    if (baseer_log_level() >= BASEER_LOG_DEBUG) \
        printf("\033[90m[DEBUG]\033[0m " fmt "\n", ##__VA_ARGS__); \
} while(0)

#endif /* B_LOG_H */
