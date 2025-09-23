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
    printf(COLOR_WHITE "| Type                | Meaning                            |\n" COLOR_RESET);
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
    printf(COLOR_WHITE "| Flag | Meaning                                   |\n" COLOR_RESET);
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
    printf(COLOR_WHITE "| Type       | Meaning                                  |\n" COLOR_RESET);
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
    printf(COLOR_WHITE "| Flag| Meaning                           |\n" COLOR_RESET);
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


