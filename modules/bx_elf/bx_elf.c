#include "bx_elf.h"

bool bx_elf(bparser* parser, void *arg)
{
    int argc = *((inputs*)arg) -> argc;
    char** args = ((inputs*)arg) -> args;

    for(int i = 2; i < argc; i++) {
        if(strcmp("-m", args[i]) == 0) {
            bparser_apply(parser, print_meta_data, arg);
        } else if (strcmp("-a", args[i]) == 0) {
            bparser_apply(parser, print_elf_disasm, arg);
        } else if(strcmp("-d", args[i]) == 0) {
            bparser_apply(parser, b_debugger, arg);
        } else if (strcmp("-c", args[i]) == 0) {
            bparser_apply(parser, decompile_elf, arg);
        } else if(strcmp("--args", args[i]) == 0) {
            break;
        } else {
            fprintf(stderr, "[!] Unsupported flag: %s\n", args[i]);
        }
    }

    return true;
}
