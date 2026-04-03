/**
 * @file test_hashmap.c
 * @brief Unit tests for the b_hashmap module.
 */
#include "test_framework.h"
#include "../modules/b_hashmap/b_hashmap.h"

TEST(test_create_map) {
    hashmap_t *map = create_map();
    ASSERT_NOT_NULL(map);
    for (int i = 0; i < TABLE_SIZE; i++) {
        ASSERT_NULL(map->buckets[i]);
    }
    free_map(map);
}

TEST(test_insert_and_get) {
    hashmap_t *map = create_map();
    int val1 = 42;
    int val2 = 99;
    insert(map, "key1", &val1);
    insert(map, "key2", &val2);

    void *result1 = get(map, "key1");
    void *result2 = get(map, "key2");
    ASSERT_NOT_NULL(result1);
    ASSERT_NOT_NULL(result2);
    ASSERT_EQ(*(int*)result1, 42);
    ASSERT_EQ(*(int*)result2, 99);
    free_map(map);
}

TEST(test_get_nonexistent) {
    hashmap_t *map = create_map();
    void *result = get(map, "nonexistent");
    ASSERT_NULL(result);
    free_map(map);
}

TEST(test_hash_collision_handling) {
    hashmap_t *map = create_map();
    /* Insert many keys to exercise collision chains */
    int values[50];
    char keys[50][16];
    for (int i = 0; i < 50; i++) {
        values[i] = i * 10;
        snprintf(keys[i], sizeof(keys[i]), "key_%d", i);
        insert(map, keys[i], &values[i]);
    }
    /* Verify all can be retrieved */
    for (int i = 0; i < 50; i++) {
        void *result = get(map, keys[i]);
        ASSERT_NOT_NULL(result);
        ASSERT_EQ(*(int*)result, i * 10);
    }
    free_map(map);
}

TEST(test_duplicate_key_insert) {
    hashmap_t *map = create_map();
    int val1 = 10;
    int val2 = 20;
    insert(map, "dup", &val1);
    insert(map, "dup", &val2);
    /* Should return the most recently inserted (head of chain) */
    void *result = get(map, "dup");
    ASSERT_NOT_NULL(result);
    ASSERT_EQ(*(int*)result, 20);
    free_map(map);
}

TEST(test_empty_key) {
    hashmap_t *map = create_map();
    int val = 7;
    insert(map, "", &val);
    void *result = get(map, "");
    ASSERT_NOT_NULL(result);
    ASSERT_EQ(*(int*)result, 7);
    free_map(map);
}

int main(void) {
    printf("\n=== Hashmap Tests ===\n");
    RUN_TEST(test_create_map);
    RUN_TEST(test_insert_and_get);
    RUN_TEST(test_get_nonexistent);
    RUN_TEST(test_hash_collision_handling);
    RUN_TEST(test_duplicate_key_insert);
    RUN_TEST(test_empty_key);
    TEST_REPORT();
}
