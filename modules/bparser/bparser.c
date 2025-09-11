#include "bparser.h"

bparser* bparser_load(bparser_type type, void *data) {
    bparser *p = malloc(sizeof(bparser));
    if (!p) return NULL;

    p->type = type;

    if (type == BPARSER_FILE) {
        p->source.fp = (FILE*)data;
    } else if (type == BPARSER_MEM) {

        p->source.mem.size = 0;
        p->source.mem.data = data;
    }

    return p;
}

size_t bparser_read(bparser* parser, void* buf, unsigned int pos, size_t size) {
    if (!parser || !buf) return 0;
    if (parser->type == BPARSER_FILE) {
        return fread(buf, 1, size, parser->source.fp);
    } else if (parser->type == BPARSER_MEM) {
        unsigned int chunck_size=0;
        // unsigned char* i = ;

        if(size >= parser->source.mem.size || pos + size >= parser->source.mem.size) {
            return 0;
        }

        if(size >= parser->source.mem.size) {
            chunck_size = parser->source.mem.size-1;
        } else {
            chunck_size = size;
        }
        memcpy(buf, (unsigned char*)parser->source.mem.data+pos, chunck_size);
        return size;
    }
    return 0;
}

bool bparser_apply(bparser* parser, bparser_callback_t callback, void* arg) {
    if(parser == NULL || callback == NULL) {
        return 0;
    }
    callback(parser, arg);
    return 1;
}

