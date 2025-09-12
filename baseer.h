/**
 * @file baseer.h
 * @brief Core function
 */

/* Baseer 0.1.0a */
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

#define BASEER_VERSION_MAJOR 0  
#define BASEER_VERSION_MINOR 1   
#define BASEER_VERSION_MICRO 0  

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define BASEER_VERSION  \
    TOSTRING(BASEER_VERSION_MAJOR) "." \
    TOSTRING(BASEER_VERSION_MINOR) "." \
    TOSTRING(BASEER_VERSION_MICRO)

#define BASEER_MAX_FILE_SIZE 1024 * 1024 * 4
#define RETURN_NULL_IF(con) \
    if ((con))              \
    {                       \
        return NULL;        \
    }

#define BASEER_BASE_OFFSET(b, i, sf) (b) + ((i) * (sf))


#define COLOR_RESET      "\033[0m"
#define COLOR_GREEN      "\033[1;32m"
#define COLOR_BLUE       "\033[1;34m"
#define COLOR_YELLOW     "\033[1;33m"
#define COLOR_RED        "\033[1;31m"
#define COLOR_MAGENTA    "\033[35m"
#define COLOR_CYAN       "\033[36m"
#define COLOR_RAND(i, c1, c2) (i & 1)?c1:c2;
#define BLOCK_LENGTH 40




typedef struct baseer_target_t baseer_target_t;
typedef bool (*baseer_callback_t)(baseer_target_t *, void *arg);

typedef struct {
    int* argc;
    char**args;
} inputs;

struct baseer_target_t
{
    unsigned int size;
    void *block;
};

baseer_target_t *baseer_open(char *file_path);
void baseer_close(baseer_target_t *target);
void baseer_print(baseer_target_t *target);
bool baseer_execute(baseer_target_t *target, baseer_callback_t callback, void *arg);

#endif
