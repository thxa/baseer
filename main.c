#include "baseer.h"
// #include "modules/default/bx_default.h"
#include "modules/binhead/bx_binhead.h"
#include "modules/b_debugger/debugger.h"


int main(int argc, char**args)
{
    if(argc != 2) {
        printf("Baseer version: %s\n", BASEER_VERSION);
        return 1;
    }
    
    baseer_target_t *target = baseer_open(args[1]);
    if (target == NULL)
        return 1;

    // if (!baseer_execute(target, bx_default, NULL))
    //     printf("Execution error\n");
    
    // printf("%s\n", args[1]);
    //if (!baseer_execute(target, bx_binhead, NULL))
    //    printf("Execution error\n");

    if (!baseer_execute(target, b_debugger, NULL))
        printf("Execution error\n");
    // if (!baseer_execute(target, bparser_load, NULL))
    //     printf("Execution error\n");


    baseer_close(target);
    return 0;
}
