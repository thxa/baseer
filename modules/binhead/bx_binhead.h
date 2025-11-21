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
#include "../../baseer.h"
#include "../bparser/bparser.h"

#define ELF_MAGIC 0x7F454C46 // https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
#define PNG_MAGIC 0x89504e470d0a1a0a // https://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html 
#define ZIP_MAGIC 0x504B
#define PDF_MAGIC 0x255044462D


// Mac OS ABI
#define FAT_MAGIC   0xCAFEBABE

/* Constant for the magic field of the mach_header (32-bit architectures) */
#define MH_MAGIC 0xfeedface /* the mach magic number */
#define MH_CIGAM 0xcefaedfe /* NXSwapInt(MH_MAGIC) */

/* Constant for the magic field of the mach_header_64 (64-bit architectures) */
#define MH_MAGIC_64 0xfeedfacf /* the 64-bit mach magic number */
#define MH_CIGAM_64 0xcffaedfe /* NXSwapInt(MH_MAGIC_64) */



#define TAR_MAGIC 0x7573746172 
                        // 00 30 30
                        // 20 20 00
#define BYTE 8

typedef struct{
    char* name;
    unsigned long long int number;
    unsigned long long int rnumber;
    bparser_callback_t parser;
    unsigned int pos;
} bmagic;

bool bx_binhead(baseer_target_t *target, void *arg);

#endif
