#ifndef BX_DECOMPILER_ELF
#define BX_DECOMPILER_ELF
#include "../bparser/bparser.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Pretty-print C-like code returned by retdec_decompile_fp().
 * If with_line_numbers is nonzero, adds line numbers before each line.
 */
void print_decompiled_code(const char *c_code, int with_line_numbers);
/**
 * Decompile ELF/binary file using RetDec.
 * Input comes from a bparser (FILE* or memory buffer).
 * Returns a malloc'd buffer containing C-like code on success,
 * or NULL on failure. Caller must free() the returned string.
 */
bool decompile_elf(bparser* parser, void *arg);


#ifdef __cplusplus
}
#endif
#endif // BX_DECOMPILER_ELF
