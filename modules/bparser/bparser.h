#ifndef BPARSER_H
#define BPARSER_H 
#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../baseer.h"

typedef bool (*bparser_callback_t)(const void* data, size_t len);

typedef enum {
    BPARSER_FILE,
    BPARSER_MEM
} bparser_type;

typedef struct {
    const void *data;
    size_t pos;
    size_t size;
} bparser_mem_t;

typedef struct {
    bparser_type type;
    union {
        FILE *fp;
        // void *mem;
        bparser_mem_t mem;
    } source;
} bparser;


bparser* bparser_load(bparser_type type, void *data);
size_t bparser_read(bparser* parser, void* buf, size_t size);
int bparser_apply(bparser* parser, bparser_callback_t callback, void* arg);

#endif

