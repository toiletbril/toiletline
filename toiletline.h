/*
 *  toiletline 0.4.2
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

/* If not defined, Ctrl-Z will raise SIGTSTP internally and toiletline will
 * resume normally on SIGCONT. This is the preferred way of doing SIGTSTP without
 * breaking terminal's state if you haven't called tl_exit yet. */
#if !defined TL_NO_SUSPEND
    #define ITL_SUSPEND
#endif /* TL_NO_SUSPEND */

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
#define TL_PRESSED_EOF -3
#define TL_PRESSED_SUSPEND -4
#define TL_PRESSED_CONTROL_SEQUENCE -5
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
    TL_KEY_KILL_LINE,
    TL_KEY_KILL_LINE_BEFORE,

    TL_KEY_TAB,

    TL_KEY_SUSPEND,
    TL_KEY_EOF,
    TL_KEY_INTERRUPT,
} TL_KEY_KIND;

#define TL_MOD_CTRL  (1 << 24)
#define TL_MOD_SHIFT (1 << 25)
#define TL_MOD_ALT   (1 << 26)

#define TL_MASK_KEY 0x00FFFFFF
#define TL_MASK_MOD 0xFF000000

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
#define ITL_MIN(type, i, j) ((((type)i) < ((type)j)) ? ((type)i) : ((type)j))

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

#if defined ITL_SUSPEND
#if defined ITL_POSIX
static void itl_handle_sigcont(int signal_number)
{
    (void)signal_number;
    itl_enter_raw_mode();
}
#endif

// Internally raise SIGTSTP and resume normally on SIGCONT, exit(1) on Windows
static void itl_raise_suspend()
{
#if defined ITL_POSIX
    signal(SIGCONT, itl_handle_sigcont);
    itl_exit_raw_mode();
    raise(SIGTSTP);
#else /* ITL_POSIX */
    exit(1);
#endif
}
#endif /* ITL_SUSPEND */

static ITL_THREAD_LOCAL size_t itl_global_alloc_count = 0;

static void *itl_malloc(size_t size)
{
    itl_global_alloc_count += 1;

    void *allocated = TL_MALLOC(size);

#if !defined TL_NO_ABORT
    if (allocated == NULL)
        TL_ABORT();
#endif

    return allocated;
}

#if 0 // @@@
static void *itl_calloc(size_t count, size_t size)
{
    itl_global_alloc_count += 1;

    void *allocated = TL_MALLOC(count * size);

#if !defined TL_NO_ABORT
    if (allocated == NULL)
        TL_ABORT();
#endif

    memset(allocated, 0, count * size);

    return allocated;
}

static void *itl_realloc(void *block, size_t size)
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
#endif /* if 0 */

#define itl_free(ptr)                     \
    do {                                  \
        if (ptr != NULL) {                \
            itl_global_alloc_count -= 1;  \
            TL_FREE(ptr);                 \
            ptr = NULL;                   \
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

    for (uint8_t i = 0; i < size; ++i)
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

static void itl_string_copy(itl_string_t *dst, const itl_string_t *src)
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
        } else
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

#if defined(ITL_WIN32)
    #define ITL_LF "\r\n"
    #define ITL_LF_LEN 2
#else /* ITL_WIN32 */
    #define ITL_LF "\n"
    #define ITL_LF_LEN 1
#endif

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

typedef struct itl_history_item itl_history_item_t;

struct itl_history_item
{
    itl_history_item_t *next;
    itl_history_item_t *prev;
    itl_string_t *str;
};

static ITL_THREAD_LOCAL itl_history_item_t *itl_global_history = NULL;
static ITL_THREAD_LOCAL itl_history_item_t *itl_global_history_first = NULL;
static ITL_THREAD_LOCAL int itl_global_history_length = 0;

static ITL_THREAD_LOCAL itl_string_t itl_global_line_buffer = {0};

typedef struct itl_le itl_le_t;

// Line editor
struct itl_le
{
    itl_string_t *line;
    itl_char_t *current_character;
    size_t cursor_position;

    itl_history_item_t *history_selected_item;

    char *out_buf;
    size_t out_size;
    const char *prompt;
};

itl_history_item_t *itl_history_item_alloc(const itl_string_t *str)
{
    itl_history_item_t *item = (itl_history_item_t *)itl_malloc(sizeof(itl_history_item_t));

    item->next = NULL;
    item->prev = NULL;

    item->str = itl_string_alloc();
    itl_string_copy(item->str, str);

    return item;
}

static void itl_history_item_free(itl_history_item_t *item)
{
    itl_string_free(item->str);
    itl_free(item);
}

inline static void itl_global_history_free(void)
{
    itl_history_item_t *item = itl_global_history;

    if (item == NULL)
        return;

    while (item->next)
        item = item->next;

    itl_history_item_t *prev_item;

    while (item) {
        prev_item = item->prev;
        itl_history_item_free(item);
        item = prev_item;
    }

    itl_global_history = NULL;
}

static int itl_global_history_append(itl_string_t *str)
{
    if (str->length <= 0)
        return TL_ERROR;

    if (itl_global_history_length >= TL_HISTORY_MAX_SIZE) {
        if (itl_global_history_first) {
            itl_history_item_t *next_item = itl_global_history_first->next;
            itl_history_item_free(itl_global_history_first);

            itl_global_history_first = next_item;

            if (itl_global_history_first)
                itl_global_history_first->prev = NULL;

            --itl_global_history_length;
        }
    }

    if (itl_global_history == NULL) {
        itl_global_history = itl_history_item_alloc(str);
        itl_global_history_first = itl_global_history;
    }
    else {
        int cmp_result = itl_string_cmp(itl_global_history->str, str);
        if (cmp_result == TL_SUCCESS)
            return TL_ERROR;

        itl_history_item_t *item = itl_history_item_alloc(str);
        item->prev = itl_global_history;
        itl_global_history->next = item;
        itl_global_history = item;
    }

    ++itl_global_history_length;

    return TL_SUCCESS;
}

static itl_le_t itl_le_new(itl_string_t *line_buf, char *out_buf, size_t out_size, const char *prompt)
{
    itl_le_t le = {
        /* .line                  = */ line_buf,
        /* .current_character     = */ NULL,
        /* .cursor_position       = */ 0,
        /* .history_selected_item = */ NULL,
        /* .out_buf               = */ out_buf,
        /* .out_size              = */ out_size,
        /* .prompt                = */ prompt,
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
        } else {
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
        } else
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

inline static void itl_le_clear(itl_le_t *le)
{
    itl_string_clear(le->line);
    le->cursor_position = 0;
}

static void itl_global_history_get_prev(itl_le_t *le)
{
    if (le->history_selected_item) {
        if (le->history_selected_item->prev) {
              le->history_selected_item = le->history_selected_item->prev;
        }
    } else {
        le->history_selected_item = itl_global_history;
    }

    if (le->history_selected_item) {
        itl_le_clear(le);
        itl_string_copy(le->line, le->history_selected_item->str);
        le->cursor_position = le->line->length;
    }
}

static void itl_global_history_get_next(itl_le_t *le)
{
    if (le->history_selected_item) {
        if (le->history_selected_item->next) {
            le->history_selected_item = le->history_selected_item->next;

            itl_le_clear(le);
            itl_string_copy(le->line, le->history_selected_item->str);
            le->cursor_position = le->line->length;
        } else {
            le->history_selected_item = NULL;
            itl_le_clear(le);
        }
    }
}

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

static ITL_THREAD_LOCAL size_t itl_global_tty_prev_lines = 1;
static ITL_THREAD_LOCAL size_t itl_global_tty_prev_wrap_row = 1;

#define itl_tty_hide_cursor() fputs("\x1b[?25l", stdout)
#define itl_tty_show_cursor() fputs("\x1b[?25h", stdout)

#define itl_tty_move_to_column(col) printf("\x1b[%zuG", (size_t)col)
#define itl_tty_move_up(rows) printf("\x1b[%zuA", (size_t)rows)
#define itl_tty_move_down(rows) printf("\x1b[%zuB", (size_t)rows)

#define itl_tty_clear_whole_line() fputs("\r\x1b[0K", stdout)
#define itl_tty_clear_to_end() fputs("\x1b[K", stdout)

static int itl_le_tty_refresh(itl_le_t *le)
{
    TL_ASSERT(le->line);
    itl_tty_hide_cursor();

    size_t prompt_len = (le->prompt) ? strlen(le->prompt) : 0;

    size_t rows = 0;
    size_t cols = 0;
    itl_tty_size(&rows, &cols);

    size_t current_lines = (le->line->length + prompt_len) / ITL_MAX(size_t, cols, 1) + 1;

    size_t wrap_cursor_col = (le->cursor_position + prompt_len) % ITL_MAX(size_t, cols, 1) + 1;
    size_t wrap_cursor_row = (le->cursor_position + prompt_len) / ITL_MAX(size_t, cols, 1) + 1;

    itl_trace("wrow: %zu, prev: %zu, col: %zu\n",
              wrap_cursor_row, itl_global_tty_prev_wrap_row, wrap_cursor_col);

    // Move appropriate amount of lines back, while clearing previous output
    for (size_t i = 0; i < itl_global_tty_prev_lines; ++i) {
        itl_tty_clear_whole_line();
        if (i < itl_global_tty_prev_wrap_row - 1) {
            itl_tty_move_up(1);
        }
    }

    if (le->prompt) fputs(le->prompt, stdout);

    // Print current contents of the line editor
    size_t character_index = 0;
    itl_char_t *c = le->line->begin;
    while (c) {
        for (size_t j = 0; j < c->rune.size; ++j) {
            fputc(c->rune.bytes[j], stdout);
        }

        // If line is full, wrap
        size_t current_col = (character_index++ + prompt_len) % ITL_MAX(size_t, cols, 1);
        if (current_col == cols - 1)
            fputs(ITL_LF, stdout);

        c = c->next;
    }

    // If current amount of lines is less than previous amount of lines, then
    // input was cleared by kill line or such. Clear each dirty line, then go
    // back up
    if (current_lines < itl_global_tty_prev_lines) {
        size_t dirty_lines = itl_global_tty_prev_lines - current_lines;
        for (size_t i = 0; i < dirty_lines; ++i) {
            itl_tty_move_down(1);
            itl_tty_clear_whole_line();
        }
        itl_tty_move_up(dirty_lines);
    }
    // Otherwise clear to the end of line
    else {
        itl_tty_clear_to_end();
    }

    // Move cursor to appropriate row and column. If row didn't change, stay on
    // the same line
    if (wrap_cursor_row < current_lines) {
        itl_tty_move_up(current_lines - wrap_cursor_row);
    }
    itl_tty_move_to_column(wrap_cursor_col);

    itl_global_tty_prev_lines = current_lines;
    itl_global_tty_prev_wrap_row = wrap_cursor_row;

    itl_tty_show_cursor();
    fflush(stdout);

    return TL_SUCCESS;
}

static int itl_esc_parse(int byte)
{
    int event = 0;

    switch (byte) { // plain bytes
        case 1: return TL_KEY_HOME; // ctrl a
        case 5: return TL_KEY_END;  // ctrl e

        case 2: return TL_KEY_LEFT;  // ctrl f
        case 6: return TL_KEY_RIGHT; // ctrl b

        case 3:  return TL_KEY_INTERRUPT; // ctrl c
        case 4:  return TL_KEY_EOF;       // ctrl d
        case 26: return TL_KEY_SUSPEND;   // ctrl z

        case 9: return TL_KEY_TAB;

        case 13: // cr
        case 10: return TL_KEY_ENTER;

        case 11: return TL_KEY_KILL_LINE;        // ctrl k
        case 21: return TL_KEY_KILL_LINE_BEFORE; // ctrl u
        case 23: return TL_KEY_BACKSPACE | TL_MOD_CTRL;

        case 8: // old backspace
        case 127: return TL_KEY_BACKSPACE;
    }

#if defined(ITL_WIN32)
    if (byte == 224) { // esc
        switch (itl_read_byte()) {
            case 'H': event = TL_KEY_UP;    break;
            case 'P': event = TL_KEY_DOWN;  break;
            case 'K': event = TL_KEY_LEFT;  break;
            case 'M': event = TL_KEY_RIGHT; break;

            case 's': event = TL_KEY_LEFT  | TL_MOD_CTRL; break;
            case 't': event = TL_KEY_RIGHT | TL_MOD_CTRL; break;

            case 'G': event = TL_KEY_HOME; break;
            case 'O': event = TL_KEY_END;  break;

            case 147: event = TL_KEY_DELETE | TL_MOD_CTRL; break; // ctrl del
            case 'S': event = TL_KEY_DELETE; break;

            default: event = TL_KEY_UNKN;
        }
    } else if (iscntrl(byte)) {
        return TL_KEY_UNKN;
    } else {
        return TL_KEY_CHAR;
    }

#elif defined ITL_POSIX
    int read_mod = 0;

    if (byte == 27) { // esc
        byte = itl_read_byte();

        if (byte != '[' && byte != 'O') {
            switch (byte) {
                case 'b': return TL_KEY_LEFT  | TL_MOD_CTRL;
                case 'f': return TL_KEY_RIGHT | TL_MOD_CTRL;

                case 'd': return TL_KEY_DELETE | TL_MOD_CTRL;

                default: return TL_KEY_CHAR | TL_MOD_ALT;
            }
        }

        byte = itl_read_byte();

        if (byte == '1') {
            if (itl_read_byte() != ';')
                return TL_KEY_UNKN;

            switch (itl_read_byte()) {
                case '2': event |= TL_MOD_SHIFT; break;
                case '5': event |= TL_MOD_CTRL;  break;
            }

            read_mod = 1;

            byte = itl_read_byte();
        }

        switch (byte) { // escape codes based on xterm
            case 'A': return event | TL_KEY_UP;
            case 'B': return event | TL_KEY_DOWN;
            case 'C': return event | TL_KEY_RIGHT;
            case 'D': return event | TL_KEY_LEFT;

            case 'F': return event | TL_KEY_END;
            case 'H': return event | TL_KEY_HOME;

            case '3': event |= TL_KEY_DELETE; break;

            default: event |= TL_KEY_UNKN;
        }
    } else if (iscntrl(byte)) {
        return TL_KEY_UNKN;
    } else {
        return TL_KEY_CHAR;
    }

    if (!read_mod) {
        byte = itl_read_byte();

        if (byte == ';') {
            switch (itl_read_byte()) {
                case '3': event |= TL_MOD_SHIFT; break;
                case '5': event |= TL_MOD_CTRL;  break;
            }

            byte = itl_read_byte();
        }

        if (byte != '~')
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

static int itl_le_esc_handle(itl_le_t *le, int esc)
{
    itl_global_last_control = esc;

    switch (esc & TL_MASK_KEY) {
        case TL_KEY_UP: {
            // @@@: When there is any text in the current line, append it to history
            itl_global_history_get_prev(le);
        } break;

        case TL_KEY_DOWN: {
            itl_global_history_get_next(le);
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
            if (err) return err;
            else return TL_PRESSED_ENTER;
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

        case TL_KEY_KILL_LINE: {
            itl_le_erase_forward(le, le->line->length - le->cursor_position);
        } break;

        case TL_KEY_KILL_LINE_BEFORE: {
            itl_le_erase_backward(le, le->cursor_position);
        } break;

        case TL_KEY_SUSPEND: {
#if defined ITL_SUSPEND
            itl_raise_suspend();
#endif
        } break;

        case TL_KEY_EOF: {
            if (le->line->length > 0) {
                itl_le_erase_forward(le, 1);
            } else {
                itl_string_to_cstr(le->line, le->out_buf, le->out_size);
                return TL_PRESSED_EOF;
            }
        } break;

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
    return itl_enter_raw_mode();
}

int tl_exit(void)
{
    itl_global_history_free();
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

    itl_le_tty_refresh(&le);

    int in = itl_read_byte();
    int esc = itl_esc_parse(in);

    if (esc != TL_KEY_CHAR) {
        itl_global_last_control = esc;
        itl_le_clear(&le);
        fflush(stdout);
        return TL_PRESSED_CONTROL_SEQUENCE;
    }

    itl_utf8_t ch = itl_utf8_parse(in);
    itl_le_putc(&le, ch);
    itl_le_tty_refresh(&le);

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

    itl_le_tty_refresh(&le);

    int esc;
    int in;

    while (1) {
        in = itl_read_byte();

#if defined ITL_SEE_BYTES
        if (in == 3) exit(0);
        continue;
#endif /* TL_SEE_BYTES */

        esc = itl_esc_parse(in);

        if (esc != TL_KEY_CHAR) {
            int code = itl_le_esc_handle(&le, esc);

            if (code != TL_SUCCESS) {
                fflush(stdout);
                itl_le_clear(&le);
                return code;
            }
        } else {
            itl_utf8_t ch = itl_utf8_parse(in);
            itl_le_putc(&le, ch);
        }

        itl_le_tty_refresh(&le);
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
 *  - Tab completion.
 */
