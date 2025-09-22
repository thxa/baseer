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
void format_sh_flags(uint64_t sh_flags, char *buf, size_t size) {
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
 * @brief Print ELF32 symbols from the given symbol and string table sections.
 *
 * @param parser Pointer to the ELF parser structure (contains mapped file).
 * @param symtab Pointer to the symbol table section header.
 * @param strtab Pointer to the string table section header.
 */
void print_symbols_32bit(bparser* parser, Elf32_Ehdr* elf, Elf32_Shdr* shdrs, Elf32_Shdr *symtab, Elf32_Shdr *strtab) 
{
    Elf32_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(parser->block + shstr.sh_offset);

    const char* name = &shstrtab[symtab -> sh_name];
    Elf32_Sym *syms = (Elf32_Sym *)(parser->block + symtab->sh_offset);
    const char *strs = (const char *)(parser->block + strtab->sh_offset);

    int count = symtab->sh_size / sizeof(Elf32_Sym);

    printf(COLOR_YELLOW "\n=== Symbols (%s) ===\n" COLOR_RESET, name);
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
 * @brief Print ELF64 symbols from the given symbol and string table sections.
 *
 * @param parser Pointer to the ELF parser structure (contains mapped file).
 * @param symtab Pointer to the symbol table section header.
 * @param strtab Pointer to the string table section header.
 */
void print_symbols_64bit(bparser* parser, Elf64_Shdr *symtab, Elf64_Shdr *strtab) 
{
    Elf64_Sym *syms = (Elf64_Sym *)(parser->block + symtab->sh_offset);
    const char *strs = (const char *)(parser->block + strtab->sh_offset);
    int count = symtab->sh_size / sizeof(Elf64_Sym);

    printf(COLOR_YELLOW "\n=== Symbols (.symtab) ===\n" COLOR_RESET);
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
    // left padding
    printf("%13s", "");

    // digits 0-9
    printf(COLOR_GREEN);
    for (int i = 0; i <= 9; i++) {
        printf("%d", i);
        printf((i & 1) ? "  " : " ");
    }

    // letters A-F
    for (char c = 'A'; c <= 'F'; c++) {
        printf("%c", c);
        printf((c & 1) ? " " : "  ");
    }
    printf(COLOR_RESET "\n");

    // offset row start
    printf(COLOR_GREEN "0x%08llx: " COLOR_RESET, offset);
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

        // printf(COLOR_GREEN  "%-3s %-20s %-10s %-6s %-10s %-10s %-8s %-5s %-5s %-10s %-10s\n"  COLOR_RESET,
        //         "ID", "Name", "Type", "Flags", "Addr", "Offset", "Size", "Link", "Info", "Align", "EntSize");
        // printf(COLOR_CYAN "%-3d " COLOR_BLUE "%-20s " COLOR_CYAN "%-10s %-6s 0x%08x 0x%08x 0x%08x %-5d %-5d 0x%08x 0x%08x\n" COLOR_RESET,
        //         i, name, type_str, flags,
        //         shdrs[i].sh_addr,
        //         shdrs[i].sh_offset,
        //         shdrs[i].sh_size,
        //         shdrs[i].sh_link,
        //         shdrs[i].sh_info,
        //         shdrs[i].sh_addralign,
        //         shdrs[i].sh_entsize);

        // printf("--------------------\n");

        printf("\n");       
        printf(COLOR_CYAN "|--Section [%d] " COLOR_BLUE "%s" COLOR_CYAN ":\n" COLOR_RESET, i, name);
        printf(COLOR_GREEN "|---%-*s" COLOR_RESET "%d\n",   META_LABEL_WIDTH, "ID:", i);
        printf(COLOR_GREEN "|---%-*s" COLOR_RESET "%s\n",   META_LABEL_WIDTH, "Name:", name);
        printf(COLOR_GREEN "|---%-*s" COLOR_RESET "%s\n",   META_LABEL_WIDTH, "Type:", type_str);
        printf(COLOR_GREEN "|---%-*s" COLOR_RESET "%s\n",   META_LABEL_WIDTH, "Flags:", flags);
        printf(COLOR_GREEN "|---%-*s" COLOR_RESET "0x%x\n", META_LABEL_WIDTH, "Addr:", shdrs[i].sh_addr);
        printf(COLOR_GREEN "|---%-*s" COLOR_RESET "0x%x\n", META_LABEL_WIDTH, "Offset:", shdrs[i].sh_offset);
        printf(COLOR_GREEN "|---%-*s" COLOR_RESET "0x%x\n", META_LABEL_WIDTH, "Size:", shdrs[i].sh_size);
        printf(COLOR_GREEN "|---%-*s" COLOR_RESET "%d\n",   META_LABEL_WIDTH, "Link:", shdrs[i].sh_link);
        printf(COLOR_GREEN "|---%-*s" COLOR_RESET "0x%x\n", META_LABEL_WIDTH, "Info:", shdrs[i].sh_info);
        printf(COLOR_GREEN "|---%-*s" COLOR_RESET "0x%x\n", META_LABEL_WIDTH, "Align:", shdrs[i].sh_addralign);
        printf(COLOR_GREEN "|---%-*s" COLOR_RESET "0x%x\n", META_LABEL_WIDTH, "EntSize:", shdrs[i].sh_entsize);
        // ============================ END SECTION METADATA =============================
        

        // ============================ BEGIN SECTION BODY =============================
        long long int offset = shdrs[i].sh_offset;
        if (shdrs[i].sh_size > 0) {
            void* block = malloc(shdrs[i].sh_size);
            bparser_read(parser, block, shdrs[i].sh_offset, shdrs[i].sh_size);

            // printf(COLOR_CYAN "Section [%d] " COLOR_BLUE "%s" COLOR_CYAN " bytes:\n" COLOR_RESET, i, name);
            unsigned char* ptr = (unsigned char*)block;
            int cnt=0;
            printf(COLOR_GREEN "|" COLOR_RESET  );
            printf("%17s", "");
            printf(COLOR_GREEN);
            for(int i=0; i<=9; i++) {
                printf("%d", i);
                printf((i & 1)?"  ":" ");
            }
            for(char c='A'; c <= 'F'; c++){
                printf("%c", c);
                printf((c & 1)?" ":"  ");
            }

            printf(COLOR_RESET);
            printf("\n");
            printf(COLOR_GREEN "|----0x%08llx: " COLOR_RESET, offset);


            printf(" ");
            for (size_t j = 0; j < shdrs[i].sh_size; j++) {
                if(cnt == 2) {
                    printf(" ");
                    cnt=0;
                }
                
                display_byte(&ptr[j]);

                if ((j + 1) % BLOCK_LENGTH  == 0){
                    printf("\n");
                    printf(COLOR_GREEN "|----0x%08llx: " COLOR_RESET, offset);
                }

                cnt++;
                offset += 16;
            }
            printf("\n\n");

            free(block);
        }
        // ============================ END SECTION BODY =============================
    }

    Elf32_Shdr *symtab, *strtab;
    if((symtab = (Elf32_Shdr*)get(map, ".dynsym")) != NULL && (strtab = (Elf32_Shdr*)get(map, ".dynstr")) != NULL) {
        print_symbols_32bit(parser, elf, shdrs, symtab, strtab);
    }

    printf("\n\n");

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

        printf(COLOR_GREEN  "%-3s %-20s %-10s %-6s %-10s %-10s %-8s %-5s %-5s %-10s %-10s\n"  COLOR_RESET,
                "ID", "Name", "Type", "Flags", "Addr", "Offset", "Size", "Link", "Info", "Align", "EntSize");
        printf(COLOR_CYAN "%-3d " COLOR_BLUE "%-20s " COLOR_CYAN "%-10s %-6s 0x%08lx 0x%08lx 0x%08lx %-5d %-5d 0x%08lx 0x%08lx\n" COLOR_RESET,
                i, name, type_str, flags,
                shdrs[i].sh_addr,
                shdrs[i].sh_offset,
                shdrs[i].sh_size,
                shdrs[i].sh_link,
                shdrs[i].sh_info,
                shdrs[i].sh_addralign,
                shdrs[i].sh_entsize);

        // ============================ END SECTION METADATA =============================
        

        // ============================ BEGIN SECTION BODY =============================
        if (shdrs[i].sh_size > 0) {
            void* block = malloc(shdrs[i].sh_size);
            bparser_read(parser, block, shdrs[i].sh_offset, shdrs[i].sh_size);

            printf(COLOR_CYAN "Section [%d] " COLOR_BLUE "%s" COLOR_CYAN " bytes:\n" COLOR_RESET, i, name);
            unsigned char* ptr = (unsigned char*)block;
            for (size_t j = 0; j < shdrs[i].sh_size; j++) {
                display_byte(&ptr[j]);
                if ((j + 1) % BLOCK_LENGTH  == 0) printf("\n");
            }
            printf("\n\n");

            free(block);
        }
       // ============================ END SECTION BODY =============================
    }


    Elf64_Shdr *symtab, *strtab;
    if((symtab = (Elf64_Shdr*)get(map, ".symtab")) != NULL && (strtab = (Elf64_Shdr*)get(map, ".strtab")) != NULL) {
        print_symbols_64bit(parser, symtab, strtab);
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
        printf(COLOR_GREEN 
                "%-3s %-15s %-8s %-10s %-10s %-10s %-10s %-10s %-10s\n" COLOR_RESET,
                "ID", "Type", "Flags", "Offset", "VirtAddr", "PhysAddr", "FileSz", "MemSz", "Align");
        char flags[64];
        format_p_flags(phdr[i].p_flags, flags, sizeof(flags));
        printf(COLOR_CYAN "%-3d %-15s %-8s 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n" COLOR_RESET,
                i, type_str, flags,
                phdr[i].p_offset,
                phdr[i].p_vaddr,
                phdr[i].p_paddr,
                phdr[i].p_filesz,
                phdr[i].p_memsz,
                phdr[i].p_align);

        if (phdr[i].p_type == PT_INTERP) {
            char *interp = (char*)(parser->block + phdr[i].p_offset);
            printf(COLOR_YELLOW "    Interpreter: " COLOR_RESET "%s\n", interp);
        }
        if (phdr[i].p_type == PT_DYNAMIC) {
            printf(COLOR_YELLOW "    Dynamically linked\n" COLOR_RESET);
        }
        // ============================ END PROGRAM METADATA =============================

        // ============================ BEGIN PROGRAM BODY =============================
        if (phdr[i].p_filesz > 0) {
            printf(COLOR_CYAN "    Segment [%d] %s raw bytes:\n" COLOR_RESET, i, type_str);

            void *block = malloc(phdr[i].p_filesz);
            bparser_read(parser, block, phdr[i].p_offset, phdr[i].p_filesz);

            unsigned char *ptr = (unsigned char*)block;
            for (size_t j = 0; j < phdr[i].p_filesz; j++) {
                display_byte(&ptr[j]);
                if ((j + 1) % BLOCK_LENGTH == 0) printf("\n");
            }
            printf("\n\n");

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

        printf(COLOR_GREEN 
                "%-3s %-15s %-8s %-10s %-10s %-10s %-10s %-10s %-10s\n" COLOR_RESET,
                "ID", "Type", "Flags", "Offset", "VirtAddr", "PhysAddr", "FileSz", "MemSz", "Align");

        char flags[64];
        format_p_flags(phdr[i].p_flags, flags, sizeof(flags));

        printf(COLOR_CYAN "%-3d %-15s %-8s 0x%08lx 0x%08lx 0x%08lx 0x%08lx 0x%08lx 0x%08lx\n" COLOR_RESET,
                i, type_str, flags,
                phdr[i].p_offset,
                phdr[i].p_vaddr,
                phdr[i].p_paddr,
                phdr[i].p_filesz,
                phdr[i].p_memsz,
                phdr[i].p_align);

        if (phdr[i].p_type == PT_INTERP) {
            char *interp = (char*)(parser->block + phdr[i].p_offset);
            printf(COLOR_YELLOW "    Interpreter: " COLOR_RESET "%s\n", interp);
        }
        if (phdr[i].p_type == PT_DYNAMIC) {
            printf(COLOR_YELLOW "    Dynamically linked\n" COLOR_RESET);
        }

        // ============================ END PROGRAM METADATA =============================
        
        // ============================ BEGIN PROGRAM BODY =============================
        if (phdr[i].p_filesz > 0) {
            printf(COLOR_CYAN "    Segment [%d] %s raw bytes:\n" COLOR_RESET, i, type_str);

            void *block = malloc(phdr[i].p_filesz);
            bparser_read(parser, block, phdr[i].p_offset, phdr[i].p_filesz);

            unsigned char *ptr = (unsigned char*)block;
            for (size_t j = 0; j < phdr[i].p_filesz; j++) {
                display_byte(&ptr[j]);
                if ((j + 1) % BLOCK_LENGTH == 0) printf("\n");
            }
            printf("\n\n");

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
bool print_meta_data(bparser* parser, void* args) {
    unsigned char *data = (unsigned char*) parser->block;
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
        Elf32_Phdr* phdr = (Elf32_Phdr*) (data + elf->e_phoff);
        Elf32_Shdr* shdrs = (Elf32_Shdr*)(data + elf->e_shoff);
        dump_elf32hdr(elf);
        dump_elf32_shdr(elf, shdrs, parser);
        dump_elf32_phdr(elf, phdr, parser);

    } else if (bit_type == ELFCLASS64) {
        Elf64_Ehdr* elf = (Elf64_Ehdr*) data;
        Elf64_Phdr* phdr = (Elf64_Phdr*) (data + elf->e_phoff);
        Elf64_Shdr* shdrs = (Elf64_Shdr*)(data + elf->e_shoff);
        dump_elf64hdr(elf);
        dump_elf64_shdr(elf, shdrs, parser);
        dump_elf64_phdr(elf, phdr, parser);

    } else {
        printf(COLOR_RED "Unknown ELF class: %d\n" COLOR_RESET, bit_type);
        return false;
    }

    // printf(COLOR_BLUE "=========================\n" COLOR_RESET);
    return true;
}

