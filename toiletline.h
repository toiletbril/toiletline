/*
 *  toiletline 0.5.2
 *  Small single-header replacement of GNU Readline :3
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

#if defined _WIN32
    #define _CRT_SECURE_NO_WARNINGS
#endif /* _WIN32 */

#include <stddef.h>

/* If not defined, Ctrl-Z will raise `SIGTSTP` internally and toiletline will
 * resume normally on SIGCONT. This is the preferred way of doing `SIGTSTP`
 * without breaking terminal's state if you haven't called tl_exit yet. */
#if !defined TL_NO_SUSPEND
    #define ITL_SUSPEND
#endif /* !TL_NO_SUSPEND */

/* To use custom assertions or to disable them, you can define `TL_ASSERT` to
 * some other function or nothing before including. */
#if !defined TL_ASSERT
    #define ITL_DEFAULT_ASSERT
#endif /* !TL_ASSERT */

/* Replaceable macros that are used to allocate memory. */
#if !defined TL_MALLOC
    #define TL_MALLOC(size)         malloc(size)
    #define TL_REALLOC(block, size) realloc(block, size)
    #define TL_FREE(ptr)            free(ptr)
    /* Will be called on failed allocation. `TL_NO_ABORT` can be defined to
     * disable failure checking. */
    #define TL_ABORT() abort()
#endif /* !TL_MALLOC */

/* Macros that are placed before definitions. */
#if !defined TL_DEF
    /* Public prototypes */
    #define TL_DEF extern
#endif /* !TL_DEF */

#if !defined ITL_DEF
    /* Internal definitions */
    #define ITL_DEF static
#endif /* !ITL_DEF */

/* Max size of in-memory history, must be a power of 2. */
#if !defined TL_HISTORY_MAX_SIZE
    #define TL_HISTORY_MAX_SIZE 64
#endif /* TL_HISTORY_MAX_SIZE */

#define TL_SUCCESS 0
/**
 * Codes which may be returned from reading functions.
 */
#define TL_PRESSED_ENTER 1
#define TL_PRESSED_INTERRUPT 2
#define TL_PRESSED_EOF 3
#define TL_PRESSED_SUSPEND 4
#define TL_PRESSED_CONTROL_SEQUENCE 5
#if defined TL_MANUAL_TAB_COMPLETION
    #define TL_PRESSED_TAB 6
#endif /* TL_MANUAL_TAB_COMPLETION */

/**
 * Codes below 0 are errors.
 */
#define TL_ERROR -1
#define TL_ERROR_SIZE -2
#define TL_ERROR_ALLOC -3

/**
 * Control sequences.
 * Last control sequence used will be stored in `tl_last_control`.
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

/* This is a helper and is not meant to be used directly. Use `tl_last_control`
 * instead. */
TL_DEF int *itl__last_control_location(void);

/**
 * Last pressed control sequence.
*/
#define tl_last_control (*itl__last_control_location())
/**
 * Initialize toiletline and put terminal in semi-raw mode.
 */
TL_DEF int tl_init(void);
/**
 * Exit toiletline and restore terminal state.
 */
TL_DEF int tl_exit(void);
/**
 * Read input into the buffer.
 */
TL_DEF int tl_readline(char *buffer, size_t buffer_size, const char *prompt);
/**
 * Predefine input for `tl_readline()`.
 */
TL_DEF void tl_setline(const char *str);
/**
 * Read a character without waiting and modify `tl_last_control`.
 */
TL_DEF int tl_getc(char *char_buffer, size_t char_buffer_size,
                   const char *prompt);
/**
 * Load history from a file.
 *
 * Returns `TL_SUCCESS`, `-EINVAL` if file is invalid or `-errno` on other
 * failures.
 */
TL_DEF int tl_load_history(const char *file_path);
/**
 * Dump history to a file, overwriting it.
 *
 * Returns `TL_SUCCESS`, `-EINVAL` if file is invalid or `-errno` on other
 * failures.
 */
TL_DEF int tl_dump_history(const char *file_path);
/**
 * Returns the number of UTF-8 characters.
 *
 * Since number of bytes can be bigger than amount of characters, regular
 * `strlen()` will not work, and will only return the number of bytes before \0.
 */
TL_DEF size_t tl_utf8_strlen(const char *utf8_str);

#if !defined TL_MANUAL_TAB_COMPLETION
/**
 * Add a tab completion.
 *
 * Returns an opaque pointer that points to the added completion. Use it as
 * `*prefix` parameter to add further completions. If `*prefix` is NULL, adds a
 * root completion.
 */
TL_DEF void *tl_add_completion(void *prefix, const char *label);
/**
 *  Change a tab completion to `*label` using pointer returned from
 * `tl_add_completion()`.
 */
TL_DEF void tl_change_completion(void *completion, const char *label);
/**
 * Delete a tab completion and it's children using pointer returned from
 * `tl_add_completion()`.
 */
TL_DEF void tl_delete_completion(void *completion);
#endif /* !TL_MANUAL_TAB_COMPLETION */

#if !defined TOILETLINE_IMPLEMENTATION && defined _CRT_SECURE_NO_WARNINGS
    #undef _CRT_SECURE_NO_WARNINGS
#endif /* !TOILETLINE_IMPLEMENTATION && _CRT_SECURE_NO_WARNINGS */

#endif /* TOILETLINE_H_ */ /* End of header file */

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
    #define ITL_DEBUG_TRAP __debugbreak()
#elif defined __GNUC__ || defined __clang__
    #define ITL_THREAD_LOCAL __thread
    #define ITL_UNREACHABLE __builtin_unreachable()
    #define ITL_DEBUG_TRAP __builtin_trap()
#elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L
    #define ITL_THREAD_LOCAL _Thread_local
    _Noreturn ITL_DEF void itl__unreachable() { while (true); }
    #define ITL_UNREACHABLE itl__unreachable()
    #define ITL_DEBUG_TRAP abort()
#else /* __STDC_VERSION__ && __STDC_VERSION__ >= 201112L */
    #define ITL_THREAD_LOCAL /* nothing */
    #define ITL_UNREACHABLE  /* nothing */
    #define ITL_DEBUG_TRAP abort()
#endif

#if defined ITL_WIN32
    #define WIN32_LEAN_AND_MEAN

    #include <windows.h>
    #include <conio.h>
    #include <fcntl.h>
    #include <io.h>

    #define STDIN_FILENO  _fileno(stdin)
    #define STDOUT_FILENO _fileno(stdout)

    #define write(fd, buf, size) _write(fd, buf, (unsigned long)(size))

    /* <https://learn.microsoft.com/en-US/troubleshoot/windows-client/shell-experience/command-line-string-limitation> */
    #define ITL_STRING_MAX_LEN 8191

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
    #define ITL_STRING_MAX_LEN 4095

    ITL_DEF int itl_read_byte_raw()
    {
        int buf[1];
        if (read(STDIN_FILENO, buf, 1) != 1) {
            return -1;
        }
        return *buf;
    }
#endif /* ITL_POSIX */

#if defined ITL_DEFAULT_ASSERT
    /* Fatal error :[ */
    #define TL_ASSERT(boolval)                           \
        if (!(boolval)) {                                \
            fprintf(stderr, "%s:%d: assert fail: %s\n",  \
                    __FILE__, __LINE__, #boolval);       \
            fflush(stderr);                              \
            ITL_DEBUG_TRAP;                              \
        }
#endif /* ITL_DEFAULT_ASSERT */

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined ITL_DEBUG
    #define itl_traceln(...)  fprintf(stderr, "\n[TRACE] " __VA_ARGS__)
    /* Print to stderr on the same line */
    #define itl_trace(...) fprintf(stderr, __VA_ARGS__)
#else /* ITL_DEBUG */
    #define itl_traceln(...) /* nothing */
    #define itl_trace(...) /* nothing */
#endif

#define ITL_MAX(type, i, j) ((((type)i) > ((type)j)) ? ((type)i) : ((type)j))
#define ITL_MIN(type, i, j) ((((type)i) < ((type)j)) ? ((type)i) : ((type)j))

#define ITL__TRY(boolval, expr)                    \
    if (!(boolval)) {                              \
        itl_traceln("%s:%d: try fail: %s\n",       \
                    __FILE__, __LINE__, #boolval); \
        expr;                                      \
    }
#define ITL_TRY(boolval, return_error) \
    ITL__TRY(boolval, return (return_error))

#if defined ITL_WIN32
ITL_DEF ITL_THREAD_LOCAL DWORD itl_global_original_tty_mode = 0;
ITL_DEF ITL_THREAD_LOCAL UINT itl_global_original_tty_cp    = 0;
ITL_DEF ITL_THREAD_LOCAL int itl_global_original_mode       = 0;
#elif defined ITL_POSIX
ITL_DEF ITL_THREAD_LOCAL struct termios itl_global_original_tty_mode = {0};
#endif

ITL_DEF bool itl_enter_raw_mode_impl(void)
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
    tty_mode &= (DWORD)
        ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);

    ITL_TRY(SetConsoleMode(stdin_handle, tty_mode), false);

    codepage = GetConsoleCP();
    ITL_TRY(codepage != 0, false);

    itl_global_original_tty_cp = codepage;
    ITL_TRY(SetConsoleCP(CP_UTF8), false);

    mode = _setmode(STDIN_FILENO, _O_BINARY);
    ITL_TRY(mode != -1, false);

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

ITL_DEF bool itl_exit_raw_mode(void)
{
#if defined ITL_WIN32
    bool something_failed = false;
    HANDLE stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
    ITL_TRY(stdin_handle != INVALID_HANDLE_VALUE, false);

    if (itl_global_original_tty_mode != 0) {
        ITL__TRY(SetConsoleMode(stdin_handle, itl_global_original_tty_mode),
                 something_failed = true);
    }
    if (itl_global_original_tty_cp != 0) {
       ITL__TRY(SetConsoleCP(itl_global_original_tty_cp),
                something_failed = true);
    }
    if (itl_global_original_mode != 0) {
        ITL__TRY(_setmode(STDIN_FILENO, itl_global_original_mode) != -1,
                 something_failed = true);
    }
    return (something_failed == false);
#elif defined ITL_POSIX
    struct termios zeroed_termios = {0};
    if (memcmp(&itl_global_original_tty_mode, &zeroed_termios,
               sizeof(struct termios)) != 0) {
        ITL_TRY(tcsetattr(STDIN_FILENO, TCSAFLUSH,
                          &itl_global_original_tty_mode) == 0,
                false);
    }
    return true;
#endif /* ITL_POSIX */
}

ITL_DEF bool itl_enter_raw_mode(void)
{
    /* If raw mode failed, restore terminal's state */
    if (!itl_enter_raw_mode_impl()) {
        itl_exit_raw_mode();
        return false;
    }
    return true;
}

ITL_DEF bool itl_read_byte(uint8_t *buffer)
{
    int byte = itl_read_byte_raw();
    /* Catch `read()` errors. `getch()` on Windows does not have error
       returns */
    ITL_TRY(byte != -1, false);
    /* I think `read()` is not supposed to return EOF in the context of a
       terminal session */
    ITL_TRY(byte != EOF, false);
    (*buffer) = (uint8_t)byte;
    return true;
}

#define ITL_TRY_READ_BYTE(buffer, return_error) \
        ITL_TRY(itl_read_byte(buffer), return_error)

#if defined ITL_SUSPEND
#if defined ITL_POSIX
ITL_DEF void itl_handle_sigcont(int signal_number)
{
    (void)signal_number;
    itl_enter_raw_mode();
}
#endif /* ITL_POSIX */

/* Internally raise SIGTSTP and resume normally on SIGCONT, exit(1) on
   Windows */
ITL_DEF void itl_raise_suspend(void)
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

ITL_DEF ITL_THREAD_LOCAL size_t itl_global_alloc_count = 0;

ITL_DEF void *itl_malloc(size_t size)
{
    void *allocated = TL_MALLOC(size);
    itl_global_alloc_count += 1;

#if !defined TL_NO_ABORT
    if (allocated == NULL) {
        TL_ABORT();
    }
#endif /* !TL_NO_ABORT */

    return allocated;
}

#if 0
ITL_DEF void *itl_calloc(size_t count, size_t size)
{
    void *allocated = TL_MALLOC(size);
    itl_global_alloc_count += 1;

#if !defined TL_NO_ABORT
    if (allocated == NULL) {
        TL_ABORT();
    }
#endif /* !TL_NO_ABORT */

    memset(allocated, 0, count * size);

    return allocated;
}
#endif /* if 0 */

ITL_DEF void *itl_realloc(void *block, size_t size)
{
    void *allocated;

    if (block == NULL) {
        allocated = TL_MALLOC(size);
        itl_global_alloc_count += 1;
    } else {
        allocated = TL_REALLOC(block, size);
    }

#if !defined TL_NO_ABORT
    if (allocated == NULL) {
        TL_ABORT();
    }
#endif /* !TL_NO_ABORT */

    return allocated;
}

#if defined ITL_DEBUG
#define itl_free(ptr)                 \
    do {                              \
        TL_ASSERT(ptr != NULL);       \
        memset(ptr, 0, sizeof(*ptr)); \
        TL_FREE(ptr);                 \
        itl_global_alloc_count -= 1;  \
        ptr = NULL;                   \
    } while (0)
#else /* ITL_DEBUG */
#define itl_free(ptr)                \
    do {                             \
        itl_global_alloc_count -= 1; \
        TL_FREE(ptr);                \
    } while (0)
#endif

typedef struct itl_utf8 itl_utf8_t;

struct itl_utf8
{
    uint8_t bytes[4];
    size_t size;
};

ITL_DEF itl_utf8_t itl_utf8_new(const uint8_t *bytes, size_t size)
{
    itl_utf8_t ch;

    TL_ASSERT(size <= 4);

    memcpy(ch.bytes, bytes, size);
    ch.size = size;

    return ch;
}

#define itl_utf8_copy(dst, src) memcpy(dst, src, sizeof(itl_utf8_t))

ITL_DEF bool itl_utf8_equal(itl_utf8_t ch1, itl_utf8_t ch2)
{
    if (ch1.size != ch2.size) {
        return false;
    }

    TL_ASSERT(ch1.size <= 4 && ch2.size <= 4);

    if (memcmp(ch1.bytes,
               ch2.bytes,
               ch1.size * sizeof(uint8_t)) != 0) {
        return false;
    }
    return true;
}

ITL_DEF size_t itl_utf8_width(int byte)
{
    if ((byte & 0x80) == 0) return 1;         /* 1 byte */
    else if ((byte & 0xE0) == 0xC0) return 2; /* 2 bytes */
    else if ((byte & 0xF0) == 0xE0) return 3; /* 3 bytes */
    else if ((byte & 0xF8) == 0xF0) return 4; /* 4 bytes */
    else return 0; /* invalid character */
}

#define itl_is_surrogate(first_byte, second_byte) \
    (((first_byte) & 0xE0) == 0xD0 &&             \
     (second_byte) >= 0x80 &&                     \
     (second_byte) <= 0xBF)

#define itl_replacement_character \
    itl_utf8_new((uint8_t[]){ 0xEF, 0xBF, 0xBD }, 3)

ITL_DEF itl_utf8_t itl_utf8_parse(uint8_t first_byte)
{
    int i;
    size_t size;
    uint8_t bytes[4];

    size = itl_utf8_width(first_byte);
    if (size == 0) { /* invalid character */
        itl_traceln("Invalid UTF-8 sequence '%d'\n", (uint8_t)first_byte);
        return itl_replacement_character;
    }

    bytes[0] = first_byte;
    for (i = 1; i < (int)size; ++i) { /* consequent bytes */
        ITL_TRY_READ_BYTE(&bytes[i], itl_replacement_character);
    }

    /* Codepoints U+D800 to U+DFFF (known as UTF-16 surrogates) are invalid. */
    if (size > 1 && itl_is_surrogate(first_byte, bytes[1])) {
        itl_traceln("Invalid UTF-16 surrogate: '%02X %02X'\n",
                  first_byte, bytes[1]);
        return itl_replacement_character;
    }

#if defined ITL_DEBUG
    itl_traceln("utf8 char size: %zu\n", size);
    itl_traceln("utf8 char bytes: '");

    for (i = 0; i < (int)size - 1; ++i) {
        itl_trace("%02X ", bytes[i]);
    }
    itl_trace("%02X'\n", bytes[size - 1]);
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

ITL_DEF void itl_string_init(itl_string_t *str)
{
    str->length = 0;
    str->size = 0;

    str->capacity = ITL_STRING_INIT_SIZE;
    str->chars = (itl_utf8_t *)
        itl_malloc(str->capacity * sizeof(itl_utf8_t));
}

ITL_DEF itl_string_t *itl_string_alloc(void)
{
    itl_string_t *ptr = (itl_string_t *)itl_malloc(sizeof(itl_string_t));
    itl_string_init(ptr);
    return ptr;
}

ITL_DEF void itl_string_extend(itl_string_t *str)
{
    str->capacity = ITL_STRING_REALLOC_CAPACITY(str->capacity);
    str->chars = (itl_utf8_t *)
        itl_realloc(str->chars, str->capacity * sizeof(itl_utf8_t));
}

/* Return length of the matching prefix */
ITL_DEF size_t itl_string_prefix_with_offset(itl_string_t *str1, size_t start,
                                             size_t end, itl_string_t *str2)
{
    size_t i, k;

    TL_ASSERT(start <= end);

    if (end > str1->length) {
        end = ITL_MIN(size_t, str1->length, str2->length);
    }
    for (i = start, k = 0; i < end && k < str2->length; ++i) {
        if (!itl_utf8_equal(str1->chars[i], str2->chars[k])) {
            break;
        }
        k += 1;
    }
    return k;
}

ITL_DEF bool itl_string_equal(itl_string_t *str1, itl_string_t *str2)
{
    if (str1->size != str2->size) {
        return false;
    }
    if (str1->length == 0) {
        if (str2->length == 0) {
            return true;
        } else {
            return false;
        }
    }
    return itl_string_prefix_with_offset(str1, 0, str1->length,
                                         str2) == str1->length;
}

ITL_DEF void itl_string_copy(itl_string_t *dst, const itl_string_t *src)
{
    size_t i;

    TL_ASSERT(dst);
    TL_ASSERT(src);

    while (dst->capacity < src->capacity) {
        itl_string_extend(dst);
    }
    for (i = 0; i < src->length; ++i) {
        itl_utf8_copy(&dst->chars[i], &src->chars[i]);
    }
    dst->length = src->length;
    dst->size = src->size;
}

ITL_DEF void itl_string_recalc_size(itl_string_t *str)
{
    size_t i;

    TL_ASSERT(str->length <= ITL_STRING_MAX_LEN);

    str->size = 0;
    for (i = 0; i < str->length; ++i) {
        str->size += str->chars[i].size;
    }
}

/* Shrinks string to capacity of ITL_STRING_INIT_SIZE */
ITL_DEF void itl_string_shrink(itl_string_t *str)
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

ITL_DEF void itl_string_clear(itl_string_t *str)
{
    str->size = 0;
    str->length = 0;
    itl_string_shrink(str);
}

ITL_DEF void itl_string_shift(itl_string_t *str, size_t position,
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

ITL_DEF void itl_string_erase(itl_string_t *str, size_t position,
                             size_t count, bool backwards)
{
    itl_traceln("string_erase: pos: %zu, count: %zu, backwards: %d, len %zu\n",
                position, count, backwards, str->length);

    if (count > str->length) {
        count = str->length;
    }

    if (backwards) {
        /* Deleting at the start or at the end */
        if (position >= str->length) {
            str->length -= count;
            itl_string_recalc_size(str);
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


ITL_DEF void itl_string_insert(itl_string_t *str, size_t position,
                               itl_utf8_t ch)
{
    while (str->capacity < str->length + 1) {
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

#define itl_string_free(str)      \
    do {                          \
        if (str != NULL) {        \
            itl_free(str->chars); \
            itl_free(str);        \
        }                         \
    } while (0)

#if defined ITL_WIN32
    #define ITL_LF "\r\n"
    #define ITL_LF_LEN 2
#elif defined ITL_POSIX
    #define ITL_LF "\n"
    #define ITL_LF_LEN 1
#endif /* ITL_POSIX */

ITL_DEF bool itl_string_to_cstr(itl_string_t *str, char *cstr, size_t cstr_size)
{
    size_t i, j, k;

    k = 0;
    for (i = 0; i < str->length; ++i) {
        if (cstr_size - k - 1 < str->chars[i].size) break;
        for (j = 0; j != str->chars[i].size; ++j, ++k) {
            cstr[k] = (char)str->chars[i].bytes[j];
        }
    }
    cstr[k] = '\0';

    if (k != str->size) {
        return false;
    }

    return true;
}

/* Requires null-terminated string. */
ITL_DEF void itl_string_from_cstr(itl_string_t *str, const char *cstr)
{
    size_t i, j, k, rune_width;

    k = 0;
    for (i = 0; cstr[k]; ++i) {
        while (str->capacity < i + 1) {
            itl_string_extend(str);
        }
        rune_width = itl_utf8_width(cstr[k]);

        str->chars[i].size = rune_width;
        for (j = 0; j < rune_width && cstr[k]; ++j, ++k) {
            str->chars[i].bytes[j] = (uint8_t)cstr[k];
        }
    }

    str->length = i;
    itl_string_recalc_size(str);
}

#define ITL_SPACE \
    itl_utf8_new((uint8_t[]){ 0x20 }, 1)

typedef struct itl_history_item itl_history_item_t;

struct itl_history_item
{
    itl_string_t *str;
    itl_history_item_t *next;
    itl_history_item_t *prev;
};

ITL_DEF ITL_THREAD_LOCAL itl_history_item_t *itl_global_history_last = NULL;
ITL_DEF ITL_THREAD_LOCAL itl_history_item_t *itl_global_history_first = NULL;
ITL_DEF ITL_THREAD_LOCAL size_t itl_global_history_length = 0;

ITL_DEF ITL_THREAD_LOCAL itl_string_t itl_global_line_buffer = {0};

typedef struct itl_le itl_le_t;

/* Line editor */
struct itl_le
{
    itl_string_t *line;
    size_t cursor_position;
    bool appended_to_history;

    itl_history_item_t *history_selected_item;

    char *out_buf;
    size_t out_size;
    const char *prompt;
};

ITL_DEF itl_history_item_t *itl_history_item_alloc(const itl_string_t *str)
{
    itl_history_item_t *item = (itl_history_item_t *)
        itl_malloc(sizeof(itl_history_item_t));

    item->next = NULL;
    item->prev = NULL;

    item->str = itl_string_alloc();
    itl_string_copy(item->str, str);

    return item;
}

ITL_DEF void itl_history_item_free(itl_history_item_t *item)
{
    itl_string_free(item->str);
    itl_free(item);
}

ITL_DEF void itl_global_history_free(void)
{
    itl_history_item_t *item;
    itl_history_item_t *prev_item;

    item = itl_global_history_last;
    if (item == NULL) {
        return;
    }

    while (item->next) {
        item = item->next;
    }
    while (item) {
        prev_item = item->prev;
        itl_history_item_free(item);
        item = prev_item;
    }

    itl_global_history_last = NULL;
}

ITL_DEF bool itl_global_history_append(itl_string_t *str)
{
    /* If history size was exceeded, release the last item first */
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

    if (itl_global_history_last == NULL) {
        itl_global_history_last = itl_history_item_alloc(str);
        itl_global_history_first = itl_global_history_last;
    }
    else {
        /* Do not append the same string */
        if (itl_string_equal(itl_global_history_last->str, str)) {
            return false;
        }

        itl_history_item_t *item = itl_history_item_alloc(str);
        item->prev = itl_global_history_last;
        itl_global_history_last->next = item;
        itl_global_history_last = item;
    }

    itl_global_history_length += 1;

    return true;
}

#define ITL_HISTORY_FILE_BUFFER_INIT_SIZE 64

/* If this is true, do not overwrite file on `history_dump_to_file()` */
ITL_DEF ITL_THREAD_LOCAL bool itl_global_history_is_file_bad = false;

/* Returns TL_SUCCESS, -EINVAL on invalid file, or -errno on other errors */
ITL_DEF int itl_global_history_load_from_file(const char *path)
{
    int ch;
    size_t line_pos;

    int ret = TL_SUCCESS;
    size_t buffer_size = ITL_HISTORY_FILE_BUFFER_INIT_SIZE;
    char *buffer = (char *)
        itl_malloc(sizeof(char) * ITL_HISTORY_FILE_BUFFER_INIT_SIZE);
    itl_string_t *str = itl_string_alloc();

    itl_global_history_free();
    itl_global_history_is_file_bad = false;

    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        ret = -errno;
        if (ret != -ENOENT) {
            itl_global_history_is_file_bad = true;
        }
        errno = 0;
        goto end;
    }

    line_pos = 0;
    while (!feof(file)) {
        if (buffer_size < line_pos + 1) {
            buffer = (char *)
                itl_realloc(buffer, buffer_size * 2);
        }

        ch = fgetc(file);
        if (ch == EOF) {
            ret = -errno;
            errno = 0;
            goto end;
        }
#if defined ITL_WIN32
        else if (ch == '\r') {
            line_pos += 1;
            continue;
        }
#endif /* ITL_WIN32 */
        else if (ch == '\n') {
            buffer[line_pos] = '\0';
            itl_string_from_cstr(str, buffer);
            itl_global_history_append(str);
            itl_traceln("loaded history entry: %s\n", buffer);
            line_pos = 0;
        /* Loaded a binary file on accident? */
        } else if (iscntrl((uint8_t)ch)) {
            ret = -EINVAL;
            itl_traceln("non-text byte '%X' detected in history file\n",
                        (uint8_t)ch);
            itl_global_history_free();
            itl_global_history_is_file_bad = true;
            goto end;
        } else {
            buffer[line_pos] = (char)ch;
            line_pos += 1;
        }
    }

end:
    if (file) {
        fclose(file);
    }
    itl_free(buffer);
    itl_string_free(str);

    return ret;
}

/* Returns TL_SUCCESS, -EINVAL on invalid file, or -errno on other errors */
ITL_DEF int itl_global_history_dump_to_file(const char *path)
{
    size_t cstr_size;
    itl_history_item_t *item;
    itl_history_item_t *prev_item;

    if (itl_global_history_is_file_bad) {
        return -EINVAL;
    }

    int ret = TL_SUCCESS;
    size_t buffer_size = ITL_HISTORY_FILE_BUFFER_INIT_SIZE;
    char *buffer = (char *)
        itl_malloc(sizeof(char) * ITL_HISTORY_FILE_BUFFER_INIT_SIZE);

    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        ret = -errno;
        errno = 0;
        goto end;
    }

    item = itl_global_history_first;
    if (item == NULL) {
        goto end;
    }

    while (item->prev) {
        item = item->prev;
    }
    while (item) {
        prev_item = item->next;
        cstr_size = item->str->size + 1;
        if (buffer_size < cstr_size) {
            buffer = (char *)
                itl_realloc(buffer, cstr_size);
        }
        itl_string_to_cstr(item->str, buffer, cstr_size);
        if (item->str->length > 1) {
            if (fputs(buffer, file) == EOF || fputc('\n', file) == EOF) {
                ret = -errno;
                errno = 0;
                goto end;
            }
        }
        item = prev_item;
    }

end:
    if (file) {
        fclose(file);
    }
    itl_free(buffer);

    return ret;
}

ITL_DEF itl_le_t itl_le_new(itl_string_t *line_buf, char *out_buf,
                            size_t out_size, const char *prompt)
{
    itl_le_t le = {
        /* .line                  = */ line_buf,
        /* .cursor_position       = */ line_buf->length,
        /* .appended_to_history   = */ false,
        /* .history_selected_item = */ NULL,
        /* .out_buf               = */ out_buf,
        /* .out_size              = */ out_size,
        /* .prompt                = */ prompt,
    };

    return le;
}

ITL_DEF void itl_le_move_right(itl_le_t *le, size_t steps)
{
    if (le->cursor_position + steps >= le->line->length) {
        le->cursor_position = le->line->length;
    } else {
        le->cursor_position += steps;
    }
}

ITL_DEF void itl_le_move_left(itl_le_t *le, size_t steps)
{
    if (steps <= le->cursor_position) {
        le->cursor_position -= steps;
    } else {
        le->cursor_position = 0;
    }
}

ITL_DEF void itl_le_erase(itl_le_t *le, size_t count, bool backwards)
{
    if (count == 0) {
        return;
    }

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
ITL_DEF bool itl_le_insert(itl_le_t *le, const itl_utf8_t ch)
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
ITL_DEF size_t itl_string_steps_to_token(itl_string_t *str, size_t position,
                                         int token, bool backwards)
{
    uint8_t byte;
    bool should_break;

    size_t i = position;
    size_t steps = 0;

    /* Prevent usage of uninitialized characters */
    if (backwards && i > 0 && i == str->length) {
        ++steps;
        --i;
    }

    itl_traceln("steps: i: %zu\n", i);

    while (i < str->length) {
        byte = str->chars[i].bytes[0];

        switch (token) {
            case ITL_TOKEN_DELIM: should_break = itl_is_delim(byte); break;
            case ITL_TOKEN_WORD: should_break = !itl_is_delim(byte); break;
            default: ITL_UNREACHABLE;
        }
        if (should_break) {
            break;
        }

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

ITL_DEF void itl_le_clear(itl_le_t *le)
{
    itl_string_clear(le->line);
    le->cursor_position = 0;
}

ITL_DEF void itl_global_history_get_prev(itl_le_t *le)
{
    if (itl_global_history_last == NULL) {
        return;
    }

    if (le->history_selected_item) {
        if (le->history_selected_item->prev) {
            le->history_selected_item = le->history_selected_item->prev;
        }
    } else {
        le->history_selected_item = itl_global_history_last;
    }

    if (le->history_selected_item) {
        itl_le_clear(le);
        itl_string_copy(le->line, le->history_selected_item->str);
        le->cursor_position = le->line->length;
    }
}

ITL_DEF void itl_global_history_get_next(itl_le_t *le)
{
    if (le->history_selected_item) {
        if (le->history_selected_item->next) {
            le->history_selected_item = le->history_selected_item->next;
            itl_le_clear(le);
            itl_string_copy(le->line, le->history_selected_item->str);
            le->cursor_position = le->line->length;
        }
    }
}

#define ITL_CHAR_BUFFER_INIT_SIZE 32

typedef struct itl_char_buf itl_char_buf_t;

struct itl_char_buf
{
    char *data;
    size_t size;
    size_t capacity;
};

ITL_DEF void itl_char_buf_init(itl_char_buf_t *cb)
{
    cb->size = 0;
    cb->capacity = ITL_CHAR_BUFFER_INIT_SIZE;
    cb->data = (char *)
        itl_malloc(sizeof(char) * cb->capacity);
}

ITL_DEF itl_char_buf_t* itl_char_buf_alloc(void)
{
    itl_char_buf_t *cb = (itl_char_buf_t *)
        itl_malloc(sizeof(itl_char_buf_t));
    itl_char_buf_init(cb);
    return cb;
}

#define itl_char_buf_free(cb) \
    do {                      \
        itl_free(cb->data);   \
        itl_free(cb);         \
    } while (0)

#define ITL_CHAR_BUF_REALLOC_CAPACITY(old_capacity) \
    (old_capacity * 2)

ITL_DEF void itl_char_buf_extend(itl_char_buf_t *cb)
{
    cb->capacity = ITL_CHAR_BUF_REALLOC_CAPACITY(cb->capacity);
    cb->data = (char *)
        itl_realloc(cb->data, cb->capacity);
}

ITL_DEF void itl_char_buf_append_cstr(itl_char_buf_t *cb, const char *data)
{
    size_t i, j;
    size_t cstr_size = strlen(data);
    size_t new_size = cb->size + cstr_size;

    while (cb->capacity < new_size) {
        itl_char_buf_extend(cb);
    }
    for (i = cb->size, j = 0; j < cstr_size; ++i, ++j) {
        cb->data[i] = data[j];
    }
    cb->size = new_size;
}

ITL_DEF void itl_char_buf_append_size(itl_char_buf_t *cb, size_t data)
{
    char digit;
    size_t new_size, i;

    size_t data_len = 0;
    size_t data_copy = data;

    while (data_copy > 0) {
        data_len += 1;
        data_copy /= 10;
    }

    new_size = cb->size + data_len;
    while (cb->capacity < new_size) {
        itl_char_buf_extend(cb);
    }

    /* Digits are put in reverse order */
    for (i = new_size - 1; i >= cb->size; --i) {
        digit = (char)(data % 10) + '0';
        cb->data[i] = digit;
        data /= 10;
    }

    cb->size = new_size;
}

ITL_DEF void itl_char_buf_append_byte(itl_char_buf_t *cb, uint8_t data)
{
    while (cb->capacity < cb->size + 1) {
        itl_char_buf_extend(cb);
    }
    cb->data[cb->size] = (char)data;
    cb->size += 1;
}

ITL_DEF void itl_char_buf_dump(itl_char_buf_t *cb)
{
/* stdio is slow and requires stinky `setbuf(stdout, NULL)` */
#if defined TL_REFRESH_STDIO
    size_t i;
    const char* s = cb->data;
    for (i = 0; i < cb->size; ++i) {
        fputc(s[i], stdout);
    }
    fflush(stdout);
#else
    write(STDOUT_FILENO, cb->data, cb->size);
#endif
}

#define itl_tty_hide_cursor(buffer) \
    itl_char_buf_append_cstr(buffer, "\x1b[?25l")

#define itl_tty_show_cursor(buffer) \
    itl_char_buf_append_cstr(buffer, "\x1b[?25h")

#define itl_tty_move_to_column(buffer, col)        \
    itl_char_buf_append_cstr(buffer, "\x1b[");     \
    itl_char_buf_append_size(buffer, (size_t)col); \
    itl_char_buf_append_byte(buffer, 'G');

#define itl_tty_move_forward(buffer, steps)          \
    itl_char_buf_append_cstr(buffer, "\x1b[");       \
    itl_char_buf_append_size(buffer, (size_t)steps); \
    itl_char_buf_append_byte(buffer, 'C');

#define itl_tty_move_up(buffer, rows)               \
    itl_char_buf_append_cstr(buffer, "\x1b[");      \
    itl_char_buf_append_size(buffer, (size_t)rows); \
    itl_char_buf_append_byte(buffer, 'A');

#define itl_tty_move_down(buffer, rows)             \
    itl_char_buf_append_cstr(buffer, "\x1b[");      \
    itl_char_buf_append_size(buffer, (size_t)rows); \
    itl_char_buf_append_byte(buffer, 'B');

#define itl_tty_clear_whole_line(buffer) \
    itl_char_buf_append_cstr(buffer, "\r\x1b[0K")

#define itl_tty_clear_to_end(buffer) \
    itl_char_buf_append_cstr(buffer, "\x1b[K")

#define itl_tty_goto_home(buffer) \
    itl_char_buf_append_cstr(buffer, "\x1b[H")

#define itl_tty_erase_screen(buffer) \
    itl_char_buf_append_cstr(buffer, "\033[2J")

#define itl_tty_status_report(buffer) \
    itl_char_buf_append_cstr(buffer, "\x1b[6n")

ITL_DEF ITL_THREAD_LOCAL itl_char_buf_t itl_global_refresh_char_buffer = {0};

ITL_DEF bool itl_tty_get_size(size_t *rows, size_t *cols) {
#if defined TL_SIZE_USE_ESCAPES
    char buf[32];
    size_t i;

    itl_char_buf_t *buffer = &itl_global_refresh_char_buffer;
    itl_tty_move_forward(buffer, 999);
    itl_tty_status_report(buffer);
    itl_char_buf_dump(buffer);
    buffer->size = 0;

    i = 0;
    while (i < sizeof(buf) - 1) {
        ITL_TRY_READ_BYTE(&buf[i], false);
        if (buf[i] == 'R') {
            break;
        }
        i += 1;
    }
    buf[i + 1] = '\0';

    ITL_TRY(buf[0] == '\x1b' || buf[1] == '[', false);
    /* @@@: replace `sscanf()` here */
    ITL_TRY(sscanf(&buf[2], "%zu;%zu", rows, cols) == 2, false);

#elif defined ITL_WIN32
    CONSOLE_SCREEN_BUFFER_INFO buffer_info;

    ITL_TRY(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
                                       &buffer_info),
            false);

    (*cols) = (size_t)
        (buffer_info.srWindow.Right - buffer_info.srWindow.Left + 1);
    (*rows) = (size_t)
        (buffer_info.srWindow.Bottom - buffer_info.srWindow.Top + 1);

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

ITL_DEF ITL_THREAD_LOCAL size_t itl_global_tty_prev_lines = 1;
ITL_DEF ITL_THREAD_LOCAL size_t itl_global_tty_prev_wrap_row = 1;

ITL_DEF bool itl_le_tty_refresh(itl_le_t *le)
{
    size_t i, j;
    size_t rows, cols;
    size_t current_col, current_lines, dirty_lines;
    size_t prompt_len, wrap_cursor_col, wrap_cursor_row;

    /* Write everything into a buffer, then dump it all at once */
    itl_char_buf_t *buffer = &itl_global_refresh_char_buffer;

    TL_ASSERT(le->line);
    TL_ASSERT(le->line->chars);
    TL_ASSERT(le->line->size >= le->line->length);
    TL_ASSERT(le->line->length <= ITL_STRING_MAX_LEN);

    itl_tty_hide_cursor(buffer);

    rows = 0;
    cols = 0;
    ITL_TRY(itl_tty_get_size(&rows, &cols), false);

    TL_ASSERT(rows != 0 && cols != 0);

    prompt_len = (le->prompt) ? strlen(le->prompt) : 0;

    current_lines =
        (le->line->length + prompt_len) / ITL_MAX(size_t, cols, 1) + 1;

    wrap_cursor_col =
        (le->cursor_position + prompt_len) % ITL_MAX(size_t, cols, 1) + 1;
    wrap_cursor_row =
        (le->cursor_position + prompt_len) / ITL_MAX(size_t, cols, 1) + 1;

    itl_traceln("wrow: %zu, prev: %zu, col: %zu, curp: %zu\n",
                wrap_cursor_row, itl_global_tty_prev_wrap_row,
                wrap_cursor_col, le->cursor_position);

    /* Move appropriate amount of lines back, while clearing previous output */
    for (i = 0; i < itl_global_tty_prev_lines; ++i) {
        itl_tty_clear_whole_line(buffer);
        if (i < itl_global_tty_prev_wrap_row - 1) {
            itl_tty_move_up(buffer, 1);
        }
    }

    if (le->prompt) {
        itl_char_buf_append_cstr(buffer, le->prompt);
    }

    /* Print current contents of the line editor */
    for (i = 0; i < le->line->length; ++i) {
        for (j = 0; j < le->line->chars[i].size; ++j) {
            itl_char_buf_append_byte(buffer,
                                     le->line->chars[i].bytes[j]);
        }

        /* If line is full, wrap */
        current_col = (prompt_len + i) % ITL_MAX(size_t, cols, 1);
        if (current_col == cols - 1) {
            itl_char_buf_append_cstr(buffer, ITL_LF);
        }
    }

    /* If current amount of lines is less than previous amount of lines, then
       input was cleared by kill line or such. Clear each dirty line, then go
       back up */
    if (current_lines < itl_global_tty_prev_lines) {
        dirty_lines = itl_global_tty_prev_lines - current_lines;
        for (i = 0; i < dirty_lines; ++i) {
            itl_tty_move_down(buffer, 1);
            itl_tty_clear_whole_line(buffer);
        }
        itl_tty_move_up(buffer, dirty_lines);
    }
    /* Otherwise clear to the end of line */
    else {
        itl_tty_clear_to_end(buffer);
    }

    /* Move cursor to appropriate row and column. If row didn't change, stay on
       the same line */
    if (wrap_cursor_row < current_lines) {
        itl_tty_move_up(buffer, current_lines - wrap_cursor_row);
    }
    itl_tty_move_to_column(buffer, wrap_cursor_col);

    itl_global_tty_prev_lines = current_lines;
    itl_global_tty_prev_wrap_row = wrap_cursor_row;

    itl_tty_show_cursor(buffer);

    itl_char_buf_dump(buffer);
    /* Reset the buffer */
    buffer->size = 0;

    return true;
}

#ifdef ITL_POSIX
ITL_DEF int itl_esc_parse_posix(uint8_t byte)
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
ITL_DEF int itl_esc_parse_win32(uint8_t byte)
{
    int event = 0;

    /* https://learn.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-6.0/aa299374(v=vs.60) */
    if (byte == 224 || byte == 0) { /* esc */
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

ITL_DEF int itl_esc_parse(uint8_t byte)
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

        case 9:  return TL_KEY_TAB;
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

ITL_DEF ITL_THREAD_LOCAL int itl_global_last_control = TL_KEY_UNKN;

int *itl__last_control_location(void) {
    return &itl_global_last_control;
}

#if !defined TL_MANUAL_TAB_COMPLETION
ITL_DEF void itl_char_buf_append_string(itl_char_buf_t *cb, itl_string_t *str)
{
    while (cb->capacity < cb->size + str->size) {
        itl_char_buf_extend(cb);
    }
    char *data = cb->data + (cb->size * sizeof(char));
    itl_string_to_cstr(str, data, str->size + 1);
    cb->size += str->size; /* Ignore null at the end */
}

ITL_DEF void itl_string_append_completion(itl_string_t *dst, itl_string_t *src,
                                          size_t prefix_length)
{
    size_t i, j;
    size_t new_len = dst->length + src->length - prefix_length;

    while (dst->capacity < new_len) {
        itl_string_extend(dst);
    }

    /* Insert a space before new word, if there is no matching prefix */
    i = dst->length;
    if (!itl_utf8_equal(dst->chars[i - 1], ITL_SPACE) && prefix_length == 0) {
        dst->chars[i++] = ITL_SPACE;
        dst->length += 1;
    }
    /* Append missing chars */
    for (j = prefix_length; j <= new_len; ++i, ++j) {
        dst->chars[i] = src->chars[j];
    }
    dst->length = new_len;

    itl_string_recalc_size(dst);
}

typedef struct itl_completion itl_completion_t;

/* For example, Git completions in this structure:
   ... -- checkout -- branch -- status   <- siblings of root
             |          |
            ...       master -- staging  <- children of `branch` completion,
                     /      \      |        siblings of `master`
                  HEAD      ...   ...
*/
struct itl_completion
{
    itl_string_t *str;
    itl_completion_t *sibling;
    itl_completion_t *child;
};

ITL_DEF ITL_THREAD_LOCAL itl_completion_t *itl_global_completion_root = NULL;

ITL_DEF itl_completion_t *itl_completion_alloc(void)
{
    itl_completion_t *completion = (itl_completion_t *)
        itl_malloc(sizeof(itl_completion_t));

    completion->str = NULL;
    completion->child = NULL;
    completion->sibling = NULL;

    return completion;
}

/* Takes ownership of *str */
ITL_DEF itl_completion_t *itl_completion_append(itl_completion_t *completion,
                                                itl_string_t *str)
{
    itl_completion_t *new_child;

    new_child = itl_completion_alloc();
    new_child->str = str;

    new_child->sibling = completion->child;
    completion->child = new_child;

    return new_child;
}

#define itl_completion_free(completion)   \
    do {                                  \
        itl_string_free(completion->str); \
        itl_free(completion);             \
    } while (0)

/* Recursively free all completions connected to current one */
ITL_DEF void itl_completion_free_all(itl_completion_t *completion)
{
    if (completion) {
        itl_completion_free_all(completion->child);
        itl_completion_free_all(completion->sibling);
        itl_completion_free(completion);
    }
}

#define itl_global_completion_free() \
        itl_completion_free_all(itl_global_completion_root);

typedef struct itl_offset itl_offset_t;

struct itl_offset
{
    size_t start;
    size_t end;
};

ITL_DEF itl_offset_t *itl_offset_alloc(void)
{
    itl_offset_t *offset = (itl_offset_t *)
        itl_malloc(sizeof(itl_offset_t));

    offset->start = 0;
    offset->end = 0;

    return offset;
}

#define itl_offset_free(offset) itl_free(offset)

typedef struct itl_split itl_split_t;

#define ITL_STRING_SPLIT_INIT_CAPACITY 4
#define ITL_STRING_SPLIT_REALLOC_CAPACITY(old_size) \
    ((old_size) << 1)

struct itl_split
{
    itl_offset_t **offsets;
    size_t size;
    size_t capacity;
};

ITL_DEF itl_split_t *itl_split_alloc(void)
{
    size_t i;

    itl_split_t *split = (itl_split_t *)
        itl_malloc(sizeof(itl_split_t));
    split->offsets = (itl_offset_t **)
        itl_malloc(sizeof(itl_offset_t) * ITL_STRING_SPLIT_INIT_CAPACITY);

    split->capacity = ITL_STRING_SPLIT_INIT_CAPACITY;
    split->size = 0;

    for (i = 0; i < split->capacity; ++i) {
        split->offsets[i] = itl_offset_alloc();
    }

    return split;
}

ITL_DEF void itl_split_free(itl_split_t *split)
{
    size_t i;
    for (i = 0; i < split->capacity; ++i) {
        itl_offset_free(split->offsets[i]);
    }
    itl_free(split->offsets);
    itl_free(split);
}

ITL_DEF void itl_split_extend(itl_split_t *split)
{
    size_t i;
    itl_offset_t **new_offsets;

    size_t old_capacity = split->capacity;

    split->capacity = ITL_STRING_SPLIT_REALLOC_CAPACITY(split->capacity);
    new_offsets = (itl_offset_t **)
        itl_malloc(sizeof(itl_split_t *) * split->capacity);

    for (i = 0; i < split->capacity; ++i) {
        new_offsets[i] = itl_offset_alloc();
        if (i < old_capacity) {
            memcpy(new_offsets[i], split->offsets[i], sizeof(itl_offset_t));
            itl_offset_free(split->offsets[i]);
        }
    }
    itl_free(split->offsets);
    split->offsets = new_offsets;
}

ITL_DEF void itl_split_append(itl_split_t *split, size_t start,
                              size_t end)
{
    itl_offset_t *offset;

    while (split->capacity < split->size + 1) {
        itl_split_extend(split);
    }
    offset = split->offsets[split->size];

    offset->start = start;
    offset->end = end;

    split->size += 1;
}

ITL_DEF itl_split_t *itl_string_split(itl_string_t *str, char delimiter)
{
    size_t i, j;
    itl_utf8_t ch;
    bool is_prev_delim;

    itl_split_t *split = itl_split_alloc();

    is_prev_delim = false;
    for (i = 0, j = 0; i < str->length; ++i) {
        ch = str->chars[i];
        if (ch.bytes[0] == (uint8_t)delimiter) {
            /* Skip characters, if there is multiple delimiters in a row */
            if (!is_prev_delim) {
                itl_split_append(split, j, i);
                is_prev_delim = true;
                j = i + 1;
            } else {
                j += 1;
            }
        } else {
            is_prev_delim = false;
        }
    }
    if (j <= i) {
        itl_split_append(split, j, i);
    }

    return split;
}

typedef struct itl_completion_list itl_completion_list_t;

struct itl_completion_list
{
    itl_completion_t* completion;
    itl_completion_list_t *next;
};

ITL_DEF itl_completion_list_t *itl_completion_list_alloc(void)
{
    itl_completion_list_t *list = (itl_completion_list_t *)
        itl_malloc(sizeof(itl_completion_list_t));

    list->completion = NULL;
    list->next = NULL;

    return list;
}

ITL_DEF void itl_completion_list_free(itl_completion_list_t *list)
{
    itl_completion_list_t *next;

    while (list) {
        next = list->next;
        itl_free(list);
        list = next;
    }
}

ITL_DEF void itl_completion_list_append(itl_completion_list_t *list,
                                        itl_completion_t *completion)
{
    if (list->completion == NULL) {
        list->completion = completion;
        return;
    }
    while (list->next) {
        list = list->next;
    }
    list->next = itl_completion_list_alloc();
    list->next->completion = completion;
}

#define ITL_COMPLETIONS_ON_SINGLE_LINE 5
 /* Minimum word width */
#define ITL_COMPLETIONS_MARGIN 12

ITL_DEF void itl_completion_list_dump(itl_completion_list_t *list)
{
    size_t i;
    itl_string_t *str;
    itl_completion_list_t *next;

    size_t count = 0;
    itl_char_buf_t *buffer = itl_char_buf_alloc();

    itl_char_buf_append_cstr(buffer, ITL_LF);
    while (true) {
        next = list->next;
        str = list->completion->str;
        itl_char_buf_append_string(buffer, str);
        count += 1;
        list = next;
        if (list == NULL) {
            break;
        }
        if (count < ITL_COMPLETIONS_ON_SINGLE_LINE) {
            for (i = str->length; i < ITL_COMPLETIONS_MARGIN - 2; ++i) {
                itl_char_buf_append_byte(buffer, ' ');
            }
            itl_char_buf_append_cstr(buffer, "  ");
        } else {
            itl_char_buf_append_cstr(buffer, ITL_LF);
            count = 0;
        }
    }
    itl_char_buf_append_cstr(buffer, ITL_LF);

    itl_char_buf_dump(buffer);
    itl_char_buf_free(buffer);
}

/* Split the string by spaces. If a split fully matches, look at it's children.
   If none of the children fully match, rank them by longest common prefix,
   complete the string, and return NULL. If more than one have the same longest
   common prefix, or current split is empty, return a list of possible
   matches. Otherwise, return NULL too. */
ITL_DEF itl_completion_list_t *itl_string_complete(itl_string_t *str)
{
    itl_offset_t *offset;
    size_t offset_difference;
    itl_string_t *possible_completion;
    size_t i, prefix_length, longest_prefix;

    itl_completion_list_t *completion_list = NULL;
    size_t completion_count = 0;
    itl_split_t *split = itl_string_split(str, ' ');
    itl_completion_t *completion = itl_global_completion_root->child;

    for (i = 0; i < split->size; ++i) {
        longest_prefix = 0;
        possible_completion = NULL;
        offset = split->offsets[i];
        offset_difference = offset->end - offset->start;

        while (completion) {
            prefix_length = itl_string_prefix_with_offset(str,
                                                          offset->start,
                                                          offset->end,
                                                          completion->str);

            /* If completion fully matches, it's just a prefix. Advance to the
               next split offset, and check prefix's children. */
            if (prefix_length == completion->str->length &&
                prefix_length == offset_difference) {
                completion = completion->child;
                break;
            /* If offset difference is the prefix, it must be a new longest
               prefix. If offset difference is 0, the string is empty. */
            } else if (prefix_length == offset_difference ||
                       offset_difference == 0) {
                if (longest_prefix < prefix_length) {
                    longest_prefix = prefix_length;
                    possible_completion = completion->str;
                }
                /* If this prefix is the longest, add this match to completion
                   list. If there is more than one match of this kind, dump them
                   instead of appending to the string. If offset difference is
                   0, the string is empty, add it should add all completions. */
                if (longest_prefix == prefix_length || offset_difference == 0) {
                    if (completion_list == NULL) {
                        completion_list = itl_completion_list_alloc();
                    }
                    itl_completion_list_append(completion_list, completion);
                    completion_count += 1;
                }
            }
            /* If prefix didn't fully match, advance to completion's sibling */
            completion = completion->sibling;
        }
        /* Dump completion list only if there are more than 2 completions
           with the same prefix, or the string is empty, otherwise
           autocomplete */
        if (completion_list) {
            if (completion_count > 1 || offset_difference == 0) {
                itl_split_free(split);
                return completion_list;
            } else {
                itl_completion_list_free(completion_list);
            }
        }
        if (longest_prefix != 0) {
            itl_string_append_completion(str, possible_completion,
                                         longest_prefix);
            itl_split_free(split);
            return NULL;
        }
    }

    itl_split_free(split);
    return NULL;
}

ITL_DEF bool itl_le_complete(itl_le_t *le)
{
    size_t old_len = le->line->length;
    itl_completion_list_t *list = itl_string_complete(le->line);

    if (list) {
        itl_completion_list_dump(list);
        itl_completion_list_free(list);
        return false;
    }
    if (old_len != le->line->length) {
        le->cursor_position = le->line->length;
    }
    return true;
}
#endif /* !TL_MANUAL_TAB_COMPLETION */

ITL_DEF int itl_le_key_handle(itl_le_t *le, int esc)
{
    size_t i, steps;

    tl_last_control = esc;
    switch (esc & TL_MASK_KEY) {
        case TL_KEY_TAB: {
#if !defined TL_MANUAL_TAB_COMPLETION
            if (itl_global_completion_root != NULL) {
                itl_le_complete(le);
            }
#else /* !TL_MANUAL_TAB_COMPLETION */
            ITL_TRY(itl_string_to_cstr(le->line, le->out_buf, le->out_size),
                    TL_ERROR_SIZE);
            return TL_PRESSED_TAB;
#endif
        } break;

        case TL_KEY_UP: {
            itl_string_t *prev_line;

            if (!le->appended_to_history) {
                prev_line = itl_string_alloc();
                itl_string_copy(prev_line, le->line);
                itl_global_history_get_prev(le);
                /* Avoid appending same strings or empty strings */
                if (!itl_string_equal(le->line, prev_line) &&
                    prev_line->length > 0) {
                    itl_global_history_append(prev_line);
                }
                itl_string_free(prev_line);
                le->appended_to_history = true;
            } else if (itl_global_history_last == le->history_selected_item) {
                /* If some string was already appended, just update it */
                itl_string_copy(itl_global_history_last->str, le->line);
                itl_global_history_get_prev(le);
            } else {
                itl_global_history_get_prev(le);
            }
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
                        steps = itl_le_steps_to_token(le, ITL_TOKEN_WORD,
                                                      false);
                        itl_le_move_right(le, steps);
                        steps = itl_le_steps_to_token(le, ITL_TOKEN_DELIM,
                                                      false);
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
                        steps = itl_le_steps_to_token(le, ITL_TOKEN_WORD,
                                                      true);
                        itl_le_move_left(le, steps);
                        steps = itl_le_steps_to_token(le, ITL_TOKEN_DELIM,
                                                      true);
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
            if (esc & TL_MOD_CTRL && le->line->length > 0) {
                steps = itl_le_steps_to_token(le, ITL_TOKEN_DELIM, true);
                if (steps > 1) {
                    itl_le_erase_backward(le, steps - 1);
                } else if (le->cursor_position >= steps) {
                    /* `cursor_position - steps` can wrap here. Handle it in
                       steps_to_token. */
                    steps +=
                        itl_string_steps_to_token(le->line,
                                                  le->cursor_position - steps,
                                                  ITL_TOKEN_WORD, true);
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
                } else if (le->cursor_position < le->line->length) {
                    steps +=
                        itl_string_steps_to_token(le->line,
                                                  le->cursor_position + steps,
                                                  ITL_TOKEN_WORD, false);
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
#endif /* ITL_SUSPEND */
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
            itl_char_buf_t *buffer = &itl_global_refresh_char_buffer;
            itl_tty_goto_home(buffer);
            itl_tty_erase_screen(buffer);
            itl_char_buf_dump(buffer);
            buffer->size = 0;
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

ITL_DEF ITL_THREAD_LOCAL bool itl_is_active = false;

TL_DEF int tl_init(void)
{
    TL_ASSERT(TL_HISTORY_MAX_SIZE % 2 == 0 &&
              "History size must be a power of 2");

    ITL_TRY(isatty(STDIN_FILENO), TL_ERROR);
    ITL_TRY(itl_enter_raw_mode(), TL_ERROR);

    itl_string_init(&itl_global_line_buffer);
    itl_char_buf_init(&itl_global_refresh_char_buffer);

    itl_is_active = true;
    return TL_SUCCESS;
}

TL_DEF int tl_exit(void)
{
    TL_ASSERT(itl_is_active && "tl_init() should be called");

    itl_global_history_free();
#if !defined TL_MANUAL_TAB_COMPLETION
    itl_global_completion_free();
#endif /* !TL_MANUAL_TAB_COMPLETION */
    itl_free(itl_global_line_buffer.chars);
    itl_free(itl_global_refresh_char_buffer.data);

    itl_traceln("Exited, alloc count: %zu\n", itl_global_alloc_count);
    TL_ASSERT(itl_global_alloc_count == 0);

    ITL_TRY(itl_exit_raw_mode(), TL_ERROR);

    itl_is_active = false;
    return TL_SUCCESS;
}

TL_DEF int tl_readline(char *buffer, size_t buffer_size, const char *prompt)
{
    itl_le_t le;
    itl_utf8_t ch;
    uint8_t input_byte;
    int input_type, code;

    TL_ASSERT(itl_is_active && "tl_init() should be called");
    TL_ASSERT(buffer_size > 1 &&
        "Size should be enough at least for one byte and a null terminator");
    TL_ASSERT(buffer_size <= ITL_STRING_MAX_LEN &&
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

        itl_traceln("strlen: %zu, hist: %zu\n",
                    le.line->length, (size_t)le.history_selected_item);

        itl_le_tty_refresh(&le);
    }

    return TL_ERROR;
}

TL_DEF void tl_setline(const char *str)
{
    TL_ASSERT(itl_is_active && "tl_init() should be called");
    itl_string_shrink(&itl_global_line_buffer);
    itl_string_from_cstr(&itl_global_line_buffer, str);
}

TL_DEF int tl_getc(char *char_buffer, size_t char_buffer_size,
                   const char *prompt)
{
    itl_le_t le;
    itl_utf8_t ch;
    int input_type;
    uint8_t input_byte;

    TL_ASSERT(itl_is_active && "tl_init() should be called");
    TL_ASSERT(char_buffer_size > 1 &&
        "Size should be enough at least for one byte and a null terminator");
    TL_ASSERT(char_buffer_size <= sizeof(char) * 5 &&
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

TL_DEF int tl_load_history(const char *file_path)
{
    return itl_global_history_load_from_file(file_path);
}

TL_DEF int tl_dump_history(const char *file_path)
{
    return itl_global_history_dump_to_file(file_path);
}

TL_DEF size_t tl_utf8_strlen(const char *utf8_str)
{
    size_t len = 0;
    while (*utf8_str) {
        if ((*utf8_str & 0xC0) != 0x80)
            ++len;
        ++utf8_str;
    }
    return len;
}

#if !defined TL_MANUAL_TAB_COMPLETION
TL_DEF void *tl_add_completion(void *prefix, const char *label)
{
    itl_completion_t *completion_node;
    itl_string_t *str;

    if (prefix != NULL) {
        str = itl_string_alloc();
        itl_string_from_cstr(str, label);
        completion_node = itl_completion_append((itl_completion_t *)prefix,
                                                str);
    } else {
        if (itl_global_completion_root == NULL) {
            itl_global_completion_root = itl_completion_alloc();
        }
        str = itl_string_alloc();
        itl_string_from_cstr(str, label);
        completion_node = itl_completion_append(itl_global_completion_root,
                                                str);
    }

    return completion_node;
}

TL_DEF void tl_change_completion(void *completion, const char *label)
{
    itl_completion_t *completion_node = (itl_completion_t *)completion;
    itl_string_from_cstr(completion_node->str, label);
}

TL_DEF void tl_delete_completion(void *completion)
{
    itl_completion_t *completion_node = (itl_completion_t *)completion;
    if (completion_node != NULL) {
        itl_completion_free_all(completion_node->child);
        itl_completion_free(completion_node);
    }
}
#endif /* !TL_MANUAL_TAB_COMPLETION */

#if defined _CRT_SECURE_NO_WARNINGS
    #undef _CRT_SECURE_NO_WARNINGS
#endif /* _CRT_SECURE_NO_WARNINGS */

#endif /* TOILETLINE_IMPLEMENTATION */

#if defined __cplusplus
}
#endif

/*
 * TODO:
 *  - Test all of this. On different OSes.
 *  - Make refresh fasterrrr.
 *  - Squash things to make this tiny again :(
 */
