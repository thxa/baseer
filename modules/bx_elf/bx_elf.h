<<<<<<< HEAD
#ifndef BX_ELF
#define BX_ELF
=======
#ifndef BX_ELF_H
#define BX_ELF_H
>>>>>>> 9c46ae5 (new changes)
#include "../bparser/bparser.h"
#include "../../baseer.h"
#include <elf.h>
#include <string.h>
<<<<<<< HEAD
#include "../b_debugger/debugger.h"
#include "../b_elf_metadata/b_elf_metadata.h"
#include "../bx_deElf/bx_deElf.h"
#include "../bx_elf_disasm/bx_elf_disasm.h"
=======
#include <stdbool.h>
#include "../b_debugger/debugger.h"
#include "../b_elf_metadata/b_elf_metadata.h"
#include "../bx_deElf/bx_deElf.h"
>>>>>>> 9c46ae5 (new changes)

bool bx_elf(bparser* parser, void *arg);

#endif 
