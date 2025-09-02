/* Baseer 0.1.0a */
#include "baseer.h"

baseer_partition_t *baseer_partitionize(baseer_target_t *target)
{
    assert(target != NULL);
    assert(target->partition_size > 0);
    assert(target->partition_count > 0);

    baseer_partition_t *partitions = (baseer_partition_t *)
        malloc(sizeof(baseer_partition_t) * target->partition_count);
    RETURN_NULL_IF(partitions == NULL);

    for (int i = 0; i < target->partition_count; i++)
        partitions[i].index = i;

    return partitions;
}

void baseer_departitionize(baseer_partition_t *partitions)
{
    if (partitions)
        free(partitions);
}

baseer_target_t *baseer_open(char *file_path, unsigned int partition_count)
{
    baseer_target_t *target = NULL;

    struct stat info;
    unsigned int tmp_partition_count = partition_count;
    unsigned int read;
    FILE *handler = NULL;

    RETURN_NULL_IF(file_path == NULL)

    if (partition_count == 0)
        tmp_partition_count = BASEER_DEFAULT_PARTITION_COUNT;

    RETURN_NULL_IF(stat(file_path, &info) != 0)
    RETURN_NULL_IF((info.st_size == 0) || (info.st_size > BASEER_MAX_FILE_SIZE))

    handler = fopen(file_path, "rb");
    RETURN_NULL_IF(handler == NULL)

    target = (baseer_target_t *)malloc(sizeof(baseer_target_t));
    if (target == NULL)
    {
        fclose(handler);
        return NULL;
    }

    target->size = info.st_size;
    target->partition_count = tmp_partition_count;
    target->extra = target->size % target->partition_count;

    if (target->extra > 0)
        target->partition_size = (target->size - target->extra) / target->partition_count;
    else
        target->partition_size = (target->size / target->partition_count);

    target->block = malloc(target->size);
    if (target->block == NULL)
    {
        free(target);
        fclose(handler);
        return NULL;
    }

    read = fread(target->block,
                 target->size, 1, handler);
    // TODO: read condition
    fclose(handler);

    target->version = BASEER_VERSION;
    target->partitions = baseer_partitionize(target);

    return target;
}

void baseer_close(baseer_target_t *target)
{
    if (target)
    {
        target->size = 0;
        target->extra = 0;
        target->partition_count = 0;
        target->partition_size = 0;
        target->version = 0;

        baseer_departitionize(target->partitions);

        if (target->block)
            free(target->block);

        free(target);
    }
}

void baseer_print(baseer_target_t *target)
{
    if (target == NULL)
        return;

    char *ptr = (char *)target->block;
    printf("block-size: %u\n", target->size);
    printf("block-address: %p\n", target->block);
    printf("partition-count: %u\n", target->partition_count);
    printf("partition-size: %u\n", target->partition_size);
    printf("partition-extra: %u\n", target->extra);
    printf("\n\n");

    for (int i = 0; i < target->size; i++)
        printf("%02x", ptr[i]);
    printf("\n\n");

    printf("partitions:\n");
    for (int i = 0; i < target->partition_count; i++)
    {
        printf("[%d] -> %p\n", target->partitions[i].index,
               target->block + (target->partitions[i].index * target->partition_size));
    }
}

bool baseer_execute(baseer_target_t *target, baseer_partition_callback_t callback, void *arg)
{
    if (target == NULL || callback == NULL)
        return false;

    bool result = true;
    for (int i = 0; i < target->partition_count; i++)
    {
        result = result && callback(target, i, arg);
        // TODO: check for failure
    }

    return result;
}
