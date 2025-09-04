/**
 * @file baseer.c
 * @brief Core 
 */
/* Baseer 0.1.0a */

#include "baseer.h"

baseer_target_t *baseer_open(char *file_path)
{
    baseer_target_t *target = NULL;

    struct stat info;
    unsigned int read;
    FILE *handler = NULL;

    RETURN_NULL_IF(file_path == NULL)


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
    return target;
}

void baseer_close(baseer_target_t *target)
{
    if (target)
    {
        target->size = 0;
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
    printf("\n\n");

    for (int i = 0; i < target->size; i++)
        printf("%02x", ptr[i]);
    printf("\n\n");
}

bool baseer_execute(baseer_target_t *target, baseer_callback_t callback, void *arg)
{
    if (target == NULL || callback == NULL)
        return false;

    // TODO: check for failure
    if(!callback(target, arg)) {
        return false;
    }

    return true;
}
