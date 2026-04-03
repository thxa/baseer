/**
 * @file bx_macho_utils.h
 * @brief Utility functions for Mach-O binary analysis.
 *
 * Provides human-readable string conversion, pretty-printing, and
 * helper routines for Mach-O headers, load commands, segments,
 * sections, symbols, and version decoding.
 */
#ifndef BX_MACHO_UTILS
#define BX_MACHO_UTILS

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "macho.h"

/* =================== Type-to-string conversions =================== */

const char* get_mach_o_type(unsigned int type);
const char* get_load_command_name(uint32_t cmd);
const char* platform_to_string(int platform);
const char* tool_to_string(int tool);
const char* cpu_type_to_string(cpu_type_t cputype);
const char* cpu_subtype_to_string(cpu_type_t cputype, cpu_subtype_t cpusubtype);
const char* nlist_type_to_string(uint8_t n_type);
const char* nlist_section_to_string(uint8_t n_sect);

/* =================== Flag / field printers =================== */

void print_mach_o_flags(unsigned int flags);
void print_load_command_info(uint32_t cmd);
void print_segment_flags(uint32_t flags);

/* =================== Metadata printers =================== */

void print_macho_segment64_metadata(segment_command_64 *seg_cmd);
void print_macho_segment32_metadata(segment_command *seg_cmd);
void print_macho_section64_metadata(section_64 *sec_cmd);
void print_macho_section32_metadata(section *sec_cmd);

/* =================== Symbol table printers =================== */

void print_macho_symbols_64(const unsigned char *block, size_t file_size,
                            uint32_t symoff, uint32_t nsyms,
                            uint32_t stroff, uint32_t strsize);
void print_macho_symbols_32(const unsigned char *block, size_t file_size,
                            uint32_t symoff, uint32_t nsyms,
                            uint32_t stroff, uint32_t strsize);

/* =================== Version / UUID decoders =================== */

void print_tool_version(uint32_t version);
void print_uuid(const uint8_t uuid[16]);
void print_source_version(uint64_t packed_version);
void print_version_xyz(uint32_t version);

/* =================== Section attribute helpers =================== */

bool macho_section_is_executable(uint32_t flags);
const char* macho_section_type_to_string(uint32_t flags);

#endif /* BX_MACHO_UTILS */
