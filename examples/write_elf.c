#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>

int main() {
    const char msg[] = "Hello\n";
    size_t msg_len = sizeof(msg) - 1;

    // Machine code for:
    //   write(1, msg, msg_len)
    //   exit(0)
    // Syscalls in x86-64 Linux:
    //   write: rax=1, rdi=fd, rsi=buf, rdx=len, syscall
    //   exit : rax=60, rdi=code, syscall
    unsigned char code[] = {
        0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00,       // mov rax,1 (sys_write)
        0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00,       // mov rdi,1 (stdout)
        0x48, 0xbe, 0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00, // mov rsi,0x400000+offset
        0x48, 0xc7, 0xc2, msg_len, 0x00, 0x00, 0x00,       // mov rdx,12 (len)
        0x0f, 0x05,                                     // syscall
        0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00,       // mov rax,60 (exit)
        0x48, 0x31, 0xff,                               // xor rdi,rdi
        0x0f, 0x05                                      // syscall
    };

    // Create ELF header
    Elf64_Ehdr eh = {0};
    memcpy(eh.e_ident, ELFMAG, SELFMAG);
    eh.e_ident[EI_CLASS] = ELFCLASS64;
    eh.e_ident[EI_DATA] = ELFDATA2LSB;
    eh.e_ident[EI_VERSION] = EV_CURRENT;
    eh.e_ident[EI_OSABI] = ELFOSABI_SYSV;
    eh.e_type = ET_EXEC;
    eh.e_machine = EM_X86_64;
    eh.e_version = EV_CURRENT;
    eh.e_phoff = sizeof(Elf64_Ehdr);
    eh.e_ehsize = sizeof(Elf64_Ehdr);
    eh.e_phentsize = sizeof(Elf64_Phdr);
    eh.e_phnum = 1;
    eh.e_entry = 0x400000 + sizeof(Elf64_Ehdr) + (sizeof(Elf64_Phdr) * eh.e_phnum) + (sizeof(Elf64_Shdr) * eh.e_shnum);  // entry point (after headers, where code starts)
                                                                                 
    // Program header
    Elf64_Phdr ph = {0};
    ph.p_type = PT_LOAD;
    ph.p_flags = PF_R | PF_X | PF_W;
    ph.p_offset = 0;
    ph.p_vaddr = 0x400000;
    ph.p_paddr = 0x400000;
    ph.p_filesz = sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) + sizeof(code) + msg_len;
    ph.p_memsz = ph.p_filesz;
    ph.p_align = 0x1000;

    // Patch message address into machine code (offset for mov rsi)
    uint64_t msg_addr = 0x400000 + sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) + sizeof(code);
    memcpy(&code[16], &msg_addr, sizeof(msg_addr));

    // Write to file
    int fd = open("hello", O_CREAT | O_WRONLY | O_TRUNC, 0755);
    if (fd < 0) { perror("open"); exit(1); }

    write(fd, &eh, sizeof(eh));
    write(fd, &ph, sizeof(ph));
    write(fd, code, sizeof(code));
    write(fd, msg, msg_len);
    close(fd);

    printf("ELF file 'hello' created. Run with ./hello\n");
    return 0;
}
