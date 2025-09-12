#define _GNU_SOURCE
#include <elf.h>
#include <stdio.h>
#include "udis86.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "debugger.h"
#include <sys/ptrace.h>

/**
 * @file debugger.c
 * @brief Functions to handle debugging binary and command logic. 
 *
 */



void parse_cmd(context *ctx){
	if(ctx == NULL)
		return;
	char cmd[1024] = {0} ;
	char *tokens[3];
	unsigned int counter = 0;
	bool flag = false;
	int status;
	while (counter  == 0 ) {
		printf("baseer> ");
		read(0, cmd, 1020);
		cmd[strcspn(cmd, "\n")] = 0;
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
						flag = true;
						ctx->do_wait = false;
						break;
					
					case CMD_dp:
						if(counter != 2){
							printf("dp {break point id}\n");
							break;
						}
						uint32_t id = strtol(tokens[1],NULL,10);
						if(addr == 0)printf("wrong id\n");
						delBP(ctx,id);
						flag = true;
						ctx->do_wait = false;
						break;
					case CMD_lp:
						listBP(ctx);
						flag = true;
						ctx->do_wait = false;
						break;
					case CMD_si:
						if (ptrace(PTRACE_SINGLESTEP, ctx->pid, 0, 0) == -1) {
							perror("SINGLESTEP");
							break;
						}
						flag = true;
						ctx->do_wait = true;

						break;

					case CMD_so:
						restore_all_BP(ctx, 1);
						step_over(ctx);
						if (ptrace(PTRACE_CONT, ctx->pid, 0, 0) == -1) {
							perror("STEPOVER");
							break;
						}
						ctx->do_wait = true;
						flag = true;
						break;
					case CMD_C:
						restore_all_BP(ctx,0);

						if (ptrace(PTRACE_CONT, ctx->pid, 0, 0) == -1) {
							printf("error doing CONT\n");
							break;
						}
						flag = true;
						ctx->do_wait = true;
						break;
					case CMD_h:
						print_helpCMD();
						flag = true;
					case CMD_q:
						ptrace(PTRACE_CONT, ctx->pid, NULL, SIGKILL);
						destroy_all(ctx);
						flag = true;
						exit(0);

				}
				break;

			}
		}
		if(!flag){
			printf("unkonw command\n");
			print_helpCMD();
		}
		free(ctx->cmd.op);
		ctx->cmd.op = 0;

	}


}
void print_helpCMD(){
	printf("bp : set breakpoint {ex: bp 0x12354}\n");
	printf("dp : delete breakpoint {ex: dp breakpoint_id}\n");
	printf("lp : list all breakpoints {ex: lp}\n");
	printf("si : take one step execution (step into) {ex: si}\n");
	printf("so : take one step execution (step over){ex: so}\n");
	printf("c  : continue execution {ex: c}\n");
	printf("h  : display help commands {ex: h}\n");
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
void step_over(context *ctx){

	ud_t ud_obj;
	uint8_t data[160];
	for(int i = 0; i < 20; i++) {
		long inst = ptrace(PTRACE_PEEKDATA, ctx->pid, (void*)(ctx->regs.rip + i*8), NULL);
		memcpy((void*)&data[i*8], (void*)&inst , sizeof(inst));
	}
	ud_init(&ud_obj);
	ud_set_input_buffer(&ud_obj, data, sizeof(data));
	ud_set_mode(&ud_obj, ctx->arch);
	ud_set_syntax(&ud_obj, UD_SYN_INTEL);
	uint64_t eip = (ctx->regs.rip & 0xffffffff);
	ud_set_pc(&ud_obj, eip);
	ud_disassemble(&ud_obj);
	if(ud_disassemble(&ud_obj)){
		setBP(ctx,(uint64_t)ud_insn_off(&ud_obj));
		
	}
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
		ctx->list->last->next = bpoint;
		ctx->list->last = bpoint;
	}

}
void handle_bpoint(context *ctx) {
	ptrace(PTRACE_GETREGS, ctx->pid, 0, &ctx->regs);

	bp *b = findBP(ctx, ctx->regs.rip);
	if (!b) {
		return;
	}

	ptrace(PTRACE_POKETEXT, ctx->pid, (void*)b->addr, (void*)b->orig);

	ctx->regs.rip = b->addr;
	ptrace(PTRACE_SETREGS, ctx->pid, 0, &ctx->regs);
	dis_ctx(ctx);

	if (ptrace(PTRACE_SINGLESTEP, ctx->pid, 0, 0) == -1) {
		perror("SINGLESTEP");
		return;
	}
	waitpid(ctx->pid, 0, 0);

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
	ptrace(PTRACE_GETREGS, ctx->pid, NULL, &ctx->regs);
	printf("------------- regs ----------------\n");
	if(ctx->arch == 64){
	printf("\tRAX => 0x%llx\n", ctx->regs.rax);
	printf("\tRDX => 0x%llx\n", ctx->regs.rdx);
	printf("\tRCX => 0x%llx\n", ctx->regs.rcx);
	printf("\tRBX => 0x%llx\n", ctx->regs.rbx);
	printf("\tRDI => 0x%llx\n", ctx->regs.rdi);
	printf("\tRSI => 0x%llx\n", ctx->regs.rsi);
	printf("\tR8  => 0x%llx\n", ctx->regs.r8);
	printf("\tR9  => 0x%llx\n", ctx->regs.r9);
	printf("\tR10 => 0x%llx\n", ctx->regs.r10);
	printf("\tR11 => 0x%llx\n", ctx->regs.r11);
	printf("\tR12 => 0x%llx\n", ctx->regs.r12);
	printf("\tR13 => 0x%llx\n", ctx->regs.r13);
	printf("\tR14 => 0x%llx\n", ctx->regs.r14);
	printf("\tR15 => 0x%llx\n", ctx->regs.r15);
	printf("\tRSP => 0x%llx\n", ctx->regs.rsp);
	printf("\tRBP => 0x%llx\n", ctx->regs.rbp);
	printf("\tRIP => 0x%llx\n", ctx->regs.rip);
	}else {
	
	printf("\tEAX => 0x%llx\n", ctx->regs.rax);
	printf("\tEDX => 0x%llx\n", ctx->regs.rdx);
	printf("\tECX => 0x%llx\n", ctx->regs.rcx);
	printf("\tEBX => 0x%llx\n", ctx->regs.rbx);
	printf("\tEDI => 0x%llx\n", ctx->regs.rdi);
	printf("\tESI => 0x%llx\n", ctx->regs.rsi);
	printf("\tR8d  => 0x%llx\n", ctx->regs.r8);
	printf("\tR9d  => 0x%llx\n", ctx->regs.r9);
	printf("\tR10d => 0x%llx\n", ctx->regs.r10);
	printf("\tR11d => 0x%llx\n", ctx->regs.r11);
	printf("\tR12d => 0x%llx\n", ctx->regs.r12);
	printf("\tR13d => 0x%llx\n", ctx->regs.r13);
	printf("\tR14d => 0x%llx\n", ctx->regs.r14);
	printf("\tR15d => 0x%llx\n", ctx->regs.r15);
	printf("\tESP => 0x%llx\n", ctx->regs.rsp);
	printf("\tEBP => 0x%llx\n", ctx->regs.rbp);
	printf("\tEIP => 0x%llx\n", ctx->regs.rip);
	}

	printf("------------- disass ----------------\n");
	for(int i = 0; i < 20; i++) {
		long inst = ptrace(PTRACE_PEEKDATA, ctx->pid, (void*)(ctx->regs.rip + i*8), NULL);
		memcpy((void*)&data[i*8], (void*)&inst , sizeof(inst));
	}
	ud_init(&ud_obj);
	ud_set_input_buffer(&ud_obj, data, sizeof(data));
	ud_set_mode(&ud_obj, ctx->arch); // x86_64 mode
	ud_set_syntax(&ud_obj, UD_SYN_INTEL);
	ud_set_pc(&ud_obj, ctx->regs.rip);
	while(ud_disassemble(&ud_obj)){
		printf("\t0x%llx: %s\n",(uint64_t)ud_insn_off(&ud_obj),ud_insn_asm(&ud_obj));
		if(ud_insn_mnemonic(&ud_obj)  == UD_Iret )
			break;
	}

	printf("------------- stack ----------------\n");
	for(int i = 0; i < 10; i++) {
		if(ctx->arch == 64){
			long v = ptrace(PTRACE_PEEKDATA, ctx->pid, (void*)(ctx->regs.rsp + i*8), NULL);
			printf("\t0x%llx => 0x%lx\n", (ctx->regs.rsp + i *8), v);
		}else {
			long v = ptrace(PTRACE_PEEKDATA, ctx->pid, (void*)(ctx->regs.rsp + i*4), NULL);
			printf("\t0x%llx => 0x%04x\n", (ctx->regs.rsp + i *4), v);
		}
	}

}

void init_values(bparser *target, context *ctx){
	bp_list *list = malloc(sizeof(bp_list));
	char mmaps[512]= {0};
	char line[512]= {0};
	ctx->do_wait = false;
	memset(list, 0, sizeof(bp_list));
	list->counter = 0;
	ctx->list = list;
	Elf64_Ehdr *ehdr = (Elf64_Ehdr*)target->source.mem.data;
	sprintf(mmaps, "/proc/%d/maps", ctx->pid);
	FILE *file = fopen(mmaps,"r");
	size_t size = 0;
	ssize_t b_read = getdelim(&ctx->mmaps,&size,'\0' , file);
	if(ehdr->e_type == ET_EXEC){
		ctx->entry = ehdr->e_entry;
	}else {
		sscanf(ctx->mmaps, "%lx", &ctx->base);
		ctx->entry = ctx->base + ehdr->e_entry;
	}	
	if(ehdr->e_ident[EI_CLASS] == ELFCLASS32){
		ctx->arch = 32;
	}else{
		ctx->arch = 64;
	}
}
void destroy_bp(bp *bpoint){
	bp *ptr = bpoint;
	bp *tmp = NULL;
	while (ptr != NULL) {
		tmp = ptr->next;
		free(ptr);
		ptr = tmp;
	}
}
void destroy_all(context *ctx){
	destroy_bp(ctx->list->first);
	free(ctx->list);
	ctx->list = NULL;
	free(ctx);
	ctx = NULL;
}
bool b_debugger(bparser *target, void *arg){
	setbuf(stdout, NULL);
	context *ctx  = malloc(sizeof(context));
	memset(ctx, 0, sizeof(ctx));
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

		ssize_t written = write(fd, target->source.mem.data,  target->source.mem.size);
		if (written == -1 || (size_t)written != target->source.mem.size) {
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
			exit(1);
		}
	}else {
		waitpid(pid, &stats, 0);

		ctx->pid = pid;
		init_values(target, ctx);
		uint64_t entry= 0;
		if(ctx->arch == 32){
			 entry = ctx->entry & 0xffffffff;
		}else {
			entry = ctx->entry ;
		
		}

		// setBreakPoint at entry point
		unsigned long orig = ptrace(PTRACE_PEEKTEXT, ctx->pid, (void*)entry, NULL);
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
			parse_cmd(ctx);
			if(ctx->do_wait){
				waitpid(ctx->pid, &stats, 0);
				if (WIFSTOPPED(stats) && WSTOPSIG(stats) == SIGTRAP) {
					dis_ctx(ctx);
					handle_bpoint(ctx);
				} else if (WIFEXITED(stats) || (WIFSTOPPED(stats) && WSTOPSIG(stats) != SIGTRAP)  ) {
					destroy_all(ctx);
					exit(0);
				}
			}
		
		}
	}

	return true;
}
