/**
 * @file baseer_gui.c
 * @brief Raylib + Raygui GUI frontend for Baseer binary analysis framework.
 *
 * Mirrors all CLI commands (open, close, metadata, disassembler, decompiler,
 * debugger, args) in a graphical interface with dark theme.
 *
 * Build: cmake -S . -B build -DBUILD_GUI=ON && cmake --build build
 * Run:   ./build/baseer-gui [file]
 */

#define _GNU_SOURCE
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "../baseer.h"
#include "../modules/binhead/bx_binhead.h"
#include "../modules/b_hashmap/b_hashmap.h"

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pty.h>
#include <sys/ioctl.h>
#include <errno.h>

#define INIT_WIDTH       1280
#define INIT_HEIGHT      720
#define SIDEBAR_W        270
#define STATUSBAR_H      32
#define PADDING          10
#define BTN_H            32
#define INPUT_H          28
#define OUTPUT_FONT_SIZE 18
#define LINE_HEIGHT      (OUTPUT_FONT_SIZE + 4)
#define FONT_LOAD_SIZE   36   /* Load at 2x for crisp rendering with bilinear filter */
#define FONT_GLYPH_COUNT 1024 /* Covers Latin, box-drawing, arrows, math symbols */
#define MAX_OUTPUT       (4 * 1024 * 1024)

#define DBG_INPUT_H      30
#define DBG_BUF_SIZE     (2 * 1024 * 1024)

#define COL_BG           CLITERAL(Color){ 22, 22, 38, 255 }
#define COL_SIDEBAR      CLITERAL(Color){ 28, 28, 48, 255 }
#define COL_PANEL        CLITERAL(Color){ 16, 20, 34, 255 }
#define COL_ACCENT       CLITERAL(Color){ 15, 52, 96, 255 }
#define COL_ACCENT_HOT   CLITERAL(Color){ 233, 69, 96, 255 }
#define COL_TEXT         CLITERAL(Color){ 210, 210, 210, 255 }
#define COL_TEXT_DIM     CLITERAL(Color){ 130, 130, 150, 255 }
#define COL_STATUS_BG    CLITERAL(Color){ 15, 15, 28, 255 }
#define COL_GREEN        CLITERAL(Color){ 80, 200, 120, 255 }
#define COL_RED          CLITERAL(Color){ 233, 69, 96, 255 }
#define COL_SEPARATOR    CLITERAL(Color){ 50, 50, 75, 255 }
#define COL_INPUT_BG     CLITERAL(Color){ 30, 30, 50, 255 }
#define COL_PROMPT       CLITERAL(Color){ 100, 200, 255, 255 }

static const char *MSG_NO_FILE = "[!] No file open. Open a file first.";
static const char *TOOL_NONE   = "None";

/* ── Debugger state (embedded PTY) ─────────────────────────────────── */
typedef struct {
    int   master_fd;       /* PTY master file descriptor              */
    pid_t pid;             /* Debugger child process PID              */
    bool  running;         /* True while debugger process is alive    */
    char *buf;             /* Accumulated output buffer               */
    int   buf_len;         /* Current length of output in buf         */
    int   buf_cap;         /* Allocated capacity of buf               */
    int   line_count;      /* Cached newline count (updated in dbg_append) */
    char  cmd[512];        /* Current command being typed             */
    bool  cmd_edit;        /* Whether the command input is focused    */
    bool  auto_scroll;     /* Auto-scroll to bottom on new output    */
    Vector2 scroll;        /* Scroll position (independent from normal output) */
} DbgState;

typedef struct {
    baseer_target_t *target;
    char filepath[512];
    char args_text[512];
    char *output;
    char  status[256];
    bool  file_open;

    bool filepath_edit;
    bool args_edit;

    Vector2 scroll;
    int     content_h;

    const char *active_tool;

    DbgState dbg;
} AppState;

static void strip_ansi(char *s)
{
    char *r = s, *w = s;
    while (*r) {
        if (*r == '\033') {
            r++;
            if (*r == '[') {
                r++;
                while (*r && !((*r >= 'A' && *r <= 'Z') || (*r >= 'a' && *r <= 'z')))
                    r++;
                if (*r) r++;
            }
        } else {
            *w++ = *r++;
        }
    }
    *w = '\0';
}

static int count_lines(const char *text)
{
    if (!text || !*text) return 0;
    int n = 1;
    for (const char *p = text; *p; p++)
        if (*p == '\n') n++;
    return n;
}

/* Replace current output, recompute derived state, reset scroll. */
static void set_output(AppState *st, char *text, const char *tool_name)
{
    free(st->output);
    st->output = text;
    st->content_h = count_lines(st->output) * LINE_HEIGHT;
    st->scroll = (Vector2){ 0, 0 };
    st->active_tool = tool_name;
}

/**
 * Redirect stdout+stderr to a temp file, run the tool, read back output
 * with ANSI codes stripped.  Caller must free() the result.
 */
static char *capture_tool(AppState *st, const char *flag)
{
    fflush(stdout);
    fflush(stderr);

    int saved_out = dup(STDOUT_FILENO);
    int saved_err = dup(STDERR_FILENO);

    FILE *tmp = tmpfile();
    if (!tmp) { return strdup("[Error: tmpfile() failed]\n"); }

    dup2(fileno(tmp), STDOUT_FILENO);
    dup2(fileno(tmp), STDERR_FILENO);

    /* Mimic CLI convention: baseer <file> <flag> */
    int argc = 3;
    char *argv[4] = { "baseer", st->filepath, (char *)flag, NULL };
    inputs input = { &argc, argv, .input_argc = 0, .map = NULL };
    input.map = create_map();

    if (st->args_text[0]) {
        char tmp_args[512];
        strncpy(tmp_args, st->args_text, sizeof(tmp_args) - 1);
        tmp_args[sizeof(tmp_args) - 1] = '\0';
        char *tok = strtok(tmp_args, " ");
        while (tok && input.input_argc < MAX_INPUT_ARGS) {
            input.input_args[input.input_argc++] = tok;
            tok = strtok(NULL, " ");
        }
    }

    /* Temporarily restore default SIGCHLD so that tools using fork+waitpid
     * (e.g. the decompiler's run_retdec) work correctly.  SIG_IGN causes
     * automatic child reaping, which makes waitpid() return -1/ECHILD. */
    struct sigaction old_sa;
    sigaction(SIGCHLD, &(struct sigaction){ .sa_handler = SIG_DFL }, &old_sa);

    baseer_execute(st->target, bx_binhead, &input);

    sigaction(SIGCHLD, &old_sa, NULL);

    free_map(input.map);

    fflush(stdout);
    fflush(stderr);

    dup2(saved_out, STDOUT_FILENO);
    dup2(saved_err, STDERR_FILENO);
    close(saved_out);
    close(saved_err);

    long sz = ftell(tmp);
    if (sz <= 0) { fclose(tmp); return strdup("[No output]\n"); }
    if (sz > MAX_OUTPUT) sz = MAX_OUTPUT;

    rewind(tmp);
    char *buf = malloc(sz + 1);
    if (!buf) { fclose(tmp); return strdup("[Error: out of memory]\n"); }
    fread(buf, 1, sz, tmp);
    buf[sz] = '\0';
    fclose(tmp);

    strip_ansi(buf);
    return buf;
}

static void open_file(AppState *st)
{
    if (st->file_open) {
        snprintf(st->status, sizeof(st->status), "[!] File already open. Close first.");
        return;
    }
    if (!st->filepath[0]) {
        snprintf(st->status, sizeof(st->status), "[!] Enter a file path.");
        return;
    }

    st->target = baseer_open(st->filepath, BASEER_MODE_BOTH);
    if (st->target) {
        st->file_open = true;
        snprintf(st->status, sizeof(st->status), "Opened: %s  (%u bytes)", st->filepath, st->target->size);
    } else {
        snprintf(st->status, sizeof(st->status), "[!] Failed to open: %s", st->filepath);
    }
}

static void close_file(AppState *st)
{
    if (!st->file_open) {
        snprintf(st->status, sizeof(st->status), "[!] No file is open.");
        return;
    }
    baseer_close(st->target);
    st->target = NULL;
    st->file_open = false;
    snprintf(st->status, sizeof(st->status), "File closed.");
}

static void run_tool(AppState *st, const char *flag, const char *name)
{
    if (!st->file_open) {
        snprintf(st->status, sizeof(st->status), "%s", MSG_NO_FILE);
        return;
    }

    set_output(st, capture_tool(st, flag), name);

    int lines = st->content_h / LINE_HEIGHT;
    snprintf(st->status, sizeof(st->status), "%s  |  %s  |  %u bytes  |  %d lines",
             name, st->filepath, st->target ? st->target->size : 0, lines);
}

/* ── Embedded debugger (PTY-based) ─────────────────────────────────── */

static void dbg_stop(AppState *st)
{
    DbgState *d = &st->dbg;
    if (!d->running) return;

    if (d->pid > 0) {
        kill(d->pid, SIGKILL);
        waitpid(d->pid, NULL, 0);
    }
    if (d->master_fd >= 0) {
        close(d->master_fd);
        d->master_fd = -1;
    }
    d->running = false;
    d->pid = 0;
    d->cmd[0] = '\0';
    d->cmd_edit = false;

    snprintf(st->status, sizeof(st->status), "Debugger stopped.");
}

static void dbg_append(DbgState *d, const char *data, int len)
{
    if (len <= 0) return;

    /* Grow buffer if needed */
    while (d->buf_len + len + 1 > d->buf_cap) {
        int new_cap = d->buf_cap * 2;
        if (new_cap < 4096) new_cap = 4096;
        char *nb = realloc(d->buf, new_cap);
        if (!nb) return;
        d->buf = nb;
        d->buf_cap = new_cap;
    }

    memcpy(d->buf + d->buf_len, data, len);
    d->buf_len += len;
    d->buf[d->buf_len] = '\0';

    /* Update cached line count for the appended data */
    for (int i = 0; i < len; i++)
        if (data[i] == '\n') d->line_count++;

    /* Trim if too large: keep the last half, recount lines */
    if (d->buf_len > DBG_BUF_SIZE) {
        int keep = DBG_BUF_SIZE / 2;
        memmove(d->buf, d->buf + d->buf_len - keep, keep);
        d->buf_len = keep;
        d->buf[d->buf_len] = '\0';
        d->line_count = count_lines(d->buf);
    }
}

/* Read any available output from the PTY master (non-blocking).
 * Caps reads per frame to avoid stalling the render loop. */
#define DBG_MAX_READ_PER_FRAME (16 * 1024)

static void dbg_poll(AppState *st)
{
    DbgState *d = &st->dbg;
    if (!d->running || d->master_fd < 0) return;

    int wstatus;
    pid_t w = waitpid(d->pid, &wstatus, WNOHANG);
    if (w > 0 || (w < 0 && errno == ECHILD)) {
        /* Child exited — drain remaining output */
        char tmp[4096];
        for (;;) {
            ssize_t n = read(d->master_fd, tmp, sizeof(tmp) - 1);
            if (n <= 0) break;
            tmp[n] = '\0';
            strip_ansi(tmp);
            dbg_append(d, tmp, (int)strlen(tmp));
        }
        close(d->master_fd);
        d->master_fd = -1;
        d->running = false;
        d->pid = 0;
        dbg_append(d, "\n[Debugger exited]\n", 18);
        d->auto_scroll = true;
        snprintf(st->status, sizeof(st->status), "Debugger exited.");
        return;
    }

    char tmp[4096];
    int total = 0;
    while (total < DBG_MAX_READ_PER_FRAME) {
        ssize_t n = read(d->master_fd, tmp, sizeof(tmp) - 1);
        if (n <= 0) break;
        tmp[n] = '\0';
        strip_ansi(tmp);
        int slen = (int)strlen(tmp);
        dbg_append(d, tmp, slen);
        total += slen;
        d->auto_scroll = true;
    }
}

static void dbg_send(AppState *st, const char *cmd)
{
    DbgState *d = &st->dbg;
    if (!d->running || d->master_fd < 0) return;

    int len = (int)strlen(cmd);
    if (len > 0 && write(d->master_fd, cmd, len) < 0) {
        snprintf(st->status, sizeof(st->status), "[!] Failed to send command to debugger.");
        return;
    }
    if (write(d->master_fd, "\n", 1) < 0) {
        snprintf(st->status, sizeof(st->status), "[!] Failed to send command to debugger.");
    }
}

static void launch_debugger(AppState *st)
{
    if (!st->file_open) {
        snprintf(st->status, sizeof(st->status), "%s", MSG_NO_FILE);
        return;
    }

    /* If debugger is already running, stop it first */
    if (st->dbg.running) {
        dbg_stop(st);
    }

    /* Resolve baseer CLI path: look next to baseer-gui, fall back to PATH. */
    char self_path[512] = {0};
    char baseer_bin[512] = "baseer";
    ssize_t len = readlink("/proc/self/exe", self_path, sizeof(self_path) - 1);
    if (len > 0) {
        self_path[len] = '\0';
        char *slash = strrchr(self_path, '/');
        if (slash) {
            char candidate[512];
            *(slash + 1) = '\0';
            snprintf(candidate, sizeof(candidate), "%sbaseer", self_path);
            if (access(candidate, X_OK) == 0)
                memcpy(baseer_bin, candidate, sizeof(baseer_bin));
        }
    }

    int master_fd, slave_fd;
    if (openpty(&master_fd, &slave_fd, NULL, NULL, NULL) == -1) {
        snprintf(st->status, sizeof(st->status), "[!] Failed to create PTY.");
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        /* ── CHILD: run debugger with PTY slave as terminal ── */
        close(master_fd);

        /* Create a new session and set slave as controlling terminal */
        setsid();
        ioctl(slave_fd, TIOCSCTTY, 0);

        dup2(slave_fd, STDIN_FILENO);
        dup2(slave_fd, STDOUT_FILENO);
        dup2(slave_fd, STDERR_FILENO);
        if (slave_fd > STDERR_FILENO)
            close(slave_fd);

        /* Set TERM so linenoise/ncurses work */
        setenv("TERM", "xterm-256color", 1);

        execlp(baseer_bin, "baseer", st->filepath, "-d", NULL);

        /* If exec fails */
        perror("exec baseer failed");
        _exit(1);
    } else if (pid > 0) {
        /* ── PARENT: store master fd, set non-blocking ── */
        close(slave_fd);

        int fl = fcntl(master_fd, F_GETFL);
        if (fl != -1) fcntl(master_fd, F_SETFL, fl | O_NONBLOCK);

        DbgState *d = &st->dbg;
        d->master_fd = master_fd;
        d->pid = pid;
        d->running = true;
        d->cmd[0] = '\0';
        d->cmd_edit = true;  /* Auto-focus the command input */
        d->auto_scroll = true;

        if (!d->buf) {
            d->buf_cap = 8192;
            d->buf = malloc(d->buf_cap);
        }
        if (d->buf) {
            d->buf[0] = '\0';
            d->buf_len = 0;
        }
        d->line_count = 0;
        d->scroll = (Vector2){ 0, 0 };

        snprintf(st->status, sizeof(st->status), "Debugger running (PID %d) — type commands below", pid);
        st->active_tool = "Debugger";

    } else {
        close(master_fd);
        close(slave_fd);
        snprintf(st->status, sizeof(st->status), "[!] Failed to fork for debugger.");
    }
}

static void clear_output(AppState *st)
{
    if (st->dbg.running) {
        st->dbg.buf_len = 0;
        st->dbg.line_count = 0;
        if (st->dbg.buf) st->dbg.buf[0] = '\0';
        snprintf(st->status, sizeof(st->status), "Debugger output cleared.");
        return;
    }
    set_output(st, NULL, TOOL_NONE);
    snprintf(st->status, sizeof(st->status), "Output cleared.");
}

static void setup_dark_theme(void)
{
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL,  ColorToInt(COL_SEPARATOR));
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL,    ColorToInt(COL_SIDEBAR));
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL,     ColorToInt(COL_TEXT));

    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, ColorToInt(COL_ACCENT_HOT));
    GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED,   ColorToInt(COL_ACCENT));
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED,    ColorToInt(WHITE));

    GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, ColorToInt(COL_ACCENT_HOT));
    GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED,   ColorToInt(COL_ACCENT));
    GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED,    ColorToInt(WHITE));

    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(COL_BG));
    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 1);

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL,  ColorToInt(COL_ACCENT));
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL,   ColorToInt(COL_TEXT));
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED,  ColorToInt((Color){ 25, 72, 126, 255 }));
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED,  ColorToInt(COL_ACCENT_HOT));
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, ColorToInt(COL_SEPARATOR));

    GuiSetStyle(TEXTBOX, BASE_COLOR_NORMAL,    ColorToInt(COL_PANEL));
    GuiSetStyle(TEXTBOX, TEXT_COLOR_NORMAL,     ColorToInt(COL_TEXT));
    GuiSetStyle(TEXTBOX, BORDER_COLOR_NORMAL,   ColorToInt(COL_SEPARATOR));
    GuiSetStyle(TEXTBOX, BASE_COLOR_FOCUSED,    ColorToInt((Color){ 30, 35, 55, 255 }));
    GuiSetStyle(TEXTBOX, BORDER_COLOR_FOCUSED,  ColorToInt(COL_ACCENT_HOT));

    GuiSetStyle(SCROLLBAR, BASE_COLOR_NORMAL,   ColorToInt(COL_PANEL));
    GuiSetStyle(SCROLLBAR, BORDER_COLOR_NORMAL,  ColorToInt(COL_SEPARATOR));

    GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(COL_TEXT_DIM));

    GuiSetStyle(STATUSBAR, BASE_COLOR_NORMAL,  ColorToInt(COL_STATUS_BG));
    GuiSetStyle(STATUSBAR, TEXT_COLOR_NORMAL,   ColorToInt(COL_TEXT));
    GuiSetStyle(STATUSBAR, BORDER_COLOR_NORMAL, ColorToInt(COL_SEPARATOR));
}

static void draw_output(const char *text, Font font, Rectangle view, Vector2 scroll)
{
    if (!text || !*text) return;

    int y = (int)(view.y + scroll.y);
    int view_bottom = (int)(view.y + view.height);
    const char *line = text;

    while (*line) {
        const char *eol = strchr(line, '\n');
        int len = eol ? (int)(eol - line) : (int)strlen(line);

        if (y > view_bottom) break;  /* all remaining lines are below viewport */

        if (y + LINE_HEIGHT >= (int)view.y) {
            char buf[2048];
            int n = len < (int)sizeof(buf) - 1 ? len : (int)sizeof(buf) - 1;
            memcpy(buf, line, n);
            buf[n] = '\0';
            DrawTextEx(font, buf, (Vector2){ view.x + 8 + scroll.x, (float)y },
                       OUTPUT_FONT_SIZE, 1, COL_TEXT);
        }

        y += LINE_HEIGHT;
        line = eol ? eol + 1 : line + len;
    }
}

static void draw_sep(int x, int y, int w)
{
    DrawLine(x + PADDING, y, x + w - PADDING, y, COL_SEPARATOR);
}

int main(int argc, char **argv)
{
    AppState st = { 0 };
    st.dbg.master_fd = -1;
    snprintf(st.status, sizeof(st.status), "Ready. Open a file or drag & drop.");
    st.active_tool = TOOL_NONE;

    if (argc >= 2) {
        strncpy(st.filepath, argv[1], sizeof(st.filepath) - 1);
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(INIT_WIDTH, INIT_HEIGHT, "Baseer - Binary Analysis Framework");
    SetTargetFPS(60);
    SetExitKey(0);

    Font mono = { 0 };
    /* Load at 2x size for crisp bilinear-filtered rendering at OUTPUT_FONT_SIZE. */
    static const char *font_paths[] = {
        /* JetBrains Mono */
        "/usr/share/fonts/TTF/JetBrainsMonoNerdFont-Regular.ttf",
        "/usr/share/fonts/TTF/JetBrainsMono-Regular.ttf",
        "/usr/share/fonts/truetype/jetbrains-mono/JetBrainsMono-Regular.ttf",
        "/usr/share/fonts/jetbrains-mono/JetBrainsMono-Regular.ttf",
        /* Fira Code */
        "/usr/share/fonts/TTF/FiraCodeNerdFont-Regular.ttf",
        "/usr/share/fonts/TTF/FiraCode-Regular.ttf",
        "/usr/share/fonts/truetype/firacode/FiraCode-Regular.ttf",
        "/usr/share/fonts/fira-code/FiraCode-Regular.ttf",
        /* Hack */
        "/usr/share/fonts/TTF/HackNerdFont-Regular.ttf",
        "/usr/share/fonts/TTF/Hack-Regular.ttf",
        "/usr/share/fonts/truetype/hack/Hack-Regular.ttf",
        /* Cascadia Code */
        "/usr/share/fonts/TTF/CascadiaCode.ttf",
        "/usr/share/fonts/truetype/cascadia-code/CascadiaCode.ttf",
        /* Source Code Pro */
        "/usr/share/fonts/TTF/SourceCodePro-Regular.ttf",
        "/usr/share/fonts/adobe-source-code-pro/SourceCodePro-Regular.ttf",
        "/usr/share/fonts/truetype/adobe/SourceCodePro-Regular.ttf",
        /* IBM Plex Mono */
        "/usr/share/fonts/TTF/IBMPlexMono-Regular.ttf",
        "/usr/share/fonts/truetype/ibm-plex/IBMPlexMono-Regular.ttf",
        /* Iosevka */
        "/usr/share/fonts/TTF/Iosevka-Regular.ttf",
        "/usr/share/fonts/truetype/iosevka/Iosevka-Regular.ttf",
        /* Classic fallbacks */
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/noto/NotoSansMono-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSansMono-Regular.ttf",
        "/usr/share/fonts/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        NULL,
    };
    for (int i = 0; font_paths[i]; i++) {
        if (FileExists(font_paths[i])) {
            mono = LoadFontEx(font_paths[i], FONT_LOAD_SIZE, NULL, FONT_GLYPH_COUNT);
            if (mono.glyphCount > 0) {
                SetTextureFilter(mono.texture, TEXTURE_FILTER_BILINEAR);
                break;
            }
        }
    }
    bool mono_loaded = (mono.glyphCount > 0);
    if (!mono_loaded) mono = GetFontDefault();

    setup_dark_theme();

    if (st.filepath[0]) open_file(&st);

    while (!WindowShouldClose()) {
        int W = GetScreenWidth();
        int H = GetScreenHeight();

        /* Poll debugger PTY for new output */
        if (st.dbg.running) {
            dbg_poll(&st);
        }

        if (IsFileDropped()) {
            FilePathList files = LoadDroppedFiles();
            if (files.count > 0) {
                if (st.dbg.running) dbg_stop(&st);
                if (st.file_open) close_file(&st);
                strncpy(st.filepath, files.paths[0], sizeof(st.filepath) - 1);
                open_file(&st);
            }
            UnloadDroppedFiles(files);
        }

        /* Handle debugger command input: Enter key sends command */
        if (st.dbg.running && st.dbg.cmd_edit) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
                dbg_send(&st, st.dbg.cmd);
                st.dbg.cmd[0] = '\0';
                st.dbg.auto_scroll = true;
            }
        }

        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
            if (IsKeyPressed(KEY_O)) st.filepath_edit = true;
            if (IsKeyPressed(KEY_M) && st.file_open) run_tool(&st, "-m", "Metadata");
            if (IsKeyPressed(KEY_A) && st.file_open) run_tool(&st, "-a", "Disassembler");
            if (IsKeyPressed(KEY_E) && st.file_open) run_tool(&st, "-c", "Decompiler");
            if (IsKeyPressed(KEY_D) && st.file_open) launch_debugger(&st);
            if (IsKeyPressed(KEY_W)) close_file(&st);
            if (IsKeyPressed(KEY_L)) clear_output(&st);
            if (IsKeyPressed(KEY_Q)) break;
        }

        BeginDrawing();
        ClearBackground(COL_BG);

        DrawRectangle(0, 0, SIDEBAR_W, H, COL_SIDEBAR);
        DrawLine(SIDEBAR_W, 0, SIDEBAR_W, H, COL_SEPARATOR);

        int sx = PADDING;
        int sy = PADDING;
        int sw = SIDEBAR_W - 2 * PADDING;

        DrawTextEx(mono, "BASEER", (Vector2){ sx + sw/2 - 36, sy }, 24, 2, COL_ACCENT_HOT);
        sy += 28;
        GuiLabel((Rectangle){ sx, sy, sw, 16 }, "Binary Analysis Framework");
        sy += 24;
        draw_sep(0, sy, SIDEBAR_W);
        sy += 10;

        GuiLabel((Rectangle){ sx, sy, sw, 16 }, "File Path:");
        sy += 20;
        if (GuiTextBox((Rectangle){ sx, sy, sw, INPUT_H }, st.filepath, sizeof(st.filepath), st.filepath_edit))
            st.filepath_edit = !st.filepath_edit;
        sy += INPUT_H + 6;

        int half = (sw - 6) / 2;
        if (GuiButton((Rectangle){ sx, sy, half, BTN_H }, "Open")) open_file(&st);
        if (GuiButton((Rectangle){ sx + half + 6, sy, half, BTN_H }, "Close")) close_file(&st);
        sy += BTN_H + 10;

        DrawCircle(sx + 8, sy + 6, 5, st.file_open ? COL_GREEN : COL_RED);
        GuiLabel((Rectangle){ sx + 20, sy - 2, sw - 20, 16 },
                 st.file_open ? "File loaded" : "No file");
        sy += 20;

        draw_sep(0, sy, SIDEBAR_W);
        sy += 10;

        GuiLabel((Rectangle){ sx, sy, sw, 16 }, "Analysis Tools:");
        sy += 22;

        if (GuiButton((Rectangle){ sx, sy, sw, BTN_H }, "#191#  Metadata     (Ctrl+M)"))
            run_tool(&st, "-m", "Metadata");
        sy += BTN_H + 4;

        if (GuiButton((Rectangle){ sx, sy, sw, BTN_H }, "#159#  Disassembler (Ctrl+A)"))
            run_tool(&st, "-a", "Disassembler");
        sy += BTN_H + 4;

        if (GuiButton((Rectangle){ sx, sy, sw, BTN_H }, "#186#  Decompiler   (Ctrl+E)"))
            run_tool(&st, "-c", "Decompiler");
        sy += BTN_H + 4;

        if (GuiButton((Rectangle){ sx, sy, sw, BTN_H },
                       st.dbg.running ? "#152#  Stop Debugger (Ctrl+D)"
                                      : "#152#  Debugger     (Ctrl+D)")) {
            if (st.dbg.running)
                dbg_stop(&st);
            else
                launch_debugger(&st);
        }
        sy += BTN_H + 10;

        draw_sep(0, sy, SIDEBAR_W);
        sy += 10;

        GuiLabel((Rectangle){ sx, sy, sw, 16 }, "Extra Arguments:");
        sy += 20;
        if (GuiTextBox((Rectangle){ sx, sy, sw, INPUT_H }, st.args_text, sizeof(st.args_text), st.args_edit))
            st.args_edit = !st.args_edit;
        sy += INPUT_H + 6;

        if (st.args_text[0]) {
            char args_display[128];
            snprintf(args_display, sizeof(args_display), "Args: %s", st.args_text);
            GuiLabel((Rectangle){ sx, sy, sw, 16 }, args_display);
        } else {
            GuiLabel((Rectangle){ sx, sy, sw, 16 }, "No extra args set.");
        }
        sy += 22;

        draw_sep(0, sy, SIDEBAR_W);
        sy += 10;

        if (GuiButton((Rectangle){ sx, sy, sw, BTN_H }, "Clear Output  (Ctrl+L)"))
            clear_output(&st);
        sy += BTN_H + 4;

        if (GuiButton((Rectangle){ sx, sy, sw, BTN_H }, "Help")) {
            if (st.dbg.running) dbg_stop(&st);
            set_output(&st, strdup(
                "=== Baseer GUI Help ===\n\n"
                "GUI Action           | CLI Equivalent\n"
                "---------------------+----------------------------\n"
                "Open                 | open <file>\n"
                "Close                | close\n"
                "Metadata (Ctrl+M)    | metadata    / baseer <f> -m\n"
                "Disassembler (Ctrl+A)| disassembler/ baseer <f> -a\n"
                "Decompiler (Ctrl+E)  | decompiler  / baseer <f> -c\n"
                "Debugger (Ctrl+D)    | debugger    / baseer <f> -d\n"
                "Extra Arguments      | args <a1 a2 ...>\n"
                "Clear Output (Ctrl+L)| (clear screen)\n"
                "Quit (Ctrl+Q)        | quit / exit\n\n"
                "Drag & drop a file onto the window to open it.\n\n"
                "Keyboard shortcuts:\n"
                "  Ctrl+O   Focus file path input\n"
                "  Ctrl+M   Run Metadata\n"
                "  Ctrl+A   Run Disassembler\n"
                "  Ctrl+E   Run Decompiler\n"
                "  Ctrl+D   Launch/Stop Debugger\n"
                "  Ctrl+W   Close file\n"
                "  Ctrl+L   Clear output\n"
                "  Ctrl+Q   Quit\n"
            ), "Help");
            snprintf(st.status, sizeof(st.status), "Help");
        }
        sy += BTN_H + 4;

        if (GuiButton((Rectangle){ sx, H - STATUSBAR_H - BTN_H - PADDING, sw, BTN_H },
                       "#113#  Quit  (Ctrl+Q)"))
            break;

        /* ── Main output panel ───────────────────────────────────── */
        int main_x = SIDEBAR_W + 1;
        int main_w = W - main_x;
        int main_h = H - STATUSBAR_H;

        /* Title bar */
        DrawRectangle(main_x, 0, main_w, 30, COL_STATUS_BG);
        {
            char title[128];
            if (st.dbg.running)
                snprintf(title, sizeof(title), "  Debugger (PID %d)", st.dbg.pid);
            else
                snprintf(title, sizeof(title), "  Output: %s", st.active_tool);
            DrawTextEx(mono, title, (Vector2){ main_x + 8, 6 }, 18, 1, COL_ACCENT_HOT);
        }

        if (st.dbg.running || (st.dbg.buf && st.dbg.buf_len > 0 && strcmp(st.active_tool, "Debugger") == 0)) {
            /* ── Debugger mode: show PTY output + command input ── */
            int input_area_h = DBG_INPUT_H + 8; /* input field + padding */
            int panel_top = 30;
            int panel_h = main_h - panel_top - input_area_h;

            const char *dbg_text = st.dbg.buf ? st.dbg.buf : "";
            int dbg_content_h = (st.dbg.line_count + 1) * LINE_HEIGHT;

            Rectangle panel_bounds = { main_x, panel_top, main_w, panel_h };
            Rectangle content_rect = { 0, 0, main_w - 20,
                                        dbg_content_h > panel_h ? dbg_content_h + 20 : panel_h };
            Rectangle view = { 0 };

            if (st.dbg.auto_scroll && dbg_content_h > panel_h) {
                st.dbg.scroll.y = -(dbg_content_h - panel_h + 20);
                st.dbg.auto_scroll = false;
            }

            GuiScrollPanel(panel_bounds, NULL, content_rect, &st.dbg.scroll, &view);

            BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);
            DrawRectangle((int)view.x, (int)view.y, (int)view.width, (int)view.height, COL_PANEL);
            draw_output(dbg_text, mono, view, st.dbg.scroll);
            EndScissorMode();

            /* ── Command input area at the bottom ── */
            int input_y = panel_top + panel_h + 2;
            DrawRectangle(main_x, input_y, main_w, input_area_h, COL_INPUT_BG);
            DrawLine(main_x, input_y, main_x + main_w, input_y, COL_SEPARATOR);

            /* Prompt label */
            const char *prompt = "dbg>";
            int prompt_w = 45;
            DrawTextEx(mono, prompt,
                       (Vector2){ main_x + 8, input_y + (input_area_h - OUTPUT_FONT_SIZE) / 2.0f },
                       OUTPUT_FONT_SIZE, 1, COL_PROMPT);

            /* Command text box */
            Rectangle cmd_rect = { main_x + prompt_w + 4, input_y + 4,
                                    main_w - prompt_w - 12, DBG_INPUT_H };
            if (GuiTextBox(cmd_rect, st.dbg.cmd, sizeof(st.dbg.cmd), st.dbg.cmd_edit))
                st.dbg.cmd_edit = !st.dbg.cmd_edit;

        } else {
            /* ── Normal output mode ── */
            Rectangle panel_bounds = { main_x, 30, main_w, main_h - 30 };
            Rectangle content_rect = { 0, 0, main_w - 20, st.content_h > 0 ? st.content_h + 20 : main_h - 30 };
            Rectangle view = { 0 };

            GuiScrollPanel(panel_bounds, NULL, content_rect, &st.scroll, &view);

            BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);
            DrawRectangle((int)view.x, (int)view.y, (int)view.width, (int)view.height, COL_PANEL);

            if (st.output && st.output[0]) {
                draw_output(st.output, mono, view, st.scroll);
            } else {
                DrawTextEx(mono, "No output yet. Open a file and run a tool.",
                           (Vector2){ view.x + 20, view.y + 20 }, OUTPUT_FONT_SIZE, 1, COL_TEXT_DIM);
                DrawTextEx(mono, "You can drag & drop a file onto this window.",
                           (Vector2){ view.x + 20, view.y + 50 }, OUTPUT_FONT_SIZE, 1, COL_TEXT_DIM);
            }
            EndScissorMode();
        }

        GuiStatusBar((Rectangle){ 0, H - STATUSBAR_H, W, STATUSBAR_H }, st.status);

        EndDrawing();
    }

    /* Cleanup */
    if (st.dbg.running) dbg_stop(&st);
    free(st.dbg.buf);
    free(st.output);
    if (st.file_open && st.target) baseer_close(st.target);
    if (mono_loaded) UnloadFont(mono);
    CloseWindow();

    return 0;
}
