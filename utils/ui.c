#include <stdio.h>
#include "ui.h"

/**
 * @brief Prints the Baseer ASCII banner.
 *
 * Used when program is run with insufficient arguments or for branding.
 */

void print_banner(void) 
{
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

void print_usage(void)
{
    printf("Usage: baseer <file> <flags...>\n");
    printf("\n\033[5;31m( NEW )"COLOR_RESET COLOR_RED" baseer -i     - To use interactive Baseer\n"COLOR_RESET);
    printf("Flags:\n      ");
    printf("-m Metadata\n      ");
    printf("-a Disassemble\n      ");
    printf("-c Decompiler\n      ");
    printf("-d Debugger\n");
}


/**
 * @brief Display a single byte in hexadecimal with color coding.
 *
 * This function prints the value of a byte in hex format (two digits) 
 * and applies a color depending on its meaning:
 * - Default (COLOR_YELLOW) for active instructions.
 * - COLOR_RED for NOP (0x90), INT3 (0xCC), or filler bytes (0xFF).
 * - COLOR_GRAY for padding or null bytes (0x00).
 *
 * @param byte Pointer to the byte to display.
 *
 * @note The color codes are ANSI escape sequences.
 *       The color is reset after printing each byte using COLOR_RESET.
 */
void display_byte(const unsigned char *byte)
{
    const char *color = COLOR_YELLOW; // default for active instruction

    if (*byte == 0x90 || *byte == 0xCC || *byte == 0xff) {
        color = COLOR_RED;  // low / unused
    }
    else if (*byte == 0x00) {
        color = COLOR_GRAY; // padding or null bytes
    }
    printf("%s%02x%s", color, *byte, COLOR_RESET);
}

/**
 * @brief Display a single byte as a printable character with color coding.
 * 
 * This function prints the ASCII representation of a byte. Non-printable
 * bytes are shown as a '.' character. The output is color-coded based
 * on the byte value:
 *   - 0x90, 0xCC, 0xFF → COLOR_RED (low / unused bytes)
 *   - 0x00             → COLOR_GRAY (padding or null bytes)
 *   - All other bytes  → COLOR_YELLOW (default / active instruction)
 * 
 * Printable ASCII characters (32-126) are displayed as-is, while
 * non-printable bytes are represented with '.'.
 * 
 * @param byte Pointer to the byte to display.
 */
void display_byte_char(const unsigned char *byte)
{
    const char *color = COLOR_YELLOW; // default for active instruction 
    unsigned char c = *byte;
    if (*byte == 0x90 || *byte == 0xCC || *byte == 0xff) {
        color = COLOR_RED;  // low / unused
    }
    else if (*byte == 0x00) {
        color = COLOR_GRAY; // padding or null bytes
    }
    if (c >= 32 && c <= 126) { // printable ASCII range
        printf("%s%c%s", color, c, COLOR_RESET);
    } else {
        printf("%s.%s", color, COLOR_RESET); // non-printable
    }
}

/**
 * @brief Print the hex dump header row.
 *
 * This function prints the header line used in a hex dump,
 * showing both the hexadecimal column labels (00–0F) and
 * the ASCII column labels. It also highlights the header
 * with green color using ANSI escape codes.
 *
 * Example output (simplified):
 * @code
 * |
 * |    --Offset--   0 1 2 3 4 5 6 7 8 9 A B C D E F    0123456789ABCDEF
 * @endcode
 *
 * @param offset Starting offset for the hex dump (not currently used
 *               in the header itself, but included for consistency).
 *
 * @note The function assumes ANSI color macros like COLOR_GREEN
 *       and COLOR_RESET are defined.
 */
void print_hex_header(unsigned long long offset)
{
    printf(COLOR_GREEN "\n|\n" COLOR_GREEN);
    printf(COLOR_GREEN "|    --Offset--%3s" COLOR_RESET, "");
    printf(COLOR_GREEN);

    // Hex dump
    for(int i=0; i<=9; i++) {
        printf("%d", i);
        printf((i & 1)?"  ":" ");
    }
    for(char c='A'; c <= 'F'; c++){
        printf("%c", c);
        printf((c & 1)?" ":"  ");
    }
    
    // Ascii Dump
    printf("%-2s", "");
    for(int i=0; i<=9; i++)
        printf("%d", i);
    for(char c='A'; c <= 'F'; c++)
        printf("%c", c);

    printf(COLOR_RESET);
    printf("\n");
}

