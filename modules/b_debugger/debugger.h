#ifndef DEBUG_H
#define DEBUG_H
#include "../bparser/bparser.h"
#include <stdint.h>
#include <stdbool.h>
#include <sys/user.h>

#define ERROR(str) printf(COLOR_RED "[x] " COLOR_RESET "%s",str);
#define INFO(str) printf(COLOR_BLUE "[*] " COLOR_RESET "%s",str);

typedef struct bp_list bp_list;
typedef struct bp bp;
typedef struct sym_list sym_list;
typedef struct context context;
typedef struct func_list func_list;
typedef struct pos_name pos_name;
typedef bool (*func_callback_t)(context *ctx,void *args);
typedef struct{
    char *op;
    uint64_t addr;
}Cmd;
struct sym_list{
    char *name;
    uint64_t addr;
    sym_list *next;
};
struct bp_list{
    bp *first;
    bp *last;
    uint32_t counter ;
};
struct bp{
    uint64_t orig;
    uint64_t addr;
    bp *next;
    unsigned int id;
};

struct func_list{
    char *cmd;
    func_callback_t func;
};

struct pos_name{
    char *name;
    int pos;
};
// base will have 2 bit flags to PIE status and exit the program
struct context{
    char *mmaps;
    uint64_t base;
    uint64_t entry;
    struct user_regs_struct regs;
    bp_list *list;
    sym_list *sym;
    Cmd cmd;
    unsigned int pid;
    bool do_wait;
    uint32_t arch;
};
void destroy_bp_sym(context *ctx);
void destroy_all(context *ctx);
void print_helpCMD();
void init_values(bparser *target,context *ctx);
bool b_debugger(bparser *target,void *arg);
void dis_ctx(context *ctx);
void handle_bpoint(context *ctx);
bool setBP(context *ctx, void *args);
bool delBP(context *ctx, void *args);
bool step_over(context *ctx,void *args);
bool listBP(context *ctx, void *args);
bool examin_mem(context *ctx,void *args);
bool set_mem_reg(context *ctx,void *args);
void restore_all_BP(context *ctx,int opt);
bool handle_action(context *ctx,void *);
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
#endif
