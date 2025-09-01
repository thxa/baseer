
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_EXTENSIONS 10

typedef struct baseer_target_t baseer_target_t;
typedef struct baseer_extension_t baseer_extension_t;
typedef struct baseer_extension_info_t baseer_extension_info_t;

struct baseer_extension_info_t {
    char *name;
    char *author;
    char *description;
};

struct baseer_extension_t {
    baseer_extension_info_t info;
    bool (*execute)(baseer_target_t *);
};

struct baseer_target_t {
    unsigned int version;
    unsigned int size;
    unsigned int loaded_extensions;
    void *block;
    baseer_extension_t **extensions;
};

// Sample extension function: print block as hex
bool print_ex(baseer_target_t* target) {
    if (!target || !target->block) return false;
    unsigned char *ptr = (unsigned char*)target->block;
    printf("Block content (hex): ");
    for (unsigned int i = 0; i < target->size; i++)
        printf("%02x ", ptr[i]);
    printf("\n");
    return true;
}

// Open file and initialize target
baseer_target_t *baseer_open(char *file_path) {
    FILE *f = fopen(file_path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    unsigned int size = ftell(f);
    fseek(f, 0, SEEK_SET);

    baseer_target_t *target = malloc(sizeof(baseer_target_t));
    if (!target) { fclose(f); return NULL; }

    target->block = malloc(size);
    if (!target->block) { free(target); fclose(f); return NULL; }

    fread(target->block, 1, size, f);
    fclose(f);

    target->size = size;
    target->version = 1;
    target->loaded_extensions = 0;

    target->extensions = malloc(sizeof(baseer_extension_t*) * MAX_EXTENSIONS);
    if (!target->extensions) { free(target->block); free(target); return NULL; }

    return target;
}

// Close target and free memory
void baseer_close(baseer_target_t *target) {
    if (!target) return;
    if (target->block) free(target->block);
    if (target->extensions) free(target->extensions);
    free(target);
}

// Load an extension
bool baseer_load_extension(baseer_target_t *target, baseer_extension_t *extension) {
    if (!target || !extension) return false;
    if (target->loaded_extensions >= MAX_EXTENSIONS) return false;
    target->extensions[target->loaded_extensions++] = extension;
    printf("Extension %s loaded\n", extension->info.name);
    return true;
}

// Execute all loaded extensions
void baseer_execute_extensions(baseer_target_t *target) {
    for (unsigned int i = 0; i < target->loaded_extensions; i++) {
        target->extensions[i]->execute(target);
    }
}

int main() {
    baseer_target_t *target = baseer_open("main.c");
    if (!target) { printf("Failed to open file\n"); return 1; }

    baseer_extension_info_t ext_info = {"PrintHex", "Thamer", "Prints block as hex"};
    baseer_extension_t exten = {ext_info, print_ex};

    baseer_load_extension(target, &exten);
    baseer_execute_extensions(target);

    baseer_close(target);
    return 0;
}
