/**
 * @file bparser.c
 * @brief Binary parser implementation
 *
 * Supports reading from memory or streaming files, and executing callbacks.
 */

#include "bparser.h"

/* ========================= Load Parser ========================= */
// bparser* bparser_load(bparser_type type, void *data) {
bparser* bparser_load(baseer_target_t *target) {
    RETURN_NULL_IF(!target);

    bparser *p = malloc(sizeof(bparser));
    RETURN_NULL_IF(!p);
    p->fp = target ->fp;
    p->block = target->block;
    p->size = target->size;
    // p->type = type;
    // if (type == BPARSER_FILE) {
        // p->source.fp = (FILE*)data;
    // } else if (type == BPARSER_MEM) {
    //     p->block = data;
    //     p->source.mem.size = 0;
    // }
    return p;
}

/* ========================= Read Data ========================= */
size_t bparser_read(bparser* parser, void* buf, unsigned int pos, size_t size) {
    if (!parser || !buf) return 0;

    // if (parser->type == BPARSER_FILE) {
    //     if(!parser->source.fp) return 0;
    //     if(fseek(parser->source.fp, pos, SEEK_SET) != 0) return 0;
    //     return fread(buf, 1, size, parser->source.fp);
    // } else if (parser->type == BPARSER_MEM) {
        unsigned int chunck_size=0;
        // unsigned char* i = ;
        // if(pos + size >= parser->block.size) {
        if(pos + size >= parser->size) {
            return 0;
        } else {
            chunck_size = size;
        }

        // memcpy(buf, (unsigned char*)parser->block+pos, chunck_size);
        memcpy(buf, (unsigned char*)parser->block+pos, chunck_size);
        return size;
    // }
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

