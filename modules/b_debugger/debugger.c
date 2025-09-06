#define _GNU_SOURCE
#include <elf.h>
#include <stdio.h>
#include <udis86.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "debugger.h"
#include <sys/ptrace.h>
void parseCmd(context *ctx){
	char cmd[1024] = {0} ;
	char *tokens[3];
	unsigned int counter = 0;
	int status;
	while (counter  == 0) {
		printf("baseer> ");
		read(0, cmd, 1020);
		cmd[strcspn(cmd, "\n")] = 0;
		//printf("%s",cmd);
		// tokinze the string
		char *token = strtok(cmd, " ");
		while(token != NULL && counter < 3) {
			tokens[counter++] = token;
			token = strtok(NULL, " ");
		}
		// setup the command 
		ctx->cmd.op = strdup(tokens[0]); 
		for (int i = 0; i< CMD_COUNT; i++) {
			if(strcmp(ctx->cmd.op,cmds[i]) == 0){
				switch (i) {
					case CMD_bp:
						if(counter != 2){
							printf("bp 0x12354\n");
							break;
						}
						uint64_t addr = strtol(tokens[1],NULL,16);
						if(addr == 0)printf("wrong address");
						setBP(ctx, addr);
						dis_ctx(ctx);
						break;
					
					case CMD_dp:
						if(counter != 2){
							printf("dp {break point id}\n");
							break;
						}
						uint32_t id = strtol(tokens[1],NULL,10);
						if(addr == 0)printf("wrong id\n");
						delBP(ctx,id);
						break;
					case CMD_lp:
						listBP(ctx);
						break;
					case CMD_si:
						if (ptrace(PTRACE_SINGLESTEP, ctx->pid, 0, 0) == -1) {
							perror("SINGLESTEP");
							break;
						}
						waitpid(ctx->pid, &status, 0);
						if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
							bp *b = findBP(ctx, ctx->regs.rip);
							if (b) {
								handle_bpoint(ctx);
							} else {
								dis_ctx(ctx);
							}
						} else if (WIFEXITED(status)) {
							printf(">>> Child exited\n");
							exit(0);
						}
						break;
					case CMD_C:
						if (ptrace(PTRACE_CONT, ctx->pid, 0, 0) == -1) {
							printf("error doing CONT\n");
							break;
						}
						waitpid(ctx->pid, &status, 0);
						if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
							handle_bpoint(ctx);
						} else if (WIFEXITED(status) || (WIFSTOPPED(status) && WSTOPSIG(status) != SIGTRAP)  ) {
							printf(">>> Child exited\n");
							exit(0);
						}
						break;
					default:
						printf("unkonw command\n");

				}
				break;

			}
		}
		free(ctx->cmd.op);
		ctx->cmd.op = 0;

	}


}
bp* findBP(context *ctx, uint64_t rip) {
	bp *p = ctx->list->first;
	while (p != NULL) {
		if (p->addr == rip - 1) return p;
		p = p->next;
	}
	return NULL;
}
void delBP(context *ctx, uint32_t id){
	if(ctx->list->first == NULL){
		printf("no breakpoints found\n");
		return;
	}
	bp *ptr = ctx->list->first;
	bp *tmp = NULL;
	while (ptr != NULL) {
		if(ptr->id == id){

			if (ptrace(PTRACE_POKETEXT, ctx->pid, (void*)ptr->addr, (void*)ptr->orig) == -1) {
				perror("PTRACE_POKETEXT");
				return ;
			}
			if(tmp != NULL){
				tmp->next = ptr->next;
			}else {
				ctx->list->first = ptr->next;
			}

			if(ptr == ctx->list->last){
				ctx->list->last = tmp;
			}
			printf("deleted breakpoint at: %llx\n",ptr->addr);
			free(ptr);
			return;
		}
		tmp = ptr;
		ptr = ptr->next;
	}
	printf("breakpoint with id: %d not found\n",id);
	
}
void setBP(context *ctx, uint64_t addr){
	bp *bpoint = malloc(sizeof(bp));
	bpoint->id = ++ctx->list->counter;
	bpoint->addr = addr;
	bpoint->next = NULL;
	bpoint->orig = ptrace(PTRACE_PEEKTEXT, ctx->pid, (void*)addr, NULL);
	long trap = (bpoint->orig & ~0xff) | 0xCC;
	if (ptrace(PTRACE_POKETEXT, ctx->pid, (void*)addr, (void*)trap) == -1) {
		perror("PTRACE_POKETEXT");
		return ;
	}
	if (ctx->list->first == NULL){
		ctx->list->first = bpoint;
		ctx->list->last = bpoint;
	}else {
		// link to the last
		ctx->list->last->next = bpoint;
		ctx->list->last = bpoint;
	}

}
void handle_bpoint(context *ctx) {
	ptrace(PTRACE_GETREGS, ctx->pid, 0, &ctx->regs);

	bp *b = findBP(ctx, ctx->regs.rip);
	if (!b) {
		printf(">>> Hit SIGTRAP but no breakpoint at RIP=0x%llx\n",(unsigned long long)ctx->regs.rip);
		return;
	}

	// restore original instruction
	ptrace(PTRACE_POKETEXT, ctx->pid, (void*)b->addr, (void*)b->orig);

	// adjust RIP back
	ctx->regs.rip = b->addr;
	ptrace(PTRACE_SETREGS, ctx->pid, 0, &ctx->regs);
	dis_ctx(ctx);

	// single step over
	if (ptrace(PTRACE_SINGLESTEP, ctx->pid, 0, 0) == -1) {
		perror("SINGLESTEP");
		return;
	}
	waitpid(ctx->pid, 0, 0);

	// reinsert trap
	long trap = (b->orig & ~0xff) | 0xCC;
	if(ptrace(PTRACE_POKETEXT, ctx->pid, (void*)b->addr, (void*)trap) == -1){
		perror("PTRACE_POKETEXT");
		return ;
	}
}

void listBP(context *ctx){
	ptrace(PTRACE_GETREGS, ctx->pid, NULL, &ctx->regs);
	if(ctx->list->first == NULL){
		printf("there is no break points\n");
		return;
	}

	bp *head = ctx->list->first;
	bp *ptr = head;
	while (ptr != NULL) {
		printf("[*] break point id: %d at : 0x%llx\n",ptr->id,ptr->addr);
		ptr = ptr->next;
	}
}
void restore_all_BP(context *ctx,int opt){
	ptrace(PTRACE_GETREGS, ctx->pid, NULL, &ctx->regs);
	if(ctx->list->first != NULL){
		bp *head = ctx->list->first;
		bp *ptr = head;
		while (ptr != NULL) {
			if (opt == 1){
				if (ptrace(PTRACE_POKETEXT, ctx->pid, (void*)ptr->addr, (void*)ptr->orig) == -1) {
					perror("PTRACE_POKETEXT");
					return ;
				}
			}else{
				long trap = (ptr->orig & ~0xff) | 0xCC;
				if (ptrace(PTRACE_POKETEXT, ctx->pid, (void*)ptr->addr, (void*)trap) == -1) {
					perror("PTRACE_POKETEXT");
					return ;
				}
			}
			ptr = ptr->next;
		}
	}
}
void dis_ctx(context *ctx){

	ud_t ud_obj;
	uint8_t data[160];
	restore_all_BP(ctx,1);
	printf("inside dis_ctx\n");
	ptrace(PTRACE_GETREGS, ctx->pid, NULL, &ctx->regs);
	printf("------------- regs ----------------\n");
	printf("RAX = 0x%llx\n", ctx->regs.rax);
	printf("RDX = 0x%llx\n", ctx->regs.rdx);
	printf("RCX = 0x%llx\n", ctx->regs.rcx);
	printf("RBX = 0x%llx\n", ctx->regs.rbx);
	printf("RDI = 0x%llx\n", ctx->regs.rdi);
	printf("RSI = 0x%llx\n", ctx->regs.rsi);
	printf("R8 = 0x%llx\n", ctx->regs.r8);
	printf("R9 = 0x%llx\n", ctx->regs.r9);
	printf("R10 = 0x%llx\n", ctx->regs.r10);
	printf("R11 = 0x%llx\n", ctx->regs.r11);
	printf("R12 = 0x%llx\n", ctx->regs.r12);
	printf("R13 = 0x%llx\n", ctx->regs.r13);
	printf("R14 = 0x%llx\n", ctx->regs.r14);
	printf("R15 = 0x%llx\n", ctx->regs.r15);
	printf("RSP = 0x%llx\n", ctx->regs.rsp);
	printf("RBP = 0x%llx\n", ctx->regs.rbp);
	printf("RIP = 0x%llx\n", ctx->regs.rip);

	printf("------------- disass ----------------\n");
	for(int i = 0; i < 20; i++) {
		long inst = ptrace(PTRACE_PEEKDATA, ctx->pid, (void*)(ctx->regs.rip + i*8), NULL);
		memcpy((void*)&data[i*8], (void*)&inst , sizeof(inst));
	}
	ud_init(&ud_obj);
	ud_set_input_buffer(&ud_obj, data, sizeof(data));
	ud_set_mode(&ud_obj, 64); // x86_64 mode
	ud_set_syntax(&ud_obj, UD_SYN_INTEL);
	ud_set_pc(&ud_obj, ctx->regs.rip);
	while(ud_disassemble(&ud_obj)){
		printf("0x%llx: %s\n",(unsigned long long)ud_insn_off(&ud_obj),ud_insn_asm(&ud_obj));
		if(ud_insn_mnemonic(&ud_obj)  == UD_Iret)
			break;
	}

	printf("------------- stack ----------------\n");
	for(int i = 0; i < 10; i++) {
		long v = ptrace(PTRACE_PEEKDATA, ctx->pid, (void*)(ctx->regs.rsp + i*8), NULL);
		printf("Stack[0x%llx] = 0x%lx\n", (ctx->regs.rsp + i *8), v);
	}

	restore_all_BP(ctx,0);
}
void init_values(baseer_target_t *target, context *ctx){
	bp_list *list = malloc(sizeof(bp_list));
	char mmaps[512]= {0};
	char line[512]= {0};
	uint64_t start = 0;
	memset(list, 0, sizeof(bp_list));
	list->counter = 0;
	ctx->list = list;
	Elf64_Ehdr *ehdr = target->block;
	if(ehdr->e_type == ET_EXEC){
		ctx->entry = ehdr->e_entry;
	}else {
		sprintf(mmaps, "/proc/%d/maps", ctx->pid);
		FILE *file = fopen(mmaps,"r");
		fgets(line,sizeof(line),file);
		sscanf(line, "%lx", &start);
		ctx->entry = start + ehdr->e_entry;
	
	}

}

bool b_debugger(baseer_target_t *target,void *arg){
	setbuf(stdout, NULL);
	context *ctx  = malloc(sizeof(context));
	memset(ctx, 0, sizeof(bp_list));
	int stats = 0;
	int pid = fork();
	if (pid < 0 ){
		printf("faild to fork");
		return false;
	}else if (pid == 0) {
		int fd = memfd_create("in_memory_elf", MFD_CLOEXEC);
		if (fd == -1) {
			perror("memfd_create failed");
			return -1;
		}

		ssize_t written = write(fd, target->block, target->size);
		if (written == -1 || (size_t)written != target->size) {
			perror("write failed");
			close(fd);
			return -1;
		}

		if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
			perror("PTRACE_TRACEME failed");
			close(fd);
			return -1;
		}

		char *argv[] = {"in_memory_elf", NULL};
		char *envp[] = {NULL};

		if (fexecve(fd, argv, envp) == -1) {
			perror("fexecve failed");
			close(fd);
			exit(EXIT_FAILURE); // child should exit
		}
	}else {
		waitpid(pid, &stats, 0);

		ctx->pid = pid;
		init_values(target, ctx);
		uint64_t entry = ctx->entry;

		// setBreakPoint at entry point
		uint64_t orig = ptrace(PTRACE_PEEKTEXT, ctx->pid, (void*)entry, NULL);
		long trap = (orig & ~0xff) | 0xCC;
		if (ptrace(PTRACE_POKETEXT, ctx->pid, (void*)entry, (void*)trap) == -1) {
			perror("PTRACE_POKETEXT");
			return false;
		}
		ptrace(PTRACE_CONT, ctx->pid, NULL, NULL);
		waitpid(ctx->pid, &stats, 0);
		if (WIFSTOPPED(stats) && WSTOPSIG(stats) == SIGTRAP) {

			if(ptrace(PTRACE_GETREGS, ctx->pid, NULL, &ctx->regs) == -1){
				printf("failed to read regs\n");
			}
			// restore the orignal instruction
			if(ptrace(PTRACE_POKETEXT, ctx->pid, (void*)entry, (void*)orig)== -1){
				printf("failed to restore entry point\n");
			}
			ctx->regs.rip = entry;
			ptrace(PTRACE_SETREGS, ctx->pid, NULL, &ctx->regs);
		}
		dis_ctx(ctx);

		while (1) {
			parseCmd(ctx);
		
		}
	}

	return true;
}
