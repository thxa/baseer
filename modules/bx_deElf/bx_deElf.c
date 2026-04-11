#include "bx_deElf.h"
#include "../bparser/bparser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdint.h>

#ifndef RETDEC_DEFAULT_BIN
#define RETDEC_DEFAULT_BIN "/opt/baseer/decompiler/bin/retdec-decompiler"
#endif

#define COLOR_STRING  "\033[0;32m"  // green
#define COLOR_LINENO "\033[0;35m"  // magenta
#define COLOR_COMMENT  "\033[0;90m"  // gray
#define COLOR_PUNCTUATION "\033[0;37m" // Light gray for punctuation (braces, etc.)

static const char *keywords[] = {
    "if", "else", "for", "while", "do", "switch", "case", "break", "continue", "return", "goto", "default", "const", "static", "volatile", NULL
};

static const char *types[] = {
    "int", "long", "short", "char", "void", "bool", "float", "double", "size_t", "unsigned", "signed", "struct", "union", "enum", "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", NULL
};

static int is_word(const char *p, const char *word) {
    size_t len = strlen(word);
    return strncmp(p, word, len) == 0 &&
           !isalnum((unsigned char)p[len]) && p[len] != '_';
}

static int is_punctuation(char c) { 
    return c == '{' || c == '}' || c == '(' || c == ')' || c == ';' || c == ',' || c == '=' || c == '*' || c == '&' || c == '[' || c == ']'; 
}


void print_decompiled_code(const char *c_code, int with_line_numbers) {
    if (!c_code) return;

    const char *p = c_code;
    int line_number = 1;
    int in_function_decl = 0; // Track if we're in a function declaration
    int after_type = 0;       // Track if we just saw a type (for function/variable detection)
    int skipped_lines = 0;    // Count skipped header comment lines

    // Skip the first four comment lines
    while (*p && skipped_lines < 4) {
        if (*p == '/' && *(p + 1) == '/') {
            while (*p && *p != '\n') {
                p++;
            }
            if (*p == '\n') {
                p++;
                skipped_lines++;
            }
        } else if (*p == '\n') {
            p++;
            skipped_lines++;
        } else {
            break; // Non-comment line encountered before 4 lines
        }
    }

    if (with_line_numbers)
        printf(COLOR_LINENO "%4d | " COLOR_RESET, line_number);

    while (*p) {
        // Handle comments
        if (*p == '/' && *(p + 1) == '/') {
            printf(COLOR_COMMENT "//");
            p += 2;
            while (*p && *p != '\n') {
                putchar(*p++);
            }
            printf(COLOR_RESET);
            continue;
        }
        if (*p == '/' && *(p + 1) == '*') {
            printf(COLOR_COMMENT "/*");
            p += 2;
            while (*p && !(*p == '*' && *(p + 1) == '/')) {
                putchar(*p++);
            }
            if (*p) { printf("*/"); p += 2; }
            printf(COLOR_RESET);
            continue;
        }

        // Handle preprocessor directives
        if (*p == '#' && (p == c_code || *(p - 1) == '\n')) {
            printf(COLOR_BMAGENTA "#");
            p++;
            while (*p && *p != '\n' && *p != ' ') {
                putchar(*p++);
            }
            while (*p && *p != '\n') {
                putchar(*p++);
            }
            printf(COLOR_RESET);
            continue;
        }

        // Handle strings
        if (*p == '"' || *p == '\'') {
            char quote = *p;
            printf(COLOR_STRING "%c", quote);
            p++;
            while (*p && *p != quote) {
                if (*p == '\\' && *(p + 1)) {
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
        if (isdigit((unsigned char)*p) || (*p == '0' && *(p + 1) == 'x')) {
            printf(COLOR_BYELLOW);
            if (*p == '0' && *(p + 1) == 'x') {
                printf("0x");
                p += 2;
                while (isxdigit((unsigned char)*p)) {
                    putchar(*p++);
                }
            } else {
                while (isdigit((unsigned char)*p)) {
                    putchar(*p++);
                }
            }
            printf(COLOR_RESET);
            continue;
        }

        // Handle identifiers (keywords, types, functions, variables)
        if (isalpha((unsigned char)*p) || *p == '_') {
            char identifier[256];
            int i = 0;
            while ((isalnum((unsigned char)*p) || *p == '_') && i < 255) {
                identifier[i++] = *p++;
            }
            identifier[i] = '\0';

            int matched = 0;
            // Check for keywords
            for (const char **kw = keywords; *kw; kw++) {
                if (strcmp(identifier, *kw) == 0) {
                    printf(COLOR_BBLUE "%s" COLOR_RESET, identifier);
                    matched = 1;
                    break;
                }
            }
            if (!matched) {
                // Check for types
                for (const char **ty = types; *ty; ty++) {
                    if (strcmp(identifier, *ty) == 0) {
                        printf(COLOR_BCYAN "%s" COLOR_RESET, identifier);
                        after_type = 1; // Set flag for next identifier (potential function/variable)
                        matched = 1;
                        break;
                    }
                }
            }
            if (!matched) {
                // Check if in function declaration or call
                if (after_type && *p == '(') {
                    // Function declaration (type followed by identifier and '(')
                    printf(COLOR_BGREEN "%s" COLOR_RESET, identifier);
                    in_function_decl = 1; // Enter function declaration context
                    after_type = 0;
                    matched = 1;
                } else if (in_function_decl && (*p == ',' || *p == ')')) {
                    // Parameter in function declaration
                    printf(COLOR_BWHITE "%s" COLOR_RESET, identifier);
                    if (*p == ')') in_function_decl = 0; // Exit function declaration
                    matched = 1;
                } else if (!in_function_decl && *p == '(') {
                    // Function call
                    printf(COLOR_BGREEN "%s" COLOR_RESET, identifier);
                    matched = 1;
                } else {
                    // Variable or other identifier
                    printf(COLOR_BWHITE "%s" COLOR_RESET, identifier);
                    matched = 1;
                }
            }
            continue;
        }

        // Handle punctuation
        if (is_punctuation(*p)) {
            printf(COLOR_PUNCTUATION "%c" COLOR_RESET, *p++);
            after_type = 0; // Reset after punctuation
            continue;
        }

        // Handle newline
        if (*p == '\n') {
            putchar('\n');
            p++;
            after_type = 0; // Reset on new line
            in_function_decl = 0; // Reset function declaration context
            if (with_line_numbers && *p) {
                line_number++;
                printf(COLOR_LINENO "%4d | " COLOR_RESET, line_number);
            }
            continue;
        }

        // Default: print character as is
        putchar(*p++);
    }
    if (p != c_code && *(p - 1) != '\n') {
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

    #define CHUNK_SIZE (64 * 1024)
    unsigned char *buffer = malloc(CHUNK_SIZE);
    if (!buffer) { fclose(out); return -1; }
    size_t pos = 0;

    while (pos < parser->size) {
        size_t to_read = parser->size - pos;
        if (to_read > CHUNK_SIZE) to_read = CHUNK_SIZE;

        size_t bytes_read = bparser_read(parser, buffer, pos, to_read);
        if (bytes_read != to_read) {
            fprintf(stderr, "[!] Failed to read binary at offset %zu (%zu of %zu bytes)\n",
                    pos, bytes_read, to_read);
            free(buffer);
            fclose(out);
            return -1;
        }
        if (fwrite(buffer, 1, bytes_read, out) != bytes_read) {
            perror("fwrite");
            free(buffer);
            fclose(out);
            return -1;
        }
        pos += bytes_read;
    }
    free(buffer);
    #undef CHUNK_SIZE

    fclose(out);
    return 0;
}

static bool run_retdec(const char *in_path, const char *out_path) {
    /* Allow overriding the decompiler path via environment variable */
    const char *retdec_bin = getenv("BASEER_RETDEC_BIN");
    if (!retdec_bin || *retdec_bin == '\0') {
        retdec_bin = RETDEC_DEFAULT_BIN;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return false;
    }

    if (pid == 0) {
        /* Child process: redirect stdout/stderr to /dev/null */
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull != -1) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        execl(retdec_bin, retdec_bin, "--cleanup", "-s", in_path, "-o", out_path, (char *)NULL);
        /* execl only returns on failure */
        _exit(127);
    }

    /* Parent process: wait for child */
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return false;
    }

    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        if (exit_status == 127) {
            fprintf(stderr, "[!] Failed to execute retdec-decompiler at: %s\n"
                            "[!] Install it with: sudo /opt/baseer/install_decompiler.sh\n"
                            "[!] Or set BASEER_RETDEC_BIN environment variable\n", retdec_bin);
            return false;
        }
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

    char in_path[] = "/tmp/baseer_input_XXXXXX";
    char out_path[] = "/tmp/baseer_output_XXXXXX";

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
        perror("dump_to_temp_file");
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

