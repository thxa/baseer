#include "bx_deElf.h"
#include "../bparser/bparser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdint.h>

#ifndef RETDEC_DEFAULT_BIN
#define RETDEC_DEFAULT_BIN "/home/t/RetDec-v5.0-Linux-Release/bin/retdec-decompiler"
#endif

static int dump_to_temp_file(bparser *parser, const char *path) {

    if (!parser || !path) {
        errno = EINVAL;
        return -1;
    }

    FILE *out = fopen(path, "wb");
    if (!out) {
        perror("fopen");
        return -1;
    }

    unsigned char buffer[parser->size];
    unsigned int offset = 0; // new

    bparser_read(parser, buffer, offset, parser->size);
    if (fwrite(buffer, 1, parser->size, out) != parser->size) {
        perror("fwrite");
        fclose(out);
        return -1;
    }


    // Optional: check if bparser_read failed
    if (ferror(out)) {
        perror("write error");
        fclose(out);
        return -1;
    }

    fclose(out);
    return 0;
}

void print_decompiled_code(const char *c_code, int with_line_numbers){
    if (!c_code) return;

    if (!with_line_numbers) {
        printf("%s\n", c_code);
        return;
    }

    const char *p = c_code;
    int line_number = 1;
    printf("%4d |  ", line_number);
    while (*p) {
        putchar(*p);
        if (*p == '\n') {
            if (*(p + 1)) {
                line_number++;
                printf("%4d |  ", line_number);
            }
        }
        p++;
    }
    if(p != c_code && *(p - 1) != '\n') {
        putchar('\n');
    }
}

static bool run_retdec(const char *in_path, const char *out_path) {
    char cmd[2048];
    const char *retdec_bin = RETDEC_DEFAULT_BIN;

    int n = snprintf(cmd, sizeof(cmd), "%s -o %s %s > /dev/null 2>&1", retdec_bin, out_path, in_path);
    if (n < 0 || n >= (size_t)sizeof(cmd)) {
        fprintf(stderr, "[!] command too long or snprintf error\n");
        return false;
    }

    int ret = system(cmd);
    if (ret == -1) {
        perror("system");
        return false;
    }

    if (WIFEXITED(ret)){
        int exit_status = WEXITSTATUS(ret);
        if (exit_status != 0) {
            fprintf(stderr, "[!] retdec-decompiler exited with status %d\n", exit_status);
            return false;
        }
    } else {
        fprintf(stderr, "[!] retdec-decompiler did not terminate normally\n");
        return false;
    }

    return true;
}

bool decompile_elf(bparser *parser, void *arg){
    if(!parser) {
        return false;
    }

    char in_path[] = "./tmp/baseer_input_XXXXXX";
    char out_path[] = "./tmp/baseer_output_XXXXXX";

    int in_fd = mkstemp(in_path);
    if (in_fd < 0) {
        perror("mkstemp");
        return false;
    }
    close(in_fd);

    int out_fd = mkstemp(out_path);
    if (out_fd < 0) {
        perror("mkstemp");
        unlink(in_path);
        return false;
    }
    close(out_fd);

    if (dump_to_temp_file(parser, in_path) != 0) {
        perror("dump_tp_temp_file");
        unlink(in_path);
        unlink(out_path);
        return false;
    }
    
    if (!run_retdec(in_path, out_path)) {
        unlink(in_path);
        unlink(out_path);
        return false;
    }

    FILE *f = fopen(out_path, "rb");
    if (!f) {
        perror("fopen");
        unlink(in_path);
        unlink(out_path);
        return false;
    }
    
    if (fseek(f, 0, SEEK_END) != 0) {
        perror("fseek");
        fclose(f);
        unlink(in_path);
        unlink(out_path);
        return false;
    }
    
    long out_size = ftell(f);
    if (out_size < 0) {
        perror("ftell");
        fclose(f);
        unlink(in_path);
        unlink(out_path);
        return false;
    }
    rewind(f);
    
    char *c_code = malloc((size_t)out_size + 1);
    if (!c_code) {
        perror("malloc");
        fclose(f);
        unlink(in_path);
        unlink(out_path);
        return false;
    }

    size_t read_size = fread(c_code, 1, (size_t)out_size, f);
    fclose(f);
    c_code[read_size] = '\0'; // Null-terminate


    print_decompiled_code(c_code, 1);
    free(c_code);
    unlink(in_path);
    unlink(out_path);
    
    return true;
}
