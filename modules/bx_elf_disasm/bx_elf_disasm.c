#include "bx_elf_disasm.h"

const char* elf_disasm_type_to_str(int type)
{
    char *result;
    switch (type) {
        case ET_NONE:   result = "No file type"; break;
        case ET_REL:    result = "Relocatable file"; break;
        case ET_EXEC:   result = "Executable file"; break;
        case ET_DYN:    result = "Shared object file"; break;
        case ET_CORE:   result = "Core file"; break;
        case ET_NUM:    result = "Number of defined types"; break;
        default:
            if (type >= ET_LOOS && type <= ET_HIOS)
                result = "OS-specific file type";
            if (type >= ET_LOPROC && type <= ET_HIPROC)
                result = "Processor-specific file type";
            result = "Unknown file type";
    }
    return result;
}

const char* elf_disasm_machine_to_str(int machine)
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




void dump_disasm_elf32_phdr(Elf32_Ehdr *elf, Elf32_Phdr* phdr, bparser*parser)
{
    printf(COLOR_BLUE "\n=== Program Headers ===\n" COLOR_RESET);

    for (int i = 0; i < elf->e_phnum; i++) {

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

void dump_disasm_elf32_shdr(Elf32_Ehdr* elf , Elf32_Shdr* shdrs, bparser* parser) {
    Elf32_Shdr shstr = shdrs[elf->e_shstrndx];
    const char* shstrtab = (const char*)(parser->source.mem.data + shstr.sh_offset);
    printf(COLOR_BLUE "\n=== Section Headers ===\n" COLOR_RESET);

    for (int i = 0; i < elf->e_shnum; i++) {
        const char* name = &shstrtab[shdrs[i].sh_name];
        if (shdrs[i].sh_size > 0) {
            void* block = malloc(shdrs[i].sh_size);
            bparser_read(parser, block, shdrs[i].sh_offset, shdrs[i].sh_size);
            printf(COLOR_YELLOW "Section [%d] %s bytes:\n" COLOR_RESET, i, name);
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

            free(block);
        }
    }

}

// // ELF 64 Bits

void dump_disasm_elf64_phdr(Elf64_Ehdr *elf, Elf64_Phdr* phdr, bparser*parser)
{
    printf(COLOR_BLUE "\n=== Program Headers ===\n" COLOR_RESET);
    for (int i = 0; i < elf->e_phnum; i++) {


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
    for (int i = 0; i < elf->e_shnum; i++) {
        const char* name = &shstrtab[shdrs[i].sh_name];

        if (shdrs[i].sh_size > 0) {
            void* block = malloc(shdrs[i].sh_size);
            bparser_read(parser, block, shdrs[i].sh_offset, shdrs[i].sh_size);
            printf(COLOR_YELLOW "Section [%d] %s bytes:\n" COLOR_RESET, i, name);
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
                elf_disasm_type_to_str(elf->e_type), elf->e_type);



        printf(COLOR_GREEN "Machine: " COLOR_RESET "%s (%d)\n",
                elf_disasm_machine_to_str(elf->e_machine), elf->e_machine);


        if(elf->e_machine != EM_X86_64 && elf->e_machine !=  EM_386) {
            printf(COLOR_RED "Not Supported machine: %s\n" COLOR_RESET, elf_disasm_machine_to_str(elf->e_machine));
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
                elf_disasm_type_to_str(elf->e_type), elf->e_type);
        printf(COLOR_GREEN "Machine: " COLOR_RESET "%s (%d)\n",
                elf_disasm_machine_to_str(elf->e_machine), elf->e_machine);

        printf(COLOR_GREEN "Entry point: " COLOR_RESET "0x%x\n", elf->e_entry);

        if(elf->e_machine != EM_X86_64 && elf->e_machine !=  EM_386) {
            printf(COLOR_RED "Not Supported machine: %s\n" COLOR_RESET, elf_disasm_machine_to_str(elf->e_machine));
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
