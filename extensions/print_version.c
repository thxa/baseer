#include <stdio.h>
#include <stdbool.h>
#include "../baseer.h"  

bool print_version(baseer_target_t* target) {
    if (target == NULL)
        return false;
    printf("Tool version: %d\n", target ->version);
    return true;
}

baseer_extension_t baseer_extension = {
    {"PrintVersion", "Thamer", "Prints app version"}, 
    print_version
};
