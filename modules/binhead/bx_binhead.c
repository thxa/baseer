/**
 * @file bx_binhead.c
 * @brief Handle the magic number of a file header and trigger extensions.
 */

#include "bx_binhead.h"
#include "../bx_elf/bx_elf.h"
#include "../bx_tar/bx_tar.h"
#include "../bx_macho/bx_macho.h"

unsigned int count_bits(unsigned long long int n)
{
    unsigned int cnt = 0;
    while (n) {
        cnt++;
        n >>= 1;
    }
    return cnt;
}

unsigned int count_bytes(unsigned long long n)
{
    unsigned int cnt = count_bits(n);
    if((cnt % BYTE) > 0) {
        cnt = (cnt / BYTE) + 1;
    } else {
        cnt /= BYTE;
    }
    return cnt;
}

unsigned long long int reverse_bytes(unsigned long long int n)
{
    unsigned long long int result = 0;
    unsigned int cnt = count_bytes(n);
    for (int i = 0; i < cnt; i++) {
        result <<= BYTE;         
        result |= (n & 0xFF); 
        n >>= BYTE;             
    }
    return result;
}

unsigned int NXSwapInt(unsigned int inv) 
{
    return ((inv & 0x000000FF) << 24) |
           ((inv & 0x0000FF00) << 8)  |
           ((inv & 0x00FF0000) >> 8)  |
           ((inv & 0xFF000000) >> 24);
}

unsigned long NXSwapLong(unsigned long inv) 
{
    return ((inv & 0x00000000000000FFUL) << 56) |
           ((inv & 0x000000000000FF00UL) << 40) |
           ((inv & 0x0000000000FF0000UL) << 24) |
           ((inv & 0x00000000FF000000UL) << 8)  |
           ((inv & 0x000000FF00000000UL) >> 8)  |
           ((inv & 0x0000FF0000000000UL) >> 24) |
           ((inv & 0x00FF000000000000UL) >> 40) |
           ((inv & 0xFF00000000000000UL) >> 56);
}

bool bx_binhead(baseer_target_t *target, void *arg)
{   

    // printf("This is from binhead: %p\n", ((inputs*)arg));
    if (target == NULL || target->block == NULL)
        return false;

    bparser* bp = NULL;
    // printf("%d\n", target->size);
    bp = bparser_load(target);

    bmagic magics[] = {
        {"ELF", ELF_MAGIC, reverse_bytes(ELF_MAGIC), bx_elf, 0},
        {"TAR", TAR_MAGIC, reverse_bytes(TAR_MAGIC), bx_tar, 257},
        {"Mach-o", MH_MAGIC, NXSwapInt(MH_MAGIC), bx_macho, 0},
        {"Mach-o", MH_MAGIC_64, NXSwapInt(MH_MAGIC_64), bx_macho, 0},
        // {"PDF", PDF_MAGIC, reverse_bytes(PDF_MAGIC), NULL, 0},
        // {"PNG", PNG_MAGIC, reverse_bytes(PNG_MAGIC), NULL, 0},
        // {"ZIP", ZIP_MAGIC, reverse_bytes(ZIP_MAGIC), NULL, 0},
        // { NULL, 0,         0,                 NULL }
    };



    bool flag_1 = 1, flag_2 = 1, f=0;
    for(int i=0; i< sizeof(magics)/sizeof(magics[0]); i++)
    {
        int len = count_bytes(magics[i].number);
        void* pattern = malloc(len);
        size_t n = bparser_read(bp, pattern, magics[i].pos, len);
        unsigned char* p = (unsigned char*)pattern;
        unsigned char* mgn = (unsigned char*)&magics[i].number;
        unsigned char* mgn_r = (unsigned char*)&magics[i].rnumber;
        flag_1 = 1, flag_2 = 1, f=0;
        for(int j=0; j< n; j++) {
            f=1;
            flag_1 &= (*mgn == *p);
            flag_2 &= (*mgn_r == *p); 
            if(!flag_1 && !flag_2) {
                break;
            }
            p++, mgn++, mgn_r++;
        }
        if(f == 0){
            flag_1 = 0, flag_2 = 0;
        }

        if(flag_1 || flag_2){
            // printf("This file is %s\n", magics[i].name);
            bparser_apply(bp, *magics[i].parser, arg);
            free(pattern); // clean up because next free will not be reached
            break;
        }

        free(pattern);
        pattern = NULL;
    }
    free(bp);


    if(!flag_1 && !flag_2) {
        printf("unknown file\n");
    }
    return true;
}
