#define _GNU_SOURCE
#include <stdint.h>
#include "elf.h"
#include <stdio.h>
#include <ctype.h>
#include "udis86.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "debugger.h"
#include <sys/ptrace.h>
#include "../bx_elf_utils/bx_elf_utils.h"

/* Command dispatch table */
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

/* CPU flags bit positions */
static pos_name flags[] = {
    {"CF",0},
    {"PF",2},
    {"AF",4},
    {"ZF",6},
    {"SF",7},
    {"DF",10},
    {"OF",11},
};

/* 64-bit register name-to-offset mappings */
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

/* 32-bit register name-to-offset mappings */
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

/**
 * @file debugger.c
 * @brief Implementation of a lightweight debugger for ELF binaries using ptrace.
 *
 * This file provides functionality for:
 * - Parsing user commands
 * - Handling breakpoints
 * - Inspecting and modifying memory and registers
 * - Displaying disassembly and process state
 * - Running a target program under debugger control
 */


/**
 * @brief Parse and execute a user command entered in the debugger prompt.
 *
 * Reads a command from stdin, splits it into operator and arguments,
 * and executes the corresponding debugger function.
 *
 * @param ctx Pointer to debugger context structure containing process state.
 */
void parse_cmd(context *ctx){
	if(ctx == NULL)
		return;
	bool flag = false;
        char *cmd = linenoise(COLOR_WHITE"Baseer-\033[5;34mDBG"COLOR_RESET"-> "COLOR_RESET);
        if(!cmd) return;
        if(*cmd) linenoiseHistoryAdd(cmd);
	cmd[strcspn(cmd, "\n")] = 0;
	char *op ;
	char *args ;

	char *delm = strchr(cmd,' ');
	if (delm){
		*delm = '\0';
		op = cmd;
		args = delm +1;
		if(*args == '\0')
			args = NULL;
	}else {
		op = cmd;
		args = NULL;
	}
	ctx->cmd.op = strdup(op); 
	uint32_t len = sizeof(cmds)/sizeof(func_list); 
	for (int i = 0; i< len ; i++) {
		if(strcmp(ctx->cmd.op,cmds[i].cmd) == 0){
			if(ctx->base == 0 && (strcmp(ctx->cmd.op,"q") != 0) ){
				free(cmd);
				return;
			}
			flag = cmds[i].func(ctx,(void*)args);
			break;

		}

	}
	if(!flag){
		ERROR("wrong command\n");
		print_helpCMD();
		ctx->do_wait = false;
	}
	free(cmd);

}

/**
 * @brief Handle actions that correspond directly to simple commands
 * (e.g., quit, help, continue, single-step).
 *
 * @param ctx Pointer to debugger context.
 * @param args Optional arguments passed with the command.
 * @return true if the command was successfully handled, false otherwise.
 */
bool handle_action(context *ctx,void *args){

	if(strcmp(ctx->cmd.op,"q") == 0){
		ptrace(PTRACE_CONT, ctx->pid, NULL, SIGKILL);
		ctx->do_wait = false;
		ctx->do_exit = true;
		return true;
	}else if (strcmp(ctx->cmd.op,"h") == 0) {
		print_helpCMD();
		return true;
	}else if (strcmp(ctx->cmd.op,"vmmap") == 0) {
		free(ctx->mmaps);
		ctx->mmaps = NULL;
		char mmaps[512] = {0};
		snprintf(mmaps, sizeof(mmaps), "/proc/%d/maps", ctx->pid);
		FILE *file = fopen(mmaps,"r");
		if (!file) {
			ERROR("failed to open process maps\n");
			ctx->do_wait = false;
			return true;
		}
		size_t size = 0;
		getdelim(&ctx->mmaps,&size,'\0' , file);
		fclose(file);
		if (ctx->mmaps)
			printf("%s\n",ctx->mmaps);
		ctx->do_wait = false;
		return true;
	}else if (strcmp(ctx->cmd.op,"c") == 0) {
		restore_all_BP(ctx,0);
		if (ptrace(PTRACE_CONT, ctx->pid, 0, 0) == -1) {
			perror("PTRACE_CONT");
		}
		ctx->do_wait = true;
		return true;
	}else if (strcmp(ctx->cmd.op,"si") == 0) {
		if (ptrace(PTRACE_SINGLESTEP, ctx->pid, 0, 0) == -1) {
			perror("SINGLESTEP");
			return true;
		}
		ctx->do_wait = true;
		return true;
	}else if (strcmp(ctx->cmd.op,"i") == 0) {
		sym_list *sym = ctx->sym;
		while (sym != NULL) {
			printf("function %s 0x%lx\n",sym->name,sym->addr);
			sym = sym->next;
		}
		ctx->do_wait = false;
		return true;
	}

	return false;
}

/**
 * @brief Print the list of available debugger commands.
 */
void print_helpCMD(){
	printf("\n");
	printf(COLOR_BLUE "bp   " COLOR_RESET " : set breakpoint {ex: bp 0x12354 or bp func_name}\n");
	printf(COLOR_BLUE "dp   " COLOR_RESET " : delete breakpoint {ex: dp breakpoint_id}\n");
	printf(COLOR_BLUE "lp   " COLOR_RESET " : list all breakpoints {ex: lp}\n");
	printf(COLOR_BLUE "si   " COLOR_RESET " : take one step execution (step into) {ex: si}\n");
	printf(COLOR_BLUE "so   " COLOR_RESET " : take one step execution (step over){ex: so}\n");
	printf(COLOR_BLUE "c    " COLOR_RESET " : continue execution {ex: c}\n");
	printf(COLOR_BLUE "h    " COLOR_RESET " : display help commands {ex: h}\n");
	printf(COLOR_BLUE "vmmap" COLOR_RESET " : display maps memory {ex: vmmap}\n");
	printf(COLOR_BLUE "i    " COLOR_RESET " : display functions name and address {ex: i}\n");
	printf(COLOR_BLUE "x    " COLOR_RESET " : examin value in memory {ex: x addr size : x 0x1234 10}\n");
	printf(COLOR_BLUE "set  " COLOR_RESET " : change memory or register value {ex: set $eax=0x20 : set 0x1234=0x20}\n");
	printf(COLOR_BLUE "q    " COLOR_RESET " : for quit the debugger \n");
}

/**
 * @brief Modify the value of a register or memory location.
 *
 * Command format: `$REG=VALUE` or `ADDR=VALUE`
 *
 * @param ctx Pointer to debugger context.
 * @param args Command argument string.
 * @return true on success, false otherwise.
 */
bool set_mem_reg(context *ctx,void *args){
	ctx->do_wait = false;
	char *arg = (char*)args;
	if(!arg || *arg == '\0')
		return false;
	while(*arg != '\0' && isspace((unsigned char)*arg))arg++;
	uint32_t counter = 0 ;
	char *tokens[2] = {0};
	char *token = strtok(arg,"=");
	if(token == NULL){
		return false;
	}
	while(token != NULL && counter < 2){
		tokens[counter++] = token;
		token = strtok(NULL," ");
	}
	if(counter < 2) 
		return false;
	uint64_t val = (strstr(tokens[1],"0x") != NULL) ? strtoul(tokens[1], NULL, 16) : strtoul(tokens[1], NULL, 10);
	if(val == 0){
		ERROR("wrong value\n");
		return false;
	}
	// check if the first is register 
	if(tokens[0][0] == '$'){
		tokens[0]++ ;
		int len = sizeof(regs_64)/sizeof(pos_name);
		pos_name *regs = (ctx->arch == 64) ? regs_64 : regs_32;
       
		char *ptr = tokens[0];
		while(*ptr){
			if(isupper(*ptr))
				continue;
			*ptr = toupper(*ptr);
			ptr++;
		}
		for (int i=0; i< len; i++) {
			if(strncmp(tokens[0], regs[i].name,3) == 0){
				*(unsigned long*)((unsigned char*)&ctx->regs + regs[i].pos)  = val;
			}
		
		}
		ptrace(PTRACE_SETREGS, ctx->pid, 0, &ctx->regs);
	
	// if the first is memory address 
	}else if (strstr(tokens[0],"0x") != NULL) {
		uint64_t addr = strtoul(tokens[0], NULL, 16);
		if (ptrace(PTRACE_POKETEXT, ctx->pid, (void*)addr, (void*)val) == -1) {
			ERROR("invalid address\n");
			return false;
		}
	
	}
	return true;
}

/**
 * @brief Examine memory at a specific address.
 *
 * Command format: `x ADDR SIZE`
 *
 * @param ctx Pointer to debugger context.
 * @param args Address and size arguments.
 * @return true on success, false otherwise.
 */
bool examin_mem(context *ctx,void *args){
	char *arg = (char*)args;
	if(*arg == '\0') return false;
	while(*arg != '\0' && isspace((unsigned char)*arg))arg++;
	uint64_t addr = 0 ;
	uint64_t size = 0 ;
	uint32_t counter = 0 ;
	char *tokens[2] = {0};
	char *token = strtok(arg," ");
	if(token == NULL){
		return false;
	}
	while(token != NULL && counter < 2){
		tokens[counter++] = token;
		token = strtok(NULL," ");
	}
	if(counter == 2){
		addr = strtoul(tokens[0],NULL,16);
		size = strtoul(tokens[1],NULL,10);
	}else {
		addr = strtoul(tokens[0],NULL,16);
		size = 1;
	}

	char *fmt = (ctx->arch == 64) ? " 0x%016llx" : " 0x%08x";
	for (int i = 0; i < size; i++) {

		long v = ptrace(PTRACE_PEEKTEXT, ctx->pid, 
				(void*)(addr + i*(ctx->arch / 8 )), NULL);
		if(v == -1){
			ERROR("wrong address \n");
			return true;
		}
		if(i % 2 == 0 ){
			printf(COLOR_YELLOW "0x%lx" COLOR_RESET " : " , 
				(addr + i * (ctx->arch / 8 )));
			printf(fmt,v);

		}else {
			printf(fmt ,v);
			printf("\n");
		}
	}
	if(size % 2 == 1)
		printf("\n");
	return true;
}


/**
 * @brief Delete a breakpoint by ID.
 *
 * @param ctx Pointer to debugger context.
 * @param args Breakpoint ID.
 * @return true if deletion succeeds, false otherwise.
 */
bool delBP(context *ctx, void* args){
	char *arg = (char*)args;
	ctx->do_wait = false;
	while(isspace((unsigned char)*arg)) arg++;
	char *token = strtok(arg, " ");
	uint64_t id = strtol(token,NULL,16);
	if(id == 0){
		ERROR("wrong id\n");
		return false;
	}
	if(ctx->list->first == NULL){
		INFO("no breakpoints found\n");
		return true;
	}
	bp *ptr = ctx->list->first;
	bp *tmp = NULL;
	while (ptr != NULL) {
		if(ptr->id == id){

			if (ptrace(PTRACE_POKETEXT, ctx->pid, (void*)ptr->addr, (void*)ptr->orig) == -1) {
				perror("PTRACE_POKETEXT");
				return true;
			}
			if(tmp != NULL){
				tmp->next = ptr->next;
			}else {
				ctx->list->first = ptr->next;
			}

			if(ptr == ctx->list->last){
				ctx->list->last = tmp;
			}
			INFO("")
			printf("deleted breakpoint at: %lx\n",ptr->addr);
			free(ptr);
			return true;
		}
		tmp = ptr;
		ptr = ptr->next;
	}
	ERROR("")
	printf("breakpoint with id: %ld not found\n", id);
	return true;
	
}
/**
 * @brief Step over a function call instruction.
 *
 * Inserts a temporary breakpoint after the call instruction and continues execution.
 *
 * @param ctx Pointer to debugger context.
 * @param args Unused.
 * @return true on success, false otherwise.
 */
bool step_over(context *ctx,void *args){
	ctx->do_wait = false;
	ud_t ud_obj;
	uint64_t addr;
	uint64_t orig;
	uint8_t data[160];
	for(int i = 0; i < 20; i++) {
		long inst = ptrace(PTRACE_PEEKTEXT, ctx->pid, (void*)(ctx->regs.rip + i*8), NULL);
		memcpy((void*)&data[i*8], (void*)&inst , sizeof(inst));
	}
	ud_init(&ud_obj);
	ud_set_input_buffer(&ud_obj, data, sizeof(data));
	ud_set_mode(&ud_obj, ctx->arch);
	ud_set_pc(&ud_obj, ctx->regs.rip);
	ud_disassemble(&ud_obj);
	if(ud_disassemble(&ud_obj)){
		addr = ud_insn_off(&ud_obj);
		orig = ptrace(PTRACE_PEEKTEXT, ctx->pid, (void*)addr, NULL);
		long trap = (orig & ~0xff) | 0xCC;
		if (ptrace(PTRACE_POKETEXT, ctx->pid, (void*)addr, (void*)trap) == -1) {
			perror("PTRACE_POKETEXT");
			return true;
		}

	}
	ptrace(PTRACE_CONT, ctx->pid, NULL, NULL);
	waitpid(ctx->pid, 0, 0);
	if (ptrace(PTRACE_POKETEXT, ctx->pid, (void*)addr, (void*)orig) == -1) {
		perror("PTRACE_POKETEXT");
		return true;
	}
	ptrace(PTRACE_GETREGS, ctx->pid, 0, &ctx->regs);
	ctx->regs.rip = addr;
	ptrace(PTRACE_SETREGS, ctx->pid, 0, &ctx->regs);
	dis_ctx(ctx);
	return true;
}

/**
 * @brief Find the address of a symbol in the loaded program.
 *
 * @param ctx Pointer to debugger context.
 * @param name Symbol name to search for.
 * @return Symbol address if found, 0 otherwise.
 */

uint64_t find_sym(context *ctx,char *name){
	sym_list *sym = ctx->sym;
	while (sym != NULL) {
		if (strcmp(sym->name,name) == 0 ){ 
			return sym->addr;
			break;
		}
		sym = sym->next;
	}
	return 0;
}


/**
 * @brief Set a breakpoint at a given address or symbol name.
 *
 * @param ctx Pointer to debugger context.
 * @param args Address or symbol name.
 * @return true if the breakpoint was successfully set, false otherwise.
 */
bool setBP(context *ctx, void* args){
	char *arg = (char*)args;
	ctx->do_wait = false;

	if(!arg || *arg == '\0')
		return false;
	while(*arg != '\0' && isspace((unsigned char)*arg))arg++;

	char *token = strtok(arg, " ");
	if(token == NULL){
		return false;
	}
	uint64_t addr ;
	bool is_str = isalpha(token[0]);
	if(is_str){
		addr = find_sym(ctx, token);
		if(addr == 0){
			ERROR("");
			printf("undefined symbol %s \n",token);
			return true;
		}
	}else {
		addr = strtol(token,NULL,16);
	}
	bp *bpoint = malloc(sizeof(bp));
	bpoint->id = ++ctx->list->counter;
	bpoint->addr = addr;
	bpoint->next = NULL;
	bpoint->orig = ptrace(PTRACE_PEEKTEXT, ctx->pid, (void*)addr, NULL);
	long trap = (bpoint->orig & ~0xff) | 0xCC;
	if (ptrace(PTRACE_POKETEXT, ctx->pid, (void*)addr, (void*)trap) == -1) {
		printf("wrong address\n");
		free(bpoint);
		ctx->list->counter--;
		return true;
	}
	if (ctx->list->first == NULL){
		ctx->list->first = bpoint;
		ctx->list->last = bpoint;
	}else {
		ctx->list->last->next = bpoint;
		ctx->list->last = bpoint;
	}

	return true;

}

/**
 * @brief Handle a breakpoint hit by restoring the instruction and adjusting RIP.
 *
 * @param ctx Pointer to debugger context.
 */
void handle_bpoint(context *ctx) {
	ptrace(PTRACE_GETREGS, ctx->pid, 0, &ctx->regs);
	bp *b = NULL;
	bp *p = ctx->list->first;
	while (p != NULL) {
		if (p->addr == ctx->regs.rip - 1){ 
			b = p;
			break;
		}
		p = p->next;
	}
	if (b == NULL || strcmp(ctx->cmd.op,"si") == 0) {
		return;
	}
	ptrace(PTRACE_POKETEXT, ctx->pid, (void*)b->addr, (void*)b->orig);

	ctx->regs.rip = b->addr;
	ptrace(PTRACE_SETREGS, ctx->pid, 0, &ctx->regs);

}

/**
 * @brief List all active breakpoints.
 *
 * @param ctx Pointer to debugger context.
 * @param args Unused.
 * @return true always.
 */

bool listBP(context *ctx,void *args){
	ptrace(PTRACE_GETREGS, ctx->pid, NULL, &ctx->regs);
	ctx->do_wait = false;
	if(ctx->list->first == NULL){
		INFO("")
		printf("there is no break points\n");
		return true;
	}

	bp *head = ctx->list->first;
	bp *ptr = head;
	while (ptr != NULL) {
		INFO("")
		printf("break point id: %d at : 0x%lx\n",ptr->id,ptr->addr);
		ptr = ptr->next;
	}
	return true;
}
/**
 * @brief Restore all breakpoints to their original values or reapply traps.
 *
 * @param ctx Pointer to debugger context.
 * @param opt If 1, restore original instruction. If 0, reset breakpoint trap.
 */
void restore_all_BP(context *ctx,int opt){
	ptrace(PTRACE_GETREGS, ctx->pid, NULL, &ctx->regs);
	if(ctx->list->first != NULL){
		bp *head = ctx->list->first;
		bp *ptr = head;
		while (ptr != NULL) {
			if (opt == 1){
				if(ptrace(PTRACE_POKETEXT, ctx->pid, (void*)ptr->addr, ptr->orig) == -1){
					ERROR("error restore org value\n");
				}
			}else if(opt == 0){
				if(ctx->regs.rip == ptr->addr){
					ptr = ptr->next;
					continue;
				}
				long trap = (ptr->orig & ~0xff) | 0xCC;
				if(ptrace(PTRACE_POKETEXT, ctx->pid, (void*)ptr->addr, (void*)trap) == -1){
					ERROR("error reset bp\n");
				}
			}
			ptr = ptr->next;
		}
	}
}

/**
 * @brief Display process registers, flags, disassembly, and stack contents.
 *
 * @param ctx Pointer to debugger context.
 */
void dis_ctx(context *ctx){
        linenoiseClearScreen();
	ptrace(PTRACE_GETREGS, ctx->pid, NULL, &ctx->regs);
	ud_t ud_obj;
	uint8_t data[160];
	printf(COLOR_YELLOW "------------- regs ----------------\n" COLOR_RESET);
	int len = sizeof(regs_64)/sizeof(pos_name);
	pos_name *regs = (ctx->arch == 64) ? regs_64 : regs_32;

	for (int i = 0 ; i < len; i++) {
		printf(COLOR_CYAN "\t%s" COLOR_RESET "=> " COLOR_BLUE "0x%lx\n"COLOR_RESET, regs[i].name ,*(uint64_t*)((char*)&ctx->regs+regs[i].pos) );
	}
	len =sizeof(flags)/sizeof(pos_name) ;
	printf("FLAGS: ");
	for (int i = 0 ; i<len ; i++) {
		int is_set = (ctx->regs.eflags >> flags[i].pos) & 1;
		if(is_set){
			printf(COLOR_GREEN  "%s " COLOR_RESET, flags[i].name);
		}else {
			printf(COLOR_RED    "%s " COLOR_RESET, flags[i].name);
		}
	}
	printf("\n");
	printf(COLOR_YELLOW "------------- disass ----------------\n" COLOR_RESET);
	for(int i = 0; i < 20; i++) {
		long inst = ptrace(PTRACE_PEEKTEXT, ctx->pid, (void*)(ctx->regs.rip + i*8), NULL);
		memcpy((void*)&data[i*8], (void*)&inst , sizeof(inst));
	}
	ud_init(&ud_obj);
	ud_set_input_buffer(&ud_obj, data, sizeof(data));
	ud_set_mode(&ud_obj, ctx->arch);
	ud_set_syntax(&ud_obj, UD_SYN_INTEL);
	ud_set_pc(&ud_obj, ctx->regs.rip);
        while(ud_disassemble(&ud_obj)){
            // if(ud_insn_off(&ud_obj) == ctx->regs.rip){
            // 	printf(COLOR_GREEN "    --> 0x%llx: %s\n"COLOR_RESET,(uint64_t)ud_insn_off(&ud_obj),ud_insn_asm(&ud_obj));
            // }else {
            // 	printf(COLOR_MAGENTA "\t0x%llx: %s\n"COLOR_RESET,(uint64_t)ud_insn_off(&ud_obj),ud_insn_asm(&ud_obj));
            // }
            if(ud_insn_off(&ud_obj) == ctx->regs.rip){
                printf(COLOR_GREEN "    --> 0x%lx: "COLOR_RESET,(uint64_t)ud_insn_off(&ud_obj));
                print_highlight_asm(ud_insn_asm(&ud_obj));
            }else {
                printf(COLOR_MAGENTA "\t0x%lx: " COLOR_RESET,(uint64_t)ud_insn_off(&ud_obj));
                print_highlight_asm(ud_insn_asm(&ud_obj));
            }

            if(ud_insn_mnemonic(&ud_obj)  == UD_Iret || ud_insn_off(&ud_obj) >= (ctx->regs.rip + 0x20))
                break;
        }

	printf(COLOR_YELLOW "------------- stack ----------------\n" COLOR_RESET);
	char *fmt = (ctx->arch == 64) ? " 0x%llx" : " 0x%x";
	int data_len = ctx->arch / 8;
	for(int i = 0; i < 10; i++) {
		long v = ptrace(PTRACE_PEEKTEXT, ctx->pid, (void*)(ctx->regs.rsp + i*data_len), NULL);
		printf(COLOR_YELLOW "\t0x%llx" COLOR_RESET " =>" COLOR_BLUE , (ctx->regs.rsp + i *data_len));
		printf(fmt,v);
		printf("\n" COLOR_RESET);
	}

}



/**
 * @brief Load symbols from a Mach-O binary into the debugger context.
 */
static void init_macho_symbols(bparser *target, context *ctx)
{
	unsigned char *block = (unsigned char *)target->block;
	uint32_t magic = *(uint32_t *)block;
	bool is_64 = (magic == 0xfeedfacf);
	size_t hdr_size = is_64 ? sizeof(Elf64_Ehdr) : sizeof(Elf32_Ehdr); /* placeholder for header skip */

	/* Parse Mach-O header to get ncmds and sizeofcmds */
	uint32_t ncmds, sizeofcmds;
	unsigned char *cmd_ptr;
	if (is_64) {
		hdr_size = 32; /* sizeof(mach_header_64) */
		ncmds = *(uint32_t *)(block + 16);
		sizeofcmds = *(uint32_t *)(block + 20);
		ctx->arch = 64;
	} else {
		hdr_size = 28; /* sizeof(mach_header) */
		ncmds = *(uint32_t *)(block + 12);
		sizeofcmds = *(uint32_t *)(block + 16);
		ctx->arch = 32;
	}

	if (hdr_size + sizeofcmds > target->size) return;
	cmd_ptr = block + hdr_size;
	unsigned char *end_ptr = block + target->size;

	/* Find LC_SYMTAB (cmd=0x2) and LC_MAIN (cmd=0x80000028) for entry point */
	uint32_t symoff = 0, nsyms = 0, stroff = 0, strsize = 0;
	uint64_t entry_off = 0;

	for (uint32_t i = 0; i < ncmds; i++) {
		if (cmd_ptr + 8 > end_ptr) break;
		uint32_t cmd = *(uint32_t *)cmd_ptr;
		uint32_t cmdsize = *(uint32_t *)(cmd_ptr + 4);
		if (cmdsize < 8 || cmd_ptr + cmdsize > end_ptr) break;

		if (cmd == 0x2) { /* LC_SYMTAB */
			symoff  = *(uint32_t *)(cmd_ptr + 8);
			nsyms   = *(uint32_t *)(cmd_ptr + 12);
			stroff  = *(uint32_t *)(cmd_ptr + 16);
			strsize = *(uint32_t *)(cmd_ptr + 20);
		} else if (cmd == (0x28 | 0x80000000)) { /* LC_MAIN */
			entry_off = *(uint64_t *)(cmd_ptr + 8);
		}
		cmd_ptr += cmdsize;
	}

	/* Set entry to base + entry offset (Mach-O is always PIE on modern systems) */
	ctx->entry = ctx->base + entry_off;
	ctx->pie = false;

	/* Load symbols from nlist */
	if (nsyms == 0 || symoff == 0 || stroff == 0) return;

	if (is_64) {
		size_t nlist_size = 16; /* sizeof(nlist_64) */
		if (symoff + nsyms * nlist_size > target->size) return;
		if (stroff + strsize > target->size) return;

		unsigned char *nl = block + symoff;
		char *strs = (char *)(block + stroff);

		for (uint32_t j = 0; j < nsyms; j++) {
			uint32_t n_strx = *(uint32_t *)(nl + j * nlist_size);
			uint8_t  n_type = *(uint8_t  *)(nl + j * nlist_size + 4);
			uint64_t n_value = *(uint64_t *)(nl + j * nlist_size + 8);

			/* Only defined symbols (N_SECT=0xe, not stab) */
			if ((n_type & 0xe0) != 0 || (n_type & 0x0e) != 0x0e || n_value == 0)
				continue;

			if (n_strx >= strsize) continue;
			char *name = strs + n_strx;
			if (*name == '\0') continue;

			sym_list *sym = malloc(sizeof(sym_list));
			if (!sym) break;
			sym->name = strdup(name);
			sym->addr = ctx->base + n_value; /* relocated */
			sym->next = NULL;
			if (ctx->sym == NULL) { ctx->sym = sym; }
			else { sym_list *l = ctx->sym; while (l->next) l = l->next; l->next = sym; }
		}
	} else {
		size_t nlist_size = 12; /* sizeof(nlist) */
		if (symoff + nsyms * nlist_size > target->size) return;
		if (stroff + strsize > target->size) return;

		unsigned char *nl = block + symoff;
		char *strs = (char *)(block + stroff);

		for (uint32_t j = 0; j < nsyms; j++) {
			uint32_t n_strx = *(uint32_t *)(nl + j * nlist_size);
			uint8_t  n_type = *(uint8_t  *)(nl + j * nlist_size + 4);
			uint32_t n_value = *(uint32_t *)(nl + j * nlist_size + 8);

			if ((n_type & 0xe0) != 0 || (n_type & 0x0e) != 0x0e || n_value == 0)
				continue;

			if (n_strx >= strsize) continue;
			char *name = strs + n_strx;
			if (*name == '\0') continue;

			sym_list *sym = malloc(sizeof(sym_list));
			if (!sym) break;
			sym->name = strdup(name);
			sym->addr = ctx->base + n_value;
			sym->next = NULL;
			if (ctx->sym == NULL) { ctx->sym = sym; }
			else { sym_list *l = ctx->sym; while (l->next) l = l->next; l->next = sym; }
		}
	}
}

/**
 * @brief Initialize the debugger context from a parsed binary.
 *
 * Reads ELF or Mach-O headers and symbol tables to populate context values.
 *
 * @param target Pointer to binary parser structure with file data.
 * @param ctx Pointer to debugger context to initialize.
 */
void init_values(bparser *target, context *ctx){
	bp_list *list = malloc(sizeof(bp_list));
	char mmaps[512]= {0};
	ctx->do_wait = false;
	ctx->do_exit = false;
	ctx->sym = NULL;
	ctx->mmaps = NULL;
	memset(list, 0, sizeof(bp_list));
	list->counter = 0;
	ctx->list = list;

	snprintf(mmaps, sizeof(mmaps), "/proc/%d/maps", ctx->pid);
	FILE *file = fopen(mmaps,"r");
	if (!file) {
		fprintf(stderr, "failed to open process maps\n");
		return;
	}
	size_t size = 0;
	getdelim(&ctx->mmaps,&size,'\0' , file);
	fclose(file);
	sscanf(ctx->mmaps, "%lx", &ctx->base);

	/* Detect file format by magic bytes */
	if (target->size >= 4) {
		uint32_t magic = *(uint32_t *)target->block;
		if (magic == 0xfeedface || magic == 0xfeedfacf) {
			/* Mach-O binary */
			init_macho_symbols(target, ctx);
			return;
		}
	}

	/* ELF binary */
	Elf64_Ehdr *ehdr = (Elf64_Ehdr*)target->block;
	if(ehdr->e_type == ET_EXEC){
		ctx->entry = ehdr->e_entry;
		ctx->pie = true ;
	}else {
		ctx->entry = ctx->base + ehdr->e_entry;
	}	
	if(ehdr->e_ident[EI_CLASS] == ELFCLASS32){
		ctx->arch = 32;
		ctx->entry = ctx->entry & 0xffffffff;

		Elf32_Ehdr *ehdr = (Elf32_Ehdr*)target->block;
		Elf32_Shdr *shdr = target->block + ehdr->e_shoff;
		if (ehdr->e_shnum != 0) {
			for (int i = 0; i < ehdr->e_shnum; i++) {
				if (shdr[i].sh_type == SHT_SYMTAB) {
					Elf32_Sym *syms = target->block + shdr[i].sh_offset;
					Elf32_Shdr *strtab_hdr = &shdr[shdr[i].sh_link];
					char *strtab = (char*)target->block + strtab_hdr->sh_offset;
					int num = shdr[i].sh_size / shdr[i].sh_entsize;
					for (int j = 0; j < num; j++) {
						if (ELF32_ST_TYPE(syms[j].st_info) == STT_FUNC && syms[j].st_shndx != SHN_UNDEF) {
							char *name = strtab + syms[j].st_name;
							if (*name) {
								sym_list *sym = malloc(sizeof(sym_list));
								sym->name = strdup(name);
								sym->addr = (ctx->pie) ? syms[j].st_value : ctx->base + syms[j].st_value;
								sym->next = NULL;

								if (ctx->sym == NULL) {
									ctx->sym = sym;
								} else {
									sym_list *last = ctx->sym;
									while (last->next != NULL) {
										last = last->next;
									}
									last->next = sym;
								}
							}
						}
					}
				}
			}
		}
	}else{
		ctx->arch = 64;
		Elf64_Shdr *shdr = target->block + ehdr->e_shoff;
		if(ehdr->e_shnum != 0){
			for (int i = 0; i< ehdr->e_shnum; i++) {
				if(shdr[i].sh_type == SHT_SYMTAB){
					Elf64_Sym *syms = target->block + shdr[i].sh_offset;
					Elf64_Shdr *strtab_hdr = &shdr[shdr[i].sh_link];
					char *strtab = (char*)target->block + strtab_hdr->sh_offset;
					int num = shdr[i].sh_size / shdr[i].sh_entsize;
					for (int j = 0; j< num; j++) {
						if(ELF64_ST_TYPE(syms[j].st_info) == STT_FUNC && syms[j].st_shndx != SHN_UNDEF){
							char *name = strtab + syms[j].st_name;
							if(*name){
								sym_list *sym = malloc(sizeof(sym_list));
								sym->name = strdup(name);
								sym->addr = (ctx->pie) ? syms[j].st_value : ctx->base + syms[j].st_value;
								sym->next = NULL;
								if(ctx->sym == NULL){
									ctx->sym = sym;
								}else {
									sym_list *last = ctx->sym;
									while ( last->next != NULL) {
										last = last->next;
									}
									last->next = sym;
								}
							}
						}
					}
				}
			}
		}
	}
}

/**
 * @brief Free all breakpoint and symbol structures in the context.
 *
 * @param ctx Pointer to debugger context.
 */
void destroy_bp_sym(context *ctx){
	bp *ptr = ctx->list->first;
	bp *tmp = NULL;
	while (ptr != NULL) {
		tmp = ptr->next;
		free(ptr);
		ptr = tmp;
	}


	sym_list *ptr1 = ctx->sym;
	sym_list *temp = NULL;
	while (ptr1 != NULL) {
		temp = ptr1->next;
		free(ptr1->name);
		free(ptr1);
		ptr1 = temp;
	}
}

/**
 * @brief Free all allocated resources in the debugger context.
 *
 * @param ctx Pointer to debugger context.
 */
void destroy_all(context *ctx){
	if(ctx == NULL)
		return;
	destroy_bp_sym(ctx);
	free(ctx->list);
	ctx->list = NULL;
	free(ctx);
	ctx = NULL;
}

/**
 * @brief Run the debugger on a given binary.
 *
 * Forks a child process, loads the binary via memfd_create, and sets an initial breakpoint.
 * Then enters the main command loop for user interaction.
 *
 * @param target Pointer to binary parser structure with ELF data.
 * @param arg Arguments structure containing argc and argv.
 * @return true on success, false otherwise.
 */
bool b_debugger(bparser *target, void *arg){

	setbuf(stdout, NULL);
	int argc = *((inputs*)arg) -> argc;
	char** args = ((inputs*)arg) -> args;
	context *ctx  = malloc(sizeof(context));
	if (!ctx) {
		fprintf(stderr, "failed to allocate debugger context\n");
		return false;
	}
	memset(ctx, 0, sizeof(context));
	int stats = 0;
	int pid = fork();
	if (pid < 0 ){
		fprintf(stderr, "failed to fork\n");
		return false;
	}else if (pid == 0) {
		int fd = memfd_create(args[1], MFD_CLOEXEC);
		if (fd == -1) {
			perror("memfd_create failed");
			return -1;
		}

		ssize_t written = write(fd, target->block,  target->size);
		if (written == -1 || (size_t)written != target->size) {
			perror("write failed");
			close(fd);
			return -1;
		}
		if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
			perror("PTRACE_TRACEME failed");
			return -1;
		}
		
		char *argv[MAX_INPUT_ARGS];
		argv[0] = args[1];
		if (((inputs*)arg)->input_argc > 0 ){
			int j = 1;
			for (int i = 0; i < ((inputs*)arg)->input_argc; i++, j++){
				argv[j] = ((inputs*)arg)->input_args[i];
			}
			argv[j] = NULL;
		} else {
			argv[1] = NULL;
		}

		
		char *envp[] = {NULL};

		if (fexecve(fd, argv, envp) == -1) {
			perror("fexecve failed");
			close(fd);
		}
	}else {
		waitpid(pid, &stats, 0);

		ctx->pid = pid;
		init_values(target, ctx);
		unsigned long orig = ptrace(PTRACE_PEEKTEXT, ctx->pid, (void*)ctx->entry, NULL);
		long trap = (orig & ~0xff) | 0xCC;
		if (ptrace(PTRACE_POKETEXT, ctx->pid, (void*)ctx->entry, (void*)trap) == -1) {
			perror("PTRACE_POKETEXT");
			return false;
		}
		ptrace(PTRACE_CONT, ctx->pid, NULL, NULL);
		waitpid(ctx->pid, &stats, 0);
		if (WIFSTOPPED(stats) && WSTOPSIG(stats) == SIGTRAP) {

			if(ptrace(PTRACE_GETREGS, ctx->pid, NULL, &ctx->regs) == -1){
				ERROR("failed to read regs\n");
			}
			if(ptrace(PTRACE_POKETEXT, ctx->pid, (void*)ctx->entry, (void*)orig)== -1){
				ERROR("failed to restore entry point\n");
			}
			ctx->regs.rip = ctx->entry;
			ptrace(PTRACE_SETREGS, ctx->pid, NULL, &ctx->regs);
		}

		dis_ctx(ctx);
		while (1) {
			parse_cmd(ctx);
			if(ctx->do_wait){
				waitpid(ctx->pid, &stats, 0);
				if (WIFSTOPPED(stats) && WSTOPSIG(stats) == SIGTRAP) {
					handle_bpoint(ctx);
					restore_all_BP(ctx, 1);
					dis_ctx(ctx);
				} else if (WIFEXITED(stats) )   {
					// using base to determen if the process exit 
					INFO("the process has EXITED\n")
					ctx->base = 0;
				}

			}
			free(ctx->cmd.op);
			ctx->cmd.op = 0;
			ctx->cmd.addr = 0;

			if((ctx->do_exit)){
				destroy_all(ctx);
				break;
			}
		}
	}

	return true;
}
