#include "../bparser/bparser.h"
#include "bx_deElf.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int dump_to_temp_file(bparser *parser, const char *path){
    FILE *out = fopen(path, "wb");
    if (!out) {
        return -1;
    }

    char buffer[8192];
    size_t bytesRead;
    while ((bytesRead = bparser_read(parser, buffer, sizeof(buffer))) > 0) {
        if (fwrite(buffer, 1, bytesRead, out) != bytesRead) {
            fclose(out);
            return -1;
        }
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
        if (*p == '\n' && *(p + 1)) {
            line_number++;
            printf("%4d |  ", line_number);
        }
        p++;
    }
    if(*(p - 1) != '\n') {
        putchar('\n');
    }
}

bool decompile_elf(bparser *parser, void *arg){
    if(!parser) {
        return false;
    }

    char in_path[] = "/tmp/baseer_input_XXXXXX";
    char out_path[] = "/tmp/baseer_output_XXXXXX.c";

    int in_fd = mkstemp(in_path);
    if (in_fd < 0) {
        return false;
    }
    close(in_fd);

    int out_fd = mkstemp(out_path);
    if (out_fd < 0) {
        unlink(in_path);
        return false;
    }
    close(out_fd);

    if (dump_to_temp_file(parser, in_path) != 0) {
        unlink(in_path);
        unlink(out_path);
        return false;
    }

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "retdec-decompiler --cleanup -o %s %s > /dev/null 2>&1", out_path, in_path);
    int ret = system(cmd);
    unlink(in_path);

    if (ret != 0) {
        unlink(out_path);
        return false;
    }

    FILE *f = fopen(out_path, "rb");
    if (!f) {
        unlink(out_path);
        return false;
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *c_code = (char *)malloc(fsize + 1);
    if (!c_code) {
        fclose(f);
        unlink(out_path);
        return false;
    }

    fread(c_code, 1, fsize, f);
    c_code[fsize] = '\0';
    fclose(f);
    unlink(out_path);

    print_decompiled_code(c_code, 1);
    free(c_code);
    return true;
}