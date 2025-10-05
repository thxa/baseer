#include "bx_elf.h"

bool bx_elf(bparser* parser, void *arg)
{
    int argc = *((inputs*)arg) -> argc;
    char** args = ((inputs*)arg) -> args;
    hashmap_t *maps = ((inputs*)arg) -> map;
    
    // create hashmap of any hashmaps needed by baseer extentions to used it for other extentions...
    // ((inputs*)arg) -> map = create_map();
    // hashmap_t *maps = ((inputs*)arg) -> map;
    // if((hashmap_t*)get(maps, "sections") != NULL){
    //     hashmap_t *map = (hashmap_t*)get(maps, "sections");
    // }

    // 32 bit
    // if((hashmap_t*)get(maps, "symbols") != NULL){
    //     hashmap_t *symbols = (hashmap_t*)get(maps, "symbols");
    //     if((Elf32_Sym*)get(symbols, "main") != NULL){
    //         Elf32_Sym* func = get(symbols, "main");
    //         func.st_value; // offset
    //         func.st_size;  // size
    //     }
    // }

    // 64 bit
    // if((hashmap_t*)get(maps, "symbols") != NULL){
    //     hashmap_t *symbols = (hashmap_t*)get(maps, "symbols");
    //     if((Elf64_Sym*)get(symbols, "main") != NULL){
    //         Elf64_Sym* func = get(symbols, "main");
    //         // func -> st_value; // offset
    //         // func -> st_size;  // size
    //         printf("%offset: lx\n size: %d\n", func->st_value, func->st_size);
    //     }
    // }



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

    // free_maps(maps);

    return true;
}
