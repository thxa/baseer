#include "bx_elf.h"

bool bx_elf(bparser* parser, void *arg)
{
    int argc = *((inputs*)arg) -> argc;
    char** args = ((inputs*)arg) -> args;

    // if (map
    // ((inputs*)arg) -> 
    // create hashmap of any hashmaps needed by baseer extentions to used it for other extentions...
    // hashmap_t *map = create_map();

    // Insert section header pointers into a hashmap for quick retrieval by name
    // insert(map, name, &shdrs[i]);
    // (Elf32_Shdr*)get(map, ".dynstr");


    for(int i = 2; i < argc; i++) {
        if(strcmp("-m", args[i]) == 0) {
            bparser_apply(parser, print_meta_data, arg);
        } else if (strcmp("-a", args[i]) == 0) {
            bparser_apply(parser, print_elf_disasm, arg);
        } else if(strcmp("-d", args[i]) == 0) {
            bparser_apply(parser, b_debugger, arg);
        } else if (strcmp("-c", args[i]) == 0) {
            bparser_apply(parser, decompile_elf, arg);
        }else if(strcmp("--args", args[i]) == 0){
            break;
        }else {
            fprintf(stderr, "[!] Unsupported flag: %s\n", args[i]);
        }
    }

    // if((symtab = get(map, "symbols")) != NULL) {
        // print_symbols_32bit(parser, elf, shdrs, symtab, strtab);
        // printf("\n\n");
    // }
    // free_map(map);

    return true;
}
