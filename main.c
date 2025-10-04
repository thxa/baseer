#include "baseer.h"
// #include "modules/default/bx_default.h"
#include "modules/binhead/bx_binhead.h"
#include "utils/ui.h"
#include "utils/b_CLI.h"
#include "modules/b_hashmap/b_hashmap.h"
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
    linenoiseClearScreen();

    print_banner();
    if (argc == 2 && strcmp("-i", args[1]) == 0) {
        baseer_CLI();
        return 0;
    } 
    if (argc < 3){
        print_usage();
        fprintf(stderr, "[!] Invalid usage.\n");
        return 1;
    }
    if (argc >= 3){
        baseer_target_t *target = baseer_open(args[1], BASEER_MODE_BOTH);
        if (!target) {
            fprintf(stderr, COLOR_RED"[!] Failed to open file : "COLOR_RESET"%s\n", args[1]);
            return 1;
        }

        inputs input = {&argc, args};

        parse_args(&input);
        
        if(target){
            if(!baseer_execute(target , bx_binhead, &input)){
                fprintf(stderr, "[!] Execution error\n");
            }
        }
        if(target) baseer_close(target);


        return 0;
    }
    print_usage();
    return 1;
}

