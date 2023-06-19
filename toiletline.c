/**
 * toiletline 0.0.1
 * Raw CLI shell implementation, meant to be a tiny replacement of GNU Readline :3
 *
 * #define TOILETLINE_IMPL
 * Before you include this file in C or C++ file to create the implementation.
 *
 * Accepts UTF-8 input. Parsed UTF-8 is not guranteed to be valid.
 * Does NOT work on Windows YET.
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
#include <stdint.h>

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

// utf-8 char
typedef struct {
    uint8_t bytes[4];
    size_t length;
} itl_utf8_c_t;

itl_utf8_c_t itl_utf8_new(const uint8_t *bytes, uint8_t length) {
    itl_utf8_c_t utf8_char;
    utf8_char.length = length;

    for (uint8_t i = 0; i < length; i++) {
        utf8_char.bytes[i] = bytes[i];
    }

    return utf8_char;
}

void itl_utf8_put(const itl_utf8_c_t *utf_c)
{
    for (size_t i = 0; i < utf_c->length; ++i)
        fputc(utf_c->bytes[i], stdout);
}

typedef struct itl_char_t itl_char_t;

// utf-8 string node
struct itl_char_t
{
    itl_utf8_c_t ch;
    itl_char_t *next;
};

static itl_char_t itl_char_new(const itl_utf8_c_t utf8_c, itl_char_t *next)
{
    struct itl_char_t c = {
        .ch = utf8_c,
        .next = next,
    };

    return c;
}

static itl_char_t *itl_char_alloc() {
    itl_char_t *ptr = (itl_char_t *)calloc(1, sizeof(itl_char_t *));
    ptr->next = NULL;
    return ptr;
}

static void itl_char_copy(itl_char_t *dst, itl_char_t *src)
{
    memcpy(dst, src, sizeof(itl_char_t));
}

static void itl_char_free(itl_char_t *c) {
    if (c != NULL)
        free(c);
}

void itl_char_put(const itl_char_t *c)
{
    itl_utf8_put(&c->ch);
}

typedef struct itl_string_t itl_string_t;

// string, a linked list of utf-8 chars
struct itl_string_t
{
    itl_char_t *c;
    size_t size;
};

static itl_string_t itl_string_new()
{
    itl_string_t str = {
        .c = NULL,
        .size = 0,
    };

    return str;
}

static itl_string_t *itl_string_alloc() {
    itl_string_t *ptr = (itl_string_t *)calloc(1, sizeof(itl_string_t *));
    ptr->size = 0;
    ptr->c = NULL;
    return ptr;
}

static void itl_string_copy(itl_string_t *dst, itl_string_t *src)
{
    itl_char_t *src_c = src->c;
    itl_char_t *prev_new_c = NULL;
    itl_char_t *new_c;

    dst->c = NULL;

    while (src_c) {
        new_c = itl_char_alloc();
        itl_char_copy(new_c, src_c);

        if (prev_new_c)
            prev_new_c->next = new_c;
        else
            dst->c = new_c;

        prev_new_c = new_c;
        src_c = src_c->next;
    }

    dst->size = src->size;
}

// frees every character in a string, does not free the string itself
static void itl_string_clear(itl_string_t *str)
{
    itl_char_t *c = str->c;
    itl_char_t *next;

    while (c) {
        next = c->next;
        itl_char_free(c);
        c = next;
    }

    str->size = 0;
    str->c = NULL;
}

static void itl_string_free(itl_string_t *str)
{
    itl_string_clear(str);
    free(str);
}

// prints out a string
static void itl_string_put(itl_string_t *str)
{
    itl_char_t *c = str->c;

    while (c) {
        itl_char_put(c);
        c = c->next;
    }

    fflush(stdout);
}

static itl_char_t **itl_string_at(itl_string_t *str, size_t pos)
{
    size_t i = 0;
    itl_char_t **c = &(str->c);

    while (i++ != pos) {
        if (*c == NULL) {
            return NULL;
        }
        c = &((*c)->next);
    }

    return c;
}

static void itl_string_to_cstr(itl_string_t *str, char *cstr, size_t size)
{
    itl_char_t *c = str->c;
    size_t i = 0;;

    while (c && i < size) {
        for (size_t j = 0; j != c->ch.length && i < size; ++j)
            cstr[i++] = (char)c->ch.bytes[j];
        c = c->next;
    }

    if (i < size)
        cstr[i] = '\0';
}

static itl_string_t *lbuf = NULL;

// line editor
struct itl_le
{
    struct itl_string_t *lbuf;
    size_t cursor_pos;
    int h_item_sel;
};

static struct itl_le itl_le_new(itl_string_t *lbuf)
{
    struct itl_le le = {
        .lbuf        = lbuf,
        .cursor_pos  = 0,
        .h_item_sel  = -1,
    };

    return le;
}

// removes character at cursor position - 1 (like backspace)
// frees removed characters
static bool itl_le_unputc(struct itl_le *le)
{
    if (le->lbuf->c == NULL) {
        return false;
    }

    itl_char_t *to_free;
    itl_char_t **cur_c = itl_string_at(le->lbuf, le->cursor_pos - 2);

    if (le->cursor_pos == 0) {
        to_free = le->lbuf->c;
        (*cur_c)->next = to_free->next;
    }
    else if (cur_c && *cur_c) {
        to_free = (*cur_c)->next;
        (*cur_c)->next = to_free->next;
    }
    else {
        to_free = le->lbuf->c;
        le->lbuf->c = to_free->next;
    }

    itl_char_free(to_free);

    le->cursor_pos -= 1;
    le->lbuf->size -= 1;

    return true;
}

// allocates memory for new characters
// inserts character at cursor position
static bool itl_le_putc(struct itl_le *le, const itl_utf8_c_t ch)
{
    if (le->cursor_pos > le->lbuf->size) {
        return false;
    }

    itl_char_t *new_c = itl_char_alloc();
    new_c->ch = ch;

    itl_char_t **cur_c = itl_string_at(le->lbuf, le->cursor_pos - 1);

    if (cur_c) {
        new_c->next = (*cur_c)->next;
        (*cur_c)->next = new_c;
    } else {
        new_c->next = le->lbuf->c;
        le->lbuf->c = new_c;
    }

    le->lbuf->size += 1;
    le->cursor_pos += 1;

    return true;
}

int itl_le_update(struct itl_le *le)
{
    fputs("\x1b[2K", stdout);
    fputc('\r', stdout);
    itl_string_put(le->lbuf);
    printf("\x1b[%zuG", le->cursor_pos + 1);

    return fflush(stdout);
}

// frees line buffer's string's characters, does not free the string itself
static void itl_le_clear(struct itl_le *le)
{
    itl_string_clear(le->lbuf);

    le->lbuf->size = 0;
    le->cursor_pos = 0;
}

#define TL_HISTORY_INIT_SIZE 16

static itl_string_t **itl_history = NULL;
static int itl_h_size     = TL_HISTORY_INIT_SIZE;
static int itl_h_index    = 0;

static void itl_history_free()
{
    for (int i = 0; i < itl_h_index; ++i) {
        free(itl_history[i]);
    }
    free(itl_history);
}

// copies string to global history
// allocates memory for a new string
// allocates memory if needed by multiplying size by 2
static void itl_history_append(itl_string_t *str)
{
    if (str->size <= 0) {
        return;
    }

    if (itl_h_index >= itl_h_size) {
        itl_h_size *= 2;
        itl_history = (itl_string_t **)realloc(itl_history, itl_h_size * sizeof(itl_string_t *));
    }

    itl_string_t *new_str = itl_string_alloc();
    itl_string_copy(new_str, str);

    itl_history[itl_h_index++] = new_str;
}

// copies string from history to line editor
// does not free anything
static void itl_history_get(struct itl_le *le)
{
    if (le->h_item_sel >= itl_h_index) {
        return;
    }

    itl_string_t *h_entry = itl_history[le->h_item_sel];
    itl_string_copy(lbuf, h_entry);
    le->cursor_pos = lbuf->size;
}

// allocates memory for global history and line buffer
// adds signal handle for c^c
bool tl_init()
{
    itl_history = (itl_string_t **)calloc(TL_HISTORY_INIT_SIZE, sizeof(itl_string_t **));

    lbuf = itl_string_alloc();

    signal(SIGINT, itl_handle_interrupt);
    return itl_enter_raw_mode();
}

// frees memory for global history and line buffer
// removes signal handle for c^c
bool tl_exit()
{
    itl_history_free();
    itl_string_free(lbuf);

    signal(SIGINT, SIG_DFL);
    return itl_exit_raw_mode();
}

//  0 on success
// -1 exited;
// -2 line_buffer size exceeded;
int tl_readline(char *line_buffer, size_t size)
{
    struct itl_le le = itl_le_new(lbuf);

    uint8_t bytes[4] = {0};
    uint8_t rem = 0;
    uint8_t length = 0;

    uint8_t esc_pos = 0;

    while (true) {
        int in = fgetc(stdin);

        if (esc_pos == 1 && in == '[') {
            esc_pos = 2;
            continue;
        }
        else if (esc_pos == 2) { // escape codes based on xterm
            switch (in) {
                case 65: { // up
                    if (le.h_item_sel == -1) {
                        le.h_item_sel = itl_h_index;
                        if (le.lbuf->size > 0 && itl_h_index > 0) {
                            itl_history_append(le.lbuf);
                        }
                    }

                    if (le.h_item_sel > 0) {
                        le.h_item_sel -= 1;
                        itl_le_clear(&le);
                        itl_history_get(&le);
                    }
                } break;

                case 66: { // down
                    if (le.h_item_sel < itl_h_index - 1 && le.h_item_sel >= 0) {
                        le.h_item_sel += 1;
                        itl_le_clear(&le);
                        itl_history_get(&le);
                    }
                    else {
                        itl_le_clear(&le);
                        le.h_item_sel = -1;
                    }
                } break;

                case 67: { // right
                    if (le.cursor_pos >= 0 && le.cursor_pos < le.lbuf->size) {
                        le.cursor_pos += 1;
                    }
                } break;

                case 68: { // left
                    if (le.cursor_pos > 0 && le.cursor_pos <= le.lbuf->size) {
                        le.cursor_pos -= 1;
                    }
                } break;

                case 70: { // end
                    // todo
                } break;

                case 72: { // home
                    // todo
                } break;

            }

            itl_le_update(&le);
            esc_pos = 0;

            continue;
        }
        else if (esc_pos == 1) {
            esc_pos = 0;
        }

        if (in == '\x1b') {
            esc_pos = 1;
            continue;
        }

        // utf-8 sequence
        if (rem == 0) {
            if ((in & 0x80) == 0) { // 1 byte
                bytes[0] = in;
                length = 1;
            }
            else if ((in & 0xE0) == 0xC0) { // 2 byte
                bytes[0] = in;
                rem = 1;
                length = 2;
            }
            else if ((in & 0xF8) == 0xF0) { // 3 byte
                bytes[0] = in;
                rem = 2;
                length = 3;
            }
            else if ((in & 0xF8) == 0xF0) { // 4 byte
                bytes[0] = in;
                rem = 3;
                length = 4;
            }
        }
        else
            bytes[length - rem--] = in; // consequent bytes

        if (rem != 0)
            continue;

        itl_utf8_c_t utf8_c = itl_utf8_new(bytes, length);

        if (utf8_c.length == 1) {
            switch (utf8_c.bytes[0]) {
                case 3: {  // c^c
                    return -1;
                } break;

                case 10:   // newline
                case 13: { // cr
                    itl_history_append(le.lbuf);
                    itl_string_to_cstr(le.lbuf, line_buffer, size);
                    itl_le_clear(&le);
                    return 0;
                } break;

                case 127: { // backspace
                    itl_le_unputc(&le);
                } break;

                default: {
                    itl_le_putc(&le, utf8_c);
                }
            }
        }
        else
            itl_le_putc(&le, utf8_c);

        itl_le_update(&le);
    }
    return -1;
}

#endif // TOILETLINE_IMPLEMENTATION

// TODO:
// - autocompletion
// - windows support (idk how to enter raw mode !!!)
