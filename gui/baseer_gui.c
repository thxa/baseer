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

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "../baseer.h"
#include "../modules/binhead/bx_binhead.h"
#include "../modules/b_hashmap/b_hashmap.h"

#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#define INIT_WIDTH       1280
#define INIT_HEIGHT      720
#define SIDEBAR_W        270
#define STATUSBAR_H      32
#define PADDING          10
#define BTN_H            32
#define INPUT_H          28
#define OUTPUT_FONT_SIZE 16
#define LINE_HEIGHT      (OUTPUT_FONT_SIZE + 4)
#define MAX_OUTPUT       (4 * 1024 * 1024)

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

static const char *MSG_NO_FILE = "[!] No file open. Open a file first.";
static const char *TOOL_NONE   = "None";

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

    baseer_execute(st->target, bx_binhead, &input);

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

static void launch_debugger(AppState *st)
{
    if (!st->file_open) {
        snprintf(st->status, sizeof(st->status), "%s", MSG_NO_FILE);
        return;
    }

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "baseer %s -d", st->filepath);

    pid_t pid = fork();
    if (pid == 0) {
        execlp("x-terminal-emulator", "x-terminal-emulator", "-e", "sh", "-c", cmd, NULL);
        execlp("alacritty", "alacritty", "-e", "sh", "-c", cmd, NULL);
        execlp("kitty", "kitty", "sh", "-c", cmd, NULL);
        execlp("foot", "foot", "sh", "-c", cmd, NULL);
        execlp("gnome-terminal", "gnome-terminal", "--", "sh", "-c", cmd, NULL);
        execlp("konsole", "konsole", "-e", "sh", "-c", cmd, NULL);
        execlp("xterm", "xterm", "-e", "sh", "-c", cmd, NULL);
        _exit(1);
    } else if (pid > 0) {
        snprintf(st->status, sizeof(st->status), "Debugger launched in terminal (PID %d)", pid);
        set_output(st, strdup(
            "=== Debugger ===\n\n"
            "The debugger has been launched in an external terminal.\n"
            "It uses ptrace and requires interactive input.\n\n"
            "Debugger commands:\n"
            "  bp <addr>     - Set breakpoint\n"
            "  dp <index>    - Delete breakpoint\n"
            "  lp            - List breakpoints\n"
            "  c             - Continue\n"
            "  si            - Step into\n"
            "  so            - Step over\n"
            "  x <addr> <n>  - Examine memory\n"
            "  set <target>  - Set memory/register\n"
            "  vmmap         - Show memory map\n"
            "  i             - Info\n"
            "  h             - Help\n"
            "  q             - Quit\n\n"
            "If no terminal was found, run manually:\n"
            "  baseer <file> -d\n"
        ), "Debugger");
    } else {
        snprintf(st->status, sizeof(st->status), "[!] Failed to fork for debugger.");
    }
}

static void clear_output(AppState *st)
{
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
    /* Auto-reap child processes (debugger terminals) */
    signal(SIGCHLD, SIG_IGN);

    AppState st = { 0 };
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
    static const char *font_paths[] = {
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
            mono = LoadFontEx(font_paths[i], OUTPUT_FONT_SIZE, NULL, 256);
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

        if (IsFileDropped()) {
            FilePathList files = LoadDroppedFiles();
            if (files.count > 0) {
                if (st.file_open) close_file(&st);
                strncpy(st.filepath, files.paths[0], sizeof(st.filepath) - 1);
                open_file(&st);
            }
            UnloadDroppedFiles(files);
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

        if (GuiButton((Rectangle){ sx, sy, sw, BTN_H }, "#152#  Debugger     (Ctrl+D)"))
            launch_debugger(&st);
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
                "  Ctrl+D   Launch Debugger (in terminal)\n"
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

        int main_x = SIDEBAR_W + 1;
        int main_w = W - main_x;
        int main_h = H - STATUSBAR_H;

        DrawRectangle(main_x, 0, main_w, 30, COL_STATUS_BG);
        {
            char title[128];
            snprintf(title, sizeof(title), "  Output: %s", st.active_tool);
            DrawTextEx(mono, title, (Vector2){ main_x + 8, 6 }, 18, 1, COL_ACCENT_HOT);
        }

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

        GuiStatusBar((Rectangle){ 0, H - STATUSBAR_H, W, STATUSBAR_H }, st.status);

        EndDrawing();
    }

    free(st.output);
    if (st.file_open && st.target) baseer_close(st.target);
    if (mono_loaded) UnloadFont(mono);
    CloseWindow();

    return 0;
}
