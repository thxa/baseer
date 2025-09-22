#include <stdio.h>
#include "ui.h"

/**
 * @brief Prints the Baseer ASCII banner.
 *
 * Used when program is run with insufficient arguments or for branding.
 */

void print_banner(void) {
    printf(
COLOR_BLACK"====================="COLOR_RESET"----.----"COLOR_BLACK"====================\n"COLOR_RESET
COLOR_BLACK"================"COLOR_RESET"--:::..........:::-"COLOR_BLACK"===============\n"COLOR_RESET
COLOR_BLACK"============="COLOR_RESET"-::....................::-"COLOR_BLACK"===========\n"COLOR_RESET
COLOR_BLACK"==========="COLOR_RESET"::........::------::........::-"COLOR_BLACK"========\n"COLOR_RESET
COLOR_BLACK"========="COLOR_RESET":.....:...:-==========-:...::....:-"COLOR_BLACK"======\n"COLOR_RESET
COLOR_BLACK"======="COLOR_RESET":....:--...-==="COLOR_BLUE"--------"COLOR_RESET"===-...--:....:-"COLOR_BLACK"====\n"COLOR_RESET
COLOR_BLACK"====="COLOR_RESET":...:-"COLOR_BLACK"==="COLOR_RESET":..:=="COLOR_BLUE"------------"COLOR_RESET"==:..."COLOR_BLACK"==="COLOR_RESET"-:...:-"COLOR_BLACK"==\n"COLOR_RESET
COLOR_BLACK"==="COLOR_RESET"-....:"COLOR_BLACK"====="COLOR_RESET":..-=="COLOR_BLUE"------------"COLOR_RESET"==-..."COLOR_BLACK"====="COLOR_RESET":....:"COLOR_BLACK"=\n"COLOR_RESET
COLOR_BLACK"===="COLOR_RESET"-:...:-"COLOR_BLACK"==="COLOR_RESET":..:=="COLOR_BLUE"------------"COLOR_RESET"==:..:"COLOR_BLACK"==="COLOR_RESET"-:....-"COLOR_BLACK"==\n"COLOR_RESET
COLOR_BLACK"======"COLOR_RESET":....:-=-...-==="COLOR_BLUE"--------"COLOR_RESET"===-...-=-:....:"COLOR_BLACK"====\n"COLOR_RESET
COLOR_BLACK"========"COLOR_RESET":.....::...:-==========-:...:::....:"COLOR_BLACK"======\n"COLOR_RESET
COLOR_BLACK"=========="COLOR_RESET"-:.........::------::.........:-"COLOR_BLACK"========\n"COLOR_RESET
COLOR_BLACK"============"COLOR_RESET"--:........................:"COLOR_BLACK"==========\n"COLOR_RESET
COLOR_BLACK"================"COLOR_RESET"-:::............::--:....:-"COLOR_BLACK"=======\n"COLOR_RESET
COLOR_BLACK"======================"COLOR_RESET"---.---"COLOR_BLACK"========="COLOR_RESET"-:...:-"COLOR_BLACK"=====\n"COLOR_RESET
COLOR_BLACK"========================================"COLOR_RESET"-:...:-"COLOR_BLACK"===\n"COLOR_RESET
COLOR_BLACK"=========================================="COLOR_RESET"-:...:"COLOR_BLACK"==\n"COLOR_RESET
COLOR_BLACK"============================================"COLOR_RESET"-:-"COLOR_BLACK"===\n"COLOR_RESET
COLOR_BLACK"==================================================\n"COLOR_RESET
COLOR_BLACK"=================================="COLOR_RESET"--"COLOR_BLACK"==============\n"COLOR_RESET
COLOR_BLACK"======"COLOR_RESET":::"COLOR_BLACK"======"COLOR_RESET"::-"COLOR_BLACK"======"COLOR_RESET":::"COLOR_BLACK"=="COLOR_RESET"--:......:-"COLOR_BLACK"======="COLOR_RESET"::-\n"
COLOR_BLACK"======"COLOR_RESET"..:"COLOR_BLACK"====="COLOR_RESET"-:.-"COLOR_BLACK"======"COLOR_RESET"...-:..:--"COLOR_BLACK"=="COLOR_RESET"--..:"COLOR_BLACK"====="COLOR_RESET"-..-\n"
COLOR_BLACK"======"COLOR_RESET"..:"COLOR_BLACK"====="COLOR_RESET"-:.:"COLOR_BLACK"======"COLOR_RESET"....:-"COLOR_BLACK"========"COLOR_RESET":.."COLOR_BLACK"====="COLOR_RESET"-..-\n"
COLOR_BLACK"======"COLOR_RESET"..:-----:..:------...-----------:..-----:..-\n"
COLOR_BLACK"====="COLOR_RESET"-.........::..............................:-"COLOR_BLACK"=\n"COLOR_RESET
COLOR_BLACK"====="COLOR_RESET":..-"COLOR_BLACK"=========================================\n"COLOR_RESET
COLOR_BLACK"="COLOR_RESET"---:..:"COLOR_BLACK"===="COLOR_RESET"-:-=-:-"COLOR_BLACK"=========================="COLOR_RESET"-::"COLOR_BLACK"==\n"COLOR_RESET
"-...::-"COLOR_BLACK"====="COLOR_RESET"-:-=-:-"COLOR_BLACK"==========================="COLOR_RESET"::"COLOR_BLACK"==\n"COLOR_RESET
COLOR_BLACK"==================================================\n"COLOR_RESET
    );
    printf("Baseer version: %s\n\n", BASEER_VERSION);
}

void print_usage(void){
    printf("Usage: baseer <file> <flags...>\n");
    printf("\n\033[5;31m( NEW )"COLOR_RESET COLOR_RED" baseer -i     - To use interactive Baseer\n"COLOR_RESET);
    printf("Flags:\n      ");
    printf("-m Metadata\n      ");
    printf("-a Disassemble\n      ");
    printf("-c Decompiler\n      ");
    printf("-d Debugger\n");
}