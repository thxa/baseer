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

#define BASEER_VERSION 1

#define BASEER_DEFAULT_PARTITION_COUNT 8
#define BASEER_MAX_FILE_SIZE 1024 * 1024 * 4
#define RETURN_NULL_IF(con) \
    if ((con))              \
    {                       \
        return NULL;        \
    }

#define BASEER_BASE_OFFSET(b, i, sf) (b) + ((i) * (sf))
#define BASEER_BLOCK_OFFSET(t, i) BASEER_BASE_OFFSET(((t)->block), (i), ((t)->partition_size))

typedef struct baseer_target_t baseer_target_t;
typedef struct baseer_partition_t baseer_partition_t;
typedef bool (*baseer_partition_callback_t)(baseer_target_t *, unsigned int index, void *arg);

struct baseer_partition_t
{
    unsigned int index;
};

struct baseer_target_t
{
    unsigned int version;
    unsigned int partition_size;
    unsigned int partition_count;
    unsigned int extra;
    unsigned int size;
    void *block;
    baseer_partition_t *partitions;
};

baseer_partition_t *baseer_partitionize(baseer_target_t *target);
void baseer_departitionize(baseer_partition_t *partitions);
baseer_target_t *baseer_open(char *file_path, unsigned int partition_count);
void baseer_close(baseer_target_t *target);
void baseer_print(baseer_target_t *target);
bool baseer_execute(baseer_target_t *target, baseer_partition_callback_t callback, void *arg);

#endif
