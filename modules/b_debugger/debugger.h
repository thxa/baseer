/**
 * @file debugger.h
 * @brief Core definitions, structs, and function prototypes for the debugger.
 *
 * This header defines the main data structures (context, breakpoints, symbols),
 * and function prototypes used to implement the debugger.
 */

#ifndef DEBUG_H
#define DEBUG_H

#include "../bparser/bparser.h"
#include <stdint.h>
#include <stdbool.h>
#include <sys/user.h>

/**
 * @brief Print an error message in red.
 */
#define ERROR(str) printf(COLOR_RED "[x] " COLOR_RESET "%s",str);

/**
 * @brief Print an info message in blue.
 */
#define INFO(str) printf(COLOR_BLUE "[*] " COLOR_RESET "%s",str);

/**
 * @brief Forward declaration of debugger structs.
 */
typedef struct bp_list bp_list;
typedef struct bp bp;
typedef struct sym_list sym_list;
typedef struct context context;
typedef struct func_list func_list;
typedef struct pos_name pos_name;

/**
 * @brief Callback type for command handler functions.
 * @param ctx The current debugger context.
 * @param args Additional arguments for the command.
 * @return true if successful, false otherwise.
 */
typedef bool (*func_callback_t)(context *ctx, void *args);

/**
 * @brief Represents a parsed user command.
 */
typedef struct {
    char *op;       /**< Command operation string */
    uint64_t addr;  /**< Optional command address argument */
} Cmd;

/**
 * @brief Represents a symbol entry (function name and address).
 */
struct sym_list {
    char *name;         /**< Symbol name */
    uint64_t addr;      /**< Symbol address */
    sym_list *next;     /**< Pointer to next symbol */
};

/**
 * @brief A linked list of breakpoints.
 */
struct bp_list {
    bp *first;          /**< First breakpoint */
    bp *last;           /**< Last breakpoint */
    uint32_t counter;   /**< Breakpoint counter (used for IDs) */
};

/**
 * @brief Represents a single breakpoint.
 */
struct bp {
    uint64_t orig;      /**< Original instruction word */
    uint64_t addr;      /**< Breakpoint address */
    bp *next;           /**< Next breakpoint in list */
    unsigned int id;    /**< Breakpoint ID */
};

/**
 * @brief Represents a debugger command and its associated function.
 */
struct func_list {
    char *cmd;                  /**< Command string */
    func_callback_t func;       /**< Function pointer */
};

/**
 * @brief Maps a register/flag name to its position.
 */
struct pos_name {
    char *name;     /**< Name of register or flag */
    int pos;        /**< Byte or bit offset */
};

/**
 * @brief Holds debugger state and process information.
 */
struct context {
    char *mmaps;                /**< Process memory mappings (/proc/pid/maps) */
    uint64_t base;              /**< Base address of the binary (PIE flag in LSB) */
    uint64_t entry;             /**< Entry point address */
    struct user_regs_struct regs; /**< CPU register state */
    bp_list *list;              /**< List of breakpoints */
    sym_list *sym;              /**< List of symbols */
    Cmd cmd;                    /**< Current user command */
    unsigned int pid;           /**< Debugged process PID */
    uint32_t arch;              /**< Architecture (32 or 64 bit) */
    bool do_wait;               /**< Whether to wait for process stop */
    bool pie;                   /**< True if the target binary is Position Independent Executable (PIE) */
    bool do_exit;               /**< Set when the debugger should terminate */
    char *last_cmd;             /**< Last entered command line (for Enter repeat) */
};

/* ==== Function Prototypes ==== */

/**
 * @brief Free all breakpoints and symbols in the context.
 */
void destroy_bp_sym(context *ctx);

/**
 * @brief Destroy all context-related memory.
 */
void destroy_all(context *ctx);

/**
 * @brief Print available debugger commands.
 */
void print_helpCMD();

/**
 * @brief Initialize context values from target binary.
 */
void init_values(bparser *target, context *ctx);

/**
 * @brief Launch and manage the debugger main loop.
 */
bool b_debugger(bparser *target, void *arg);

/**
 * @brief Display current registers, flags, disassembly, and stack.
 */
void dis_ctx(context *ctx);

/**
 * @brief Handle breakpoint hit logic.
 */
void handle_bpoint(context *ctx);

/**
 * @brief Set a breakpoint at an address or symbol.
 */
bool setBP(context *ctx, void *args);

/**
 * @brief Delete a breakpoint by ID.
 */
bool delBP(context *ctx, void *args);

/**
 * @brief Step over a function call.
 */
bool step_over(context *ctx, void *args);

/**
 * @brief List all breakpoints.
 */
bool listBP(context *ctx, void *args);

/**
 * @brief Examine memory at a given address.
 */
bool examin_mem(context *ctx, void *args);

/**
 * @brief Modify memory or registers.
 */
bool set_mem_reg(context *ctx, void *args);

/**
 * @brief Restore all breakpoints (enable or disable).
 */
void restore_all_BP(context *ctx, int opt);

/**
 * @brief Execute a user command.
 */
bool handle_action(context *ctx, void *args);

/**
 * @brief Parse and dispatch a command from the user.
 */
void parse_cmd(context *ctx);

#endif /* DEBUG_H */

