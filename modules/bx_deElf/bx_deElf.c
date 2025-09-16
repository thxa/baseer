<<<<<<< HEAD
#include "../bparser/bparser.h"
#include "bx_deElf.h"
=======
#include "bx_deElf.h"
#include "../bparser/bparser.h"
>>>>>>> 9c46ae5 (new changes)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
<<<<<<< HEAD

// static int dump_to_temp_file(bparser *parser, const char *path){
//     FILE *out = fopen(path, "wb");
//     if (!out) {
//         return -1;
//     }

//     char buffer[8192];
//     size_t bytesRead;
//     while ((bytesRead = bparser_read(parser, buffer, sizeof(buffer))) > 0) {
//         if (fwrite(buffer, 1, bytesRead, out) != bytesRead) {
//             fclose(out);
//             return -1;
//         }
//     }
//     fclose(out);
//     return 0;
// }

static int dump_to_temp_file(bparser *parser, const char *path) {
=======
#include <errno.h>
#include <sys/wait.h>
#include <stdint.h>

#ifndef RETDEC_DEFAULT_BIN
#define RETDEC_DEFAULT_BIN "/home/m97/Desktop/Project_dec/retttt/RetDec-v5.0-Linux-Release/bin/retdec-decompiler"
#endif

static int dump_to_temp_file(bparser *parser, const char *path) {

    if (!parser || !path) {
        errno = EINVAL;
        return -1;
    }

>>>>>>> 9c46ae5 (new changes)
    FILE *out = fopen(path, "wb");
    if (!out) {
        perror("fopen");
        return -1;
    }

<<<<<<< HEAD
    char buffer[8192];
    size_t bytesRead;
    printf("meow type: %d", parser->type);
    while ((bytesRead = bparser_read(parser, buffer, 0, sizeof(buffer))) > 0) {
=======
    unsigned char buffer[8192];
    size_t bytesRead;
    unsigned int offset = 0; // new

    printf("meow type: %d", parser->type);
    while ((bytesRead = bparser_read(parser, buffer, offset, sizeof(buffer))) > 0) {
        offset += (unsigned int)bytesRead; // new
>>>>>>> 9c46ae5 (new changes)
        if (fwrite(buffer, 1, bytesRead, out) != bytesRead) {
            perror("fwrite");
            fclose(out);
            return -1;
        }
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
<<<<<<< HEAD
        if (*p == '\n' && *(p + 1)) {
            line_number++;
            printf("%4d |  ", line_number);
        }
        p++;
    }
    if(*(p - 1) != '\n') {
=======
        if (*p == '\n') {
            if (*(p + 1)) {
                line_number++;
                printf("%4d |  ", line_number);
            }
        }
        p++;
    }
    if(p != c_code && *(p - 1) != '\n') {
>>>>>>> 9c46ae5 (new changes)
        putchar('\n');
    }
}

<<<<<<< HEAD
=======
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

>>>>>>> 9c46ae5 (new changes)
bool decompile_elf(bparser *parser, void *arg){
    if(!parser) {
        return false;
    }

    printf("meoowwwwwww");

    char in_path[] = "/home/m97/Desktop/baseer/temp/baseer_input_XXXXXX";
    char out_path[] = "/home/m97/Desktop/baseer/temp/baseer_output_XXXXXX";

    int in_fd = mkstemp(in_path);
    if (in_fd < 0) {
        perror("mkstemp");
        printf("first meow\n");
        return false;
    }
    close(in_fd);

    int out_fd = mkstemp(out_path);
    if (out_fd < 0) {
        perror("mkstemp");
        unlink(in_path);
        printf("second meow");
        return false;
    }
    close(out_fd);

    if (dump_to_temp_file(parser, in_path) != 0) {
        perror("dump_tp_temp_file");
        unlink(in_path);
        unlink(out_path);
        printf("third meow");
        return false;
    }
    
<<<<<<< HEAD
    char cmd[1024];
    printf("meow");

    snprintf(cmd, sizeof(cmd), "/home/m97/Desktop/Project_dec/retttt/RetDec-v5.0-Linux-Release/bin/retdec-decompiler -o %s %s > /dev/null 2>&1", out_path, in_path);
    int ret = system(cmd);
    unlink(in_path);

    if (ret != 0) {
        unlink(out_path);
=======
    if (!run_retdec(in_path, out_path)) {
        unlink(in_path);
        unlink(out_path);
        printf("fourth meow");
>>>>>>> 9c46ae5 (new changes)
        return false;
    }

    FILE *f = fopen(out_path, "rb");
    if (!f) {
<<<<<<< HEAD
        unlink(out_path);
        return false;
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *c_code = (char *)malloc(fsize + 1);
    if (!c_code) {
        fclose(f);
=======
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
>>>>>>> 9c46ae5 (new changes)
        unlink(out_path);
        return false;
    }

<<<<<<< HEAD
    fread(c_code, 1, fsize, f);
    c_code[fsize] = '\0';
    fclose(f);
    unlink(out_path);

    print_decompiled_code(c_code, 1);
    free(c_code);
=======
    size_t read_size = fread(c_code, 1, (size_t)out_size, f);
    fclose(f);
    c_code[read_size] = '\0'; // Null-terminate


    print_decompiled_code(c_code, 1);
    free(c_code);
    unlink(in_path);
    unlink(out_path);
    
>>>>>>> 9c46ae5 (new changes)
    return true;
}