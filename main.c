#include "baseer.h"
// #include "modules/default/bx_default.h"
#include "modules/binhead/bx_binhead.h"
#include "utils/ui.h"

/**
 * @brief Program entry point
 * 
 * Parses command-line arguments, opens file in memory and streaming mode
 * as required by the selected tools, and executes each requested tool.
 *
 * @param argc Number of command-line arguments
 * @param args Array of argument strings
 * @return 0 on success, 1 on error
 */



int main(int argc, char** args)
{
    if (argc < 3) {
        print_banner();
        printf("Usage: baseer <file> <flags...>\n");
        printf("Flags:\n      ");
        printf("-m Metadata\n      ");
        printf("-a Disassemble\n      ");
        printf("-c Decompiler\n      ");
        printf("-d Debugger\n");
    }

    baseer_target_t *target = baseer_open(args[1], BASEER_MODE_BOTH);
    if (!target) {
        fprintf(stderr, "[!] Failed to open file in required mode(s)\n");
        if(target) baseer_close(target);
        return 1;
    }

    inputs input = {&argc, args};

    if(target){
        if(!baseer_execute(target , bx_binhead, &input)){
            fprintf(stderr, "[!] Execution error\n");
        }
    }
    
    // Clean up
    if(target) baseer_close(target);

    return 0;
}

