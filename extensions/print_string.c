#include <stdio.h>
#include <stdbool.h>
#include "../baseer.h"  

bool print_string(baseer_target_t* target) {
    if (target == NULL)
        return false;
    char *ptr = (char *)target->block;
    for (int i = 0; i < target->size; i++)
        printf("%c", ptr[i]);

    printf("\n\n");
    return true;
}

baseer_extension_t baseer_extension = {
    {"PrintString", "Thamer", "Prints file as chars."}, 
    print_string
};
