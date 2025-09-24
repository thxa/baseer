/**
 * @file b_elf_metadata.h
 * @brief ELF metadata extraction and printing interface for Baseer.
 *
 * This module provides functions to parse and print metadata from ELF files,
 * including ELF headers, program headers, and section headers. It is used
 * within the Baseer binary analysis framework to display detailed ELF
 * information in a structured and colored format.
 *
 * The main entry point is `print_meta_data()`, which determines the ELF class
 * (32-bit or 64-bit), detects endianness, and invokes the appropriate dump
 * functions for headers and sections.
 *
 * @note This module depends on `bx_elf_utils` for header dumping utilities.
 * @note Supports both 32bit and 64bit binaries.
 *
 * @see bx_elf_utils.h
 * @see bparser.h
 */
#ifndef B_ELF_METADATA
#define B_ELF_METADATA
#include "../bparser/bparser.h"
#include "../../baseer.h"
#include <elf.h>
#include<string.h>
#include "../bx_elf_utils/bx_elf_utils.h"
#include "udis86.h"

void display_byte(const unsigned char *byte);
void dump_elf32hdr(Elf32_Ehdr *elf);
void dump_elf64hdr(Elf64_Ehdr *elf);
void dump_elf32_shdr(Elf32_Ehdr *elf , Elf32_Shdr *shdrs, bparser *parser);
void dump_elf64_shdr(Elf64_Ehdr *elf , Elf64_Shdr *shdrs, bparser *parser);
void dump_elf32_phdr(Elf32_Ehdr *elf, Elf32_Phdr *phdr, bparser *parser);
void dump_elf64_phdr(Elf64_Ehdr *elf, Elf64_Phdr *phdr, bparser *parser);


bool print_meta_data(bparser *parser, void *args);

#endif
