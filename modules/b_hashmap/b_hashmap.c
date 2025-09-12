#include "b_hashmap.h"
//
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

