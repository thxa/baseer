#ifndef DEBUG_H
#define DEBUG_H
#include "../bparser/bparser.h"
#include <stdint.h>
#include <sys/user.h>


typedef struct bp_list bp_list;
typedef struct bp bp;
/*
 * bp: set a breakpoint {ex: bp addr}
 * dp: delete a breakpoint {ex: dp addr}
 * lb: list all breakpoints {ex: lb}
 * */
enum {
	CMD_bp,
	CMD_dp,
	CMD_lp,
	CMD_C,
	CMD_si,
	CMD_COUNT,
};
static char *cmds[CMD_COUNT] = {
	"bp",
	"dp",
	"lp",
	"c",
	"si",
};
typedef struct{
	char *op;
	uint64_t addr;
	uint64_t extra; // value or mybe empty
}Cmd;
struct bp_list{
	bp *first;
	bp *last;
	uint32_t counter ;
};
struct bp{
	unsigned int id;
	uint64_t orig;
	uint64_t addr; // address of the breakpoint
	bp *next;
};
typedef struct{
	unsigned int pid;
	uint64_t entry;
	struct user_regs_struct regs;
	bp_list *list;
	Cmd cmd;
}context;
/*
 * bp commands
 * */
void init_values(bparser *target,context *ctx);
bool b_debugger(bparser *target,void *arg);
void dis_ctx(context *ctx);
void handle_bpoint(context *ctx);
void setBP(context *ctx, uint64_t addr);
void delBP(context *ctx, uint32_t id);
void listBP(context *ctx);
bp* findBP(context *ctx, uint64_t rip);
void restore_all_BP(context *ctx,int opt);
void parseCmd(context *ctx);
#endif
