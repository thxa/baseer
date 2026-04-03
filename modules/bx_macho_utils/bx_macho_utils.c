/**
 * @file bx_macho_utils.c
 * @brief Utility functions for Mach-O binary analysis.
 */
#include "bx_macho_utils.h"
#include "../../utils/ui.h"
#include <stdio.h>
#include <string.h>

/* =================== Type-to-string conversions =================== */

const char* get_mach_o_type(unsigned int type)
{
    switch (type) {
        case MH_OBJECT:     return "MH_OBJECT (relocatable object file)";
        case MH_EXECUTE:    return "MH_EXECUTE (demand paged executable)";
        case MH_FVMLIB:     return "MH_FVMLIB (fixed VM shared library)";
        case MH_CORE:       return "MH_CORE (core file)";
        case MH_PRELOAD:    return "MH_PRELOAD (preloaded executable)";
        case MH_DYLIB:      return "MH_DYLIB (dynamically bound shared library)";
        case MH_DYLINKER:   return "MH_DYLINKER (dynamic link editor)";
        case MH_BUNDLE:     return "MH_BUNDLE (dynamically bound bundle)";
        case MH_DYLIB_STUB: return "MH_DYLIB_STUB (shared library stub)";
        case MH_DSYM:       return "MH_DSYM (debug symbols companion)";
        case MH_KEXT_BUNDLE:return "MH_KEXT_BUNDLE (x86_64 kext)";
        case MH_FILESET:    return "MH_FILESET (set of mach-o's)";
        default:            return "Unknown Mach-O file type";
    }
}

const char* cpu_type_to_string(cpu_type_t cputype)
{
    switch (cputype) {
        case CPU_TYPE_X86:       return "x86 (i386)";
        case CPU_TYPE_X86_64:    return "x86_64";
        case CPU_TYPE_ARM:       return "ARM";
        case CPU_TYPE_ARM64:     return "ARM64 (AArch64)";
        case CPU_TYPE_POWERPC:   return "PowerPC";
        case CPU_TYPE_POWERPC64: return "PowerPC64";
        default:                 return "Unknown";
    }
}

const char* cpu_subtype_to_string(cpu_type_t cputype, cpu_subtype_t cpusubtype)
{
    if (cputype == CPU_TYPE_X86_64) {
        switch (cpusubtype) {
            case CPU_SUBTYPE_X86_64_ALL: return "ALL";
            case CPU_SUBTYPE_X86_64_H:   return "Haswell";
            default:                     return "Unknown";
        }
    } else if (cputype == CPU_TYPE_ARM64) {
        switch (cpusubtype) {
            case CPU_SUBTYPE_ARM64_ALL: return "ALL";
            case CPU_SUBTYPE_ARM64E:    return "ARM64E";
            default:                    return "Unknown";
        }
    } else if (cputype == CPU_TYPE_X86) {
        return (cpusubtype == CPU_SUBTYPE_X86_ALL) ? "ALL" : "Unknown";
    }
    return "Unknown";
}

const char* nlist_type_to_string(uint8_t n_type)
{
    if (n_type & N_STAB) return "STAB";
    switch (n_type & N_TYPE) {
        case N_UNDF: return (n_type & N_EXT) ? "UNDEF (ext)" : "UNDEF";
        case N_ABS:  return (n_type & N_EXT) ? "ABS (ext)"   : "ABS";
        case N_SECT: return (n_type & N_EXT) ? "SECT (ext)"  : "SECT";
        case N_PBUD: return "PBUD";
        case N_INDR: return "INDR";
        default:     return "???";
    }
}

const char* nlist_section_to_string(uint8_t n_sect)
{
    if (n_sect == NO_SECT) return "NO_SECT";
    static char buf[16];
    snprintf(buf, sizeof(buf), "sect_%u", n_sect);
    return buf;
}

/* =================== Flag / field printers =================== */

void print_mach_o_flags(unsigned int flags)
{
    struct { unsigned int mask; const char *name; } flag_table[] = {
        { MH_NOUNDEFS,                     "MH_NOUNDEFS" },
        { MH_INCRLINK,                     "MH_INCRLINK" },
        { MH_DYLDLINK,                     "MH_DYLDLINK" },
        { MH_BINDATLOAD,                   "MH_BINDATLOAD" },
        { MH_PREBOUND,                     "MH_PREBOUND" },
        { MH_SPLIT_SEGS,                   "MH_SPLIT_SEGS" },
        { MH_LAZY_INIT,                    "MH_LAZY_INIT (obsolete)" },
        { MH_TWOLEVEL,                     "MH_TWOLEVEL" },
        { MH_FORCE_FLAT,                   "MH_FORCE_FLAT" },
        { MH_NOMULTIDEFS,                  "MH_NOMULTIDEFS" },
        { MH_NOFIXPREBINDING,              "MH_NOFIXPREBINDING" },
        { MH_PREBINDABLE,                  "MH_PREBINDABLE" },
        { MH_ALLMODSBOUND,                 "MH_ALLMODSBOUND" },
        { MH_SUBSECTIONS_VIA_SYMBOLS,      "MH_SUBSECTIONS_VIA_SYMBOLS" },
        { MH_CANONICAL,                    "MH_CANONICAL" },
        { MH_WEAK_DEFINES,                 "MH_WEAK_DEFINES" },
        { MH_BINDS_TO_WEAK,               "MH_BINDS_TO_WEAK" },
        { MH_ALLOW_STACK_EXECUTION,        "MH_ALLOW_STACK_EXECUTION" },
        { MH_ROOT_SAFE,                    "MH_ROOT_SAFE" },
        { MH_SETUID_SAFE,                  "MH_SETUID_SAFE" },
        { MH_NO_REEXPORTED_DYLIBS,         "MH_NO_REEXPORTED_DYLIBS" },
        { MH_PIE,                          "MH_PIE" },
        { MH_DEAD_STRIPPABLE_DYLIB,        "MH_DEAD_STRIPPABLE_DYLIB" },
        { MH_HAS_TLV_DESCRIPTORS,          "MH_HAS_TLV_DESCRIPTORS" },
        { MH_NO_HEAP_EXECUTION,            "MH_NO_HEAP_EXECUTION" },
        { MH_APP_EXTENSION_SAFE,           "MH_APP_EXTENSION_SAFE" },
        { MH_NLIST_OUTOFSYNC_WITH_DYLDINFO,"MH_NLIST_OUTOFSYNC_WITH_DYLDINFO" },
        { MH_SIM_SUPPORT,                  "MH_SIM_SUPPORT" },
        { MH_DYLIB_IN_CACHE,              "MH_DYLIB_IN_CACHE" },
    };
    int n = sizeof(flag_table) / sizeof(flag_table[0]);
    for (int i = 0; i < n; i++) {
        if (flags & flag_table[i].mask)
            printf("    - %s\n", flag_table[i].name);
    }
}

const char* get_load_command_name(uint32_t cmd)
{
    uint32_t base = cmd & ~LC_REQ_DYLD;
    switch (base) {
        case 0x1:  return "LC_SEGMENT";
        case 0x2:  return "LC_SYMTAB";
        case 0x3:  return "LC_SYMSEG (obsolete)";
        case 0x4:  return "LC_THREAD";
        case 0x5:  return "LC_UNIXTHREAD";
        case 0x6:  return "LC_LOADFVMLIB";
        case 0x7:  return "LC_IDFVMLIB";
        case 0x8:  return "LC_IDENT (obsolete)";
        case 0x9:  return "LC_FVMFILE";
        case 0xa:  return "LC_PREPAGE";
        case 0xb:  return "LC_DYSYMTAB";
        case 0xc:  return "LC_LOAD_DYLIB";
        case 0xd:  return "LC_ID_DYLIB";
        case 0xe:  return "LC_LOAD_DYLINKER";
        case 0xf:  return "LC_ID_DYLINKER";
        case 0x10: return "LC_PREBOUND_DYLIB";
        case 0x11: return "LC_ROUTINES";
        case 0x12: return "LC_SUB_FRAMEWORK";
        case 0x13: return "LC_SUB_UMBRELLA";
        case 0x14: return "LC_SUB_CLIENT";
        case 0x15: return "LC_SUB_LIBRARY";
        case 0x16: return "LC_TWOLEVEL_HINTS";
        case 0x17: return "LC_PREBIND_CKSUM";
        case 0x18: return "LC_LOAD_WEAK_DYLIB";
        case 0x19: return "LC_SEGMENT_64";
        case 0x1a: return "LC_ROUTINES_64";
        case 0x1b: return "LC_UUID";
        case 0x1c: return "LC_RPATH";
        case 0x1d: return "LC_CODE_SIGNATURE";
        case 0x1e: return "LC_SEGMENT_SPLIT_INFO";
        case 0x1f: return "LC_REEXPORT_DYLIB";
        case 0x20: return "LC_LAZY_LOAD_DYLIB";
        case 0x21: return "LC_ENCRYPTION_INFO";
        case 0x22: return "LC_DYLD_INFO";
        case 0x23: return "LC_LOAD_UPWARD_DYLIB";
        case 0x24: return "LC_VERSION_MIN_MACOSX";
        case 0x25: return "LC_VERSION_MIN_IPHONEOS";
        case 0x26: return "LC_FUNCTION_STARTS";
        case 0x27: return "LC_DYLD_ENVIRONMENT";
        case 0x28: return "LC_MAIN";
        case 0x29: return "LC_DATA_IN_CODE";
        case 0x2A: return "LC_SOURCE_VERSION";
        case 0x2B: return "LC_DYLIB_CODE_SIGN_DRS";
        case 0x2C: return "LC_ENCRYPTION_INFO_64";
        case 0x2D: return "LC_LINKER_OPTION";
        case 0x2E: return "LC_LINKER_OPTIMIZATION_HINT";
        case 0x2F: return "LC_VERSION_MIN_TVOS";
        case 0x30: return "LC_VERSION_MIN_WATCHOS";
        case 0x31: return "LC_NOTE";
        case 0x32: return "LC_BUILD_VERSION";
        case 0x33: return "LC_DYLD_EXPORTS_TRIE";
        case 0x34: return "LC_DYLD_CHAINED_FIXUPS";
        case 0x35: return "LC_FILESET_ENTRY";
        default:   return "Unknown";
    }
}

void print_load_command_info(uint32_t cmd)
{
    printf(COLOR_YELLOW "  [0x%08x] %s" COLOR_RESET, cmd, get_load_command_name(cmd));
    if (cmd & LC_REQ_DYLD)
        printf(" (requires dyld)");
    printf("\n");
}

void print_segment_flags(uint32_t flags)
{
    if (flags & SG_HIGHVM)
        printf("      - SG_HIGHVM: file contents map to high VM space\n");
    if (flags & SG_FVMLIB)
        printf("      - SG_FVMLIB: allocated by fixed VM library\n");
    if (flags & SG_NORELOC)
        printf("      - SG_NORELOC: no relocations\n");
    if (flags & SG_PROTECTED_VERSION_1)
        printf("      - SG_PROTECTED_VERSION_1: protected segment\n");
    if (flags & SG_READ_ONLY)
        printf("      - SG_READ_ONLY: read-only after fixups\n");
}

/* =================== Metadata printers =================== */

static void print_prot(uint32_t prot)
{
    printf("%c%c%c",
        (prot & 1) ? 'r' : '-',
        (prot & 2) ? 'w' : '-',
        (prot & 4) ? 'x' : '-');
}

void print_macho_segment64_metadata(segment_command_64 *seg_cmd)
{
    printf(COLOR_GREEN "    Segment Name:  " COLOR_RESET "%-16s\n", seg_cmd->segname);
    printf(COLOR_GREEN "    VM Address:    " COLOR_RESET "0x%016lx\n", (unsigned long)seg_cmd->vmaddr);
    printf(COLOR_GREEN "    VM Size:       " COLOR_RESET "0x%lx (%lu bytes)\n", (unsigned long)seg_cmd->vmsize, (unsigned long)seg_cmd->vmsize);
    printf(COLOR_GREEN "    File Offset:   " COLOR_RESET "0x%lx\n", (unsigned long)seg_cmd->fileoff);
    printf(COLOR_GREEN "    File Size:     " COLOR_RESET "0x%lx (%lu bytes)\n", (unsigned long)seg_cmd->filesize, (unsigned long)seg_cmd->filesize);
    printf(COLOR_GREEN "    Max Prot:      " COLOR_RESET);
    print_prot(seg_cmd->maxprot); printf("\n");
    printf(COLOR_GREEN "    Init Prot:     " COLOR_RESET);
    print_prot(seg_cmd->initprot); printf("\n");
    printf(COLOR_GREEN "    Num Sections:  " COLOR_RESET "%u\n", seg_cmd->nsects);
    if (seg_cmd->flags)
        print_segment_flags(seg_cmd->flags);
}

void print_macho_segment32_metadata(segment_command *seg_cmd)
{
    printf(COLOR_GREEN "    Segment Name:  " COLOR_RESET "%-16s\n", seg_cmd->segname);
    printf(COLOR_GREEN "    VM Address:    " COLOR_RESET "0x%08x\n", seg_cmd->vmaddr);
    printf(COLOR_GREEN "    VM Size:       " COLOR_RESET "0x%x (%u bytes)\n", seg_cmd->vmsize, seg_cmd->vmsize);
    printf(COLOR_GREEN "    File Offset:   " COLOR_RESET "0x%x\n", seg_cmd->fileoff);
    printf(COLOR_GREEN "    File Size:     " COLOR_RESET "0x%x (%u bytes)\n", seg_cmd->filesize, seg_cmd->filesize);
    printf(COLOR_GREEN "    Max Prot:      " COLOR_RESET);
    print_prot(seg_cmd->maxprot); printf("\n");
    printf(COLOR_GREEN "    Init Prot:     " COLOR_RESET);
    print_prot(seg_cmd->initprot); printf("\n");
    printf(COLOR_GREEN "    Num Sections:  " COLOR_RESET "%u\n", seg_cmd->nsects);
    if (seg_cmd->flags)
        print_segment_flags(seg_cmd->flags);
}

void print_macho_section64_metadata(section_64 *s)
{
    printf(COLOR_CYAN "      %-16s" COLOR_RESET " %-16s  ", s->sectname, s->segname);
    printf("addr=0x%016lx  size=0x%lx  offset=0x%x  align=2^%u",
           (unsigned long)s->addr, (unsigned long)s->size, s->offset, s->align);
    if (s->flags & S_ATTR_PURE_INSTRUCTIONS) printf("  " COLOR_RED "EXEC" COLOR_RESET);
    printf("\n");
}

void print_macho_section32_metadata(section *s)
{
    printf(COLOR_CYAN "      %-16s" COLOR_RESET " %-16s  ", s->sectname, s->segname);
    printf("addr=0x%08x  size=0x%x  offset=0x%x  align=2^%u",
           s->addr, s->size, s->offset, s->align);
    if (s->flags & S_ATTR_PURE_INSTRUCTIONS) printf("  " COLOR_RED "EXEC" COLOR_RESET);
    printf("\n");
}

/* =================== Symbol table printers =================== */

void print_macho_symbols_64(const unsigned char *block, size_t file_size,
                            uint32_t symoff, uint32_t nsyms,
                            uint32_t stroff, uint32_t strsize)
{
    if (symoff + (size_t)nsyms * sizeof(struct nlist_64) > file_size) {
        fprintf(stderr, COLOR_RED "  [!] Symbol table extends beyond file bounds\n" COLOR_RESET);
        return;
    }
    if (stroff + strsize > file_size) {
        fprintf(stderr, COLOR_RED "  [!] String table extends beyond file bounds\n" COLOR_RESET);
        return;
    }

    const struct nlist_64 *syms = (const struct nlist_64 *)(block + symoff);
    const char *strs = (const char *)(block + stroff);

    printf(COLOR_BLUE "\n  === Symbol Table (%u symbols) ===\n" COLOR_RESET, nsyms);
    printf("  %-4s  %-18s  %-12s  %-8s  %s\n",
           "Idx", "Value", "Type", "Section", "Name");
    printf("  %-4s  %-18s  %-12s  %-8s  %s\n",
           "---", "------------------", "------------", "--------", "----");

    for (uint32_t i = 0; i < nsyms; i++) {
        const char *name = "";
        if (syms[i].n_strx < strsize)
            name = strs + syms[i].n_strx;

        const char *type_str = nlist_type_to_string(syms[i].n_type);
        const char *sect_str = nlist_section_to_string(syms[i].n_sect);

        /* Color external symbols differently */
        const char *color = (syms[i].n_type & N_EXT) ? COLOR_GREEN : COLOR_RESET;

        printf("  %-4u  %s0x%016lx" COLOR_RESET "  %-12s  %-8s  %s%s" COLOR_RESET "\n",
               i, color, (unsigned long)syms[i].n_value,
               type_str, sect_str, color, name);
    }
}

void print_macho_symbols_32(const unsigned char *block, size_t file_size,
                            uint32_t symoff, uint32_t nsyms,
                            uint32_t stroff, uint32_t strsize)
{
    if (symoff + (size_t)nsyms * sizeof(struct nlist) > file_size) {
        fprintf(stderr, COLOR_RED "  [!] Symbol table extends beyond file bounds\n" COLOR_RESET);
        return;
    }
    if (stroff + strsize > file_size) {
        fprintf(stderr, COLOR_RED "  [!] String table extends beyond file bounds\n" COLOR_RESET);
        return;
    }

    const struct nlist *syms = (const struct nlist *)(block + symoff);
    const char *strs = (const char *)(block + stroff);

    printf(COLOR_BLUE "\n  === Symbol Table (%u symbols) ===\n" COLOR_RESET, nsyms);
    printf("  %-4s  %-10s  %-12s  %-8s  %s\n",
           "Idx", "Value", "Type", "Section", "Name");
    printf("  %-4s  %-10s  %-12s  %-8s  %s\n",
           "---", "----------", "------------", "--------", "----");

    for (uint32_t i = 0; i < nsyms; i++) {
        const char *name = "";
        if (syms[i].n_strx < strsize)
            name = strs + syms[i].n_strx;

        const char *type_str = nlist_type_to_string(syms[i].n_type);
        const char *sect_str = nlist_section_to_string(syms[i].n_sect);
        const char *color = (syms[i].n_type & N_EXT) ? COLOR_GREEN : COLOR_RESET;

        printf("  %-4u  %s0x%08x" COLOR_RESET "  %-12s  %-8s  %s%s" COLOR_RESET "\n",
               i, color, syms[i].n_value,
               type_str, sect_str, color, name);
    }
}

/* =================== Version / UUID decoders =================== */

void print_tool_version(uint32_t version)
{
    printf("%u.%u.%u",
           (version >> 16) & 0xffff,
           (version >> 8) & 0xff,
           version & 0xff);
}

void print_version_xyz(uint32_t version)
{
    printf("%u.%u.%u",
           (version >> 16) & 0xffff,
           (version >> 8) & 0xff,
           version & 0xff);
}

void print_uuid(const uint8_t uuid[16])
{
    printf("%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        uuid[0], uuid[1], uuid[2], uuid[3],
        uuid[4], uuid[5], uuid[6], uuid[7],
        uuid[8], uuid[9], uuid[10], uuid[11],
        uuid[12], uuid[13], uuid[14], uuid[15]);
}

void print_source_version(uint64_t packed_version)
{
    uint64_t A = (packed_version >> 40) & 0xFFFFFF;
    uint64_t B = (packed_version >> 30) & 0x3FF;
    uint64_t C = (packed_version >> 20) & 0x3FF;
    uint64_t D = (packed_version >> 10) & 0x3FF;
    uint64_t E =  packed_version        & 0x3FF;
    printf("%lu.%lu.%lu.%lu.%lu", A, B, C, D, E);
}

const char* platform_to_string(int platform)
{
    switch (platform) {
        case PLATFORM_MACOS:              return "macOS";
        case PLATFORM_IOS:                return "iOS";
        case PLATFORM_TVOS:               return "tvOS";
        case PLATFORM_WATCHOS:            return "watchOS";
        case PLATFORM_BRIDGEOS:           return "bridgeOS";
        case PLATFORM_MACCATALYST:        return "Mac Catalyst";
        case PLATFORM_IOSSIMULATOR:       return "iOS Simulator";
        case PLATFORM_TVOSSIMULATOR:      return "tvOS Simulator";
        case PLATFORM_WATCHOSSIMULATOR:   return "watchOS Simulator";
        case PLATFORM_DRIVERKIT:          return "DriverKit";
        default:                          return "Unknown Platform";
    }
}

const char* tool_to_string(int tool)
{
    switch (tool) {
        case TOOL_CLANG: return "Clang";
        case TOOL_SWIFT: return "Swift";
        case TOOL_LD:    return "LD (Linker)";
        default:         return "Unknown Tool";
    }
}

/* =================== Section attribute helpers =================== */

bool macho_section_is_executable(uint32_t flags)
{
    return (flags & S_ATTR_PURE_INSTRUCTIONS) || (flags & S_ATTR_SOME_INSTRUCTIONS);
}

const char* macho_section_type_to_string(uint32_t flags)
{
    switch (flags & SECTION_TYPE) {
        case S_REGULAR:                    return "S_REGULAR";
        case S_ZEROFILL:                   return "S_ZEROFILL";
        case S_CSTRING_LITERALS:           return "S_CSTRING_LITERALS";
        case S_4BYTE_LITERALS:             return "S_4BYTE_LITERALS";
        case S_8BYTE_LITERALS:             return "S_8BYTE_LITERALS";
        case S_LITERAL_POINTERS:           return "S_LITERAL_POINTERS";
        case S_NON_LAZY_SYMBOL_POINTERS:   return "S_NON_LAZY_SYMBOL_POINTERS";
        case S_LAZY_SYMBOL_POINTERS:       return "S_LAZY_SYMBOL_POINTERS";
        case S_SYMBOL_STUBS:               return "S_SYMBOL_STUBS";
        case S_MOD_INIT_FUNC_POINTERS:     return "S_MOD_INIT_FUNC_POINTERS";
        case S_MOD_TERM_FUNC_POINTERS:     return "S_MOD_TERM_FUNC_POINTERS";
        case S_COALESCED:                  return "S_COALESCED";
        case S_GB_ZEROFILL:                return "S_GB_ZEROFILL";
        case S_INTERPOSING:                return "S_INTERPOSING";
        case S_16BYTE_LITERALS:            return "S_16BYTE_LITERALS";
        case S_DTRACE_DOF:                 return "S_DTRACE_DOF";
        case S_THREAD_LOCAL_REGULAR:       return "S_THREAD_LOCAL_REGULAR";
        case S_THREAD_LOCAL_ZEROFILL:      return "S_THREAD_LOCAL_ZEROFILL";
        case S_THREAD_LOCAL_VARIABLES:     return "S_THREAD_LOCAL_VARIABLES";
        default:                           return "UNKNOWN";
    }
}
