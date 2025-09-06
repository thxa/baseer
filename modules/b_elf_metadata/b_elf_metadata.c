#include "b_elf_metadata.h"

// #define EI_NIDENT (16)

// typedef struct
// {
//   unsigned char e_ident[EI_NIDENT];     /* Magic number and other info */
//   Elf32_Half    e_type;                 /* Object file type */
//   Elf32_Half    e_machine;              /* Architecture */
//   Elf32_Word    e_version;              /* Object file version */
//   Elf32_Addr    e_entry;                /* Entry point virtual address */
//   Elf32_Off     e_phoff;                /* Program header table file offset */
//   Elf32_Off     e_shoff;                /* Section header table file offset */
//   Elf32_Word    e_flags;                /* Processor-specific flags */
//   Elf32_Half    e_ehsize;               /* ELF header size in bytes */
//   Elf32_Half    e_phentsize;            /* Program header table entry size */
//   Elf32_Half    e_phnum;                /* Program header table entry count */
//   Elf32_Half    e_shentsize;            /* Section header table entry size */
//   Elf32_Half    e_shnum;                /* Section header table entry count */
//   Elf32_Half    e_shstrndx;             /* Section header string table index */
// } Elf32_Ehdr;

// typedef struct
// {
//   unsigned char e_ident[EI_NIDENT];     /* Magic number and other info */
//   Elf64_Half    e_type;                 /* Object file type */
//   Elf64_Half    e_machine;              /* Architecture */
//   Elf64_Word    e_version;              /* Object file version */
//   Elf64_Addr    e_entry;                /* Entry point virtual address */
//   Elf64_Off     e_phoff;                /* Program header table file offset */
//   Elf64_Off     e_shoff;                /* Section header table file offset */
//   Elf64_Word    e_flags;                /* Processor-specific flags */
//   Elf64_Half    e_ehsize;               /* ELF header size in bytes */
//   Elf64_Half    e_phentsize;            /* Program header table entry size */
//   Elf64_Half    e_phnum;                /* Program header table entry count */
//   Elf64_Half    e_shentsize;            /* Section header table entry size */
//   Elf64_Half    e_shnum;                /* Section header table entry count */
//   Elf64_Half    e_shstrndx;             /* Section header string table index */
// } Elf64_Ehdr;

/* Section header.  */
// typedef struct
// {
//   Elf32_Word    sh_name;                /* Section name (string tbl index) */
//   Elf32_Word    sh_type;                /* Section type */
//   Elf32_Word    sh_flags;               /* Section flags */
//   Elf32_Addr    sh_addr;                /* Section virtual addr at execution */
//   Elf32_Off         sh_offset;          /* Section file offset */
//   Elf32_Word    sh_size;                /* Section size in bytes */
//   Elf32_Word    sh_link;                /* Link to another section */
//   Elf32_Word    sh_info;                /* Additional section information */
//   Elf32_Word    sh_addralign;           /* Section alignment */
//   Elf32_Word    sh_entsize;             /* Entry size if section holds table */
// } Elf32_Shdr;

// typedef struct
// {
//   Elf64_Word    sh_name;                /* Section name (string tbl index) */
//   Elf64_Word    sh_type;                /* Section type */
//   Elf64_Xword   sh_flags;               /* Section flags */
//   Elf64_Addr    sh_addr;                /* Section virtual addr at execution */
//   Elf64_Off     sh_offset;              /* Section file offset */
//   Elf64_Xword   sh_size;                /* Section size in bytes */
//   Elf64_Word    sh_link;                /* Link to another section */
//   Elf64_Word    sh_info;                /* Additional section information */
//   Elf64_Xword   sh_addralign;           /* Section alignment */
//   Elf64_Xword   sh_entsize;             /* Entry size if section holds table */
// } Elf64_Shdr;



/* Program segment header.  */

// typedef struct
// {
//   Elf32_Word    p_type;                 /* Segment type */
//   Elf32_Off         p_offset;           /* Segment file offset */
//   Elf32_Addr    p_vaddr;                /* Segment virtual address */
//   Elf32_Addr    p_paddr;                /* Segment physical address */
//   Elf32_Word    p_filesz;               /* Segment size in file */
//   Elf32_Word    p_memsz;                /* Segment size in memory */
//   Elf32_Word    p_flags;                /* Segment flags */
//   Elf32_Word    p_align;                /* Segment alignment */
// } Elf32_Phdr;

// typedef struct
// {
//   Elf64_Word    p_type;                 /* Segment type */
//   Elf64_Word    p_flags;                /* Segment flags */
//   Elf64_Off         p_offset;           /* Segment file offset */
//   Elf64_Addr    p_vaddr;                /* Segment virtual address */
//   Elf64_Addr    p_paddr;                /* Segment physical address */
//   Elf64_Xword   p_filesz;               /* Segment size in file */
//   Elf64_Xword   p_memsz;                /* Segment size in memory */
//   Elf64_Xword   p_align;                /* Segment alignment */
// } Elf64_Phdr;

#define COLOR_RESET      "\033[0m"
#define COLOR_GREEN      "\033[1;32m"
#define COLOR_BLUE       "\033[1;34m"
#define COLOR_YELLOW     "\033[1;33m"
#define COLOR_RED        "\033[1;31m"
#define COLOR_MAGENTA    "\033[35m"
#define COLOR_CYAN       "\033[36m"
#define COLOR_RAND(i, c1, c2) (i & 1)?c1:c2;



const char* elf_machine_to_str(int machine)
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

const char* sh_type_to_str(int sh_type) {
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

// const char* p_type_to_str(int p_type)
// {
//     static char buffer[64];
//     switch (p_type) {
//         case PT_NULL:         snprintf(result, sizeof(buffer), "NULL"); break;
//         case PT_LOAD:         snprintf(result, sizeof(buffer), "LOAD"); break;
//         case PT_DYNAMIC:      snprintf(result, sizeof(buffer), "DYNAMIC"); break;
//         case PT_INTERP:       snprintf(result, sizeof(buffer), "INTERP"); break;
//         case PT_NOTE:         snprintf(result, sizeof(buffer), "NOTE"); break;
//         case PT_SHLIB:        snprintf(result, sizeof(buffer), "SHLIB"); break;
//         case PT_PHDR:         snprintf(result, sizeof(buffer), "PHDR"); break;
//         case PT_TLS:          snprintf(result, sizeof(buffer), "TLS"); break;

//         case PT_GNU_EH_FRAME: snprintf(result, sizeof(buffer), "GNU_EH_FRAME"); break;
//         case PT_GNU_STACK:    snprintf(result, sizeof(buffer), "GNU_STACK"); break;
//         case PT_GNU_RELRO:    snprintf(result, sizeof(buffer), "GNU_RELRO"); break;
//         case PT_GNU_PROPERTY: snprintf(result, sizeof(buffer), "GNU_PROPERTY"); break;

//         case PT_SUNWBSS:      snprintf(result, sizeof(buffer), "SUNWBSS"); break;
//         case PT_SUNWSTACK:    snprintf(result, sizeof(buffer), "SUNWSTACK"); break;

//         default:
//                               if (p_type >= PT_LOOS && p_type <= PT_HIOS)
//                                   snprintf(result, sizeof(buffer), "OS-specific (0x%x)", p_type);
//                               else if (p_type >= PT_LOPROC && p_type <= PT_HIPROC)
//                                   snprintf(result, sizeof(buffer), "Processor-specific (0x%x)", p_type);
//                               else
//                                   snprintf(result, sizeof(buffer), "0x%x", p_type);
//                               break;
//     }
//     return result;
// }



const char* elf_type_to_str(int type)
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


void dump_elf32hdr(Elf32_Ehdr* elf)
{
    printf(COLOR_GREEN "Class: " COLOR_RESET "32-bit\n");
    printf(COLOR_GREEN "Entry point: " COLOR_RESET "0x%x\n", elf->e_entry);
    printf(COLOR_GREEN "Program headers: " COLOR_RESET "%d (offset: 0x%x)\n", elf->e_phnum, elf->e_phoff);
    printf(COLOR_GREEN "Section headers: " COLOR_RESET "%d (offset: 0x%x)\n", elf->e_shnum, elf->e_shoff);
    printf(COLOR_GREEN "Section header string table index: " COLOR_RESET "%d\n", elf->e_shstrndx);
    printf(COLOR_GREEN "File Type: " COLOR_RESET "%s (%d)\n",
            elf_type_to_str(elf->e_type), elf->e_type);
    printf(COLOR_GREEN "Machine: " COLOR_RESET "%d\n", elf->e_machine);
}

void dump_elf64hdr(Elf64_Ehdr *elf)
{
    printf(COLOR_GREEN "Class: " COLOR_RESET "64-bit\n");
    printf(COLOR_GREEN "Entry point: " COLOR_RESET "0x%lx\n", elf->e_entry);
    printf(COLOR_GREEN "Program headers: " COLOR_RESET "%d (offset: 0x%lx)\n", elf->e_phnum, elf->e_phoff);
    printf(COLOR_GREEN "Section headers: " COLOR_RESET "%d (offset: 0x%lx)\n", elf->e_shnum, elf->e_shoff);
    printf(COLOR_GREEN "Section header string table index: " COLOR_RESET "%d\n", elf->e_shstrndx);
    printf(COLOR_GREEN "File Type: " COLOR_RESET "%s (%d)\n",
            elf_type_to_str(elf->e_type), elf->e_type);
    printf(COLOR_GREEN "Machine: " COLOR_RESET "%s (%d)\n",
            elf_machine_to_str(elf->e_machine), elf->e_machine);
}

void dump_elf64_phdr(Elf64_Ehdr *elf, Elf64_Phdr* phdr, void*data)
{
    printf(COLOR_BLUE "\n=== Program Headers ===\n" COLOR_RESET);

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

    for (int i = 0; i < elf->e_phnum; i++) {
        printf(COLOR_GREEN "[%d] Type: " COLOR_RESET, i);

        // char* p_type = p_type_to_str(phdr[i].p_type);
        // printf("%s\n", p_type);
        switch (phdr[i].p_type) {
            case PT_NULL:         printf("NULL"); break;
            case PT_LOAD:         printf("LOAD"); break;
            case PT_DYNAMIC:      printf("DYNAMIC"); break;
            case PT_INTERP:       printf("INTERP"); break;
            case PT_NOTE:         printf("NOTE"); break;
            case PT_SHLIB:        printf("SHLIB"); break;
            case PT_PHDR:         printf("PHDR"); break;
            case PT_TLS:          printf("TLS"); break;

            case PT_GNU_EH_FRAME: printf("GNU_EH_FRAME"); break;
            case PT_GNU_STACK:    printf("GNU_STACK"); break;
            case PT_GNU_RELRO:    printf("GNU_RELRO"); break;
            case PT_GNU_PROPERTY: printf("GNU_PROPERTY"); break;

            case PT_SUNWBSS:      printf("SUNWBSS"); break;
            case PT_SUNWSTACK:    printf("SUNWSTACK"); break;

            default:
              if (phdr[i].p_type >= PT_LOOS && phdr[i].p_type <= PT_HIOS)
                  printf("OS-specific (0x%x)", phdr[i].p_type);
              else if (phdr[i].p_type >= PT_LOPROC && phdr[i].p_type <= PT_HIPROC)
                  printf("Processor-specific (0x%x)", phdr[i].p_type);
              else
                  printf("0x%x", phdr[i].p_type);
              break;
        }

        printf("\n");        // Flags
        printf(COLOR_GREEN "    Flags: " COLOR_RESET);
        if (phdr[i].p_flags & PF_R) printf(COLOR_GREEN "R " COLOR_RESET);
        if (phdr[i].p_flags & PF_W) printf(COLOR_RED "W " COLOR_RESET);
        if (phdr[i].p_flags & PF_X) printf(COLOR_YELLOW "X " COLOR_RESET);
        printf("\n");

        printf(COLOR_GREEN "    Offset: " COLOR_RESET "0x%lx\n", phdr[i].p_offset);
        printf(COLOR_GREEN "    Virtual Address: " COLOR_RESET "0x%lx\n", phdr[i].p_vaddr);
        printf(COLOR_GREEN "    Physical Address: " COLOR_RESET "0x%lx\n", phdr[i].p_paddr);
        printf(COLOR_GREEN "    File Size: " COLOR_RESET "0x%lx\n", phdr[i].p_filesz);
        printf(COLOR_GREEN "    Memory Size: " COLOR_RESET "0x%lx\n", phdr[i].p_memsz);
        printf(COLOR_GREEN "    Alignment: " COLOR_RESET "0x%lx\n", phdr[i].p_align);

        // Special: interpreter
        if (phdr[i].p_type == PT_INTERP) {
            char *interp = (char*)(data + phdr[i].p_offset);
            printf(COLOR_YELLOW "    Interpreter: " COLOR_RESET "%s\n", interp);
        }
        // Special: dynamic
        if (phdr[i].p_type == PT_DYNAMIC) {
            printf(COLOR_YELLOW "    Dynamically linked\n" COLOR_RESET);
        }

        printf("\n");
    }
}

void dump_elf64_shdr(Elf64_Ehdr* elf , Elf64_Shdr* shdrs, void* data) {
    Elf64_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(data + shstr.sh_offset);
    printf(COLOR_BLUE "\n=== Section Headers ===\n" COLOR_RESET);

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



    printf(COLOR_GREEN "%-20s %-10s %-6s %-10s %-10s %-8s %-5s %-5s %-10s %-10s\n" COLOR_RESET,
            "Name", "Type", "Flags", "Addr", "Offset", "Size", "Link", "Info", "Align", "EntSize");

    for (int i = 0; i < elf->e_shnum; i++) {
        const char* name = &shstrtab[shdrs[i].sh_name];

        // Type
        const char* type_str = sh_type_to_str(shdrs[i].sh_type);

        // Flags
        char flags[4] = "";
        if (shdrs[i].sh_flags & SHF_WRITE) strcat(flags, "W");
        if (shdrs[i].sh_flags & SHF_ALLOC) strcat(flags, "A");
        if (shdrs[i].sh_flags & SHF_EXECINSTR) strcat(flags, "X");

        char* color = COLOR_RAND(i, COLOR_CYAN, COLOR_RESET);
        printf("%s%-20s %-10s %-6s 0x%08lx 0x%08lx 0x%08lx %-5d %-5d 0x%08lx 0x%08lx\n" COLOR_RESET,
                color,
                name, type_str, flags,
                shdrs[i].sh_addr,
                shdrs[i].sh_offset,
                shdrs[i].sh_size,
                shdrs[i].sh_link,
                shdrs[i].sh_info,
                shdrs[i].sh_addralign,
                shdrs[i].sh_entsize);
    }
}


bool print_meta_data(bparser* parser, void* args) {
    unsigned char *data = (unsigned char*) parser->source.mem.data;
    char bit_type = data[EI_CLASS];
    char endian   = data[EI_DATA];


    printf(COLOR_BLUE "=== ELF File Metadata ===\n" COLOR_RESET);

    // Endianness
    if (endian == ELFDATA2LSB) {
        printf(COLOR_GREEN "Endianness: " COLOR_RESET "Little Endian\n");
    } else if (endian == ELFDATA2MSB) {
        printf(COLOR_GREEN "Endianness: " COLOR_RESET "Big Endian\n");
    } else {
        printf(COLOR_GREEN "Endianness: " COLOR_RESET "Unknown\n");
    }

    if (bit_type == ELFCLASS32) {
        Elf32_Ehdr* elf = (Elf32_Ehdr*) data;
        dump_elf32hdr(elf);
    } else if (bit_type == ELFCLASS64) {
        Elf64_Ehdr* elf = (Elf64_Ehdr*) data;
        Elf64_Phdr* phdr = (Elf64_Phdr*) (data + elf->e_phoff);
        Elf64_Shdr* shdrs = (Elf64_Shdr*)(data + elf->e_shoff);
        dump_elf64hdr(elf);
        dump_elf64_shdr(elf, shdrs, data);
        dump_elf64_phdr(elf, phdr, data);

    } else {
        printf(COLOR_RED "Unknown ELF class: %d\n" COLOR_RESET, bit_type);
        return false;
    }

    printf(COLOR_BLUE "=========================\n" COLOR_RESET);
    return true;
}


