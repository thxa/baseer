#define _GNU_SOURCE
#include <elf.h>
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

/**
 * @file debugger.c
 * @brief Functions to handle debugging binary and command logic. 
 *
 */



/**
 * @brief Function to handle user commands. 
 * @param ctx is the cpu status
 *
 */
void parse_cmd(context *ctx){
	if(ctx == NULL)
		return;
	char cmd[1024] = {0} ;
	bool flag = false;
	printf("baseer-dbg> ");
	read(0, cmd, 1020);
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
			flag = cmds[i].func(ctx,(void*)args);
			break;

		}

	}
	if(!flag){
		printf("unkonw command\n");
		print_helpCMD();
		ctx->do_wait = false;
	}

}
/**
 * @brief Function to handle command that don't need a function . 
 * @param ctx is the cpu status
 *
 */
bool handle_action(context *ctx,void *args){

	if(strcmp(ctx->cmd.op,"q") == 0){
		ptrace(PTRACE_CONT, ctx->pid, NULL, SIGKILL);
		destroy_all(ctx);
		exit(0);
	}else if (strcmp(ctx->cmd.op,"h") == 0) {
		print_helpCMD();
		return true;
	}else if (strcmp(ctx->cmd.op,"vmmap") == 0) {
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
		restore_all_BP(ctx, 1);
		if (ptrace(PTRACE_SINGLESTEP, ctx->pid, 0, 0) == -1) {
			perror("SINGLESTEP");
			return true;
		}
		ctx->do_wait = true;
		return true;
	}else if (strcmp(ctx->cmd.op,"i") == 0) {
		sym_list *sym = ctx->sym;
		while (sym != NULL) {
			printf("function %s 0x%llx\n",sym->name,sym->addr);
			sym = sym->next;
		}
		ctx->do_wait = false;
		return true;
	}

}

/**
 * @brief print the help commands . 
 *
 */
void print_helpCMD(){
	printf("bp    : set breakpoint {ex: bp 0x12354}\n");
	printf("dp    : delete breakpoint {ex: dp breakpoint_id}\n");
	printf("lp    : list all breakpoints {ex: lp}\n");
	printf("si    : take one step execution (step into) {ex: si}\n");
	printf("so    : take one step execution (step over){ex: so}\n");
	printf("c     : continue execution {ex: c}\n");
	printf("h     : display help commands {ex: h}\n");
	printf("vmmap : display maps memory {ex: vmmap}\n");
	printf("i     : display function info {ex: i}\n");
}
/**
 * @brief delete a breakpoint. 
 *
 */
bool delBP(context *ctx, void* args){
	char *arg = (char*)args;
	ctx->do_wait = false;
	while(isspace((unsigned char)*arg)) arg++;
	char *token = strtok(arg, " ");
	uint64_t id = strtol(token,NULL,16);
	if(id == 0){
		printf("wrong id\n");
		return false;
	}
	if(ctx->list->first == NULL){
		printf("no breakpoints found\n");
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
			printf("deleted breakpoint at: %llx\n",ptr->addr);
			free(ptr);
			return true;
		}
		tmp = ptr;
		ptr = ptr->next;
	}
	printf("breakpoint with id: %d not found\n",id);
	return true;
	
}
/**
 * @brief function that do step over a function call. 
 *
 */
bool step_over(context *ctx,void *args){
	ctx->do_wait = false;
	ud_t ud_obj;
	uint64_t addr;
	uint64_t orig;
	uint8_t data[160];
	for(int i = 0; i < 20; i++) {
		long inst = ptrace(PTRACE_PEEKDATA, ctx->pid, (void*)(ctx->regs.rip + i*8), NULL);
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
	ctx->regs.rip = addr;
	ptrace(PTRACE_SETREGS, ctx->pid, 0, &ctx->regs);
	dis_ctx(ctx);
	return true;
}


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
 * @brief set a breakpoint. 
 *
 */
bool setBP(context *ctx, void* args){
	char *arg = (char*)args;
	ctx->do_wait = false;

	while(isspace((unsigned char)*arg)) arg++;
	char *token = strtok(arg, " ");
	if(token == NULL){
		return false;
	}
	uint64_t addr ;
	bool is_str = isalpha(token[0]);
	if(is_str){
		addr = find_sym(ctx, token);
		if(addr == 0){
			printf("undind symbol %s \n",token);
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

	if (ptrace(PTRACE_SINGLESTEP, ctx->pid, 0, 0) == -1) {
		perror("SINGLESTEP");
		return;
	}
}

bool listBP(context *ctx,void *args){
	ptrace(PTRACE_GETREGS, ctx->pid, NULL, &ctx->regs);
	ctx->do_wait = false;
	if(ctx->list->first == NULL){
		printf("there is no break points\n");
		return true;
	}

	bp *head = ctx->list->first;
	bp *ptr = head;
	while (ptr != NULL) {
		printf("[*] break point id: %d at : 0x%llx\n",ptr->id,ptr->addr);
		ptr = ptr->next;
	}
	return true;
}
void restore_all_BP(context *ctx,int opt){
	ptrace(PTRACE_GETREGS, ctx->pid, NULL, &ctx->regs);
	if(ctx->list->first != NULL){
		bp *head = ctx->list->first;
		bp *ptr = head;
		while (ptr != NULL) {
			if (opt == 1){
				ptrace(PTRACE_POKETEXT, ctx->pid, (void*)ptr->addr, (void*)ptr->orig);
			}else if(opt == 0){
				long trap = (ptr->orig & ~0xff) | 0xCC;
				ptrace(PTRACE_POKETEXT, ctx->pid, (void*)ptr->addr, (void*)trap);
			}
			ptr = ptr->next;
		}
	}
}
void dis_ctx(context *ctx){

	ud_t ud_obj;
	uint8_t data[160];
	printf(COLOR_YELLOW "------------- regs ----------------\n" COLOR_RESET);
	if(ctx->arch == 64){
		printf(COLOR_CYAN "\tRAX " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.rax);
		printf(COLOR_CYAN "\tRDX " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.rdx);
		printf(COLOR_CYAN "\tRCX " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.rcx);
		printf(COLOR_CYAN "\tRBX " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.rbx);
		printf(COLOR_CYAN "\tRDI " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.rdi);
		printf(COLOR_CYAN "\tRSI " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.rsi);
		printf(COLOR_CYAN "\tR8  " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.r8);
		printf(COLOR_CYAN "\tR9  " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.r9);
		printf(COLOR_CYAN "\tR10 " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.r10);
		printf(COLOR_CYAN "\tR11 " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.r11);
		printf(COLOR_CYAN "\tR12 " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.r12);
		printf(COLOR_CYAN "\tR13 " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.r13);
		printf(COLOR_CYAN "\tR14 " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.r14);
		printf(COLOR_CYAN "\tR15 " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.r15);
		printf(COLOR_CYAN "\tRSP " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.rsp);
		printf(COLOR_CYAN "\tRBP " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.rbp);
		printf(COLOR_CYAN "\tRIP " COLOR_RESET "=> " COLOR_BLUE "0x%llx\n"COLOR_RESET, ctx->regs.rip);
	}else {

		printf(COLOR_CYAN "\tEAX  " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.rax);
		printf(COLOR_CYAN "\tEDX  " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.rdx);
		printf(COLOR_CYAN "\tECX  " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.rcx);
		printf(COLOR_CYAN "\tEBX  " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.rbx);
		printf(COLOR_CYAN "\tEDI  " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.rdi);
		printf(COLOR_CYAN "\tESI  " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.rsi);
		printf(COLOR_CYAN "\tR8d  " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.r8);
		printf(COLOR_CYAN "\tR9d  " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.r9);
		printf(COLOR_CYAN "\tR10d " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.r10);
		printf(COLOR_CYAN "\tR11d " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.r11);
		printf(COLOR_CYAN "\tR12d " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.r12);
		printf(COLOR_CYAN "\tR13d " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.r13);
		printf(COLOR_CYAN "\tR14d " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.r14);
		printf(COLOR_CYAN "\tR15d " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.r15);
		printf(COLOR_CYAN "\tESP  " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.rsp);
		printf(COLOR_CYAN "\tEBP  " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.rbp);
		printf(COLOR_CYAN "\tEIP  " COLOR_RESET "=> " COLOR_RESET "0x%llx\n", ctx->regs.rip);
	}
	int len =sizeof(flags)/sizeof(e_FLAGS) ;
	printf("FLAGS: ");
	for (int i = 0 ; i<len ; i++) {
		int is_set = (ctx->regs.eflags >> flags[i].offset) & 1;
		if(is_set){
			printf(COLOR_GREEN  "%s " COLOR_RESET, flags[i].FLAG_NAME);
		}else {
			printf(COLOR_RED    "%s " COLOR_RESET, flags[i].FLAG_NAME);
		}
	}
	printf("\n");
	printf(COLOR_YELLOW "------------- disass ----------------\n" COLOR_RESET);
	for(int i = 0; i < 20; i++) {
		long inst = ptrace(PTRACE_PEEKDATA, ctx->pid, (void*)(ctx->regs.rip + i*8), NULL);
		memcpy((void*)&data[i*8], (void*)&inst , sizeof(inst));
	}
	ud_init(&ud_obj);
	ud_set_input_buffer(&ud_obj, data, sizeof(data));
	ud_set_mode(&ud_obj, ctx->arch);
	ud_set_syntax(&ud_obj, UD_SYN_INTEL);
	ud_set_pc(&ud_obj, ctx->regs.rip);
	while(ud_disassemble(&ud_obj)){
		if(ud_insn_off(&ud_obj) == ctx->regs.rip){
			printf(COLOR_GREEN "    --> 0x%llx: %s\n"COLOR_RESET,(uint64_t)ud_insn_off(&ud_obj),ud_insn_asm(&ud_obj));
		}else {
			printf(COLOR_MAGENTA "\t0x%llx: %s\n"COLOR_RESET,(uint64_t)ud_insn_off(&ud_obj),ud_insn_asm(&ud_obj));
		}
		if(ud_insn_mnemonic(&ud_obj)  == UD_Iret || ud_insn_off(&ud_obj) >= (ctx->regs.rip + 0x20))
			break;
	}

	printf(COLOR_YELLOW "------------- stack ----------------\n" COLOR_RESET);
	for(int i = 0; i < 10; i++) {
		if(ctx->arch == 64){
			long v = ptrace(PTRACE_PEEKDATA, ctx->pid, (void*)(ctx->regs.rsp + i*8), NULL);
			printf(COLOR_YELLOW "\t0x%llx" COLOR_RESET " =>" COLOR_BLUE " 0x%lx\n"COLOR_RESET, (ctx->regs.rsp + i *8), v);
		}else {
			long v = ptrace(PTRACE_PEEKDATA, ctx->pid, (void*)(ctx->regs.rsp + i*4), NULL);
			printf(COLOR_YELLOW "\t0x%llx" COLOR_RESET " =>"COLOR_BLUE " 0x%04x\n"COLOR_RESET, (ctx->regs.rsp + i *4), v);
		}
	}

}




void init_values(bparser *target, context *ctx){
	bp_list *list = malloc(sizeof(bp_list));
	char mmaps[512]= {0};
	ctx->do_wait = false;
	ctx->sym = NULL;
	memset(list, 0, sizeof(bp_list));
	list->counter = 0;
	ctx->list = list;
	Elf64_Ehdr *ehdr = (Elf64_Ehdr*)target->block;
	sprintf(mmaps, "/proc/%d/maps", ctx->pid);
	FILE *file = fopen(mmaps,"r");
	size_t size = 0;
	ssize_t b_read = getdelim(&ctx->mmaps,&size,'\0' , file);
	sscanf(ctx->mmaps, "%lx", &ctx->base);
	if(ehdr->e_type == ET_EXEC){
		ctx->entry = ehdr->e_entry;
		ctx->base |= 1 ;
	}else {
		ctx->entry = ctx->base + ehdr->e_entry;
	}	
	if(ehdr->e_ident[EI_CLASS] == ELFCLASS32){
		ctx->arch = 32;
		ctx->entry = ctx->entry & 0xffffffff;
		Elf32_Ehdr *ehdr = (Elf32_Ehdr*)target->block;
		Elf32_Shdr *shdr = target->block + ehdr->e_shoff;

		if (ehdr->e_shnum != 0) {
			char *shstrtab = (char*)target->block + shdr[ehdr->e_shstrndx].sh_offset;

			for (int i = 0; i < ehdr->e_shnum; i++) {
				if (shdr[i].sh_type == SHT_SYMTAB) {
					Elf32_Sym *syms = target->block + shdr[i].sh_offset;
					Elf32_Shdr *strtab_hdr = &shdr[shdr[i].sh_link];
					char *strtab = (char*)target->block + strtab_hdr->sh_offset;

					int num = shdr[i].sh_size / shdr[i].sh_entsize;
					for (int j = 0; j < num; j++) {
						if (ELF32_ST_TYPE(syms[j].st_info) == STT_FUNC &&
							syms[j].st_shndx != SHN_UNDEF) {

							char *name = strtab + syms[j].st_name;
							if (*name) {
								sym_list *sym = malloc(sizeof(sym_list));

								sym->name = strdup(name);
								sym->addr = (ctx->base & 1) ? syms[j].st_value : ctx->base + syms[j].st_value;
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
			char *shstrtab = target->block + shdr[ehdr->e_shstrndx].sh_offset;
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
								sym->addr = (ctx->base & 1) ? syms[j].st_value : ctx->base + syms[j].st_value;
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
	int argc = *((inputs*)arg) -> argc;
	char** args = ((inputs*)arg) -> args;
	context *ctx  = malloc(sizeof(context));
	memset(ctx, 0, sizeof(ctx));
	int stats = 0;
	int pid = fork();
	if (pid < 0 ){
		printf("faild to fork");
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
		char *argv[] = {args[1], NULL};
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
				printf("failed to read regs\n");
			}
			if(ptrace(PTRACE_POKETEXT, ctx->pid, (void*)ctx->entry, (void*)orig)== -1){
				printf("failed to restore entry point\n");
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
					restore_all_BP(ctx,1);
					dis_ctx(ctx);
				} else if (WIFEXITED(stats) || (WIFSTOPPED(stats) && WSTOPSIG(stats) != SIGTRAP)  ) {
					destroy_all(ctx);
					exit(0);
				}

			}
			free(ctx->cmd.op);
			ctx->cmd.op = 0;
			ctx->cmd.addr = 0;
			ctx->cmd.extra = 0;
		}
	}

	return true;
}
