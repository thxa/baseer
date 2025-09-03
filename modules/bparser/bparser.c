#include "bparser.h"

bparser* bparser_load(bparser_type type, void *data) {
    bparser *p = malloc(sizeof(bparser));
    if (!p) return NULL;

    p->type = type;

    if (type == BPARSER_FILE) {
        p->source.fp = (FILE*)data;
    } else if (type == BPARSER_MEM) {

        p->source.mem.size = 0;
        p->source.mem.pos = 0;
        p->source.mem.data = data;
    }

    return p;
}

size_t bparser_read(bparser* parser, void* buf, size_t size) {
    if (!parser || !buf) return 0;
    if (parser->type == BPARSER_FILE) {
        return fread(buf, 1, size, parser->source.fp);
    } else if (parser->type == BPARSER_MEM) {
        memcpy(buf, (unsigned char*)parser->source.mem.data + parser->source.mem.pos, size);
        return size;
    }
    return 0;
}

int bparser_apply(bparser* parser, bparser_callback_t callback, void* arg) {
    if(parser == NULL || callback == NULL) {
        return 1;
    }
    callback(parser, arg);
    return 0;
}

