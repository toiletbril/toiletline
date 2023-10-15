
/*
 *  toiletline 0.3.4
 *  Raw shell implementation, a tiny replacement of GNU Readline :3
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

#if defined __cplusplus
extern "C" {
#endif

#if !defined TOILETLINE_H_
#define TOILETLINE_H_

#include <stddef.h>

/* To use custom assertions or to disable them, you can define TL_ASSERT before
 * including. */
#if !defined TL_ASSERT
    #define ITL_DEFAULT_ASSERT
#endif /* TL_ASSERT */

/* Allocation functions can also be replaced before including. */
#if !defined TL_MALLOC
    #define TL_MALLOC(size)         malloc(size)
    #define TL_REALLOC(block, size) realloc(block, size)
    #define TL_FREE(ptr)            free(ptr)
    /* Will be called on failed allocation.
     * TL_NO_ABORT can be defined to disable failure checking. */
    #define TL_ABORT() abort()
#endif /* TL_MALLOC */

/**
 * Max size of in-memory history, must be a power of 2.
 */
#if !defined TL_HISTORY_MAX_SIZE
    #define TL_HISTORY_MAX_SIZE 128
#endif

#define TL_SUCCESS 0
/**
 * Codes which may be returned from reading functions.
 */
#define TL_PRESSED_ENTER -1
#define TL_PRESSED_INTERRUPT -2
#define TL_PRESSED_CONTROL_SEQUENCE -3
/**
 * Codes above 0 are errors.
 */
#define TL_ERROR 1
#define TL_ERROR_SIZE 2
#define TL_ERROR_ALLOC 3

/**
 * Control sequences.
 * Last control sequence used will be stored in 'tl_last_control'.
 */
typedef enum
{
    TL_KEY_CHAR = 0,
    TL_KEY_UNKN,

    TL_KEY_UP,
    TL_KEY_DOWN,
    TL_KEY_RIGHT,
    TL_KEY_LEFT,

    TL_KEY_END,
    TL_KEY_HOME,

    TL_KEY_ENTER,

    TL_KEY_BACKSPACE,
    TL_KEY_DELETE,

    TL_KEY_CTRLK,
    TL_KEY_TAB,

    TL_KEY_SUSPEND,
    TL_KEY_EOF,
    TL_KEY_INTERRUPT,
} TL_KEY_KIND;

#define TL_MOD_CTRL  32
#define TL_MOD_SHIFT 64
#define TL_MOD_ALT   128

#define TL_MASK_KEY  15
#define TL_MASK_MOD  224

/**
 * Last pressed control sequence.
*/
#define tl_last_control (*itl__get_last_control())
/**
 * Initialize toiletline, enter raw mode.
 */
int tl_init(void);
/**
 * Exit toiletline and free resources.
 */
int tl_exit(void);
/**
 * Read one character without waiting for Enter.
 */
int tl_getc(char *char_buffer, size_t size, const char *prompt);
/**
 * Read one line.
 */
int tl_readline(char *line_buffer, size_t size, const char *prompt);
/**
 * Returns the number of UTF-8 characters.
 *
 * Since number of bytes can be bigger than amount of characters,
 * regular strlen will not work, and will only return the number of bytes before \0.
 */
size_t tl_utf8_strlen(const char *utf8_str);

#endif /* TOILETLINE_H_ */

#if defined TOILETLINE_IMPLEMENTATION

#if defined(_WIN32)
    #define ITL_WIN32
#elif defined __linux__ || defined BSD || defined __APPLE__
    #define ITL_POSIX
#else /* __linux__ || BSD || __APPLE__ */
    #error "Your system is not supported"
#endif

#if defined _MSC_VER
    #define ITL_THREAD_LOCAL __declspec(thread)
#elif defined __GNUC__ || defined __clang__
    #define ITL_THREAD_LOCAL __thread
#else
    #define ITL_THREAD_LOCAL /* nothing */
#endif /* __GNUC__ || __clang__ */

#if defined(ITL_WIN32)
    #define WIN32_LEAN_AND_MEAN

    #include <windows.h>
    #include <conio.h>
    #include <fcntl.h>
    #include <io.h>

    #define STDIN_FILENO  _fileno(stdin)
    #define STDOUT_FILENO _fileno(stdout)

    #define write(file, buf, count) _write(file, buf, count)

    /* <https://learn.microsoft.com/en-US/troubleshoot/windows-client/shell-experience/command-line-string-limitation> */
    #define ITL_MAX_STRING_LEN 8191

    #define itl_read_byte() _getch()
#elif defined ITL_POSIX
    #if !defined _DEFAULT_SOURCE
        #define _DEFAULT_SOURCE
    #endif

    #include <sys/ioctl.h>
    #include <termios.h>
    #include <unistd.h>

    /* <https://man7.org/linux/man-pages/man3/termios.3.html> */
    #define ITL_MAX_STRING_LEN 4095

    #define itl_read_byte() fgetc(stdin)
#endif /* ITL_POSIX */

#if defined ITL_DEFAULT_ASSERT
    #include <assert.h>
    #define TL_ASSERT(boolval) assert(boolval)
#endif /* ITL_DEFAULT_ASSERT */

#include <ctype.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined ITL_DEBUG
    #define itl_trace(...)    fprintf(stderr, "[TRACE] " __VA_ARGS__)
    // Print to stderr on the same line
    #define itl_trace_sl(...) fprintf(stderr, __VA_ARGS__)
    // Print LF to stderr
    #define itl_trace_lf()    fprintf(stderr, "\n")
#else /* ITL_DEBUG */
    #define itl_trace(...)    /* nothing */
    #define itl_trace_sl(...) /* nothing */
    #define itl_trace_lf()    /* nothing */
#endif

#define ITL_MAX(type, i, j) ((((type)i) > ((type)j)) ? ((type)i) : ((type)j))

#if defined(ITL_WIN32)
static ITL_THREAD_LOCAL DWORD itl_global_original_tty_mode = 0;
static ITL_THREAD_LOCAL UINT itl_global_original_tty_cp    = 0;
static ITL_THREAD_LOCAL int itl_global_original_mode       = 0;
#elif defined ITL_POSIX
static ITL_THREAD_LOCAL struct termios itl_global_original_tty_mode = { 0 };
#endif

inline static int itl_enter_raw_mode(void)
{
#if defined(ITL_WIN32)
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hInput == INVALID_HANDLE_VALUE)
        return TL_ERROR;

    DWORD tty_mode;
    if (!GetConsoleMode(hInput, &tty_mode))
        return TL_ERROR;

    itl_global_original_tty_mode = tty_mode;
    tty_mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);

    if (!SetConsoleMode(hInput, tty_mode))
        return TL_ERROR;

    UINT codepage = GetConsoleCP();
    if (codepage == 0)
        return TL_ERROR;

    itl_global_original_tty_cp = codepage;

    if (!SetConsoleCP(CP_UTF8))
        return TL_ERROR;

    int mode = _setmode(STDIN_FILENO, _O_BINARY);
    if (mode == -1)
        return TL_ERROR;

    itl_global_original_mode = mode;

#elif defined ITL_POSIX
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) != 0)
        return TL_ERROR;

    itl_global_original_tty_mode = term;

    cfmakeraw(&term);
    // Map \r to each \n so cursor gets back to the beginning
    term.c_oflag = OPOST | ONLCR;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) != 0)
        return TL_ERROR;

#endif /* ITL_POSIX */
    return TL_SUCCESS;
}

inline static int itl_exit_raw_mode(void)
{
#if defined(ITL_WIN32)
    HANDLE h_input = GetStdHandle(STD_INPUT_HANDLE);
    if (h_input == INVALID_HANDLE_VALUE)
        return TL_ERROR;

    if (!SetConsoleMode(h_input, itl_global_original_tty_mode))
        return TL_ERROR;

    if (!SetConsoleCP(itl_global_original_tty_cp))
        return TL_ERROR;

    if (_setmode(STDIN_FILENO, itl_global_original_mode) == -1)
        return TL_ERROR;

#elif defined ITL_POSIX
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &itl_global_original_tty_mode) != 0)
        return TL_ERROR;
#endif /* ITL_POSIX */
    return TL_SUCCESS;
}

static void itl_handle_cont(int signal_number)
{
    (void)signal_number;
    itl_enter_raw_mode();
}

static void itl_handle_sigcont(int signal_number)
{
#if defined ITL_POSIX
    (void)signal_number;
    signal(SIGTSTP, SIG_DFL);
    itl_enter_raw_mode();
#endif
}

static ITL_THREAD_LOCAL size_t itl_global_alloc_count = 0;

inline static void *itl_malloc(size_t size)
{
    itl_global_alloc_count += 1;

    void *allocated = TL_MALLOC(size);

#if !defined TL_NO_ABORT
    if (allocated == NULL)
        TL_ABORT();
#endif

    return allocated;
}

inline static void *itl_calloc(size_t count, size_t size)
{
    itl_global_alloc_count += 1;

    // TODO: This should be a loop to avoid wrapping with very high values, but
    // I don't think there will be such problem
    void *allocated = TL_MALLOC(count * size);

#if !defined TL_NO_ABORT
    if (allocated == NULL)
        TL_ABORT();
#endif

    memset(allocated, 0, count * size);

    return allocated;
}

inline static void *itl_realloc(void *block, size_t size)
{
    if (block == NULL)
        itl_global_alloc_count += 1;

    void *allocated = TL_REALLOC(block, size);

#if !defined TL_NO_ABORT
    if (allocated == NULL)
        TL_ABORT();
#endif

    return allocated;
}

#define itl_free(ptr)                     \
    do {                                  \
        if (ptr != NULL) {                \
            itl_global_alloc_count -= 1;  \
            TL_FREE(ptr);                 \
        }                                 \
    } while (0)

typedef struct itl_utf8 itl_utf8_t;

struct itl_utf8
{
    size_t size;
    uint8_t bytes[4];
};

static itl_utf8_t itl_utf8_new(const uint8_t *bytes, uint8_t size)
{
    itl_utf8_t utf8_char;
    utf8_char.size = size;

    for (uint8_t i = 0; i < size; i++)
        utf8_char.bytes[i] = bytes[i];

    return utf8_char;
}

// TODO: Codepoints U+D800 to U+DFFF (known as UTF-16 surrogates) are invalid
static itl_utf8_t itl_utf8_parse(int byte)
{
    uint8_t bytes[4] = { (uint8_t)byte, 0, 0, 0 };
    uint8_t len = 0;

    if ((byte & 0x80) == 0) // 1 byte
        len = 1;
    else if ((byte & 0xE0) == 0xC0) // 2 byte
        len = 2;
    else if ((byte & 0xF0) == 0xE0) // 3 byte
        len = 3;
    else if ((byte & 0xF8) == 0xF0) // 4 byte
        len = 4;
    else { // invalid character
        itl_trace_lf();
        itl_trace("Invalid UTF-8 sequence '%d'\n", (uint8_t)byte);
        uint8_t replacement_character[3] = { 0xEF, 0xBF, 0xBD };
        return itl_utf8_new(replacement_character, 3);
    }

    for (uint8_t i = 1; i < len; ++i) // consequent bytes
        bytes[i] = itl_read_byte();

#if defined ITL_DEBUG
    itl_trace_lf();
    itl_trace("utf8 char bytes: '");

    for (uint8_t i = 0; i < len - 1; ++i)
        itl_trace_sl("%02X ", bytes[i]);

    itl_trace_sl("%02X'\n", bytes[len - 1]);
#endif /* ITL_DEBUG */

    return itl_utf8_new(bytes, len);
}

typedef struct itl_char itl_char_t;

// UTF-8 string list node
struct itl_char
{
    itl_char_t *next;
    itl_char_t *prev;
    itl_utf8_t rune;
};

static itl_char_t *itl_char_alloc(void)
{
    itl_char_t *ptr = (itl_char_t *)itl_malloc(sizeof(itl_char_t));

    ptr->next = NULL;
    ptr->prev = NULL;

    return ptr;
}

inline static void itl_char_copy(itl_char_t *dst, itl_char_t *src)
{
    memcpy(dst, src, sizeof(itl_char_t));
}

#define itl_char_free(c) itl_free(c)

typedef struct itl_string itl_string_t;

// Linked list of utf-8 characters
struct itl_string
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

static int itl_string_cmp(itl_string_t *str1, itl_string_t *str2)
{
    if (str1->size != str2->size)
        return TL_ERROR;

    itl_char_t *str1_c = str1->begin;
    itl_char_t *str2_c = str2->begin;

    while (str1_c) {
        if (str1_c->rune.size != str2_c->rune.size)
            return TL_ERROR;

        int cmp_result = memcmp(str1_c->rune.bytes, str2_c->rune.bytes, 4);

        if (cmp_result != 0)
            return TL_ERROR;

        str1_c = str1_c->next;
        str2_c = str2_c->next;
    }

    return TL_SUCCESS;
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

// Frees every character in a string, does not free the string itself
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

#define itl_string_free(str)   \
    do {                       \
        itl_string_clear(str); \
        itl_free(str);         \
    } while (0)

static int itl_string_to_cstr(itl_string_t *str, char *c_str, size_t size)
{
    itl_char_t *c = str->begin;
    size_t i = 0;

    // NOTE: (size - i - 1) can wrap around?
    while (c && size - i > c->rune.size) {
        for (size_t j = 0; j != c->rune.size; ++j)
            c_str[i++] = c->rune.bytes[j];

        if (c != c->next)
            c = c->next;
    }

    c_str[i] = '\0';

    // If *c exists, then size was exceeded
    if (c)
        return TL_ERROR_SIZE;

    return TL_SUCCESS;
}

#if defined(ITL_WIN32)
    #define ITL_LF "\r\n"
    #define ITL_LF_LEN 2
#else /* ITL_WIN32 */
    #define ITL_LF "\n"
    #define ITL_LF_LEN 1
#endif

static int
itl_string_to_tty_cstr(itl_string_t *str, char *c_str, size_t size,
                       size_t cols, size_t pr_len, size_t *cur_offset)
{
    (void)cols;
    (void)pr_len;
    (void)cur_offset;

    return itl_string_to_cstr(str, c_str, size);
}

static ITL_THREAD_LOCAL itl_string_t itl_global_line_buffer = { 0 };

typedef struct itl_le itl_le_t;

// Line editor
struct itl_le
{
    itl_string_t *line;
    itl_char_t *current_character;
    size_t cursor_position;
    int history_selected_item;
    char *out_buf;
    size_t out_size;
    const char *prompt;
};

static itl_le_t itl_le_new(itl_string_t *line_buf, char *out_buf, size_t out_size, const char *prompt)
{
    itl_le_t le = {
        /* .line                   = */ line_buf,
        /* .current_character      = */ NULL,
        /* .cursor_position        = */ 0,
        /* .history_selected_item  = */ -1,
        /* .out_buf                = */ out_buf,
        /* .out_size               = */ out_size,
        /* .prompt                 = */ prompt,
    };

    return le;
}

// Removes and frees characters from cursor position
static int itl_le_erase(itl_le_t *le, size_t count, int behind)
{
    size_t i = 0;
    itl_char_t *to_free = NULL;

    while (i++ < count) {
        if (behind) {
            if (le->cursor_position == 0)
                return TL_ERROR;
            else if (le->current_character) {
                to_free = le->current_character->prev;
                le->cursor_position -= 1;
            }
            else if (le->cursor_position == le->line->length) {
                to_free = le->line->end;
                le->line->end = le->line->end->prev;
                le->cursor_position -= 1;
            }
        }
        else {
            if (le->cursor_position == le->line->length)
                return TL_ERROR;
            else if (le->current_character) {
                to_free = le->current_character;
                le->current_character = le->current_character->next;
                if (le->cursor_position == le->line->length - 1)
                    le->line->end = le->line->end->prev;
            }
        }

        if (le->cursor_position == 0)
            le->line->begin = le->line->begin->next;

        if (to_free->prev)
            to_free->prev->next = to_free->next;
        if (to_free->next)
            to_free->next->prev = to_free->prev;

        le->line->size -= to_free->rune.size;
        le->line->length -= 1;

        itl_char_free(to_free);
    }

    return TL_SUCCESS;
}

#define itl_le_erase_forward(le, count) itl_le_erase(le, count, 0)
#define itl_le_erase_backward(le, count) itl_le_erase(le, count, 1)

// Inserts character at cursor position
static int itl_le_putc(itl_le_t *le, const itl_utf8_t ch)
{
    if (le->line->size >= le->out_size - 1)
        return TL_ERROR;

    if (le->out_size - 1 < le->line->size + ch.size)
        return TL_ERROR;

    itl_char_t *new_c = itl_char_alloc();
    new_c->rune = ch;

    if (le->cursor_position == 0) {
        new_c->next = le->line->begin;
        if (le->line->begin)
            le->line->begin->prev = new_c;
        le->line->begin = new_c;
        if (le->line->end == NULL)
            le->line->end = new_c;
    }
    else if (le->cursor_position == le->line->length) {
        if (le->line->end)
            le->line->end->next = new_c;
        new_c->prev = le->line->end;
        le->line->end = new_c;
    }
    else if (le->current_character) {
        new_c->next = le->current_character;
        if (le->current_character->prev) {
            le->current_character->prev->next = new_c;
            new_c->prev = le->current_character->prev;
        }
        le->current_character->prev = new_c;
    }

    le->line->length += 1;
    le->line->size += ch.size;
    le->cursor_position += 1;

    return TL_SUCCESS;
}

static void itl_le_move_right(itl_le_t *le, size_t steps)
{
    size_t i = 0;
    while (i++ < steps) {
        if (le->current_character) {
            le->current_character = le->current_character->next;
            le->cursor_position += 1;
        }
        else
            return;
    }
}

static void itl_le_move_left(itl_le_t *le, size_t steps)
{
    size_t i = 0;
    while (i++ < steps) {
        if (le->cursor_position == 0)
            return;
        else if (le->current_character) {
            le->current_character = le->current_character->prev;
            le->cursor_position -= 1;
        }
        else if (le->cursor_position == le->line->length) {
            le->current_character = le->line->end;
            le->cursor_position -= 1;
        }
        else
            return;
    }
}

#define itl_is_delim(c) (ispunct(c) || isspace(c))

#define ITL_TOKEN_WHITESPACE 0
#define ITL_TOKEN_WORD 1

// Returns amount of steps required to reach specified token
static size_t itl_le_goto_token(itl_le_t *le, int behind, int token)
{
    itl_char_t *ch = le->current_character;
    size_t steps = 1;

    if (ch)
        if (behind)
            ch = ch->prev;
        else
            ch = ch->next;
    else
        if (le->cursor_position == le->line->length && behind)
            ch = le->line->end;
        else
            return 0;

    while (ch) {
        int should_break = 0;

        switch (token) {
            case ITL_TOKEN_WHITESPACE:
                should_break = itl_is_delim(ch->rune.bytes[0]);
                break;
            case ITL_TOKEN_WORD:
                should_break = !itl_is_delim(ch->rune.bytes[0]);
                break;
        }

        if (should_break)
            break;

        steps += 1;

        if (behind)
            ch = ch->prev;
        else
            ch = ch->next;
    }

    return steps;
}

#define itl_le_next_word(le) itl_le_goto_token(le, 0, ITL_TOKEN_WORD)
#define itl_le_prev_word(le) itl_le_goto_token(le, 1, ITL_TOKEN_WORD)
#define itl_le_next_whitespace(le) itl_le_goto_token(le, 0, ITL_TOKEN_WHITESPACE)
#define itl_le_prev_whitespace(le) itl_le_goto_token(le, 1, ITL_TOKEN_WHITESPACE)

// Frees line buffer's string's characters, does not free the string itself
inline static void itl_le_clear(itl_le_t *le)
{
    itl_string_clear(le->line);

    le->cursor_position = 0;
}

typedef struct itl_char_buf itl_char_buf_t;

// Buffer for output before writing it to stdout
struct itl_char_buf
{
  char *data;
  size_t size;
};

inline static void itl_char_buf_append(itl_char_buf_t *buf, const char *s, size_t size)
{
    char *new_s = (char *)itl_realloc(buf->data, buf->size + size);

    memcpy(&new_s[buf->size], s, size);

    buf->data = new_s;
    buf->size += size;
}

#define itl_char_buf_free(char_buf) itl_free((char_buf)->data)

inline static int itl_tty_size(size_t *rows, size_t *cols) {
#if defined TL_SIZE_USE_ESCAPES
    char buf[32];
    size_t i = 0;

    fputs("\x1b[999C", stdout);
    fputs("\x1b[6n", stdout);

    // This does not work without flushing if setvbuf was called previously
    fflush(stdout);

    while (i < sizeof(buf) - 1) {
        buf[i] = itl_read_byte();
        if (buf[i] == 'R')
            break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[')
        return TL_ERROR;

    if (sscanf(&buf[2], "%zu;%zu", rows, cols) != 2)
        return TL_ERROR;

#elif defined(ITL_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO buffer_info;

    int success = GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &buffer_info);

    if (!success)
        return TL_ERROR;

    (*cols) = buffer_info.srWindow.Right - buffer_info.srWindow.Left + 1;
    (*rows) = buffer_info.srWindow.Bottom - buffer_info.srWindow.Top + 1;

#elif defined ITL_POSIX
    struct winsize window;

    int err = ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);

    if (err)
        return TL_ERROR;

    (*rows) = (size_t)window.ws_row;
    (*cols) = (size_t)window.ws_col;
#else /* ITL_POSIX */
    return TL_ERROR;
#endif
    return TL_SUCCESS;
}

static int itl_le_update_tty(itl_le_t *le)
{
    fputs("\x1b[?25l", stdout);

    itl_char_buf_t to_be_printed = { /* .data = */ NULL, /* .size = */ 0 };

    size_t prompt_len;

    // prompt = NULL is valid
    if (le->prompt != NULL)
        prompt_len = strlen(le->prompt);
    else
        prompt_len = 0;

    size_t cstr_size = le->line->size + 1;

    size_t buf_size =
        ITL_MAX(size_t, cstr_size, 8) * sizeof(char) + 2;

    size_t rows = 0;
    size_t cols = 0;

    itl_tty_size(&rows, &cols);

    size_t wrap_value = (le->line->length + prompt_len) / ITL_MAX(size_t, 1, cols);
    size_t wrap_cursor_pos = le->cursor_position + prompt_len - wrap_value * cols + 1;

    itl_trace_lf();
    itl_trace("Line length: %zu\n", le->line->length);
    itl_trace("Dimensions: %zu, %zu\n", rows, cols);
    itl_trace("wrap_value: %zu\n", wrap_value);
    itl_trace("wrap_cursor_pos: %zu\n", wrap_cursor_pos);

    char *temp_buf = (char *)itl_malloc(buf_size);
    if (!temp_buf)
        return TL_ERROR_ALLOC;

    if (wrap_value > 0 && wrap_cursor_pos > 1) {
        snprintf(temp_buf, buf_size, "\x1b[%zuF", wrap_value);
        itl_char_buf_append(&to_be_printed, temp_buf, strlen(temp_buf));
    }

    itl_char_buf_append(&to_be_printed, "\r", 1);
    itl_char_buf_append(&to_be_printed, "\x1b[0K", 4);

    itl_char_buf_append(&to_be_printed, le->prompt, prompt_len);

    size_t cur_offset = 0;
    itl_string_to_tty_cstr(le->line, temp_buf, buf_size, cols, prompt_len, &cur_offset);

    itl_char_buf_append(&to_be_printed, temp_buf, cstr_size + cur_offset);

    snprintf(temp_buf, buf_size, "\x1b[%zuG", wrap_cursor_pos + cur_offset);
    itl_char_buf_append(&to_be_printed, temp_buf, strlen(temp_buf));

    write(STDOUT_FILENO, to_be_printed.data, (unsigned)to_be_printed.size);

    itl_char_buf_free(&to_be_printed);
    itl_free(temp_buf);

    fputs("\x1b[?25h", stdout);

    fflush(stdout);

    return TL_SUCCESS;
}

#define ITL_HISTORY_INIT_SIZE 16

static ITL_THREAD_LOCAL itl_string_t **itl_global_history = NULL;
static ITL_THREAD_LOCAL int itl_global_history_size = ITL_HISTORY_INIT_SIZE;
static ITL_THREAD_LOCAL int itl_global_history_index = 0;

inline static void itl_global_history_alloc(void)
{
    itl_global_history = (itl_string_t **)
        itl_calloc(ITL_HISTORY_INIT_SIZE, sizeof(itl_string_t *));
}

inline static void itl_global_history_free(void)
{
    for (int i = 0; i < itl_global_history_index; ++i)
        itl_string_free(itl_global_history[i]);

    itl_free(itl_global_history);
}

// Copies string to global history. Allocates memory for a new string
static int itl_global_history_append(itl_string_t *str)
{
    if (str->length <= 0)
        return TL_ERROR;

    if (itl_global_history_size >= TL_HISTORY_MAX_SIZE)
        return TL_ERROR;

    // Allocate more memory if needed
    if (itl_global_history_index >= itl_global_history_size && itl_global_history_size < TL_HISTORY_MAX_SIZE) {
        itl_global_history_size *= 2;
        itl_global_history = (itl_string_t **)
            itl_realloc(itl_global_history, itl_global_history_size * sizeof(itl_string_t *));
    }

    if (itl_global_history) {
        // Avoid adding the same string to history
        if (itl_global_history_index > 0) {
            int cmp_result = itl_string_cmp(itl_global_history[itl_global_history_index - 1], str);

            if (cmp_result == TL_SUCCESS)
                return TL_ERROR;
        }
    }
    else
        return TL_ERROR_ALLOC;

    itl_string_t *new_str = itl_string_alloc();

    itl_string_copy(new_str, str);

    itl_global_history[itl_global_history_index++] = new_str;

    return TL_SUCCESS;
}

// Copies string from history to line editor. Does not free anything
static void itl_global_history_get(itl_le_t *le)
{
    if (le->history_selected_item >= itl_global_history_index)
        return;

    itl_string_t *h_entry = itl_global_history[le->history_selected_item];

    itl_string_copy(le->line, h_entry);

    le->cursor_position = le->line->length;
}

static int itl_esc_parse(int byte)
{
    int event = 0;

    switch (byte) { // plain bytes
        case 1: return TL_KEY_HOME; // ctrl a
        case 5: return TL_KEY_END;  // ctrl e

        case 3:  return TL_KEY_INTERRUPT;
        case 4:  return TL_KEY_EOF; // ctrl d
        case 26: return TL_KEY_SUSPEND; // ctrl z

        case 9: return TL_KEY_TAB;

        case 13: // cr
        case 10: return TL_KEY_ENTER;

        case 11: return TL_KEY_CTRLK;
        case 23: return TL_KEY_BACKSPACE | TL_MOD_CTRL;

        case 8: // old backspace
        case 127: return TL_KEY_BACKSPACE;
    }

#if defined(ITL_WIN32)
    if (byte == 224) { // escape
        switch (itl_read_byte()) {
            case 72: event = TL_KEY_UP;     break;
            case 80: event = TL_KEY_DOWN;   break;
            case 75: event = TL_KEY_LEFT;   break;
            case 77: event = TL_KEY_RIGHT;  break;

            case 115: event = TL_KEY_LEFT  | TL_MOD_CTRL; break;
            case 116: event = TL_KEY_RIGHT | TL_MOD_CTRL; break;

            case 71: event = TL_KEY_HOME; break;
            case 79: event = TL_KEY_END;  break;

            case 147: event = TL_KEY_DELETE | TL_MOD_CTRL; break; // ctrl del
            case 83:  event = TL_KEY_DELETE; break;

            default: event = TL_KEY_UNKN;
        }
    } else if (iscntrl(byte)) {
        return TL_KEY_UNKN;
    } else {
        return TL_KEY_CHAR;
    }

#elif defined ITL_POSIX
    int read_mod = 0;

    if (byte == 27) { // \x1b
        byte = itl_read_byte();

        if (byte != 91 && byte != 79) { // [
            return TL_KEY_CHAR | TL_MOD_ALT;
        }

        byte = itl_read_byte();

        if (byte == 49) {
            if (itl_read_byte() != 59) // ;
                return TL_KEY_UNKN;

            switch (itl_read_byte()) {
                case 50: event |= TL_MOD_SHIFT; break;
                case 53: event |= TL_MOD_CTRL;  break;
            }

            read_mod = 1;

            byte = itl_read_byte();
        }

        switch (byte) { // escape codes based on xterm
            case 65: return event | TL_KEY_UP;
            case 66: return event | TL_KEY_DOWN;
            case 67: return event | TL_KEY_RIGHT;
            case 68: return event | TL_KEY_LEFT;

            case 70: return event | TL_KEY_END;
            case 72: return event | TL_KEY_HOME;

            case 51: event |= TL_KEY_DELETE; break;

            default: event |= TL_KEY_UNKN;
        }
    } else if (iscntrl(byte)) {
        return TL_KEY_UNKN;
    } else {
        return TL_KEY_CHAR;
    }

    if (!read_mod) {
        byte = itl_read_byte();

        if (byte == 59) { // ;
            switch (itl_read_byte()) {
                case 53: event |= TL_MOD_CTRL;  break;
                case 51: event |= TL_MOD_SHIFT; break;
            }

            byte = itl_read_byte();
        }

        if (byte != 126) // ~
            event = TL_KEY_UNKN;
    }
#endif /* ITL_POSIX */

    return event;
}

static ITL_THREAD_LOCAL int itl_global_last_control = TL_KEY_UNKN;

// Not meant to be used directly. Use tl_last_control instead
int *itl__get_last_control(void) {
    return &itl_global_last_control;
}

static int itl_esc_handle(itl_le_t *le, int esc)
{
    itl_global_last_control = esc;

    switch (esc & TL_MASK_KEY) {
        case TL_KEY_UP: {
            if (le->history_selected_item == -1) {
                le->history_selected_item = itl_global_history_index;
                if (le->line->length > 0 && itl_global_history_index > 0) {
                    itl_global_history_append(le->line);
                }
            }

           if (le->history_selected_item > 0) {
                le->history_selected_item -= 1;
                itl_le_clear(le);
                itl_global_history_get(le);
            }
        } break;

        case TL_KEY_DOWN: {
            if (le->history_selected_item < itl_global_history_index - 1 && le->history_selected_item >= 0) {
                le->history_selected_item += 1;

                itl_le_clear(le);
                itl_global_history_get(le);
            }
            else if (itl_global_history_index > 0) {
                itl_le_clear(le);
                le->history_selected_item = -1;
            }
        } break;

        case TL_KEY_RIGHT: {
            if (le->cursor_position < le->line->length) {
                size_t count = 1;

                if (esc & TL_MOD_CTRL) {
                    size_t next_ws = itl_le_next_whitespace(le);

                    if (next_ws <= 1) {
                        count = itl_le_next_word(le);
                    } else
                        count = next_ws;
                }

                itl_le_move_right(le, count);
            }
        } break;

        case TL_KEY_LEFT: {
            if (le->cursor_position > 0 && le->cursor_position <= le->line->length) {
                size_t count = 1;

                if (esc & TL_MOD_CTRL) {
                    size_t next_ws = itl_le_prev_whitespace(le);

                    if (next_ws <= 1) {
                        count = itl_le_prev_word(le);
                        itl_le_move_left(le, count);
                        count = itl_le_prev_whitespace(le) - 1;
                    } else
                        count = next_ws - 1;
                }

                itl_le_move_left(le, count);
            }
        } break;

        case TL_KEY_END: {
            itl_le_move_right(le, le->line->length - le->cursor_position);
        } break;

        case TL_KEY_HOME: {
            itl_le_move_left(le, le->cursor_position);
        } break;

        case TL_KEY_ENTER: {
            int err = itl_string_to_cstr(le->line, le->out_buf, le->out_size);
            itl_global_history_append(le->line);
            itl_le_clear(le);

            if (err)
                return err;
            else {
                fflush(stdout);
                return TL_PRESSED_ENTER;
            }
        } break;

        case TL_KEY_BACKSPACE: {
            size_t count = 1;

            if (esc & TL_MOD_CTRL) {
                size_t next_ws = itl_le_prev_whitespace(le);

                if (next_ws <= 1) {
                    count = itl_le_prev_word(le);
                    itl_le_erase_backward(le, count);
                    count = itl_le_prev_whitespace(le) - 1;
                } else
                    count = next_ws - 1;
            }

            itl_le_erase_backward(le, count);
        } break;

        case TL_KEY_DELETE: {
            size_t count = 1;

            if (esc & TL_MOD_CTRL) {
                size_t next_ws = itl_le_next_whitespace(le);

                if (next_ws <= 1) {
                    count = itl_le_next_word(le);
                    itl_le_erase_forward(le, count);
                    count = itl_le_next_whitespace(le);
                } else
                    count = next_ws;
            }

            itl_le_erase_forward(le, count);
        } break;

        case TL_KEY_CTRLK: {
            itl_le_erase_forward(le, le->line->length - le->cursor_position);
        } break;

        case TL_KEY_SUSPEND: {
#if defined ITL_POSIX
            signal(SIGCONT, itl_handle_sigcont);
            itl_exit_raw_mode();
            raise(SIGTSTP);
#elif defined ITL_WIN32
            itl_string_to_cstr(le->line, le->out_buf, le->out_size);
            return TL_PRESSED_INTERRUPT;
#endif
        } break;

        case TL_KEY_EOF:
        case TL_KEY_INTERRUPT: {
            itl_string_to_cstr(le->line, le->out_buf, le->out_size);
            return TL_PRESSED_INTERRUPT;
        } break;
    }

    return TL_SUCCESS;
}

int tl_init(void)
{
    TL_ASSERT(TL_HISTORY_MAX_SIZE % 2 == 0 && "History size must be a power of 2");

    itl_global_history_alloc();

    return itl_enter_raw_mode();
}

int tl_exit(void)
{
    itl_global_history_free();
    // NOTE: no free since line_buffer is not malloced
    itl_string_clear(&itl_global_line_buffer);

    itl_trace("Exited, alloc count: %zu\n", itl_global_alloc_count);

    int code = itl_exit_raw_mode();

    TL_ASSERT(itl_global_alloc_count == 0);

    return code;
}

// Returns the number of UTF-8 characters in a null terminated string
size_t tl_utf8_strlen(const char *utf8_str)
{
    int len = 0;

    while (*utf8_str) {
        if ((*utf8_str & 0xC0) != 0x80)
            ++len;

        ++utf8_str;
    }

    return len;
}

// Gets one character, does not wait for Enter
int tl_getc(char *char_buffer, size_t size, const char *prompt)
{
    TL_ASSERT(size > 1 &&
        "Size should be enough at least for one byte and a null terminator");
    TL_ASSERT(size <= sizeof(char)*5 &&
        "Size should be less or equal to size of 4 characters with a null terminator.");
    TL_ASSERT(char_buffer != NULL);

    itl_le_t le = itl_le_new(&itl_global_line_buffer, char_buffer, size, prompt);

    itl_le_update_tty(&le);

    int in = itl_read_byte();
    int esc = itl_esc_parse(in);

    if (esc != TL_KEY_CHAR) {
        itl_global_last_control = esc;
        return TL_PRESSED_CONTROL_SEQUENCE;
    }

    itl_utf8_t ch = itl_utf8_parse(in);
    itl_le_putc(&le, ch);
    itl_le_update_tty(&le);

    itl_string_to_cstr(le.line, char_buffer, size);

    itl_le_clear(&le);
    fflush(stdout);

    return TL_SUCCESS;
}

int tl_readline(char *line_buffer, size_t size, const char *prompt)
{
    TL_ASSERT(size > 1 &&
        "Size should be enough at least for one byte and a null terminator");
    TL_ASSERT(size <= ITL_MAX_STRING_LEN &&
        "Size should be less than platform's allowed maximum string length");
    TL_ASSERT(line_buffer != NULL);

    itl_le_t le = itl_le_new(&itl_global_line_buffer, line_buffer, size, prompt);

    itl_le_update_tty(&le);

    int esc;
    int in;

    while (1) {
        in = itl_read_byte();

#if defined ITL_SEE_BYTES
        if (in == 3) exit(0);
        printf("%d\n", in);
        continue;
#endif /* TL_SEE_BYTES */

        esc = itl_esc_parse(in);

        if (esc != TL_KEY_CHAR) {
            int code = itl_esc_handle(&le, esc);

            if (code != TL_SUCCESS)
                return code;
        }
        else {
            itl_utf8_t ch = itl_utf8_parse(in);
            itl_le_putc(&le, ch);
        }

        itl_le_update_tty(&le);
    }

    return TL_ERROR;
}

#endif /* TOILETLINE_IMPLEMENTATION */

#if defined __cplusplus
}
#endif

/*
 * TODO:
 *  - Better memory management.
 *  - itl_string_to_tty_cstr() to support multiple lines.
 *  - Properly replace history when reached limit.
 *  - Tab completion.
 */
