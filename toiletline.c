/**
 *  toiletline 0.0.4
 *  Raw CLI shell implementation
 *  Meant to be a tiny replacement of GNU Readline :3
 *
 *  #define TOILETLINE_IMPLEMENTATION
 *  Before you include this file in C or C++ file to create the implementation.
 *
 *  Copyright (c) 2023 toiletbril
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TOILETLINE_H_
#define TOILETLINE_H_

#ifdef _WIN32
    #include <Windows.h>
    #include <conio.h>
    #include <fcntl.h>
    #include <io.h>
    #define STDIN_FILENO  _fileno(stdin)
    #define STDOUT_FILENO _fileno(stdout)
#else // Linux
    #include <termios.h>
    #include <unistd.h>
#endif // Linux

// for no asserts, define TL_ASSERT before including
#ifndef TL_ASSERT
    #include <assert.h>
    #define TL_ASSERT(boolval) assert(boolval)
#endif // TL_ASSERT

#include <ctype.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 *  Initialize toiletline, enter raw mode.
 */
int tl_init(void);
/**
 *  Exit toiletline and free history.
 */
int tl_exit(void);
/**
 *  Read one character without waiting for Enter.
 *  This does not append to history.
 *
 *  Returns:
 *     0 on success.
 *     1 when exited.
 *    -1 internal error.
 */
int tl_getc(char *buffer, size_t size, const char *prompt);
/**
 *  Read one line.
 *
 *  Returns:
 *     0 on success.
 *     1 when exited.
 *    -1 internal error.
 */
int tl_readline(char *line_buffer, size_t size, const char *prompt);

#endif // TOILETLINE_H_

#ifdef TOILETLINE_IMPLEMENTATION

// general debug messages
#ifdef TL_DEBUG
    #define ITL_DBG(message, val) \
        printf("%s: %d\n", message, val)
#else
    #define ITL_DBG(message, val)
#endif

inline static int itl_enter_raw_mode(void)
{
#ifdef _WIN32
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hInput == INVALID_HANDLE_VALUE)
        return 0;

    // NOTE:
    // ENABLE_VIRTUAL_TERMINAL_INPUT seems to not work on older versions of Windows
    DWORD mode = ENABLE_EXTENDED_FLAGS | ENABLE_VIRTUAL_TERMINAL_INPUT | ENABLE_QUICK_EDIT_MODE;

    if (!SetConsoleMode(hInput, mode))
        return 0;

    if (_setmode(STDIN_FILENO, _O_BINARY) == -1)
        return 0;

    if (SetConsoleCP(CP_UTF8) == 0)
        return 0;
#else // Linux
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) != 0)
        return 0;

    struct termios raw = term;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0)
        return 0;

#endif // Linux
    return 1;
}

inline static int itl_exit_raw_mode(void)
{
#ifdef _WIN32
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hInput == INVALID_HANDLE_VALUE)
        return 0;

    DWORD mode;
    if (!GetConsoleMode(hInput, &mode))
        return 0;

    mode |= ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT;
    if (!SetConsoleMode(hInput, mode))
        return 0;
#else // Linux
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) != 0)
        return 0;

    term.c_lflag |= ICANON | ECHO;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) != 0)
        return 0;
#endif // Linux
    return 1;
}

inline static void itl_handle_interrupt(int sign)
{
    signal(sign, SIG_IGN);
    fputc('\n', stdout);
    printf("Interrupted.\n");
    fflush(stdout);
    exit(0);
}

static int itl_global_alloc_count = 0;

// messages about allocations
#ifdef TL_DEBUG_ALLOC
    #define ITL_ALLOC_DBG(message) \
        printf("\n%s, alloc: %d\n", message, itl_global_alloc_count)
#else
    #define ITL_ALLOC_DBG(message)
#endif // TL_DEBUG_ALLOC

inline static void *itl_malloc(size_t size)
{
    itl_global_alloc_count += 1;
    ITL_ALLOC_DBG("malloc");
    return malloc(size);
}

inline static void *itl_calloc(size_t count, size_t size)
{
    itl_global_alloc_count += 1;
    ITL_ALLOC_DBG("calloc");
    return calloc(count, size);
}

inline static void *itl_realloc(void *block, size_t size)
{
    ITL_ALLOC_DBG("realloc");
    if (block == NULL)
        itl_global_alloc_count += 1;
    return realloc(block, size);
}

#define itl_free(ptr)                     \
    do {                                  \
        if (ptr != NULL) {                \
            itl_global_alloc_count -= 1;  \
            ITL_ALLOC_DBG("free");        \
            free(ptr);                    \
        }                                 \
    } while (0)

typedef struct itl_utf8_t itl_utf8_t;

#define itl_max(a, b)       \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b;      \
  })

struct itl_utf8_t
{
    uint8_t bytes[4];
    size_t size;
};

static itl_utf8_t itl_utf8_new(const uint8_t *bytes, uint8_t length)
{
    itl_utf8_t utf8_char;
    utf8_char.size = length;

    for (uint8_t i = 0; i < length; i++)
        utf8_char.bytes[i] = bytes[i];

    return utf8_char;
}

typedef struct itl_char_t itl_char_t;

// utf-8 string node
struct itl_char_t
{
    itl_utf8_t c;
    itl_char_t *next;
    itl_char_t *prev;
};

static itl_char_t *itl_char_alloc(void)
{
    itl_char_t *ptr = (itl_char_t *)itl_malloc(sizeof(itl_char_t));
    ptr->next = NULL;
    ptr->prev = NULL;
    return ptr;
}

static void itl_char_copy(itl_char_t *dst, itl_char_t *src)
{
    memcpy(dst, src, sizeof(itl_char_t));
}

static void itl_char_free(itl_char_t *c)
{
    itl_free(c);
}

typedef struct itl_string_t itl_string_t;

// linked list of utf-8 chars
struct itl_string_t
{
    itl_char_t *begin;
    itl_char_t *end;
    size_t length;
    size_t size;
};

static itl_string_t *itl_string_alloc(void)
{
    itl_string_t *ptr = (itl_string_t *)itl_malloc(sizeof(itl_string_t));

    ptr->begin = NULL;
    ptr->end = NULL;
    ptr->length = 0;
    ptr->size = 0;

    return ptr;
}

static void itl_string_copy(itl_string_t *dst, itl_string_t *src)
{
    itl_char_t *src_c = src->begin;
    itl_char_t *prev_new_c = NULL;
    itl_char_t *new_c;

    dst->begin = NULL;
    dst->end = NULL;

    while (src_c) {
        new_c = itl_char_alloc();
        itl_char_copy(new_c, src_c);

        if (prev_new_c) {
            prev_new_c->next = new_c;
            new_c->prev = prev_new_c;
        }
        else
            dst->begin = new_c;

        prev_new_c = new_c;
        src_c = src_c->next;
    }

    dst->end = prev_new_c;
    dst->length = src->length;
    dst->size = src->size;
}

// frees every character in a string, does not free the string itself
static void itl_string_clear(itl_string_t *str)
{
    itl_char_t *c = str->begin;
    itl_char_t *next;

    while (c) {
        next = c->next;
        itl_char_free(c);
        c = next;
    }

    str->begin = NULL;
    str->end = NULL;
    str->length = 0;
    str->size = 0;
}

inline static void itl_string_free(itl_string_t *str)
{
    itl_string_clear(str);
    itl_free(str);
}

static int itl_string_to_cstr(itl_string_t *str, char *c_str, size_t size)
{
    itl_char_t *c = str->begin;
    size_t i = 0;

    while (c && size - i > c->c.size) {
        for (size_t j = 0; j != c->c.size; ++j)
            c_str[i++] = c->c.bytes[j];

        if (c != c->next)
            c = c->next;
    }

    if (i < size)
        c_str[i] = '\0';
    else
        return 2;

    return 0;
}

static itl_string_t *lbuf = NULL;

// line editor
typedef struct
{
    struct itl_string_t *lbuf;
    itl_char_t *cur_char;
    size_t cur_pos;
    size_t cur_col;
    int h_item_sel;
    char *out;
    size_t out_size;
    const char *prompt;
} itl_le_t;

static itl_le_t itl_le_new(itl_string_t *intern_lbuf, char *out_buffer, size_t out_size, const char *prompt)
{
    itl_le_t le = {
        .lbuf = intern_lbuf,
        .cur_char = NULL,
        .cur_pos = 0,
        .cur_col = 0,
        .h_item_sel = -1,
        .out = out_buffer,
        .out_size = out_size,
        .prompt = prompt,
    };

    return le;
}

// removes and frees characters from cursor position
static int itl_le_erase(itl_le_t *le, size_t count, int behind)
{
    size_t i = 0;
    itl_char_t *to_free = NULL;

    while (i++ < count) {
        if (behind) {
            if (le->cur_pos == 0)
                return 0;
            else if (le->cur_char) {
                to_free = le->cur_char->prev;
                le->cur_pos -= 1;
            }
            else if (le->cur_pos == le->lbuf->length) {
                to_free = le->lbuf->end;
                le->lbuf->end = le->lbuf->end->prev;
                le->cur_pos -= 1;
            }
        }
        else {
            if (le->cur_pos == le->lbuf->length)
                return 0;
            else if (le->cur_char) {
                to_free = le->cur_char;
                le->cur_char = le->cur_char->next;
                if (le->cur_pos == le->lbuf->length - 1)
                    le->lbuf->end = le->lbuf->end->prev;
            }
        }

        if (le->cur_pos == 0)
            le->lbuf->begin = le->lbuf->begin->next;

        if (to_free->prev)
            to_free->prev->next = to_free->next;
        if (to_free->next)
            to_free->next->prev = to_free->prev;

        le->lbuf->size -= to_free->c.size;
        le->lbuf->length -= 1;

        itl_char_free(to_free);
    }

    return 1;
}

// inserts character at cursor position
static int itl_le_putc(itl_le_t *le, const itl_utf8_t ch)
{
    if (le->lbuf->size >= le->out_size - 1)
        return 0;

    if (le->out_size - 1 < le->lbuf->size + ch.size)
        return 0;

    itl_char_t *new_c = itl_char_alloc();
    new_c->c = ch;

    if (le->cur_pos == 0) {
        new_c->next = le->lbuf->begin;
        if (le->lbuf->begin)
            le->lbuf->begin->prev = new_c;
        le->lbuf->begin = new_c;
        if (le->lbuf->end == NULL)
            le->lbuf->end = new_c;
    }
    else if (le->cur_pos == le->lbuf->length) {
        if (le->lbuf->end)
            le->lbuf->end->next = new_c;
        new_c->prev = le->lbuf->end;
        le->lbuf->end = new_c;
    }
    else if (le->cur_char) {
        new_c->next = le->cur_char;
        if (le->cur_char->prev) {
            le->cur_char->prev->next = new_c;
            new_c->prev = le->cur_char->prev;
        }
        le->cur_char->prev = new_c;
    }

    le->lbuf->length += 1;
    le->lbuf->size += ch.size;
    le->cur_pos += 1;

    return 1;
}

static void itl_le_move_right(itl_le_t *le, size_t steps)
{
    size_t i = 0;
    while (i++ < steps) {
        if (le->cur_char) {
            le->cur_char = le->cur_char->next;
            le->cur_pos += 1;
        }
        else
            return;
    }
}

static void itl_le_move_left(itl_le_t *le, size_t steps)
{
    size_t i = 0;
    while (i++ < steps) {
        if (le->cur_pos == 0)
            return;
        else if (le->cur_char) {
            le->cur_char = le->cur_char->prev;
            le->cur_pos -= 1;
        }
        else if (le->cur_pos == le->lbuf->length) {
            le->cur_char = le->lbuf->end;
            le->cur_pos -= 1;
        }
        else
            return;
    }
}

#define itl_isdelim(c) (ispunct(c) || isspace(c))

static size_t itl_le_next_word(itl_le_t *le, int behind)
{
    itl_char_t *ch = le->cur_char;
    size_t steps = 1;

    if (ch)
        if (behind)
            ch = ch->prev;
        else
            ch = ch->next;
    else
        if (le->cur_pos == le->lbuf->size && behind)
            ch = le->lbuf->end;
        else
            return 0;

    while (ch) {
        if (!itl_isdelim(ch->c.bytes[0]))
            break;

        steps += 1;

        if (behind)
            ch = ch->prev;
        else
            ch = ch->next;
    }

    return steps;
}

static size_t itl_le_next_whitespace(itl_le_t *le, int behind)
{
    itl_char_t *ch = le->cur_char;
    size_t steps = 1;

    if (ch)
        if (behind)
            ch = ch->prev;
        else
            ch = ch->next;
    else
        if (le->cur_pos == le->lbuf->size && behind)
            ch = le->lbuf->end;
        else
            return 0;

    while (ch) {
        if (itl_isdelim(ch->c.bytes[0]))
            break;

        steps += 1;

        if (behind)
            ch = ch->prev;
        else
            ch = ch->next;
    }

    return steps;
}

// frees line buffer's string's characters, does not free the string itself
inline static void itl_le_clear(itl_le_t *le)
{
    itl_string_clear(le->lbuf);
    le->lbuf->length = 0;
    le->cur_pos = 0;
}

#ifdef _WIN32
    #define itl_read_byte() _getch()
#else // Linux
    #define itl_read_byte() fgetc(stdin)
#endif // Linux

// buffer output before writing it all to stdout
typedef struct
{
  char *string;
  int size;
} itl_pbuf_t;

static void itl_pbuf_append(itl_pbuf_t *pbuf, const char *s, int len)
{
    char *n_s = itl_realloc(pbuf->string, pbuf->size + len);
    if (n_s == NULL)
        return;

    memcpy(&n_s[pbuf->size], s, len);

    pbuf->string = n_s;
    pbuf->size += len;
}

inline static void itl_pbuf_free(itl_pbuf_t *pbuf)
{
    itl_free(pbuf->string);
}

int itl_tty_cur_pos(size_t *rows, size_t *cols) {
    char buf[32];
    size_t i = 0;

    fputs("\x1b[999C\x1b[999B", stdout);
    fputs("\x1b[6n", stdout);

    while (i < sizeof(buf) - 1) {
        buf[i] = itl_read_byte();
        if (buf[i] == 'R')
            break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[')
        return 1;
    if (sscanf(&buf[2], "%zu;%zu", rows, cols) != 2)
        return 1;

    return 0;
}

static int itl_get_window_size(size_t *rows, size_t *cols) {
    if (itl_tty_cur_pos(rows, cols))
        return 1;
    return 0;
}

static int itl_le_update_tty(itl_le_t *le)
{
    fputs("\x1b[?25l", stdout);

    size_t cstr_size = le->lbuf->length + 1;
    itl_pbuf_t pbuf = { .string = NULL, .size = 0 };
    size_t pr_len = strlen(le->prompt);
    size_t buf_size = itl_max(cstr_size, (size_t)8) * sizeof(char);

    size_t rows, cols;
    itl_get_window_size(&rows, &cols);

    size_t wrap_value = wrap_value = (le->lbuf->length + pr_len) / cols;
    size_t wrap_cursor_pos = le->cur_pos + pr_len - wrap_value * cols + 1;

    ITL_DBG("len", le->lbuf->length);
    ITL_DBG("cols", cols);
    ITL_DBG("wrap_value", wrap_value);
    ITL_DBG("wrap_cursor_pos", wrap_cursor_pos);

    char *buf = (char *)itl_malloc(buf_size);
    if (!buf)
        return 0;

    // TODO
    if (wrap_value > 0 && wrap_cursor_pos > 1) {
        snprintf(buf, buf_size, "\x1b[%zuF", wrap_value);
        itl_pbuf_append(&pbuf, buf, strlen(buf));
    }

    itl_pbuf_append(&pbuf, "\r", 1);
    itl_pbuf_append(&pbuf, "\x1b[0K", 4);

    itl_pbuf_append(&pbuf, le->prompt, pr_len);

    itl_string_to_cstr(le->lbuf, buf, buf_size);
    itl_pbuf_append(&pbuf, buf, cstr_size);

    snprintf(buf, buf_size, "\x1b[%zuG", wrap_cursor_pos);
    itl_pbuf_append(&pbuf, buf, strlen(buf));

    write(STDOUT_FILENO, pbuf.string, pbuf.size);

    itl_pbuf_free(&pbuf);
    itl_free(buf);

    fputs("\x1b[?25h", stdout);
    return fflush(stdout);
}

#define TL_HISTORY_INIT_SIZE 16
#define ITL_HISTORY_MAX_SIZE 128

static itl_string_t **itl_history = NULL;
static int itl_h_size = TL_HISTORY_INIT_SIZE;
static int itl_h_index = 0;

static void itl_history_alloc(void)
{
    itl_history = (itl_string_t **)
        itl_calloc(TL_HISTORY_INIT_SIZE, sizeof(itl_string_t *));
}

static void itl_history_free(void)
{
    for (int i = 0; i < itl_h_index; ++i)
        itl_string_free(itl_history[i]);
    itl_free(itl_history);
}

// copies string to global history
// allocates memory for a new string
// allocates memory for history array if needed by multiplying it's size by 2
static int itl_history_append(itl_string_t *str)
{
    if (str->length <= 0)
        return 0;

    if (itl_h_size >= ITL_HISTORY_MAX_SIZE)
        return 0;

    if (itl_h_index >= itl_h_size && itl_h_size < ITL_HISTORY_MAX_SIZE) {
        itl_h_size *= 2;
        itl_history = (itl_string_t **)
            itl_realloc(itl_history, itl_h_size * sizeof(itl_string_t *));
    }

    itl_string_t *new_str = itl_string_alloc();
    itl_string_copy(new_str, str);

    itl_history[itl_h_index++] = new_str;

    return 1;
}

// copies string from history to line editor
// does not free anything
static void itl_history_get(itl_le_t *le)
{
    if (le->h_item_sel >= itl_h_index)
        return;

    itl_string_t *h_entry = itl_history[le->h_item_sel];
    itl_string_copy(lbuf, h_entry);
    le->cur_pos = lbuf->length;
}

// allocates memory for global history and line buffer
// adds signal handle for c^c
int tl_init(void)
{
    itl_history_alloc();
    lbuf = itl_string_alloc();

    signal(SIGINT, itl_handle_interrupt);
    return itl_enter_raw_mode();
}

// frees memory for global history and line buffer
// removes signal handle for c^c
int tl_exit(void)
{
    itl_history_free();
    itl_string_free(lbuf);

    signal(SIGINT, SIG_DFL);
    ITL_DBG("exit, alloc count", itl_global_alloc_count);
    TL_ASSERT(itl_global_alloc_count == 0);

    return itl_exit_raw_mode();
}

typedef enum
{
    ITL_KEY_CHAR = 0,
    ITL_KEY_UNKN,
    ITL_KEY_UP,
    ITL_KEY_DOWN,
    ITL_KEY_RIGHT,
    ITL_KEY_LEFT,
    ITL_KEY_END,
    ITL_KEY_HOME,
    ITL_KEY_ENTER,
    ITL_KEY_BACKSPACE,
    ITL_KEY_DELETE,
    ITL_KEY_TAB,
    ITL_KEY_ESC,
    ITL_KEY_INTERRUPT,
} ITL_KEY_KIND;

#define ITL_CTRL_BIT  32
#define ITL_SHIFT_BIT 64
#define ITL_ALT_BIT   128

#define ITL_KEY_MASK  15
#define ITL_MOD_MASK  224

static int itl_parse_esc(int byte)
{
    ITL_KEY_KIND event = 0;

    switch (byte) { // plain bytes
        case 3:
            return ITL_KEY_INTERRUPT;
        case 9:
            return ITL_KEY_TAB;
        case 10: // newline
        case 13: // cr
            return ITL_KEY_ENTER;
        case 23:
            return ITL_KEY_BACKSPACE | ITL_CTRL_BIT;
        case 8:
        case 127:
            return ITL_KEY_BACKSPACE;
    }

#ifdef _WIN32
    if (byte == 224) { // escape
        switch (itl_read_byte()) {
            case 72: {
                event = ITL_KEY_UP;
            } break;

            case 75: {
                event = ITL_KEY_LEFT;
            } break;

            case 77: {
                event = ITL_KEY_RIGHT;
            } break;

            case 71: {
                event = ITL_KEY_HOME;
            } break;

            case 79: {
                event = ITL_KEY_END;
            } break;

            case 80: {
                event = ITL_KEY_DOWN;
            } break;

            case 83: {
                event = ITL_KEY_DELETE;
            } break;

            case 115: {
                event = ITL_KEY_LEFT | ITL_CTRL_BIT;
            } break;

            case 116: {
                event = ITL_KEY_RIGHT | ITL_CTRL_BIT;
            } break;

            case 147: { // ctrl del
                event = ITL_KEY_DELETE | ITL_CTRL_BIT;
            } break;

            default:
                event = ITL_KEY_UNKN;
        }
    } else {
        return ITL_KEY_CHAR;
    }
#else // Linux
    int read_mod = 0;

    if (byte == 27) { // \x1b
        byte = itl_read_byte();

        if (byte != 91) { // [
            return ITL_KEY_CHAR | ITL_ALT_BIT;
        }

        byte = itl_read_byte();

        if (byte == 49) {
            if (itl_read_byte() != 59) // ;
                return ITL_KEY_UNKN;

            switch (itl_read_byte()) {
                case 50: {
                    event |= ITL_SHIFT_BIT;
                } break;

                case 53: {
                    event |= ITL_CTRL_BIT;
                } break;
            }

            read_mod = 1;
            byte = itl_read_byte();
        }

        switch (byte) { // escape codes based on xterm
            case 51: {
                event |= ITL_KEY_DELETE;
            } break;

            case 65: {
                event |= ITL_KEY_UP;
                return event;
            } break;

            case 66: {
                event |= ITL_KEY_DOWN;
                return event;
            } break;

            case 67: {
                event |= ITL_KEY_RIGHT;
                return event;
            } break;

            case 68: {
                event |= ITL_KEY_LEFT;
                return event;
            } break;

            case 70: {
                event |= ITL_KEY_END;
                return event;
            } break;

            case 72: {
                event |= ITL_KEY_HOME;
                return event;
            } break;

            default:
                event |= ITL_KEY_UNKN;
        }
    } else
        return ITL_KEY_CHAR;

    if (!read_mod) {
        byte = itl_read_byte();

        if (byte == 59) { // ;
            switch (itl_read_byte()) {
                case 53: {
                    event |= ITL_CTRL_BIT;
                } break;

                case 51: {
                    event |= ITL_SHIFT_BIT;
                } break;
            }

            byte = itl_read_byte();
        }

        if (byte != 126) // ~
            event = ITL_KEY_UNKN;
    }
#endif // Linux
    return event;
}

static itl_utf8_t itl_parse_utf8(int byte)
{
    uint8_t bytes[4] = {byte, 0, 0, 0};
    uint8_t len = 0;

    if ((byte & 0x80) == 0) // 1 byte
        len = 1;
    else if ((byte & 0xE0) == 0xC0) // 2 byte
        len = 2;
    else if ((byte & 0xF8) == 0xF0) // 3 byte
        len = 3;
    else if ((byte & 0xF8) == 0xF0) // 4 byte
        len = 4;

    for (uint8_t i = 1; i < len; ++i) // consequent bytes
        bytes[i] = itl_read_byte();

#ifdef TL_DEBUG
    printf("\nutf8 char bytes: '");
    for (uint8_t i = 0; i < len - 1; ++i)
        printf("%02X ", bytes[i]);
    printf("%02X'\n", bytes[len - 1]);
#endif

    return itl_utf8_new(bytes, len);
}

//  0 on enter
//  1 on interrupt
// -1 if should continue
static int itl_handle_esc(itl_le_t *le, int esc)
{
    switch (esc & ITL_KEY_MASK) {
        case ITL_KEY_UP: {
            if (le->h_item_sel == -1) {
                le->h_item_sel = itl_h_index;
                if (le->lbuf->length > 0 && itl_h_index > 0) {
                    itl_history_append(le->lbuf);
                }
            }

            if (le->h_item_sel > 0) {
                le->h_item_sel -= 1;
                itl_le_clear(le);
                itl_history_get(le);
            }
        } break;

        case ITL_KEY_DOWN: {
            if (le->h_item_sel < itl_h_index - 1 && le->h_item_sel >= 0) {
                le->h_item_sel += 1;
                itl_le_clear(le);
                itl_history_get(le);
            }
            else if (itl_h_index > 0) {
                itl_le_clear(le);
                le->h_item_sel = -1;
            }
        } break;

        case ITL_KEY_RIGHT: {
            if (le->cur_pos < le->lbuf->length) {
                size_t count = 1;

                if (esc & ITL_CTRL_BIT) {
                    size_t next_ws = itl_le_next_whitespace(le, 0);

                    if (next_ws <= 1) {
                        count = itl_le_next_word(le, 0);
                    }
                    else
                        count = next_ws;
                }

                itl_le_move_right(le, count);
            }
        } break;

        case ITL_KEY_LEFT: {
            if (le->cur_pos > 0 && le->cur_pos <= le->lbuf->length) {
                size_t count = 1;

                if (esc & ITL_CTRL_BIT) {
                    size_t next_ws = itl_le_next_whitespace(le, 1);

                    if (next_ws <= 1) {
                        count = itl_le_next_word(le, 1);
                        itl_le_move_left(le, count);

                        count = itl_le_next_whitespace(le, 1) - 1;
                    }
                    else
                        count = next_ws - 1;
                }

                itl_le_move_left(le, count);
            }
        } break;

        case ITL_KEY_END: {
            itl_le_move_right(le, le->lbuf->length - le->cur_pos);
        } break;

        case ITL_KEY_HOME: {
            itl_le_move_left(le, le->cur_pos);
        } break;

        case ITL_KEY_ENTER: {
            int code = itl_string_to_cstr(le->lbuf, le->out, le->out_size);
            itl_history_append(le->lbuf);

            itl_le_clear(le);

            if (code == 0)
                fputc('\n', stdout);

            return code;
        } break;

        case ITL_KEY_BACKSPACE: {
            size_t count = 1;

            if (esc & ITL_CTRL_BIT) {
                size_t next_ws = itl_le_next_whitespace(le, 1);

                if (next_ws <= 1) {
                    count = itl_le_next_word(le, 1);
                    itl_le_erase(le, count, 1);
                    count = itl_le_next_whitespace(le, 1) - 1;
                }
                else
                    count = next_ws - 1;
            }

            itl_le_erase(le, count, 1);
        } break;

        case ITL_KEY_DELETE: {
            size_t count = 1;

            if (esc & ITL_CTRL_BIT) {
                size_t next_ws = itl_le_next_whitespace(le, 0);

                if (next_ws <= 1) {
                    count = itl_le_next_word(le, 0);
                    itl_le_erase(le, count, 0);
                    count = itl_le_next_whitespace(le, 0);
                }
                else
                    count = next_ws;
            }

            itl_le_erase(le, count, 0);
        } break;

        case ITL_KEY_INTERRUPT: {
            itl_string_to_cstr(le->lbuf, le->out, le->out_size);
            return 1;
        } break;

        default: {
            ITL_DBG("key wasn't handled", esc | ITL_KEY_MASK);
            ITL_DBG("modifier", esc | ITL_MOD_MASK);
        }
    }

    return -1;
}

int tl_getc(char *buffer, size_t size, const char *prompt)
{
    itl_le_t le = itl_le_new(lbuf, buffer, size, prompt);

    int esc;
    int in = itl_read_byte();

    if ((esc = itl_parse_esc(in)) != ITL_KEY_CHAR) {
            if ((esc = itl_handle_esc(&le, esc)) != -1) {
                return esc;
            }
        }
    else {
        itl_utf8_t ch = itl_parse_utf8(in);
        itl_le_putc(&le, ch);
        itl_string_to_cstr(le.lbuf, buffer, size);
        itl_le_clear(&le);
    }

    return 0;
}

//  0 on success
//  1 on interrupt
// -1 internal error (reserved)
int tl_readline(char *line_buffer, size_t size, const char *prompt)
{
    itl_le_t le = itl_le_new(lbuf, line_buffer, size, prompt);

    itl_le_update_tty(&le);

    int esc;
    int in;

    while (1) {
        in = itl_read_byte();

#ifdef TL_SEE_BYTES
        printf("%d\n", in);
        continue;
#endif

        if ((esc = itl_parse_esc(in)) != ITL_KEY_CHAR) {
            if ((esc = itl_handle_esc(&le, esc)) != -1) {
                return esc;
            }
        }
        else {
            itl_utf8_t ch = itl_parse_utf8(in);
            itl_le_putc(&le, ch);
        }

        itl_le_update_tty(&le);
    }

    return -1;
}

#endif // TOILETLINE_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

/**
 * TODO:
 *  - option to disable/enable C^C
 *  - test this on old Windows
 *  - wrapper around itl_string_to_cstr which also inserts \n\r when exceeding column width
 *  - replace history instead of ignoring it on limit
 *  - autocompletion
 *  - tty struct refactor
 */
