#ifndef BX_DEFAULT_H
#define BX_DEFAULT_H
#include <stdbool.h>
// #include <elf.h>
#include "../../baseer.h"

#define ELF_MAGIC 0x7F454C46 // https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
#define PNG_MAGIC 0x89504e470d0a1a0a // https://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html
#define PDF_MAGIC 0x255044462D
#define BYTE 8

bool bx_binhead(baseer_target_t *target, unsigned int index, void *arg);
#endif
