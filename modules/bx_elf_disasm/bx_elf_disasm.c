#include "bx_elf_disasm.h"

void dump_disasm_elf32_phdr(Elf32_Ehdr *elf, Elf32_Phdr* phdr, bparser*parser)
{
    printf(COLOR_BLUE "\n=== Program Headers ===\n" COLOR_RESET);
    printf(COLOR_GREEN 
            "%-3s %-15s %-8s %-10s %-10s %-10s %-10s %-10s %-10s\n" 
            COLOR_RESET,
            "ID", "Type", "Flags", "Offset", "VirtAddr", "PhysAddr", "FileSz", "MemSz", "Align");

    for (int i = 0; i < elf->e_phnum; i++) {
        // --- Type ---
        const char *type_str;
        switch (phdr[i].p_type) {
            case PT_NULL:         type_str = "NULL"; break;
            case PT_LOAD:         type_str = "LOAD"; break;
            case PT_DYNAMIC:      type_str = "DYNAMIC"; break;
            case PT_INTERP:       type_str = "INTERP"; break;
            case PT_NOTE:         type_str = "NOTE"; break;
            case PT_SHLIB:        type_str = "SHLIB"; break;
            case PT_PHDR:         type_str = "PHDR"; break;
            case PT_TLS:          type_str = "TLS"; break;
            case PT_GNU_EH_FRAME: type_str = "GNU_EH_FRAME"; break;
            case PT_GNU_STACK:    type_str = "GNU_STACK"; break;
            case PT_GNU_RELRO:    type_str = "GNU_RELRO"; break;
            case PT_GNU_PROPERTY: type_str = "GNU_PROPERTY"; break;
            case PT_SUNWBSS:      type_str = "SUNWBSS"; break;
            case PT_SUNWSTACK:    type_str = "SUNWSTACK"; break;
            default:
                                  if (phdr[i].p_type >= PT_LOOS && phdr[i].p_type <= PT_HIOS)
                                      type_str = "OS-SPECIFIC";
                                  else if (phdr[i].p_type >= PT_LOPROC && phdr[i].p_type <= PT_HIPROC)
                                      type_str = "PROC-SPECIFIC";
                                  else
                                      type_str = "UNKNOWN";
                                  break;
        }

        char flags[4] = "";
        if (phdr[i].p_flags & PF_R) strcat(flags, "R");
        if (phdr[i].p_flags & PF_W) strcat(flags, "W");
        if (phdr[i].p_flags & PF_X) strcat(flags, "X");

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

        if (phdr[i].p_filesz > 0) {
            printf(COLOR_CYAN "    Segment [%d] %s raw bytes:\n" COLOR_RESET, i, type_str);

            void *block = malloc(phdr[i].p_filesz);
            bparser_read(parser, block, phdr[i].p_offset, phdr[i].p_filesz);

            ud_t ud_obj;
            ud_init(&ud_obj);
            ud_set_input_buffer(&ud_obj, (uint8_t*)block, (size_t)phdr[i].p_filesz);
            ud_set_mode(&ud_obj, 32);
            ud_set_syntax(&ud_obj, UD_SYN_INTEL);
            ud_set_pc(&ud_obj, (uint64_t) phdr[i].p_offset);
            while (ud_disassemble(&ud_obj)) {
                printf("0x%llx: %s\n",
                        (unsigned long long)ud_insn_off(&ud_obj),
                        ud_insn_asm(&ud_obj));
                if (ud_insn_mnemonic(&ud_obj) == UD_Iret)
                    break;
            }


            // unsigned char *ptr = (unsigned char*)block;
            // for (size_t j = 0; j < phdr[i].p_filesz; j++) {
            //     printf("%02x ", ptr[j]);
            //     if ((j + 1) % BLOCK_LENGTH == 0) printf("\n");
            // }
            // printf("\n\n");

            free(block);
        }
    }
}

void dump_disasm_elf32_shdr(Elf32_Ehdr* elf , Elf32_Shdr* shdrs, bparser* parser) {
    Elf32_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(parser->source.mem.data + shstr.sh_offset);
    printf(COLOR_BLUE "\n=== Section Headers ===\n" COLOR_RESET);
    printf(COLOR_GREEN 
            "%-3s %-20s %-10s %-6s %-10s %-10s %-8s %-5s %-5s %-10s %-10s\n" 
            COLOR_RESET,
            "ID", "Name", "Type", "Flags", "Addr", "Offset", "Size", "Link", "Info", "Align", "EntSize");

    for (int i = 0; i < elf->e_shnum; i++) {
        const char* name = &shstrtab[shdrs[i].sh_name];

        // Type

        // Flags
        char flags[8] = "";
        if (shdrs[i].sh_flags & SHF_WRITE)     strcat(flags, "W");
        if (shdrs[i].sh_flags & SHF_ALLOC)     strcat(flags, "A");
        if (shdrs[i].sh_flags & SHF_EXECINSTR) strcat(flags, "X");


        if (shdrs[i].sh_size > 0) {
            void* block = malloc(shdrs[i].sh_size);
            bparser_read(parser, block, shdrs[i].sh_offset, shdrs[i].sh_size);

            ud_t ud_obj;
            ud_init(&ud_obj);
            ud_set_input_buffer(&ud_obj, (uint8_t*)block, (size_t)shdrs[i].sh_size);
            ud_set_mode(&ud_obj, 32);
            ud_set_syntax(&ud_obj, UD_SYN_INTEL);
            ud_set_pc(&ud_obj, (uint64_t) shdrs[i].sh_offset);
            while (ud_disassemble(&ud_obj)) {
                printf("0x%llx: %s\n",
                        (unsigned long long)ud_insn_off(&ud_obj),
                        ud_insn_asm(&ud_obj));
                if (ud_insn_mnemonic(&ud_obj) == UD_Iret)
                    break;
            }




            // printf(COLOR_YELLOW "Section [%d] %s bytes:\n" COLOR_RESET, i, name);
            // unsigned char* ptr = (unsigned char*)block;
            // for (size_t j = 0; j < shdrs[i].sh_size; j++) {
            //     printf("%02x ", ptr[j]);
            //     if ((j + 1) % BLOCK_LENGTH  == 0) printf("\n");
            // }
            // printf("\n\n");

            free(block);
        }
    }

}

// // ELF 64 Bits
// void dump_disasm_elf64hdr(Elf64_Ehdr *elf)
// {
//     printf(COLOR_GREEN "Class: " COLOR_RESET "64-bit\n");
//     printf(COLOR_GREEN "Entry point: " COLOR_RESET "0x%lx\n", elf->e_entry);
//     printf(COLOR_GREEN "Program headers: " COLOR_RESET "%d (offset: 0x%lx)\n", elf->e_phnum, elf->e_phoff);
//     printf(COLOR_GREEN "Section headers: " COLOR_RESET "%d (offset: 0x%lx)\n", elf->e_shnum, elf->e_shoff);
//     printf(COLOR_GREEN "Section header string table index: " COLOR_RESET "%d\n", elf->e_shstrndx);
//     printf(COLOR_GREEN "File Type: " COLOR_RESET "%s (%d)\n",
//             elf_type_to_str(elf->e_type), elf->e_type);
//     printf(COLOR_GREEN "Machine: " COLOR_RESET "%s (%d)\n",
//             elf_machine_to_str(elf->e_machine), elf->e_machine);
// }


void dump_disasm_elf64_phdr(Elf64_Ehdr *elf, Elf64_Phdr* phdr, bparser*parser)
{
    printf(COLOR_BLUE "\n=== Program Headers ===\n" COLOR_RESET);
    // print_program_header_legend();
    printf(COLOR_GREEN 
            "%-3s %-15s %-8s %-10s %-10s %-10s %-10s %-10s %-10s\n" 
            COLOR_RESET,
            "ID", "Type", "Flags", "Offset", "VirtAddr", "PhysAddr", "FileSz", "MemSz", "Align");

    for (int i = 0; i < elf->e_phnum; i++) {
        // --- Type ---
        const char *type_str;
        switch (phdr[i].p_type) {
            case PT_NULL:         type_str = "NULL"; break;
            case PT_LOAD:         type_str = "LOAD"; break;
            case PT_DYNAMIC:      type_str = "DYNAMIC"; break;
            case PT_INTERP:       type_str = "INTERP"; break;
            case PT_NOTE:         type_str = "NOTE"; break;
            case PT_SHLIB:        type_str = "SHLIB"; break;
            case PT_PHDR:         type_str = "PHDR"; break;
            case PT_TLS:          type_str = "TLS"; break;
            case PT_GNU_EH_FRAME: type_str = "GNU_EH_FRAME"; break;
            case PT_GNU_STACK:    type_str = "GNU_STACK"; break;
            case PT_GNU_RELRO:    type_str = "GNU_RELRO"; break;
            case PT_GNU_PROPERTY: type_str = "GNU_PROPERTY"; break;
            case PT_SUNWBSS:      type_str = "SUNWBSS"; break;
            case PT_SUNWSTACK:    type_str = "SUNWSTACK"; break;
            default:
                                  if (phdr[i].p_type >= PT_LOOS && phdr[i].p_type <= PT_HIOS)
                                      type_str = "OS-SPECIFIC";
                                  else if (phdr[i].p_type >= PT_LOPROC && phdr[i].p_type <= PT_HIPROC)
                                      type_str = "PROC-SPECIFIC";
                                  else
                                      type_str = "UNKNOWN";
                                  break;
        }

        char flags[4] = "";
        if (phdr[i].p_flags & PF_R) strcat(flags, "R");
        if (phdr[i].p_flags & PF_W) strcat(flags, "W");
        if (phdr[i].p_flags & PF_X) strcat(flags, "X");

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


        if (phdr[i].p_filesz > 0) {
            printf(COLOR_CYAN "    Segment [%d] %s raw bytes:\n" COLOR_RESET, i, type_str);


            void *block = malloc(phdr[i].p_filesz);
            bparser_read(parser, block, phdr[i].p_offset, phdr[i].p_filesz);
            ud_t ud_obj;
            ud_init(&ud_obj);
            ud_set_input_buffer(&ud_obj, (uint8_t*)block, (size_t)phdr[i].p_filesz);
            ud_set_mode(&ud_obj, 32);
            ud_set_syntax(&ud_obj, UD_SYN_INTEL);
            ud_set_pc(&ud_obj, (uint64_t) phdr[i].p_offset);
            while (ud_disassemble(&ud_obj)) {
                printf("0x%llx: %s\n",
                        (unsigned long long)ud_insn_off(&ud_obj),
                        ud_insn_asm(&ud_obj));
                if (ud_insn_mnemonic(&ud_obj) == UD_Iret)
                    break;
            }

            free(block);
        }
    }
}

void dump_disasm_elf64_shdr(Elf64_Ehdr* elf , Elf64_Shdr* shdrs, bparser* parser) {
    Elf64_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(parser->source.mem.data + shstr.sh_offset);
    printf(COLOR_BLUE "\n=== Section Headers ===\n" COLOR_RESET);
    printf(COLOR_GREEN 
            "%-3s %-20s %-10s %-6s %-10s %-10s %-8s %-5s %-5s %-10s %-10s\n" 
            COLOR_RESET,
            "ID", "Name", "Type", "Flags", "Addr", "Offset", "Size", "Link", "Info", "Align", "EntSize");

    for (int i = 0; i < elf->e_shnum; i++) {
        const char* name = &shstrtab[shdrs[i].sh_name];

        // Type
        // const char* type_str = sh_type_to_str(shdrs[i].sh_type);

        // Flags
        char flags[8] = "";
        if (shdrs[i].sh_flags & SHF_WRITE)     strcat(flags, "W");
        if (shdrs[i].sh_flags & SHF_ALLOC)     strcat(flags, "A");
        if (shdrs[i].sh_flags & SHF_EXECINSTR) strcat(flags, "X");

        // char* color = COLOR_CYAN;
        // printf("%s%-3d %-20s %-10s %-6s 0x%08lx 0x%08lx 0x%08lx %-5d %-5d 0x%08lx 0x%08lx\n" 
        //         COLOR_RESET,
        //         color,
        //         i, name, type_str, flags,
        //         shdrs[i].sh_addr,
        //         shdrs[i].sh_offset,
        //         shdrs[i].sh_size,
        //         shdrs[i].sh_link,
        //         shdrs[i].sh_info,
        //         shdrs[i].sh_addralign,
        //         shdrs[i].sh_entsize);


        if (shdrs[i].sh_size > 0) {
            void* block = malloc(shdrs[i].sh_size);
            bparser_read(parser, block, shdrs[i].sh_offset, shdrs[i].sh_size);

            printf(COLOR_YELLOW "Section [%d] %s bytes:\n" COLOR_RESET, i, name);
            // unsigned char* ptr = (unsigned char*)block;

            ud_t ud_obj;
            ud_init(&ud_obj);
            ud_set_input_buffer(&ud_obj, (uint8_t*)block, (size_t)shdrs[i].sh_size);
            ud_set_mode(&ud_obj, 64);
            ud_set_syntax(&ud_obj, UD_SYN_INTEL);
            ud_set_pc(&ud_obj, (uint64_t) shdrs[i].sh_offset);
            while (ud_disassemble(&ud_obj)) {
                printf("0x%llx: %s\n",
                        (unsigned long long)ud_insn_off(&ud_obj),
                        ud_insn_asm(&ud_obj));
                if (ud_insn_mnemonic(&ud_obj) == UD_Iret)
                    break;
            }

            free(block);
        }
    }

}

bool print_elf_disasm(bparser* parser, void* args) {
    unsigned char *data = (unsigned char*) parser->source.mem.data;
    char bit_type = data[EI_CLASS];
    char endian   = data[EI_DATA];

    printf("HEllo disasm\n");

    printf(COLOR_BLUE "=== ELF File Disasm ===\n" COLOR_RESET);

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
        // dump_elf32hdr(elf);
        dump_disasm_elf32_shdr(elf, shdrs, parser);
        dump_disasm_elf32_phdr(elf, phdr, parser);



    } else if (bit_type == ELFCLASS64) {
        Elf64_Ehdr* elf = (Elf64_Ehdr*) data;
        Elf64_Phdr* phdr = (Elf64_Phdr*) (data + elf->e_phoff);
        Elf64_Shdr* shdrs = (Elf64_Shdr*)(data + elf->e_shoff);
        // dump_elf64hdr(elf);
        dump_disasm_elf64_shdr(elf, shdrs, parser);
        dump_disasm_elf64_phdr(elf, phdr, parser);

    } else {
        printf(COLOR_RED "Unknown ELF class: %d\n" COLOR_RESET, bit_type);
        return false;
    }

    printf(COLOR_BLUE "=========================\n" COLOR_RESET);
    return true;
}
