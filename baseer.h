#ifndef BASEER_H
#define BASEER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <dlfcn.h>


#define BASEER_VERSION 1
#define BASEER_MAX_EXTENSIONS 10
#define BASEER_MAX_FILE_SIZE 1024 * 1024 * 4
#define RETURN_NULL_IF(con) \
    if ((con))              \
    {                       \
        return NULL;        \
    }

typedef struct baseer_target_t baseer_target_t;
typedef struct baseer_extension_t baseer_extension_t;
typedef struct baseer_extension_info_t baseer_extension_info_t;

struct baseer_extension_info_t
{
    char *name; 
    char *author;
    char *description;
};

struct baseer_extension_t
{
    baseer_extension_info_t info;
    bool (*execute)(baseer_target_t *);
};

struct baseer_target_t
{
    unsigned int version;
    unsigned int size;
    unsigned int loaded_extensions;
    void *block;
    baseer_extension_t **extensions;
};


// Core functions
// baseer_target_t* baseer_open(const char *file_path);
// void baseer_close(baseer_target_t *target);
// bool baseer_load_extension(baseer_target_t *target, baseer_extension_t *ext);
// void baseer_execute_extensions(baseer_target_t *target);

// // Example extension function type
// typedef bool (*baseer_extension_func_t)(baseer_target_t *target);

#endif // BASEER_H
