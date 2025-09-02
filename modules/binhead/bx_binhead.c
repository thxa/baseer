#include "bx_binhead.h"

// const unsigned long long int BYTE = 8;

int reverse_4bytes(int n) {
    return ((n & 0xFF) << (BYTE * 3) )     |
        ((n & (0xFF << BYTE*3)) >> BYTE*3) |
        ((n & (0xFF << BYTE*1)) << BYTE*1) | 
        ((n & (0xFF << BYTE*2)) >> BYTE*1);
}


/*
 * [0] [1] [2] [3] [4] [5] [6] [7]
 * [7] [6] [5] [4] [3] [2] [1] [0]
 *
 *
 *
 */

// unsigned long long int reverse_8byte_bin(unsigned long long int n) {
//      return (
//          ((n & 0xFFULL) << (BYTE * 7) )                  |
//          ((n & (0xFFULL << (BYTE * 7)) >> (BYTE * 7)))    |

//          ((n & (0xFFULL << (BYTE * 1)) << (BYTE * 5)))    |
//          ((n & (0xFFULL << (BYTE * 6)) >> (BYTE * 5)))    |


//          ((n & (0xFFULL << (BYTE * 2)) << (BYTE * 3)))    |
//          ((n & (0xFFULL  << (BYTE * 5)) >> (BYTE * 3)))    |


//          ((n & (0xFFULL << (BYTE * 3)) << (BYTE * 1)))    |
//          ((n & (0xFFULL << (BYTE * 4)) >> (BYTE * 1)))
//          );
//
    // unsigned long long int x = 0;
    // printf("n -> %064b\n", n);
    // printf("x -> %064b\n", x);
    // int i = 0;
    // while(n) {
        // printf("x -> %064b\n", x);
        // x |= n & 1;
        // x<<=1, n>>=1;
        // i++;
    // }
    // while(i < 64) {
    //     x <<= 1;
        // i++;
    // }
    // return x;
// }

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

    printf("%d\n", cnt);
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
    for(int i=0; i<count_bytes(magic_value); i++) {

    }

    return magic_value == magic_number || magic_value == reverse_bytes(magic_number);
}


bool bx_binhead(baseer_target_t *target, unsigned int index, void *arg)
{   
    if (target == NULL || target->block == NULL || index >= target->partition_count)
        return false;

    void *partition = BASEER_BLOCK_OFFSET(target, index);

    // TODO: handle most of magic numbers.
    unsigned int magic_value_4byte = *((unsigned int*)partition);
    if(check_4bytes(magic_value_4byte, ELF_MAGIC)){
        printf("This is ELF file\n");
        return true;
    }

    unsigned long long magic_value = *((unsigned long long*)partition);
    if(check(magic_value, PNG_MAGIC)){
        printf("This is PNG file\n");
        return true;
    }

    if(check(magic_value, PDF_MAGIC)){
        printf("This is PDF file\n");
        return true;
    }

    return true;
}
