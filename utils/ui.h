#include "../baseer.h"
/**
 * @file ui.h
 * @brief User interface utilities for Baseer.
 */

#ifndef UI_H
#define UI_H

/**
 * @brief Prints the Baseer banner.
 */
void print_banner(void);
void print_usage(void);
void display_byte(const unsigned char *byte);
void display_byte_char(const unsigned char *byte);
void print_hex_header(unsigned long long offset);

#endif
