#ifndef BX_ELF
#define BX_ELF
#include "../bparser/bparser.h"



typedef bool (*bparser_callback_t)(bparser* parser, void* arg);

bool bx_elf(bparser* parser, void *arg);

#endif 
