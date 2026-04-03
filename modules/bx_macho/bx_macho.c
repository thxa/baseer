/**
 * @file bx_macho.c
 * @brief Mach-O format handler for Baseer.
 *
 * Dispatches Mach-O analysis based on command-line flags:
 *   -m  Metadata (headers, load commands, segments, symbols)
 *   -a  Disassembly (executable sections with symbol labels)
 *   -d  Debugger (ptrace-based, same as ELF)
 *   -c  Decompiler (RetDec integration, same as ELF)
 */
#include "./bx_macho.h"
#include "../../baseer.h"
#include "../b_macho_metadata/b_macho_metadata.h"
#include "../bx_macho_disasm/bx_macho_disasm.h"
#include "../b_debugger/debugger.h"
#include "../bx_deElf/bx_deElf.h"
#include <string.h>
#include <stdio.h>

bool bx_macho(bparser *parser, void *arg)
{
    int argc = *((inputs *)arg)->argc;
    char **args = ((inputs *)arg)->args;

    for (int i = 2; i < argc; i++) {
        if (strcmp("-m", args[i]) == 0) {
            bparser_apply(parser, b_macho_metadata, arg);
        } else if (strcmp("-a", args[i]) == 0) {
            bparser_apply(parser, print_macho_disasm, arg);
        } else if (strcmp("-d", args[i]) == 0) {
            bparser_apply(parser, b_debugger, arg);
        } else if (strcmp("-c", args[i]) == 0) {
            bparser_apply(parser, decompile_elf, arg);
        } else if (strcmp("--args", args[i]) == 0) {
            break;
        } else {
            fprintf(stderr, "[!] Unsupported flag for Mach-O: %s\n", args[i]);
        }
    }

    return true;
}
