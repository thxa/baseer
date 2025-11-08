#include "./bx_macho.h"
#include "../../baseer.h"
#include <string.h>
#include "../b_macho_metadata/b_macho_metadata.h"

bool bx_macho(bparser* parser, void *arg)
{
    int argc = *((inputs*)arg) -> argc;
    char** args = ((inputs*)arg) -> args;
    hashmap_t *maps = ((inputs*)arg) -> map;

    for(int i = 2; i < argc; i++) {
        if(strcmp("-m", args[i]) == 0) {
            bparser_apply(parser, b_macho_metadata, arg);
        // } else if (strcmp("-a", args[i]) == 0) {
            // bparser_apply(parser, print_elf_disasm, arg);
        // } else if(strcmp("-d", args[i]) == 0) {
            // bparser_apply(parser, b_debugger, arg);
        // } else if (strcmp("-c", args[i]) == 0) {
            // bparser_apply(parser, decompile_elf, arg);
        }else if(strcmp("--args", args[i]) == 0){
            break;
        }else {
            fprintf(stderr, "[!] Unsupported flag: %s\n", args[i]);
        }
    }

    // free_maps(maps);

    return true;
}
