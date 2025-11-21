#include "bx_macho_utils.h"
#include "../../utils/ui.h"
#include <stdio.h>
const char* get_mach_o_type(unsigned int type) 
{
    switch (type) {
        case 0x1: return "MH_OBJECT (relocatable object file)";
        case 0x2: return "MH_EXECUTE (demand paged executable file)";
        case 0x3: return "MH_FVMLIB (fixed VM shared library file)";
        case 0x4: return "MH_CORE (core file)";
        case 0x5: return "MH_PRELOAD (preloaded executable file)";
        case 0x6: return "MH_DYLIB (dynamically bound shared library)";
        case 0x7: return "MH_DYLINKER (dynamic link editor)";
        case 0x8: return "MH_BUNDLE (dynamically bound bundle file)";
        case 0x9: return "MH_DYLIB_STUB (shared library stub for static linking only)";
        case 0xa: return "MH_DSYM (debug symbols companion file)";
        case 0xb: return "MH_KEXT_BUNDLE (x86_64 kext bundle)";
        case 0xc: return "MH_FILESET (set of mach-o's)";
        default:  return "Unknown Mach-O file type";
    }
}

void print_mach_o_flags(unsigned int flags) 
{
    if (flags & 0x1) printf(" - MH_NOUNDEFS\n");
    if (flags & 0x2) printf(" - MH_INCRLINK\n");
    if (flags & 0x4) printf(" - MH_DYLDLINK\n");
    if (flags & 0x8) printf(" - MH_BINDATLOAD\n");
    if (flags & 0x10) printf(" - MH_PREBOUND\n");
    if (flags & 0x20) printf(" - MH_SPLIT_SEGS\n");
    if (flags & 0x40) printf(" - MH_LAZY_INIT (obsolete)\n");
    if (flags & 0x80) printf(" - MH_TWOLEVEL\n");
    if (flags & 0x100) printf(" - MH_FORCE_FLAT\n");
    if (flags & 0x200) printf(" - MH_NOMULTIDEFS\n");
    if (flags & 0x400) printf(" - MH_NOFIXPREBINDING\n");
    if (flags & 0x800) printf(" - MH_PREBINDABLE\n");
    if (flags & 0x1000) printf(" - MH_ALLMODSBOUND\n");
    if (flags & 0x2000) printf(" - MH_SUBSECTIONS_VIA_SYMBOLS\n");
    if (flags & 0x4000) printf(" - MH_CANONICAL\n");
    if (flags & 0x8000) printf(" - MH_WEAK_DEFINES\n");
    if (flags & 0x10000) printf(" - MH_BINDS_TO_WEAK\n");
    if (flags & 0x20000) printf(" - MH_ALLOW_STACK_EXECUTION\n");
    if (flags & 0x40000) printf(" - MH_ROOT_SAFE\n");
    if (flags & 0x80000) printf(" - MH_SETUID_SAFE\n");
    if (flags & 0x100000) printf(" - MH_NO_REEXPORTED_DYLIBS\n");
    if (flags & 0x200000) printf(" - MH_PIE\n");
    if (flags & 0x400000) printf(" - MH_DEAD_STRIPPABLE_DYLIB\n");
    if (flags & 0x800000) printf(" - MH_HAS_TLV_DESCRIPTORS\n");
    if (flags & 0x1000000) printf(" - MH_NO_HEAP_EXECUTION\n");
    if (flags & 0x02000000) printf(" - MH_APP_EXTENSION_SAFE\n");
    if (flags & 0x04000000) printf(" - MH_NLIST_OUTOFSYNC_WITH_DYLDINFO\n");
    if (flags & 0x08000000) printf(" - MH_SIM_SUPPORT\n");
    if (flags & 0x80000000) printf(" - MH_DYLIB_IN_CACHE\n");
}

const char* get_load_command_name(uint32_t cmd) 
{
    uint32_t base = cmd & ~LC_REQ_DYLD; // Remove LC_REQ_DYLD bit if present
    switch (base) {
        case 0x1: return "LC_SEGMENT — segment of this file to be mapped";
        case 0x2: return "LC_SYMTAB — link-edit stab symbol table info";
        case 0x3: return "LC_SYMSEG — link-edit gdb symbol table info (obsolete)";
        case 0x4: return "LC_THREAD — thread state";
        case 0x5: return "LC_UNIXTHREAD — unix thread (includes a stack)";
        case 0x6: return "LC_LOADFVMLIB — load fixed VM shared library";
        case 0x7: return "LC_IDFVMLIB — fixed VM shared library identification";
        case 0x8: return "LC_IDENT — object identification info (obsolete)";
        case 0x9: return "LC_FVMFILE — fixed VM file inclusion (internal use)";
        case 0xa: return "LC_PREPAGE — prepage command (internal use)";
        case 0xb: return "LC_DYSYMTAB — dynamic link-edit symbol table info";
        case 0xc: return "LC_LOAD_DYLIB — load a dynamically linked shared library";
        case 0xd: return "LC_ID_DYLIB — dynamically linked shared lib ident";
        case 0xe: return "LC_LOAD_DYLINKER — load a dynamic linker";
        case 0xf: return "LC_ID_DYLINKER — dynamic linker identification";
        case 0x10: return "LC_PREBOUND_DYLIB — prebound dynamically linked library";
        case 0x11: return "LC_ROUTINES — image routines";
        case 0x12: return "LC_SUB_FRAMEWORK — sub framework";
        case 0x13: return "LC_SUB_UMBRELLA — sub umbrella";
        case 0x14: return "LC_SUB_CLIENT — sub client";
        case 0x15: return "LC_SUB_LIBRARY — sub library";
        case 0x16: return "LC_TWOLEVEL_HINTS — two-level namespace lookup hints";
        case 0x17: return "LC_PREBIND_CKSUM — prebind checksum";
        case 0x18: return "LC_LOAD_WEAK_DYLIB — load weak dylib (may be missing)";
        case 0x19: return "LC_SEGMENT_64 — 64-bit segment";
        case 0x1a: return "LC_ROUTINES_64 — 64-bit image routines";
        case 0x1b: return "LC_UUID — unique build UUID";
        case 0x1c: return "LC_RPATH — runtime search path (requires dyld)";
        case 0x1d: return "LC_CODE_SIGNATURE — code signature data";
        case 0x1e: return "LC_SEGMENT_SPLIT_INFO — segment split info";
        case 0x1f: return "LC_REEXPORT_DYLIB — re-exported dylib (requires dyld)";
        case 0x20: return "LC_LAZY_LOAD_DYLIB — delay load dylib until use";
        case 0x21: return "LC_ENCRYPTION_INFO — encrypted segment info";
        case 0x22: return "LC_DYLD_INFO — compressed dyld info";
        case 0x23: return "LC_LOAD_UPWARD_DYLIB — upward dylib (requires dyld)";
        case 0x24: return "LC_VERSION_MIN_MACOSX — minimum macOS version";
        case 0x25: return "LC_VERSION_MIN_IPHONEOS — minimum iOS version";
        case 0x26: return "LC_FUNCTION_STARTS — table of function starts";
        case 0x27: return "LC_DYLD_ENVIRONMENT — dyld environment variable";
        case 0x28: return "LC_MAIN — entry point (requires dyld)";
        case 0x29: return "LC_DATA_IN_CODE — non-instruction regions";
        case 0x2A: return "LC_SOURCE_VERSION — source version";
        case 0x2B: return "LC_DYLIB_CODE_SIGN_DRS — dylib code signing DRs";
        case 0x2C: return "LC_ENCRYPTION_INFO_64 — 64-bit encrypted segment";
        case 0x2D: return "LC_LINKER_OPTION — linker options";
        case 0x2E: return "LC_LINKER_OPTIMIZATION_HINT — optimization hints";
        case 0x2F: return "LC_VERSION_MIN_TVOS — minimum tvOS version";
        case 0x30: return "LC_VERSION_MIN_WATCHOS — minimum watchOS version";
        case 0x31: return "LC_NOTE — arbitrary Mach-O note";
        case 0x32: return "LC_BUILD_VERSION — build version info";
        case 0x33: return "LC_DYLD_EXPORTS_TRIE — dyld exports trie";
        case 0x34: return "LC_DYLD_CHAINED_FIXUPS — dyld chained fixups";
        case 0x35: return "LC_FILESET_ENTRY — fileset entry";
        default: return "Unknown or reserved load command";
    }
}

void print_load_command_info(uint32_t cmd) 
{
    printf(COLOR_GREEN "Command:" COLOR_RESET " 0x%08x\n", cmd);
    printf(COLOR_GREEN "Type Name: "COLOR_RESET"%s\n", get_load_command_name(cmd));
    if (cmd & LC_REQ_DYLD)
        printf("Note: This command requires dyld support (LC_REQ_DYLD set)\n");
}

void print_segment_flags(uint32_t flags)
{
    int x = 4;
    if (flags & SG_HIGHVM)
        printf("%*s- SG_HIGHVM: File contents map to high VM space; low part is zero-filled.\n", x, "");

    if (flags & SG_FVMLIB)
        printf("%*s- SG_FVMLIB: Segment allocated by a fixed VM library (used for overlap checking).\n", x, "");

    if (flags & SG_NORELOC)
        printf("%*s- SG_NORELOC: Segment has no relocations; safe to replace without relocation.\n", x, "");

    if (flags & SG_PROTECTED_VERSION_1)
        printf("%*s- SG_PROTECTED_VERSION_1: Protected segment; only first page may be unprotected.\n", x, "");

    if (flags & SG_READ_ONLY)
        printf("%*s- SG_READ_ONLY: Segment is made read-only after fixups.\n", x, "");

    if (!(flags & (SG_HIGHVM | SG_FVMLIB | SG_NORELOC | SG_PROTECTED_VERSION_1 | SG_READ_ONLY)))
        printf("%*s- No special segment flags set.\n", x, "");
}

void print_macho_segment64_metadata(segment_command_64  *seg_cmd)
{
    printf(COLOR_GREEN "Segment Command (LC_SEGMENT_64):" COLOR_RESET "\n");
    printf(COLOR_GREEN "  Segment Name:  " COLOR_RESET "%s\n", seg_cmd->segname);
    printf(COLOR_GREEN "  VM Address:    " COLOR_RESET "0x%lx\n", seg_cmd->vmaddr);
    printf(COLOR_GREEN "  VM Size:       " COLOR_RESET "0x%lx\n", seg_cmd->vmsize);
    printf(COLOR_GREEN "  File Offset:   " COLOR_RESET "%lu\n", seg_cmd->fileoff);
    printf(COLOR_GREEN "  File Size:     " COLOR_RESET "%lu\n", seg_cmd->filesize);
    printf(COLOR_GREEN "  Max Prot:      " COLOR_RESET "0x%x\n", seg_cmd->maxprot);
    printf(COLOR_GREEN "  Init Prot:     " COLOR_RESET "0x%x\n", seg_cmd->initprot);
    printf(COLOR_GREEN "  Num Sections:  " COLOR_RESET "%u\n", seg_cmd->nsects);
    printf(COLOR_GREEN "  Flags:         " COLOR_RESET "0x%x\n", seg_cmd->flags);
    print_segment_flags(seg_cmd->flags);
}

void print_macho_section64_metadata(section_64* sec_cmd)
{
    printf(COLOR_GREEN "  Section name:"COLOR_RESET" %s\n", sec_cmd->sectname);
    printf(COLOR_GREEN "  Segment name:"COLOR_RESET" %s\n" , sec_cmd->segname);
    printf(COLOR_GREEN "  Memory Address:"COLOR_RESET" 0x%lx\n", sec_cmd->addr);
    printf(COLOR_GREEN "  File Offset:"COLOR_RESET" 0x%x\n", sec_cmd->offset);
    printf(COLOR_GREEN "  Size:"COLOR_RESET" %ld\n", sec_cmd->size);
    printf(COLOR_GREEN "  Section Alignment(x**2):"COLOR_RESET" %d\n", sec_cmd->align);
    printf(COLOR_GREEN "  File offset relocation entries:"COLOR_RESET" 0x%x\n", sec_cmd->reloff);
    printf(COLOR_GREEN "  Number of relocation entries:"COLOR_RESET" %d\n" , sec_cmd->nreloc);
    printf(COLOR_GREEN "  Flags:"COLOR_RESET" %x\n", sec_cmd->flags);
    printf(COLOR_GREEN "  reserved for (offset or index):"COLOR_RESET" %d\n", sec_cmd->reserved1);	/* reserved (for offset or index) */
    printf(COLOR_GREEN "  reserved for (count and sizeof):"COLOR_RESET" %d\n", sec_cmd->reserved2);	/* reserved (for count or sizeof) */
    printf(COLOR_GREEN "  reserved: "COLOR_RESET"%d\n", sec_cmd->reserved3);	/* reserved */
}

const char* platform_to_string(int platform) 
{
    switch (platform) {
        case PLATFORM_MACOS: return "macOS";
        case PLATFORM_IOS: return "iOS";
        case PLATFORM_TVOS: return "tvOS";
        case PLATFORM_WATCHOS: return "watchOS";
        case PLATFORM_BRIDGEOS: return "bridgeOS";
        case PLATFORM_MACCATALYST: return "Mac Catalyst";
        case PLATFORM_IOSSIMULATOR: return "iOS Simulator";
        case PLATFORM_TVOSSIMULATOR: return "tvOS Simulator";
        case PLATFORM_WATCHOSSIMULATOR: return "watchOS Simulator";
        case PLATFORM_DRIVERKIT: return "DriverKit";
        default: return "Unknown Platform";
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

/* Helper to decode version numbers (e.g. 0x0e050000 → 14.5.0) */
void print_tool_version(uint32_t version)
{
    printf("%u.%u.%u",
           (version >> 16) & 0xffff,
           (version >> 8) & 0xff,
           version & 0xff);
}


/* Helper to decode uuid 128 bit*/
void print_uuid(const uint8_t uuid[16]) 
{
    printf("%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        uuid[0], uuid[1], uuid[2], uuid[3],
        uuid[4], uuid[5],
        uuid[6], uuid[7],
        uuid[8], uuid[9],
        uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
}

/* Helper to decode source version */
void print_source_version(uint64_t packed_version) 
{
    uint64_t A = (packed_version >> 40) & 0xFFFFFF;  // top 24 bits
    uint64_t B = (packed_version >> 30) & 0x3FF;
    uint64_t C = (packed_version >> 20) & 0x3FF;
    uint64_t D = (packed_version >> 10) & 0x3FF;
    uint64_t E =  packed_version        & 0x3FF;
    printf("%lu.%lu.%lu.%lu.%lu",  A, B, C, D, E);
}



