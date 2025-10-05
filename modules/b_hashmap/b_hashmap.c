/**
 * @file b_hashmap.c
 * @brief Simple string-keyed hashmap implementation.
 *
 * This file provides a basic hash table implementation that maps
 * string keys to generic pointer values (`void*`). It uses
 * separate chaining (linked lists) for collision handling.
 */
#include "b_hashmap.h"

/**
 * @brief Compute hash value of a string key.
 *
 * Uses the djb2 algorithm by Dan Bernstein to generate
 * an unsigned integer hash for the given key.
 *
 * @param key Null-terminated string key.
 * @return unsigned int Hash value within the range [0, TABLE_SIZE).
 */
unsigned int hash(const char *key)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *key++))
        hash = ((hash << 5) + hash) + c;
    return hash % TABLE_SIZE;
}

/**
 * @brief Allocate and initialize a new hashmap.
 *
 * Creates a hashmap with all buckets initialized to NULL.
 *
 * @return hashmap_t* Pointer to newly allocated hashmap.
 */
hashmap_t *create_map(void)
{
    hashmap_t *map = malloc(sizeof(hashmap_t));
    for (int i = 0; i < TABLE_SIZE; i++)
        map->buckets[i] = NULL;
    return map;
}


/**
 * @brief Insert a key-value pair into the hashmap.
 *
 * The key is duplicated internally, so the caller does not need to
 * maintain the lifetime of the original string.
 *
 * @param map Pointer to hashmap.
 * @param name Null-terminated string key.
 * @param bht_node_p Pointer to the value to store.
 */
void insert(hashmap_t *map, const char *name, void *bht_node_p)
{
    unsigned int index = hash(name);
    bht_node_t *new_bht_node = malloc(sizeof(bht_node_t));
    new_bht_node->name = strdup(name);
    new_bht_node->bht_node_p = bht_node_p;
    new_bht_node->next = map->buckets[index];
    map->buckets[index] = new_bht_node;
}

/**
 * @brief Retrieve a value from the hashmap by key.
 *
 * Performs a lookup in the hashmap and returns the pointer
 * associated with the key, if found.
 *
 * @param map Pointer to hashmap.
 * @param name Null-terminated string key.
 * @return void* Pointer to stored value, or NULL if not found.
 */
void* get(hashmap_t *map, const char *name)
{
    unsigned int index = hash(name);
    bht_node_t *bht_node = map->buckets[index];
    while (bht_node != NULL) {
        if (strcmp(bht_node->name, name) == 0)
            return bht_node->bht_node_p;
        bht_node = bht_node->next;
    }
    return NULL; // Not found
}

/**
 * @brief Free all memory used by the hashmap.
 *
 * Releases all keys, nodes, and the hashmap structure itself.
 * The caller is responsible for freeing values stored inside
 * if they were dynamically allocated.
 *
 * @param map Pointer to hashmap.
 */
void free_map(hashmap_t *map)
{
    for (int i = 0; i < TABLE_SIZE; i++) {
        bht_node_t *bht_node = map->buckets[i];
        while (bht_node != NULL) {
            bht_node_t *temp = bht_node;
            bht_node = bht_node->next;
            free(temp->name);
            free(temp);
        }
    }
    free(map);
}

void free_maps(hashmap_t *map)
{
    for (int i = 0; i < TABLE_SIZE; i++) {
        bht_node_t *bht_node = map->buckets[i];
        while (bht_node != NULL) {
            bht_node_t *temp = bht_node;
            bht_node = bht_node->next;
            free(temp->name);
            free_map((hashmap_t*)temp->bht_node_p);
            free(temp);
        }
    }
    free(map);
}

