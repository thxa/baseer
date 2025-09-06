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
