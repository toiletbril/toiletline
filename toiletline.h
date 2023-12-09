/*
 *  toiletline 0.5.0
 *  Tiny single-header replacement of GNU Readline :3
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
 * resume normally on SIGCONT. This is the preferred way of doing SIGTSTP
 * without breaking terminal's state if you haven't called tl_exit yet. */
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
    #define TL_HISTORY_MAX_SIZE 64
#endif

#define TL_SUCCESS 0
/**
 * Codes which may be returned from reading functions.
 */
#define TL_PRESSED_ENTER 1
#define TL_PRESSED_INTERRUPT 2
#define TL_PRESSED_EOF 3
#define TL_PRESSED_SUSPEND 4
#define TL_PRESSED_CONTROL_SEQUENCE 5
/**
 * Codes below 0 are errors.
 */
#define TL_ERROR -1
#define TL_ERROR_SIZE -2
#define TL_ERROR_ALLOC -3

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

    TL_KEY_HISTORY_END,
    TL_KEY_HISTORY_BEGINNING,

    TL_KEY_END,
    TL_KEY_HOME,

    TL_KEY_ENTER,

    TL_KEY_BACKSPACE,
    TL_KEY_DELETE,
    TL_KEY_KILL_LINE,
    TL_KEY_KILL_LINE_BEFORE,

    TL_KEY_TAB,
    TL_KEY_CLEAR,

    TL_KEY_SUSPEND,
    TL_KEY_EOF,
    TL_KEY_INTERRUPT
} TL_KEY_KIND;

#define TL_MOD_CTRL  (1 << 24)
#define TL_MOD_SHIFT (1 << 25)
#define TL_MOD_ALT   (1 << 26)

#define TL_MASK_KEY 0x00FFFFFF
#define TL_MASK_MOD 0xFF000000

/* This is a helper and is not meant to be used directly. Use tl_last_control
 * instead. */
int *itl__last_control_location(void);

/**
 * Last pressed control sequence.
*/
#define tl_last_control (*itl__last_control_location())
/**
 * Initialize toiletline and put terminal in semi-raw mode.
 */
int tl_init(void);
/**
 * Exit toiletline and restore terminal state.
 */
int tl_exit(void);
/**
 * Read a character without waiting and modify `tl_last_control`.
 */
int tl_getc(char *char_buffer, size_t char_buffer_size, const char *prompt);
/**
 * Read input into the buffer.
 */
int tl_readline(char *buffer, size_t buffer_size, const char *prompt);
/**
 * Predefine input for `tl_readline()`.
 */
void tl_setline(const char *str);
/**
 * Returns the number of UTF-8 characters.
 *
 * Since number of bytes can be bigger than amount of characters, regular strlen
 * will not work, and will only return the number of bytes before \0.
 */
size_t tl_utf8_strlen(const char *utf8_str);

#endif /* TOILETLINE_H_ */

#if defined TOILETLINE_IMPLEMENTATION

#if defined _WIN32
    #define ITL_WIN32
#elif defined __linux__ || defined BSD || defined __APPLE__
    #define ITL_POSIX
#elif defined __COSMOCC__
    #define ITL_POSIX
#else /* __COSMOCC__ */
    #error "Your system is not supported"
#endif

#if defined _MSC_VER
    #define ITL_THREAD_LOCAL __declspec(thread)
    #define ITL_UNREACHABLE __assume(false)
#elif defined __GNUC__ || defined __clang__
    #define ITL_THREAD_LOCAL __thread
    #define ITL_UNREACHABLE __builtin_unreachable()
#elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L
    #define ITL_THREAD_LOCAL _Thread_local
    _Noreturn static void itl__unreachable() { while (true) {} }
    #define ITL_UNREACHABLE itl__unreachable()
#else /* __STDC_VERSION__ && __STDC_VERSION__ >= 201112L */
    #define ITL_THREAD_LOCAL /* nothing */
#endif

#if defined ITL_WIN32
    #define WIN32_LEAN_AND_MEAN
    #define _CRT_SECURE_NO_WARNINGS

    #include <windows.h>
    #include <conio.h>
    #include <fcntl.h>
    #include <io.h>

    #define STDIN_FILENO  _fileno(stdin)
    #define STDOUT_FILENO _fileno(stdout)

    /* <https://learn.microsoft.com/en-US/troubleshoot/windows-client/shell-experience/command-line-string-limitation> */
    #define ITL_MAX_STRING_LEN 8191

    #define isatty(fd) _isatty(fd)
    #define itl_read_byte_raw() _getch()
#elif defined ITL_POSIX
    #if !defined _DEFAULT_SOURCE
        #define _DEFAULT_SOURCE
    #endif

    #include <sys/ioctl.h>
    #include <termios.h>
    #include <unistd.h>

    /* <https://man7.org/linux/man-pages/man3/termios.3.html> */
    #define ITL_MAX_STRING_LEN 4095

    #define itl_read_byte_raw() fgetc(stdin)
#endif /* ITL_POSIX */

#if defined ITL_DEFAULT_ASSERT
    #include <assert.h>
    #define TL_ASSERT(boolval) assert(boolval)
#endif /* ITL_DEFAULT_ASSERT */

#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined ITL_DEBUG
    #define itl_trace(...)    fprintf(stderr, "[TRACE] " __VA_ARGS__)
    /* Print to stderr on the same line */
    #define itl_trace_sl(...) fprintf(stderr, __VA_ARGS__)
    /* Print LF to stderr */
    #define itl_trace_lf()    fprintf(stderr, "\n")
#else /* ITL_DEBUG */
    #define itl_trace(...)    /* nothing */
    #define itl_trace_sl(...) /* nothing */
    #define itl_trace_lf()    /* nothing */
#endif

#define ITL_MAX(type, i, j) ((((type)i) > ((type)j)) ? ((type)i) : ((type)j))
#define ITL_MIN(type, i, j) ((((type)i) < ((type)j)) ? ((type)i) : ((type)j))

#define ITL_TRY(boolval, return_error) \
    if (!(boolval)) return (return_error)

#if defined ITL_WIN32
static ITL_THREAD_LOCAL DWORD itl_global_original_tty_mode = 0;
static ITL_THREAD_LOCAL UINT itl_global_original_tty_cp    = 0;
static ITL_THREAD_LOCAL int itl_global_original_mode       = 0;
#elif defined ITL_POSIX
static ITL_THREAD_LOCAL struct termios itl_global_original_tty_mode = { 0 };
#endif

static bool itl_enter_raw_mode(void)
{
#if defined ITL_WIN32
    HANDLE stdin_handle;
    DWORD tty_mode;
    UINT codepage;
    int mode;

    stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
    ITL_TRY(stdin_handle != INVALID_HANDLE_VALUE, false);
    ITL_TRY(GetConsoleMode(stdin_handle, &tty_mode), false);

    itl_global_original_tty_mode = tty_mode;
    tty_mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT
                                    | ENABLE_PROCESSED_INPUT);

    ITL_TRY(SetConsoleMode(stdin_handle, tty_mode), false);

    codepage = GetConsoleCP();
    ITL_TRY(codepage != 0, false);

    itl_global_original_tty_cp = codepage;
    ITL_TRY(SetConsoleCP(CP_UTF8), false);

    mode = _setmode(STDIN_FILENO, _O_BINARY);
    ITL_TRY(mode == -1, false);

    itl_global_original_mode = mode;
#elif defined ITL_POSIX
    struct termios term;

    ITL_TRY(tcgetattr(STDIN_FILENO, &term) == 0, false);
    itl_global_original_tty_mode = term;
    cfmakeraw(&term);
    /* Map \r to each \n so cursor gets back to the beginning */
    term.c_oflag = OPOST | ONLCR;

    ITL_TRY(tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) == 0, false);
#endif /* ITL_POSIX */
    return true;
}

static bool itl_exit_raw_mode(void)
{
#if defined ITL_WIN32
    HANDLE stdin_handle = GetStdHandle(STD_INPUT_HANDLE);

    ITL_TRY(stdin_handle != INVALID_HANDLE_VALUE, false);
    ITL_TRY(SetConsoleMode(stdin_handle, itl_global_original_tty_mode), false);
    ITL_TRY(SetConsoleCP(itl_global_original_tty_cp), false);
    ITL_TRY(_setmode(STDIN_FILENO, itl_global_original_mode) != -1, false);

#elif defined ITL_POSIX
    ITL_TRY(tcsetattr(STDIN_FILENO, TCSAFLUSH,
                      &itl_global_original_tty_mode) == 0,
            false);
#endif /* ITL_POSIX */
    return true;
}

static bool itl_read_byte(uint8_t *buffer)
{
    int byte = itl_read_byte_raw();
    ITL_TRY(byte != EOF, false);
    (*buffer) = (uint8_t)byte;
    return true;
}

#define ITL_TRY_READ_BYTE(buffer, return_error) \
        ITL_TRY(itl_read_byte(buffer), return_error)

#if defined ITL_SUSPEND
#if defined ITL_POSIX
static void itl_handle_sigcont(int signal_number)
{
    (void)signal_number;
    itl_enter_raw_mode();
}
#endif /* ITL_POSIX */

/* Internally raise SIGTSTP and resume normally on SIGCONT, exit(1) on
   Windows */
static void itl_raise_suspend(void)
{
#if defined ITL_POSIX
    itl_exit_raw_mode();
    signal(SIGCONT, itl_handle_sigcont);
    raise(SIGTSTP);
#else /* ITL_POSIX */
    tl_exit();
    exit(1);
#endif
}
#endif /* ITL_SUSPEND */

static ITL_THREAD_LOCAL size_t itl_global_alloc_count = 0;

static void *itl_malloc(size_t size)
{
    void *allocated = TL_MALLOC(size);

#if !defined TL_NO_ABORT
    if (allocated == NULL) {
        TL_ABORT();
    }
#endif

    itl_global_alloc_count += 1;
    return allocated;
}

#if 0
static void *itl_calloc(size_t count, size_t size)
{
    void *allocated = itl_malloc(count * size);

#if !defined TL_NO_ABORT
    if (allocated == NULL) {
        TL_ABORT();
    }
#endif

    memset(allocated, 0, count * size);
    return allocated;
}
#endif /* if 0 */

static void *itl_realloc(void *block, size_t size)
{
    void *allocated;

    if (block == NULL) {
        itl_global_alloc_count += 1;
    }
    allocated = TL_REALLOC(block, size);

#if !defined TL_NO_ABORT
    if (allocated == NULL) {
        TL_ABORT();
    }
#endif

    return allocated;
}

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
    uint8_t bytes[4];
    size_t size;
};

static itl_utf8_t itl_utf8_new(const uint8_t *bytes, size_t size)
{
    size_t i;
    itl_utf8_t ch;

    for (i = 0; i < size; ++i) {
        ch.bytes[i] = bytes[i];
    }
    ch.size = size;

    return ch;
}

static void itl_utf8_copy(itl_utf8_t *dst, const itl_utf8_t *src)
{
    size_t i;
    for (i = 0; i < src->size; ++i) {
        dst->bytes[i] = src->bytes[i];
    }
    dst->size = src->size;
}

static size_t itl_utf8_width(int byte)
{
    if ((byte & 0x80) == 0) return 1;         /* 1 byte */
    else if ((byte & 0xE0) == 0xC0) return 2; /* 2 bytes */
    else if ((byte & 0xF0) == 0xE0) return 3; /* 3 bytes */
    else if ((byte & 0xF8) == 0xF0) return 4; /* 4 bytes */
    else return 0; /* invalid character */
}

#define itl_replacement_character \
    itl_utf8_new((uint8_t[]){ 0xEF, 0xBF, 0xBD }, 3)

static itl_utf8_t itl_utf8_parse(uint8_t first_byte)
{
    int i;
    size_t size;
    uint8_t bytes[4];

    size = itl_utf8_width(first_byte);
    if (size == 0) { /* invalid character */
        itl_trace_lf();
        itl_trace("Invalid UTF-8 sequence '%d'\n", (uint8_t)first_byte);
        return itl_replacement_character;
    }

    bytes[0] = first_byte;
    for (i = 1; i < (int)size; ++i) { /* consequent bytes */
        ITL_TRY_READ_BYTE(&bytes[i], itl_replacement_character);
    }

#if defined ITL_DEBUG
    itl_trace_lf();
    itl_trace("utf8 char size: %zu\n'", size);
    itl_trace("utf8 char bytes: '");

    for (i = 0; i < (int)size - 1; ++i) {
        itl_trace_sl("%02X ", bytes[i]);
    }
    itl_trace_sl("%02X'\n", bytes[size - 1]);
#endif /* ITL_DEBUG */

    return itl_utf8_new(bytes, size);
}

#define itl_utf8_free(c) itl_free(c)

#define ITL_STRING_INIT_SIZE 64
#define ITL_STRING_REALLOC_CAPACITY(old_capacity) \
    (((old_capacity) * 3) >> 1)

typedef struct itl_string itl_string_t;

struct itl_string
{
    itl_utf8_t *chars;
    size_t length;   /* N of chars in the string */
    size_t size;     /* N of bytes in all chars, size >= length */
    size_t capacity; /* N of chars this string can store */
};

static void itl_string_init(itl_string_t *str)
{
    str->length = 0;
    str->size = 0;

    str->capacity = ITL_STRING_INIT_SIZE;
    str->chars = (itl_utf8_t *)
        itl_malloc(str->capacity * sizeof(itl_utf8_t));
}

static itl_string_t *itl_string_alloc(void)
{
    itl_string_t *ptr = (itl_string_t *)itl_malloc(sizeof(itl_string_t));
    itl_string_init(ptr);
    return ptr;
}

static void itl_string_extend(itl_string_t *str)
{
    str->capacity = ITL_STRING_REALLOC_CAPACITY(str->capacity);
    str->chars = (itl_utf8_t *)
        itl_realloc(str->chars, str->capacity * sizeof(itl_utf8_t));
}

static bool itl_string_eq(itl_string_t *str1, itl_string_t *str2)
{
    size_t i;

    ITL_TRY(str1->size == str2->size, false);
    for (i = 0; i < str1->size; ++i) {
        ITL_TRY(str1->chars[i].size == str2->chars[i].size, false);
        ITL_TRY(memcmp(str2->chars[i].bytes, str2->chars[i].bytes,
                       4 * sizeof(uint8_t)) == 0,
                false);
    }
    return true;
}

static void itl_string_copy(itl_string_t *dst, const itl_string_t *src)
{
    size_t i;

    while (dst->capacity < src->capacity) itl_string_extend(dst);
    for (i = 0; i < src->length; ++i) {
        itl_utf8_copy(&dst->chars[i], &src->chars[i]);
    }
    dst->length = src->length;
    dst->size = src->size;
}

static void itl_string_recalc_size(itl_string_t *str)
{
    size_t i;

    TL_ASSERT(str->length <= ITL_MAX_STRING_LEN);

    str->size = 0;
    for (i = 0; i < str->length; ++i) {
        str->size += str->chars[i].size;
    }
}

/* Shrinks string to capacity of ITL_STRING_INIT_SIZE */
static void itl_string_shrink(itl_string_t *str)
{
    str->chars = (itl_utf8_t *)
        itl_realloc(str->chars, ITL_STRING_INIT_SIZE * sizeof(itl_utf8_t));
    str->capacity = ITL_STRING_INIT_SIZE;

    if (str->length > ITL_STRING_INIT_SIZE) {
        str->length = ITL_STRING_INIT_SIZE;
    }
    if (str->capacity < str->length) {
        str->length = str->capacity;
    }
    itl_string_recalc_size(str);
}

static void itl_string_clear(itl_string_t *str)
{
    str->size = 0;
    str->length = 0;
    itl_string_shrink(str);
}

static void itl_string_shift(itl_string_t *str, size_t position,
                             size_t shift_by, bool backwards)
{
    size_t i;

    TL_ASSERT(position <= str->length);

    /* When shifting back, loop from the specified position towards end and move
       characters back by shift_by. If shifting forward, loop from the end back
       to the position. */
    if (backwards) {
        for (i = position; i < str->length; ++i) {
            str->chars[i - shift_by] = str->chars[i];
        }

        TL_ASSERT(str->length >= shift_by);

        str->length -= shift_by;
    } else {
        str->length += shift_by;
        while (str->capacity < str->length) {
            itl_string_extend(str);
        }

        TL_ASSERT(str->length >= shift_by + 1);

        for (i = str->length - shift_by - 1; i >= position; --i) {
            str->chars[i + shift_by] = str->chars[i];
            if (i == 0) break; /* avoid wrapping */
        }
    }
}

static void itl_string_erase(itl_string_t *str, size_t position,
                             size_t count, bool backwards)
{
    itl_trace_lf();
    itl_trace("string_erase: pos: %zu, count: %zu, backwards: %d, len %zu\n",
              position, count, backwards, str->length);

    if (count > str->length) {
        count = str->length;
    }

    if (backwards) {
        if (position >= str->length) {
            /* Deleting at the start or at the end */
            str->length -= count;
            return;
        }
    } else {
        /* Do nothing on edge case */
        if (position >= str->length) {
            return;
        }
        position += count;
    }

    /* Erase the characters by shifting */
    itl_string_shift(str, position, count, true);
    itl_string_recalc_size(str);
}

static void itl_string_insert(itl_string_t *str, size_t position, itl_utf8_t ch)
{
    if (str->capacity < str->length + 1) {
        itl_string_extend(str);
    }

    if (position == str->length) {
        str->length += 1;
    } else {
        itl_string_shift(str, position, 1, false);
    }

    str->chars[position] = ch;
    itl_string_recalc_size(str);
}

#define itl_string_free(str)   \
    do {                       \
        itl_free(str->chars);  \
        itl_free(str);         \
    } while (0)

#if defined ITL_WIN32
    #define ITL_LF "\r\n"
    #define ITL_LF_LEN 2
#else /* ITL_WIN32 */
    #define ITL_LF "\n"
    #define ITL_LF_LEN 1
#endif

static bool itl_string_to_cstr(itl_string_t *str, char *c_str, size_t c_str_size)
{
    size_t i, j, k;

    k = 0;
    for (i = 0; i < str->length; ++i) {
        if (c_str_size - k - 1 < str->chars[i].size) break;
        for (j = 0; j != str->chars[i].size; ++j) {
            c_str[k++] = (char)str->chars[i].bytes[j];
        }
    }
    c_str[k] = '\0';

    if (k != str->size) {
        return false;
    }

    return true;
}

static void itl_string_from_cstr(itl_string_t *str, const char *c_str)
{
    size_t i, j, k, rune_width;

    k = 0;
    for (i = 0; c_str[k]; ++i) {
        while (str->capacity < i) itl_string_extend(str);
        rune_width = itl_utf8_width(c_str[k]);

        str->chars[i].size = rune_width;
        for (j = 0; j < rune_width && c_str[k]; ++j, ++k) {
            str->chars[i].bytes[j] = (uint8_t)c_str[k];
        }
    }

    str->length = i;
    itl_string_recalc_size(str);
}

typedef struct itl_history_item itl_history_item_t;

struct itl_history_item
{
    itl_string_t *str;
    itl_history_item_t *next;
    itl_history_item_t *prev;
};

static ITL_THREAD_LOCAL itl_history_item_t *itl_global_history = NULL;
static ITL_THREAD_LOCAL itl_history_item_t *itl_global_history_first = NULL;
static ITL_THREAD_LOCAL size_t itl_global_history_length = 0;

static ITL_THREAD_LOCAL itl_string_t itl_global_line_buffer = {0};

typedef struct itl_le itl_le_t;

/* Line editor */
struct itl_le
{
    itl_string_t *line;
    size_t cursor_position;

    itl_history_item_t *history_selected_item;

    char *out_buf;
    size_t out_size;
    const char *prompt;
};

static itl_history_item_t *itl_history_item_alloc(const itl_string_t *str)
{
    itl_history_item_t *item = (itl_history_item_t *)
        itl_malloc(sizeof(itl_history_item_t));

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

static void itl_global_history_free(void)
{
    itl_history_item_t *item;
    itl_history_item_t *prev_item;

    item = itl_global_history;
    if (item == NULL) return;

    while (item->next) {
        item = item->next;
    }
    while (item) {
        prev_item = item->prev;
        itl_history_item_free(item);
        item = prev_item;
    }

    itl_global_history = NULL;
}

static bool itl_global_history_append(itl_string_t *str)
{
    ITL_TRY(str->length <= 0, false);
    if (itl_global_history_length >= TL_HISTORY_MAX_SIZE) {
        if (itl_global_history_first) {
            itl_history_item_t *next_item = itl_global_history_first->next;
            itl_history_item_free(itl_global_history_first);

            itl_global_history_first = next_item;

            if (itl_global_history_first) {
                itl_global_history_first->prev = NULL;
            }

            --itl_global_history_length;
        }
    }

    if (itl_global_history == NULL) {
        itl_global_history = itl_history_item_alloc(str);
        itl_global_history_first = itl_global_history;
    }
    else {
        ITL_TRY(!itl_string_eq(itl_global_history->str, str), false);

        itl_history_item_t *item = itl_history_item_alloc(str);
        item->prev = itl_global_history;
        itl_global_history->next = item;
        itl_global_history = item;
    }

    ++itl_global_history_length;

    return true;
}

static itl_le_t itl_le_new(itl_string_t *line_buf, char *out_buf,
                           size_t out_size, const char *prompt)
{
    itl_le_t le = {
        /* .line                  = */ line_buf,
        /* .cursor_position       = */ line_buf->length,
        /* .history_selected_item = */ NULL,
        /* .out_buf               = */ out_buf,
        /* .out_size              = */ out_size,
        /* .prompt                = */ prompt,
    };

    return le;
}

static void itl_le_move_right(itl_le_t *le, size_t steps)
{
    if (le->cursor_position + steps >= le->line->length) {
        le->cursor_position = le->line->length;
    } else {
        le->cursor_position += steps;
    }
}

static void itl_le_move_left(itl_le_t *le, size_t steps)
{
    if (steps <= le->cursor_position) {
        le->cursor_position -= steps;
    } else {
        le->cursor_position = 0;
    }
}

static void itl_le_erase(itl_le_t *le, size_t count, bool backwards)
{
    if (count == 0) return;

    if (backwards && le->cursor_position) {
        itl_string_erase(le->line, le->cursor_position, count, true);
        itl_le_move_left(le, count);
    } else if (!backwards) {
        itl_string_erase(le->line, le->cursor_position, count, false);
    }
}

#define itl_le_erase_forward(le, count) itl_le_erase(le, count, false)
#define itl_le_erase_backward(le, count) itl_le_erase(le, count, true)

/* Inserts character at cursor position */
static bool itl_le_insert(itl_le_t *le, const itl_utf8_t ch)
{
    ITL_TRY(le->line->size + ch.size < le->out_size, false);

    itl_string_insert(le->line, le->cursor_position, ch);
    itl_le_move_right(le, 1);

    return true;
}

#define itl_is_delim(c) (ispunct(c) || isspace(c))

#define ITL_TOKEN_DELIM 0
#define ITL_TOKEN_WORD 1

/* Returns amount of steps required to reach a token */
static size_t itl_string_steps_to_token(itl_string_t *str, size_t position,
                                        int token, bool backwards)
{
    uint8_t byte;
    size_t i, steps;
    bool should_break;

    i = position;
    steps = 0;

    /* Prevent usage of uninitialized characters */
    if (backwards && i != 0 && i == str->length) {
        ++steps;
        --i;
    }

    while (true) {
        byte = str->chars[i].bytes[0];

        switch (token) {
            case ITL_TOKEN_DELIM: should_break = itl_is_delim(byte); break;
            case ITL_TOKEN_WORD: should_break = !itl_is_delim(byte); break;
            default: ITL_UNREACHABLE;
        }

        if (should_break) break;

        steps += 1;

        if (backwards && i > 0) {
            --i;
        } else if (!backwards && i < str->length - 1) {
            ++i;
        } else {
            break;
        }
    }

    return steps;
}

#define itl_le_steps_to_token(le, token, backwards) \
    itl_string_steps_to_token(le->line, le->cursor_position, token, backwards)

static void itl_le_clear(itl_le_t *le)
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

#define itl_tty_hide_cursor() fputs("\x1b[?25l", stdout)
#define itl_tty_show_cursor() fputs("\x1b[?25h", stdout)

#define itl_tty_move_to_column(col) printf("\x1b[%zuG", (size_t)col)
#define itl_tty_move_forward(steps) printf("\x1b[%zuC", (size_t)steps)

#define itl_tty_move_up(rows) printf("\x1b[%zuA", (size_t)rows)
#define itl_tty_move_down(rows) printf("\x1b[%zuB", (size_t)rows)

#define itl_tty_clear_whole_line() fputs("\r\x1b[0K", stdout)
#define itl_tty_clear_to_end() fputs("\x1b[K", stdout)

#define itl_tty_goto_home() fputs("\x1b[H", stdout)
#define itl_tty_erase_screen() fputs("\033[2J", stdout)

#define itl_tty_status_report() fputs("\x1b[6n", stdout)

static bool itl_tty_get_size(size_t *rows, size_t *cols) {
#if defined TL_SIZE_USE_ESCAPES
    char buf[32];
    size_t i;

    itl_tty_move_forward(999);
    itl_tty_status_report();

    /* This does not work without flushing if setvbuf was called previously */
    fflush(stdout);

    i = 0;
    while (i < sizeof(buf) - 1) {
        ITL_TRY_READ_BYTE(&buf[i], false);
        if (buf[i] == 'R') break;
        ++i;
    }
    buf[i] = '\0';

    ITL_TRY(buf[0] != '\x1b' || buf[1] != '[', false);
    ITL_TRY(sscanf(&buf[2], "%zu;%zu", rows, cols) != 2, false);

#elif defined ITL_WIN32
    CONSOLE_SCREEN_BUFFER_INFO buffer_info;

    ITL_TRY(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
                                       &buffer_info),
            false);

    (*cols) = buffer_info.srWindow.Right - buffer_info.srWindow.Left + 1;
    (*rows) = buffer_info.srWindow.Bottom - buffer_info.srWindow.Top + 1;

#elif defined ITL_POSIX
    struct winsize window;
    ITL_TRY(ioctl(STDOUT_FILENO, TIOCGWINSZ, &window) == 0, false);

    (*rows) = (size_t)window.ws_row;
    (*cols) = (size_t)window.ws_col;
#else /* ITL_POSIX */
    ITL_UNREACHABLE;
#endif
    return true;
}

static ITL_THREAD_LOCAL size_t itl_global_tty_prev_lines = 1;
static ITL_THREAD_LOCAL size_t itl_global_tty_prev_wrap_row = 1;

static bool itl_le_tty_refresh(itl_le_t *le)
{
    size_t i, j;
    size_t rows, cols;
    size_t current_col, current_lines, dirty_lines;
    size_t prompt_len, wrap_cursor_col, wrap_cursor_row;

    TL_ASSERT(le->line);
    TL_ASSERT(le->line->chars);
    TL_ASSERT(le->line->size >= le->line->length);
    TL_ASSERT(le->line->length <= ITL_MAX_STRING_LEN);

    itl_tty_hide_cursor();

    rows = 0;
    cols = 0;
    ITL_TRY(itl_tty_get_size(&rows, &cols), false);

    prompt_len = (le->prompt) ? strlen(le->prompt) : 0;

    current_lines =
        (le->line->length + prompt_len) / ITL_MAX(size_t, cols, 1) + 1;

    wrap_cursor_col =
        (le->cursor_position + prompt_len) % ITL_MAX(size_t, cols, 1) + 1;
    wrap_cursor_row =
        (le->cursor_position + prompt_len) / ITL_MAX(size_t, cols, 1) + 1;

    itl_trace("wrow: %zu, prev: %zu, col: %zu, curp: %zu\n",
              wrap_cursor_row, itl_global_tty_prev_wrap_row,
              wrap_cursor_col, le->cursor_position);

    /* Move appropriate amount of lines back, while clearing previous output */
    for (i = 0; i < itl_global_tty_prev_lines; ++i) {
        itl_tty_clear_whole_line();
        if (i < itl_global_tty_prev_wrap_row - 1) {
            itl_tty_move_up(1);
        }
    }

    if (le->prompt) {
        fputs(le->prompt, stdout);
    }

    /* Print current contents of the line editor */
    for (i = 0; i < le->line->length; ++i) {
        for (j = 0; j < le->line->chars[i].size; ++j) {
            fputc(le->line->chars[i].bytes[j], stdout);
        }

        /* If line is full, wrap */
        current_col = (prompt_len + i) % ITL_MAX(size_t, cols, 1);
        if (current_col == cols - 1) {
            fputs(ITL_LF, stdout);
        }
    }

    /* If current amount of lines is less than previous amount of lines, then
       input was cleared by kill line or such. Clear each dirty line, then go
       back up */
    if (current_lines < itl_global_tty_prev_lines) {
        dirty_lines = itl_global_tty_prev_lines - current_lines;
        for (i = 0; i < dirty_lines; ++i) {
            itl_tty_move_down(1);
            itl_tty_clear_whole_line();
        }
        itl_tty_move_up(dirty_lines);
    }
    /* Otherwise clear to the end of line */
    else {
        itl_tty_clear_to_end();
    }

    /* Move cursor to appropriate row and column. If row didn't change, stay on
       the same line */
    if (wrap_cursor_row < current_lines) {
        itl_tty_move_up(current_lines - wrap_cursor_row);
    }
    itl_tty_move_to_column(wrap_cursor_col);

    itl_global_tty_prev_lines = current_lines;
    itl_global_tty_prev_wrap_row = wrap_cursor_row;

    itl_tty_show_cursor();
    fflush(stdout);

    return true;
}

#ifdef ITL_POSIX
static int itl_esc_parse_posix(uint8_t byte)
{
    bool read_mod = false;
    int event = 0;

    if (byte == 27) { /* esc */
        ITL_TRY_READ_BYTE(&byte, TL_KEY_UNKN);

        if (byte != '[' && byte != 'O') {
            switch (byte) {
                case 'b': return TL_KEY_LEFT  | TL_MOD_CTRL;
                case 'f': return TL_KEY_RIGHT | TL_MOD_CTRL;

                case 'd': return TL_KEY_DELETE | TL_MOD_CTRL;
                case 'h': return TL_KEY_BACKSPACE | TL_MOD_CTRL;

                case '.':
                case '>': return TL_KEY_HISTORY_END;
                case ',':
                case '<': return TL_KEY_HISTORY_BEGINNING;

                default: return TL_KEY_CHAR | TL_MOD_ALT;
            }
        }

        ITL_TRY_READ_BYTE(&byte, TL_KEY_UNKN);

        if (byte == '1') {
            ITL_TRY_READ_BYTE(&byte, TL_KEY_UNKN);
            ITL_TRY(byte == ';', TL_KEY_UNKN);
            ITL_TRY_READ_BYTE(&byte, TL_KEY_UNKN);
            switch (byte) {
                case '2': event |= TL_MOD_SHIFT; break;
                case '5': event |= TL_MOD_CTRL;  break;
            }

            read_mod = true;
            ITL_TRY_READ_BYTE(&byte, TL_KEY_UNKN);
        }

        switch (byte) {
            case 'A': return event | TL_KEY_UP;
            case 'B': return event | TL_KEY_DOWN;
            case 'C': return event | TL_KEY_RIGHT;
            case 'D': return event | TL_KEY_LEFT;

            case 'F': return event | TL_KEY_END;
            case 'H': return event | TL_KEY_HOME;

            case '3': event |= TL_KEY_DELETE; break;

            default: event |= TL_KEY_UNKN;
        }
    } else {
        ITL_TRY(!iscntrl(byte), TL_KEY_UNKN);
        return TL_KEY_CHAR;
    }

    if (!read_mod) {
        ITL_TRY_READ_BYTE(&byte, TL_KEY_UNKN);

        if (byte == ';') {
            ITL_TRY_READ_BYTE(&byte, TL_KEY_UNKN);
            switch (byte) {
                case '3': event |= TL_MOD_SHIFT; break;
                case '5': event |= TL_MOD_CTRL;  break;
            }
            ITL_TRY_READ_BYTE(&byte, TL_KEY_UNKN);
        }

        ITL_TRY(byte == '~', TL_KEY_UNKN);
    }

    return event;
}
#endif /* ITL_POSIX */

#ifdef ITL_WIN32
static int itl_esc_parse_win32(uint8_t byte)
{
    int event = 0;

    if (byte == 224) { /* esc */
        ITL_TRY_READ_BYTE(&byte, TL_KEY_UNKN);
        switch (byte) {
            case 'H': event = TL_KEY_UP;    break;
            case 'P': event = TL_KEY_DOWN;  break;
            case 'K': event = TL_KEY_LEFT;  break;
            case 'M': event = TL_KEY_RIGHT; break;

            case 's': event = TL_KEY_LEFT  | TL_MOD_CTRL; break;
            case 't': event = TL_KEY_RIGHT | TL_MOD_CTRL; break;

            case 'G': event = TL_KEY_HOME; break;
            case 'O': event = TL_KEY_END;  break;

            case 147: event = TL_KEY_DELETE | TL_MOD_CTRL; break;
            case 'S': event = TL_KEY_DELETE; break;

            default: event = TL_KEY_UNKN;
        }
    } else {
        ITL_TRY(!iscntrl(byte), TL_KEY_UNKN);
        return TL_KEY_CHAR;
    }

    return event;
}
#endif /* ITL_WIN32 */

static int itl_esc_parse(uint8_t byte)
{
    /* plain bytes */
    switch (byte) {
        case 1: return TL_KEY_HOME; /* ctrl a */
        case 5: return TL_KEY_END;  /* ctrl e */

        case 2: return TL_KEY_LEFT;  /* ctrl f */
        case 6: return TL_KEY_RIGHT; /* ctrl b */

        case 3:  return TL_KEY_INTERRUPT; /* ctrl c */
        case 4:  return TL_KEY_EOF;       /* ctrl d */
        case 26: return TL_KEY_SUSPEND;   /* ctrl z */

        case 9: return  TL_KEY_TAB;
        case 12: return TL_KEY_CLEAR; /* ctrl l */

        case 14: return TL_KEY_DOWN; /* ctrl n */
        case 16: return TL_KEY_UP;   /* ctrl p */

        case 13: /* cr */
        case 10: return TL_KEY_ENTER;

        case 11: return TL_KEY_KILL_LINE;        /* ctrl k */
        case 21: return TL_KEY_KILL_LINE_BEFORE; /* ctrl u */
        case 23: return TL_KEY_BACKSPACE | TL_MOD_CTRL;

        case 8: /* old backspace */
        case 127: return TL_KEY_BACKSPACE;
    }

#if defined ITL_WIN32
    return itl_esc_parse_win32(byte);
#elif defined ITL_POSIX
    return itl_esc_parse_posix(byte);
#endif /* ITL_POSIX */
}

static ITL_THREAD_LOCAL int itl_global_last_control = TL_KEY_UNKN;

int *itl__last_control_location(void) {
    return &itl_global_last_control;
}

static int itl_le_key_handle(itl_le_t *le, int esc)
{
    size_t i, steps;

    tl_last_control = esc;
    switch (esc & TL_MASK_KEY) {
        case TL_KEY_UP: {
            itl_global_history_get_prev(le);
        } break;

        case TL_KEY_DOWN: {
            itl_global_history_get_next(le);
        } break;

        case TL_KEY_RIGHT: {
            if (le->cursor_position < le->line->length) {
                if (esc & TL_MOD_CTRL) {
                    steps = itl_le_steps_to_token(le, ITL_TOKEN_DELIM, false);
                    if (steps != 0) {
                        itl_le_move_right(le, steps);
                    } else {
                        steps = itl_le_steps_to_token(le, ITL_TOKEN_WORD, false);
                        itl_le_move_right(le, steps);
                        steps = itl_le_steps_to_token(le, ITL_TOKEN_DELIM, false);
                        itl_le_move_right(le, steps);
                    }
                } else {
                    itl_le_move_right(le, 1);
                }
            }
        } break;

        case TL_KEY_LEFT: {
            if (le->cursor_position > 0 &&
                le->cursor_position <= le->line->length) {
                if (esc & TL_MOD_CTRL) {
                    steps = itl_le_steps_to_token(le, ITL_TOKEN_DELIM, true);
                    if (steps > 1) {
                        itl_le_move_left(le, steps - 1);
                    } else {
                        itl_le_move_left(le, steps);
                        steps = itl_le_steps_to_token(le, ITL_TOKEN_WORD, true);
                        itl_le_move_left(le, steps);
                        steps = itl_le_steps_to_token(le, ITL_TOKEN_DELIM, true);
                        if (steps > 0) {
                            itl_le_move_left(le, steps - 1);
                        }
                    }
                } else {
                    itl_le_move_left(le, 1);
                }
            }
        } break;

        case TL_KEY_END: {
            itl_le_move_right(le, le->line->length - le->cursor_position);
        } break;

        case TL_KEY_HOME: {
            itl_le_move_left(le, le->cursor_position);
        } break;

        case TL_KEY_ENTER: {
            ITL_TRY(itl_string_to_cstr(le->line, le->out_buf, le->out_size),
                    TL_ERROR_SIZE);
            itl_global_history_append(le->line);
            return TL_PRESSED_ENTER;
        } break;

        case TL_KEY_BACKSPACE: {
            if (esc & TL_MOD_CTRL) {
                steps = itl_le_steps_to_token(le, ITL_TOKEN_DELIM, true);
                if (steps > 1) {
                    itl_le_erase_backward(le, steps - 1);
                } else {
                    itl_le_erase_backward(le, steps);
                    steps = itl_le_steps_to_token(le, ITL_TOKEN_WORD, true);
                    steps += 
                        itl_string_steps_to_token(le->line,
                                                  le->cursor_position - steps, 
                                                  ITL_TOKEN_DELIM, true);
                    if (steps > 0) {
                        itl_le_erase_backward(le, steps - 1);
                    }
                }
            } else {
                itl_le_erase_backward(le, 1);
            }
        } break;

        case TL_KEY_DELETE: {
            if (esc & TL_MOD_CTRL) {
                steps = itl_le_steps_to_token(le, ITL_TOKEN_DELIM, false);
                if (steps > 1) {
                    itl_le_erase_forward(le, steps);
                } else {
                    itl_le_erase_forward(le, steps);
                    steps = itl_le_steps_to_token(le, ITL_TOKEN_WORD, false);
                    steps += 
                        itl_string_steps_to_token(le->line,
                                                  le->cursor_position + steps, 
                                                  ITL_TOKEN_DELIM, false);
                    itl_le_erase_forward(le, steps);
                }
            } else {
                itl_le_erase_forward(le, 1);
            }
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

        case TL_KEY_CLEAR: {
            itl_tty_goto_home();
            itl_tty_erase_screen();
            itl_le_tty_refresh(le);
        } break;

        case TL_KEY_HISTORY_END: {
            for (i = 0; i < itl_global_history_length; ++i) {
                itl_global_history_get_next(le);
            }
            itl_global_history_get_prev(le);
        } break;

        case TL_KEY_HISTORY_BEGINNING: {
            for (i = 0; i < itl_global_history_length; ++i) {
                itl_global_history_get_prev(le);
            }
        } break;
    }

    return TL_SUCCESS;
}

static ITL_THREAD_LOCAL bool itl_is_active = false;

int tl_init(void)
{
    TL_ASSERT(TL_HISTORY_MAX_SIZE % 2 == 0 && "History size must be a power of 2");

    ITL_TRY(isatty(STDIN_FILENO), TL_ERROR);
    ITL_TRY(itl_enter_raw_mode(), TL_ERROR);

    itl_string_init(&itl_global_line_buffer);

    itl_is_active = true;
    return TL_SUCCESS;
}

int tl_exit(void)
{
    itl_global_history_free();
    itl_free(itl_global_line_buffer.chars);

    itl_trace("Exited, alloc count: %zu\n", itl_global_alloc_count);
    TL_ASSERT(itl_global_alloc_count == 0);

    ITL_TRY(itl_exit_raw_mode(), TL_ERROR);

    itl_is_active = false;
    return TL_SUCCESS;
}

int tl_getc(char *char_buffer, size_t char_buffer_size, const char *prompt)
{
    itl_le_t le;
    itl_utf8_t ch;
    int input_type;
    uint8_t input_byte;

    TL_ASSERT(itl_is_active && "tl_init() should be called");
    TL_ASSERT(char_buffer_size > 1 &&
        "Size should be enough at least for one byte and a null terminator");
    TL_ASSERT(char_buffer_size <= sizeof(char)*5 &&
        "Size should be less or equal to size of 4 characters with a null "
        "terminator.");
    TL_ASSERT(char_buffer != NULL);

    le = itl_le_new(&itl_global_line_buffer, char_buffer,
                             char_buffer_size, prompt);

    /* Avoid overriding buffer if tl_setline was used */
    if (itl_global_line_buffer.length != 0)
        itl_string_clear(&itl_global_line_buffer);

    itl_le_tty_refresh(&le);
    ITL_TRY_READ_BYTE(&input_byte, TL_ERROR);

    input_type = itl_esc_parse(input_byte);
    if (input_type != TL_KEY_CHAR) {
        tl_last_control = input_type;
        return TL_PRESSED_CONTROL_SEQUENCE;
    }

    ch = itl_utf8_parse(input_byte);
    itl_le_insert(&le, ch);
    itl_le_tty_refresh(&le);
    itl_string_to_cstr(le.line, char_buffer, char_buffer_size);
    itl_le_clear(&le);

    return TL_SUCCESS;
}

int tl_readline(char *buffer, size_t buffer_size, const char *prompt)
{
    itl_le_t le;
    itl_utf8_t ch;
    uint8_t input_byte;
    int input_type, code;

    TL_ASSERT(itl_is_active && "tl_init() should be called");
    TL_ASSERT(buffer_size > 1 &&
        "Size should be enough at least for one byte and a null terminator");
    TL_ASSERT(buffer_size <= ITL_MAX_STRING_LEN &&
        "Size should be less than platform's allowed maximum string length");
    TL_ASSERT(buffer != NULL);

    le = itl_le_new(&itl_global_line_buffer, buffer,
                             buffer_size, prompt);
    itl_le_tty_refresh(&le);

    while (true) {
        ITL_TRY_READ_BYTE(&input_byte, TL_ERROR);

#if defined ITL_SEE_BYTES
        if (input_byte == 3) exit(0); /* ctrl c */
        printf("%d\n", input_byte);
        continue;
#endif /* TL_SEE_BYTES */

        input_type = itl_esc_parse(input_byte);
        if (input_type != TL_KEY_CHAR) {
            code = itl_le_key_handle(&le, input_type);
            if (code != TL_SUCCESS) {
                itl_le_clear(&le);
                return code;
            }
        } else {
            ch = itl_utf8_parse(input_byte);
            itl_le_insert(&le, ch);
        }

        itl_trace_lf();
        itl_trace("strlen: %zu, hist: %zu\n",
                  le.line->length, (size_t)le.history_selected_item);

        itl_le_tty_refresh(&le);
    }

    return TL_ERROR;
}

size_t tl_utf8_strlen(const char *utf8_str)
{
    size_t len = 0;
    while (*utf8_str) {
        if ((*utf8_str & 0xC0) != 0x80)
            ++len;
        ++utf8_str;
    }
    return len;
}

void tl_setline(const char *str)
{
    TL_ASSERT(itl_is_active && "tl_init() should be called");
    itl_string_shrink(&itl_global_line_buffer);
    itl_string_from_cstr(&itl_global_line_buffer, str);
}

#endif /* TOILETLINE_IMPLEMENTATION */

#if defined __cplusplus
}
#endif

/*
 * TODO:
 *  - itl_global_history_get_prev(): If there is any text in the current line,
 *    append it to global history.
 *  - itl_utf8_parse(): Codepoints U+D800 to U+DFFF (known as UTF-16 surrogates)
 *    are invalid.
 *  - Write and document tests.
 *  - Tab completion.
 *  - Introduce TL_DEF and ITL_DEF macros.
 */
