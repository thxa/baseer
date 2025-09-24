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
#define RETDEC_DEFAULT_BIN "/home/m97/Desktop/Project_dec/retttt/RetDec-v5.0-Linux-Release/bin/retdec-decompiler"
#endif

#define COLOR_RESET   "\033[0m"
#define COLOR_KEYWORD "\033[1;34m"  // bright blue
#define COLOR_TYPE    "\033[1;36m"  // cyan
#define COLOR_STRING  "\033[0;32m"  // green
#define COLOR_NUMBER  "\033[1;33m"  // yellow
#define COLOR_COMMENT "\033[0;35m"  // magenta
#define COLOR_LINENO  "\033[0;90m"  // gray

static const char *keywords[] = {
    "if","else","for","while","do","switch","case","break",
    "continue","return","goto","default",NULL
};

static const char *types[] = {
    "int","long","short","char","void","bool","float","double",
    "size_t","unsigned","signed","struct","union","enum",NULL
};

static int is_word(const char *p, const char *word) {
    size_t len = strlen(word);
    return strncmp(p, word, len) == 0 &&
           !isalnum((unsigned char)p[len]) && p[len] != '_';
}

void print_decompiled_code(const char *c_code, int with_line_numbers) {
    if (!c_code) return;

    const char *p = c_code;
    int line_number = 1;

    if (with_line_numbers)
        printf(COLOR_LINENO "%4d | " COLOR_RESET, line_number);

    while (*p) {
        // Handle comments
        if (*p == '/' && *(p+1) == '/') {
            printf(COLOR_COMMENT "//");
            p += 2;
            while (*p && *p != '\n') {
                putchar(*p++);
            }
            printf(COLOR_RESET);
            continue;
        }
        if (*p == '/' && *(p+1) == '*') {
            printf(COLOR_COMMENT "/*");
            p += 2;
            while (*p && !(*p == '*' && *(p+1) == '/')) {
                putchar(*p++);
            }
            if (*p) { printf("*/"); p += 2; }
            printf(COLOR_RESET);
            continue;
        }

        // Handle strings
        if (*p == '"' || *p == '\'') {
            char quote = *p;
            printf(COLOR_STRING "%c", quote);
            p++;
            while (*p && *p != quote) {
                if (*p == '\\' && *(p+1)) {
                    putchar(*p++);
                }
                putchar(*p++);
            }
            if (*p == quote) {
                printf("%c", *p++);
            }
            printf(COLOR_RESET);
            continue;
        }

        // Handle numbers
        if (isdigit((unsigned char)*p)) {
            printf(COLOR_NUMBER);
            while (isdigit((unsigned char)*p)) {
                putchar(*p++);
            }
            printf(COLOR_RESET);
            continue;
        }

        // Handle identifiers (check for keywords/types)
        if (isalpha((unsigned char)*p) || *p == '_') {
            int matched = 0;
            for (const char **kw = keywords; *kw; kw++) {
                if (is_word(p, *kw)) {
                    printf(COLOR_KEYWORD "%s" COLOR_RESET, *kw);
                    p += strlen(*kw);
                    matched = 1;
                    break;
                }
            }
            if (!matched) {
                for (const char **ty = types; *ty; ty++) {
                    if (is_word(p, *ty)) {
                        printf(COLOR_TYPE "%s" COLOR_RESET, *ty);
                        p += strlen(*ty);
                        matched = 1;
                        break;
                    }
                }
            }
            if (!matched) {
                putchar(*p++);
            }
            continue;
        }

        // Handle newline
        if (*p == '\n') {
            putchar('\n');
            p++;
            if (with_line_numbers && *p) {
                line_number++;
                printf(COLOR_LINENO "%4d | " COLOR_RESET, line_number);
            }
            continue;
        }

        // Default
        putchar(*p++);
    }
    if(p != c_code && *(p - 1) != '\n') {
        putchar('\n');
    }
}


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

static bool run_retdec(const char *in_path, const char *out_path) {
    char cmd[2048];
    const char *retdec_bin = RETDEC_DEFAULT_BIN;

    int n = snprintf(cmd, sizeof(cmd), "%s --cleanup -s %s -o %s > /dev/null 2>&1", retdec_bin, in_path, out_path);
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

    char in_path[] = "/home/m97/Desktop/baseer/temp/baseer_input_XXXXXX";
    char out_path[] = "/home/m97/Desktop/baseer/temp/baseer_output_XXXXXX";

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