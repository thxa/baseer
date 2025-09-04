/**
 * @file bx_binhead.h
 * @brief Functions to handle and identify file types using magic numbers.
 *
 * This header defines the interface for checking a file's magic number
 * and dispatching the appropriate parser/extension based on the file type
 * (e.g., ELF, PNG, PDF).
 */


#ifndef BX_DEFAULT_H
#define BX_DEFAULT_H
#include <stdbool.h>
// #include <elf.h>
#include "../../baseer.h"
#include "../bparser/bparser.h"

#define ELF_MAGIC 0x7F454C46 // https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
#define PNG_MAGIC 0x89504e470d0a1a0a // https://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html 
#define ZIP_MAGIC 0x504B
#define PDF_MAGIC 0x25504446
#define BYTE 8

typedef struct{
    char* name;
    unsigned long long int number;
    unsigned long long int rnumber;
    bparser_callback_t parser;
} bmagic;

bool bx_binhead(baseer_target_t *target, void *arg);

#endif
