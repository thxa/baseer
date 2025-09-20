/**
 * @file baseer.c
 * @brief Core file operations for Baseer.
 * 
 * Supports memory and streaming modes, execution of analysis tools,
 */


/* Baseer 0.1.0a */

#include "baseer.h"


/* =================== Baseer open =================== */
baseer_target_t *baseer_open(char *file_path, baseer_mode_t mode)
{
    baseer_target_t *target = NULL;
    size_t read;

    FILE *handler = NULL;
    RETURN_NULL_IF(file_path == NULL)

    struct stat info;
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

    target->mode = mode;

    target->fp = (mode == BASEER_MODE_STREAM || mode == BASEER_MODE_BOTH) ? handler : NULL;

    target->size = info.st_size;

    if (mode == BASEER_MODE_MEMORY || mode == BASEER_MODE_BOTH){
        target->block = malloc(target->size);
        if (target->block == NULL){
            free(target);
            fclose(handler);
            return NULL;
        }
        if (fread(target->block, 1, target->size, handler) != target->size)
        {
            free(target->block);
            free(target);
            fclose(handler);
            return NULL;
        }
        // if only memory mode, close file after loading for saving resources
        if (mode == BASEER_MODE_MEMORY){
            fclose(handler);
            target->fp = NULL;
        }
    }
    return target;
}

/* =================== Closeing =================== */
void baseer_close(baseer_target_t *target)
{
    if(!target) return;

    switch (target->mode){
        case BASEER_MODE_MEMORY:
            if (target->block){
                free(target->block);
                target->block = NULL;
            }
            break;
        case BASEER_MODE_STREAM:
            if (target->fp){
                fclose(target->fp);
                target->fp = NULL;
            }
            break;
        case BASEER_MODE_BOTH:
            if (target->block){
                free(target->block);
                target->block = NULL;
            }
            if (target->fp){
                fclose(target->fp);
                target->fp = NULL;
            }
            break;
        default:
            fprintf(stderr, "[!] Failed in closeing \n");
            break;
    }
    free(target);
}

/* =================== Printing =================== */
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

/* =================== Executing =================== */
bool baseer_execute(baseer_target_t *target, baseer_callback_t callback, void *arg)
{
    if (target == NULL || callback == NULL) return false;

    // TODO: check for failure
    if(!callback(target, arg)) {
        return false;
    }

    return true;
}
