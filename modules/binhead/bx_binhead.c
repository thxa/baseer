/**
 * @file bx_binhead.c
 * @brief Handle the magic number of a file header and trigger extensions.
 */

#include "bx_binhead.h"
#include "../bx_elf/bx_elf.h"

unsigned int count_bits(unsigned long long int n) {
    unsigned int cnt = 0;
    while (n) {
        cnt++;
        n >>= 1;
    }
    return cnt;
}

unsigned int count_bytes(unsigned long long n) {
    unsigned int cnt = count_bits(n);
    if((cnt % BYTE) > 0) {
        cnt = (cnt / BYTE) + 1;
    } else {
        cnt /= BYTE;
    }
    return cnt;
}

unsigned long long int reverse_bytes(unsigned long long int n) {
    unsigned long long int result = 0;
    unsigned int cnt = count_bytes(n);
    for (int i = 0; i < cnt; i++) {
        result <<= BYTE;         
        result |= (n & 0xFF); 
        n >>= BYTE;             
    }
    return result;
}

bool bx_binhead(baseer_target_t *target, void *arg)
{   

    // printf("This is from binhead: %p\n", ((inputs*)arg));
    if (target == NULL || target->block == NULL)
        return false;

    void *block = target ->block;
    bparser* bp= bparser_load(BPARSER_MEM, block);
    bp ->source.mem.size = target ->size;

    bmagic magics[] = {
        {"ELF", ELF_MAGIC, reverse_bytes(ELF_MAGIC), bx_elf},
        {"PDF", PDF_MAGIC, reverse_bytes(PDF_MAGIC)},
        {"PNG", PNG_MAGIC, reverse_bytes(PNG_MAGIC)},
        {"ZIP", ZIP_MAGIC, reverse_bytes(ZIP_MAGIC)},
        {"Mach-o", MACHO_MAGIC, reverse_bytes(MACHO_MAGIC)},
        // { NULL, 0,         0,                 NULL }
    };

    bool flag = 1;
    for(int i=0; i< sizeof(magics)/sizeof(magics[0]); i++)
    {
        int len = count_bytes(magics[i].number);
        void* pattern = malloc(len);
        size_t n = bparser_read(bp, pattern, len);
        unsigned char* p = (unsigned char*)pattern;
        unsigned char* mgn = (unsigned char*)&magics[i].number;
        unsigned char* mgn_r = (unsigned char*)&magics[i].rnumber;

        flag = 1;
        for(int j=0; j< n; j++) {
            flag &= (*mgn == *p) || (*mgn_r == *p); 
            if(!flag) {
                break;
            }
            p++, mgn++, mgn_r++;
        }

        if(flag) {
            printf("This file is %s\n", magics[i].name);
            bparser_apply(bp, *magics[i].parser, arg);
            break;
        }

        free(pattern);
        pattern = NULL;
    }
    free(bp);

    if(!flag) {
        printf("unknown file\n");
    }
    return true;
}
