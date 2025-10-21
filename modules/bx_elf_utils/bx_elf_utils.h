#ifndef BX_ELF_UTILS
#define BX_ELF_UTILS

#include "../bparser/bparser.h"
#include "../../baseer.h"
#include <elf.h>
#include<string.h>
#include "udis86.h"
#include "../../utils/ui.h"

#define META_LABEL_WIDTH -10


typedef struct {
    const char *name;
    const char *desc;
    const char *color;
} legend_entry;

void print_program_header_legend(void);
void print_section_header_legend(void);
const char* elf_machine_to_str(unsigned int machine);
const char* sh_type_to_str(unsigned int sh_type);
const char* elf_type_to_str(unsigned int type);
const char *type_p_to_str(unsigned int p_type);
void print_highlight_asm(const char *asm_instructions);
void print_disasm(unsigned char *ptr, size_t size, unsigned long long offset, unsigned char bit_type);
void print_symbols_with_disasm_32bit(bparser* parser, Elf32_Ehdr* elf, Elf32_Shdr* shdrs, Elf32_Shdr *symtab, Elf32_Shdr *strtab);
void print_symbols_with_disasm_64bit(bparser* parser, Elf64_Ehdr* elf, Elf64_Shdr* shdrs, Elf64_Shdr *symtab, Elf64_Shdr *strtab);

void format_sh_flags(uint64_t sh_flags, char *buf, size_t size);
void print_symbols_32bit(bparser* parser, Elf32_Ehdr* elf, Elf32_Shdr* shdrs, Elf32_Shdr *symtab, Elf32_Shdr *strtab);
void print_symbols_64bit(bparser* parser, Elf64_Ehdr* elf, Elf64_Shdr* shdrs, Elf64_Shdr *reltab, Elf64_Shdr *symtab);
void print_rela_32bit(bparser* parser, Elf32_Ehdr* elf, Elf32_Shdr* shdrs, Elf32_Shdr *reltab, Elf32_Shdr *symtab);
void print_rela_64bit(bparser* parser, Elf64_Ehdr* elf, Elf64_Shdr* shdrs, Elf64_Shdr *symtab, Elf64_Shdr *strtab);
void print_section_header_metadata_32bit(unsigned int id, const char *name, const char *type_str, const char *flags, Elf32_Shdr *shdrs);
void print_section_header_metadata_64bit(unsigned int id, const char *name, const char *type_str, const char *flags, Elf64_Shdr *shdrs);
void print_program_header_metadata_32bit(unsigned int id, const char *type_str, const char *flags, Elf32_Phdr* phdr);
void print_program_header_metadata_64bit(unsigned int id, const char *type_str, const char *flags, Elf64_Phdr* phdr);
void print_body_bytes(unsigned char *ptr, size_t size, unsigned long long offset, int disasm, unsigned char bit_type);
void format_p_flags(uint32_t p_flags, char *buf, size_t size);

// bool is_metadata_section(const char* name);
#endif // !BX_ELF_UTILS
