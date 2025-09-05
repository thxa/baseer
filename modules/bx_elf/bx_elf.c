#include "bx_elf.h"
#include <elf.h>

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

const char* elf_type_to_str(int type)
{
    switch (type) {
        case ET_NONE:   return "No file type";
        case ET_REL:    return "Relocatable file";
        case ET_EXEC:   return "Executable file";
        case ET_DYN:    return "Shared object file";
        case ET_CORE:   return "Core file";
        case ET_NUM:    return "Number of defined types";
        default:
            if (type >= ET_LOOS && type <= ET_HIOS)
                return "OS-specific file type";
            if (type >= ET_LOPROC && type <= ET_HIPROC)
                return "Processor-specific file type";
            return "Unknown file type";
    }
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
    for (int i = 0; i < elf->e_phnum; i++) {
        printf(COLOR_GREEN "[%d] Type: " COLOR_RESET, i);

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
        if (phdr[i].p_flags & PF_R) printf("R ");
        if (phdr[i].p_flags & PF_W) printf("W ");
        if (phdr[i].p_flags & PF_X) printf("X ");
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
    printf(COLOR_GREEN "%-20s %-10s %-6s %-10s %-10s %-8s %-5s %-5s %-10s %-10s\n" COLOR_RESET,
            "Name", "Type", "Flags", "Addr", "Offset", "Size", "Link", "Info", "Align", "EntSize");

    for (int i = 0; i < elf->e_shnum; i++) {
        const char* name = &shstrtab[shdrs[i].sh_name];

        // Type
        const char* type_str;
        switch (shdrs[i].sh_type) {
            case SHT_NULL:          type_str="NULL"; break;
            case SHT_PROGBITS:      type_str="PROGBITS"; break;
            case SHT_SYMTAB:        type_str="SYMTAB"; break;
            case SHT_STRTAB:        type_str="STRTAB"; break;
            case SHT_RELA:          type_str="RELA"; break;
            case SHT_HASH:          type_str="HASH"; break;
            case SHT_DYNAMIC:       type_str="DYNAMIC"; break;
            case SHT_NOTE:          type_str="NOTE"; break;
            case SHT_NOBITS:        type_str="NOBITS"; break;
            case SHT_REL:           type_str="REL"; break;
            case SHT_SHLIB:         type_str="SHLIB"; break;
            case SHT_DYNSYM:        type_str="DYNSYM"; break;
            case SHT_INIT_ARRAY:    type_str="INIT_ARRAY"; break;
            case SHT_FINI_ARRAY:    type_str="FINI_ARRAY"; break;
            case SHT_PREINIT_ARRAY: type_str="PREINIT_ARRAY"; break;
            case SHT_GROUP:         type_str="GROUP"; break;
            case SHT_SYMTAB_SHNDX:  type_str="SYMTAB_SHNDX"; break;
            case SHT_RELR:          type_str="RELR"; break;

            case SHT_GNU_ATTRIBUTES: type_str="GNU_ATTRIBUTES"; break;
            case SHT_GNU_HASH:       type_str="GNU_HASH"; break;
            case SHT_GNU_LIBLIST:    type_str="GNU_LIBLIST"; break;
            case SHT_CHECKSUM:       type_str="CHECKSUM"; break;
            case SHT_GNU_verdef:     type_str="GNU_verdef"; break;
            case SHT_GNU_verneed:    type_str="GNU_verneed"; break;
            case SHT_GNU_versym:     type_str="GNU_versym"; break;

            case SHT_SUNW_move:      type_str="SUNW_move"; break;
            case SHT_SUNW_COMDAT:    type_str="SUNW_COMDAT"; break;
            case SHT_SUNW_syminfo:   type_str="SUNW_syminfo"; break;

    default:
        if (shdrs[i].sh_type >= SHT_LOOS && shdrs[i].sh_type <= SHT_HIOS)
            type_str="OS-specific";
        else if (shdrs[i].sh_type >= SHT_LOPROC && shdrs[i].sh_type <= SHT_HIPROC)
            type_str="Processor-specific";
        else if (shdrs[i].sh_type >= SHT_LOUSER && shdrs[i].sh_type <= SHT_HIUSER)
            type_str="Application-specific";
        else
            type_str="UNKNOWN";
        break;
        }

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

bool bx_elf(bparser* parser, void *arg)
{
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
