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
    if (flags & SG_HIGHVM)
        printf(" - SG_HIGHVM: File contents map to high VM space; low part is zero-filled.\n");

    if (flags & SG_FVMLIB)
        printf(" - SG_FVMLIB: Segment allocated by a fixed VM library (used for overlap checking).\n");

    if (flags & SG_NORELOC)
        printf(" - SG_NORELOC: Segment has no relocations; safe to replace without relocation.\n");

    if (flags & SG_PROTECTED_VERSION_1)
        printf(" - SG_PROTECTED_VERSION_1: Protected segment; only first page may be unprotected.\n");

    if (flags & SG_READ_ONLY)
        printf(" - SG_READ_ONLY: Segment is made read-only after fixups.\n");

    if (!(flags & (SG_HIGHVM | SG_FVMLIB | SG_NORELOC | SG_PROTECTED_VERSION_1 | SG_READ_ONLY)))
        printf(" - No special segment flags set.\n");
}

