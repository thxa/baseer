/**
 * @file flags.h
 * @brief Flag utility functions to determine execution mode.
 */

#ifndef FLAGS_H
#define FLAGS_H

#include <stdbool.h>

/**
 * @brief Checks if a command-line flag requires memory mode.
 * 
 * Memory mode is used for tools that require the whole file in memory.
 *
 * @param mode Command-line flag string
 * @return true if memory mode is required, false otherwise
 */
bool is_memory_mode(const char *mode);

/**
 * @brief Checks if a command-line flag requires streaming mode.
 * 
 * Streaming mode is used for tools that read the file sequentially.
 *
 * @param mode Command-line flag string
 * @return true if streaming mode is required, false otherwise
 */
bool is_stream_mode(const char *mode);

#endif