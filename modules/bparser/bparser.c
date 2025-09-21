/**
 * @file bparser.c
 * @brief Binary parser implementation
 *
 * Supports reading from memory or streaming files, and executing callbacks.
 */

#include "bparser.h"

/* ========================= Load Parser ========================= */
bparser* bparser_load(baseer_target_t *target) {
    RETURN_NULL_IF(!target);

    bparser *p = malloc(sizeof(bparser));
    RETURN_NULL_IF(!p);

    p->mode = target->mode;
    p->fp = target->fp; // if mem mode, this will be NULL
    p->block = target->block;
    p->size = target->size;
 
    return p;
}

/* ========================= Read Data ========================= */
size_t bparser_read(bparser* parser, void* buf, unsigned int pos, size_t size) {
    if (!parser || !buf) return 0;

    switch (parser->mode) {
        case BASEER_MODE_MEMORY:
            unsigned int chunck_size=0;
            if(pos + size > parser->size) {
                return 0;
            } else {
                chunck_size = size;
            }
            memcpy(buf, (unsigned char*)parser->block+pos, chunck_size);
            return size;

        case BASEER_MODE_STREAM:
            if (!parser->fp) return 0;
            if (fseek(parser->fp, pos, SEEK_SET) != 0) return 0;
            return fread(buf, 1, size, parser->fp);

        case BASEER_MODE_BOTH:
            if (parser->block && pos + size < parser->size) {
                memcpy(buf, (unsigned char*)parser->block + pos, size);
                return size;
            } else if (parser->fp) {
                if (fseek(parser->fp, pos, SEEK_SET) != 0) return 0;
                return fread(buf, 1, size, parser->fp);
            }
            return 0;

        default:
            return 0;
    }

    return 0;
}

/* ========================= Apply Callback ========================= */
bool bparser_apply(bparser* parser, bparser_callback_t callback, void* arg) {
    if(parser == NULL || callback == NULL) {
        return 0;
    }
    callback(parser, arg);
    return 1;
}

