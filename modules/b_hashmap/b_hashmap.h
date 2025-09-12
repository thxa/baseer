#include "stdlib.h"
#include "string.h"
#define TABLE_SIZE 200

typedef struct func {
    char *name;
    void *func_p;
    struct func *next;
} func_t;

typedef struct {
    func_t *buckets[TABLE_SIZE];
} hashmap_t;


