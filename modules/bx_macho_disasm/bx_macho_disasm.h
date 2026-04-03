/**
 * @file bx_macho_disasm.h
 * @brief Mach-O disassembly module for Baseer.
 *
 * Provides disassembly of executable sections in Mach-O files
 * using the udis86 engine, with per-function symbol labeling.
 */
#ifndef BX_MACHO_DISASM_H
#define BX_MACHO_DISASM_H

#include "../bparser/bparser.h"

/**
 * @brief Disassemble executable sections of a Mach-O binary.
 *
 * Iterates over all segments/sections, finds those with executable
 * attributes (S_ATTR_PURE_INSTRUCTIONS), and disassembles them.
 * If a symbol table is present, functions are labeled by name.
 *
 * @param parser Pointer to the bparser with the file loaded in memory.
 * @param arg    Pointer to inputs struct (unused for disasm).
 * @return true on success, false on error.
 */
bool print_macho_disasm(bparser *parser, void *arg);

#endif /* BX_MACHO_DISASM_H */
