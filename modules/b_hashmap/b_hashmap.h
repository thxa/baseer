#ifndef B_HASHMAP_H
#define B_HASHMAP_H

#include "stdlib.h"
#include "string.h"
#define TABLE_SIZE 200

typedef struct bht_node {
    char *name;
    void *bht_node_p;
    struct bht_node *next;
} bht_node_t;

typedef struct {
    bht_node_t *buckets[TABLE_SIZE];
} hashmap_t;


unsigned int hash(const char *key);
hashmap_t *create_map(void);
void insert(hashmap_t *map, const char *name, void *bht_node_p);
void* get(hashmap_t *map, const char *name);
void free_map(hashmap_t *map);
void free_maps(hashmap_t *map);

#endif
