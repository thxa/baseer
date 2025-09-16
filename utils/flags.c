#include "flags.h"
#include <string.h>

/**
 * @brief Return true if the flag requests memory mode.
 */

bool is_memory_mode(const char *mode){
    return (strcmp(mode, "-m") == 0 ||
            strcmp(mode, "-a") == 0 ||
            strcmp(mode, "-d") == 0);
}

/**
 * @brief Return true if the flag requests stream mode.
 */

bool is_stream_mode(const char *mode){
    return (strcmp(mode, "-c") == 0);
}