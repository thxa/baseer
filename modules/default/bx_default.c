/**
 * @file bx_default.c
 * @brief default function
 */
#include "bx_default.h"
bool bx_default(baseer_target_t *target, unsigned int index, void *arg)
{
    if (target == NULL || target->block == NULL || index >= target->partition_count)
        return false;

    unsigned char *partition = BASEER_BLOCK_OFFSET(target, index);
    for (int i = 0; i < target->partition_size; i++)
        printf("%#02x ", partition[i]);
    printf("\n\n");
    return true;
}
