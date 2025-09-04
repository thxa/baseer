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




bool bx_elf(bparser* parser, void *arg)
{
    char bit_type = *(((char*)parser -> source.mem.data) + 4);
    if(bit_type == 1) {
        Elf32_Ehdr* elf_header = (Elf32_Ehdr*) parser -> source.mem.data;



    } else if(bit_type == 2){

        Elf64_Ehdr* elf_header = (Elf64_Ehdr*) parser ->source.mem.data;
        printf("Number of program headers: %d\n", elf_header ->e_phnum);
        printf("Number of section headers %d\n", elf_header ->e_shnum);
        if(elf_header ->e_machine == 0x3E) {
            printf("X86-64\n");
        }
        if(elf_header ->e_machine == 0xB7) {
            printf("AArch64\n");
        }
        


    } else {
        printf("The bit type is unknown. Try to fix it first...\n");
        return false;
    }
    // printf
    // printf("Hello New Extension\n");
    // Elf64_Ehdr elf_header* = parser -> ;
    return true;
}


