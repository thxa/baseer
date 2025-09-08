#include "bx_elf.h"
#include <dlfcn.h>

#define TABLE_SIZE 200

typedef struct func {
    char *name;
    void *func_p;
    struct func *next;
} func_t;

typedef struct {
    func_t *buckets[TABLE_SIZE];
} hashmap_t;

// Simple hash function (djb2 by Dan Bernstein)
unsigned int hash(const char *key) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++))
        hash = ((hash << 5) + hash) + c;
    return hash % TABLE_SIZE;
}

// Create a new hashmap
hashmap_t *create_map() {
    hashmap_t *map = malloc(sizeof(hashmap_t));
    for (int i = 0; i < TABLE_SIZE; i++)
        map->buckets[i] = NULL;
    return map;
}

// Insert name-func_p
void insert(hashmap_t *map, const char *name, void *func_p) {
    unsigned int index = hash(name);
    func_t *new_func = malloc(sizeof(func_t));
    new_func->name = strdup(name);
    new_func->func_p = func_p;
    new_func->next = map->buckets[index];
    map->buckets[index] = new_func;
}

// Get func_p by key
void* get(hashmap_t *map, const char *name) {
    unsigned int index = hash(name);
    func_t *func = map->buckets[index];
    while (func != NULL) {
        if (strcmp(func->name, name) == 0)
            return func->func_p;
        func = func->next;
    }
    return NULL; // Not found
}

// Free memory
void free_map(hashmap_t *map) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        func_t *func = map->buckets[i];
        while (func != NULL) {
            func_t *temp = func;
            func = func->next;
            free(temp->name);
            free(temp);
        }
    }
    free(map);
}




bool bx_elf(bparser* parser, void *arg)
{
    int argc = *((inputs*)arg) -> argc;
    char** args = ((inputs*)arg) -> args;
    // printf("This is from elf: %d\n", argc);
    // printf("This is from elf: %s\n", args_4);



    hashmap_t *map = create_map();
    void *handle_1, *handle_2;
    handle_1 = dlopen("./modules/b_elf_metadata.so", RTLD_LAZY);
    handle_2 = dlopen("./modules/b_debugger.so", RTLD_LAZY);

    if (!handle_1 || !handle_2) {
        fprintf(stderr, "%s\n", dlerror());
        return 1;
    }
    dlerror();

    insert(map, "b_elf_metadata", (bparser_callback_t*)dlsym(handle_1, "b_elf_metadata"));
    insert(map, "b_debugger", (bparser_callback_t*)dlsym(handle_2, "b_debugger"));

    bparser_callback_t b_elf_metadata = (bparser_callback_t)get(map, "b_elf_metadata");
    bparser_callback_t b_debugger = (bparser_callback_t)get(map, "b_debugger");

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

    dlclose(handle_1);
    free_map(map);

    return true;
}
