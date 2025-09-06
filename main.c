#include "baseer.h"
// #include "modules/default/bx_default.h"
#include "modules/binhead/bx_binhead.h"
#include "modules/b_debugger/debugger.h"


int main(int argc, char**args)
{

    if (argc != 3) {
        printf("Baseer version: %s\n\n", BASEER_VERSION);
        printf("Usage:\n");
        printf("  %s <file> -m   ; for metadata of a file\n", args[0]);
        printf("  %s <file> -a   ; for disassemble of a file\n", args[0]);
        printf("  %s <file> -d   ; for debugger of a file\n", args[0]);
        return 1;
    }
    inputs input = {&argc, args};

    baseer_target_t *target = baseer_open(args[1]);
    if (target == NULL)
        return 1;

    // if (!baseer_execute(target, bx_default, NULL))
    //     printf("Execution error\n");
    
    // printf("%s\n", args[1]);
<<<<<<< HEAD
    if (!baseer_execute(target, bx_binhead, &input))
        printf("Execution error\n");
=======
    //if (!baseer_execute(target, bx_binhead, NULL))
    //    printf("Execution error\n");
>>>>>>> 84aec77b567b7e9c49ad1df775d85bf60e42b671

    if (!baseer_execute(target, b_debugger, NULL))
        printf("Execution error\n");
    // if (!baseer_execute(target, bparser_load, NULL))
    //     printf("Execution error\n");


    baseer_close(target);
    return 0;
}
