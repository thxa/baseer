#ifndef BX_ELF_UTILS
#define BX_ELF_UTILS

#include "../bparser/bparser.h"
#include "../../baseer.h"
#include <elf.h>
#include<string.h>


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
// bool is_metadata_section(const char* name);
#endif // !BX_ELF_UTILS
