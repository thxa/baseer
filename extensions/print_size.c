#include <stdio.h>
#include <stdbool.h>
#include "../baseer.h"  


bool print_size(baseer_target_t* target) {
    if (target == NULL)
        return false;
    printf("file size: %d\n", target ->size);
    return true;
}

baseer_extension_t baseer_extension = {
    {"PrintSize", "Thamer", "Prints file size."}, 
    print_size
};
