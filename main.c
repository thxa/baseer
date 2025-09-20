#include "baseer.h"
// #include "modules/default/bx_default.h"
#include "modules/binhead/bx_binhead.h"
#include "utils/ui.h"
// #include "utils/flags.h"

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

    // bool need_memory = false;
    // bool need_stream = false;

    //Determine required modes besed on flags
    // for (int i = 2; i < argc; i++) {
    //     if(is_memory_mode(args[i])) need_memory = true;
        // if(is_stream_mode(args[i])) need_stream = true;
    // }

    // printf("mem mode: %d, stream mode: %d\n", need_memory, need_stream);

    // Open file in required modes
    // baseer_target_t *mem_target = need_memory ? baseer_open_memory(args[1]) : NULL;
    // baseer_target_t *stream_target = need_stream ? baseer_open_file(args[1]) : NULL;

    baseer_target_t *target = baseer_open(args[1]);
//     if ((need_memory && !mem_target)) {
//         fprintf(stderr, "[!] Failed to open file in required mode(s)\n");
//         if(mem_target) baseer_close(mem_target, MEMORY);
//         if(stream_target) baseer_close(stream_target, STREAM);
//         return 1;
//     }

    // Execute all requested tools
    for (int i = 2; i < argc; i++) {
        int fake_argc = 3;
        char *fake_args[4] = {args[0], args[1], args[i], NULL};
        inputs input = {&fake_argc, fake_args};

        // baseer_target_t *target = NULL;
        // if(is_memory_mode(args[i]) && mem_target) target = mem_target;
        // else if(is_stream_mode(args[i]) && stream_target) target = stream_target;
        // printf("%p\n", target);
        if(target){
            if(!baseer_execute(target , bx_binhead, &input)){
                fprintf(stderr, "[!] Execution error for flag %s\n", args[i]);
            }
        } else {
            fprintf(stderr, "[!] Unsupported flag: %s\n", args[i]);
        }

    }
    
    // Clean up
    baseer_close(target);
    // if(mem_target) baseer_close(mem_target, MEMORY);
    // if(stream_target) baseer_close(stream_target, STREAM);

    return 0;
}

