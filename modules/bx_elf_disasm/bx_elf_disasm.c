/**
 * @file bx_elf_disasm.c
 * @brief Functions for disassembling ELF files and printing metadata.
 */
#include "bx_elf_disasm.h"
#include "../b_hashmap/b_hashmap.h"

// List of metadata sections
const char* metadata_sections[] = {
    ".rela.dyn",
    ".rela.plt",
    ".dynsym",
    ".dynstr",
    ".eh_frame",
    ".eh_frame_hdr",
    ".note.gnu.property",
    ".note.gnu.build-id",
    ".comment",
    ".gnu.version",
    ".interp",
    ".gnu.hash",
    ".gnu.version_r",
    NULL
};

bool is_metadata_section(const char* name) 
{
    for (int i = 0; metadata_sections[i] != NULL; i++) {
        if (strcmp(name, metadata_sections[i]) == 0)
            return true;
    }
    return false;
}

// ========================= BEGIN SECTION HEADER ==================================
/**
 * @brief Disassemble and print the ELF32 section headers.
 *
 * Iterates over all section headers in a 32-bit ELF file.
 * Prints metadata (name, type, flags, address, size, etc.) and
 * disassembles machine code sections using udis86. Metadata sections
 * (e.g., .rela.dyn, .interp) are identified and not disassembled.
 *
 * @param elf Pointer to the ELF32 header.
 * @param shdrs Pointer to the array of ELF32 section headers.
 * @param parser Pointer to a bparser structure for reading binary data.
 */
void dump_disasm_elf32_shdr(Elf32_Ehdr* elf , Elf32_Shdr* shdrs, bparser* parser)
{
    Elf32_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(parser->block + shstr.sh_offset);

    printf(COLOR_BLUE "\n=== Sections ===\n" COLOR_RESET);
    // print_section_header_legend();

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

        if(shdrs[i].sh_flags & SHF_EXECINSTR) {
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
                unsigned char *ptr = (unsigned char*)block;
                unsigned char bit_type = ((unsigned char*)parser->block)[EI_CLASS];
                // print_body_bytes(ptr, shdrs[i].sh_size, shdrs[i].sh_offset, shdrs[i].sh_flags, bit_type);
                print_disasm(ptr, shdrs[i].sh_size, shdrs[i].sh_offset, bit_type);
                free(block);
            }
        }
        // ============================ END SECTION BODY =============================
    }

    Elf32_Shdr *symtab, *strtab;
    // if((symtab = (Elf32_Shdr*)get(map, ".dynsym")) != NULL && (strtab = (Elf32_Shdr*)get(map, ".dynstr")) != NULL) {
    //     print_symbols_32bit(parser, elf, shdrs, symtab, strtab);
    //     printf("\n\n");
    // }

    if((symtab = (Elf32_Shdr*)get(map, ".symtab")) != NULL && (strtab = (Elf32_Shdr*)get(map, ".strtab")) != NULL) {
        print_symbols_with_disasm_32bit(parser, elf, shdrs, symtab, strtab);
    }

    free_map(map);
}

/**
 * @brief Disassemble and print the ELF64 section headers.
 *
 * Iterates over all section headers in a 64-bit ELF file.
 * Prints metadata (name, type, flags, address, size, etc.) and
 * disassembles machine code sections using udis86. Metadata sections
 * (e.g., .rela.dyn, .interp) are identified and not disassembled.
 *
 * @param elf Pointer to the ELF64 header.
 * @param shdrs Pointer to the array of ELF64 section headers.
 * @param parser Pointer to a bparser structure for reading binary data.
 */
void dump_disasm_elf64_shdr(Elf64_Ehdr* elf , Elf64_Shdr* shdrs, bparser* parser)
{
    Elf64_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(parser->block + shstr.sh_offset);

    printf(COLOR_BLUE "\n=== Sections ===\n" COLOR_RESET);
    // print_section_header_legend();

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

        if(shdrs[i].sh_flags & SHF_EXECINSTR) {
            // Flags
            char flags[64] = "";
            format_sh_flags(shdrs[i].sh_flags, flags, sizeof(flags));
            printf("\n");
            print_section_header_metadata_64bit(i, name, type_str, flags, shdrs);
            // ============================ END SECTION METADATA =============================

            // ============================ BEGIN SECTION BODY =============================
            long long int offset = shdrs[i].sh_offset;
            if (shdrs[i].sh_size > 0) {
                void* block = malloc(shdrs[i].sh_size);
                bparser_read(parser, block, shdrs[i].sh_offset, shdrs[i].sh_size);
                unsigned char *ptr = (unsigned char*)block;
                unsigned char bit_type = ((unsigned char*)parser->block)[EI_CLASS];
                // print_body_bytes(ptr, shdrs[i].sh_size, shdrs[i].sh_offset, shdrs[i].sh_flags, bit_type);
                print_disasm(ptr, shdrs[i].sh_size, shdrs[i].sh_offset, bit_type);
                free(block);
            }
        }
        // ============================ END SECTION BODY =============================
    }

    Elf64_Shdr *symtab, *strtab;
    // if((symtab = (Elf64_Shdr*)get(map, ".dynsym")) != NULL && (strtab = (Elf64_Shdr*)get(map, ".dynstr")) != NULL) {
    //     // print_symbols_64bit(parser, elf, shdrs, symtab, strtab);
    //     printf("\n\n");
    // }

    if((symtab = (Elf64_Shdr*)get(map, ".symtab")) != NULL && (strtab = (Elf64_Shdr*)get(map, ".strtab")) != NULL) {
        print_symbols_with_disasm_64bit(parser, elf, shdrs, symtab, strtab);
    }



    free_map(map);
}
// ========================= END SECTION HEADER ==================================




// ========================= BEGIN PROGRAM HEADER ==================================
/**
 * @brief Disassembles and prints the program headers of a 32-bit ELF file.
 *
 * This function iterates through all program headers in a 32-bit ELF file,
 * prints their metadata (type, flags, addresses, sizes), and optionally
 * disassembles the binary contents of loadable segments using udis86.
 *
 * @param elf    Pointer to the ELF header structure (Elf32_Ehdr).
 * @param phdr   Pointer to the first program header (array of Elf32_Phdr).
 * @param parser Pointer to the Baseer parser object used for reading file contents.
 *
 * @details
 * - Prints program header table with columns:
 *   ID, Type, Flags, Offset, VirtAddr, PhysAddr, FileSz, MemSz, Align.
 * - Highlights special segments:
 *   - PT_INTERP: prints interpreter path.
 *   - PT_DYNAMIC: marks the file as dynamically linked.
 * - If a program header contains data (`p_filesz > 0`), its content is read
 *   and disassembled instruction by instruction until either the end of the
 *   segment or a `ret` instruction is encountered.
 *
 * @note Only supports EM_386 and EM_X86_64 architectures at the moment.
 * @warning Stops disassembly early when encountering `ret` (`UD_Iret`).
 */
void dump_disasm_elf32_phdr(Elf32_Ehdr *elf, Elf32_Phdr* phdr, bparser*parser)
{
    printf(COLOR_BLUE "\n=== Program segments ===\n" COLOR_RESET);
    // print_program_header_legend();

    for (int i = 0; i < elf->e_phnum; i++) {
        if(!(phdr[i].p_flags & PF_X)) continue; 
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
            unsigned char bit_type = ((unsigned char*)parser->block)[EI_CLASS];
            print_body_bytes(ptr, phdr[i].p_filesz, phdr[i].p_offset, 0, 0);
            print_disasm(ptr, phdr[i].p_filesz, phdr[i].p_offset, bit_type);
            // print_body_bytes(ptr, phdr[i].p_filesz, phdr[i].p_offset, 0, 0);
            // print_body_bytes(ptr, phdr[i].p_filesz, phdr[i].p_offset, phdr[i].p_flags);
            free(block);
        }
        // ============================ END PROGRAM BODY =============================
    }
}

/**
 * @brief Disassembles and prints the program headers of a 64-bit ELF file.
 *
 * This function iterates through all program headers in a 64-bit ELF file,
 * prints their metadata (type, flags, addresses, sizes), and optionally
 * disassembles the binary contents of loadable segments using udis86.
 *
 * @param elf    Pointer to the ELF header structure (Elf64_Ehdr).
 * @param phdr   Pointer to the first program header (array of Elf64_Phdr).
 * @param parser Pointer to the Baseer parser object used for reading file contents.
 *
 * @details
 * - Prints program header table with columns:
 *   ID, Type, Flags, Offset, VirtAddr, PhysAddr, FileSz, MemSz, Align.
 * - Highlights special segments:
 *   - PT_INTERP: prints interpreter path.
 *   - PT_DYNAMIC: marks the file as dynamically linked.
 * - If a program header contains data (`p_filesz > 0`), its content is read
 *   and disassembled instruction by instruction until either the end of the
 *   segment or a `ret` instruction is encountered.
 *
 * @note Only supports EM_386 and EM_X86_64 architectures at the moment.
 * @warning Stops disassembly early when encountering `ret` (`UD_Iret`).
 */
void dump_disasm_elf64_phdr(Elf64_Ehdr *elf, Elf64_Phdr* phdr, bparser*parser)
{
    printf(COLOR_BLUE "\n=== Program segments ===\n" COLOR_RESET);
    // print_program_header_legend();

    for (int i = 0; i < elf->e_phnum; i++) {
        if(!(phdr[i].p_flags & PF_X)) continue; 
        // ============================ BEGIN PROGRAM METADATA =============================
    
        const char *type_str = type_p_to_str(phdr[i].p_type);
        char flags[64];
        format_p_flags(phdr[i].p_flags, flags, sizeof(flags));

        printf("\n");
        print_program_header_metadata_64bit(i, type_str, flags, phdr);

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
            unsigned char bit_type = ((unsigned char*)parser->block)[EI_CLASS];
            print_body_bytes(ptr, phdr[i].p_filesz, phdr[i].p_offset, 0, 0);
            print_disasm(ptr, phdr[i].p_filesz, phdr[i].p_offset, bit_type);
            // print_body_bytes(ptr, phdr[i].p_filesz, phdr[i].p_offset, phdr[i].p_flags);
            free(block);
        }
        // ============================ END PROGRAM BODY =============================
    }

}
// ========================= END PROGRAM HEADER ==================================



/**
 * @brief Print ELF file disassembly and metadata.
 *
 * This function analyzes an ELF file in memory using a bparser and prints
 * key information including endianness, class (32-bit or 64-bit), entry point,
 * section headers, program headers, file type, and machine type.
 * It calls the appropriate ELF32 or ELF64 section and program header disassembly
 * functions. Only x86 (32-bit) and x86_64 (64-bit) architectures are supported.
 *
 * @param parser Pointer to a bparser structure containing the ELF file in memory.
 * @param args Optional arguments (currently unused).
 * 
 * @return true if the ELF file was successfully analyzed and disassembled; false
 * if the ELF class or machine type is unsupported or unknown.
 */
bool print_elf_disasm(bparser* parser, void* args) {
    unsigned char *data = (unsigned char*) parser->block;
    char bit_type = data[EI_CLASS];
    char endian   = data[EI_DATA];

    printf(COLOR_BLUE "=== ELF File Disasm ===\n" COLOR_RESET);

    // Endianness
    if (endian == ELFDATA2LSB) {
        printf(COLOR_GREEN "Endianness: " COLOR_RESET "Little Endian\n");
    } else if (endian == ELFDATA2MSB) {
        printf(COLOR_GREEN "Endianness: " COLOR_RESET "Big Endian\n");
    } else {
        printf(COLOR_GREEN "Endianness: " COLOR_RESET "Unknown\n");
    }

    if(bit_type == ELFCLASS32) {
        printf(COLOR_GREEN "Class: " COLOR_RESET "32 bit\n");
    } else if(bit_type == ELFCLASS64) {
        printf(COLOR_GREEN "Class: " COLOR_RESET "64 bit\n");
    } 

    if (bit_type == ELFCLASS32) {
        Elf32_Ehdr* elf = (Elf32_Ehdr*) data;
        Elf32_Phdr* phdr = (Elf32_Phdr*) (data + elf->e_phoff);
        Elf32_Shdr* shdrs = (Elf32_Shdr*)(data + elf->e_shoff);

        printf(COLOR_GREEN "Entry point: " COLOR_RESET "0x%x\n", elf->e_entry);
        printf(COLOR_GREEN "Section headers: " COLOR_RESET "%d (offset: 0x%x)\n", elf->e_shnum, elf->e_shoff);
        printf(COLOR_GREEN "Section header string table index: " COLOR_RESET "%d\n", elf->e_shstrndx);
        printf(COLOR_GREEN "File Type: " COLOR_RESET "%s (%d)\n",
                elf_type_to_str(elf->e_type), elf->e_type);



        printf(COLOR_GREEN "Machine: " COLOR_RESET "%s (%d)\n",
                elf_machine_to_str(elf->e_machine), elf->e_machine);


        if(elf->e_machine != EM_X86_64 && elf->e_machine !=  EM_386) {
            printf(COLOR_RED "Not Supported machine: %s\n" COLOR_RESET, elf_machine_to_str(elf->e_machine));
            return false;
        }
      
        dump_disasm_elf32_shdr(elf, shdrs, parser);
        dump_disasm_elf32_phdr(elf, phdr, parser);

    } else if (bit_type == ELFCLASS64) {
        Elf64_Ehdr* elf = (Elf64_Ehdr*) data;
        Elf64_Phdr* phdr = (Elf64_Phdr*) (data + elf->e_phoff);
        Elf64_Shdr* shdrs = (Elf64_Shdr*)(data + elf->e_shoff);

        printf(COLOR_GREEN "Entry point: " COLOR_RESET "0x%x\n", elf->e_entry);
        printf(COLOR_GREEN "Section headers: " COLOR_RESET "%d (offset: 0x%x)\n", elf->e_shnum, elf->e_shoff);
        printf(COLOR_GREEN "Section header string table index: " COLOR_RESET "%d\n", elf->e_shstrndx);
        printf(COLOR_GREEN "File Type: " COLOR_RESET "%s (%d)\n",
                elf_type_to_str(elf->e_type), elf->e_type);
        printf(COLOR_GREEN "Machine: " COLOR_RESET "%s (%d)\n",
                elf_machine_to_str(elf->e_machine), elf->e_machine);

        printf(COLOR_GREEN "Entry point: " COLOR_RESET "0x%x\n", elf->e_entry);

        if(elf->e_machine != EM_X86_64 && elf->e_machine !=  EM_386) {
            printf(COLOR_RED "Not Supported machine: %s\n" COLOR_RESET, elf_machine_to_str(elf->e_machine));
            return false;
        }

        dump_disasm_elf64_shdr(elf, shdrs, parser);
        dump_disasm_elf64_phdr(elf, phdr, parser);

    } else {
        printf(COLOR_RED "Unknown ELF class: %d\n" COLOR_RESET, bit_type);
        return false;
    }
    return true;
}
