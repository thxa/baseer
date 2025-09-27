#include "bx_elf_utils.h"



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


/**
 * @brief Display a single byte in hexadecimal with color coding.
 *
 * This function prints the value of a byte in hex format (two digits) 
 * and applies a color depending on its meaning:
 * - Default (COLOR_YELLOW) for active instructions.
 * - COLOR_RED for NOP (0x90), INT3 (0xCC), or filler bytes (0xFF).
 * - COLOR_GRAY for padding or null bytes (0x00).
 *
 * @param byte Pointer to the byte to display.
 *
 * @note The color codes are ANSI escape sequences.
 *       The color is reset after printing each byte using COLOR_RESET.
 */
void display_byte(const unsigned char *byte)
{
    const char *color = COLOR_YELLOW; // default for active instruction

    if (*byte == 0x90 || *byte == 0xCC || *byte == 0xff) {
        color = COLOR_RED;  // low / unused
    }
    else if (*byte == 0x00) {
        color = COLOR_GRAY; // padding or null bytes
    }
    printf("%s%02x%s", color, *byte, COLOR_RESET);
}

/**
 * @brief Display a single byte as a printable character with color coding.
 * 
 * This function prints the ASCII representation of a byte. Non-printable
 * bytes are shown as a '.' character. The output is color-coded based
 * on the byte value:
 *   - 0x90, 0xCC, 0xFF → COLOR_RED (low / unused bytes)
 *   - 0x00             → COLOR_GRAY (padding or null bytes)
 *   - All other bytes  → COLOR_YELLOW (default / active instruction)
 * 
 * Printable ASCII characters (32-126) are displayed as-is, while
 * non-printable bytes are represented with '.'.
 * 
 * @param byte Pointer to the byte to display.
 */
void display_byte_char(const unsigned char *byte)
{
    const char *color = COLOR_YELLOW; // default for active instruction 
    unsigned char c = *byte;
    if (*byte == 0x90 || *byte == 0xCC || *byte == 0xff) {
        color = COLOR_RED;  // low / unused
    }
    else if (*byte == 0x00) {
        color = COLOR_GRAY; // padding or null bytes
    }
    if (c >= 32 && c <= 126) { // printable ASCII range
        printf("%s%c%s", color, c, COLOR_RESET);
    } else {
        printf("%s.%s", color, COLOR_RESET); // non-printable
    }
}






// // List of metadata sections
// const char* metadata_sections[] = {
//     ".rela.dyn",
//     ".rela.plt",
//     ".dynsym",
//     ".dynstr",
//     ".eh_frame",
//     ".eh_frame_hdr",
//     ".note.gnu.property",
//     ".note.gnu.build-id",
//     ".comment",
//     NULL
// };

// bool is_metadata_section(const char* name) 
// {
//     for (int i = 0; metadata_sections[i] != NULL; i++) {
//         if (strcmp(name, metadata_sections[i]) == 0)
//             return true;
//     }
//     return false;
// }

#define COLOR_STRING  "\033[0;32m"  // green
#define COLOR_LINENO "\033[0;35m"  // magenta
#define COLOR_COMMENT  "\033[0;90m"  // gray
#define COLOR_PUNCTUATION "\033[0;37m" // Light gray for punctuation (braces, etc.)
                                       
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

// ================ Helper =====================
int is_in_list(const char *word, const char *list[]) {
    for (int i=0; list[i]; i++)
        if (strcmp(word, list[i]) == 0) return 1;
    return 0;
}

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

static int is_punctuation(char c) { 
    return c == '{' || c == '}' || c == '(' || c == ')' || c == ';' || c == ',' || 
           c == '=' || c == '*' || c == '&' || c == '[' || c == ']' || c == '-' || c == '+' || c == ':'; 
}

// ================ Printing ===================
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

// inline function to print hex header row
void print_hex_header(unsigned long long offset)
{
    printf(COLOR_GREEN "\n|\n" COLOR_GREEN);
    printf(COLOR_GREEN "|    --Offset--%3s" COLOR_RESET, "");
    printf(COLOR_GREEN);

    // Hex dump
    for(int i=0; i<=9; i++) {
        printf("%d", i);
        printf((i & 1)?"  ":" ");
    }
    for(char c='A'; c <= 'F'; c++){
        printf("%c", c);
        printf((c & 1)?" ":"  ");
    }
    
    // Ascii Dump
    printf("%-2s", "");
    for(int i=0; i<=9; i++)
        printf("%d", i);
    for(char c='A'; c <= 'F'; c++)
        printf("%c", c);

    printf(COLOR_RESET);
    printf("\n");
}

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


// Convert block of bytes to asm instructions 32 or 64
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


