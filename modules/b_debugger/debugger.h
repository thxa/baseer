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
static func_list cmds[] = {
    {"bp",setBP},
    {"dp",delBP},
    {"lp",listBP},
    {"so",step_over},
    {"x",examin_mem},
    {"set",set_mem_reg},
    {"h",handle_action},
    {"c",handle_action},
    {"q",handle_action},
    {"si",handle_action},
    {"vmmap",handle_action},
    {"i",handle_action},
};
static pos_name flags[] = {
    {"CF",0},
    {"PF",2},
    {"AF",4},
    {"ZF",6},
    {"SF",7},
    {"DF",10},
    {"OF",11},
};
static pos_name regs_64[] = {
{"RAX ",0x50},
{"RDX ",0x60},
{"RCX ",0x58},
{"RBX ",0x28},
{"RDI ",0x80},
{"RSI ",0x68},
{"R8  ",0x48},
{"R9  ",0x40},
{"R10 ",0x38},
{"R11 ",0x30},
{"R12 ",0x18},
{"R13 ",0x10},
{"R14 ",0x8},
{"R15 ",0},
{"RSP ",0x98},
{"RBP ",0x20},
{"RIP ",0x80},
};
static pos_name regs_32[] = {
{"EAX ",0x50},
{"EDX ",0x60},
{"ECX ",0x58},
{"EBX ",0x28},
{"EDI ",0x80},
{"ESI ",0x68},
{"R8d  ",0x48},
{"R9d  ",0x40},
{"R10d ",0x38},
{"R11d ",0x30},
{"R12d ",0x18},
{"R13d ",0x10},
{"R14d ",0x8},
{"R15d ",0},
{"ESP ",0x98},
{"EBP ",0x20},
{"EIP ",0x80},
};
#endif /* DEBUG_H */

