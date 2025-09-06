#ifndef BX_ELF
#define BX_ELF
#include "../bparser/bparser.h"
#include "../../baseer.h"
#include <elf.h>
#include <string.h>
#include "../b_debugger/debugger.h"
#include "../b_elf_metadata/b_elf_metadata.h"

bool bx_elf(bparser* parser, void *arg);

#endif 
