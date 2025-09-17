#ifndef BX_ELF_DISASM
#define BX_ELF_DISASM
#include "../bparser/bparser.h"
#include "../../baseer.h"
#include <elf.h>
#include <string.h>
#include "udis86.h"
#include "../bx_elf_utils/bx_elf_utils.h"

bool print_elf_disasm(bparser* parser, void* args);

#endif
