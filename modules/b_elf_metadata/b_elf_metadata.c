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

    if((symtab = (Elf32_Shdr*)get(map, ".rel.plt")) != NULL) {
        print_rela_plt_32bit(parser, elf, shdrs, symtab, strtab);
    }

    if((symtab = (Elf32_Shdr*)get(map, ".rela.plt")) != NULL) {
        print_rela_plt_32bit(parser, elf, shdrs, symtab, strtab);
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

    if((symtab = (Elf64_Shdr*)get(map, ".rel.plt")) != NULL) {
        print_rela_plt_64bit(parser, elf, shdrs, symtab, strtab);
    }

    if((symtab = (Elf64_Shdr*)get(map, ".rela.plt")) != NULL) {
        print_rela_plt_64bit(parser, elf, shdrs, symtab, strtab);
    }




    free_map(map);
}
// ========================= END SECTION ==================================




// ========================= BEGIN PROGRAM HEADER ==================================
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

