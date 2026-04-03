/**
 * @file b_macho_metadata.c
 * @brief Mach-O metadata parser and pretty-printer.
 *
 * Parses and displays full metadata for Mach-O binaries including:
 * - Header (magic, CPU type, file type, flags)
 * - All load commands with detailed output
 * - Segments and their sections
 * - Symbol tables (LC_SYMTAB)
 * - Dynamic symbol table summary (LC_DYSYMTAB)
 * - Shared libraries (LC_LOAD_DYLIB)
 * - Dynamic linker (LC_LOAD_DYLINKER)
 * - Entry point (LC_MAIN)
 * - UUID, build version, source version
 * - Code signature and function starts info
 * - Section hex dumps
 *
 * Supports both 32-bit and 64-bit Mach-O files with full input validation.
 */
#include "./b_macho_metadata.h"
#include "../bparser/bparser.h"
#include "../../baseer.h"
#include "../bx_macho_utils/bx_macho_utils.h"
#include "../../utils/ui.h"
#include <stdio.h>
#include <string.h>

/* =================== Validation helpers =================== */

/**
 * @brief Validate a Mach-O header's fields against the file size.
 * @return true if the header appears sane, false otherwise.
 */
static bool validate_macho_header(const unsigned char *block, size_t file_size,
                                  uint32_t ncmds, uint32_t sizeofcmds,
                                  size_t header_size)
{
    if (file_size < header_size) {
        fprintf(stderr, COLOR_RED "[!] File too small for Mach-O header\n" COLOR_RESET);
        return false;
    }
    if ((size_t)sizeofcmds + header_size > file_size) {
        fprintf(stderr, COLOR_RED "[!] Load commands extend beyond file bounds "
                "(header=%zu + cmds=%u > size=%zu)\n" COLOR_RESET,
                header_size, sizeofcmds, file_size);
        return false;
    }
    if (ncmds > 10000) {
        fprintf(stderr, COLOR_RED "[!] Suspicious number of load commands: %u\n" COLOR_RESET, ncmds);
        return false;
    }
    return true;
}

/**
 * @brief Check if a load command's size is valid.
 */
static bool validate_load_command(const load_command *lc, const unsigned char *cmd_ptr,
                                  const unsigned char *end_ptr, uint32_t cmd_idx)
{
    if (lc->cmdsize < sizeof(load_command)) {
        fprintf(stderr, COLOR_RED "[!] Load command %u has invalid size %u\n" COLOR_RESET,
                cmd_idx, lc->cmdsize);
        return false;
    }
    if (cmd_ptr + lc->cmdsize > end_ptr) {
        fprintf(stderr, COLOR_RED "[!] Load command %u extends beyond file bounds\n" COLOR_RESET,
                cmd_idx);
        return false;
    }
    return true;
}

/* =================== Load command handlers =================== */

static void handle_segment_64(const unsigned char *cmd_ptr, const unsigned char *block,
                              size_t file_size)
{
    segment_command_64 *seg = (segment_command_64 *)cmd_ptr;
    print_macho_segment64_metadata(seg);

    if (seg->nsects > 0) {
        const unsigned char *sec_ptr = cmd_ptr + sizeof(segment_command_64);
        for (uint32_t s = 0; s < seg->nsects; s++) {
            if (sec_ptr + sizeof(section_64) > block + file_size) break;
            section_64 *sec = (section_64 *)sec_ptr;
            print_macho_section64_metadata(sec);
            sec_ptr += sizeof(section_64);
        }
    }
}

static void handle_segment_32(const unsigned char *cmd_ptr, const unsigned char *block,
                              size_t file_size)
{
    segment_command *seg = (segment_command *)cmd_ptr;
    print_macho_segment32_metadata(seg);

    if (seg->nsects > 0) {
        const unsigned char *sec_ptr = cmd_ptr + sizeof(segment_command);
        for (uint32_t s = 0; s < seg->nsects; s++) {
            if (sec_ptr + sizeof(section) > block + file_size) break;
            section *sec = (section *)sec_ptr;
            print_macho_section32_metadata(sec);
            sec_ptr += sizeof(section);
        }
    }
}

static void handle_symtab(const unsigned char *cmd_ptr, const unsigned char *block,
                          size_t file_size, bool is_64bit)
{
    struct symtab_command *sym = (struct symtab_command *)cmd_ptr;
    printf(COLOR_GREEN "    Symbol offset: " COLOR_RESET "0x%x\n", sym->symoff);
    printf(COLOR_GREEN "    Num symbols:   " COLOR_RESET "%u\n", sym->nsyms);
    printf(COLOR_GREEN "    String offset: " COLOR_RESET "0x%x\n", sym->stroff);
    printf(COLOR_GREEN "    String size:   " COLOR_RESET "%u bytes\n", sym->strsize);

    if (is_64bit)
        print_macho_symbols_64(block, file_size, sym->symoff, sym->nsyms, sym->stroff, sym->strsize);
    else
        print_macho_symbols_32(block, file_size, sym->symoff, sym->nsyms, sym->stroff, sym->strsize);
}

static void handle_dysymtab(const unsigned char *cmd_ptr)
{
    struct dysymtab_command *dys = (struct dysymtab_command *)cmd_ptr;
    printf(COLOR_GREEN "    Local symbols:      " COLOR_RESET "%u (index %u)\n", dys->nlocalsym, dys->ilocalsym);
    printf(COLOR_GREEN "    External symbols:   " COLOR_RESET "%u (index %u)\n", dys->nextdefsym, dys->iextdefsym);
    printf(COLOR_GREEN "    Undefined symbols:  " COLOR_RESET "%u (index %u)\n", dys->nundefsym, dys->iundefsym);
    if (dys->nindirectsyms > 0)
        printf(COLOR_GREEN "    Indirect symbols:   " COLOR_RESET "%u (offset 0x%x)\n", dys->nindirectsyms, dys->indirectsymoff);
    if (dys->nextrel > 0)
        printf(COLOR_GREEN "    External relocs:    " COLOR_RESET "%u (offset 0x%x)\n", dys->nextrel, dys->extreloff);
    if (dys->nlocrel > 0)
        printf(COLOR_GREEN "    Local relocs:       " COLOR_RESET "%u (offset 0x%x)\n", dys->nlocrel, dys->locreloff);
}

static void handle_dylib(const unsigned char *cmd_ptr, uint32_t cmdsize)
{
    dylib_command *dl = (dylib_command *)cmd_ptr;
    uint32_t name_off = dl->dylib.name_offset;

    const char *name = "(invalid offset)";
    if (name_off < cmdsize)
        name = (const char *)cmd_ptr + name_off;

    printf(COLOR_GREEN "    Library:       " COLOR_RESET "%s\n", name);
    printf(COLOR_GREEN "    Version:       " COLOR_RESET);
    print_version_xyz(dl->dylib.current_version);
    printf("\n");
    printf(COLOR_GREEN "    Compat:        " COLOR_RESET);
    print_version_xyz(dl->dylib.compatibility_version);
    printf("\n");
}

static void handle_dylinker(const unsigned char *cmd_ptr, uint32_t cmdsize)
{
    dylinker_command *dl = (dylinker_command *)cmd_ptr;
    uint32_t name_off = dl->name_offset;

    const char *name = "(invalid offset)";
    if (name_off < cmdsize)
        name = (const char *)cmd_ptr + name_off;

    printf(COLOR_GREEN "    Dynamic Linker: " COLOR_RESET "%s\n", name);
}

static void handle_uuid(const unsigned char *cmd_ptr)
{
    uuid_command *uc = (uuid_command *)cmd_ptr;
    printf(COLOR_GREEN "    UUID: " COLOR_RESET);
    print_uuid(uc->uuid);
    printf("\n");
}

static void handle_main(const unsigned char *cmd_ptr)
{
    entry_point_command *ep = (entry_point_command *)cmd_ptr;
    printf(COLOR_GREEN "    Entry offset:  " COLOR_RESET "0x%lx\n", (unsigned long)ep->entryoff);
    if (ep->stacksize > 0)
        printf(COLOR_GREEN "    Stack size:    " COLOR_RESET "0x%lx\n", (unsigned long)ep->stacksize);
}

static void handle_build_version(const unsigned char *cmd_ptr)
{
    build_version_command *bv = (build_version_command *)cmd_ptr;
    printf(COLOR_GREEN "    Platform:      " COLOR_RESET "%s\n", platform_to_string(bv->platform));
    printf(COLOR_GREEN "    Min OS:        " COLOR_RESET);
    print_tool_version(bv->minos); printf("\n");
    printf(COLOR_GREEN "    SDK:           " COLOR_RESET);
    print_tool_version(bv->sdk); printf("\n");

    if (bv->ntools > 0) {
        build_tool_version *tools = (build_tool_version *)(cmd_ptr + sizeof(build_version_command));
        for (uint32_t i = 0; i < bv->ntools; i++) {
            printf(COLOR_GREEN "    Tool:          " COLOR_RESET "%s ", tool_to_string(tools[i].tool));
            print_tool_version(tools[i].version);
            printf("\n");
        }
    }
}

static void handle_source_version(const unsigned char *cmd_ptr)
{
    source_version_command *sv = (source_version_command *)cmd_ptr;
    printf(COLOR_GREEN "    Version:       " COLOR_RESET);
    print_source_version(sv->version);
    printf("\n");
}

static void handle_linkedit_data(const unsigned char *cmd_ptr)
{
    linkedit_data_command *led = (linkedit_data_command *)cmd_ptr;
    printf(COLOR_GREEN "    Data offset:   " COLOR_RESET "0x%x\n", led->dataoff);
    printf(COLOR_GREEN "    Data size:     " COLOR_RESET "%u bytes\n", led->datasize);
}

/* =================== Main metadata function =================== */

/**
 * @brief Parse and print metadata for a 64-bit Mach-O file.
 */
static bool parse_macho_64(const unsigned char *block, size_t file_size)
{
    mach_header_64 *hdr = (mach_header_64 *)block;

    if (!validate_macho_header(block, file_size, hdr->ncmds, hdr->sizeofcmds,
                               sizeof(mach_header_64)))
        return false;

    /* Print header */
    printf(COLOR_BLUE "================= Mach-O Header ====================\n" COLOR_RESET);
    printf(COLOR_GREEN "Class:             " COLOR_RESET "64-bit\n");
    printf(COLOR_GREEN "CPU Type:          " COLOR_RESET "%s\n", cpu_type_to_string(hdr->cputype));
    printf(COLOR_GREEN "CPU Subtype:       " COLOR_RESET "%s\n", cpu_subtype_to_string(hdr->cputype, hdr->cpusubtype));
    printf(COLOR_GREEN "File Type:         " COLOR_RESET "%s\n", get_mach_o_type(hdr->filetype));
    printf(COLOR_GREEN "Load Commands:     " COLOR_RESET "%u (%u bytes)\n", hdr->ncmds, hdr->sizeofcmds);
    printf(COLOR_GREEN "Flags:             " COLOR_RESET "0x%08x\n", hdr->flags);
    print_mach_o_flags(hdr->flags);
    printf(COLOR_GREEN "Reserved:          " COLOR_RESET "0x%x\n", hdr->reserved);

    /* Iterate load commands */
    printf(COLOR_BLUE "\n================= Load Commands (%u) ====================\n" COLOR_RESET,
           hdr->ncmds);

    const unsigned char *cmd_ptr = block + sizeof(mach_header_64);
    const unsigned char *end_ptr = block + file_size;

    for (uint32_t i = 0; i < hdr->ncmds; i++) {
        if (cmd_ptr + sizeof(load_command) > end_ptr) break;
        load_command *lc = (load_command *)cmd_ptr;

        if (!validate_load_command(lc, cmd_ptr, end_ptr, i)) break;

        printf("\n" COLOR_BLUE "[%u/%u] " COLOR_RESET, i + 1, hdr->ncmds);
        print_load_command_info(lc->cmd);
        printf(COLOR_GREEN "    Cmd Size:      " COLOR_RESET "%u bytes\n", lc->cmdsize);

        switch (lc->cmd) {
            case LC_SEGMENT_64:
                handle_segment_64(cmd_ptr, block, file_size);
                break;
            case LC_SYMTAB:
                handle_symtab(cmd_ptr, block, file_size, true);
                break;
            case LC_DYSYMTAB:
                handle_dysymtab(cmd_ptr);
                break;
            case LC_LOAD_DYLIB:
            case LC_LOAD_WEAK_DYLIB:
            case LC_REEXPORT_DYLIB:
            case LC_LAZY_LOAD_DYLIB:
            case LC_ID_DYLIB:
                handle_dylib(cmd_ptr, lc->cmdsize);
                break;
            case LC_LOAD_DYLINKER:
            case LC_ID_DYLINKER:
            case LC_DYLD_ENVIRONMENT:
                handle_dylinker(cmd_ptr, lc->cmdsize);
                break;
            case LC_UUID:
                handle_uuid(cmd_ptr);
                break;
            case LC_MAIN:
                handle_main(cmd_ptr);
                break;
            case LC_BUILD_VERSION:
                handle_build_version(cmd_ptr);
                break;
            case LC_SOURCE_VERSION:
                handle_source_version(cmd_ptr);
                break;
            case LC_CODE_SIGNATURE:
            case LC_FUNCTION_STARTS:
            case LC_DATA_IN_CODE:
            case LC_DYLD_EXPORTS_TRIE:
            case LC_DYLD_CHAINED_FIXUPS:
            case LC_SEGMENT_SPLIT_INFO:
            case LC_DYLIB_CODE_SIGN_DRS:
            case LC_LINKER_OPTIMIZATION_HINT:
                handle_linkedit_data(cmd_ptr);
                break;
            case LC_RPATH: {
                /* rpath_command has name offset at byte 8 */
                uint32_t off = *(uint32_t *)(cmd_ptr + 8);
                if (off < lc->cmdsize)
                    printf(COLOR_GREEN "    Path:          " COLOR_RESET "%s\n", (const char*)cmd_ptr + off);
                break;
            }
            default:
                break;
        }

        cmd_ptr += lc->cmdsize;
    }

    printf("\n");
    return true;
}

/**
 * @brief Parse and print metadata for a 32-bit Mach-O file.
 */
static bool parse_macho_32(const unsigned char *block, size_t file_size)
{
    mach_header *hdr = (mach_header *)block;

    if (!validate_macho_header(block, file_size, hdr->ncmds, hdr->sizeofcmds,
                               sizeof(mach_header)))
        return false;

    /* Print header */
    printf(COLOR_BLUE "================= Mach-O Header ====================\n" COLOR_RESET);
    printf(COLOR_GREEN "Class:             " COLOR_RESET "32-bit\n");
    printf(COLOR_GREEN "CPU Type:          " COLOR_RESET "%s\n", cpu_type_to_string(hdr->cputype));
    printf(COLOR_GREEN "CPU Subtype:       " COLOR_RESET "%s\n", cpu_subtype_to_string(hdr->cputype, hdr->cpusubtype));
    printf(COLOR_GREEN "File Type:         " COLOR_RESET "%s\n", get_mach_o_type(hdr->filetype));
    printf(COLOR_GREEN "Load Commands:     " COLOR_RESET "%u (%u bytes)\n", hdr->ncmds, hdr->sizeofcmds);
    printf(COLOR_GREEN "Flags:             " COLOR_RESET "0x%08x\n", hdr->flags);
    print_mach_o_flags(hdr->flags);

    /* Iterate load commands */
    printf(COLOR_BLUE "\n================= Load Commands (%u) ====================\n" COLOR_RESET,
           hdr->ncmds);

    const unsigned char *cmd_ptr = block + sizeof(mach_header);
    const unsigned char *end_ptr = block + file_size;

    for (uint32_t i = 0; i < hdr->ncmds; i++) {
        if (cmd_ptr + sizeof(load_command) > end_ptr) break;
        load_command *lc = (load_command *)cmd_ptr;

        if (!validate_load_command(lc, cmd_ptr, end_ptr, i)) break;

        printf("\n" COLOR_BLUE "[%u/%u] " COLOR_RESET, i + 1, hdr->ncmds);
        print_load_command_info(lc->cmd);
        printf(COLOR_GREEN "    Cmd Size:      " COLOR_RESET "%u bytes\n", lc->cmdsize);

        switch (lc->cmd) {
            case LC_SEGMENT:
                handle_segment_32(cmd_ptr, block, file_size);
                break;
            case LC_SYMTAB:
                handle_symtab(cmd_ptr, block, file_size, false);
                break;
            case LC_DYSYMTAB:
                handle_dysymtab(cmd_ptr);
                break;
            case LC_LOAD_DYLIB:
            case LC_LOAD_WEAK_DYLIB:
            case LC_REEXPORT_DYLIB:
            case LC_LAZY_LOAD_DYLIB:
            case LC_ID_DYLIB:
                handle_dylib(cmd_ptr, lc->cmdsize);
                break;
            case LC_LOAD_DYLINKER:
            case LC_ID_DYLINKER:
            case LC_DYLD_ENVIRONMENT:
                handle_dylinker(cmd_ptr, lc->cmdsize);
                break;
            case LC_UUID:
                handle_uuid(cmd_ptr);
                break;
            case LC_BUILD_VERSION:
                handle_build_version(cmd_ptr);
                break;
            case LC_SOURCE_VERSION:
                handle_source_version(cmd_ptr);
                break;
            case LC_CODE_SIGNATURE:
            case LC_FUNCTION_STARTS:
            case LC_DATA_IN_CODE:
            case LC_DYLD_EXPORTS_TRIE:
            case LC_DYLD_CHAINED_FIXUPS:
                handle_linkedit_data(cmd_ptr);
                break;
            default:
                break;
        }

        cmd_ptr += lc->cmdsize;
    }

    printf("\n");
    return true;
}

/* =================== Public entry point =================== */

bool b_macho_metadata(bparser *parser, void *arg)
{
    if (!parser || !parser->block) {
        fprintf(stderr, COLOR_RED "[!] Invalid parser state\n" COLOR_RESET);
        return false;
    }

    const unsigned char *block = (const unsigned char *)parser->block;
    size_t file_size = parser->size;

    if (file_size < sizeof(uint32_t)) {
        fprintf(stderr, COLOR_RED "[!] File too small to be a Mach-O binary\n" COLOR_RESET);
        return false;
    }

    uint32_t magic = *(const uint32_t *)block;

    if (magic == MH_MAGIC_64) {
        return parse_macho_64(block, file_size);
    } else if (magic == MH_MAGIC) {
        return parse_macho_32(block, file_size);
    } else {
        fprintf(stderr, COLOR_RED "[!] Unknown Mach-O magic: 0x%08x\n" COLOR_RESET, magic);
        return false;
    }
}
