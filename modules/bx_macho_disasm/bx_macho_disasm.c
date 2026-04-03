/**
 * @file bx_macho_disasm.c
 * @brief Mach-O disassembly module using udis86.
 *
 * Disassembles executable Mach-O sections with:
 * - Per-function labels from the symbol table
 * - Color-coded instruction output
 * - Support for both 32-bit and 64-bit Mach-O files
 */
#include "bx_macho_disasm.h"
#include "../../baseer.h"
#include "../bparser/bparser.h"
#include "../bx_macho_utils/bx_macho_utils.h"
#include "../bx_elf_utils/bx_elf_utils.h"
#include "../../utils/ui.h"
#include "macho.h"
#include "udis86.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* =================== Internal helpers =================== */

/**
 * @brief Symbol entry for sorting and lookup during disassembly.
 */
typedef struct {
    uint64_t addr;
    const char *name;
} macho_sym_entry;

static int sym_compare(const void *a, const void *b)
{
    const macho_sym_entry *sa = (const macho_sym_entry *)a;
    const macho_sym_entry *sb = (const macho_sym_entry *)b;
    if (sa->addr < sb->addr) return -1;
    if (sa->addr > sb->addr) return 1;
    return 0;
}

/**
 * @brief Look up a symbol name by address using binary search.
 */
static const char* find_symbol(const macho_sym_entry *syms, uint32_t count, uint64_t addr)
{
    if (!syms || count == 0) return NULL;

    int lo = 0, hi = (int)count - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (syms[mid].addr == addr) return syms[mid].name;
        if (syms[mid].addr < addr) lo = mid + 1;
        else hi = mid - 1;
    }
    return NULL;
}

/**
 * @brief Collect function symbols from a 64-bit Mach-O symbol table.
 */
static macho_sym_entry* collect_symbols_64(const unsigned char *block, size_t file_size,
                                            uint32_t symoff, uint32_t nsyms,
                                            uint32_t stroff, uint32_t strsize,
                                            uint32_t *out_count)
{
    *out_count = 0;
    if (symoff + (size_t)nsyms * sizeof(struct nlist_64) > file_size) return NULL;
    if (stroff + strsize > file_size) return NULL;

    const struct nlist_64 *nl = (const struct nlist_64 *)(block + symoff);
    const char *strs = (const char *)(block + stroff);

    /* Count function symbols (defined in a section, not stab) */
    uint32_t count = 0;
    for (uint32_t i = 0; i < nsyms; i++) {
        if ((nl[i].n_type & N_STAB) == 0 && (nl[i].n_type & N_TYPE) == N_SECT && nl[i].n_value != 0)
            count++;
    }
    if (count == 0) return NULL;

    macho_sym_entry *syms = malloc(count * sizeof(macho_sym_entry));
    if (!syms) return NULL;

    uint32_t idx = 0;
    for (uint32_t i = 0; i < nsyms; i++) {
        if ((nl[i].n_type & N_STAB) == 0 && (nl[i].n_type & N_TYPE) == N_SECT && nl[i].n_value != 0) {
            syms[idx].addr = nl[i].n_value;
            syms[idx].name = (nl[i].n_strx < strsize) ? strs + nl[i].n_strx : "???";
            idx++;
        }
    }

    qsort(syms, count, sizeof(macho_sym_entry), sym_compare);
    *out_count = count;
    return syms;
}

/**
 * @brief Collect function symbols from a 32-bit Mach-O symbol table.
 */
static macho_sym_entry* collect_symbols_32(const unsigned char *block, size_t file_size,
                                            uint32_t symoff, uint32_t nsyms,
                                            uint32_t stroff, uint32_t strsize,
                                            uint32_t *out_count)
{
    *out_count = 0;
    if (symoff + (size_t)nsyms * sizeof(struct nlist) > file_size) return NULL;
    if (stroff + strsize > file_size) return NULL;

    const struct nlist *nl = (const struct nlist *)(block + symoff);
    const char *strs = (const char *)(block + stroff);

    uint32_t count = 0;
    for (uint32_t i = 0; i < nsyms; i++) {
        if ((nl[i].n_type & N_STAB) == 0 && (nl[i].n_type & N_TYPE) == N_SECT && nl[i].n_value != 0)
            count++;
    }
    if (count == 0) return NULL;

    macho_sym_entry *syms = malloc(count * sizeof(macho_sym_entry));
    if (!syms) return NULL;

    uint32_t idx = 0;
    for (uint32_t i = 0; i < nsyms; i++) {
        if ((nl[i].n_type & N_STAB) == 0 && (nl[i].n_type & N_TYPE) == N_SECT && nl[i].n_value != 0) {
            syms[idx].addr = nl[i].n_value;
            syms[idx].name = (nl[i].n_strx < strsize) ? strs + nl[i].n_strx : "???";
            idx++;
        }
    }

    qsort(syms, count, sizeof(macho_sym_entry), sym_compare);
    *out_count = count;
    return syms;
}

/**
 * @brief Disassemble a section's bytes with symbol labels.
 */
static void disasm_section_with_symbols(const unsigned char *code, size_t size,
                                         uint64_t base_addr, int mode,
                                         const macho_sym_entry *syms, uint32_t nsyms)
{
    ud_t ud_obj;
    ud_init(&ud_obj);
    ud_set_input_buffer(&ud_obj, code, size);
    ud_set_mode(&ud_obj, mode);
    ud_set_pc(&ud_obj, base_addr);
    ud_set_syntax(&ud_obj, UD_SYN_INTEL);

    while (ud_disassemble(&ud_obj)) {
        uint64_t pc = ud_insn_off(&ud_obj);

        /* Check if this address has a symbol label */
        const char *label = find_symbol(syms, nsyms, pc);
        if (label) {
            printf("\n" COLOR_YELLOW "<%s>:" COLOR_RESET "\n", label);
        }

        /* Print address and instruction */
        const char *asm_str = ud_insn_asm(&ud_obj);
        printf("  " COLOR_GREEN "0x%08lx" COLOR_RESET ":  ", (unsigned long)pc);
        print_highlight_asm(asm_str);
        printf("\n");
    }
}

/* =================== 64-bit Mach-O disassembly =================== */

static bool disasm_macho_64(const unsigned char *block, size_t file_size)
{
    mach_header_64 *hdr = (mach_header_64 *)block;

    if (file_size < sizeof(mach_header_64)) return false;
    if ((size_t)hdr->sizeofcmds + sizeof(mach_header_64) > file_size) return false;

    printf(COLOR_BLUE "=== Mach-O Disassembly (64-bit) ===\n" COLOR_RESET);
    printf(COLOR_GREEN "CPU Type:      " COLOR_RESET "%s\n", cpu_type_to_string(hdr->cputype));
    printf(COLOR_GREEN "File Type:     " COLOR_RESET "%s\n", get_mach_o_type(hdr->filetype));

    /* Determine disassembly mode */
    int mode = 64;
    if (hdr->cputype == CPU_TYPE_X86) mode = 32;
    else if (hdr->cputype == CPU_TYPE_ARM64 || hdr->cputype == CPU_TYPE_ARM) {
        fprintf(stderr, COLOR_RED "[!] ARM disassembly not yet supported (x86/x86-64 only)\n" COLOR_RESET);
        return false;
    }

    /* First pass: find LC_SYMTAB for symbol labels */
    uint32_t symoff = 0, nsyms = 0, stroff = 0, strsize = 0;
    const unsigned char *cmd_ptr = block + sizeof(mach_header_64);
    for (uint32_t i = 0; i < hdr->ncmds; i++) {
        if (cmd_ptr + sizeof(load_command) > block + file_size) break;
        load_command *lc = (load_command *)cmd_ptr;
        if (lc->cmdsize < sizeof(load_command) || cmd_ptr + lc->cmdsize > block + file_size) break;

        if (lc->cmd == LC_SYMTAB) {
            struct symtab_command *sc = (struct symtab_command *)cmd_ptr;
            symoff = sc->symoff; nsyms = sc->nsyms;
            stroff = sc->stroff; strsize = sc->strsize;
        }
        cmd_ptr += lc->cmdsize;
    }

    /* Collect symbols */
    uint32_t sym_count = 0;
    macho_sym_entry *syms = collect_symbols_64(block, file_size, symoff, nsyms, stroff, strsize, &sym_count);

    if (sym_count > 0)
        printf(COLOR_GREEN "Symbols:       " COLOR_RESET "%u function symbols found\n", sym_count);

    /* Second pass: disassemble executable sections */
    cmd_ptr = block + sizeof(mach_header_64);
    for (uint32_t i = 0; i < hdr->ncmds; i++) {
        if (cmd_ptr + sizeof(load_command) > block + file_size) break;
        load_command *lc = (load_command *)cmd_ptr;
        if (lc->cmdsize < sizeof(load_command) || cmd_ptr + lc->cmdsize > block + file_size) break;

        if (lc->cmd == LC_SEGMENT_64) {
            segment_command_64 *seg = (segment_command_64 *)cmd_ptr;
            const unsigned char *sec_ptr = cmd_ptr + sizeof(segment_command_64);

            for (uint32_t s = 0; s < seg->nsects; s++) {
                if (sec_ptr + sizeof(section_64) > block + file_size) break;
                section_64 *sec = (section_64 *)sec_ptr;

                if (macho_section_is_executable(sec->flags) && sec->size > 0) {
                    if (sec->offset + sec->size <= file_size) {
                        printf(COLOR_BLUE "\n--- %s,%s (0x%lx bytes at offset 0x%x) ---\n" COLOR_RESET,
                               sec->segname, sec->sectname,
                               (unsigned long)sec->size, sec->offset);

                        const unsigned char *code = block + sec->offset;
                        disasm_section_with_symbols(code, sec->size, sec->addr, mode, syms, sym_count);
                    }
                }
                sec_ptr += sizeof(section_64);
            }
        }
        cmd_ptr += lc->cmdsize;
    }

    free(syms);
    printf("\n");
    return true;
}

/* =================== 32-bit Mach-O disassembly =================== */

static bool disasm_macho_32(const unsigned char *block, size_t file_size)
{
    mach_header *hdr = (mach_header *)block;

    if (file_size < sizeof(mach_header)) return false;
    if ((size_t)hdr->sizeofcmds + sizeof(mach_header) > file_size) return false;

    printf(COLOR_BLUE "=== Mach-O Disassembly (32-bit) ===\n" COLOR_RESET);
    printf(COLOR_GREEN "CPU Type:      " COLOR_RESET "%s\n", cpu_type_to_string(hdr->cputype));
    printf(COLOR_GREEN "File Type:     " COLOR_RESET "%s\n", get_mach_o_type(hdr->filetype));

    int mode = 32;

    /* First pass: find LC_SYMTAB */
    uint32_t symoff = 0, nsyms = 0, stroff = 0, strsize = 0;
    const unsigned char *cmd_ptr = block + sizeof(mach_header);
    for (uint32_t i = 0; i < hdr->ncmds; i++) {
        if (cmd_ptr + sizeof(load_command) > block + file_size) break;
        load_command *lc = (load_command *)cmd_ptr;
        if (lc->cmdsize < sizeof(load_command) || cmd_ptr + lc->cmdsize > block + file_size) break;

        if (lc->cmd == LC_SYMTAB) {
            struct symtab_command *sc = (struct symtab_command *)cmd_ptr;
            symoff = sc->symoff; nsyms = sc->nsyms;
            stroff = sc->stroff; strsize = sc->strsize;
        }
        cmd_ptr += lc->cmdsize;
    }

    uint32_t sym_count = 0;
    macho_sym_entry *syms = collect_symbols_32(block, file_size, symoff, nsyms, stroff, strsize, &sym_count);

    if (sym_count > 0)
        printf(COLOR_GREEN "Symbols:       " COLOR_RESET "%u function symbols found\n", sym_count);

    /* Second pass: disassemble executable sections */
    cmd_ptr = block + sizeof(mach_header);
    for (uint32_t i = 0; i < hdr->ncmds; i++) {
        if (cmd_ptr + sizeof(load_command) > block + file_size) break;
        load_command *lc = (load_command *)cmd_ptr;
        if (lc->cmdsize < sizeof(load_command) || cmd_ptr + lc->cmdsize > block + file_size) break;

        if (lc->cmd == LC_SEGMENT) {
            segment_command *seg = (segment_command *)cmd_ptr;
            const unsigned char *sec_ptr = cmd_ptr + sizeof(segment_command);

            for (uint32_t s = 0; s < seg->nsects; s++) {
                if (sec_ptr + sizeof(section) > block + file_size) break;
                section *sec = (section *)sec_ptr;

                if (macho_section_is_executable(sec->flags) && sec->size > 0) {
                    if (sec->offset + sec->size <= file_size) {
                        printf(COLOR_BLUE "\n--- %s,%s (0x%x bytes at offset 0x%x) ---\n" COLOR_RESET,
                               sec->segname, sec->sectname, sec->size, sec->offset);

                        const unsigned char *code = block + sec->offset;
                        disasm_section_with_symbols(code, sec->size, sec->addr, mode, syms, sym_count);
                    }
                }
                sec_ptr += sizeof(section);
            }
        }
        cmd_ptr += lc->cmdsize;
    }

    free(syms);
    printf("\n");
    return true;
}

/* =================== Public entry point =================== */

bool print_macho_disasm(bparser *parser, void *arg)
{
    (void)arg;
    if (!parser || !parser->block || parser->size < sizeof(uint32_t)) {
        fprintf(stderr, COLOR_RED "[!] Invalid parser state for disassembly\n" COLOR_RESET);
        return false;
    }

    const unsigned char *block = (const unsigned char *)parser->block;
    uint32_t magic = *(const uint32_t *)block;

    if (magic == MH_MAGIC_64)
        return disasm_macho_64(block, parser->size);
    else if (magic == MH_MAGIC)
        return disasm_macho_32(block, parser->size);

    fprintf(stderr, COLOR_RED "[!] Not a valid Mach-O file\n" COLOR_RESET);
    return false;
}
