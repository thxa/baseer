/**
 * @file bx_elf_disasm.c
 * @brief Functions for disassembling ELF files and printing metadata.
 */
#include "bx_elf_disasm.h"

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
    printf(COLOR_BLUE "\n=== Program Headers ===\n" COLOR_RESET);

    for (int i = 0; i < elf->e_phnum; i++) {

        // ============================ BEGIN PROGRAM METADATA =============================
        const char *type_str = type_p_to_str(phdr[i].p_type);

        char flags[4] = "";
        if (phdr[i].p_flags & PF_R) strcat(flags, "R");
        if (phdr[i].p_flags & PF_W) strcat(flags, "W");
        if (phdr[i].p_flags & PF_X) strcat(flags, "X");


        printf(COLOR_GREEN 
            "%-3s %-15s %-8s %-10s %-10s %-10s %-10s %-10s %-10s\n" 
            COLOR_RESET,
            "ID", "Type", "Flags", "Offset", "VirtAddr", "PhysAddr", "FileSz", "MemSz", "Align");
        printf(COLOR_GREEN "%-3d %-15s %-8s 0x%08lx 0x%08lx 0x%08lx 0x%08lx 0x%08lx 0x%08lx\n" COLOR_RESET,
                i, type_str, flags,
                phdr[i].p_offset,
                phdr[i].p_vaddr,
                phdr[i].p_paddr,
                phdr[i].p_filesz,
                phdr[i].p_memsz,
                phdr[i].p_align);

        if (phdr[i].p_type == PT_INTERP) {
            char *interp = (char*)(parser->source.mem.data + phdr[i].p_offset);
            printf(COLOR_YELLOW "    Interpreter: " COLOR_RESET "%s\n", interp);
        }
        if (phdr[i].p_type == PT_DYNAMIC) {
            printf(COLOR_YELLOW "    Dynamically linked\n" COLOR_RESET);
        }
        // ============================ END PROGRAM METADATA =============================


        // ============================ BEGIN PROGRAM BODY ===============================
        if (phdr[i].p_filesz > 0) {
            void *block = malloc(phdr[i].p_filesz);
            bparser_read(parser, block, phdr[i].p_offset, phdr[i].p_filesz);

            ud_t ud_obj;
            ud_init(&ud_obj);
            ud_set_input_buffer(&ud_obj, (uint8_t*)block, (size_t)phdr[i].p_filesz);
            ud_set_mode(&ud_obj, 32);
            ud_set_syntax(&ud_obj, UD_SYN_INTEL);
            ud_set_pc(&ud_obj, (uint64_t) phdr[i].p_offset);
            while (ud_disassemble(&ud_obj)) {
                printf(COLOR_YELLOW "0x%llx:" COLOR_RESET " %s\n",
                        (unsigned long long)ud_insn_off(&ud_obj),
                        ud_insn_asm(&ud_obj));
                if (ud_insn_mnemonic(&ud_obj) == UD_Iret)
                    break;
            }

            free(block);
        }
        // ============================ END PROGRAM BODY ===============================

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
    printf(COLOR_BLUE "\n=== Program Headers ===\n" COLOR_RESET);
    for (int i = 0; i < elf->e_phnum; i++) {

        // ============================ BEGIN PROGRAM METADATA =============================
        const char *type_str = type_p_to_str(phdr[i].p_type);

        char flags[4] = "";
        if (phdr[i].p_flags & PF_R) strcat(flags, "R");
        if (phdr[i].p_flags & PF_W) strcat(flags, "W");
        if (phdr[i].p_flags & PF_X) strcat(flags, "X");


        printf(COLOR_GREEN 
            "%-3s %-15s %-8s %-10s %-10s %-10s %-10s %-10s %-10s\n" 
            COLOR_RESET,
            "ID", "Type", "Flags", "Offset", "VirtAddr", "PhysAddr", "FileSz", "MemSz", "Align");
        printf(COLOR_GREEN "%-3d %-15s %-8s 0x%08lx 0x%08lx 0x%08lx 0x%08lx 0x%08lx 0x%08lx\n" COLOR_RESET,
                i, type_str, flags,
                phdr[i].p_offset,
                phdr[i].p_vaddr,
                phdr[i].p_paddr,
                phdr[i].p_filesz,
                phdr[i].p_memsz,
                phdr[i].p_align);

        if (phdr[i].p_type == PT_INTERP) {
            char *interp = (char*)(parser->source.mem.data + phdr[i].p_offset);
            printf(COLOR_YELLOW "    Interpreter: " COLOR_RESET "%s\n", interp);
        }
        if (phdr[i].p_type == PT_DYNAMIC) {
            printf(COLOR_YELLOW "    Dynamically linked\n" COLOR_RESET);
        }
        // ============================ END PROGRAM METADATA =============================


        // ============================ BEGIN PROGRAM BODY ===============================
        if (phdr[i].p_filesz > 0) {
            void *block = malloc(phdr[i].p_filesz);
            bparser_read(parser, block, phdr[i].p_offset, phdr[i].p_filesz);
            ud_t ud_obj;
            ud_init(&ud_obj);
            ud_set_input_buffer(&ud_obj, (uint8_t*)block, (size_t)phdr[i].p_filesz);
            ud_set_mode(&ud_obj, 64);
            ud_set_syntax(&ud_obj, UD_SYN_INTEL);
            ud_set_pc(&ud_obj, (uint64_t) phdr[i].p_offset);
            while (ud_disassemble(&ud_obj)) {
                printf(COLOR_YELLOW "0x%llx:" COLOR_RESET " %s\n",
                        (unsigned long long)ud_insn_off(&ud_obj),
                        ud_insn_asm(&ud_obj));
                if (ud_insn_mnemonic(&ud_obj) == UD_Iret)
                    break;
            }
            free(block);
        }

        // ============================ END PROGRAM BODY ===============================
    }
}
// ========================= END PROGRAM HEADER ==================================

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
void dump_disasm_elf32_shdr(Elf32_Ehdr* elf , Elf32_Shdr* shdrs, bparser* parser) {
    Elf32_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(parser->source.mem.data + shstr.sh_offset);
    printf(COLOR_BLUE "\n=== Section Headers ===\n" COLOR_RESET);

    for (int i = 0; i < elf->e_shnum; i++) {
        // ============================ BEGIN SECTION METADATA =============================
        // Section name
        const char* name = &shstrtab[shdrs[i].sh_name];

        // Type
        const char* type_str = sh_type_to_str(shdrs[i].sh_type);

        // Flags
        char flags[8] = "";
        if (shdrs[i].sh_flags & SHF_WRITE)     strcat(flags, "W");
        if (shdrs[i].sh_flags & SHF_ALLOC)     strcat(flags, "A");
        if (shdrs[i].sh_flags & SHF_EXECINSTR) strcat(flags, "X");

        printf(COLOR_GREEN 
                "%-3s %-20s %-10s %-6s %-10s %-10s %-8s %-5s %-5s %-10s %-10s\n" 
                COLOR_RESET,
                "ID", "Name", "Type", "Flags", "Addr", "Offset", "Size", "Link", "Info", "Align", "EntSize");
        char* color = COLOR_CYAN;
        printf("%s%-3d %-20s %-10s %-6s 0x%08lx 0x%08lx 0x%08lx %-5d %-5d 0x%08lx 0x%08lx\n" 
                COLOR_RESET,
                color,
                i, name, type_str, flags,
                shdrs[i].sh_addr,
                shdrs[i].sh_offset,
                shdrs[i].sh_size,
                shdrs[i].sh_link,
                shdrs[i].sh_info,
                shdrs[i].sh_addralign,
                shdrs[i].sh_entsize);
        // ============================ END SECTION METADATA =============================
        // ========================= BEGIN SECTION BODY ==================================
        if (shdrs[i].sh_size > 0) {
            void* block = malloc(shdrs[i].sh_size);
            bparser_read(parser, block, shdrs[i].sh_offset, shdrs[i].sh_size);
            printf(COLOR_YELLOW "Section [%d] %s bytes:\n" COLOR_RESET, i, name);

            if(is_metadata_section(name)) {
                printf(COLOR_YELLOW "[!] This is metadata... not implemented yet \n" COLOR_RESET);
                // unsigned char* ptr = (unsigned char*)block;
                // for (size_t j = 0; j < shdrs[i].sh_size; j++) {
                //     printf("%02x ", ptr[j]);
                    // if ((j + 1) % BLOCK_LENGTH  == 0) printf("\n");
                // }
                printf("\n\n");
            } else {
                ud_t ud_obj;
                ud_init(&ud_obj);
                ud_set_input_buffer(&ud_obj, (uint8_t*)block, (size_t)shdrs[i].sh_size);
                ud_set_mode(&ud_obj, 32);
                ud_set_syntax(&ud_obj, UD_SYN_INTEL);
                ud_set_pc(&ud_obj, (uint64_t) shdrs[i].sh_offset);
                while (ud_disassemble(&ud_obj)) {

                    printf(COLOR_YELLOW "0x%08x:" COLOR_RESET " %s\n",
                            (unsigned long long)ud_insn_off(&ud_obj),
                            ud_insn_asm(&ud_obj));
                    if (ud_insn_mnemonic(&ud_obj) == UD_Iret)
                        break;
                }
            }
            

            free(block);
        }
        // ========================= END SECTION BODY ==================================
    }
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
void dump_disasm_elf64_shdr(Elf64_Ehdr* elf , Elf64_Shdr* shdrs, bparser* parser) {
    Elf64_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(parser->source.mem.data + shstr.sh_offset);
    for (int i = 0; i < elf->e_shnum; i++) {
        // ============================ BEGIN SECTION METADATA =============================
        // Section name
        const char* name = &shstrtab[shdrs[i].sh_name];
        // Type
        const char* type_str = sh_type_to_str(shdrs[i].sh_type);

        // Flags
        char flags[8] = "";
        if (shdrs[i].sh_flags & SHF_WRITE)     strcat(flags, "W");
        if (shdrs[i].sh_flags & SHF_ALLOC)     strcat(flags, "A");
        if (shdrs[i].sh_flags & SHF_EXECINSTR) strcat(flags, "X");

        printf(COLOR_GREEN 
                "%-3s %-20s %-10s %-6s %-10s %-10s %-8s %-5s %-5s %-10s %-10s\n" 
                COLOR_RESET,
                "ID", "Name", "Type", "Flags", "Addr", "Offset", "Size", "Link", "Info", "Align", "EntSize");
        char* color = COLOR_CYAN;
        printf("%s%-3d %-20s %-10s %-6s 0x%08lx 0x%08lx 0x%08lx %-5d %-5d 0x%08lx 0x%08lx\n" 
                COLOR_RESET,
                color,
                i, name, type_str, flags,
                shdrs[i].sh_addr,
                shdrs[i].sh_offset,
                shdrs[i].sh_size,
                shdrs[i].sh_link,
                shdrs[i].sh_info,
                shdrs[i].sh_addralign,
                shdrs[i].sh_entsize);
        // ============================ END SECTION METADATA =============================
        // ========================= BEGIN SECTION BODY ==================================
        if (shdrs[i].sh_size > 0) {
            void* block = malloc(shdrs[i].sh_size);
            bparser_read(parser, block, shdrs[i].sh_offset, shdrs[i].sh_size);
            printf(COLOR_YELLOW "Section [%d] %s bytes:\n" COLOR_RESET, i, name);
            if(is_metadata_section(name)) {

                printf(COLOR_YELLOW "[!] This is metadata... not implemented yet \n" COLOR_RESET);
                
                // unsigned char* ptr = (unsigned char*)block;
                // for (size_t j = 0; j < shdrs[i].sh_size; j++) {
                    // printf("%02x ", ptr[j]);
                    // if ((j + 1) % BLOCK_LENGTH  == 0) printf("\n");
                // }
                
                printf("\n\n");
            } else {
                ud_t ud_obj;
                ud_init(&ud_obj);
                ud_set_input_buffer(&ud_obj, (uint8_t*)block, (size_t)shdrs[i].sh_size);
                ud_set_mode(&ud_obj, 64);
                ud_set_syntax(&ud_obj, UD_SYN_INTEL);
                ud_set_pc(&ud_obj, (uint64_t) shdrs[i].sh_offset);
                while (ud_disassemble(&ud_obj)) {

                    printf("0x%016llx: %s\n", 
                            (unsigned long long)ud_insn_off(&ud_obj), 
                            ud_insn_asm(&ud_obj));

                    if (ud_insn_mnemonic(&ud_obj) == UD_Iret)
                        break;
                }
            }

           free(block);
        }
        // ========================= END SECTION BODY ==================================
    }
}
// ========================= END SECTION HEADER ==================================

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
    unsigned char *data = (unsigned char*) parser->source.mem.data;
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

    printf(COLOR_BLUE "=========================\n" COLOR_RESET);
    return true;
}
