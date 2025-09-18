/**
 * @file bparser.h
 * @brief Binary parser abstraction supporting memory and streaming files.
 *
 * Provides a unified interface to read from memory blocks or FILE* streams
 * and execute callbacks on the parsed content.
 */

#ifndef BPARSER_H
#define BPARSER_H 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../baseer.h"

/**
 * @brief Parser type: memory or streaming
 */
// typedef enum {
//     BPARSER_FILE,
//     BPARSER_MEM
// } bparser_type;

/**
 * @brief Memory source for the parser
 */
// typedef struct {
//     const void *data;
//     size_t size;
// } bparser_mem_t;

/**
 * @brief Parser object
 */
typedef struct {
    // union {
        FILE *fp;
        const void *block;
        size_t size;
        // void *mem;
        // bparser_mem_t mem;
    // } source;

    // bparser_type type;
} bparser;

/**
 * @brief Callback for bparser tools
 * 
 * @param parser Pointer to parser object
 * @param arg Additional argument (e.g., inputs)
 * @return true on success, false on failure
 */
typedef bool (*bparser_callback_t)(bparser* parser, void* arg);

/**
 * @brief Load a parser object
 * 
 * @param type Parser type: BPARSER_MEM or BPARSER_FILE
 * @param data Memory block or FILE* pointer
 * @return Pointer to bparser on success, NULL on failure
 */
bparser* bparser_load(baseer_target_t *data);

/**
 * @brief Read bytes from parser
 * 
 * @param parser Pointer to parser object
 * @param buf Output buffer
 * @param pos Offset in memory or file
 * @param size Number of bytes to read
 * @return Number of bytes successfully read
 */
size_t bparser_read(bparser* parser, void* buf, unsigned int pos, size_t size);

/**
 * @brief Execute a callback on the parser
 * 
 * @param parser Pointer to parser
 * @param callback Callback function
 * @param arg Additional argument
 * @return true on success, false on failure
 */
bool bparser_apply(bparser* parser, bparser_callback_t callback, void* arg);

#endif

