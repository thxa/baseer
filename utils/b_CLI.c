#include "b_CLI.h"

void parse_args(inputs *input){
    input->input_argc = 0;
    for (int i = 2; i < *(input->argc); i++){
        if (strcmp(input->args[i], "--args") == 0){
            i++;
            while (i < *(input->argc) && input->input_argc < MAX_INPUT_ARGS){
                input->input_args[input->input_argc++] = input->args[i];
                i++;
            }
        }
    }
}

void completion(const char *prefix, linenoiseCompletions *lc) {
    if (strncmp(prefix, "o", 4) == 0) linenoiseAddCompletion(lc, "open");
    if (strncmp(prefix, "open", 4) == 0 ) {
        // file autocom after "open"
        const char *path = prefix + 4;
        if (*path == ' ') path++; //default to current dir

        const char *s_path = (*path == '\0') ? "." : path;
        const char *slash = strrchr(path, '/');

        char dirpath[512], base[256];
        if (slash){
            snprintf(dirpath, sizeof(dirpath), "%.*s", (int)(slash - s_path + 1), s_path);
            snprintf(base, sizeof(base), "%s", slash + 1);
        } else {
            strcpy(dirpath, "./");
            strcpy(base, s_path);
        }

        DIR *dir = opendir(dirpath);
        if (dir){
            struct dirent *de;
            while((de = readdir(dir))){
                if (strncmp(de->d_name, base, strlen(base)) == 0){
                    char suggestion[512];
                    snprintf(suggestion, sizeof(suggestion), "open %s%s", dirpath, de->d_name);
                    linenoiseAddCompletion(lc, suggestion);
                }
            }
            closedir(dir);
        }
        return;
    }
    if (strncmp(prefix, "a", 1) == 0) linenoiseAddCompletion(lc, "args ");
    if (strncmp(prefix, "s", 1) == 0) linenoiseAddCompletion(lc, "stored-args");
    if (strncmp(prefix, "m", 1) == 0) linenoiseAddCompletion(lc, "metadata");
    if (strncmp(prefix, "di", 2) == 0) linenoiseAddCompletion(lc, "disassembler");
    if (strncmp(prefix, "dec", 3) == 0) linenoiseAddCompletion(lc, "decompiler");
    if (strncmp(prefix, "deb", 3) == 0) linenoiseAddCompletion(lc, "debugger");
    if (strncmp(prefix, "c", 1) == 0) linenoiseAddCompletion(lc, "close");
    if (strncmp(prefix, "h", 1) == 0) linenoiseAddCompletion(lc, "help");
    if (strncmp(prefix, "q", 1) == 0) linenoiseAddCompletion(lc, "quit");
    if (strncmp(prefix, "e", 1) == 0) linenoiseAddCompletion(lc, "exit");
}

void baseer_CLI(void) {
    baseer_target_t *target = NULL;
    int cli_argc = 3;
    char *cli_args[4] = {"baseer", NULL, NULL, NULL};
    inputs input = {&cli_argc, cli_args};
    // create hashmap of any hashmaps needed by baseer extentions to used it for other extentions...
    input.map = create_map();
    linenoiseSetCompletionCallback(completion);

    printf("\nWelcome to Baseer CLI. Type 'help' for commands.\n");

    while (1) {
        fflush(stdout);
        char *line = linenoise(COLOR_WHITE"Baseer-\033[5;31mCLI"COLOR_RESET"-> "COLOR_RESET);
        if(!line) break; // Ctrl + D
        if(*line) linenoiseHistoryAdd(line);

        
        if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) {
            free(line);
            break;
        } else if (strcmp(line, "help") == 0) {
            printf("Commands:\n");
            printf("  open <file>        - Open a file with baseer\n");
            printf("  args <a1 a2 ...>   - Add extra arguments to send it to tool(s)\n");
            printf("  stored-args        - Print stored arguments\n");
            printf("  metadata           - Print metadata of the file\n");
            printf("  disassembler       - Disassemble the file\n");
            printf("  decompiler         - Decompile the file\n");
            printf("  debugger           - Debug the file\n");
            printf("  close              - Close current file\n");
            printf("  quit/exit          - Exit the program\n");
        } else if (strncmp(line, "open ", 5) == 0) {
            char *fname = line + 5;
            if (target){
                printf(COLOR_RED"[!] There is a file already open. Use 'close' first.\n"COLOR_RESET);
                continue;
            } else {
                target = baseer_open(fname, BASEER_MODE_BOTH);
                if (target){cli_args[1] = fname;printf("Opening file: %s\n", fname);}
                else {printf(COLOR_RED "[!] Failed to open file: %s\n" COLOR_RESET, fname);}
            }
        } else if (strcmp(line, "stored-args") == 0) {
            if (input.input_argc == 0) {
                printf(COLOR_YELLOW"[!] No arguments stored.\n"COLOR_RESET);
                continue;
            } else {
                printf("Stored arguments:\n");
                for (int i = 0; i < input.input_argc; i++) {
                    printf("  [%d] %s\n", i, input.input_args[i]);
                }
                continue;
            }
        } else if (strncmp(line, "metadata", 8) == 0) {
            linenoiseClearScreen();
            cli_args[2] = "-m";
            if (!target){
                printf(COLOR_RED"[!] No file opened. use 'open <file>' first.\n"COLOR_RESET);
                continue;
            }
            if (!baseer_execute(target, bx_binhead, &input)) {
                fprintf(stderr, "[!] Execution error\n");
                continue;
            }
        } else if (strncmp(line, "disassembler", 12) == 0) {
            linenoiseClearScreen();
            cli_args[2] = "-a";
            if (!target){
                printf(COLOR_RED"[!] No file opened. use 'open <file>' first.\n"COLOR_RESET);
                continue;
            }
            if (!baseer_execute(target, bx_binhead, &input)) {
                fprintf(stderr, "[!] Execution error\n");
                continue;
            }
        } else if (strncmp(line, "decompiler", 10) == 0) {
            linenoiseClearScreen();
            cli_args[2] = "-c";
            if (!target){
                printf(COLOR_RED"[!] No file opened. use 'open <file>' first.\n"COLOR_RESET);
                continue;
            }
            if (!baseer_execute(target, bx_binhead, &input)) {
                fprintf(stderr, "[!] Execution error\n");
                continue;
            }
        } else if (strncmp(line, "debugger", 8) == 0) {
            linenoiseClearScreen();
            cli_args[2] = "-d";
            if (!target){
                printf(COLOR_RED"[!] No file opened. use 'open <file>' first.\n"COLOR_RESET);
                continue;
            }
            if (!baseer_execute(target, bx_binhead, &input)) {
                fprintf(stderr, "[!] Execution error\n");
                continue;
            }
        } else if (strncmp(line, "args ", 5) == 0) {
            char *argline = line + 5;
            input.input_argc = 0;

            char *tok = strtok(argline, " ");
            while (input.input_argc < MAX_INPUT_ARGS && tok) {
                input.input_args[input.input_argc] = strdup(tok);
                input.input_argc++;
                tok = strtok(NULL, " ");
            }
            if (input.input_argc == 0) {
                printf(COLOR_RED"[!] No arguments entered.\n"COLOR_RESET);
                continue;
             } else {
                for (int i = 0; i < input.input_argc; i++) {
                    printf("  [%d] %s\n", i, input.input_args[i]);
                }
                continue;
            }
        } else if (strcmp(line, "close") == 0) {
            if(target){
                baseer_close(target);
                target = NULL;
                printf("Closed.\n");
                continue;
            } else {
                printf(COLOR_YELLOW"[!] No file is currently open.\n"COLOR_RESET);
                continue;
            }
        } else {
            printf(COLOR_YELLOW"Unknown command: %s\n"COLOR_RESET, line);
        }
        free(line);
    }
    if (target){
        free_map(input.map);
        baseer_close(target);
    }
}
