/**
 * toiletline 0.0.1
 * Raw CLI shell implementation, meant to be a tiny replacement of GNU Readline :3
 *
 * Does NOT support multi-byte characters YET.
 * Does NOT work on Windows YET.
 *
 * #define TOILETLINE_IMPL
 * Before you include this file in C or C++ file to create the implementation.
 */

#ifndef TOILETLINE_H_
#define TOILETLINE_H_

#ifdef _WIN32
    #include <Windows.h>
    #include <fcntl.h>
    #include <conio.h>
    #include <io.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Initialize toiletline, enter raw mode.
*/
bool tl_init();
/**
 * Exit toiletline and free history.
*/
bool tl_exit();
/**
 * Read one line.
 * Returns:
 *  0 on success;
 * -1 exited;
 * -2 line_buffer size exceeded;
 */
int tl_readline(char *line_buffer, size_t size);

#endif // TOILETLINE_H_

#ifdef TOILETLINE_IMPL

static int itl_fputsn(char *array, size_t size)
{
    int code;
    for (size_t i = 0; i < size; ++i) {
        if ((code = fputc(array[i], stdout)) < 0) {
            return code;
        };
    }
    return 0;
}

static bool itl_enter_raw_mode()
{
#ifdef _WIN32
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hInput == INVALID_HANDLE_VALUE)
        return false;

    if (!SetConsoleMode(hInput, 0))
        return false;

    _setmode(_fileno(stdin), _O_BINARY);

    return true;
#else
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) != 0)
        return false;

    struct termios raw = term;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0)
        return false;

    return true;
#endif
}

static bool itl_exit_raw_mode()
{
#ifdef _WIN32
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hInput == INVALID_HANDLE_VALUE)
        return false;

    DWORD mode;
    if (!GetConsoleMode(hInput, &mode))
        return false;

    mode |= ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT;
    if (!SetConsoleMode(hInput, mode))
        return false;
#else
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) != 0)
        return false;

    term.c_lflag |= ICANON | ECHO;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) != 0)
        return false;
#endif
    return true;
}

static void itl_handle_interrupt(int sign)
{
    signal(sign, SIG_IGN);
    fputc('\n', stdout);
    printf("Interrupted.\n");
    fflush(stdout);
    exit(0);
}

struct itl_le
{
    char *char_buffer;
    const size_t cbuf_size;
    char *line_buffer;
    const size_t lbuf_size;
    size_t lbuf_pos;
    size_t cursor_pos;
    int h_item_sel;
};

struct itl_le itl_le_new(char *char_buffer, size_t cbuf_size, char *line_buffer, size_t lbuf_size)
{
    memset(line_buffer, 0, lbuf_size);
    memset(char_buffer, 0, cbuf_size);

    struct itl_le le = {
        .char_buffer = char_buffer,
        .cbuf_size   = cbuf_size,
        .line_buffer = line_buffer,
        .lbuf_size   = lbuf_size,
        .lbuf_pos    = 0,
        .cursor_pos  = 0,
        .h_item_sel  = -1,
    };

    return le;
}

static bool itl_le_unputc(struct itl_le *le)
{
    if (le->cursor_pos > le->lbuf_pos || le->cursor_pos == 0) {
        return false;
    }

    if (le->cursor_pos != le->lbuf_pos) {
        for (int i = le->cursor_pos - 1; i < le->lbuf_pos; ++i) {
            le->line_buffer[i] = le->line_buffer[i + 1];
        }
    }
    else {
        le->line_buffer[le->cursor_pos - 1] = 0;
    }

    le->cursor_pos -= 1;
    le->lbuf_pos -= 1;

    return true;
}

static bool itl_le_putc(struct itl_le *le, char c)
{
    if (le->cursor_pos > le->lbuf_pos) {
        return false;
    }

    if (le->cursor_pos != le->lbuf_pos) {
        for (int i = le->cursor_pos; i < le->lbuf_pos - 1; ++i) {
            le->line_buffer[i + i] = le->line_buffer[i];
        }
    }

    le->line_buffer[le->cursor_pos] = c;

    le->cursor_pos += 1;
    le->lbuf_pos += 1;

    return true;
}

static bool itl_le_puts(struct itl_le *le, const char *str, size_t size)
{
    if (le->cursor_pos > le->lbuf_pos) {
        return false;
    }

    if (size >= le->lbuf_size) {
        return false;
    }

    if (le->cursor_pos != le->lbuf_pos) {
        for (int i = le->cursor_pos; i < le->lbuf_pos; ++i) {
            le->line_buffer[i + size] = le->line_buffer[i];
        }
    }

    memcpy(le->line_buffer + le->lbuf_pos, str, size);

    le->cursor_pos += size;
    le->lbuf_pos += size;

    return true;
}

int itl_le_put(struct itl_le *le)
{
    fputs("\x1b[2K", stdout);
    fputc('\r', stdout);
    itl_fputsn(le->line_buffer, le->lbuf_pos);
    printf("\x1b[%zuG", le->cursor_pos + 1);

    return fflush(stdout);
}

static void itl_le_debug(struct itl_le *le)
{
    printf("\nci=%zu, li=%zu\n", le->cursor_pos, le->lbuf_pos);
    for (size_t i = 0; i < le->lbuf_size; ++i) {
        printf("%c|", le->line_buffer[i]);
    }
    printf("\n");
}

static void itl_le_clear(struct itl_le *le)
{
    memset(le->line_buffer, 0, le->lbuf_size);
    le->lbuf_pos   = 0;
    le->cursor_pos = 0;
}

#define TL_INIT_HISTORY_SIZE 16
static char **itl_history = NULL;
static int itl_h_size     = 0;
static int itl_h_index    = 0;

static void itl_history_append(char *buf, size_t size)
{
    if (size <= 0) {
        return;
    }

    if (itl_h_index >= itl_h_size) {
        itl_h_size *= 2;
        itl_history = (char **)realloc(itl_history, itl_h_size * sizeof(char *));
    }

    char *str = (char *)calloc(size + 1, sizeof(char));
    memcpy(str, buf, size);
    str[size] = '\0';

    itl_history[itl_h_index++] = str;
}

static void itl_history_free()
{
    for (size_t i = 0; i < itl_h_index; ++i) {
        free(itl_history[i]);
    }
    free(itl_history);
}

static void itl_history_get(struct itl_le *le)
{
    if (le->h_item_sel >= itl_h_index) {
        return;
    }

    char *item        = itl_history[le->h_item_sel];
    size_t cursor_pos = strlen(item);

    itl_le_clear(le);
    memcpy(le->line_buffer, item, cursor_pos);

    le->lbuf_pos   = cursor_pos;
    le->cursor_pos = cursor_pos;
}

bool tl_init()
{
    itl_history = (char **)calloc(TL_INIT_HISTORY_SIZE, sizeof(char **));
    itl_h_size  = TL_INIT_HISTORY_SIZE;
    signal(SIGINT, itl_handle_interrupt);
    return itl_enter_raw_mode();
}

bool tl_exit()
{
    itl_history_free();
    signal(SIGINT, SIG_DFL);
    return itl_exit_raw_mode();
}

#define CHAR_BUF_LEN 4

// Return values:
// 0 on success
// -1 exited;
// -2 line_buffer size exceeded;
int tl_readline(char *line_buffer, size_t size)
{
    char char_buf[CHAR_BUF_LEN + 1] = {0};
    struct itl_le le            = itl_le_new(char_buf, CHAR_BUF_LEN, line_buffer, size);

    while (true) {
        int len                = read(0, char_buf, CHAR_BUF_LEN);
        char_buf[CHAR_BUF_LEN] = '\0';

        if (len == 1) {
            if (iscntrl(char_buf[0])) {
                switch (char_buf[0]) {
                    // c^c
                    case 3: {
                        return -1;
                    } break;

                    // newline
                    case 10:
                    // cr
                    case 13: {
                        itl_history_append(le.line_buffer, le.lbuf_pos);
                        return 0;
                    } break;

                    // backspace
                    case 127: {
                        itl_le_unputc(&le);
                    } break;

                    default: {
                        printf("%d", char_buf[0]);
                    }
                }
            }
            else {
                if (le.lbuf_pos >= le.lbuf_size) {
                    return -2;
                }
                itl_le_putc(&le, char_buf[0]);
            }
        }
        else {
            if (len == 3 && char_buf[0] == '\x1b') {
                // Right
                if (char_buf[2] == 67) {
                    if (le.cursor_pos >= 0 && le.cursor_pos < le.lbuf_pos) {
                        le.cursor_pos += 1;
                        fputs(char_buf, stdout);
                    }
                }

                // Left
                if (char_buf[2] == 68) {
                    if (le.cursor_pos > 0 && le.cursor_pos <= le.lbuf_pos) {
                        le.cursor_pos -= 1;
                        fputs(char_buf, stdout);
                    }
                }

                // Up, history back
                if (char_buf[2] == 65) {
                    if (le.h_item_sel == -1) {
                        le.h_item_sel = itl_h_index;
                        if (le.lbuf_pos > 0 && itl_h_index > 0) {
                            itl_history_append(le.line_buffer, le.lbuf_pos);
                        }
                    }

                    if (le.h_item_sel > 0) {
                        le.h_item_sel -= 1;
                        itl_history_get(&le);
                    }
                }

                // Down, history forward
                if (char_buf[2] == 66) {
                    if (le.h_item_sel < itl_h_index - 1 && le.h_item_sel >= 0) {
                        le.h_item_sel += 1;
                        itl_history_get(&le);
                    }
                    else {
                        itl_le_clear(&le);
                        le.h_item_sel = -1;
                    }
                }
            }
        }
        itl_le_put(&le);
    }

    return -1;
}

#endif // TOILETLINE_IMPLEMENTATION

// TODO:
// - Char-size agnostic LineEditor
// - Autocompletion