#ifndef BX_ELF_H
#define BX_ELF_H
#include "../bparser/bparser.h"
#include "../../baseer.h"
#include <elf.h>
#include <string.h>
#include "../b_debugger/debugger.h"
#include "../b_elf_metadata/b_elf_metadata.h"
#include "../bx_deElf/bx_deElf.h"
#include "../bx_elf_disasm/bx_elf_disasm.h"

bool bx_elf(bparser* parser, void *arg);

#endif 
