/**
 * @file baseer.h
 * @brief Core file handling and execution API for Baseer.
 *
 * Provides memory and streaming file access, as well as execution
 * of analysis tools using a unified callback interface.
 */

/* Baseer 0.2.0 */
#ifndef BASEER_H
#define BASEER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "linenoise.h"
#define BASEER_VERSION_MAJOR 0  
#define BASEER_VERSION_MINOR 2
#define BASEER_VERSION_MICRO 0  

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define BASEER_VERSION  \
    TOSTRING(BASEER_VERSION_MAJOR) "." \
    TOSTRING(BASEER_VERSION_MINOR) "." \
    TOSTRING(BASEER_VERSION_MICRO)

#define BASEER_MAX_FILE_SIZE 1024 * 1024 * 4 /**< Max file size for memory mode */
#define RETURN_NULL_IF(con) \
    if ((con))              \
    {                       \
        return NULL;        \
    }

#define BASEER_BASE_OFFSET(b, i, sf) (b) + ((i) * (sf))

#define COLOR_RESET      "\033[0m"
// Regular colors
#define COLOR_BLACK      "\033[30m"
#define COLOR_GRAY       "\033[90m"
#define COLOR_RED        "\033[31m"
#define COLOR_GREEN      "\033[32m"
#define COLOR_YELLOW     "\033[33m"
#define COLOR_BLUE       "\033[34m"
#define COLOR_MAGENTA    "\033[35m"
#define COLOR_CYAN       "\033[36m"
#define COLOR_WHITE      "\033[37m"
// Bold (bright) colors
#define COLOR_BBLACK     "\033[1;30m"
#define COLOR_BRED       "\033[1;31m"
#define COLOR_BGREEN     "\033[1;32m"
#define COLOR_BYELLOW    "\033[1;33m"
#define COLOR_BBLUE      "\033[1;34m"
#define COLOR_BMAGENTA   "\033[1;35m"
#define COLOR_BCYAN      "\033[1;36m"
#define COLOR_BWHITE     "\033[1;37m"
// Background colors
#define COLOR_BG_BLACK   "\033[40m"
#define COLOR_BG_RED     "\033[41m"
#define COLOR_BG_GREEN   "\033[42m"
#define COLOR_BG_YELLOW  "\033[43m"
#define COLOR_BG_BLUE    "\033[44m"
#define COLOR_BG_MAGENTA "\033[45m"
#define COLOR_BG_CYAN    "\033[46m"
#define COLOR_BG_WHITE   "\033[47m"

#define MAX_INPUT_ARGS 20
#define DEFAULT_BLOCK_LENGTH 16
// #define BLOCK_LENGTH 40

#define BLOCK_LENGTH (get_block_length())

static inline int get_block_length(void)
{
    char *env = getenv("BLOCK_LENGTH");
    if (env != NULL) {
        int val = atoi(env);
        if (val > 0) return val;
    }
    return DEFAULT_BLOCK_LENGTH;
}



/**
 * @brief Struct representing command-line inputs
 */
typedef struct {
    int *argc;
    char** args;
    int input_argc;
    char* input_args[MAX_INPUT_ARGS];
} inputs;

/**
 * @brief Enum representing file access modes
 */
typedef enum {
    BASEER_MODE_MEMORY,
    BASEER_MODE_STREAM,
    BASEER_MODE_BOTH
} baseer_mode_t;

/**
 * @brief Struct representing a file target in memory or streaming mode.
 */
typedef struct baseer_target_t {
    FILE* fp;           /**< File pointer */
    baseer_mode_t mode; /**< File access mode */
    unsigned int size;  /**< File size in bytes */
    void *block;        /**< Memory block or FILE* cast */
} baseer_target_t;

/**
 * @brief Callback function type for file analysis.
 * 
 * @param target Pointer to the file target
 * @param arg Additional user argument
 * @return true on success, false on failure
 */
typedef bool (*baseer_callback_t)(baseer_target_t *, void *arg);

/**
 * @brief Open a file in specified mode (memory, streaming, or both)
 * 
 * @param file_path Path to the file
 * @param mode Access mode (MEMORY, STREAM, BOTH)
 * @return Pointer to baseer_target_t on success, NULL on failure
 */
// baseer_target_t *baseer_open_memory(char *file_path);
baseer_target_t *baseer_open(char *file_path, baseer_mode_t mode);

/**
 * @brief Close a file target and free associated resources
 * 
 * @param target Pointer to the file target
 */
void baseer_close(baseer_target_t *target);

/**
 * @brief Print a hex dump of the file target
 * 
 * @param target Pointer to the file target
 */
void baseer_print(baseer_target_t *target);

/**
 * @brief Execute a callback on a file target
 * 
 * @param target Pointer to the file target
 * @param callback Callback function
 * @param arg Additional argument
 * @return true on success, false on failure
 */
bool baseer_execute(baseer_target_t *target, baseer_callback_t callback, void *arg);

#endif
