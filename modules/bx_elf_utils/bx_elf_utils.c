#include "bx_elf_utils.h"

void print_program_header_legend(void)
{
    printf(COLOR_YELLOW "=== Program Header Types Legend ===\n" COLOR_RESET);
    printf("+------------+------------------------------------------+\n");
    printf("| Type       | Meaning                                  |\n");
    printf("+------------+------------------------------------------+\n");
    printf("| NULL       | PT_NULL: Unused entry                    |\n");
    printf("| LOAD       | PT_LOAD: Loadable segment                |\n");
    printf("| DYNAMIC    | PT_DYNAMIC: Dynamic linking info         |\n");
    printf("| INTERP     | PT_INTERP: Interpreter path              |\n");
    printf("| NOTE       | PT_NOTE: Auxiliary information           |\n");
    printf("| SHLIB      | PT_SHLIB: Reserved                       |\n");
    printf("| PHDR       | PT_PHDR: Program header table itself     |\n");
    printf("| TLS        | PT_TLS: Thread-Local Storage template    |\n");
    printf("| GNU_STACK  | PT_GNU_STACK: Stack flags                |\n");
    printf("| GNU_RELRO  | PT_GNU_RELRO: Read-only after relocation |\n");
    printf("+------------+------------------------------------------+\n\n");

    printf(COLOR_YELLOW "=== Program Header Flags Legend ===\n" COLOR_RESET);
    printf("+-----+-----------------------------------+\n");
    printf("| Flag| Meaning                           |\n");
    printf("+-----+-----------------------------------+\n");
    printf("| " COLOR_GREEN  " R " COLOR_RESET "  | PF_R: Readable                    |\n");
    printf("| " COLOR_RED    " W " COLOR_RESET "  | PF_W: Writable                    |\n");
    printf("| " COLOR_YELLOW " X " COLOR_RESET "  | PF_X: Executable                   |\n");
    printf("+-----+-----------------------------------+\n\n");
}


void print_section_header_legend(void)
{
    printf(COLOR_YELLOW "=== Section Header Flags Legend ===\n" COLOR_RESET);
    printf("+-----+-----------------------------------+\n");
    printf("| Flag| Meaning                           |\n");
    printf("+-----+-----------------------------------+\n");
    printf("|  W  | SHF_WRITE: Writable               |\n");
    printf("|  A  | SHF_ALLOC: Occupies memory        |\n");
    printf("|  X  | SHF_EXECINSTR: Executable code    |\n");
    printf("|  M  | SHF_MERGE: Might be merged        |\n");
    printf("|  S  | SHF_STRINGS: Contains strings     |\n");
    printf("|  I  | SHF_INFO_LINK: sh_info contains   |\n");
    printf("|     | special meaning                   |\n");
    printf("|  L  | SHF_LINK_ORDER: Link order        |\n");
    printf("|  O  | SHF_OS_NONCONFORMING: OS specific |\n");
    printf("|  G  | SHF_GROUP: Section group          |\n");
    printf("|  T  | SHF_TLS: Thread-Local Storage     |\n");
    printf("+-----+-----------------------------------+\n\n");


    printf(COLOR_YELLOW "=== Section Header Types Legend ===\n" COLOR_RESET);
    printf("+---------------------+-----------------------------------+\n");
    printf("| Type                | Meaning                           |\n");
    printf("+---------------------+-----------------------------------+\n");
    printf("| NULL                | SHT_NULL: Unused section          |\n");
    printf("| PROGBITS            | SHT_PROGBITS: Program-defined data|\n");
    printf("| SYMTAB              | SHT_SYMTAB: Symbol table          |\n");
    printf("| STRTAB              | SHT_STRTAB: String table          |\n");
    printf("| RELA                | SHT_RELA: Relocation with addends |\n");
    printf("| HASH                | SHT_HASH: Symbol hash table       |\n");
    printf("| DYNAMIC             | SHT_DYNAMIC: Dynamic linking info |\n");
    printf("| NOTE                | SHT_NOTE: Auxiliary information   |\n");
    printf("| NOBITS              | SHT_NOBITS: Occupies no file space|\n");
    printf("| REL                 | SHT_REL: Relocation without addends|\n");
    printf("| SHLIB               | SHT_SHLIB: Reserved               |\n");
    printf("| DYNSYM              | SHT_DYNSYM: Dynamic symbol table  |\n");
    printf("| INIT_ARRAY          | SHT_INIT_ARRAY: Constructors array|\n");
    printf("| FINI_ARRAY          | SHT_FINI_ARRAY: Destructors array |\n");
    printf("| PREINIT_ARRAY       | SHT_PREINIT_ARRAY: Pre-initializers|\n");
    printf("| GROUP               | SHT_GROUP: Section group          |\n");
    printf("| SYMTAB_SHNDX        | SHT_SYMTAB_SHNDX: Extended indices|\n");
    printf("+---------------------+-----------------------------------+\n\n");
}

const char* elf_machine_to_str(unsigned int machine)
{
    switch (machine) {
        case EM_NONE:       return "No machine";
        case EM_M32:        return "AT&T WE 32100";
        case EM_SPARC:      return "SUN SPARC";
        case EM_386:        return "Intel 80386";
        case EM_68K:        return "Motorola 68000";
        case EM_88K:        return "Motorola 88000";
        case EM_IAMCU:      return "Intel MCU";
        case EM_860:        return "Intel 80860";
        case EM_MIPS:       return "MIPS R3000 (big-endian)";
        case EM_S370:       return "IBM System/370";
        case EM_MIPS_RS3_LE:return "MIPS R3000 (little-endian)";
        case EM_PARISC:     return "HPPA";
        case EM_VPP500:     return "Fujitsu VPP500";
        case EM_SPARC32PLUS:return "Sun SPARC v8+";
        case EM_960:        return "Intel 80960";
        case EM_PPC:        return "PowerPC";
        case EM_PPC64:      return "PowerPC 64-bit";
        case EM_S390:       return "IBM S/390";
        case EM_ARM:        return "ARM";
        case EM_SH:         return "Hitachi SH";
        case EM_SPARCV9:    return "SPARC v9 (64-bit)";
        case EM_IA_64:      return "Intel Itanium (IA-64)";
        case EM_X86_64:     return "AMD x86-64";
        case EM_AARCH64:    return "ARM AArch64";
        case EM_RISCV:      return "RISC-V";
        case EM_BPF:        return "Linux BPF VM";
        case EM_LOONGARCH:  return "LoongArch";
        case EM_MSP430:     return "TI MSP430";
        case EM_BLACKFIN:   return "Analog Devices Blackfin";
        case EM_AVR:        return "Atmel AVR 8-bit";
        case EM_CRIS:       return "Axis CRIS";
        case EM_TILEGX:     return "Tilera TILE-Gx";
        case EM_MICROBLAZE: return "Xilinx MicroBlaze";
        case EM_CUDA:       return "NVIDIA CUDA";
        case EM_AMDGPU:     return "AMD GPU";
        case EM_RL78:       return "Renesas RL78";
        case EM_OPENRISC:   return "OpenRISC";
        case EM_XTENSA:     return "Tensilica Xtensa";
        case EM_Z80:        return "Zilog Z80";
        case EM_VAX:        return "DEC VAX";
        case EM_M32R:       return "Mitsubishi M32R";
        case EM_MN10300:    return "Matsushita MN10300";
        case EM_MN10200:    return "Matsushita MN10200";
        case EM_PJ:         return "picoJava";
        default:            return "Unknown/Unsupported machine";
    }
}

const char* sh_type_to_str(unsigned int sh_type)
{
    char* result;
    switch (sh_type) {
        case SHT_NULL:          result = "NULL"; break;
        case SHT_PROGBITS:      result = "PROGBITS"; break;
        case SHT_SYMTAB:        result = "SYMTAB"; break;
        case SHT_STRTAB:        result = "STRTAB"; break;
        case SHT_RELA:          result = "RELA"; break;
        case SHT_HASH:          result = "HASH"; break;
        case SHT_DYNAMIC:       result = "DYNAMIC"; break;
        case SHT_NOTE:          result = "NOTE"; break;
        case SHT_NOBITS:        result = "NOBITS"; break;
        case SHT_REL:           result = "REL"; break;
        case SHT_SHLIB:         result = "SHLIB"; break;
        case SHT_DYNSYM:        result = "DYNSYM"; break;
        case SHT_INIT_ARRAY:    result = "INIT_ARRAY"; break;
        case SHT_FINI_ARRAY:    result = "FINI_ARRAY"; break;
        case SHT_PREINIT_ARRAY: result = "PREINIT_ARRAY"; break;
        case SHT_GROUP:         result = "GROUP"; break;
        case SHT_SYMTAB_SHNDX:  result = "SYMTAB_SHNDX"; break;
        case SHT_RELR:          result = "RELR"; break;

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
        case PT_NULL:         type_str = "NULL"; break;
        case PT_LOAD:         type_str = "LOAD"; break;
        case PT_DYNAMIC:      type_str = "DYNAMIC"; break;
        case PT_INTERP:       type_str = "INTERP"; break;
        case PT_NOTE:         type_str = "NOTE"; break;
        case PT_SHLIB:        type_str = "SHLIB"; break;
        case PT_PHDR:         type_str = "PHDR"; break;
        case PT_TLS:          type_str = "TLS"; break;
        case PT_GNU_EH_FRAME: type_str = "GNU_EH_FRAME"; break;
        case PT_GNU_STACK:    type_str = "GNU_STACK"; break;
        case PT_GNU_RELRO:    type_str = "GNU_RELRO"; break;
        case PT_GNU_PROPERTY: type_str = "GNU_PROPERTY"; break;
        case PT_SUNWBSS:      type_str = "SUNWBSS"; break;
        case PT_SUNWSTACK:    type_str = "SUNWSTACK"; break;
        default:
                              if (p_type >= PT_LOOS && p_type <= PT_HIOS)
                                  type_str = "OS-SPECIFIC";
                              else if (p_type >= PT_LOPROC && p_type <= PT_HIPROC)
                                  type_str = "PROC-SPECIFIC";
                              else
                                  type_str = "UNKNOWN";
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


