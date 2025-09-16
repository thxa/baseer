#ifndef DEBUG_H
#define DEBUG_H
#include "../bparser/bparser.h"
#include <stdint.h>
#include <stdbool.h>
#include <sys/user.h>


typedef struct bp_list bp_list;
typedef struct bp bp;
typedef struct sym_list sym_list;
typedef struct context context;
typedef struct func_list func_list;
typedef bool (*func_callback_t)(context *ctx,void *args);
typedef struct{
    char *op;
    uint64_t addr;
    uint64_t extra;
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
struct context{
    char *mmaps;
    uint64_t base;
    unsigned long entry;
    struct user_regs_struct regs;
    bp_list *list;
    sym_list *sym;
    Cmd cmd;
    unsigned int pid;
    bool do_wait;
    uint32_t arch;
};

struct func_list{
    char *cmd;
    func_callback_t func;
};

void destroy_bp(bp *bpoint);
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
bp* findBP(context *ctx, uint64_t rip);
void restore_all_BP(context *ctx,int opt);
bool handle_action(context *ctx,void *);
void parse_cmd(context *ctx);
static func_list cmds[] = {
    {"bp",setBP},
    {"dp",delBP},
    {"lp",listBP},
    {"so",step_over},
    {"h",handle_action},
    {"c",handle_action},
    {"q",handle_action},
    {"si",handle_action},
    {"vmmap",handle_action},
};
#endif
