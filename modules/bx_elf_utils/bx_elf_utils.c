#include "bx_elf_utils.h"
/**
 * @file bx_elf_utils.c
 * @brief Implementation of ELF parsing and display utilities.
 *
 * This file provides functions for:
 * - Converting ELF machine, file, section, and program header types to strings
 * - Formatting ELF program header flags for display
 * - Printing metadata for ELF section headers and program headers (32-bit and 64-bit)
 * - Displaying section or segment bytes in hex and ASCII
 * - Disassembling executable sections using the UDis86 library
 * - Printing legends for ELF section and program header types and flags
 *
 * It supports both 32-bit and 64-bit ELF binaries and uses ANSI color codes
 * to enhance terminal output readability.
 */

/**
 * @brief Print legends for ELF section header types and flags with color highlighting.
 *
 * This function prints two tables to the console:
 * 1. Section Header Types Legend: Describes common `sh_type` values with colors.
 * 2. Section Header Flags Legend: Describes common `sh_flags` values with colors.
 *
 * Each entry is highlighted using ANSI color codes for better readability.
 *
 * @note Uses the `legend_entry` struct:
 * @code
 * typedef struct {
 *     const char *name;   // Short name or flag
 *     const char *desc;   // Description of the type or flag
 *     const char *color;  // ANSI color code
 * } legend_entry;
 * @endcode
 *
 * Example output (truncated):
 * @code
 * === Section Header Types Legend ===
 * +---------------------+------------------------------------+
 * | Type                | Description                        |
 * +---------------------+------------------------------------+
 * | NULL                | SHT_NULL: Unused section           |
 * | PROGBITS            | SHT_PROGBITS: Program-defined data|
 * ...
 * === Section Header Flags Legend ===
 * +------+-------------------------------------------+
 * | Flag | Description                               |
 * +------+-------------------------------------------+
 * | W    | SHF_WRITE: Writable                        |
 * | A    | SHF_ALLOC: Occupies memory                 |
 * ...
 * @endcode
 *
 * @note Uses ANSI color codes (COLOR_RED, COLOR_GREEN, COLOR_YELLOW, COLOR_CYAN, COLOR_MAGENTA, COLOR_WHITE, COLOR_GRAY, COLOR_RESET).
 */
void print_section_header_legend(void)
{
    legend_entry types[] = {
        {"NULL",          "SHT_NULL: Unused section",              COLOR_RED},
        {"PROGBITS",      "SHT_PROGBITS: Program-defined data",    COLOR_CYAN},
        {"SYMTAB",        "SHT_SYMTAB: Symbol table",              COLOR_MAGENTA},
        {"STRTAB",        "SHT_STRTAB: String table",              COLOR_GREEN},
        {"RELA",          "SHT_RELA: Relocation with addends",     COLOR_YELLOW},
        {"HASH",          "SHT_HASH: Symbol hash table",           COLOR_BLUE},
        {"DYNAMIC",       "SHT_DYNAMIC: Dynamic linking info",     COLOR_CYAN},
        {"NOTE",          "SHT_NOTE: Auxiliary information",       COLOR_MAGENTA},
        {"NOBITS",        "SHT_NOBITS: Occupies no file space",    COLOR_GRAY},
        {"REL",           "SHT_REL: Relocation without addends",   COLOR_WHITE},
        {"SHLIB",         "SHT_SHLIB: Reserved",                   COLOR_RED},
        {"DYNSYM",        "SHT_DYNSYM: Dynamic symbol table",      COLOR_GREEN},
        {"INIT_ARRAY",    "SHT_INIT_ARRAY: Constructors array",    COLOR_YELLOW},
        {"FINI_ARRAY",    "SHT_FINI_ARRAY: Destructors array",     COLOR_BLUE},
        {"PREINIT_ARRAY", "SHT_PREINIT_ARRAY: Pre-initializers",   COLOR_CYAN},
        {"GROUP",         "SHT_GROUP: Section group",              COLOR_MAGENTA},
        {"SYMTAB_SHNDX",  "SHT_SYMTAB_SHNDX: Extended indices",    COLOR_WHITE}
    };

    printf(COLOR_YELLOW "=== Section Header Types Legend ===\n" COLOR_RESET);
    printf(COLOR_WHITE "+---------------------+------------------------------------+\n" COLOR_RESET);
    printf(COLOR_WHITE "| Type                | Description                        |\n" COLOR_RESET);
    printf(COLOR_WHITE "+---------------------+------------------------------------+\n" COLOR_RESET);

    for (size_t i = 0; i < sizeof(types)/sizeof(types[0]); i++) {
        printf("| %s%-19s%s | %s%s%s%*s|\n",
               types[i].color, types[i].name, COLOR_RESET,
               types[i].color, types[i].desc, COLOR_RESET,
               (int)(35 - strlen(types[i].desc)), "");  
    }
    printf(COLOR_WHITE "+---------------------+------------------------------------+\n\n" COLOR_RESET);

    legend_entry flags[] = {
        {"W", "SHF_WRITE: Writable", COLOR_RED},
        {"A", "SHF_ALLOC: Occupies memory", COLOR_GREEN},
        {"X", "SHF_EXECINSTR: Executable code", COLOR_YELLOW},
        {"M", "SHF_MERGE: Might be merged", COLOR_CYAN},
        {"S", "SHF_STRINGS: Contains strings", COLOR_MAGENTA},
        {"I", "SHF_INFO_LINK: sh_info has special meaning", COLOR_WHITE},
        {"L", "SHF_LINK_ORDER: Link order", COLOR_BLUE},
        {"O", "SHF_OS_NONCONFORMING: OS specific", COLOR_GRAY},
        {"G", "SHF_GROUP: Section group", COLOR_CYAN},
        {"T", "SHF_TLS: Thread-Local Storage", COLOR_YELLOW}
    };

    printf(COLOR_YELLOW "=== Section Header Flags Legend ===\n" COLOR_RESET);
    printf(COLOR_WHITE "+------+-------------------------------------------+\n" COLOR_RESET);
    printf(COLOR_WHITE "| Flag | Description                               |\n" COLOR_RESET);
    printf(COLOR_WHITE "+------+-------------------------------------------+\n" COLOR_RESET);

    for (size_t i = 0; i < sizeof(flags)/sizeof(flags[0]); i++) {
        printf("| %s%-4s%s | %s%s%s%*s|\n",
               flags[i].color, flags[i].name, COLOR_RESET,
               flags[i].color, flags[i].desc, COLOR_RESET,
               (int)(42 - strlen(flags[i].desc)), "");  
    }
    printf(COLOR_WHITE "+------+-------------------------------------------+\n\n" COLOR_RESET);
}

/**
 * @brief Print legends for ELF program header types and flags with color highlighting.
 *
 * This function prints two tables to the console:
 * 1. Program Header Types Legend: Describes common `p_type` values with colors.
 * 2. Program Header Flags Legend: Describes common `p_flags` values with colors.
 *
 * Each entry is highlighted using ANSI color codes for better readability.
 *
 * @note Uses the `legend_entry` struct:
 * @code
 * typedef struct {
 *     const char *name;   // Short name or flag
 *     const char *desc;   // Description of the type or flag
 *     const char *color;  // ANSI color code
 * } legend_entry;
 * @endcode
 *
 * Example output (truncated):
 * @code
 * === Program Header Types Legend ===
 * +------------+------------------------------------------+
 * | Type       | Description                              |
 * +------------+------------------------------------------+
 * | NULL       | PT_NULL: Unused entry                     |
 * | LOAD       | PT_LOAD: Loadable segment                 |
 * ...
 * === Program Header Flags Legend ===
 * +-----+-----------------------------------+
 * | Flag| Description                       |
 * +-----+-----------------------------------+
 * | R   | PF_R: Readable                     |
 * | W   | PF_W: Writable                     |
 * | X   | PF_X: Executable                   |
 * ...
 * @endcode
 *
 * @note Uses ANSI color codes (COLOR_RED, COLOR_GREEN, COLOR_YELLOW, COLOR_CYAN, COLOR_MAGENTA, COLOR_WHITE, COLOR_RESET).
 */
void print_program_header_legend(void)
{
    legend_entry types[] = {
        {"NULL",      "PT_NULL: Unused entry", COLOR_RED},
        {"LOAD",      "PT_LOAD: Loadable segment", COLOR_CYAN},
        {"DYNAMIC",   "PT_DYNAMIC: Dynamic linking info", COLOR_MAGENTA},
        {"INTERP",    "PT_INTERP: Interpreter path", COLOR_GREEN},
        {"NOTE",      "PT_NOTE: Auxiliary information", COLOR_BLUE},
        {"SHLIB",     "PT_SHLIB: Reserved", COLOR_WHITE},
        {"PHDR",      "PT_PHDR: Program header table itself", COLOR_YELLOW},
        {"TLS",       "PT_TLS: Thread-Local Storage template", COLOR_CYAN},
        {"GNU_STACK", "PT_GNU_STACK: Stack flags", COLOR_MAGENTA},
        {"GNU_RELRO", "PT_GNU_RELRO: Read-only after relocation", COLOR_GREEN}
    };

    printf(COLOR_YELLOW "=== Program Header Types Legend ===\n" COLOR_RESET);
    printf(COLOR_WHITE "+------------+------------------------------------------+\n" COLOR_RESET);
    printf(COLOR_WHITE "| Type       | Description                              |\n" COLOR_RESET);
    printf(COLOR_WHITE "+------------+------------------------------------------+\n" COLOR_RESET);
    for (size_t i = 0; i < sizeof(types)/sizeof(types[0]); i++) {
        printf("| %s%-10s%s | %s%-40s%s|\n",
               types[i].color, types[i].name, COLOR_RESET,
               types[i].color, types[i].desc, COLOR_RESET);
    }
    printf(COLOR_WHITE "+------------+------------------------------------------+\n\n" COLOR_RESET);

    legend_entry flags[] = {
        {"R", "PF_R: Readable", COLOR_GREEN},
        {"W", "PF_W: Writable", COLOR_RED},
        {"X", "PF_X: Executable", COLOR_YELLOW}
    };
    printf(COLOR_YELLOW "=== Program Header Flags Legend ===\n" COLOR_RESET);
    printf(COLOR_WHITE "+-----+-----------------------------------+\n" COLOR_RESET);
    printf(COLOR_WHITE "| Flag| Description                       |\n" COLOR_RESET);
    printf(COLOR_WHITE "+-----+-----------------------------------+\n" COLOR_RESET);
    for (size_t i = 0; i < sizeof(flags)/sizeof(flags[0]); i++) {
        printf("| %s%-3s%s | %s%-33s%s|\n",
               flags[i].color, flags[i].name, COLOR_RESET,
               flags[i].color, flags[i].desc, COLOR_RESET);
    }
    printf(COLOR_WHITE "+-----+-----------------------------------+\n\n" COLOR_RESET);
}

/**
 * @brief Convert ELF e_machine value to human-readable string.
 *
 * This function takes an ELF machine type (e_machine) from the ELF header
 * and returns a descriptive string representing the target architecture.
 *
 * @param machine The e_machine field from the ELF header.
 * @return const char* Human-readable string describing the architecture.
 *
 * @note Returns "Unknown/Unsupported machine" for unrecognized values.
 *
 * Example:
 * @code
 * const char* arch = elf_machine_to_str(EM_X86_64);
 * printf("Architecture: %s\n", arch); // Output: Architecture: AMD x86-64
 * @endcode
 */
const char* elf_machine_to_str(unsigned int machine)
{
    char* name;
    switch (machine) {
        case EM_NONE:       name = "No machine";break;
        case EM_M32:        name = "AT&T WE 32100";break;
        case EM_SPARC:      name = "SUN SPARC";break;
        case EM_386:        name = "Intel 80386";break;
        case EM_68K:        name = "Motorola 68000";break;
        case EM_88K:        name = "Motorola 88000";break;
        case EM_IAMCU:      name = "Intel MCU";break;
        case EM_860:        name = "Intel 80860";break;
        case EM_MIPS:       name = "MIPS R3000 (big-endian)";break;
        case EM_S370:       name = "IBM System/370";break;
        case EM_MIPS_RS3_LE:name = "MIPS R3000 (little-endian)";break;
        case EM_PARISC:     name = "HPPA";break;
        case EM_VPP500:     name = "Fujitsu VPP500";break;
        case EM_SPARC32PLUS:name = "Sun SPARC v8+";break;
        case EM_960:        name = "Intel 80960";break;
        case EM_PPC:        name = "PowerPC";break;
        case EM_PPC64:      name = "PowerPC 64-bit";break;
        case EM_S390:       name = "IBM S/390";break;
        case EM_ARM:        name = "ARM";break;
        case EM_SH:         name = "Hitachi SH";break;
        case EM_SPARCV9:    name = "SPARC v9 (64-bit)";break;
        case EM_IA_64:      name = "Intel Itanium (IA-64)";break;
        case EM_X86_64:     name = "AMD x86-64";break;
        case EM_AARCH64:    name = "ARM AArch64";break;
        case EM_RISCV:      name = "RISC-V";break;
        case EM_BPF:        name = "Linux BPF VM";break;
        case EM_LOONGARCH:  name = "LoongArch";break;
        case EM_MSP430:     name = "TI MSP430";break;
        case EM_BLACKFIN:   name = "Analog Devices Blackfin";break;
        case EM_AVR:        name = "Atmel AVR 8-bit";break;
        case EM_CRIS:       name = "Axis CRIS";break;
        case EM_TILEGX:     name = "Tilera TILE-Gx";break;
        case EM_MICROBLAZE: name = "Xilinx MicroBlaze";break;
        case EM_CUDA:       name = "NVIDIA CUDA";break;
        case EM_AMDGPU:     name = "AMD GPU";break;
        case EM_RL78:       name = "Renesas RL78";break;
        case EM_OPENRISC:   name = "OpenRISC";break;
        case EM_XTENSA:     name = "Tensilica Xtensa";break;
        case EM_Z80:        name = "Zilog Z80";break;
        case EM_VAX:        name = "DEC VAX";break;
        case EM_M32R:       name = "Mitsubishi M32R";break;
        case EM_MN10300:    name = "Matsushita MN10300";break;
        case EM_MN10200:    name = "Matsushita MN10200";break;
        case EM_PJ:         name = "picoJava";break;
        default:            name = "Unknown/Unsupported machine";
    }
    return name;
}

/**
 * @brief Converts an ELF section header type to a human-readable string with color.
 *
 * This function maps ELF section header type constants (SHT_*) to
 * descriptive strings. ANSI color codes are added for colored output
 * in terminal.
 *
 * @param sh_type The section header type (e.g., SHT_PROGBITS, SHT_SYMTAB, ...).
 * @return const char* A pointer to a static string representing the type.
 *         The string includes ANSI color codes for terminal output.
 *
 * @note The returned string should not be freed, as it points to static memory.
 * @note Supports standard, GNU, and SUNW section types, as well as OS/Processor/Application-specific ranges.
 */
const char* sh_type_to_str(unsigned int sh_type)
{
    char* result;
    switch (sh_type) {
        case SHT_NULL:          result= COLOR_RED "NULL" COLOR_RESET; break;
        case SHT_PROGBITS:      result= COLOR_CYAN "PROGBITS" COLOR_RESET; break;
        case SHT_SYMTAB:        result= COLOR_MAGENTA "SYMTAB" COLOR_RESET; break;
        case SHT_STRTAB:        result= COLOR_GREEN "STRTAB" COLOR_RESET; break;
        case SHT_RELA:          result= COLOR_YELLOW "RELA" COLOR_RESET; break;
        case SHT_HASH:          result= COLOR_BLUE "HASH" COLOR_RESET; break;
        case SHT_DYNAMIC:       result= COLOR_CYAN "DYNAMIC" COLOR_RESET; break;
        case SHT_NOTE:          result= COLOR_MAGENTA "NOTE" COLOR_RESET; break;
        case SHT_NOBITS:        result= COLOR_GRAY "NOBITS" COLOR_RESET; break;
        case SHT_REL:           result= COLOR_WHITE "REL" COLOR_RESET; break;
        case SHT_SHLIB:         result= COLOR_RED "SHLIB" COLOR_RESET; break;
        case SHT_DYNSYM:        result= COLOR_GREEN "DYNSYM" COLOR_RESET; break;
        case SHT_INIT_ARRAY:    result= COLOR_YELLOW "INIT_ARRAY" COLOR_RESET; break;
        case SHT_FINI_ARRAY:    result= COLOR_BLUE "FINI_ARRAY" COLOR_RESET; break;
        case SHT_PREINIT_ARRAY: result= COLOR_CYAN "PREINIT_ARRAY" COLOR_RESET; break;
        case SHT_GROUP:         result= COLOR_MAGENTA "GROUP" COLOR_RESET; break;
        case SHT_SYMTAB_SHNDX:  result= COLOR_WHITE "SYMTAB_SHNDX" COLOR_RESET; break;
        case SHT_RELR:          result= COLOR_YELLOW "RELR" COLOR_RESET; break;

        case SHT_GNU_ATTRIBUTES: result = "GNU_ATTRIBUTES"; break;
        case SHT_GNU_HASH:       result = "GNU_HASH"; break;
        case SHT_GNU_LIBLIST:    result = "GNU_LIBLIST"; break;
        case SHT_CHECKSUM:       result = "CHECKSUM"; break;
        case SHT_GNU_verdef:     result = "GNU_verdef"; break;
        case SHT_GNU_verneed:    result = "GNU_verneed"; break;
        case SHT_GNU_versym:     result = "GNU_versym"; break;

        case SHT_SUNW_move:      result = "SUNW_move"; break;
        case SHT_SUNW_COMDAT:    result = "SUNW_COMDAT"; break;
        case SHT_SUNW_syminfo:   result = "SUNW_syminfo"; break;

        default:
             if (sh_type >= SHT_LOOS && sh_type <= SHT_HIOS)
                 result = "OS-specific";
             else if (sh_type >= SHT_LOPROC && sh_type <= SHT_HIPROC)
                 result = "Processor-specific";
             else if (sh_type >= SHT_LOUSER && sh_type <= SHT_HIUSER)
                 result = "Application-specific";
             else
                 result = "UNKNOWN";
             break;
    }
    return result;
}

/**
 * @brief Converts an ELF file type to a human-readable string.
 *
 * This function maps ELF file type constants (ET_*) to descriptive strings.
 * Examples include relocatable files, executables, shared objects, and core files.
 *
 * @param type The ELF file type (e.g., ET_REL, ET_EXEC, ET_DYN, etc.).
 * @return const char* A pointer to a string describing the ELF file type.
 *
 * @note The returned string points to static memory and must not be freed.
 * @note Supports standard ELF types, OS-specific, and processor-specific ranges.
 */
const char* elf_type_to_str(unsigned int type)
{
    char *result;
    switch (type) {
        case ET_NONE:   result = "No file type"; break;
        case ET_REL:    result = "Relocatable file"; break;
        case ET_EXEC:   result = "Executable file"; break;
        case ET_DYN:    result = "Shared object file"; break;
        case ET_CORE:   result = "Core file"; break;
        case ET_NUM:    result = "Number of defined types"; break;
        default:
            if (type >= ET_LOOS && type <= ET_HIOS)
                result = "OS-specific file type";
            if (type >= ET_LOPROC && type <= ET_HIPROC)
                result = "Processor-specific file type";
            result = "Unknown file type";
    }
    return result;
}

/**
 * @brief Converts a program header type (p_type) to a human-readable string with color.
 *
 * This function maps ELF program header type constants (PT_*) to descriptive strings,
 * optionally including ANSI color codes for terminal highlighting.
 *
 * @param p_type The program header type (e.g., PT_LOAD, PT_DYNAMIC, PT_INTERP, etc.).
 * @return const char* A pointer to a string describing the program header type.
 *
 * @note The returned string points to static memory and must not be freed.
 * @note Supports standard ELF program header types, OS-specific, and processor-specific ranges.
 * @note Some types include ANSI color codes for terminal output.
 */
const char *type_p_to_str(unsigned int p_type)
{
    const char *type_str;
    switch (p_type) {
        case PT_NULL:         type_str = COLOR_RED     "NULL"        COLOR_RESET; break;
        case PT_LOAD:         type_str = COLOR_CYAN    "LOAD"        COLOR_RESET; break;
        case PT_DYNAMIC:      type_str = COLOR_MAGENTA "DYNAMIC"     COLOR_RESET; break;
        case PT_INTERP:       type_str = COLOR_GREEN   "INTERP"      COLOR_RESET; break;
        case PT_NOTE:         type_str = COLOR_BLUE    "NOTE"        COLOR_RESET; break;
        case PT_SHLIB:        type_str = COLOR_WHITE   "SHLIB"       COLOR_RESET; break;
        case PT_PHDR:         type_str = COLOR_YELLOW  "PHDR"        COLOR_RESET; break;
        case PT_TLS:          type_str = COLOR_CYAN    "TLS"         COLOR_RESET; break;
        case PT_GNU_STACK:    type_str = COLOR_MAGENTA "GNU_STACK"   COLOR_RESET; break;
        case PT_GNU_RELRO:    type_str = COLOR_GREEN   "GNU_RELRO"   COLOR_RESET; break;

        case PT_GNU_EH_FRAME: type_str = "GNU_EH_FRAME"; break;
        case PT_GNU_PROPERTY: type_str = "GNU_PROPERTY"; break;
        case PT_SUNWBSS:      type_str = "SUNWBSS"; break;
        case PT_SUNWSTACK:    type_str = "SUNWSTACK"; break;
        default:
            if (p_type >= PT_LOOS && p_type <= PT_HIOS)
                type_str = COLOR_MAGENTA "OS-SPECIFIC" COLOR_RESET;
            else if (p_type >= PT_LOPROC && p_type <= PT_HIPROC)
                type_str = COLOR_YELLOW  "PROC-SPECIFIC" COLOR_RESET;
            else
                type_str = COLOR_GRAY    "UNKNOWN" COLOR_RESET;
            break;
    }
    return type_str;
}

                                      
// ================= Categories =================
static const char *data_mov[] = {
    "mov","movsx","movzx","lea","push","pop","xchg", NULL
};

static const char* invalid[] = {
    "invalid", NULL
};

static const char *arithmetic[] = {
    "add","sub","inc","dec","imul","mul","idiv","div","neg","adc","sbb", "cmp", NULL
};

static const char *logic_ops[] = {
    "and","or","xor","not","test","shl","shr","sar","sal",
    "rol","ror","rcl","rcr","bt","bts","btr","btc",NULL
};

static const char *jumps[] = {
    "jmp","je","jne","jz","jnz","ja","jae","jb","jbe",
    "jg","jge","jl","jle","jc","jnc","jo","jno","js","jns",
    "jp","jnp","jcxz","jecxz","loop","loope","loopne",
    "call","ret","enter","leave",NULL
};

static const char *string_ops[] = {
    "movsb","movsw","movsd","movsq","cmpsb","cmpsw","cmpsd","cmpsq",
    "scasb","scasw","scasd","scasq","lodsb","lodsw","lodsd","lodsq",
    "stosb","stosw","stosd","stosq","rep","repe","repne",NULL
};

static const char *system_ops[] = {
    "int","int3","into","iret","syscall","sysret","sysenter","sysexit",
    "nop","hlt","wait","stc","clc","cmc","std","cld","sti","cli",
    "cpuid","rdtsc","rsm",NULL
};

static const char *asm_types[] = {
    // ----- Data definition -----
    "db",   // define byte
    "dw",   // define word (16-bit)
    "dd",   // define double word (32-bit)
    "dq",   // define quad word (64-bit)
    "dt",   // define ten bytes (80-bit, FPU)
    "do",   // define octa word (128-bit)
    "dy",   // define yword (256-bit)
    "dz",   // define zword (512-bit)

    // ----- Reservation -----
    "resb", // reserve bytes
    "resw", // reserve words
    "resd", // reserve double words
    "resq", // reserve quad words
    "rest", // reserve ten bytes
    "reso", // reserve 128-bit
    "resy", // reserve 256-bit
    "resz", // reserve 512-bit

    // ----- Size specifiers -----
    "byte",
    "word",
    "dword",
    "qword",
    "tword",  // 80-bit
    "oword",  // 128-bit
    "yword",  // 256-bit
    "zword",  // 512-bit

    // ----- Instruction prefixes -----
    "lock",
    "rep",
    "repe", "repz",
    "repne", "repnz",
    "o16",   // operand-size override (0x66)
    "a16",   // address-size override (0x67)
    "a32",
    "a64",
    "rex", "rexw", "rexr", "rexx", "rexb",

    // ----- Segment overrides -----
    "cs", "ds", "ss", "es", "fs", "gs",

    // ----- Directives -----
    "equ",     // constant definition
    "org",     // origin
    "align",   // alignment
    "section", "segment",
    "global", "extern",

    // ----- Special keywords -----
    "short",
    "near",
    "far",

    NULL
};

static const char *asm_registers[] = {
    // 64-bit
    "rax","rbx","rcx","rdx",
    "rsi","rdi","rbp","rsp",
    "r8","r9","r10","r11","r12","r13","r14","r15",
    "rip",

    // 32-bit
    "eax","ebx","ecx","edx",
    "esi","edi","ebp","esp",
    "r8d","r9d","r10d","r11d","r12d","r13d","r14d","r15d",

    // 16-bit
    "ax","bx","cx","dx",
    "si","di","bp","sp",
    "r8w","r9w","r10w","r11w","r12w","r13w","r14w","r15w",

    // 8-bit (low)
    "al","bl","cl","dl",
    "sil","dil","bpl","spl",
    "r8b","r9b","r10b","r11b","r12b","r13b","r14b","r15b",

    // 8-bit (high) - legacy only
    "ah","bh","ch","dh",

    NULL
};

/**
 * @brief Check if a word exists in a null-terminated string list.
 *
 * This function searches for an exact match of the given word
 * in the provided list of strings. The list must be null-terminated.
 *
 * @param word The null-terminated string to search for.
 * @param list A null-terminated array of strings.
 * @return Returns 1 if the word is found in the list, 0 otherwise.
 *
 * @note Comparison is case-sensitive and uses strcmp().
 *       Typically used to check if a token belongs to categories
 *       like registers, opcodes, instruction types, etc.
 */
int is_in_list(const char *word, const char *list[]) {
    for (int i=0; list[i]; i++)
        if (strcmp(word, list[i]) == 0) return 1;
    return 0;
}

/**
 * @brief Check if a string represents a numeric value.
 *
 * This function determines whether the given string `word` is a number.
 * It supports:
 *   - Hexadecimal numbers starting with "0x" or "0X"
 *   - Decimal numbers (only digits)
 *
 * @param word A null-terminated string to check.
 * @return Returns 1 if the string is a valid number (hex or decimal), 0 otherwise.
 *
 * @note Hexadecimal detection only checks the "0x" prefix; it does not
 *       validate that the remaining characters are valid hex digits.
 *       Decimal numbers are checked using isdigit().
 */
int is_number(const char *word) {
    // Hexadecimal like 0x123
    if (strlen(word) > 2 && word[0]=='0' && (word[1]=='x' || word[1]=='X'))
        return 1;
    // Decimal like 1234
    for (int i=0; word[i]; i++) {
        if (!isdigit((unsigned char)word[i])) return 0;
    }
    return (word[0] != '\0'); // not empty
}

/**
 * @brief Get the ANSI color code for a given assembly token.
 *
 * This function determines the color that should be used to print
 * a single word/token from an assembly instruction or line.
 * It checks the token against several categories such as numbers,
 * registers, opcodes, instruction types, and invalid/unknown tokens.
 *
 * The color codes are defined by macros like COLOR_RED, COLOR_GREEN, etc.
 *
 * @param word A null-terminated string representing the token to check.
 * @return A string containing the ANSI color code to use for this token.
 *         Returns COLOR_RESET if the token does not match any known category.
 *
 * @note The helper functions used include:
 *       - ::is_number() : returns true if the token is a numeric value.
 *       - ::is_in_list() : checks if the token exists in a given list of strings.
 *       - Token lists used:
 *           - data_mov     : data movement instructions (mov, lea, etc.)
 *           - arithmetic  : add, sub, mul, etc.
 *           - logic_ops   : and, or, xor, etc.
 *           - jumps       : jmp, je, jne, call, ret, etc.
 *           - string_ops  : rep movsb, stosb, etc.
 *           - system_ops  : syscall, int, etc.
 *           - asm_registers : all CPU registers (rax, rbx, r8d, al, etc.)
 *           - asm_types   : db, dw, qword ptr, o16, etc.
 *           - invalid     : invalid/unknown tokens
 */
const char* get_color(const char *word) {
    if (is_number(word)) return COLOR_YELLOW;
    if (is_in_list(word, data_mov)) return COLOR_RED;
    if (is_in_list(word, arithmetic)) return COLOR_GREEN;
    if (is_in_list(word, logic_ops)) return COLOR_YELLOW;
    if (is_in_list(word, jumps)) return COLOR_CYAN;
    if (is_in_list(word, string_ops)) return COLOR_MAGENTA;
    if (is_in_list(word, system_ops)) return COLOR_BLUE;
    if (is_in_list(word, asm_registers)) return COLOR_CYAN;
    if (is_in_list(word, asm_types )) return COLOR_WHITE;
    if(is_in_list(word, invalid)) return COLOR_GRAY;
    return COLOR_RESET;
}

/**
 * @brief Check if a character is considered punctuation in ASM parsing.
 *
 * This function determines whether the given character `c` is a punctuation
 * character commonly used in assembly code. Punctuation includes braces,
 * parentheses, commas, semicolons, arithmetic operators, brackets, and colon.
 *
 * @param c The character to check.
 * @return Non-zero (true) if the character is punctuation, zero (false) otherwise.
 *
 * @note This function is used by tokenizers like ::print_highlight_asm
 *       to split words while preserving punctuation for output.
 */
static int is_punctuation(char c) { 
    return c == '{' || c == '}' || c == '(' || c == ')' || c == ';' || c == ',' || 
           c == '=' || c == '*' || c == '&' || c == '[' || c == ']' || c == '-' || c == '+' || c == ':'; 
}

/**
 * @brief Print an assembly instruction line with syntax highlighting.
 *
 * This function scans through a single line of assembly code
 * and applies syntax highlighting (using ANSI color codes)
 * to recognized tokens such as opcodes, registers, numbers, etc.
 * Highlighting is determined by the helper function ::get_color().
 *
 * Tokens are split based on whitespace and punctuation characters,
 * but punctuation itself (commas, brackets, colons, etc.) is preserved
 * and printed without coloring.
 *
 * Example:
 * @code
 * Input : "mov rax, [rbx+0x10]"
 * Output: mov (red) rax (cyan) , [ rbx (cyan) + 0x10 (yellow) ]
 * @endcode
 *
 * @param line A null-terminated string containing the assembly
 *             instruction line to highlight. If NULL, the function
 *             returns immediately.
 *
 * @note Requires that COLOR_* macros and get_color() are defined.
 *       The word buffer is currently fixed at 64 bytes.
 */
void print_highlight_asm(const char *line) {
    if (!line) return;

    const char *p = line;
    char word[64];

    while (*p) {
        if (isspace(*p) || is_punctuation(*p)) {
            putchar(*p);
            p++;
            continue;
        }

        int i=0;
        while (*p && !isspace(*p) && !is_punctuation(*p)) {
            word[i++] = *p;
            p++;
        }

        word[i] = '\0';
        const char *color = get_color(word);

        if (color != COLOR_RESET)
            printf("%s%s%s", color, word, COLOR_RESET);
        else
            printf("%s", word);

    }
    printf("\n");
}


/**
 * @brief Format ELF section header flags into a colored string.
 *
 * @param sh_flags Section header flags (bitmask).
 * @param buf Buffer to write formatted flags into.
 * @param size Size of the buffer.
 */
void format_sh_flags(uint64_t sh_flags, char *buf, size_t size)
{
    buf[0] = '\0'; // start empty
    if (sh_flags & SHF_WRITE)
        strncat(buf, COLOR_RED "W" COLOR_RESET, size - strlen(buf) - 1);
    if (sh_flags & SHF_ALLOC)
        strncat(buf, COLOR_GREEN "A" COLOR_RESET, size - strlen(buf) - 1);
    if (sh_flags & SHF_EXECINSTR)
        strncat(buf, COLOR_YELLOW "X" COLOR_RESET, size - strlen(buf) - 1);
    if (sh_flags & SHF_MERGE)
        strncat(buf, COLOR_CYAN "M" COLOR_RESET, size - strlen(buf) - 1);
    if (sh_flags & SHF_STRINGS)
        strncat(buf, COLOR_MAGENTA "S" COLOR_RESET, size - strlen(buf) - 1);
    if (sh_flags & SHF_INFO_LINK)
        strncat(buf, COLOR_WHITE "I" COLOR_RESET, size - strlen(buf) - 1);
    if (sh_flags & SHF_LINK_ORDER)
        strncat(buf, COLOR_BLUE "L" COLOR_RESET, size - strlen(buf) - 1);
    if (sh_flags & SHF_OS_NONCONFORMING)
        strncat(buf, COLOR_GRAY "O" COLOR_RESET, size - strlen(buf) - 1);
    if (sh_flags & SHF_GROUP)
        strncat(buf, COLOR_CYAN "G" COLOR_RESET, size - strlen(buf) - 1);
    if (sh_flags & SHF_TLS)
        strncat(buf, COLOR_YELLOW "T" COLOR_RESET, size - strlen(buf) - 1);

    // printf("================= Size of flag: %ld ===========\n", size - strlen(buf) - 1);
    for(int i=0; i<5; i++)
        strncat(buf, " ", size - strlen(buf) - 1);
    strncat(buf, COLOR_CYAN, size - strlen(buf) - 1);
}

/**
 * @brief Print all symbols from a 32-bit ELF file (with colors).
 *
 * This function iterates over the ELF32 symbol table (`symtab`) and its
 * associated string table (`strtab`) to display information about each symbol.
 * It prints a nicely formatted and colorized table that includes:
 * - **Index** : The symbol index in the table.
 * - **Value** : The symbol value (address in executables/shared objects,
 *               or section offset in relocatable files).
 * - **Size**  : The size of the symbol in bytes.
 * - **Type**  : The decoded symbol type (FUNC, OBJECT, SECTION, FILE, etc.).
 * - **Name**  : The symbol name resolved from the string table.
 *
 * @param parser   Pointer to the binary parser context (holds loaded ELF data in memory).
 * @param elf      Pointer to the ELF32 header structure.
 * @param shdrs    Pointer to the array of section headers in the ELF file.
 * @param symtab   Pointer to the section header for the symbol table (.symtab).
 * @param strtab   Pointer to the section header for the associated string table (.strtab).
 *
 * @note 
 * - For **relocatable ELF (.o)** files: `Value` is the symbol’s offset
 *   relative to its section.
 * - For **executables / shared objects**: `Value` is the virtual memory
 *   address of the symbol at runtime.
 * - For **undefined symbols**: `Value = 0` since the address will be resolved later.
 *
 * @warning
 * Output uses ANSI color codes, so results may look odd in non-color terminals.
 */
void print_symbols_32bit(bparser* parser, Elf32_Ehdr* elf, Elf32_Shdr* shdrs, Elf32_Shdr *symtab, Elf32_Shdr *strtab) 
{
    Elf32_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(parser->block + shstr.sh_offset);

    const char* symname = &shstrtab[symtab -> sh_name];
    const char* strname = &shstrtab[strtab -> sh_name];

    Elf32_Sym *syms = (Elf32_Sym *)(parser->block + symtab->sh_offset);
    const char *strs = (const char *)(parser->block + strtab->sh_offset);

    unsigned int count = symtab->sh_size / sizeof(Elf32_Sym);

    printf(COLOR_RESET);
    printf(COLOR_YELLOW"=== Symbols (" COLOR_BLUE "%s" COLOR_YELLOW " + " COLOR_BLUE "%s" COLOR_YELLOW ") ===\n" COLOR_RESET, symname, strname);
    printf(COLOR_WHITE "%-6s %-10s %-10s %-12s %s\n" COLOR_RESET,
           "Index", "Value", "Size", "Type", "Name");
    printf(COLOR_WHITE "------------------------------------------------------------\n" COLOR_RESET);

    for (int i = 0; i < count; i++) {
        const char *name = strs + syms[i].st_name;

        // Decode type
        unsigned char type = ELF32_ST_TYPE(syms[i].st_info);
        const char *type_str = "UNKNOWN";
        const char *color    = COLOR_GRAY; // default

        switch (type) {
            case STT_NOTYPE:   type_str = "NOTYPE";  color = COLOR_GRAY;    break;
            case STT_OBJECT:   type_str = "OBJECT";  color = COLOR_GREEN;   break;
            case STT_FUNC:     type_str = "FUNC";    color = COLOR_YELLOW;  break;
            case STT_SECTION:  type_str = "SECTION"; color = COLOR_BLUE;    break;
            case STT_FILE:     type_str = "FILE";    color = COLOR_CYAN;    break;
        }

        printf("%-6d 0x%08x %-10u %s%-12s%s %s\n",
               i,
               syms[i].st_value,
               syms[i].st_size,
               color, type_str, COLOR_RESET,
               name);
    }
}

/**
 * @brief Print all symbols from a 64-bit ELF file (with colors).
 *
 * This function iterates over the ELF64 symbol table (`symtab`) and its
 * associated string table (`strtab`) to display information about each symbol.
 * It prints a formatted and colorized table including:
 * - **Index** : The symbol index in the table.
 * - **Value** : The symbol value (for executables/shared objects, this is
 *               the virtual address at runtime; for relocatable objects,
 *               this is the section-relative offset).
 * - **Size**  : The size of the symbol in bytes.
 * - **Type**  : The decoded symbol type (FUNC, OBJECT, SECTION, FILE, etc.).
 * - **Name**  : The symbol’s name resolved from the string table.
 *
 * @param parser   Pointer to the binary parser context (holds loaded ELF data in memory).
 * @param elf      Pointer to the ELF64 header structure.
 * @param shdrs    Pointer to the array of section headers in the ELF file.
 * @param symtab   Pointer to the section header for the symbol table (.symtab).
 * @param strtab   Pointer to the section header for the associated string table (.strtab).
 *
 * @note 
 * - For **relocatable ELF (.o)**: `Value` is the offset relative to its section.
 * - For **executables / shared objects**: `Value` is the virtual memory
 *   address of the symbol.
 * - For **undefined symbols**: `Value = 0` since resolution is deferred to the linker/loader.
 *
 * @warning
 * Output uses ANSI color codes, so results may appear incorrect in terminals
 * without color support.
 */
void print_symbols_64bit(bparser* parser, Elf64_Ehdr* elf, Elf64_Shdr* shdrs, Elf64_Shdr *symtab, Elf64_Shdr *strtab) 
{
    Elf64_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(parser->block + shstr.sh_offset);

    const char* symname = &shstrtab[symtab -> sh_name];
    const char* strname = &shstrtab[strtab -> sh_name];

    Elf64_Sym *syms = (Elf64_Sym *)(parser->block + symtab->sh_offset);
    const char *strs = (const char *)(parser->block + strtab->sh_offset);

    unsigned int count = symtab->sh_size / sizeof(Elf64_Sym);

    printf(COLOR_YELLOW "\n=== Symbols (%s + %s) ===\n" COLOR_RESET, symname, strname);

    printf(COLOR_WHITE "%-6s %-10s %-10s %-12s %s\n" COLOR_RESET,
           "Index", "Value", "Size", "Type", "Name");
    printf(COLOR_WHITE "------------------------------------------------------------\n" COLOR_RESET);

    for (int i = 0; i < count; i++) {
        const char *name = strs + syms[i].st_name;

        // Decode type
        unsigned char type = ELF64_ST_TYPE(syms[i].st_info);
        const char *type_str = "UNKNOWN";
        const char *color    = COLOR_GRAY; // default

        switch (type) {
            case STT_NOTYPE:   type_str = "NOTYPE";  color = COLOR_GRAY;    break;
            case STT_OBJECT:   type_str = "OBJECT";  color = COLOR_GREEN;   break;
            case STT_FUNC:     type_str = "FUNC";    color = COLOR_YELLOW;  break;
            case STT_SECTION:  type_str = "SECTION"; color = COLOR_BLUE;    break;
            case STT_FILE:     type_str = "FILE";    color = COLOR_CYAN;    break;
        }

        printf("%-6d 0x%08lx %-10lu %s%-12s%s %s\n",
               i,
               syms[i].st_value,
               syms[i].st_size,
               color, type_str, COLOR_RESET,
               name);
    }
}

/**
 * @brief Convert an AMD x86-64 relocation type to its string representation.
 *
 * This function takes a relocation type constant (R_X86_64_*) and returns
 * a human-readable string describing the type. If the type is unknown,
 * it returns "UNKNOWN".
 *
 * @param type The relocation type value (uint32_t), typically extracted from
 *             the r_info field of an Elf64_Rela or Elf64_Rel entry using
 *             ELF64_R_TYPE().
 *
 * @return A constant string representing the relocation type name.
 *
 * @note This only covers standard AMD x86-64 relocation types.
 *
 * @code
 * uint32_t type = ELF64_R_TYPE(rela_entry.r_info);
 * printf("Relocation type: %s\n", rel_R_X86_64_type_to_str(type));
 * @endcode
 */
const char* rel_R_X86_64_type_to_str(uint32_t type) 
{
    switch (type) {
        case R_X86_64_NONE:        return "R_X86_64_NONE";
        case R_X86_64_64:          return "R_X86_64_64";
        case R_X86_64_PC32:        return "R_X86_64_PC32";
        case R_X86_64_GOT32:       return "R_X86_64_GOT32";
        case R_X86_64_PLT32:       return "R_X86_64_PLT32";
        case R_X86_64_COPY:        return "R_X86_64_COPY";
        case R_X86_64_GLOB_DAT:    return "R_X86_64_GLOB_DAT";
        case R_X86_64_JUMP_SLOT:   return "R_X86_64_JUMP_SLOT";
        case R_X86_64_RELATIVE:    return "R_X86_64_RELATIVE";
        case R_X86_64_GOTPCREL:    return "R_X86_64_GOTPCREL";
        case R_X86_64_32:          return "R_X86_64_32";
        case R_X86_64_32S:         return "R_X86_64_32S";
        case R_X86_64_16:          return "R_X86_64_16";
        case R_X86_64_PC16:        return "R_X86_64_PC16";
        case R_X86_64_8:           return "R_X86_64_8";
        case R_X86_64_PC8:         return "R_X86_64_PC8";
        case R_X86_64_DTPMOD64:    return "R_X86_64_DTPMOD64";
        case R_X86_64_DTPOFF64:    return "R_X86_64_DTPOFF64";
        case R_X86_64_TPOFF64:     return "R_X86_64_TPOFF64";
        case R_X86_64_TLSGD:       return "R_X86_64_TLSGD";
        case R_X86_64_TLSLD:       return "R_X86_64_TLSLD";
        case R_X86_64_DTPOFF32:    return "R_X86_64_DTPOFF32";
        case R_X86_64_GOTTPOFF:    return "R_X86_64_GOTTPOFF";
        case R_X86_64_TPOFF32:     return "R_X86_64_TPOFF32";
        case R_X86_64_PC64:        return "R_X86_64_PC64";
        case R_X86_64_GOTOFF64:    return "R_X86_64_GOTOFF64";
        case R_X86_64_GOTPC32:     return "R_X86_64_GOTPC32";
        case R_X86_64_GOT64:       return "R_X86_64_GOT64";
        case R_X86_64_GOTPCREL64:  return "R_X86_64_GOTPCREL64";
        case R_X86_64_GOTPC64:     return "R_X86_64_GOTPC64";
        case R_X86_64_GOTPLT64:    return "R_X86_64_GOTPLT64";
        case R_X86_64_PLTOFF64:    return "R_X86_64_PLTOFF64";
        case R_X86_64_SIZE32:      return "R_X86_64_SIZE32";
        case R_X86_64_SIZE64:      return "R_X86_64_SIZE64";
        case R_X86_64_GOTPC32_TLSDESC: return "R_X86_64_GOTPC32_TLSDESC";
        case R_X86_64_TLSDESC_CALL: return "R_X86_64_TLSDESC_CALL";
        case R_X86_64_TLSDESC:     return "R_X86_64_TLSDESC";
        case R_X86_64_IRELATIVE:   return "R_X86_64_IRELATIVE";
        case R_X86_64_RELATIVE64:  return "R_X86_64_RELATIVE64";
        case R_X86_64_GOTPCRELX:   return "R_X86_64_GOTPCRELX";
        case R_X86_64_REX_GOTPCRELX:return "R_X86_64_REX_GOTPCRELX";
        default:                   return "UNKNOWN";
    }
}


const char* dynamic_type(uint32_t type) 
{
    char* tag_name;
    switch (type) {
        case DT_NULL:            tag_name = "DT_NULL"; break;
        case DT_NEEDED:          tag_name = "DT_NEEDED"; break;
        case DT_PLTRELSZ:        tag_name = "DT_PLTRELSZ"; break;
        case DT_PLTGOT:          tag_name = "DT_PLTGOT"; break;
        case DT_HASH:            tag_name = "DT_HASH"; break;
        case DT_STRTAB:          tag_name = "DT_STRTAB"; break;
        case DT_SYMTAB:          tag_name = "DT_SYMTAB"; break;
        case DT_RELA:            tag_name = "DT_RELA"; break;
        case DT_RELASZ:          tag_name = "DT_RELASZ"; break;
        case DT_RELAENT:         tag_name = "DT_RELAENT"; break;
        case DT_STRSZ:           tag_name = "DT_STRSZ"; break;
        case DT_SYMENT:          tag_name = "DT_SYMENT"; break;
        case DT_INIT:            tag_name = "DT_INIT"; break;
        case DT_FINI:            tag_name = "DT_FINI"; break;
        case DT_SONAME:          tag_name = "DT_SONAME"; break;
        case DT_RPATH:           tag_name = "DT_RPATH"; break;
        case DT_SYMBOLIC:        tag_name = "DT_SYMBOLIC"; break;
        case DT_REL:             tag_name = "DT_REL"; break;
        case DT_RELSZ:           tag_name = "DT_RELSZ"; break;
        case DT_RELENT:          tag_name = "DT_RELENT"; break;
        case DT_PLTREL:          tag_name = "DT_PLTREL"; break;
        case DT_DEBUG:           tag_name = "DT_DEBUG"; break;
        case DT_TEXTREL:         tag_name = "DT_TEXTREL"; break;
        case DT_JMPREL:          tag_name = "DT_JMPREL"; break;
        case DT_BIND_NOW:        tag_name = "DT_BIND_NOW"; break;
        case DT_INIT_ARRAY:      tag_name = "DT_INIT_ARRAY"; break;
        case DT_FINI_ARRAY:      tag_name = "DT_FINI_ARRAY"; break;
        case DT_INIT_ARRAYSZ:    tag_name = "DT_INIT_ARRAYSZ"; break;
        case DT_FINI_ARRAYSZ:    tag_name = "DT_FINI_ARRAYSZ"; break;
        case DT_RUNPATH:         tag_name = "DT_RUNPATH"; break;
        case DT_FLAGS:           tag_name = "DT_FLAGS"; break;
        case DT_PREINIT_ARRAY:   tag_name = "DT_PREINIT_ARRAY"; break;
        case DT_PREINIT_ARRAYSZ: tag_name = "DT_PREINIT_ARRAYSZ"; break;
        case DT_SYMTAB_SHNDX:    tag_name = "DT_SYMTAB_SHNDX"; break;
        case DT_RELRSZ:          tag_name = "DT_RELRSZ"; break;
        case DT_RELR:            tag_name = "DT_RELR"; break;
        case DT_RELRENT:         tag_name = "DT_RELRENT"; break;
        default:                 tag_name = "UNKNOWN"; break;    
    }
    return tag_name;
}


void print_rela_32bit(bparser* parser, Elf32_Ehdr* elf, Elf32_Shdr* shdrs, Elf32_Shdr *reltab, Elf32_Shdr *symtab) 
{
    Elf32_Shdr shstr  = shdrs[elf->e_shstrndx];
    Elf32_Shdr strtab = shdrs[symtab->sh_link];

    const char* shstrtab = (const char*)(parser->block + shstr.sh_offset);

    const char* relname = &shstrtab[reltab->sh_name];
    const char* symname = &shstrtab[symtab->sh_name];
    const char* strname = &shstrtab[strtab.sh_name];

    Elf32_Rela* rela = (Elf32_Rela*)(parser->block + reltab->sh_offset);
    size_t count = reltab->sh_size / sizeof(Elf32_Rela);

    Elf32_Sym* syms = (Elf32_Sym*)(parser->block + symtab->sh_offset);
    const char* strs = (const char*)(parser->block + strtab.sh_offset);

    printf(COLOR_YELLOW "\n=== Relocation Table: %-15s ===\n" COLOR_RESET, relname);
    printf("  Uses symbol table:  %s\n", symname);
    printf("  Uses string table:  %s\n", strname);
    printf(COLOR_WHITE "--------------------------------------------------------------------------\n" COLOR_RESET);
    printf(COLOR_WHITE "%-6s %-14s %-14s %-10s %-20s %-12s\n" COLOR_RESET,
           "Idx", "Offset", "Info", "Addend", "Symbol", "Type");
    printf(COLOR_WHITE "--------------------------------------------------------------------------\n" COLOR_RESET);

    for (size_t i = 0; i < count; i++) {
        uint32_t r_info  = rela[i].r_info;
        uint32_t sym_idx = ELF32_R_SYM(r_info);
        uint32_t type    = ELF32_R_TYPE(r_info);
        int32_t  addend  = rela[i].r_addend;

        const char* name = (sym_idx < (symtab->sh_size / sizeof(Elf32_Sym)))
            ? (strs + syms[sym_idx].st_name)
            : "<invalid>";

        const char* type_str = rel_R_X86_64_type_to_str(type);
        if (!type_str) type_str = "UNKNOWN";

        // Optional: color the type for easier reading
        const char* type_color = COLOR_CYAN;
        if (strstr(type_str, "PLT")) type_color = COLOR_GREEN;
        else if (strstr(type_str, "GLOB")) type_color = COLOR_MAGENTA;
        else if (strstr(type_str, "RELATIVE")) type_color = COLOR_BLUE;

        printf("%-6zu 0x%012x 0x%012x %-10d %-20s %s%-12s" COLOR_RESET "\n",
               i,
               rela[i].r_offset,
               r_info,
               addend,
               name,
               type_color,
               type_str);
    }
    printf("\n\n");
}

void print_rela_64bit(bparser* parser, Elf64_Ehdr* elf, Elf64_Shdr* shdrs,
                      Elf64_Shdr* reltab, Elf64_Shdr* symtab)
{
    Elf64_Shdr shstr  = shdrs[elf->e_shstrndx];
    Elf64_Shdr strtab = shdrs[symtab->sh_link];

    const char* shstrtab = (const char*)(parser->block + shstr.sh_offset);

    const char* relname = &shstrtab[reltab->sh_name];
    const char* symname = &shstrtab[symtab->sh_name];
    const char* strname = &shstrtab[strtab.sh_name];

    Elf64_Rela* rela = (Elf64_Rela*)(parser->block + reltab->sh_offset);
    size_t count = reltab->sh_size / sizeof(Elf64_Rela);

    Elf64_Sym* syms = (Elf64_Sym*)(parser->block + symtab->sh_offset);
    const char* strs = (const char*)(parser->block + strtab.sh_offset);

    printf(COLOR_YELLOW "\n=== Relocation Table: %-15s ===\n" COLOR_RESET, relname);
    printf("  Uses symbol table:  %s\n", symname);
    printf("  Uses string table:  %s\n", strname);
    printf(COLOR_WHITE "--------------------------------------------------------------------------\n" COLOR_RESET);
    printf(COLOR_WHITE "%-6s %-14s %-14s %-10s %-20s %-12s\n" COLOR_RESET,
           "Idx", "Offset", "Info", "Addend", "Symbol", "Type");
    printf(COLOR_WHITE "--------------------------------------------------------------------------\n" COLOR_RESET);

    for (size_t i = 0; i < count; i++) {
        uint64_t r_info  = rela[i].r_info;
        uint64_t sym_idx = ELF64_R_SYM(r_info);
        uint64_t type    = ELF64_R_TYPE(r_info);
        int64_t  addend  = rela[i].r_addend;

        const char* name = (sym_idx < (symtab->sh_size / sizeof(Elf64_Sym)))
            ? (strs + syms[sym_idx].st_name)
            : "<invalid>";

        const char* type_str = rel_R_X86_64_type_to_str(type);
        if (!type_str) type_str = "UNKNOWN";

        // Optional: color the type for easier reading
        const char* type_color = COLOR_CYAN;
        if (strstr(type_str, "PLT")) type_color = COLOR_GREEN;
        else if (strstr(type_str, "GLOB")) type_color = COLOR_MAGENTA;
        else if (strstr(type_str, "RELATIVE")) type_color = COLOR_BLUE;

        printf("%-6zu 0x%012lx 0x%012lx %-10ld %-20s %s%-12s" COLOR_RESET "\n",
               i,
               rela[i].r_offset,
               r_info,
               addend,
               name,
               type_color,
               type_str);
    }
    printf("\n\n");
}

void print_dynamic_table_32bit(bparser* parser, Elf32_Ehdr* elf, Elf32_Shdr* shdrs,
                               Elf32_Shdr *dynmaictab, Elf32_Shdr *strtab)
{
    Elf32_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(parser->block + shstr.sh_offset);

    const char* dynname = &shstrtab[dynmaictab -> sh_name];
    const char* strname = &shstrtab[strtab -> sh_name];

    Elf32_Dyn *dyns = (Elf32_Dyn *)(parser->block + dynmaictab->sh_offset);
    const char *strs = (const char *)(parser->block + strtab->sh_offset);
    unsigned int count = dynmaictab->sh_size / sizeof(Elf32_Dyn);

    printf(COLOR_YELLOW "\n=== Dynamic (%s + %s) ===\n" COLOR_RESET, dynname, strname);

    printf("%-20s %-18s %s\n", "Tag", "Value", "Interpretation");
    printf("-------------------------------------------------------------\n");

    for (size_t i = 0; i < count; i++) {
        Elf32_Dyn *dyn = &dyns[i];

        if (dyn->d_tag == DT_NULL)
            break;  // End of table

        const char *tag_name = dynamic_type(dyn->d_tag);
        printf("%-20s 0x%016lx ", tag_name, (unsigned long)dyn->d_un.d_val);

        // Interpretation
        if (dyn->d_tag == DT_NEEDED || dyn->d_tag == DT_SONAME ||
                dyn->d_tag == DT_RPATH || dyn->d_tag == DT_RUNPATH) {
            printf("%s", strs + dyn->d_un.d_val);
        }

        printf("\n");
    }
}

void print_dynamic_table_64bit(bparser* parser, Elf64_Ehdr* elf, Elf64_Shdr* shdrs, 
                               Elf64_Shdr *dynmaictab, Elf64_Shdr *strtab)
{
    Elf64_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(parser->block + shstr.sh_offset);

    const char* dynname = &shstrtab[dynmaictab -> sh_name];
    const char* strname = &shstrtab[strtab -> sh_name];

    Elf64_Dyn *dyns = (Elf64_Dyn *)(parser->block + dynmaictab->sh_offset);
    const char *strs = (const char *)(parser->block + strtab->sh_offset);
    unsigned int count = dynmaictab->sh_size / sizeof(Elf64_Dyn);

    printf(COLOR_YELLOW "\n=== Dynamic (%s + %s) ===\n" COLOR_RESET, dynname, strname);

    printf("%-20s %-18s %s\n", "Tag", "Value", "Interpretation");
    printf("-------------------------------------------------------------\n");

    for (size_t i = 0; i < count; i++) {
        Elf64_Dyn *dyn = &dyns[i];

        if (dyn->d_tag == DT_NULL)
            break;  // End of table

        const char *tag_name = dynamic_type(dyn->d_tag);
        printf("%-20s 0x%016lx ", tag_name, (unsigned long)dyn->d_un.d_val);

        // Interpretation
        if (dyn->d_tag == DT_NEEDED || dyn->d_tag == DT_SONAME ||
                dyn->d_tag == DT_RPATH || dyn->d_tag == DT_RUNPATH) {
            printf("%s", strs + dyn->d_un.d_val);
        }
        printf("\n");
    }
}

/**
 * @brief Print ELF32 symbols along with disassembly for functions.
 *
 * This function iterates over the symbol table of a 32-bit ELF file,
 * printing information about each symbol and disassembling function symbols.
 * It highlights the type of each symbol using ANSI color codes.
 *
 * @param parser Pointer to a bparser structure containing the loaded ELF data.
 * @param elf Pointer to the ELF32 file header (Elf32_Ehdr).
 * @param shdrs Pointer to the section header array (Elf32_Shdr[]).
 * @param symtab Pointer to the section header of the symbol table (Elf32_Shdr).
 * @param strtab Pointer to the section header of the associated string table (Elf32_Shdr).
 *
 * @note The function assumes:
 *       - ANSI color macros (COLOR_RESET, COLOR_YELLOW, COLOR_BLUE, etc.) are defined.
 *       - Helper function `print_disasm` exists to disassemble memory regions.
 *       - ELF macros like `ELF32_ST_TYPE`, and constants like `STT_FUNC`, `STT_OBJECT` are defined.
 *       - Only symbols with st_size > 0 are disassembled.
 *
 * Example output:
 * @code
 * === Symbols (symtab + strtab) ===
 * |-- main:
 *   0x08001000:  mov eax, ebx
 *   ...
 * @endcode
 */
void print_symbols_with_disasm_32bit(bparser* parser, Elf32_Ehdr* elf, Elf32_Shdr* shdrs, Elf32_Shdr *symtab, Elf32_Shdr *strtab) 
{
    Elf32_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(parser->block + shstr.sh_offset);

    const char* symname = &shstrtab[symtab -> sh_name];
    const char* strname = &shstrtab[strtab -> sh_name];

    Elf32_Sym *syms = (Elf32_Sym *)(parser->block + symtab->sh_offset);
    const char *strs = (const char *)(parser->block + strtab->sh_offset);

    unsigned int count = symtab->sh_size / sizeof(Elf32_Sym);

    printf(COLOR_RESET);
    printf("\n");
    printf(COLOR_YELLOW"=== Symbols (" COLOR_BLUE "%s" COLOR_YELLOW " + " COLOR_BLUE "%s" COLOR_YELLOW ") ===\n" COLOR_RESET, symname, strname);

    for (int i = 0; i < count; i++) {
        const char *name = strs + syms[i].st_name;
        unsigned char type = ELF32_ST_TYPE(syms[i].st_info);
        if(syms[i].st_size > 0){
            if(type == STT_FUNC) {
                unsigned char* ptr = (unsigned char*)parser->block + syms[i].st_value;
                printf("\n");
                printf(COLOR_WHITE "|-- %s:" COLOR_RESET "\n", name);
                print_disasm(ptr, syms[i].st_size, syms[i].st_value, ELFCLASS32);
            }
        }
    }
}

/**
 * @brief Print ELF64 symbols along with disassembly for functions.
 *
 * This function iterates over the symbol table of a 64-bit ELF file,
 * printing information about each symbol and disassembling function symbols.
 * It highlights the type of each symbol using ANSI color codes.
 *
 * @param parser Pointer to a bparser structure containing the loaded ELF data.
 * @param elf Pointer to the ELF64 file header (Elf64_Ehdr).
 * @param shdrs Pointer to the section header array (Elf64_Shdr[]).
 * @param symtab Pointer to the section header of the symbol table (Elf64_Shdr).
 * @param strtab Pointer to the section header of the associated string table (Elf64_Shdr).
 *
 * @note The function assumes:
 *       - ANSI color macros (COLOR_RESET, COLOR_YELLOW, COLOR_BLUE, etc.) are defined.
 *       - Helper function `print_disasm` exists to disassemble memory regions.
 *       - ELF macros like `ELF64_ST_TYPE`, and constants like `STT_FUNC`, `STT_OBJECT` are defined.
 *       - Only symbols with st_size > 0 are disassembled.
 *
 * Example output:
 * @code
 * === Symbols (symtab + strtab) ===
 * |-- main:
 *   0x00401000:  mov rax, rbx
 *   ...
 * @endcode
 */
void print_symbols_with_disasm_64bit(bparser* parser, Elf64_Ehdr* elf, Elf64_Shdr* shdrs, Elf64_Shdr *symtab, Elf64_Shdr *strtab) 
{
    Elf64_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(parser->block + shstr.sh_offset);

    const char* symname = &shstrtab[symtab -> sh_name];
    const char* strname = &shstrtab[strtab -> sh_name];

    Elf64_Sym *syms = (Elf64_Sym *)(parser->block + symtab->sh_offset);
    const char *strs = (const char *)(parser->block + strtab->sh_offset);

    unsigned int count = symtab->sh_size / sizeof(Elf64_Sym);

    printf(COLOR_RESET);
    printf("\n");
    printf(COLOR_YELLOW"=== Symbols (" COLOR_BLUE "%s" COLOR_YELLOW " + " COLOR_BLUE "%s" COLOR_YELLOW ") ===\n" COLOR_RESET, symname, strname);

    for (int i = 0; i < count; i++) {
        const char *name = strs + syms[i].st_name;
        unsigned char type = ELF64_ST_TYPE(syms[i].st_info);
        if(syms[i].st_size > 0){
            if(type == STT_FUNC) {
                unsigned char* ptr = (unsigned char*)parser->block + syms[i].st_value;
                printf("\n");
                printf(COLOR_WHITE "|-- %s:" COLOR_RESET "\n", name);
                print_disasm(ptr, syms[i].st_size, syms[i].st_value, ELFCLASS64);
            }
        }
    }
}



/**
 * @brief Print metadata of a 32-bit ELF section header.
 *
 * This function prints detailed information about a single ELF32 section,
 * including type, flags, address, offset, size, link, info, alignment, and entry size.
 * It uses ANSI color codes for visual distinction.
 *
 * @param id The index of the section in the section header table.
 * @param name The name of the section.
 * @param type_str A string representing the section type (e.g., "SHT_PROGBITS").
 * @param flags A string representing the section flags (e.g., "AX" for alloc+execute).
 * @param shdrs Pointer to the array of ELF32 section headers (Elf32_Shdr[]).
 *
 * @note Assumes that the macros for ANSI colors (COLOR_CYAN, COLOR_BLUE, COLOR_BG_WHITE, COLOR_BCYAN, COLOR_RESET)
 *       and META_LABEL_WIDTH are defined.
 *
 * Example output:
 * @code
 * |--Section [1] .text:
 * |---Type:        PROGBITS
 * |---Flags:       AX
 * |---Addr:        0x08048000
 * |---Offset:      0x00001000
 * |---Size:        0x00002000
 * |---Link:        0
 * |---Info:        0
 * |---Align:       0x10
 * |---EntSize:     0
 * @endcode
 */
void print_section_header_metadata_32bit(unsigned int id, const char* name, const char*type_str, const char* flags, Elf32_Shdr* shdrs) 
{
    printf(COLOR_BG_WHITE COLOR_BCYAN "|--Section [%d]"  COLOR_RESET COLOR_BLUE " %s" COLOR_CYAN ":\n" COLOR_RESET, id, name);
    // printf(COLOR_GREEN "|---%-*s" COLOR_RESET "%d\n",   META_LABEL_WIDTH, "ID:", id);
    // printf(COLOR_GREEN "|---%-*s" COLOR_RESET "%s\n",   META_LABEL_WIDTH, "Name:", name);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "%s\n",   META_LABEL_WIDTH, "Type:", type_str);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "%s\n",   META_LABEL_WIDTH, "Flags:", flags);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%08x\n", META_LABEL_WIDTH, "Addr:", shdrs[id].sh_addr);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%08x\n", META_LABEL_WIDTH, "Offset:", shdrs[id].sh_offset);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%x\n", META_LABEL_WIDTH, "Size:", shdrs[id].sh_size);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "%d\n",   META_LABEL_WIDTH, "Link:", shdrs[id].sh_link);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%x\n", META_LABEL_WIDTH, "Info:", shdrs[id].sh_info);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%x\n", META_LABEL_WIDTH, "Align:", shdrs[id].sh_addralign);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%x\n", META_LABEL_WIDTH, "EntSize:", shdrs[id].sh_entsize);
}

/**
 * @brief Print metadata of a 64-bit ELF section header.
 *
 * This function prints detailed information about a single ELF64 section,
 * including type, flags, address, offset, size, link, info, alignment, and entry size.
 * It uses ANSI color codes for visual distinction.
 *
 * @param id The index of the section in the section header table.
 * @param name The name of the section.
 * @param type_str A string representing the section type (e.g., "SHT_PROGBITS").
 * @param flags A string representing the section flags (e.g., "AX" for alloc+execute).
 * @param shdrs Pointer to the array of ELF64 section headers (Elf64_Shdr[]).
 *
 * @note Assumes that the macros for ANSI colors (COLOR_CYAN, COLOR_BLUE, COLOR_BG_WHITE, COLOR_BCYAN, COLOR_RESET)
 *       and META_LABEL_WIDTH are defined.
 *
 * Example output:
 * @code
 * |--Section [1] .text:
 * |---Type:        PROGBITS
 * |---Flags:       AX
 * |---Addr:        0x00401000
 * |---Offset:      0x00001000
 * |---Size:        0x00002000
 * |---Link:        0
 * |---Info:        0
 * |---Align:       0x10
 * |---EntSize:     0
 * @endcode
 */
void print_section_header_metadata_64bit(unsigned int id, const char* name, const char*type_str, const char* flags, Elf64_Shdr* shdrs) 
{
    printf(COLOR_BG_WHITE COLOR_BCYAN "|--Section [%d]"  COLOR_RESET COLOR_BLUE " %s" COLOR_CYAN ":\n" COLOR_RESET, id, name);
    // printf(COLOR_GREEN "|---%-*s" COLOR_RESET "%d\n",   META_LABEL_WIDTH, "ID:", id);
    // printf(COLOR_GREEN "|---%-*s" COLOR_RESET "%s\n",   META_LABEL_WIDTH, "Name:", name);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "%s\n",   META_LABEL_WIDTH, "Type:", type_str);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "%s\n",   META_LABEL_WIDTH, "Flags:", flags);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%lx\n", META_LABEL_WIDTH, "Addr:", shdrs[id].sh_addr);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%lx\n", META_LABEL_WIDTH, "Offset:", shdrs[id].sh_offset);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%lx\n", META_LABEL_WIDTH, "Size:", shdrs[id].sh_size);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "%d\n",   META_LABEL_WIDTH, "Link:", shdrs[id].sh_link);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%x\n", META_LABEL_WIDTH, "Info:", shdrs[id].sh_info);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%lx\n", META_LABEL_WIDTH, "Align:", shdrs[id].sh_addralign);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%lx\n", META_LABEL_WIDTH, "EntSize:", shdrs[id].sh_entsize);
}

/**
 * @brief Print metadata of a 32-bit ELF program header (segment).
 *
 * This function prints detailed information about a single ELF32 program segment,
 * including type, flags, offset, virtual/physical addresses, file/memory size, and alignment.
 * It uses ANSI color codes for visual distinction.
 *
 * @param id The index of the program header in the program header table.
 * @param type_str A string representing the segment type (e.g., "PT_LOAD").
 * @param flags A string representing the segment flags (e.g., "R E" for readable + executable).
 * @param phdr Pointer to the array of ELF32 program headers (Elf32_Phdr[]).
 *
 * @note Assumes that the macros for ANSI colors (COLOR_CYAN, COLOR_BG_WHITE, COLOR_BCYAN, COLOR_RESET)
 *       and META_LABEL_WIDTH are defined.
 *
 * Example output:
 * @code
 * |--Program Segment [0]:
 * |---Type:       PT_LOAD
 * |---Flags:      R E
 * |---Offset:     0x00001000
 * |---VirtAddr:   0x08048000
 * |---PhysAddr:   0x08048000
 * |---FileSz:     0x00002000
 * |---MemSz:      0x00002000
 * |---Align:      0x1000
 * @endcode
 */
void print_program_header_metadata_32bit(unsigned int id, const char*type_str, const char* flags, Elf32_Phdr* phdr) 
{
    printf(COLOR_BG_WHITE COLOR_BCYAN "|--Program Segment [%d]:" COLOR_RESET "\n", id);
    // printf(COLOR_GREEN "|---%-*s" COLOR_RESET "%d\n",   META_LABEL_WIDTH, "ID:", id);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "%s\n",   META_LABEL_WIDTH, "Type:", type_str);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "%s\n",   META_LABEL_WIDTH,"Flags:", flags);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%08x\n", META_LABEL_WIDTH, "Offset:", phdr[id].p_offset);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%08x\n", META_LABEL_WIDTH, "VirtAddr:", phdr[id].p_vaddr);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%08x\n", META_LABEL_WIDTH, "PhysAddr:", phdr[id].p_paddr);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%x\n",   META_LABEL_WIDTH, "FileSz:", phdr[id].p_filesz);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%x\n", META_LABEL_WIDTH, "MemSz:", phdr[id].p_memsz);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%x\n", META_LABEL_WIDTH, "Align:", phdr[id].p_align);
}

/**
 * @brief Print metadata of a 64-bit ELF program header (segment).
 *
 * This function prints detailed information about a single ELF64 program segment,
 * including type, flags, offset, virtual/physical addresses, file/memory size, and alignment.
 * It uses ANSI color codes for visual distinction.
 *
 * @param id The index of the program header in the program header table.
 * @param type_str A string representing the segment type (e.g., "PT_LOAD").
 * @param flags A string representing the segment flags (e.g., "R E" for readable + executable).
 * @param phdr Pointer to the array of ELF64 program headers (Elf64_Phdr[]).
 *
 * @note Assumes that the macros for ANSI colors (COLOR_CYAN, COLOR_BG_WHITE, COLOR_BCYAN, COLOR_RESET)
 *       and META_LABEL_WIDTH are defined.
 *
 * Example output:
 * @code
 * |--Program Segment [0]:
 * |---Type:       PT_LOAD
 * |---Flags:      R E
 * |---Offset:     0x00001000
 * |---VirtAddr:   0x00401000
 * |---PhysAddr:   0x00401000
 * |---FileSz:     0x00002000
 * |---MemSz:      0x00002000
 * |---Align:      0x1000
 * @endcode
 */
void print_program_header_metadata_64bit(unsigned int id, const char*type_str, const char* flags, Elf64_Phdr* phdr) 
{
    // printf(COLOR_CYAN "|--Program Segment [%d]:\n" COLOR_RESET, id);
    printf(COLOR_BG_WHITE COLOR_BCYAN "|--Program Segment [%d]:" COLOR_RESET "\n", id);
    // printf(COLOR_GREEN "|---%-*s" COLOR_RESET "%d\n",   META_LABEL_WIDTH, "ID:", id);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "%s\n",   META_LABEL_WIDTH, "Type:", type_str);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "%s\n",   META_LABEL_WIDTH,"Flags:", flags);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%08lx\n", META_LABEL_WIDTH, "Offset:", phdr[id].p_offset);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%lx\n", META_LABEL_WIDTH, "VirtAddr:", phdr[id].p_vaddr);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%lx\n", META_LABEL_WIDTH, "PhysAddr:", phdr[id].p_paddr);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%lx\n",   META_LABEL_WIDTH, "FileSz:", phdr[id].p_filesz);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%lx\n", META_LABEL_WIDTH, "MemSz:", phdr[id].p_memsz);
    printf(COLOR_CYAN "|---%-*s" COLOR_RESET "0x%lx\n", META_LABEL_WIDTH, "Align:", phdr[id].p_align);
}

/**
 * @brief Print a block of bytes in hex and ASCII format, optionally disassembling executable instructions.
 *
 * This function prints the content of a memory block in a traditional hex dump format,
 * showing the offset, hexadecimal bytes, and ASCII representation. If the block contains
 * executable instructions (SHF_EXECINSTR flag is set), it also disassembles the instructions
 * using the Udis86 library and highlights the assembly.
 *
 * @param ptr Pointer to the memory block to print.
 * @param size Number of bytes to print from the memory block.
 * @param offset Starting offset to display in the hex dump.
 * @param disasm If non-zero and contains SHF_EXECINSTR, the function disassembles the bytes.
 * @param bit_type ELF class: ELFCLASS32 for 32-bit, ELFCLASS64 for 64-bit.
 *
 * @note Uses ANSI color codes for highlighting offsets, hex bytes, and disassembly.
 *       Assumes the following helper functions exist:
 *         - print_hex_header(offset)
 *         - display_byte(unsigned char *ptr)
 *         - display_byte_char(unsigned char *ptr)
 *         - print_highlight_asm(const char* line)
 *
 * @note BLOCK_LENGTH macro defines how many bytes per line (commonly 16).
 *
 * Example output:
 * @code
 * | 
 * |    --Offset--   0 1  2 3  4 5  6 7  8 9  A B  C D  E F    0123456789ABCDEF
 * |----0x00001000:  4865 6C6C 6F20 576F 726C 6421 0000 0000  |Hello World!....|
 * |----0x00001010:  ... (next 16 bytes)
 * 
 * Disassembly (if executable):
 * |----0x00001000:  mov eax, 1
 * |----0x00001005:  ret
 * @endcode
 */
void print_body_bytes(unsigned char *ptr, size_t size, unsigned long long offset, int disasm, unsigned char bit_type)
{
    printf(COLOR_GREEN "|" COLOR_RESET  );
    print_hex_header(offset);
    
    size_t i, j;
    for (i = 0; i < size; i += BLOCK_LENGTH) {
        // Print offset
        printf(COLOR_GREEN "|----0x%08llx:  " COLOR_RESET, offset + i);

        // Print hex bytes
        for (j = 0; j < BLOCK_LENGTH; j++) {
            if (i + j < size)
                display_byte(&ptr[i+j]);
                // printf("%02x", ptr[i + j]);
            else
                printf("   "); // padding for alignment
                               
            if ((j + 1) % 2 == 0)
                printf(" "); // extra space every 2 bytes
        }

        // Print ASCII chars
        printf(" |");
        for (j = 0; j < BLOCK_LENGTH && i + j < size; j++) {
            unsigned char c = ptr[i + j];
            display_byte_char(&ptr[i+j]);
        }
        printf("|");
        printf("\n");

    }

    if (disasm & SHF_EXECINSTR) {
        ud_t ud_obj;
        ud_init(&ud_obj);
        ud_set_input_buffer(&ud_obj, ptr, size);
        ud_set_mode(&ud_obj, (bit_type == ELFCLASS32) ? 32: 64);        // change to 64 for x86_64
        ud_set_syntax(&ud_obj, UD_SYN_INTEL);
        ud_set_pc(&ud_obj, offset);
        // printf(COLOR_YELLOW "\nDisassembly:\n" COLOR_RESET);
        while (ud_disassemble(&ud_obj)) {
            printf(COLOR_YELLOW "|----0x%08llx:  " COLOR_RESET, (unsigned long long)ud_insn_off(&ud_obj));
            print_highlight_asm(ud_insn_asm(&ud_obj));
        }
    }
}

/**
 * @brief Disassemble a block of machine code and print the assembly with highlighting.
 *
 * This function uses the Udis86 library to disassemble a memory block and prints
 * each instruction in Intel syntax. The output includes offsets and color highlighting
 * for opcodes, registers, and addresses.
 *
 * @param ptr Pointer to the memory block containing machine code.
 * @param size Number of bytes to disassemble.
 * @param offset Starting address to display in the disassembly output.
 * @param bit_type ELF class: ELFCLASS32 for 32-bit instructions, ELFCLASS64 for 64-bit instructions.
 *
 * @note Uses the helper function `print_highlight_asm()` to colorize the instructions.
 * @note ANSI color codes (COLOR_YELLOW, COLOR_RESET) are used for highlighting.
 *
 * Example output:
 * @code
 * |----0x00001000:  mov eax, 1
 * |----0x00001005:  add ebx, eax
 * |----0x00001007:  ret
 * @endcode
 */
void print_disasm(unsigned char *ptr, size_t size, unsigned long long offset, unsigned char bit_type)
{
    ud_t ud_obj;
    ud_init(&ud_obj);
    ud_set_input_buffer(&ud_obj, ptr, size);
    ud_set_mode(&ud_obj, (bit_type == ELFCLASS32) ? 32: 64);        // 32 or 64 bit
    ud_set_syntax(&ud_obj, UD_SYN_INTEL);
    ud_set_pc(&ud_obj, offset);
    // printf(COLOR_YELLOW "\nDisassembly:\n" COLOR_RESET);
    while (ud_disassemble(&ud_obj)) {
        printf(COLOR_YELLOW "|----0x%08llx:  " COLOR_RESET, (unsigned long long)ud_insn_off(&ud_obj));
        print_highlight_asm(ud_insn_asm(&ud_obj));
    }
}

/**
 * @brief Format ELF program header flags into a colored string.
 *
 * This function converts the `p_flags` field of an ELF program header into
 * a human-readable string with color highlighting for readability:
 * - Readable (R) → green
 * - Writable (W) → red
 * - Executable (X) → yellow
 *
 * @param p_flags The flags from the ELF program header (p_flags field of Elf32_Phdr or Elf64_Phdr).
 * @param buf Output buffer to store the formatted string. Must be preallocated.
 * @param size Size of the output buffer.
 *
 * @note Uses ANSI color codes: COLOR_GREEN, COLOR_RED, COLOR_YELLOW, COLOR_CYAN, COLOR_RESET.
 * @note Adds 5 spaces after the flags for alignment.
 *
 * Example usage:
 * @code
 * char flags_buf[32];
 * format_p_flags(phdr[i].p_flags, flags_buf, sizeof(flags_buf));
 * printf("|---Flags: %s\n", flags_buf);
 * @endcode
 */
void format_p_flags(uint32_t p_flags, char *buf, size_t size)
{
    buf[0] = '\0';
    if (p_flags & PF_R) strncat(buf, COLOR_GREEN "R" COLOR_RESET, size - strlen(buf) - 1);
    if (p_flags & PF_W) strncat(buf, COLOR_RED "W" COLOR_RESET, size - strlen(buf) - 1);
    if (p_flags & PF_X) strncat(buf, COLOR_YELLOW "X" COLOR_RESET, size - strlen(buf) - 1);
    for(int i=0; i<5; i++)
        strncat(buf, " ", size - strlen(buf) - 1);
    strncat(buf, COLOR_CYAN , size - strlen(buf) - 1);
}

