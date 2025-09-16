/**
 * @file baseer.c
<<<<<<< HEAD
 * @brief Core 
 */
=======
 * @brief Core file operations for Baseer.
 * 
 * Supports memory and streaming modes, execution of analysis tools,
 */


>>>>>>> 9c46ae5 (new changes)
/* Baseer 0.1.0a */

#include "baseer.h"

<<<<<<< HEAD
baseer_target_t *baseer_open(char *file_path)
{
    baseer_target_t *target = NULL;

    struct stat info;
    unsigned int read;
    FILE *handler = NULL;

    RETURN_NULL_IF(file_path == NULL)

=======

/* =================== Memory Mode =================== */
baseer_target_t *baseer_open_memory(char *file_path)
{
    baseer_target_t *target = NULL;

    size_t read;
    FILE *handler = NULL;

    RETURN_NULL_IF(file_path == NULL)
    struct stat info;
>>>>>>> 9c46ae5 (new changes)

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

<<<<<<< HEAD
    read = fread(target->block,
                 target->size, 1, handler);
    // TODO: read condition
=======
    read = fread(target->block, 1, target->size, handler);
    // TODO: read condition ->
    if (read != target->size)
    {
        free(target->block);
        free(target);
        fclose(handler);
        return NULL;
    }
    
>>>>>>> 9c46ae5 (new changes)
    fclose(handler);
    return target;
}

<<<<<<< HEAD
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

=======

/* =================== Streaming Mode =================== */
baseer_target_t *baseer_open_file(char *file_path)
{
    RETURN_NULL_IF(file_path == NULL)
    baseer_target_t *target = NULL;
    FILE *handler = NULL;
    handler = fopen(file_path, "rb");
    RETURN_NULL_IF(handler == NULL)
    target = (baseer_target_t *)malloc(sizeof(baseer_target_t));
    if (target == NULL){
        fclose(handler);
        return NULL;
    }
    target->size = 0;          // Size unkown or not needed in streaming mode
    target->block = handler;   // Store the FILE pointer in block
    return target;
}

/* =================== Closeing =================== */
void baseer_close(baseer_target_t *target)
{
    if(!target) return;
    target->size = 0;
    if(target->block) {
        // If block is a FILE pointer, close it; otherwise, free the memory
        FILE *handler = (FILE*)target->block;
        if(handler) {
            fclose(handler);
            handler = NULL;
        }else {
            free(target->block);
            target->block = NULL;
        }
    }
    free(target);
}

/* =================== Printing =================== */
>>>>>>> 9c46ae5 (new changes)
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

<<<<<<< HEAD
bool baseer_execute(baseer_target_t *target, baseer_callback_t callback, void *arg)
{
    if (target == NULL || callback == NULL)
        return false;
=======
/* =================== Executing =================== */
bool baseer_execute(baseer_target_t *target, baseer_callback_t callback, void *arg)
{
    if (target == NULL || callback == NULL) return false;
>>>>>>> 9c46ae5 (new changes)

    // TODO: check for failure
    if(!callback(target, arg)) {
        return false;
    }

    return true;
}
