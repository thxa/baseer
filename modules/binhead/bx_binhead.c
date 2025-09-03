/**
 * @file bx_binhead.c
 * @brief function that handle magic number of header file.
 */

#include "bx_binhead.h"

int reverse_4bytes(int n) {
    return ((n & 0xFF) << (BYTE * 3) )     |
        ((n & (0xFF << BYTE*3)) >> BYTE*3) |
        ((n & (0xFF << BYTE*1)) << BYTE*1) | 
        ((n & (0xFF << BYTE*2)) >> BYTE*1);
}

unsigned long long int reverse_8byte_bin(unsigned long long int n) {
    return
        ((n & (0xFFULL << (BYTE * 0))) << (BYTE * 7)) |
        ((n & (0xFFULL << (BYTE * 1))) << (BYTE * 5)) |
        ((n & (0xFFULL << (BYTE * 2))) << (BYTE * 3)) |
        ((n & (0xFFULL << (BYTE * 3))) << (BYTE * 1)) |
        ((n & (0xFFULL << (BYTE * 4))) >> (BYTE * 1)) |
        ((n & (0xFFULL << (BYTE * 5))) >> (BYTE * 3)) |
        ((n & (0xFFULL << (BYTE * 6))) >> (BYTE * 5)) |
        ((n & (0xFFULL << (BYTE * 7))) >> (BYTE * 7));
}

unsigned long long int reverse_bin(unsigned long long int n) {
    unsigned long long int x = 0;
    while(n > 0) {
        x<<=1;
        x |= (n & 1);
        n>>=1;
    }
    return x;
}

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

bool check_4bytes(unsigned int magic_value, unsigned int magic_number) {
    return magic_value == magic_number || magic_value == reverse_4bytes(magic_number);
}

bool check(unsigned long long int magic_value, unsigned long long int magic_number) {
    unsigned long long int clean_bytes = 0xFFULL << (BYTE * 7);
    return magic_value == magic_number || magic_value == reverse_bytes(magic_number);
}


bool bx_binhead(baseer_target_t *target, unsigned int index, void *arg)
{   
    if (target == NULL || target->block == NULL || index >= target->partition_count)
        return false;

    void *block = BASEER_BLOCK_OFFSET(target, index);
    bparser* bp= bparser_load(BPARSER_MEM, block);
    bp ->source.mem.pos = 0;
    bp ->source.mem.size = target ->size;

    magic_number magics[] = {
        {"ELF", ELF_MAGIC, reverse_bytes(ELF_MAGIC)},
        {"PDF", PDF_MAGIC, reverse_bytes(PDF_MAGIC)},
        {"PNG", PNG_MAGIC, reverse_bytes(PNG_MAGIC)},
    };

    for(int i=0; i< sizeof(magics)/ sizeof(magics[0]); i++)
    {
        int len = count_bytes(magics[i].number);
        void* pattern = malloc(len);
        size_t n = bparser_read(bp, pattern, len);
        unsigned char* p = (unsigned char*)pattern;
        unsigned char* mgn = (unsigned char*)&magics[i].number;
        unsigned char* mgn_r = (unsigned char*)&magics[i].rnumber;

        bool flag = 1;
        for(int j=0; j< n; j++) {
            flag &= (*mgn == *p) || (*mgn_r == *p); 
            if(!flag) {
                break;
            }
            p++, mgn++, mgn_r++;
        }
        printf("\n");

        if(flag) {
            printf("%s\n", magics[i].name);
            break;
        }

        free(pattern);
        pattern = NULL;
    }
    free(bp);



    // printf("%d\n", bp ->source.mem.size);

    // TODO: handle most of magic numbers.
    // unsigned int magic_value_4byte = *((unsigned int*)block);
    // if(check_4bytes(magic_value_4byte, ELF_MAGIC)){
    //     printf("This is ELF file\n");
    //     return true;
    // }

    // if(check(magic_value_4byte, PDF_MAGIC)){
    //     printf("This is PDF file\n");
    //     return true;
    // }



    // unsigned long long magic_value = *((unsigned long long*)block);
    // if(check(magic_value, PNG_MAGIC)){
    //     printf("This is PNG file\n");
    //     return true;
    // }
    return true;
}
