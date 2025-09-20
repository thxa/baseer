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


