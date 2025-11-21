#ifndef BX_MACHO_UTILS
#define BX_MACHO_UTILS
#include <stdint.h>
#include "macho.h"

// Helper functions for convert enums to actual meaning ...
const char* get_mach_o_type(unsigned int type);
void print_mach_o_flags(unsigned int flags);
const char* get_load_command_name(uint32_t cmd);
void print_load_command_info(uint32_t cmd);
void print_segment_flags(uint32_t flags);
const char* platform_to_string(int platform);
const char* tool_to_string(int tool);

// Helper functions for decode and print stuff...
void print_tool_version(uint32_t version);
void print_uuid(const uint8_t uuid[16]);
void print_source_version(uint64_t packed_version);

// print metadata
void print_macho_segment64_metadata(segment_command_64  *seg_cmd);
void print_macho_section64_metadata(section_64* sec_cmd);

#endif 

