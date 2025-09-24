/**
 * @file b_elf_metadata.c
 * @brief Functions for parsing and printing ELF file metadata.
 *
 * This module provides utilities to extract and display metadata
 * from ELF binaries. It determines the ELF class (32-bit or 64-bit),
 * endianness, and dispatches specialized functions to dump headers:
 * - ELF headers
 * - Section headers
 * - Program headers
 *
 * The output is formatted with colors for readability.
 *
 * @note This file depends on:
 * - @ref bparser for memory access
 * - ELF structures (Elf32_Ehdr, Elf32_Phdr, Elf32_Shdr, etc.)
 * - Helper functions like @ref dump_elf32hdr(), @ref dump_elf64hdr(),
 *   @ref dump_elf32_shdr(), @ref dump_elf64_shdr(),
 *   @ref dump_elf32_phdr(), @ref dump_elf64_phdr()
 *
 */
#include "b_elf_metadata.h"
#include "../b_hashmap/b_hashmap.h"
#include <elf.h>
#include <string.h>
#define META_LABEL_WIDTH -10

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
inline void display_byte(const unsigned char *byte)
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


// ========================= BEGIN ELF HEADER ==================================
/**
 * @brief Dump the ELF header information for a 32-bit ELF file.
 *
 * This function prints human-readable information about the provided
 * 32-bit ELF header, including:
 * - The ELF class (32-bit)
 * - Entry point address
 * - Program header count and offset
 * - Section header count and offset
 * - Section header string table index
 * - File type and machine type (both as strings and raw values)
 *
 * @param elf Pointer to an Elf32_Ehdr structure representing the ELF header.
 *
 * @note The output is printed to standard output with color formatting.
 * @see dump_elf64hdr()
 */
void dump_elf32hdr(Elf32_Ehdr* elf)
{
    printf(COLOR_GREEN "Class: " COLOR_RESET "32-bit\n");
    printf(COLOR_GREEN "Entry point: " COLOR_RESET "0x%x\n", elf->e_entry);
    printf(COLOR_GREEN "Program headers: " COLOR_RESET "%d (offset: 0x%x)\n", elf->e_phnum, elf->e_phoff);
    printf(COLOR_GREEN "Section headers: " COLOR_RESET "%d (offset: 0x%x)\n", elf->e_shnum, elf->e_shoff);
    printf(COLOR_GREEN "Section header string table index: " COLOR_RESET "%d\n", elf->e_shstrndx);
    printf(COLOR_GREEN "File Type: " COLOR_RESET "%s (%d)\n",
            elf_type_to_str(elf->e_type), elf->e_type);
    printf(COLOR_GREEN "Machine: " COLOR_RESET "%s (%d)\n",
            elf_machine_to_str(elf->e_machine), elf->e_machine);
}

/**
 * @brief Dump the ELF header information for a 64-bit ELF file.
 *
 * This function prints human-readable information about the provided
 * 64-bit ELF header, including:
 * - The ELF class (64-bit)
 * - Entry point address
 * - Program header count and offset
 * - Section header count and offset
 * - Section header string table index
 * - File type and machine type (both as strings and raw values)
 *
 * @param elf Pointer to an Elf64_Ehdr structure representing the ELF header.
 *
 * @note The output is printed to standard output with color formatting.
 * @see dump_elf32hdr()
 */
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
// ========================= END ELF HEADER ==================================

// ========================= BEGIN SECTION HEADER ==================================

/**
 * @brief Format ELF section header flags into a colored string.
 *
 * @param sh_flags Section header flags (bitmask).
 * @param buf Buffer to write formatted flags into.
 * @param size Size of the buffer.
 */
static inline void format_sh_flags(uint64_t sh_flags, char *buf, size_t size) {
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
static inline void print_symbols_32bit(bparser* parser, Elf32_Ehdr* elf, Elf32_Shdr* shdrs, Elf32_Shdr *symtab, Elf32_Shdr *strtab) 
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
static inline void print_symbols_64bit(bparser* parser, Elf64_Ehdr* elf, Elf64_Shdr* shdrs, Elf64_Shdr *symtab, Elf64_Shdr *strtab) 
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

// inline function to print hex header row
static inline void print_hex_header(unsigned long long offset) {
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
    // printf(COLOR_GREEN "|----0x%08llx: " COLOR_RESET, offset);
    // printf(" ");
}


static inline void print_section_header_metadata_32bit(unsigned int id, const char* name, const char*type_str, const char* flags, Elf32_Shdr* shdrs) 
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

static inline void print_section_header_metadata_64bit(unsigned int id, const char* name, const char*type_str, const char* flags, Elf64_Shdr* shdrs) 
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

static inline void print_program_header_metadata_32bit(unsigned int id, const char*type_str, const char* flags, Elf32_Phdr* phdr) 
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

static inline void print_program_header_metadata_64bit(unsigned int id, const char*type_str, const char* flags, Elf64_Phdr* phdr) 
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

void print_body_bytes(unsigned char *ptr, size_t size, unsigned long long offset, int disasm, unsigned char bit_type) {
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
            printf(COLOR_YELLOW "|----0x%08llx:  " COLOR_RESET " %s\n",
                    (unsigned long long)ud_insn_off(&ud_obj),
                    ud_insn_asm(&ud_obj));
        }
    }


}

/**
 * @brief Dump the section header table of a 32-bit ELF file.
 *
 * This function iterates over the section headers of a 32-bit ELF file
 * and prints human-readable information for each section.  
 *
 * Printed metadata includes:
 * - Section index (ID)
 * - Section name (resolved from the section string table)
 * - Section type (as string and numeric)
 * - Flags (W/A/X)
 * - Virtual address, file offset, and section size
 * - Link, Info, Alignment, and Entry size
 *
 * Raw section bytes:
 * - If a section has size > 0, its contents are read from the ELF file
 *   using @ref bparser_read and printed in hexadecimal format.
 *
 * @param elf Pointer to the ELF header (Elf32_Ehdr).
 * @param shdrs Pointer to the section header table (array of Elf32_Shdr).
 * @param parser Pointer to a bparser structure that provides access to the file data.
 *
 * @note Output is color formatted and written to standard output.
 * @see dump_elf64_shdr()
 */
void dump_elf32_shdr(Elf32_Ehdr* elf, Elf32_Shdr* shdrs, bparser* parser) 
{
    Elf32_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(parser->block + shstr.sh_offset);

    printf(COLOR_BLUE "\n=== Section Headers ===\n" COLOR_RESET);
    print_section_header_legend();

    // create hashmap of section headers to retreve the specifa name of section header needed
    hashmap_t *map = create_map();
    
    for (int i = 0; i < elf->e_shnum; i++) {
        // ============================ BEGIN SECTION METADATA =============================
        // Section name
        const char* name = &shstrtab[shdrs[i].sh_name];

        // Type
        const char* type_str = sh_type_to_str(shdrs[i].sh_type);

        // Insert section header pointers into a hashmap for quick retrieval by name
        insert(map, name, &shdrs[i]);

        // Flags
        char flags[64] = "";
        format_sh_flags(shdrs[i].sh_flags, flags, sizeof(flags));
        printf("\n");

        print_section_header_metadata_32bit(i, name, type_str, flags, shdrs);
        // ============================ END SECTION METADATA =============================
        

        // ============================ BEGIN SECTION BODY =============================
        long long int offset = shdrs[i].sh_offset;
        if (shdrs[i].sh_size > 0) {
            void* block = malloc(shdrs[i].sh_size);
            bparser_read(parser, block, shdrs[i].sh_offset, shdrs[i].sh_size);
            unsigned char* ptr = (unsigned char*)block;
            unsigned char bit_type = ((unsigned char*)parser->block)[EI_CLASS];
            print_body_bytes(ptr, shdrs[i].sh_size, shdrs[i].sh_offset, shdrs[i].sh_flags, bit_type);
            free(block);
        }
        // ============================ END SECTION BODY =============================
    }

    Elf32_Shdr *symtab, *strtab;
    if((symtab = (Elf32_Shdr*)get(map, ".dynsym")) != NULL && (strtab = (Elf32_Shdr*)get(map, ".dynstr")) != NULL) {
        print_symbols_32bit(parser, elf, shdrs, symtab, strtab);
        printf("\n\n");
    }

    if((symtab = (Elf32_Shdr*)get(map, ".symtab")) != NULL && (strtab = (Elf32_Shdr*)get(map, ".strtab")) != NULL) {
        print_symbols_32bit(parser, elf, shdrs, symtab, strtab);
    }

    free_map(map);
}

/**
 * @brief Dump the section header table of a 64-bit ELF file.
 *
 * This function iterates over the section headers of a 64-bit ELF file
 * and prints human-readable information for each section.  
 *
 * Printed metadata includes:
 * - Section index (ID)
 * - Section name (resolved from the section string table)
 * - Section type (as string and numeric)
 * - Flags (W/A/X)
 * - Virtual address, file offset, and section size
 * - Link, Info, Alignment, and Entry size
 *
 * Raw section bytes:
 * - If a section has size > 0, its contents are read from the ELF file
 *   using @ref bparser_read and printed in hexadecimal format.
 *
 * @param elf Pointer to the ELF header (Elf64_Ehdr).
 * @param shdrs Pointer to the section header table (array of Elf64_Shdr).
 * @param parser Pointer to a bparser structure that provides access to the file data.
 *
 * @note Output is color formatted and written to standard output.
 * @see dump_elf32_shdr()
 */
void dump_elf64_shdr(Elf64_Ehdr* elf , Elf64_Shdr* shdrs, bparser* parser) 
{
    Elf64_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(parser->block + shstr.sh_offset);
    printf(COLOR_BLUE "\n=== Section Headers ===\n" COLOR_RESET);
    print_section_header_legend();

    // create hashmap of section headers to retreve the specifa name of section header needed
    hashmap_t *map = create_map();

    for (int i = 0; i < elf->e_shnum; i++) {
        // ============================ BEGIN SECTION METADATA =============================
        // Section name
        const char* name = &shstrtab[shdrs[i].sh_name];

        // Type
        const char* type_str = sh_type_to_str(shdrs[i].sh_type);

        // Insert section header pointers into a hashmap for quick retrieval by name
        insert(map, name, &shdrs[i]);

        // Flags
        char flags[64] = "";
        format_sh_flags(shdrs[i].sh_flags, flags, sizeof(flags));
        printf("\n");       

        print_section_header_metadata_64bit(i, name, type_str, flags, shdrs);
        // ============================ END SECTION METADATA =============================


        // ============================ BEGIN SECTION BODY =============================
        if (shdrs[i].sh_size > 0) {
            void* block = malloc(shdrs[i].sh_size);
            bparser_read(parser, block, shdrs[i].sh_offset, shdrs[i].sh_size);
            unsigned char* ptr = (unsigned char*)block;
            // print_body_bytes(ptr, shdrs[i].sh_size, shdrs[i].sh_offset, shdrs[i].sh_flags);
            unsigned char bit_type = ((unsigned char*)parser->block)[EI_CLASS];
            print_body_bytes(ptr, shdrs[i].sh_size, shdrs[i].sh_offset, shdrs[i].sh_flags, bit_type);
            free(block);
        }
        // ============================ END SECTION BODY =============================
    }

    Elf64_Shdr *symtab, *strtab;
    if((symtab = (Elf64_Shdr*)get(map, ".dynsym")) != NULL && (strtab = (Elf64_Shdr*)get(map, ".dynstr")) != NULL) {
        print_symbols_64bit(parser, elf, shdrs, symtab, strtab);
        printf("\n\n");
    }

    if((symtab = (Elf64_Shdr*)get(map, ".symtab")) != NULL && (strtab = (Elf64_Shdr*)get(map, ".strtab")) != NULL) {
        print_symbols_64bit(parser, elf, shdrs, symtab, strtab);
    }




    free_map(map);
}
// ========================= END SECTION ==================================




// ========================= BEGIN PROGRAM HEADER ==================================
void format_p_flags(uint32_t p_flags, char *buf, size_t size) {
    buf[0] = '\0';
    if (p_flags & PF_R) strncat(buf, COLOR_GREEN "R" COLOR_RESET, size - strlen(buf) - 1);
    if (p_flags & PF_W) strncat(buf, COLOR_RED "W" COLOR_RESET, size - strlen(buf) - 1);
    if (p_flags & PF_X) strncat(buf, COLOR_YELLOW "X" COLOR_RESET, size - strlen(buf) - 1);
    for(int i=0; i<5; i++)
        strncat(buf, " ", size - strlen(buf) - 1);
    strncat(buf, COLOR_CYAN , size - strlen(buf) - 1);
}

/**
 * @brief Dump the program header table of a 32-bit ELF file.
 *
 * This function iterates over the program headers of a 32-bit ELF file
 * and prints metadata and raw contents for each segment.  
 *
 * Printed metadata includes:
 * - Program header type (human-readable and numeric)
 * - Flags (R/W/X)
 * - Offset
 * - Virtual and physical addresses
 * - File size, memory size, and alignment
 *
 * Special cases:
 * - If the segment type is PT_INTERP, the interpreter path is printed.
 * - If the segment type is PT_DYNAMIC, it indicates the binary is dynamically linked.
 *
 * Raw segment bytes:
 * - If a segment has file size > 0, its raw bytes are read from the ELF file
 *   using @ref bparser_read and printed in hexadecimal format.
 *
 * @param elf Pointer to the ELF header (Elf32_Ehdr).
 * @param phdr Pointer to the program header table (array of Elf32_Phdr).
 * @param parser Pointer to a bparser structure that provides access to the file data.
 *
 * @note Output is color formatted and written to standard output.
 * @see dump_elf64_phdr()
 */
void dump_elf32_phdr(Elf32_Ehdr *elf, Elf32_Phdr* phdr, bparser*parser)
{
    printf(COLOR_BLUE "\n=== Program Headers ===\n" COLOR_RESET);
    print_program_header_legend();

    for (int i = 0; i < elf->e_phnum; i++) {
        // ============================ BEGIN PROGRAM METADATA =============================
        const char *type_str = type_p_to_str(phdr[i].p_type);
        char flags[64];
        format_p_flags(phdr[i].p_flags, flags, sizeof(flags));

        printf("\n");
        print_program_header_metadata_32bit(i, type_str, flags, phdr);

        if (phdr[i].p_type == PT_INTERP) {
            char *interp = (char*)(parser->block + phdr[i].p_offset);
            printf(COLOR_GREEN "|---%-*s" COLOR_RESET   COLOR_YELLOW "%s\n" COLOR_GREEN , META_LABEL_WIDTH, "Interpreter: ", interp);
        }
        if (phdr[i].p_type == PT_DYNAMIC) {
            printf(COLOR_YELLOW "|---%-*s" COLOR_RESET "\n" COLOR_RESET, META_LABEL_WIDTH, "Dynamically linked");
        }

        // ============================ END PROGRAM METADATA ============================

        // ============================ BEGIN PROGRAM BODY =============================
        if (phdr[i].p_filesz > 0) {
            void *block = malloc(phdr[i].p_filesz);
            bparser_read(parser, block, phdr[i].p_offset, phdr[i].p_filesz);
            unsigned char* ptr = (unsigned char*)block;
            print_body_bytes(ptr, phdr[i].p_filesz, phdr[i].p_offset, 0, 0);
            // print_body_bytes(ptr, phdr[i].p_filesz, phdr[i].p_offset, phdr[i].p_flags);
            free(block);
        }
        // ============================ END PROGRAM BODY =============================

    }
}

/**
 * @brief Dump the program header table of a 64-bit ELF file.
 *
 * This function iterates over the program headers of a 64-bit ELF file
 * and prints metadata and raw contents for each segment.  
 *
 * Printed metadata includes:
 * - Program header type (human-readable and numeric)
 * - Flags (R/W/X)
 * - Offset
 * - Virtual and physical addresses
 * - File size, memory size, and alignment
 *
 * Special cases:
 * - If the segment type is PT_INTERP, the interpreter path is printed.
 * - If the segment type is PT_DYNAMIC, it indicates the binary is dynamically linked.
 *
 * Raw segment bytes:
 * - If a segment has file size > 0, its raw bytes are read from the ELF file
 *   using @ref bparser_read and printed in hexadecimal format.
 *
 * @param elf Pointer to the ELF header (Elf64_Ehdr).
 * @param phdr Pointer to the program header table (array of Elf64_Phdr).
 * @param parser Pointer to a bparser structure that provides access to the file data.
 *
 * @note Output is color formatted and written to standard output.
 * @see dump_elf32_phdr()
 */
void dump_elf64_phdr(Elf64_Ehdr *elf, Elf64_Phdr* phdr, bparser*parser)
{
    printf(COLOR_BLUE "\n=== Program Headers ===\n" COLOR_RESET);
    print_program_header_legend();

    for (int i = 0; i < elf->e_phnum; i++) {
        // ============================ BEGIN PROGRAM METADATA =============================
        const char *type_str = type_p_to_str(phdr[i].p_type);
        char flags[64];
        format_p_flags(phdr[i].p_flags, flags, sizeof(flags));
        print_program_header_metadata_64bit(i, type_str, flags, phdr);

        if (phdr[i].p_type == PT_INTERP) {
            char *interp = (char*)(parser->block + phdr[i].p_offset);
            printf(COLOR_GREEN "|---%-*s" COLOR_RESET   COLOR_YELLOW "%s\n" COLOR_GREEN , META_LABEL_WIDTH, "Interpreter: ", interp);
        }
        if (phdr[i].p_type == PT_DYNAMIC) {
            printf(COLOR_YELLOW "|---%-*s" COLOR_RESET "\n" COLOR_RESET, META_LABEL_WIDTH, "Dynamically linked");
        }

        // ============================ END PROGRAM METADATA =============================
        
        // ============================ BEGIN PROGRAM BODY =============================
        if (phdr[i].p_filesz > 0) {
            void *block = malloc(phdr[i].p_filesz);
            bparser_read(parser, block, phdr[i].p_offset, phdr[i].p_filesz);
            unsigned char* ptr = (unsigned char*)block;
            // print_body_bytes(ptr, phdr[i].p_filesz, phdr[i].p_offset, phdr[i].p_flags);
            print_body_bytes(ptr, phdr[i].p_filesz, phdr[i].p_offset, 0, 0);
            free(block);
        }
        // ============================ END PROGRAM BODY =============================
    }
}
// ========================= END PROGRAM HEADER ==================================

/**
 * @brief Print high-level metadata of an ELF file and dispatch detailed dump functions.
 *
 * This function reads the ELF identification bytes from the parser source memory
 * to determine:
 * - Endianness (Little Endian, Big Endian, or Unknown)
 * - ELF class (32-bit or 64-bit)
 *
 * Based on the ELF class, it parses the ELF header, program headers, and section headers,
 * then calls the corresponding dump functions:
 * - For 32-bit ELF: @ref dump_elf32hdr(), @ref dump_elf32_shdr(), @ref dump_elf32_phdr()
 * - For 64-bit ELF: @ref dump_elf64hdr(), @ref dump_elf64_shdr(), @ref dump_elf64_phdr()
 *
 * @param parser Pointer to a @ref bparser structure containing the ELF file in memory.
 * @param args   Optional arguments (currently unused).
 *
 * @return true if the ELF class was recognized and processed (32-bit or 64-bit),
 *         false otherwise (invalid or unknown ELF class).
 *
 * @note Output is color formatted and written to standard output.
 * @see dump_elf32hdr(), dump_elf32_shdr(), dump_elf32_phdr(),
 *      dump_elf64hdr(), dump_elf64_shdr(), dump_elf64_phdr()
 */

// #define (n, pfsz, value) \
//     Elf##pfsz_Shdr *n = #value;
// READ_BYTES(md, 64)


bool print_meta_data(bparser* parser, void* args) {
    unsigned char *data = (unsigned char*) parser->block;
    char bit_type = data[EI_CLASS];
    char endian   = data[EI_DATA];

    printf(COLOR_BLUE "=== ELF Metadata ===\n" COLOR_RESET);

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
        Elf32_Phdr* phdr = (Elf32_Phdr*) (data + elf->e_phoff);
        Elf32_Shdr* shdrs = (Elf32_Shdr*)(data + elf->e_shoff);
        dump_elf32hdr(elf);

        if(elf->e_shnum > 0)
            dump_elf32_shdr(elf, shdrs, parser);

        if(elf->e_phnum > 0)
            dump_elf32_phdr(elf, phdr, parser);

    } else if (bit_type == ELFCLASS64) {
        Elf64_Ehdr* elf = (Elf64_Ehdr*) data;
        Elf64_Phdr* phdr = (Elf64_Phdr*) (data + elf->e_phoff);
        Elf64_Shdr* shdrs = (Elf64_Shdr*)(data + elf->e_shoff);
        dump_elf64hdr(elf);
        if(elf->e_shnum > 0)
            dump_elf64_shdr(elf, shdrs, parser);

        if(elf->e_phnum > 0)
            dump_elf64_phdr(elf, phdr, parser);

    } else {
        printf(COLOR_RED "Unknown ELF class: %d\n" COLOR_RESET, bit_type);
        return false;
    }

    // printf(COLOR_BLUE "=========================\n" COLOR_RESET);
    return true;
}

