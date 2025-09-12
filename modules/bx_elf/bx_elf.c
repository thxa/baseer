#include "bx_elf.h"

bool bx_elf(bparser* parser, void *arg)
{
    int argc = *((inputs*)arg) -> argc;
    char** args = ((inputs*)arg) -> args;
    // printf("This is from elf: %d\n", argc);
    // printf("This is from elf: %s\n", args_4);

    if(strcmp("-m", args[2]) == 0) {
        bparser_apply(parser, print_meta_data, arg);
        // print_meta_data(parser);
    } else if(strcmp("-d", args[2]) == 0) {
        bparser_apply(parser, b_debugger, arg);
    } else if (strcmp("-c", args[2]) == 0) {
        bparser_apply(parser, decompile_elf, arg);
    }else {
        printf("Not %s implement yet.", args[2]);
    }

    return true;
}
