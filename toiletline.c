/**
 *  toiletline 0.0.1
 *  Raw CLI shell implementation
 *  Meant to be a tiny replacement of GNU Readline :3
 *
 *  #define TOILETLINE_IMPL
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

#ifndef TOILETLINE_H_
#define TOILETLINE_H_

#ifdef _WIN32
    #include <Windows.h>
    #include <conio.h>
    #include <fcntl.h>
    #include <io.h>
    #define STDIN_FILENO 0
    #define STDOUT_FILENO 1
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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 *  Initialize toiletline, enter raw mode.
 */
bool tl_init(void);
/**
 *  Exit toiletline and free history.
 */
bool tl_exit(void);
/**
 *  Read one line.
 *  Returns:
 *      0 on success.
 *      1 when exited.
 *      2 internal error.
 */
int tl_readline(char *line_buffer, size_t size);

#endif // TOILETLINE_H_

#ifdef TOILETLINE_IMPL

// general debug messages
#ifdef TL_DEBUG
    #define ITL_DBG(message, val) \
        printf("%s: %d\n", message, val)
#else
    #define ITL_DBG(message, val)
#endif

static bool itl_enter_raw_mode(void)
{
#ifdef _WIN32
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hInput == INVALID_HANDLE_VALUE)
        return false;

    // NOTE:
    // ENABLE_VIRTUAL_TERMINAL_INPUT seems to not work on older versions of Windows
    DWORD mode = ENABLE_EXTENDED_FLAGS | ENABLE_VIRTUAL_TERMINAL_INPUT | ENABLE_QUICK_EDIT_MODE;

    if (!SetConsoleMode(hInput, mode))
        return false;

    _setmode(_fileno(stdin), _O_BINARY);
    SetConsoleCP(CP_UTF8);

    return true;
#else // Linux
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) != 0)
        return false;

    struct termios raw = term;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0)
        return false;

    return true;
#endif // Linux
}

static bool itl_exit_raw_mode(void)
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
#else // Linux
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) != 0)
        return false;

    term.c_lflag |= ICANON | ECHO;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) != 0)
        return false;
#endif // Linux
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
    size_t size;
};

static itl_string_t *itl_string_alloc(void)
{
    itl_string_t *ptr = (itl_string_t *)itl_malloc(sizeof(itl_string_t));
    ptr->size = 0;
    ptr->begin = NULL;
    ptr->end = NULL;
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
        else {
            dst->begin = new_c;
        }

        prev_new_c = new_c;
        src_c = src_c->next;
    }

    dst->end = prev_new_c;
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

    str->size = 0;
    str->begin = NULL;
    str->end = NULL;
}

static void itl_string_free(itl_string_t *str)
{
    itl_string_clear(str);
    itl_free(str);
}

static void itl_string_to_cstr(itl_string_t *str, char *cstr, size_t size)
{
    itl_char_t *c = str->begin;
    size_t i = 0;

    while (c) {
        for (size_t j = 0; j != c->c.size && i < size; ++j) {
            cstr[i++] = c->c.bytes[j];
        }
        if (c != c->next)
            c = c->next;
    }

    if (i < size)
        cstr[i] = '\0';
}

static itl_string_t *lbuf = NULL;

// line editor
typedef struct
{
    struct itl_string_t *lbuf;
    itl_char_t *cur_char;
    size_t cur_pos;
    int h_item_sel;
    char *out;
    size_t out_size;
} itl_le;

static itl_le itl_le_new(itl_string_t *intern_lbuf, char *out_buffer, size_t out_size)
{
    itl_le le = {
        .lbuf = intern_lbuf,
        .cur_char = NULL,
        .cur_pos = 0,
        .h_item_sel = -1,
        .out = out_buffer,
        .out_size = out_size,
    };

    return le;
}

// removes and frees characters at cursor position
static void itl_le_unputc(itl_le *le, size_t count, bool behind)
{
    size_t i = 0;
    itl_char_t *to_free = NULL;

    while (i++ < count) {
        if (behind) {
            if (le->cur_pos == 0)
                return;
            else if (le->cur_char) {
                to_free = le->cur_char->prev;
                le->cur_pos -= 1;
            }
            else if (le->cur_pos == le->lbuf->size) {
                to_free = le->lbuf->end;
                le->lbuf->end = le->lbuf->end->prev;
                le->cur_pos -= 1;
            }
        }
        else {
            if (le->cur_pos == le->lbuf->size)
                return;
            else if (le->cur_char) {
                to_free = le->cur_char;
                le->cur_char = le->cur_char->next;
                if (le->cur_pos == le->lbuf->size - 1)
                    le->lbuf->end = le->lbuf->end->prev;
            }
        }

        if (le->cur_pos == 0)
            le->lbuf->begin = le->lbuf->begin->next;

        if (to_free->prev)
            to_free->prev->next = to_free->next;
        if (to_free->next)
            to_free->next->prev = to_free->prev;

        itl_char_free(to_free);
        le->lbuf->size -= 1;
    }
}

// inserts character at cursor position
static void itl_le_putc(itl_le *le, const itl_utf8_t ch)
{
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
    else if (le->cur_pos == le->lbuf->size) {
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

    le->lbuf->size += 1;
    le->cur_pos += 1;
}

static void itl_le_right(itl_le *le, size_t steps)
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

static void itl_le_left(itl_le *le, size_t steps)
{
    size_t i = 0;
    while (i++ < steps) {
        if (le->cur_char) {
            le->cur_char = le->cur_char->prev;
            le->cur_pos -= 1;
        }
        else if (le->cur_pos == le->lbuf->size) {
            le->cur_char = le->lbuf->end;
            le->cur_pos -= 1;
        }
        else
            return;
    }
}

// frees line buffer's string's characters, does not free the string itself
static void itl_le_clear(itl_le *le)
{
    itl_string_clear(le->lbuf);
    le->lbuf->size = 0;
    le->cur_pos = 0;
}

// buffer output before writing it all to stdout
typedef struct
{
  char *string;
  int length;
} itl_pbuf;

static void itl_pbuf_append(itl_pbuf *pbuf, const char *s, int len)
{
    char *new = itl_realloc(pbuf->string, pbuf->length + len);
    if (new == NULL)
        return;

    memcpy(&new[pbuf->length], s, len);

    pbuf->string = new;
    pbuf->length += len;
}

static void itl_pbuf_free(itl_pbuf *pbuf)
{
    itl_free(pbuf->string);
    // NOTE: realloc does not increase global count
    itl_global_alloc_count += 1;
}

static int itl_tty_update(itl_le *le)
{
    itl_pbuf pbuf = {NULL, 0};
    char buf[256];

    itl_pbuf_append(&pbuf, "\x1b[?25l", 6);

    itl_pbuf_append(&pbuf, "\r", 1);
    itl_pbuf_append(&pbuf, "\x1b[0K", 4);
    itl_string_to_cstr(le->lbuf, buf, 256);
    itl_pbuf_append(&pbuf, buf, strlen(buf));

    snprintf(buf, sizeof(buf), "\x1b[%zuG", le->cur_pos + 1);
    itl_pbuf_append(&pbuf, buf, strlen(buf));

    itl_pbuf_append(&pbuf, "\x1b[?25h", 6);

    write(STDOUT_FILENO, pbuf.string, pbuf.length);

    itl_pbuf_free(&pbuf);

    return fflush(stdout);
}

#define TL_HISTORY_INIT_SIZE 16

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
static void itl_history_append(itl_string_t *str)
{
    if (str->size <= 0)
        return;

    if (itl_h_index >= itl_h_size) {
        itl_h_size *= 2;
        itl_history = (itl_string_t **)
            itl_realloc(itl_history, itl_h_size * sizeof(itl_string_t *));
    }

    itl_string_t *new_str = itl_string_alloc();
    itl_string_copy(new_str, str);

    itl_history[itl_h_index++] = new_str;
}

// copies string from history to line editor
// does not free anything
static void itl_history_get(itl_le *le)
{
    if (le->h_item_sel >= itl_h_index)
        return;

    itl_string_t *h_entry = itl_history[le->h_item_sel];
    itl_string_copy(lbuf, h_entry);
    le->cur_pos = lbuf->size;
}

// allocates memory for global history and line buffer
// adds signal handle for c^c
bool tl_init(void)
{
    itl_history_alloc();
    lbuf = itl_string_alloc();

    signal(SIGINT, itl_handle_interrupt);
    return itl_enter_raw_mode();
}

// frees memory for global history and line buffer
// removes signal handle for c^c
bool tl_exit(void)
{
    itl_history_free();
    itl_string_free(lbuf);

    signal(SIGINT, SIG_DFL);
    ITL_DBG("exit, alloc count", itl_global_alloc_count);
    TL_ASSERT(itl_global_alloc_count == 0);

    return itl_exit_raw_mode();
}

#ifdef _WIN32
    #define read_byte() _getch()
#else // Linux
    #define read_byte() fgetc(stdin)
#endif // Linux

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
    ITL_KEY_INTERRUPT,
} ITL_KEY_KIND;

#define ITL_CTRL_BIT  32
#define ITL_SHIFT_BIT 64
#define ITL_ALT_BIT   128

#define ITL_KEY_MASK  63
#define ITL_MOD_MASK  448

static ITL_KEY_KIND itl_parse_esc(int byte)
{
    ITL_KEY_KIND event = ITL_KEY_UNKN;

    switch (byte) { // plain bytes
        case 3:
            return ITL_KEY_INTERRUPT;
        case 9:
            return ITL_KEY_TAB;
        case 10: // newline
        case 13: // cr
            return ITL_KEY_ENTER;
        case 23: { // ctrl bs
            return ITL_KEY_BACKSPACE | ITL_CTRL_BIT;
        }
        case 8:
        case 127:
            return ITL_KEY_BACKSPACE;
    }

#ifdef _WIN32
    if (byte == 224) { // escape
        switch (read_byte()) {
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
    if (byte == 27) { // \x1b
        byte = read_byte();

        if (byte != 91) { // [
            return ITL_KEY_CHAR | ITL_ALT_BIT;
        }

        switch (read_byte()) { // escape codes based on xterm
            case 51: {
                event = ITL_KEY_DELETE;
            } break;

            case 65: {
                return ITL_KEY_UP;
            } break;

            case 66: {
                return ITL_KEY_DOWN;
            } break;

            case 67: {
                return ITL_KEY_RIGHT;
            } break;

            case 68: {
                return ITL_KEY_LEFT;
            } break;

            case 70: {
                return ITL_KEY_END;
            } break;

            case 72: {
                return ITL_KEY_HOME;
            } break;

            default:
                event = ITL_KEY_UNKN;
        }
    } else {
        return ITL_KEY_CHAR;
    }

    byte = read_byte();

    if (byte == 59) { // ;
        switch (read_byte()) {
            case 53: {
                event |= ITL_CTRL_BIT;
            } break;

            case 51: {
                event |= ITL_SHIFT_BIT;
            } break;
        }

        byte = read_byte();
    }

    if (byte != '~')
        event = ITL_KEY_UNKN;
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
        bytes[i] = read_byte();

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
static int itl_handle_esc(itl_le *le, ITL_KEY_KIND esc)
{
    switch (esc & ITL_KEY_MASK) {
        case ITL_KEY_UP: {
            if (le->h_item_sel == -1) {
                le->h_item_sel = itl_h_index;
                if (le->lbuf->size > 0 && itl_h_index > 0) {
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
            if (le->cur_pos >= 0 && le->cur_pos < le->lbuf->size) {
                itl_le_right(le, 1);
            }
        } break;

         case ITL_KEY_LEFT: {
            if (le->cur_pos > 0 && le->cur_pos <= le->lbuf->size) {
                itl_le_left(le, 1);
            }
        } break;

        case ITL_KEY_END: {
            itl_le_right(le, le->lbuf->size - le->cur_pos);
        } break;

        case ITL_KEY_HOME: {
            itl_le_left(le, le->cur_pos);
        } break;

        case ITL_KEY_ENTER: {
            itl_history_append(le->lbuf);
            itl_string_to_cstr(le->lbuf, le->out, le->out_size);
            itl_le_clear(le);
            return 0;
        } break;

        case ITL_KEY_BACKSPACE: {
            itl_le_unputc(le, 1, true);
        } break;

        case ITL_KEY_DELETE: {
            itl_le_unputc(le, 1, false);
        } break;

        case ITL_KEY_INTERRUPT: {
            return 1;
        } break;

        default: {
            ITL_DBG("key wasn't handled", esc | ITL_KEY_MASK);
            ITL_DBG("modifier", esc | ITL_MOD_MASK);
        }
    }

    return -1;
}

//  0 on success
//  1 on interrupt
// -1 internal error (reserved)
int tl_readline(char *line_buffer, size_t size)
{
    itl_le le = itl_le_new(lbuf, line_buffer, size);
    int esc;
    int in;

    while (true) {
        in = read_byte();

        if ((esc = itl_parse_esc(in)) != ITL_KEY_CHAR) {
            if ((esc = itl_handle_esc(&le, esc)) != -1) {
                return esc;
            }
        }
        else {
            itl_utf8_t ch = itl_parse_utf8(in);
            itl_le_putc(&le, ch);
        }

        itl_tty_update(&le);
    }

    return -1;
}

#endif // TOILETLINE_IMPLEMENTATION

/**
 * TODO:
 *  - autocompletion
 *  - modifiers implementation
 */
