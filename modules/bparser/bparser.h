#ifndef BPARSER_H
#define BPARSER_H 
#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../baseer.h"


typedef enum {
    BPARSER_FILE,
    BPARSER_MEM
} bparser_type;

typedef struct {
    const void *data;
    size_t size;
} bparser_mem_t;

typedef struct {
    union {
        FILE *fp;
        // void *mem;
        bparser_mem_t mem;
    } source;
    bparser_type type;
} bparser;


typedef bool (*bparser_callback_t)(bparser* parser, void* arg);

bparser* bparser_load(bparser_type type, void *data);
size_t bparser_read(bparser* parser, void* buf, unsigned int pos, size_t size);
bool bparser_apply(bparser* parser, bparser_callback_t callback, void* arg);

#endif

