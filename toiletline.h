/*
 *  toiletline 1.0.0
 *  Small single-header replacement of GNU Readline :3
 *
 *  #define TOILETLINE_IMPLEMENTATION
 *  Before you include this file in C or C++ file to create the implementation.
 *
 *  Copyright (c) 2023 toiletbril
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#if defined __cplusplus
extern "C"
{
#endif

#if !defined TOILETLINE_H_
#define TOILETLINE_H_

#define TL_MAJOR_VERSION 0
#define TL_MINOR_VERSION 8
#define TL_PATCH_VERSION 0

/* Compile with -Werror on Windows */
#if defined TOILETLINE_IMPLEMENTATION && !defined _CRT_SECURE_NO_WARNINGS
#define ITL_WIN32_DISABLED_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif /* !_CRT_SECURE_NO_WARNINGS */

#include <stdbool.h>
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
#define TL_HISTORY_MAX_SIZE (256)
#endif /* TL_HISTORY_MAX_SIZE */

/**
 * Codes which may be returned from reading functions.
 */
typedef enum
{
  TL_SUCCESS = 0,
  TL_PRESSED_ENTER = 1,
  TL_PRESSED_INTERRUPT = 2,
  TL_PRESSED_EOF = 3,
  TL_PRESSED_SUSPEND = 4,
  TL_PRESSED_CONTROL_SEQUENCE = 5,
  TL_PRESSED_TAB = 6,
  TL_PRESSED_QUIT = 7,

  /**
   * Codes below 0 are errors.
   */
  TL_ERROR = -1,
  TL_ERROR_SIZE = -2,
  TL_ERROR_ALLOC = -3,
} tl_status_code;

/**
 * Control sequences.
 * Last control sequence used will be returned by `tl_last_control_sequence`.
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
  TL_KEY_INTERRUPT,

  TL_KEY_HISTORY_SEARCH,

  TL_KEY_UNDO,
  TL_KEY_REDO,

  /* Reported when a bracketed paste sequence begins. Handled internally. */
  TL_KEY_PASTE_BEGIN
} tl_key_kind;

#define TL_MOD_CTRL  (1 << 24)
#define TL_MOD_SHIFT (1 << 25)
#define TL_MOD_ALT   (1 << 26)

#define TL_MASK_KEY 0x00FFFFFF
#define TL_MASK_MOD 0xFF000000

typedef enum
{
  TL_EDIT_MODE_EMACS = 0,
  TL_EDIT_MODE_VI_INSERT,
  TL_EDIT_MODE_VI_COMMAND,
  TL_EDIT_MODE_VI_VISUAL
} tl_edit_mode;

/**
 * Last pressed control sequence.
 */
TL_DEF int tl_last_control_sequence(void);
/**
 * Initialize toiletline and put terminal in raw mode.
 */
TL_DEF tl_status_code tl_init(void);
/**
 * Put the terminal into raw mode without doing anything else.
 */
TL_DEF tl_status_code tl_enter_raw_mode(void);
/**
 * Exit toiletline, restore terminal state, and free internal memory.
 */
TL_DEF tl_status_code tl_exit(void);
/**
 * Restore the terminal state without doing anything else.
 */
TL_DEF tl_status_code tl_exit_raw_mode(void);
/**
 * Read a line of input into the buffer. The returned string may contain
 * newline characters that come from multiline editing or a paste.
 */
TL_DEF tl_status_code tl_get_input(char *buffer, size_t buffer_size,
                                   const char *prompt);
/**
 * Predefine input for `tl_get_input()`.
 */
TL_DEF void tl_set_predefined_input(const char *str);
/**
 * Read a character without waiting and modify `tl_last_control_sequence`.
 * `char_buffer_size` must be between 2 and 5: one UTF-8 character of up to
 * four bytes plus the null terminator.
 */
TL_DEF tl_status_code tl_get_character(char *char_buffer,
                                       size_t char_buffer_size,
                                       const char *prompt);
/**
 * Load history from a file.
 *
 * Returns `TL_SUCCESS` or `TL_ERROR`, and sets `errno` to `EINVAL` for an
 * invalid file or to the underlying value on other failures.
 */
TL_DEF tl_status_code tl_history_load(const char *file_path);
/**
 * Dump history to a file, overwriting it.
 *
 * Returns `TL_SUCCESS` or `TL_ERROR`, and sets `errno` to `EINVAL` for an
 * invalid file or to the underlying value on other failures.
 */
TL_DEF tl_status_code tl_history_dump(const char *file_path);
/** Enable or disable the automatic history append performed on submission. */
TL_DEF void tl_set_history_enabled(bool enabled);
/** Bound the number of history entries retained for recall and listing. */
TL_DEF void tl_set_history_limit(size_t entry_count);
/**
 * Returns the number of UTF-8 characters, which strlen() cannot since it counts
 * bytes.
 */
TL_DEF size_t tl_utf8_strlen(const char *utf8_str);
/**
 * Same as above, except it stops after reading `byte_count` bytes from the
 * string.
 */
TL_DEF size_t tl_utf8_strnlen(const char *utf8_str, size_t byte_count);
/**
 * Emit newlines after getting the input.
 *
 * *buffer should be the buffer used in tl_readline().
 */
TL_DEF tl_status_code tl_emit_newlines(const char *buffer);
/**
 * Sets a new title for the terminal. Returns -1 and does nothing if stdout is
 * not a tty or amount of bytes written.
 */
TL_DEF tl_status_code tl_set_title(const char *title);

/**
 * The result a completion callback fills. The host owns the storage and keeps
 * it valid until the next callback call. candidates is an array of count
 * C-strings, each a full replacement for the token. token_start and token_end
 * are codepoint indices bounding the replaced span. longest_common_prefix is
 * the longest shared prefix, inserted on the first TAB.
 */
typedef struct tl_completion
{
  const char *const *candidates;
  size_t count;
  /* An array of count description strings aligned by index with candidates, or
     NULL when no candidate carries a description. The menu shows each one
     dimmed after its candidate. */
  const char *const *descriptions;
  const char *longest_common_prefix;
  size_t token_start;
  size_t token_end;
} tl_completion;

/**
 * The completion callback. The host receives the current buffer, the cursor
 * as a codepoint index, and a result to fill. It returns nonzero when it
 * filled the result and zero when it has nothing to offer.
 */
typedef int (*tl_complete_fn)(const char *buffer, size_t cursor,
                              tl_completion *out, int for_listing);

/**
 * Register the completion callback, or NULL to disable completion. Without one
 * the TAB key returns TL_PRESSED_TAB.
 */
TL_DEF void tl_set_complete_callback(tl_complete_fn callback);

/*
 * Enables or disables the dimmed ghost suggestion shown ahead of the cursor.
 * Enabled by default. When disabled neither completion nor history fills it.
 */
TL_DEF void tl_set_ghost_enabled(int enabled);

/*
 * The ghost history validation callback. It receives a history entry the ghost
 * is about to suggest and returns nonzero to accept it, zero to skip it. NULL
 * accepts every entry.
 */
typedef int (*tl_ghost_validate_fn)(const char *entry);

/*
 * Register the ghost history validation callback, or NULL to accept every
 * entry.
 */
TL_DEF void tl_set_ghost_validate_callback(tl_ghost_validate_fn callback);

/**
 * One colored span of the line. start and end are codepoint indices, start
 * inclusive and end exclusive. sgr is the opening SGR escape such as
 * "\x1b[32m", owned by the host and stable for the callback and its render. The
 * renderer closes every span with a reset.
 */
typedef struct tl_highlight_span
{
  size_t start;
  size_t end;
  const char *sgr;
} tl_highlight_span;

/**
 * The highlight result. The editor provides spans, an array of capacity slots,
 * and the host fills the first count of them. The spans must be sorted by
 * start, non-overlapping, and within the line.
 */
typedef struct tl_highlight
{
  tl_highlight_span *spans;
  size_t count;
  size_t capacity;
} tl_highlight;

/**
 * The highlight callback. The host receives the current buffer and a result to
 * fill with colored spans. It returns nonzero when it filled one or more spans
 * and zero when the line should stay plain.
 */
typedef int (*tl_highlight_fn)(const char *buffer, tl_highlight *out);

/**
 * Register the highlight callback, or NULL to disable highlighting.
 */
TL_DEF void tl_set_highlight_callback(tl_highlight_fn callback);

/**
 * The wake hook for an out-of-band report such as a finished background job.
 * The wait loop calls phase 0 to ask whether anything must print. On a nonzero
 * answer it clears the render block, calls phase 1 for the host to write its
 * CRLF-ended rows, and re-renders the prompt and line below them.
 */
typedef int (*tl_wake_fn)(int phase);

/**
 * Register the wake hook, or NULL to disable it.
 */
TL_DEF void tl_set_wake_callback(tl_wake_fn callback);

TL_DEF void tl_set_edit_mode(int mode);

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

#if defined ITL_WIN32
#define WIN32_LEAN_AND_MEAN

#include <conio.h>
#include <io.h>
#include <stdio.h> /* SEEK_SET and SEEK_END for the file seek macros */
#include <sys/stat.h>
#include <windows.h>

#define STDIN_FILENO  0
#define STDOUT_FILENO 1

#define ITL_ISATTY _isatty

#if !defined TL_USE_STDIO
#define ITL_STDIN  0
#define ITL_STDOUT 1
#define ITL_STDERR 2
#define ITL_FILE   int

#if !defined ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ITL_NO_WIN_ESCAPES
#warning ENABLE_VIRTUAL_TERMINAL_PROCESSING is not defined. Terminal escape    \
         sequences will not work in some terminals, like conhost.exe.
#endif /* !ENABLE_VIRTUAL_TERMINAL_PROCESSING */

/* Binary mode keeps byte offsets exact, the CRT text mode would translate each
   newline to a carriage return plus newline and desync the offset ring. */
#define ITL_FILE_OPEN_FOR_READ(path) _open(path, O_RDONLY | _O_BINARY)
#define ITL_FILE_OPEN_FOR_WRITE(path)                                          \
  _open(path, O_WRONLY | O_CREAT | O_TRUNC | _O_BINARY, _S_IREAD | _S_IWRITE)
#define ITL_FILE_OPEN_FOR_APPEND(path)                                         \
  _open(path, O_WRONLY | O_CREAT | O_APPEND | _O_BINARY, _S_IREAD | _S_IWRITE)
#define ITL_FILE_IS_BAD(file) (file < 0)
#define ITL_FILE_CLOSE        _close
#define ITL_FILE_SEEK(file, offset)                                            \
  (_lseek(file, (long) (offset), SEEK_SET) == (long) (offset))
/* Seeks to the end and yields the resulting offset, the file size, or -1. */
#define ITL_FILE_SEEK_END(file) ((long) _lseek(file, 0L, SEEK_END))
#define ITL_FILE_TELL(file)     ((long) _lseek(file, 0L, SEEK_CUR))

#define ITL_WRITE(fd, buf, size) _write(fd, buf, (unsigned int) (size))
#define ITL_READ(fd, buf, size)  _read(fd, buf, (unsigned int) (size))
#endif /* !ITL_USE_STDIO */

/* <https://learn.microsoft.com/en-US/troubleshoot/windows-client/shell-experience/command-line-string-limitation>
 */
#define ITL_STRING_MAX_LEN 8191

#if !defined ITL_TTY_IS_TTY
#define ITL_TTY_IS_TTY() ITL_ISATTY(STDIN_FILENO)
#endif /* ITL_TTY_IS_TTY */

#elif defined ITL_POSIX
#if !defined _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <poll.h>
#include <termios.h>
#include <unistd.h>

/* It makes no sense to use escapes on WIN32 which does not support them
   anyway */
#if defined TL_SIZE_USE_ESCAPES
#if defined ITL_POSIX || defined ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ITL_VT_SIZE
#else
#warning Will not use terminal escapes for size.
#endif /* ITL_POSIX */
#elif defined ITL_POSIX
#include <sys/ioctl.h>
#endif /* TL_SIZE_USE_ESCAPES */

#define ITL_ISATTY isatty

#if !defined TL_USE_STDIO
#define ITL_STDIN  0
#define ITL_STDOUT 1
#define ITL_STDERR 2
#define ITL_FILE   int

#define ITL_FILE_OPEN_FOR_READ(path) open(path, O_RDONLY)
#define ITL_FILE_OPEN_FOR_WRITE(path)                                          \
  open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)
#define ITL_FILE_OPEN_FOR_APPEND(path)                                         \
  open(path, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR)
#define ITL_FILE_IS_BAD(file) (file < 0)
#define ITL_FILE_CLOSE        close
#define ITL_FILE_SEEK(file, offset)                                            \
  (lseek(file, (off_t) (offset), SEEK_SET) == (off_t) (offset))
/* Seeks to the end and yields the resulting offset, the file size, or -1. */
#define ITL_FILE_SEEK_END(file) ((long) lseek(file, (off_t) 0, SEEK_END))
#define ITL_FILE_TELL(file)     ((long) lseek(file, (off_t) 0, SEEK_CUR))

#define ITL_WRITE(fd, buf, size) write(fd, buf, (unsigned long) size)
#define ITL_READ(fd, buf, size)  read(fd, buf, (unsigned long) size)
#endif /* !ITL_USE_STDIO */

/* <https://man7.org/linux/man-pages/man3/termios.3.html> */
#define ITL_STRING_MAX_LEN 4095

#if !defined ITL_TTY_IS_TTY
#define ITL_TTY_IS_TTY() ITL_ISATTY(STDIN_FILENO)
#endif /* ITL_TTY_IS_TTY */
#endif /* ITL_POSIX */

#if defined TL_DEBUG || defined TL_USE_STDIO || defined TL_SEE_BYTES
#include <stdio.h>
#endif /* TL_DEBUG */

/* This is almost everything that this library requires for IO. If a different
   underlying API is desired, this may easily be extended. Please note that
   `errno` is required to be set appropriately for errors. Since `stdio` sucks,
   there are still some special cases, like the reason of `feof()` existence,
   so be aware :3 */
#if defined TL_USE_STDIO
#define ITL_STDIN  stdin
#define ITL_STDOUT stdout
#define ITL_STDERR stderr
#define ITL_FILE   FILE *

#define ITL_FILE_OPEN_FOR_READ(path)   fopen(path, "rb")
#define ITL_FILE_OPEN_FOR_WRITE(path)  fopen(path, "wb")
#define ITL_FILE_OPEN_FOR_APPEND(path) fopen(path, "ab")
#define ITL_FILE_IS_BAD(file)          (file == NULL)
#define ITL_FILE_CLOSE                 fclose
#define ITL_FILE_SEEK(file, offset)                                            \
  (fseek(file, (long) (offset), SEEK_SET) == 0)
/* Seeks to the end and yields the resulting offset, the file size, or -1. */
#define ITL_FILE_SEEK_END(file)                                                \
  (fseek(file, 0L, SEEK_END) == 0 ? ftell(file) : -1L)
#define ITL_FILE_TELL(file) ftell(file)

ITL_DEF int itl_write_impl(FILE *f, const void *buf, size_t size)
{
  /* Return the byte count, matching write()/_write(), so callers can detect a
     short write rather than only a hard error. */
  size_t written_bytes = fwrite(buf, 1, size, f);
  fflush(f);
  return ferror(f) ? -1 : (int) written_bytes;
}

#define ITL_WRITE(file, buf, size) itl_write_impl(file, buf, size)
#define ITL_READ(file, buf, size)  fread(buf, 1, size, file)
#endif /* ITL_USE_STDIO */

#if defined ITL_WIN32
/* Windows can't read arrow keys otherwise */
#define ITL_READ_BYTE_RAW _getch
#else /* ITL_WIN32 */
ITL_DEF int ITL_READ_BYTE_RAW(void)
{
#if !defined ITL_INJECT_KLEE
  unsigned char byte_value;
  return (ITL_READ(ITL_STDIN, &byte_value, 1) != 1) ? -1 : (int) byte_value;
#else
  static size_t i = 0;
  if (i >= ITL_KLEE_BUFFER_SIZE - 1) {
    return -1;
  }
  return ITL_KLEE_BUFFER[i++];
#endif
}
#endif

#if defined ITL_DEFAULT_ASSERT
#if defined TL_DEBUG
#define TL_ASSERT(condition)                                                   \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr, "\n%s:%d: assert fail: %s\n", __FILE__, __LINE__,        \
              #condition);                                                     \
      fflush(stderr);                                                          \
      itl_debug_trap();                                                        \
    }                                                                          \
  } while (0)
#else /* TL_DEBUG */
#define TL_ASSERT(condition)                                                   \
  do {                                                                         \
    if (!(condition)) {                                                        \
      const char *m = "\n" __FILE__ ": assert fail: " #condition "\n";         \
      ITL_WRITE(ITL_STDERR, m, strlen(m));                                     \
      itl_debug_trap();                                                        \
    }                                                                          \
  } while (0)
#endif
#endif /* ITL_DEFAULT_ASSERT */

#if !defined __STDC_VERSION__ || __STDC_VERSION__ < 199409L
#define ITL_C89
#if !defined __cplusplus
#define ITL_ZERO_INIT {0}
typedef unsigned char bool;
#define true  1
#define false 0
#else
#define ITL_ZERO_INIT                                                          \
  {}
#endif /* __cplusplus */
#else
#include <stdbool.h>
#define ITL_ZERO_INIT {0}
#endif /* !__STDC_VERSION__ || __STDC_VERSION__ >= 199409L */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h> /* sig_atomic_t for the terminal resize flag */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined ITL_POSIX || defined ITL_SUSPEND
#include <signal.h>
#endif

#if defined _MSC_VER
#include <intrin.h>
#define ITL_THREAD_LOCAL         __declspec(thread)
#define ITL_NO_RETURN            __declspec(noreturn)
#define ITL_MAYBE_UNUSED         /* nothing */
#define ITL_UNREACHABLE_INTRIN() __assume(0)
#define itl_debug_trap()         __debugbreak()
#elif defined __GNUC__ || defined __clang__
#define ITL_THREAD_LOCAL         __thread
#define ITL_NO_RETURN            __attribute__((noreturn))
#define ITL_MAYBE_UNUSED         __attribute__((unused))
#define ITL_UNREACHABLE_INTRIN() __builtin_unreachable()
#define itl_debug_trap()         __builtin_trap()
#else
#define ITL_MAYBE_UNUSED /* nothing */
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L
#define ITL_THREAD_LOCAL _Thread_local
#define ITL_NO_RETURN    _Noreturn
#else                    /* __STDC_VERSION__ && __STDC_VERSION__ >= 201112L */
#define ITL_THREAD_LOCAL /* nothing */
#define ITL_NO_RETURN    /* nothing */
#endif
#define ITL_UNREACHABLE_INTRIN()                                               \
  do {                                                                         \
    abort();                                                                   \
  } while (true)
#define itl_debug_trap() ITL_UNREACHABLE_INTRIN()
#endif

#if defined TL_DEBUG
ITL_NO_RETURN ITL_DEF void itl_unreachable_impl(const char *file, int line,
                                                const char *message)
{
  fprintf(stderr, "%s:%d: %s\n", file, line, message);
  fflush(stderr);
  ITL_UNREACHABLE_INTRIN();
}
#define ITL_UNREACHABLE()                                                      \
  itl_unreachable_impl(__FILE__, __LINE__, "unreachable fail")
#else /* TL_DEBUG */
#define ITL_UNREACHABLE() ITL_UNREACHABLE_INTRIN()
#endif

#if defined TL_DEBUG
#define ITL_TRACELN(...) fprintf(stderr, "\n[TRACE] " __VA_ARGS__)
#else /* TL_DEBUG */
#if defined ITL_C89
ITL_DEF inline void itl_do_nothing() {}
#define ITL_TRACELN(...) itl_do_nothing()
#else                    /* ITL_C89 */
#define ITL_TRACELN(...) /* nothing */
#endif
#endif

#define ITL_MAX(i, j) (((i) > (j)) ? (i) : (j))
#define ITL_MIN(i, j) (((i) < (j)) ? (i) : (j))

#define ITL_TRY(expr, catch_)                                                  \
  do {                                                                         \
    if (!(expr)) {                                                             \
      ITL_TRACELN("\n%s:%d: try fail: %s\n", __FILE__, __LINE__, #expr);       \
      catch_;                                                                  \
    }                                                                          \
  } while (0)

#define ITL_PTR_ASSIGN(p, val)                                                 \
  do {                                                                         \
    if ((p) != NULL) {                                                         \
      *(p) = val;                                                              \
    }                                                                          \
  } while (0)

ITL_DEF ITL_THREAD_LOCAL bool itl_g_is_active = false;
ITL_DEF ITL_THREAD_LOCAL bool itl_g_entered_raw_mode = false;
#if defined ITL_WIN32
ITL_DEF ITL_THREAD_LOCAL DWORD itl_g_original_tty_in_mode = 0;
ITL_DEF ITL_THREAD_LOCAL DWORD itl_g_original_tty_out_mode = 0;
ITL_DEF ITL_THREAD_LOCAL UINT itl_g_original_tty_cp = 0;
ITL_DEF ITL_THREAD_LOCAL int itl_g_original_mode = 0;
#elif defined ITL_POSIX
ITL_DEF ITL_THREAD_LOCAL struct termios itl_g_original_tty_mode = ITL_ZERO_INIT;
#endif /* ITL_POSIX */

ITL_DEF bool itl_enter_raw_mode_impl(void)
{
#if defined ITL_WIN32
  int mode = 0;
  UINT codepage = 0;
  DWORD tty_in_mode = 0, tty_out_mode = 0;
  HANDLE stdin_handle = NULL, stdout_handle = NULL;

  stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
  ITL_TRY(stdin_handle != INVALID_HANDLE_VALUE, return false);
  stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  ITL_TRY(stdout_handle != INVALID_HANDLE_VALUE, return false);

  ITL_TRY(GetConsoleMode(stdout_handle, &tty_out_mode), return false);
  ITL_TRY(GetConsoleMode(stdin_handle, &tty_in_mode), return false);

  itl_g_original_tty_in_mode = tty_in_mode;
  /* Raw input disables every cooked-mode and event source flag, the same full
     reset the previous code did, so echo, line input, mouse, window, and
     quick-edit events are all off. */
  tty_in_mode = (DWORD) 0;
#if !defined ITL_NO_WIN_ESCAPES && defined ENABLE_VIRTUAL_TERMINAL_INPUT
  /* Virtual terminal input makes the console deliver special keys as the escape
     sequences the input parser already decodes. */
  tty_in_mode |= (DWORD) ENABLE_VIRTUAL_TERMINAL_INPUT;
#endif

  itl_g_original_tty_out_mode = tty_out_mode;
#if !defined ITL_NO_WIN_ESCAPES
  tty_out_mode = (DWORD) ENABLE_PROCESSED_OUTPUT |
                 ENABLE_VIRTUAL_TERMINAL_PROCESSING |
                 DISABLE_NEWLINE_AUTO_RETURN;
#else /* !ITL_NO_WIN_ESCAPES */
  tty_out_mode = (DWORD) 0;
#endif

  ITL_TRY(SetConsoleMode(stdin_handle, tty_in_mode), return false);
  ITL_TRY(SetConsoleMode(stdout_handle, tty_out_mode), return false);

  codepage = GetConsoleCP();
  ITL_TRY(codepage != 0, return false);

  itl_g_original_tty_cp = codepage;
  ITL_TRY(SetConsoleCP(CP_UTF8), return false);

  mode = _setmode(STDIN_FILENO, _O_BINARY);
  ITL_TRY(mode != -1, return false);

  itl_g_original_mode = mode;
#elif defined ITL_POSIX
  struct termios term;
  ITL_TRY(tcgetattr(STDIN_FILENO, &term) == 0, return false);

  itl_g_original_tty_mode = term;
  cfmakeraw(&term);
  term.c_oflag = OPOST | ONLCR;

  /* TCSADRAIN keeps the pty's queued input, so a command typed ahead while
     the previous one ran, or sent by tmux before the shell finished
     starting, is read at the prompt the way bash replays it. TCSAFLUSH
     would discard that type-ahead. */
  ITL_TRY(tcsetattr(STDIN_FILENO, TCSADRAIN, &term) == 0, return false);
#endif /* ITL_POSIX */
  return true;
}

ITL_DEF bool itl_exit_raw_mode_impl(void)
{
#if defined ITL_WIN32
  bool something_failed = false;
  HANDLE stdin_handle = NULL, stdout_handle = NULL;

  stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
  ITL_TRY(stdin_handle != INVALID_HANDLE_VALUE, something_failed = true);

  stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  ITL_TRY(stdout_handle != INVALID_HANDLE_VALUE, something_failed = true);

  if (stdin_handle != INVALID_HANDLE_VALUE && itl_g_original_tty_in_mode != 0) {
    ITL_TRY(SetConsoleMode(stdin_handle, itl_g_original_tty_in_mode),
            something_failed = true);
  }
  if (stdout_handle != INVALID_HANDLE_VALUE && itl_g_original_tty_out_mode != 0)
  {
    ITL_TRY(SetConsoleMode(stdout_handle, itl_g_original_tty_out_mode),
            something_failed = true);
  }
  if (itl_g_original_tty_cp != 0) {
    ITL_TRY(SetConsoleCP(itl_g_original_tty_cp), something_failed = true);
  }
  if (itl_g_original_mode != 0) {
    ITL_TRY(_setmode(STDIN_FILENO, itl_g_original_mode) != -1,
            something_failed = true);
  }

  return !something_failed;
#elif defined ITL_POSIX
  struct termios zeroed_termios = ITL_ZERO_INIT;

  if (memcmp(&itl_g_original_tty_mode, &zeroed_termios,
             sizeof(struct termios)) != 0)
  {
    /* The same TCSADRAIN as the enter side, so bytes typed during the last
       edit survive into the command about to read them. */
    ITL_TRY(tcsetattr(STDIN_FILENO, TCSADRAIN, &itl_g_original_tty_mode) == 0,
            return false);
  }

  return true;
#endif /* ITL_POSIX */
}

#define ITL_VI_CURSOR_DEFAULT_SHAPE   0
#define ITL_VI_CURSOR_BLOCK_SHAPE     2
#define ITL_VI_CURSOR_UNDERLINE_SHAPE 4
#define ITL_VI_CURSOR_BAR_SHAPE       6

ITL_DEF ITL_THREAD_LOCAL int itl_g_vi_cursor_shape =
    ITL_VI_CURSOR_DEFAULT_SHAPE;

TL_DEF tl_status_code tl_enter_raw_mode(void)
{
  ITL_TRY(!itl_g_entered_raw_mode, return TL_SUCCESS);

  ITL_TRY(ITL_TTY_IS_TTY(), return TL_ERROR);

  ITL_TRY(itl_enter_raw_mode_impl(), {
    itl_exit_raw_mode_impl();
    return TL_ERROR;
  });

  itl_g_entered_raw_mode = true;

#if defined ITL_POSIX
  /* Ask the terminal to wrap pasted text in markers so a multiline paste does
     not submit. Only the POSIX parser decodes the markers, so on Windows the
     request would leak the bracket bytes as text. This is non-fatal when the
     terminal ignores it. */
  ITL_TRY(ITL_WRITE(ITL_STDOUT, "\x1b[?2004h", 8) != -1, {});
#endif

  return TL_SUCCESS;
}

TL_DEF tl_status_code tl_exit_raw_mode(void)
{
  ITL_TRY(itl_g_entered_raw_mode, return TL_SUCCESS);

  ITL_TRY(ITL_TTY_IS_TTY(), return TL_ERROR);
  ITL_TRY(itl_exit_raw_mode_impl(), return TL_ERROR);

#if defined ITL_POSIX
  ITL_TRY(ITL_WRITE(ITL_STDOUT, "\x1b[?2004l", 8) != -1, {});
#endif

  if (itl_g_vi_cursor_shape != ITL_VI_CURSOR_DEFAULT_SHAPE) {
    ITL_TRY(ITL_WRITE(ITL_STDOUT, "\x1b[0 q", 5) != -1, {});
    itl_g_vi_cursor_shape = ITL_VI_CURSOR_DEFAULT_SHAPE;
  }

  itl_g_entered_raw_mode = false;

  return TL_SUCCESS;
}

/* Holds at most one pushed-back byte, or -1 when empty. The read path drains
   it before the descriptor and the pending probe counts it as input. */
ITL_DEF ITL_THREAD_LOCAL int itl_g_pushback_byte = -1;

ITL_DEF bool ITL_READ_BYTE(uint8_t *buffer)
{
  int byte;
  if (itl_g_pushback_byte != -1) {
    ITL_PTR_ASSIGN(buffer, (uint8_t) itl_g_pushback_byte);
    itl_g_pushback_byte = -1;
    return true;
  }
#if defined ITL_POSIX
  /* Retry across signals so a delivered SIGWINCH or SIGCONT does not abort
     input. Catch real `read()` errors. `_getch()` on Windows has no error
     return. */
  do {
    errno = 0;
    byte = ITL_READ_BYTE_RAW();
  } while (byte == -1 && errno == EINTR);
  ITL_TRY(byte != -1, return false);
#else  /* ITL_POSIX */
  byte = ITL_READ_BYTE_RAW();
#endif /* ITL_POSIX */
  ITL_PTR_ASSIGN(buffer, (uint8_t) byte);
  return true;
}

#define ITL_TRY_READ_BYTE(buffer, expr) ITL_TRY(ITL_READ_BYTE(buffer), expr)

/* Returns true when a byte is already available without blocking, so the
   key wait loop skips its sleep when input is ready. */
ITL_DEF bool itl_input_is_pending(void)
{
  if (itl_g_pushback_byte != -1) {
    return true;
  }
#if defined ITL_INJECT_KLEE
  return false;
#elif defined ITL_WIN32
  return _kbhit() != 0;
#elif defined ITL_POSIX
  struct pollfd pfd;
  pfd.fd = STDIN_FILENO;
  pfd.events = POLLIN;
  pfd.revents = 0;
  return poll(&pfd, 1, 0) > 0;
#else
  return false;
#endif
}

#if defined ITL_POSIX && !defined ITL_INJECT_KLEE
/* Blocks until input arrives or a signal interrupts the wait. A delivered
   SIGWINCH wakes this so the caller can redraw the line as the window resizes
   instead of waiting for the next keystroke. */
ITL_DEF void itl_wait_for_input(void)
{
  struct pollfd pfd;
  pfd.fd = STDIN_FILENO;
  pfd.events = POLLIN;
  pfd.revents = 0;
  poll(&pfd, 1, -1);
}
#endif /* ITL_POSIX && !ITL_INJECT_KLEE */

ITL_DEF volatile sig_atomic_t itl_g_tty_changed_size = 1;

/* True until the first render of an input session. The first render has no
   previous block on screen to reflow, so it must draw in place rather than
   moving up and clearing as a resize would. */
ITL_DEF ITL_THREAD_LOCAL bool itl_g_tty_first_render = true;

#if defined ITL_SUSPEND
#if defined ITL_POSIX
ITL_DEF void itl_raise_suspend(void)
{
#if !defined ITL_INJECT_KLEE
  /* Leave raw mode, stop, and resume here when continued. raise() returns only
     after SIGCONT, so raw mode is restored in normal context without a handler
     calling unsafe terminal functions. */
  tl_exit_raw_mode();
  raise(SIGTSTP);
  tl_enter_raw_mode();
  itl_g_tty_changed_size = 1;
#endif
}

#else /* ITL_POSIX */
ITL_NO_RETURN ITL_DEF void itl_raise_suspend(void)
{
  tl_exit();
  exit(0);
}
#endif
#endif /* ITL_SUSPEND */

ITL_DEF ITL_THREAD_LOCAL size_t itl_g_alloc_count = 0;

ITL_DEF void *itl_malloc(size_t size)
{
  void *allocated;

  TL_ASSERT(size > 0);

  allocated = TL_MALLOC(size);
  itl_g_alloc_count += 1;

#if !defined TL_NO_ABORT
  ITL_TRY(allocated != NULL, TL_ABORT());
#endif /* !TL_NO_ABORT */

  return allocated;
}

ITL_DEF void *itl_realloc(void *block, size_t size)
{
  void *allocated;

  TL_ASSERT(size > 0);

  if (block == NULL) {
    allocated = TL_MALLOC(size);
    itl_g_alloc_count += 1;
  } else {
    allocated = TL_REALLOC(block, size);
  }

#if !defined TL_NO_ABORT
  ITL_TRY(allocated != NULL, TL_ABORT());
#endif /* !TL_NO_ABORT */

  return allocated;
}

#if defined TL_DEBUG
#define ITL_FREE(ptr)                                                          \
  do {                                                                         \
    TL_ASSERT((ptr) != NULL);                                                  \
    memset(ptr, 0x7F, sizeof(*ptr));                                           \
    TL_FREE(ptr);                                                              \
    itl_g_alloc_count -= 1;                                                    \
  } while (0)
#else /* TL_DEBUG */
#define ITL_FREE(ptr)                                                          \
  do {                                                                         \
    itl_g_alloc_count -= 1;                                                    \
    TL_FREE(ptr);                                                              \
  } while (0)
#endif

typedef struct itl_utf8 itl_utf8_t;

struct itl_utf8
{
  uint8_t bytes[4];
  uint8_t size;
};

ITL_DEF itl_utf8_t itl_utf8_new(const uint8_t *bytes, uint8_t size)
{
  itl_utf8_t ch;

  TL_ASSERT(size <= 4);

  memcpy(ch.bytes, bytes, size);
  ch.size = size;

  return ch;
}

#define ITL_UTF8_COPY(dst, src) memcpy(dst, src, sizeof(itl_utf8_t))

ITL_DEF bool itl_utf8_equal(itl_utf8_t ch1, itl_utf8_t ch2)
{
  TL_ASSERT(ch1.size <= 4 && ch2.size <= 4);

  if (ch1.size != ch2.size ||
      memcmp(ch1.bytes, ch2.bytes, ch1.size * sizeof(uint8_t)) != 0)
  {
    return false;
  }

  return true;
}

ITL_DEF uint8_t itl_ascii_fold_byte(uint8_t byte)
{
  if (byte >= 'A' && byte <= 'Z') return (uint8_t) (byte - 'A' + 'a');
  return byte;
}

ITL_DEF bool itl_utf8_equal_ascii_casefold(itl_utf8_t ch1, itl_utf8_t ch2)
{
  if (itl_utf8_equal(ch1, ch2)) return true;
  if (ch1.size != 1 || ch2.size != 1) return false;
  return itl_ascii_fold_byte(ch1.bytes[0]) ==
         itl_ascii_fold_byte(ch2.bytes[0]);
}

ITL_DEF uint8_t itl_utf8_width(int byte)
{
  if ((byte & 0x80) == 0)
    return 1;
  else if ((byte & 0xE0) == 0xC0)
    return 2;
  else if ((byte & 0xF0) == 0xE0)
    return 3;
  else if ((byte & 0xF8) == 0xF0)
    return 4;
  else
    return 0; /* invalid character */
}

#define ITL_UTF8_IS_SURROGATE(first_byte, second_byte)                         \
  (((first_byte) == 0xED) && ((second_byte) >= 0xA0 && (second_byte) <= 0xBF))

ITL_DEF const itl_utf8_t itl_replacement_character = {
    {0xEF, 0xBF, 0xBD},
    3
};

ITL_DEF const itl_utf8_t itl_newline_char = {{0x0A}, 1};

#define ITL_LE_IS_NEWLINE(ch)   ((ch).size == 1 && (ch).bytes[0] == 0x0A)
#define ITL_LE_IS_BACKSLASH(ch) ((ch).size == 1 && (ch).bytes[0] == 0x5C)

ITL_DEF itl_utf8_t itl_utf8_parse(uint8_t first_byte)
{
  uint8_t i, size;
  uint8_t bytes[4];

  if ((size = itl_utf8_width(first_byte)) == 0) { /* invalid character */
    ITL_TRACELN("Invalid UTF-8 sequence '%d'\n", (uint8_t) first_byte);
    return itl_replacement_character;
  }

  bytes[0] = first_byte;

  for (i = 1; i < size; ++i) {
    ITL_TRY_READ_BYTE(&bytes[i], return itl_replacement_character);
    /* Each continuation byte must match the bit pattern 0b10xxxxxx. */
    if ((bytes[i] & 0xC0) != 0x80) {
      ITL_TRACELN("Invalid UTF-8 continuation byte '%02X'\n", bytes[i]);
      return itl_replacement_character;
    }
  }

  /* Codepoints U+D800 to U+DFFF (known as UTF-16 surrogates) are invalid. */
  if (size > 1 && ITL_UTF8_IS_SURROGATE(first_byte, bytes[1])) {
    ITL_TRACELN("Invalid UTF-16 surrogate: '%02X %02X'\n", first_byte,
                bytes[1]);
    return itl_replacement_character;
  }

#if defined TL_DEBUG
  ITL_TRACELN("utf8 char size: %u\n", size);
  ITL_TRACELN("utf8 char bytes: '");

  for (i = 0; i < size; ++i) {
    ITL_TRACELN("%02X ", bytes[i]);
  }
#endif /* TL_DEBUG */

  return itl_utf8_new(bytes, size);
}

#define ITL_UTF8_FREE(c) itl_free(c)

#define ITL_COUNTOF(a) (sizeof(a) / sizeof((a)[0]))

typedef struct itl_cp_interval itl_cp_interval_t;

struct itl_cp_interval
{
  uint32_t first;
  uint32_t last;
};

/* Sorted ranges of zero-width and combining codepoints. */
ITL_DEF const itl_cp_interval_t itl_zero_width_intervals[] = {
    {0x0300, 0x036F},
    {0x0483, 0x0489},
    {0x0591, 0x05BD},
    {0x0610, 0x061A},
    {0x064B, 0x065F},
    {0x0670, 0x0670},
    {0x06D6, 0x06DC},
    {0x0E31, 0x0E31},
    {0x0E34, 0x0E3A},
    {0x200B, 0x200F},
    {0x2060, 0x2064},
    {0xFE00, 0xFE0F},
    {0xFE20, 0xFE2F},
};

/* Sorted ranges of East Asian wide, fullwidth, and common emoji codepoints. */
ITL_DEF const itl_cp_interval_t itl_wide_intervals[] = {
    {0x1100,  0x115F },
    {0x2E80,  0x303E },
    {0x3041,  0x33FF },
    {0x3400,  0x4DBF },
    {0x4E00,  0x9FFF },
    {0xA000,  0xA4CF },
    {0xAC00,  0xD7A3 },
    {0xF900,  0xFAFF },
    {0xFE10,  0xFE19 },
    {0xFE30,  0xFE6F },
    {0xFF00,  0xFF60 },
    {0xFFE0,  0xFFE6 },
    {0x1F300, 0x1FAFF},
    {0x20000, 0x3FFFD},
};

ITL_DEF bool itl_cp_in_table(uint32_t cp, const itl_cp_interval_t *table,
                             size_t count)
{
  size_t low = 0, high = count;

  while (low < high) {
    size_t mid = low + (high - low) / 2;
    if (cp < table[mid].first) {
      high = mid;
    } else if (cp > table[mid].last) {
      low = mid + 1;
    } else {
      return true;
    }
  }
  return false;
}

ITL_DEF uint32_t itl_utf8_codepoint(itl_utf8_t ch)
{
  switch (ch.size) {
  case 1: return ch.bytes[0];
  case 2:
    return (uint32_t) (((ch.bytes[0] & 0x1F) << 6) | (ch.bytes[1] & 0x3F));
  case 3:
    return (uint32_t) (((ch.bytes[0] & 0x0F) << 12) |
                       ((ch.bytes[1] & 0x3F) << 6) | (ch.bytes[2] & 0x3F));
  default:
    return (uint32_t) (((ch.bytes[0] & 0x07) << 18) |
                       ((ch.bytes[1] & 0x3F) << 12) |
                       ((ch.bytes[2] & 0x3F) << 6) | (ch.bytes[3] & 0x3F));
  }
}

/* Returns the terminal column width of a character, which is 0, 1, or 2. Tab is
   counted as a single column. A newline is handled by the renderer, not here.
 */
ITL_DEF size_t itl_char_width(itl_utf8_t ch)
{
  uint32_t cp = itl_utf8_codepoint(ch);

  if (cp == 0x09) {
    return 1;
  }
  if (cp < 0x20 || (cp >= 0x7F && cp < 0xA0)) {
    return 0;
  }
  if (itl_cp_in_table(cp, itl_zero_width_intervals,
                      ITL_COUNTOF(itl_zero_width_intervals)))
  {
    return 0;
  }
  if (itl_cp_in_table(cp, itl_wide_intervals, ITL_COUNTOF(itl_wide_intervals)))
  {
    return 2;
  }
  return 1;
}

/* The walker consumes at most byte_length bytes and stops at a null byte. */
ITL_DEF size_t itl_strn_width_walk(const char *cstr, size_t byte_length,
                                   size_t stop_after, size_t *out_offset)
{
  size_t width = 0, i = 0;

  if (cstr == NULL) {
    if (out_offset != NULL) {
      *out_offset = 0;
    }
    return 0;
  }

  while (i < byte_length && cstr[i] != '\0') {
    /* An ANSI escape sequence such as a color code or a window-title set
       occupies no terminal columns, so skip it whole. A prompt that carries one
       would otherwise push the caret right by the length of its escape bytes.
     */
    if ((uint8_t) cstr[i] == 0x1b) {
      i += 1;
      if (i >= byte_length) break;
      if (cstr[i] == '[') {
        /* A CSI sequence runs until a byte in the final range. */
        i += 1;
        while (i < byte_length && cstr[i] != '\0' &&
               (cstr[i] < 0x40 || cstr[i] > 0x7e))
        {
          i += 1;
        }
        if (i < byte_length && cstr[i] != '\0') {
          i += 1;
        }
      } else if (cstr[i] == ']') {
        /* An OSC sequence such as a title set runs until a BEL or a string
           terminator, ESC backslash. Its body is non-printing, so the whole run
           is skipped or the title text would be counted as caret columns. */
        i += 1;
        while (i < byte_length && cstr[i] != '\0' &&
               (uint8_t) cstr[i] != 0x07 &&
               !(i + 1 < byte_length && (uint8_t) cstr[i] == 0x1b &&
                 cstr[i + 1] == '\\'))
        {
          i += 1;
        }
        if (i + 1 < byte_length && (uint8_t) cstr[i] == 0x1b &&
            cstr[i + 1] == '\\')
        {
          i += 2;
        } else if (i < byte_length && cstr[i] != '\0') {
          i += 1;
        }
      } else if (i < byte_length && cstr[i] != '\0') {
        /* A two-byte escape such as a charset select, ESC then one byte. */
        i += 1;
      }
      continue;
    }

    uint8_t rune_width = itl_utf8_width((uint8_t) cstr[i]);
    itl_utf8_t ch;
    uint8_t j;

    if (rune_width == 0 || i + rune_width > byte_length) {
      if (width >= stop_after) break;
      width += 1;
      i += 1;
      continue;
    }
    for (j = 1; j < rune_width; ++j) {
      if (((uint8_t) cstr[i + j] & 0xC0) != 0x80) break;
    }
    if (j != rune_width) {
      if (width >= stop_after) break;
      width += 1;
      i += 1;
      continue;
    }
    for (j = 0; j < rune_width; ++j) {
      ch.bytes[j] = (uint8_t) cstr[i + j];
    }
    ch.size = j;
    {
      size_t character_width = itl_char_width(ch);
      if (character_width > 0 && width >= stop_after) break;
      width += character_width;
    }
    i += j;
  }

  if (out_offset != NULL) {
    *out_offset = i;
  }
  return width;
}

ITL_DEF size_t itl_cstr_width_walk(const char *cstr, size_t stop_after,
                                   size_t *out_offset)
{
  return itl_strn_width_walk(cstr, (size_t) -1, stop_after, out_offset);
}

ITL_DEF size_t itl_cstr_display_width(const char *cstr)
{
  return itl_cstr_width_walk(cstr, (size_t) -1, NULL);
}

ITL_DEF size_t itl_strn_display_width(const char *cstr, size_t byte_length)
{
  return itl_strn_width_walk(cstr, byte_length, (size_t) -1, NULL);
}

/* The display width of the prompt's last row and, through out_rows, the count
   of newlines before it. A single-row prompt reports its whole width and
   zero rows, so the caller's existing math is unchanged, while a multi-row
   prompt reports only the trailing row the cursor sits after. */
ITL_DEF size_t itl_prompt_last_row_width(const char *cstr, size_t *out_rows)
{
  size_t rows = 0;
  const char *last_row = cstr;
  const char *p;

  if (cstr == NULL) {
    if (out_rows != NULL) *out_rows = 0;
    return 0;
  }
  for (p = cstr; *p != '\0'; ++p) {
    if (*p == '\n') {
      rows += 1;
      last_row = p + 1;
    }
  }
  if (out_rows != NULL) *out_rows = rows;
  return itl_cstr_display_width(last_row);
}

#define ITL_PROMPT_ELLIPSIS       "..."
#define ITL_PROMPT_ELLIPSIS_WIDTH 3
/* The input cells a clamped prompt always leaves free on the first row. */
#define ITL_PROMPT_MIN_INPUT_CELLS 8

/* The byte offset prompt rendering starts from and the cells the rendered
   prompt occupies, for the given terminal width. A prompt narrower than the
   terminal renders whole from offset zero. A wider one renders as the
   ellipsis marker and its own tail, the way fish shortens an oversized
   prompt, so the cursor math never sees a prompt at or past the terminal
   width and the first row keeps room for input. */
ITL_DEF size_t itl_prompt_render_cut(const char *prompt, size_t prompt_width,
                                     size_t cols, size_t *out_width)
{
  size_t budget, dropped, cut_offset;

  if (prompt == NULL) {
    *out_width = 0;
    return 0;
  }
  if (prompt_width < cols) {
    *out_width = prompt_width;
    return 0;
  }
  if (cols <= ITL_PROMPT_ELLIPSIS_WIDTH + ITL_PROMPT_MIN_INPUT_CELLS) {
    /* The terminal is too narrow for a useful tail, so the prompt renders as
       nothing and the whole row belongs to the input. */
    *out_width = 0;
    return strlen(prompt);
  }
  budget = cols - ITL_PROMPT_ELLIPSIS_WIDTH - ITL_PROMPT_MIN_INPUT_CELLS;
  dropped = itl_cstr_width_walk(prompt, prompt_width - budget, &cut_offset);
  *out_width = ITL_PROMPT_ELLIPSIS_WIDTH + (prompt_width - dropped);
  return cut_offset;
}

#define ITL_STRING_INIT_SIZE                      64
#define ITL_STRING_REALLOC_CAPACITY(old_capacity) (((old_capacity) * 3) >> 1)

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
  str->chars = (itl_utf8_t *) itl_malloc(str->capacity * sizeof(itl_utf8_t));
}

ITL_DEF itl_string_t *itl_string_alloc(void)
{
  itl_string_t *ptr = (itl_string_t *) itl_malloc(sizeof(itl_string_t));
  itl_string_init(ptr);
  return ptr;
}

ITL_DEF void itl_string_extend(itl_string_t *str)
{
  str->capacity = ITL_STRING_REALLOC_CAPACITY(str->capacity);
  str->chars = (itl_utf8_t *) itl_realloc(str->chars,
                                          str->capacity * sizeof(itl_utf8_t));
}

ITL_DEF size_t itl_string_prefix_with_offset(const itl_string_t *str1,
                                             size_t start, size_t end,
                                             const itl_string_t *str2)
{
  size_t i, k, actual_end = ITL_MIN(end, str1->length);

  TL_ASSERT(start <= actual_end);

  for (i = start, k = 0; i < actual_end && k < str2->length; ++i, ++k) {
    if (!itl_utf8_equal(str1->chars[i], str2->chars[k])) {
      break;
    }
  }
  return k;
}

ITL_DEF bool itl_string_equal(const itl_string_t *str1,
                              const itl_string_t *str2)
{
  if (str1->size != str2->size) {
    return false;
  }
  if (str1->size == 0) {
    return true;
  }
  return itl_string_prefix_with_offset(str1, 0, str1->length, str2) ==
         str1->length;
}

ITL_DEF void itl_string_copy(itl_string_t *dst, const itl_string_t *src)
{
  size_t i;

  TL_ASSERT(dst != NULL);
  TL_ASSERT(src != NULL);

  while (dst->capacity < src->capacity) {
    itl_string_extend(dst);
  }
  for (i = 0; i < src->length; ++i) {
    ITL_UTF8_COPY(&dst->chars[i], &src->chars[i]);
  }

  dst->length = src->length;
  dst->size = src->size;
}

ITL_DEF void itl_string_recalc_size(itl_string_t *str)
{
  size_t i;
  str->size = 0;

  TL_ASSERT(str->length <= ITL_STRING_MAX_LEN);

  for (i = 0; i < str->length; ++i) {
    TL_ASSERT(str->chars[i].size > 0);
    TL_ASSERT(str->chars[i].size <= 4);
    str->size += str->chars[i].size;
  }
}

ITL_DEF void itl_string_shrink(itl_string_t *str)
{
  str->capacity = ITL_STRING_INIT_SIZE;
  str->chars = (itl_utf8_t *) itl_realloc(str->chars,
                                          str->capacity * sizeof(itl_utf8_t));

  if (str->length > str->capacity) {
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

/* Shifts all characters after `position`. When shifting forward, character on
   `position` is duplicated `shift_by` times. Does not recalculate the size */
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
      if (i == 0) {
        break; /* avoid wrapping */
      }
    }
  }
}

ITL_DEF void itl_string_erase(itl_string_t *str, size_t position, size_t count,
                              bool backwards)
{
  size_t erased_size = 0;
  size_t erased_position;
  size_t erased_end;

  ITL_TRACELN("string_erase: pos: %zu, count: %zu, backwards: %d, len %zu\n",
              position, count, backwards, str->length);

  if (count > str->length) {
    count = str->length;
  }

  if (backwards) {
    if (position >= str->length) {
      position = str->length;
    }
    if (count > position) count = position;
  } else {
    if (position >= str->length) {
      return;
    }
    if (count > str->length - position) {
      count = str->length - position;
    }
    position += count;
  }

  erased_position = position - count;
  erased_end = position;
  while (erased_position < erased_end) {
    erased_size += str->chars[erased_position].size;
    erased_position += 1;
  }
  itl_string_shift(str, position, count, true);
  TL_ASSERT(str->size >= erased_size);
  str->size -= erased_size;
}

ITL_DEF void itl_string_insert(itl_string_t *str, size_t position,
                               itl_utf8_t ch)
{
  TL_ASSERT(ch.size > 0);
  TL_ASSERT(ch.size <= 4);

  while (str->capacity < str->length + 1) {
    itl_string_extend(str);
  }

  if (position == str->length) {
    str->length += 1;
  } else {
    itl_string_shift(str, position, 1, false);
  }

  str->chars[position] = ch;
  str->size += ch.size;
}

/* Removes every backslash that is immediately followed by a newline, together
   with that newline, joining the two physical lines like a POSIX shell. */
ITL_DEF void itl_string_join_continuations(itl_string_t *str)
{
  size_t read_index, write_index = 0;

  for (read_index = 0; read_index < str->length; ++read_index) {
    if (read_index + 1 < str->length &&
        ITL_LE_IS_BACKSLASH(str->chars[read_index]) &&
        ITL_LE_IS_NEWLINE(str->chars[read_index + 1]))
    {
      read_index += 1; /* Drop the backslash and the newline that follows it. */
      continue;
    }
    str->chars[write_index] = str->chars[read_index];
    write_index += 1;
  }

  str->length = write_index;
  itl_string_recalc_size(str);
}

/* Returns true when needle occurs as an ASCII-case-folded contiguous run. */
ITL_DEF bool itl_string_find_substring_ascii_casefold(
    const itl_string_t *str, const itl_string_t *needle)
{
  size_t i, j;

  if (needle->length == 0) {
    return true;
  }
  if (needle->length > str->length) {
    return false;
  }

  for (i = 0; i + needle->length <= str->length; ++i) {
    for (j = 0; j < needle->length; ++j) {
      if (!itl_utf8_equal_ascii_casefold(str->chars[i + j],
                                         needle->chars[j]))
      {
        break;
      }
    }
    if (j == needle->length) {
      return true;
    }
  }

  return false;
}

#define ITL_STRING_FREE(str)                                                   \
  do {                                                                         \
    ITL_FREE((str)->chars);                                                    \
    ITL_FREE(str);                                                             \
  } while (0)

#if defined ITL_WIN32
#define ITL_LF     "\r\n"
#define ITL_LF_LEN 2
#elif defined ITL_POSIX
#define ITL_LF     "\n"
#define ITL_LF_LEN 1
#endif /* ITL_POSIX */

ITL_DEF tl_status_code itl_string_to_cstr(const itl_string_t *str, char *cstr,
                                          size_t cstr_size)
{
  size_t i, j, k;

  /* A zero size buffer has no room for even the null terminator, so writing it
     would land past the end. */
  if (cstr_size == 0) {
    return TL_ERROR_SIZE;
  }

  for (i = 0, k = 0; i < str->length; ++i) {
    if (k + 1 >= cstr_size || cstr_size - k - 1 < str->chars[i].size) {
      break;
    }
    for (j = 0; j < str->chars[i].size; ++j, ++k) {
      cstr[k] = (char) str->chars[i].bytes[j];
    }
  }
  cstr[k] = '\0';

  if (k != str->size) {
    return TL_ERROR_SIZE;
  }

  return TL_SUCCESS;
}

ITL_DEF bool itl_string_from_bytes(itl_string_t *str, const char *data,
                                    size_t size)
{
  size_t i, j, k;
  uint8_t rune_width;

  /* Clamp the byte count to the platform cap, the same silent-truncation
     policy itl_history_read_entry_fd uses, so a host setter that feeds an
     oversized string never trips the length assert in itl_string_recalc_size.
     The existing per-rune cutoff below drops a rune straddling the clamp. */
  if (size > ITL_STRING_MAX_LEN) {
    size = ITL_STRING_MAX_LEN;
  }

  for (i = 0, k = 0; k < size; ++i) {
    rune_width = itl_utf8_width((uint8_t) data[k]);
    if (rune_width == 0) {
      return false; /* Something went wrong. */
    }

    /* A trailing multibyte sequence cut off by `size` is dropped instead of
       claiming more bytes than were actually copied. */
    if (k + rune_width > size) {
      break;
    }

    while (str->capacity < i + 1) {
      itl_string_extend(str);
    }

    str->chars[i].size = rune_width;

    for (j = 0; j < rune_width; ++j, ++k) {
      str->chars[i].bytes[j] = (uint8_t) data[k];
    }
  }

  str->length = i;
  itl_string_recalc_size(str);

  return true;
}

/* Requires null-terminated string. */
#define ITL_STRING_FROM_CSTR(str, cstr)                                        \
  itl_string_from_bytes(str, cstr, strlen(cstr))

#define ITL_HISTORY_FILE_BUFFER_SIZE (1024 * 2)

#if !defined ITL_HISTORY_ENTRY_MAX_BYTES
#define ITL_HISTORY_ENTRY_MAX_BYTES 2048
#endif /* ITL_HISTORY_ENTRY_MAX_BYTES */

ITL_DEF ITL_THREAD_LOCAL char *itl_g_history_path = NULL;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_history_offsets[TL_HISTORY_MAX_SIZE];
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_history_head = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_history_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_history_total_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_last_history_event_number = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_history_file_size = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_history_limit = TL_HISTORY_MAX_SIZE;
ITL_DEF ITL_THREAD_LOCAL bool itl_g_history_enabled = true;

struct itl_char_buf;
ITL_DEF ITL_THREAD_LOCAL struct itl_char_buf *itl_g_history_read_buffer = NULL;
ITL_DEF ITL_THREAD_LOCAL bool itl_g_history_read_buffer_loaded = false;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_history_read_buffer_offset = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_history_read_buffer_start = 0;
#if !defined NDEBUG
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_debug_history_buffer_load_count = 0;
#endif
ITL_DEF void itl_history_read_fd_invalidate(void);

/* False when the loaded file's last line lacked a terminating newline, so the
   next append writes a separator first instead of gluing onto that line. */
ITL_DEF ITL_THREAD_LOCAL bool itl_g_history_ends_with_newline = true;

/* The line the user was editing before history navigation started, restored
   when navigation steps back past the newest entry. */
ITL_DEF ITL_THREAD_LOCAL itl_string_t *itl_g_history_draft = NULL;

/* Selected entry index sentinel meaning the editor shows the draft line rather
   than a stored entry. */
#define ITL_HISTORY_NONE ((size_t) -1)

ITL_DEF ITL_THREAD_LOCAL itl_string_t itl_g_line_buffer = ITL_ZERO_INIT;

/* The whole history entry the ghost currently suggests, kept across keystrokes
   so typing further into the suggestion stays on the same entry rather than
   re-scanning and flipping to a more recent one. Empty when no history entry is
   being suggested. Cleared when a fresh line starts in itl_le_init. */
ITL_DEF ITL_THREAD_LOCAL char itl_g_ghost_sticky_target[ITL_STRING_MAX_LEN] = {
    0};
ITL_DEF ITL_THREAD_LOCAL char
    itl_g_ghost_completion_miss_prefix[ITL_STRING_MAX_LEN] = {0};
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_ghost_completion_miss_prefix_length = 0;
ITL_DEF ITL_THREAD_LOCAL char
    itl_g_ghost_history_miss_prefix[ITL_STRING_MAX_LEN] = {0};
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_ghost_history_miss_prefix_length = 0;
ITL_DEF ITL_THREAD_LOCAL char itl_g_serialized_line[ITL_STRING_MAX_LEN] = {0};
ITL_DEF ITL_THREAD_LOCAL bool itl_g_serialized_line_ready = false;

typedef struct itl_le itl_le_t;

/* Line editor */
struct itl_le
{
  itl_string_t *line;
  size_t cursor_position;

  /* Selected history entry index, ITL_HISTORY_NONE while editing the draft. */
  size_t history_selected_index;

  char *out_buf;
  size_t out_size;

  const char *prompt;
  size_t prompt_size;  /* N of bytes in the prompt */
  size_t prompt_width; /* N of columns the prompt's last row occupies */
  size_t prompt_rows;  /* N of newlines in the prompt, 0 for a single row */

  size_t goal_column;
};

#define ITL_VI_REGISTER_COUNT   27
#define ITL_VI_REGISTER_UNNAMED 26
#define ITL_UNDO_STACK_DEPTH    64
#define ITL_VI_SGR_SELECT       "\x1b[1;7m"

typedef enum
{
  ITL_VI_OP_NONE = 0,
  ITL_VI_OP_DELETE,
  ITL_VI_OP_CHANGE,
  ITL_VI_OP_YANK
} itl_vi_operator_kind;

typedef enum
{
  ITL_VI_CHANGE_NONE = 0,
  ITL_VI_CHANGE_OPERATOR,
  ITL_VI_CHANGE_INSERT,
  ITL_VI_CHANGE_REPLACE,
  ITL_VI_CHANGE_TILDE,
  ITL_VI_CHANGE_PASTE
} itl_vi_change_kind;

typedef struct itl_vi_find_state
{
  itl_utf8_t target_char;
  bool is_forward;
  bool is_till;
  bool has_pending;
} itl_vi_find_state;

typedef struct itl_vi_change_record
{
  itl_vi_change_kind kind;
  size_t repeat_count;
  int operator_kind;
  int motion_key;
  itl_utf8_t find_char;
  itl_utf8_t replace_char;
  bool did_enter_insert;
  bool is_paste_before;
  itl_string_t *inserted_text;
} itl_vi_change_record;

typedef struct itl_undo_snapshot
{
  itl_string_t *line;
  size_t cursor_position;
} itl_undo_snapshot;

ITL_DEF ITL_THREAD_LOCAL int itl_g_edit_mode = TL_EDIT_MODE_EMACS;
ITL_DEF ITL_THREAD_LOCAL int itl_g_edit_mode_base = TL_EDIT_MODE_EMACS;

ITL_DEF ITL_THREAD_LOCAL itl_vi_operator_kind itl_g_vi_pending_operator =
    ITL_VI_OP_NONE;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_vi_pending_count = 0;
ITL_DEF ITL_THREAD_LOCAL char itl_g_vi_pending_register = 0;
ITL_DEF ITL_THREAD_LOCAL itl_vi_find_state itl_g_vi_find = ITL_ZERO_INIT;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_vi_visual_anchor = 0;

ITL_DEF ITL_THREAD_LOCAL size_t itl_g_vi_block_anchor = 0;
ITL_DEF ITL_THREAD_LOCAL bool itl_g_vi_block_insert_active = false;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_vi_block_insert_top_line = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_vi_block_insert_row_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_vi_block_insert_column = 0;
ITL_DEF ITL_THREAD_LOCAL int itl_g_vi_block_return_mode = TL_EDIT_MODE_EMACS;
ITL_DEF ITL_THREAD_LOCAL itl_string_t
    *itl_g_vi_registers[ITL_VI_REGISTER_COUNT] = ITL_ZERO_INIT;
ITL_DEF ITL_THREAD_LOCAL bool
    itl_g_vi_register_is_linewise[ITL_VI_REGISTER_COUNT] = ITL_ZERO_INIT;
ITL_DEF ITL_THREAD_LOCAL itl_vi_change_record itl_g_vi_last_change =
    ITL_ZERO_INIT;

ITL_DEF ITL_THREAD_LOCAL itl_undo_snapshot
    itl_g_undo_stack[ITL_UNDO_STACK_DEPTH] = ITL_ZERO_INIT;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_undo_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_undo_head = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_redo_count = 0;
ITL_DEF ITL_THREAD_LOCAL bool itl_g_undo_insert_run_open = false;

ITL_DEF ITL_THREAD_LOCAL bool itl_g_vi_is_recording_insert = false;

ITL_DEF void itl_undo_store(itl_undo_snapshot *slot, const itl_le_t *le)
{
  if (slot->line == NULL) {
    slot->line = itl_string_alloc();
  }
  itl_string_copy(slot->line, le->line);
  slot->cursor_position = le->cursor_position;
}

ITL_DEF void itl_undo_push(itl_le_t *le)
{
  if (itl_g_undo_count > 0) {
    size_t top =
        (itl_g_undo_head + ITL_UNDO_STACK_DEPTH - 1) % ITL_UNDO_STACK_DEPTH;
    if (itl_g_undo_stack[top].line != NULL &&
        itl_string_equal(itl_g_undo_stack[top].line, le->line))
    {
      itl_g_redo_count = 0;
      return;
    }
  }

  itl_undo_store(&itl_g_undo_stack[itl_g_undo_head], le);
  itl_g_undo_head = (itl_g_undo_head + 1) % ITL_UNDO_STACK_DEPTH;
  if (itl_g_undo_count < ITL_UNDO_STACK_DEPTH) {
    itl_g_undo_count += 1;
  }
  itl_g_redo_count = 0;
}

ITL_DEF bool itl_undo_pop(itl_le_t *le)
{
  size_t top;

  if (itl_g_undo_count == 0) {
    return false;
  }

  if (itl_g_undo_count == ITL_UNDO_STACK_DEPTH) {
    itl_g_undo_count -= 1;
  }

  itl_undo_store(&itl_g_undo_stack[itl_g_undo_head], le);
  itl_g_redo_count += 1;

  top = (itl_g_undo_head + ITL_UNDO_STACK_DEPTH - 1) % ITL_UNDO_STACK_DEPTH;
  itl_string_copy(le->line, itl_g_undo_stack[top].line);
  le->cursor_position = itl_g_undo_stack[top].cursor_position;
  if (le->cursor_position > le->line->length) {
    le->cursor_position = le->line->length;
  }

  itl_g_undo_head = top;
  itl_g_undo_count -= 1;

  return true;
}

ITL_DEF bool itl_redo(itl_le_t *le)
{
  size_t redo_index;

  if (itl_g_redo_count == 0) {
    return false;
  }

  redo_index = (itl_g_undo_head + 1) % ITL_UNDO_STACK_DEPTH;
  itl_string_copy(le->line, itl_g_undo_stack[redo_index].line);
  le->cursor_position = itl_g_undo_stack[redo_index].cursor_position;
  if (le->cursor_position > le->line->length) {
    le->cursor_position = le->line->length;
  }

  itl_g_undo_head = redo_index;
  itl_g_undo_count += 1;
  itl_g_redo_count -= 1;

  return true;
}

ITL_DEF void itl_undo_close_insert_run(void)
{
  itl_g_undo_insert_run_open = false;
}

ITL_DEF void itl_undo_reset(void)
{
  itl_g_undo_count = 0;
  itl_g_undo_head = 0;
  itl_g_redo_count = 0;
  itl_g_undo_insert_run_open = false;
}

ITL_DEF void itl_vi_free(void)
{
  size_t i;

  for (i = 0; i < ITL_UNDO_STACK_DEPTH; ++i) {
    if (itl_g_undo_stack[i].line != NULL) {
      ITL_STRING_FREE(itl_g_undo_stack[i].line);
      itl_g_undo_stack[i].line = NULL;
    }
  }

  for (i = 0; i < ITL_VI_REGISTER_COUNT; ++i) {
    if (itl_g_vi_registers[i] != NULL) {
      ITL_STRING_FREE(itl_g_vi_registers[i]);
      itl_g_vi_registers[i] = NULL;
    }
  }

  if (itl_g_vi_last_change.inserted_text != NULL) {
    ITL_STRING_FREE(itl_g_vi_last_change.inserted_text);
    itl_g_vi_last_change.inserted_text = NULL;
  }

  itl_undo_reset();
}

/* Releases the in-memory history state. Entries themselves live in the file, so
   only the path, the draft, and the offset ring counters are reset. */
ITL_DEF void itl_g_history_free(void)
{
  itl_history_read_fd_invalidate();
  if (itl_g_history_path != NULL) {
    ITL_FREE(itl_g_history_path);
    itl_g_history_path = NULL;
  }
  if (itl_g_history_draft != NULL) {
    ITL_STRING_FREE(itl_g_history_draft);
    itl_g_history_draft = NULL;
  }

  itl_g_history_head = 0;
  itl_g_history_count = 0;
  itl_g_history_total_count = 0;
  itl_g_last_history_event_number = 0;
  itl_g_history_file_size = 0;
  itl_g_history_ends_with_newline = true;
}

/* Maps a navigable entry index, where zero is the oldest, onto its byte offset
   in the history file through the ring. */
ITL_DEF size_t itl_history_index_to_offset(size_t index)
{
  TL_ASSERT(index < itl_g_history_count);
  return itl_g_history_offsets[(itl_g_history_head + index) %
                               (TL_HISTORY_MAX_SIZE)];
}

/* Reads the entry that starts at byte offset from the history file into out,
   decoding the backslash escapes the dumper wrote, a backslash n into a newline
   and a doubled backslash into one backslash. The decoded length is capped at
   ITL_STRING_MAX_LEN so one huge command cannot grow the read. Returns false
   when the file cannot be read. */
ITL_DEF bool itl_history_read_entry_fd(ITL_FILE file, size_t offset,
                                       itl_string_t *out)
{
  char chunk[ITL_HISTORY_FILE_BUFFER_SIZE];
  char decoded[ITL_STRING_MAX_LEN];
  size_t decoded_size = 0;
  bool escape_pending = false;
  bool line_done = false;
  bool was_truncated = false;

  ITL_TRACELN("reading history entry at offset %zu\n", offset);

  if (!ITL_FILE_SEEK(file, offset)) {
    ITL_TRACELN("could not seek history file to offset %zu: %s\n", offset,
                strerror(errno));
    return false;
  }

  while (!line_done) {
    int read_amount = (int) ITL_READ(file, chunk, ITL_HISTORY_FILE_BUFFER_SIZE);
    size_t i;

    if (read_amount <= 0) {
      break; /* End of file ends the last entry. */
    }

    for (i = 0; i < (size_t) read_amount && !line_done; ++i) {
      uint8_t ch = (uint8_t) chunk[i];

      if (escape_pending) {
        escape_pending = false;
        if (ch == 'n') {
          ch = 0x0A;
        } else if (ch != '\\') {
          /* An unknown escape keeps the leading backslash, then the byte falls
             through to be appended on its own. */
          if (decoded_size < ITL_STRING_MAX_LEN) {
            decoded[decoded_size++] = '\\';
          } else {
            was_truncated = true;
          }
        }
      } else if (ch == '\\') {
        escape_pending = true;
        continue;
      } else if (ch == '\n') {
        line_done = true;
        continue;
      } else if (ch == '\r') {
        continue;
      }

      if (decoded_size < ITL_STRING_MAX_LEN) {
        decoded[decoded_size++] = (char) ch;
      } else {
        was_truncated = true;
        line_done = true;
      }
    }
  }

  if (was_truncated) {
    ITL_TRACELN("history entry at offset %zu truncated to %d bytes\n", offset,
                (int) ITL_STRING_MAX_LEN);
  }

  return itl_string_from_bytes(out, decoded, decoded_size);
}

/* Opens the history file, reads one entry, and closes it. Callers that read
   many entries in a loop should open once and use itl_history_read_entry_fd
   instead. Returns false when the file cannot be opened or read. */
ITL_DEF bool itl_history_read_entry(size_t offset, itl_string_t *out)
{
  ITL_FILE file;
  bool ok;

  if (itl_g_history_path == NULL) {
    return false;
  }

  file = ITL_FILE_OPEN_FOR_READ(itl_g_history_path);
  if (ITL_FILE_IS_BAD(file)) {
    ITL_TRACELN("could not open history file for read (%s): %s\n",
                itl_g_history_path, strerror(errno));
    return false;
  }

  ok = itl_history_read_entry_fd(file, offset, out);
  ITL_FILE_CLOSE(file);

  return ok;
}

ITL_DEF void itl_le_init(itl_le_t *le, itl_string_t *line_buf, char *out_buf,
                         size_t out_size, const char *prompt)
{
  /* clang-format off */
  le->line                   = line_buf;
  le->cursor_position        = line_buf->length;
  le->history_selected_index = ITL_HISTORY_NONE;
  le->out_buf                = out_buf;
  le->out_size               = out_size;
  le->prompt                 = prompt;
  le->prompt_size            = (prompt != NULL) ? strlen(prompt) : 0;
  le->prompt_width           = itl_prompt_last_row_width(prompt, &le->prompt_rows);
  le->goal_column            = 0;
  /* clang-format on */

  /* A fresh line starts with no sticky ghost target, so the previous line's
     suggestion is not inherited. */
  itl_g_ghost_sticky_target[0] = '\0';
  itl_g_ghost_completion_miss_prefix[0] = '\0';
  itl_g_ghost_completion_miss_prefix_length = 0;
  itl_g_ghost_history_miss_prefix[0] = '\0';
  itl_g_ghost_history_miss_prefix_length = 0;
  itl_g_serialized_line_ready = false;

  /* The next refresh starts a fresh block, so it must not reflow a previous
     render that does not exist. */
  itl_g_tty_first_render = true;

  itl_g_edit_mode = itl_g_edit_mode_base;
  itl_g_vi_pending_operator = ITL_VI_OP_NONE;
  itl_g_vi_pending_count = 0;
  itl_g_vi_pending_register = 0;
  itl_g_vi_visual_anchor = 0;
  itl_g_vi_is_recording_insert = false;
  itl_g_vi_block_insert_active = false;
  itl_undo_reset();
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

  /* A no-op erase changes nothing, so it neither pushes an undo snapshot nor
     clears the redo stack. Forward erase at the line end and backward erase at
     the line start both touch zero characters. */
  if (backwards) {
    if (le->cursor_position == 0) {
      return;
    }
  } else if (le->cursor_position >= le->line->length) {
    return;
  }

  itl_undo_push(le);
  itl_g_undo_insert_run_open = false;

  if (backwards && le->cursor_position) {
    itl_string_erase(le->line, le->cursor_position, count, true);
    itl_le_move_left(le, count);

    if (itl_g_vi_is_recording_insert &&
        itl_g_vi_last_change.inserted_text != NULL)
    {
      size_t recorded_length = itl_g_vi_last_change.inserted_text->length;
      size_t trim_count = (count < recorded_length) ? count : recorded_length;
      if (trim_count > 0) {
        itl_string_erase(itl_g_vi_last_change.inserted_text, recorded_length,
                         trim_count, true);
      }
    }
  } else if (!backwards) {
    itl_string_erase(le->line, le->cursor_position, count, false);
  }

  /* An erase that empties the line drops the sticky ghost target, so a retype
     picks the newest match again rather than re-deriving the old target the
     fresh input still happens to be a prefix of. */
  if (le->line->length == 0) {
    itl_g_ghost_sticky_target[0] = '\0';
  }
}

#define ITL_LE_ERASE_FORWARD(le, count)  itl_le_erase(le, count, false)
#define ITL_LE_ERASE_BACKWARD(le, count) itl_le_erase(le, count, true)

/* Inserts character at cursor position */
ITL_DEF bool itl_le_insert(itl_le_t *le, itl_utf8_t ch)
{
  ITL_TRY(le->line->size + ch.size < le->out_size, return false);

  if (!itl_g_undo_insert_run_open) {
    itl_undo_push(le);
    itl_g_undo_insert_run_open = true;
  }

  itl_string_insert(le->line, le->cursor_position, ch);
  itl_le_move_right(le, 1);

  if (itl_g_vi_is_recording_insert &&
      itl_g_vi_last_change.inserted_text != NULL &&
      itl_g_vi_last_change.inserted_text->length < ITL_STRING_MAX_LEN)
  {
    itl_string_insert(itl_g_vi_last_change.inserted_text,
                      itl_g_vi_last_change.inserted_text->length, ch);
  }

  return true;
}

#define ITL_CHAR_IS_DELIM(c) (ispunct(c))
#define ITL_CHAR_IS_SPACE(c) (isspace(c))

typedef enum
{
  ITL_TOKEN_DELIM = 0,
  ITL_TOKEN_WORD = 1,
  ITL_TOKEN_SPACE = 2,
} itl_token_kind;

/* Returns amount of steps required to reach next/previos token */
ITL_DEF size_t itl_string_steps_to_token(const itl_string_t *str,
                                         size_t position, bool backwards)
{
  uint8_t b;
  bool should_break = false;
  size_t i = position, steps = 0;

  itl_token_kind token_kind;

  if (str->length == 0) {
    return 0;
  }
  if (!backwards && i >= str->length) {
    return 0;
  }

  if (backwards && i > 0) {
    steps += 1;
    i -= 1;
  }

  b = str->chars[i].bytes[0];

  if (ITL_CHAR_IS_SPACE(b)) {
    token_kind = ITL_TOKEN_SPACE;
  } else if (ITL_CHAR_IS_DELIM(b)) {
    token_kind = ITL_TOKEN_DELIM;
  } else {
    token_kind = ITL_TOKEN_WORD;
  }

  while (i < str->length) {
    b = str->chars[i].bytes[0];

    switch (token_kind) {
    case ITL_TOKEN_DELIM: should_break = !ITL_CHAR_IS_DELIM(b); break;
    case ITL_TOKEN_WORD:
      should_break = ITL_CHAR_IS_DELIM(b) || ITL_CHAR_IS_SPACE(b);
      break;
    case ITL_TOKEN_SPACE: should_break = !ITL_CHAR_IS_SPACE(b); break;
    default: ITL_UNREACHABLE();
    }
    if (should_break) {
      break;
    }

    steps += 1;

    if (backwards && i > 0) {
      i -= 1;
    } else if (!backwards && i < str->length - 1) {
      i += 1;
    } else {
      break;
    }
  }

  ITL_TRACELN("mode: %d, steps: %zu", token_kind, steps);

  return steps;
}

#define ITL_LE_STEPS_TO_TOKEN(le, backwards)                                   \
  itl_string_steps_to_token((le)->line, (le)->cursor_position, backwards)

#define ITL_LE_STEPS_TO_TOKEN_FORWARD(le)                                      \
  itl_string_steps_to_token((le)->line, (le)->cursor_position, false)

#define ITL_LE_STEPS_TO_TOKEN_BACKWARD(le)                                     \
  itl_string_steps_to_token((le)->line, (le)->cursor_position, true)

#define ITL_LE_CURSOR_IS_ON_SPACE(le)                                          \
  ITL_CHAR_IS_SPACE((le)->line->chars[(le)->cursor_position].bytes[0])

ITL_DEF void itl_le_clear_line(itl_le_t *le)
{
  itl_string_clear(le->line);
  le->cursor_position = 0;
  /* A cleared line has no suggestion, so the sticky ghost target resets and the
     next input picks a fresh one rather than re-deriving the old target. */
  itl_g_ghost_sticky_target[0] = '\0';
}

/* Saves the edited draft line the first time navigation leaves it, so stepping
   back past the newest entry can restore what the user was typing. */
ITL_DEF void itl_history_save_draft(itl_le_t *le)
{
  if (le->history_selected_index != ITL_HISTORY_NONE) {
    return;
  }
  if (itl_g_history_draft == NULL) {
    itl_g_history_draft = itl_string_alloc();
  }
  itl_string_copy(itl_g_history_draft, le->line);
}

/* Restores the saved draft line and returns the editor to the draft state. */
ITL_DEF void itl_history_restore_draft(itl_le_t *le)
{
  itl_le_clear_line(le);
  if (itl_g_history_draft != NULL) {
    itl_string_copy(le->line, itl_g_history_draft);
  }
  le->cursor_position = le->line->length;
  le->history_selected_index = ITL_HISTORY_NONE;
}

/* Replaces the editor line with the currently selected history entry, read from
   the file on demand. */
ITL_DEF void itl_history_show_selected(itl_le_t *le)
{
  size_t offset = itl_history_index_to_offset(le->history_selected_index);

  itl_le_clear_line(le);
  if (itl_history_read_entry(offset, le->line)) {
    le->cursor_position = le->line->length;
  } else {
    /* The entry could not be read, so fall back to the draft instead of leaving
       a blank line selected with the index still advanced. */
    itl_history_restore_draft(le);
  }
}

ITL_DEF void itl_g_history_get_prev(itl_le_t *le)
{
  if (itl_g_history_count == 0) {
    return;
  }

  if (le->history_selected_index == ITL_HISTORY_NONE) {
    /* Leaving the draft for the first time jumps to the newest entry. */
    itl_history_save_draft(le);
    le->history_selected_index = itl_g_history_count - 1;
  } else if (le->history_selected_index > 0) {
    le->history_selected_index -= 1;
  } else {
    return; /* Already at the oldest navigable entry. */
  }

  itl_history_show_selected(le);
}

ITL_DEF void itl_g_history_get_next(itl_le_t *le)
{
  if (le->history_selected_index == ITL_HISTORY_NONE) {
    return;
  }

  if (le->history_selected_index + 1 < itl_g_history_count) {
    le->history_selected_index += 1;
    itl_history_show_selected(le);
  } else {
    /* Stepping past the newest entry restores the draft line. */
    itl_history_restore_draft(le);
  }
}

#define ITL_CHAR_BUFFER_INIT_SIZE 256

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
  cb->data = (char *) itl_malloc(sizeof(char) * cb->capacity);
}

ITL_DEF itl_char_buf_t *itl_char_buf_alloc(void)
{
  itl_char_buf_t *cb = (itl_char_buf_t *) itl_malloc(sizeof(itl_char_buf_t));
  itl_char_buf_init(cb);
  return cb;
}

#define ITL_CHAR_BUF_FREE(cb)                                                  \
  do {                                                                         \
    ITL_FREE(cb->data);                                                        \
    ITL_FREE(cb);                                                              \
  } while (0)

#define ITL_CHAR_BUF_REALLOC_CAPACITY(old_capacity) (old_capacity * 2)

ITL_DEF void itl_char_buf_extend(itl_char_buf_t *cb)
{
  cb->capacity = ITL_CHAR_BUF_REALLOC_CAPACITY(cb->capacity);
  cb->data = (char *) itl_realloc(cb->data, cb->capacity);
}

/* Grow the buffer to hold at least needed bytes in one reallocation. A bulk
   load reserves up front rather than doubling through repeated appends. */
ITL_DEF void itl_char_buf_reserve(itl_char_buf_t *cb, size_t needed)
{
  if (cb->capacity >= needed) {
    return;
  }
  while (cb->capacity < needed)
    cb->capacity = ITL_CHAR_BUF_REALLOC_CAPACITY(cb->capacity);
  cb->data = (char *) itl_realloc(cb->data, cb->capacity);
}

ITL_DEF void itl_history_read_fd_invalidate(void)
{
  if (itl_g_history_read_buffer != NULL) {
    ITL_CHAR_BUF_FREE(itl_g_history_read_buffer);
    itl_g_history_read_buffer = NULL;
  }
  itl_g_history_read_buffer_loaded = false;
  itl_g_history_read_buffer_offset = 0;
  itl_g_history_read_buffer_start = 0;
  itl_g_ghost_history_miss_prefix[0] = '\0';
  itl_g_ghost_history_miss_prefix_length = 0;
}

/* Loads the whole history file into itl_g_history_read_buffer once. The ghost
   and search scans then decode each entry from memory rather than reading the
   file per entry. A failed load is recorded and not retried until the next
   invalidation. Returns true when the buffer holds the file. */
ITL_DEF bool itl_history_ensure_read_buffer(void)
{
  ITL_FILE file;
  long file_size;
  size_t retained_offset;
  size_t retained_size;
  size_t total_read = 0;

  if (itl_g_history_read_buffer_loaded) {
    return itl_g_history_read_buffer != NULL;
  }
  itl_g_history_read_buffer_loaded = true;

  if (itl_g_history_path == NULL) {
    return false;
  }

  file = ITL_FILE_OPEN_FOR_READ(itl_g_history_path);
  if (ITL_FILE_IS_BAD(file)) {
    return false;
  }

  file_size = ITL_FILE_SEEK_END(file);
  if (file_size < 0) {
    ITL_FILE_CLOSE(file);
    return false;
  }
  retained_offset = itl_g_history_count > 0
                        ? itl_history_index_to_offset(0)
                        : (size_t) file_size;
  if (retained_offset > (size_t) file_size ||
      !ITL_FILE_SEEK(file, retained_offset))
  {
    ITL_FILE_CLOSE(file);
    return false;
  }
  retained_size = (size_t) file_size - retained_offset;

#if !defined NDEBUG
  itl_g_debug_history_buffer_load_count += 1;
#endif
  itl_g_history_read_buffer = itl_char_buf_alloc();
  itl_char_buf_reserve(itl_g_history_read_buffer, retained_size + 1);

  while (total_read < retained_size) {
    int read_amount =
        (int) ITL_READ(file, itl_g_history_read_buffer->data + total_read,
                       retained_size - total_read);
    if (read_amount <= 0) {
      break;
    }
    total_read += (size_t) read_amount;
  }
  ITL_FILE_CLOSE(file);

  itl_g_history_read_buffer->size = total_read;
  itl_g_history_read_buffer_offset = retained_offset;
  itl_g_history_read_buffer_start = 0;
  return true;
}

ITL_DEF bool itl_history_decode_entry_buffered(size_t offset, char *decoded,
                                               size_t capacity,
                                               size_t *decoded_size_out)
{
  size_t decoded_size = 0;
  bool escape_pending = false;
  size_t i;
  size_t buffer_size = itl_g_history_read_buffer->size;
  const char *buffer_data = itl_g_history_read_buffer->data;

  if (capacity == 0) return false;
  if (offset < itl_g_history_read_buffer_offset) return false;
  offset -= itl_g_history_read_buffer_offset;
  if (offset < itl_g_history_read_buffer_start || offset > buffer_size)
    return false;

  for (i = offset; i < buffer_size; ++i) {
    uint8_t ch = (uint8_t) buffer_data[i];

    if (escape_pending) {
      escape_pending = false;
      if (ch == 'n') {
        ch = 0x0A;
      } else if (ch != '\\') {
        if (decoded_size + 1 >= capacity) return false;
        decoded[decoded_size++] = '\\';
      }
    } else if (ch == '\\') {
      escape_pending = true;
      continue;
    } else if (ch == '\n') {
      break;
    } else if (ch == '\r') {
      continue;
    }

    if (decoded_size + 1 >= capacity) return false;
    decoded[decoded_size++] = (char) ch;
  }

  decoded[decoded_size] = '\0';
  *decoded_size_out = decoded_size;
  return true;
}

ITL_DEF bool itl_history_read_entry_buffered(size_t offset, itl_string_t *out)
{
  char decoded[ITL_STRING_MAX_LEN + 1];
  size_t decoded_size;

  if (!itl_history_decode_entry_buffered(offset, decoded, sizeof(decoded),
                                         &decoded_size))
  {
    return false;
  }

  return itl_string_from_bytes(out, decoded, decoded_size);
}

ITL_DEF void itl_char_buf_append_cstr(itl_char_buf_t *cb, const char *cstr)
{
  /* The length is measured once and the buffer grown once, so a multi-byte
     escape sequence copies in one memcpy rather than a per-byte capacity check.
   */
  size_t len = strlen(cstr);

  while (cb->capacity < cb->size + len) {
    itl_char_buf_extend(cb);
  }

  memcpy(cb->data + cb->size, cstr, len);
  cb->size += len;
}

ITL_DEF void itl_char_buf_append_size_t(itl_char_buf_t *cb, size_t n)
{
  size_t new_size, i;
  size_t data_len = 0, data_copy = n;

  do {
    data_len += 1;
    data_copy /= 10;
  } while (data_copy > 0);

  new_size = cb->size + data_len;

  while (cb->capacity < new_size) {
    itl_char_buf_extend(cb);
  }

  /* Digits are put in reverse order */
  for (i = new_size; i > cb->size; --i) {
    cb->data[i - 1] = (char) (n % 10) + '0';
    n /= 10;
  }

  cb->size = new_size;
}

ITL_DEF tl_status_code itl_char_buf_append_string(itl_char_buf_t *cb,
                                                  const itl_string_t *str)
{
  char *data;

  while (cb->capacity < cb->size + str->size + 1) {
    itl_char_buf_extend(cb);
  }

  data = cb->data + (cb->size * sizeof(char));
  ITL_TRY(itl_string_to_cstr(str, data, str->size + 1) == TL_SUCCESS,
          return TL_ERROR_SIZE);
  cb->size += str->size; /* Ignore null at the end */

  return TL_SUCCESS;
}

ITL_DEF void itl_char_buf_append_byte(itl_char_buf_t *cb, uint8_t data)
{
  while (cb->capacity < cb->size + 1) {
    itl_char_buf_extend(cb);
  }

  cb->data[cb->size] = (char) data;
  cb->size += 1;
}

ITL_DEF void itl_history_append_read_buffer(size_t previous_file_size,
                                            const char *data,
                                            size_t data_size)
{
  size_t retained_offset;
  size_t discarded_size;

  if (!itl_g_history_read_buffer_loaded) return;
  if (itl_g_history_read_buffer == NULL) {
    itl_g_history_read_buffer = itl_char_buf_alloc();
    itl_g_history_read_buffer_offset = previous_file_size;
    itl_g_history_read_buffer_start = 0;
  }
  if (itl_g_history_read_buffer_offset +
          itl_g_history_read_buffer->size !=
      previous_file_size)
  {
    itl_history_read_fd_invalidate();
    return;
  }

  itl_char_buf_reserve(itl_g_history_read_buffer,
                       itl_g_history_read_buffer->size + data_size);
  memcpy(itl_g_history_read_buffer->data + itl_g_history_read_buffer->size,
         data, data_size);
  itl_g_history_read_buffer->size += data_size;

  if (itl_g_history_count == 0) return;
  retained_offset = itl_g_history_offsets[itl_g_history_head];
  if (retained_offset < itl_g_history_read_buffer_offset ||
      retained_offset > itl_g_history_read_buffer_offset +
                            itl_g_history_read_buffer->size)
  {
    itl_history_read_fd_invalidate();
    return;
  }

  itl_g_history_read_buffer_start =
      retained_offset - itl_g_history_read_buffer_offset;
  if (itl_g_history_read_buffer_start <=
      itl_g_history_read_buffer->size / 2)
  {
    return;
  }

  discarded_size = itl_g_history_read_buffer_start;
  memmove(itl_g_history_read_buffer->data,
          itl_g_history_read_buffer->data + discarded_size,
          itl_g_history_read_buffer->size - discarded_size);
  itl_g_history_read_buffer->size -= discarded_size;
  itl_g_history_read_buffer_offset += discarded_size;
  itl_g_history_read_buffer_start = 0;
}

ITL_DEF void itl_char_buf_append_spaces(itl_char_buf_t *cb, size_t count)
{
  /* The padding for a wrapped continuation row grows the buffer once and fills
     with one memset rather than a per-space capacity check. */
  while (cb->capacity < cb->size + count) {
    itl_char_buf_extend(cb);
  }

  memset(cb->data + cb->size, ' ', count);
  cb->size += count;
}

/* Appends a string with each newline written as backslash n and each backslash
   doubled, so a multiline entry survives the newline-delimited history file. */
ITL_DEF void itl_char_buf_append_string_escaped(itl_char_buf_t *cb,
                                                const itl_string_t *str)
{
  size_t i, j;

  for (i = 0; i < str->length; ++i) {
    itl_utf8_t ch = str->chars[i];
    if (ITL_LE_IS_NEWLINE(ch)) {
      itl_char_buf_append_byte(cb, '\\');
      itl_char_buf_append_byte(cb, 'n');
    } else if (ITL_LE_IS_BACKSLASH(ch)) {
      itl_char_buf_append_byte(cb, '\\');
      itl_char_buf_append_byte(cb, '\\');
    } else {
      for (j = 0; j < ch.size; ++j) {
        itl_char_buf_append_byte(cb, ch.bytes[j]);
      }
    }
  }
}

#define ITL_CHAR_BUF_CLEAR(cb) (cb)->size = 0

#define ITL_CHAR_BUF_DUMP(cb)                                                  \
  (void) ITL_WRITE(ITL_STDOUT, (cb)->data, (cb)->size)

#define ITL_TTY_HIDE_CURSOR(buffer)                                            \
  itl_char_buf_append_cstr(buffer, "\x1b[?25l")

#define ITL_TTY_SHOW_CURSOR(buffer)                                            \
  itl_char_buf_append_cstr(buffer, "\x1b[?25h")

#define ITL_TTY_MOVE_TO_COLUMN(buffer, col)                                    \
  do {                                                                         \
    itl_char_buf_append_cstr(buffer, "\x1b[");                                 \
    itl_char_buf_append_size_t(buffer, (size_t) col);                          \
    itl_char_buf_append_byte(buffer, 'G');                                     \
  } while (0)

#define ITL_TTY_MOVE_FORWARD(buffer, steps)                                    \
  do {                                                                         \
    itl_char_buf_append_cstr(buffer, "\x1b[");                                 \
    itl_char_buf_append_size_t(buffer, (size_t) steps);                        \
    itl_char_buf_append_byte(buffer, 'C');                                     \
  } while (0)

#define ITL_TTY_MOVE_UP(buffer, rows)                                          \
  do {                                                                         \
    itl_char_buf_append_cstr(buffer, "\x1b[");                                 \
    itl_char_buf_append_size_t(buffer, (size_t) rows);                         \
    itl_char_buf_append_byte(buffer, 'A');                                     \
  } while (0)

#define ITL_TTY_MOVE_DOWN(buffer, rows)                                        \
  do {                                                                         \
    itl_char_buf_append_cstr(buffer, "\x1b[");                                 \
    itl_char_buf_append_size_t(buffer, (size_t) rows);                         \
    itl_char_buf_append_byte(buffer, 'B');                                     \
  } while (0)

#define ITL_TTY_CLEAR_WHOLE_LINE(buffer)                                       \
  itl_char_buf_append_cstr(buffer, "\r\x1b[0K")

#define ITL_TTY_CLEAR_TO_END(buffer) itl_char_buf_append_cstr(buffer, "\x1b[K")

/* Erases from the cursor to the end of the display, used on resize where the
   reflowed row counts can no longer be trusted. */
#define ITL_TTY_CLEAR_BELOW(buffer) itl_char_buf_append_cstr(buffer, "\x1b[0J")

#define ITL_TTY_GOTO_HOME(buffer) itl_char_buf_append_cstr(buffer, "\x1b[H")

#define ITL_TTY_ERASE_SCREEN(buffer) itl_char_buf_append_cstr(buffer, "\033[2J")

#define ITL_TTY_STATUS_REPORT(buffer)                                          \
  itl_char_buf_append_cstr(buffer, "\x1b[6n")

/* Toggling autowrap lets the renderer place its own line breaks without the
   terminal also wrapping at the right edge, which would double the break. */
#define ITL_TTY_AUTOWRAP_OFF(buffer)                                           \
  itl_char_buf_append_cstr(buffer, "\x1b[?7l")
#define ITL_TTY_AUTOWRAP_ON(buffer) itl_char_buf_append_cstr(buffer, "\x1b[?7h")

/* If this is true, do not overwrite file on `history_dump_to_file()` */
ITL_DEF ITL_THREAD_LOCAL bool itl_g_history_file_is_bad = false;

/* Records the byte offset of one entry in the ring, evicting the oldest when
   the configured limit is full. */
ITL_DEF void itl_history_push_offset(size_t offset)
{
  if (itl_g_history_limit == 0) return;

  if (itl_g_history_count < itl_g_history_limit) {
    itl_g_history_offsets[(itl_g_history_head + itl_g_history_count) %
                          (TL_HISTORY_MAX_SIZE)] = offset;
    itl_g_history_count += 1;
  } else {
    itl_g_history_offsets[(itl_g_history_head + itl_g_history_count) %
                          (TL_HISTORY_MAX_SIZE)] = offset;
    itl_g_history_head = (itl_g_history_head + 1) % (TL_HISTORY_MAX_SIZE);
  }
}

/* Scans the open file from the start, rebuilding the offset ring, the recorded
   file size, and the trailing-newline flag. Returns false on a read error or a
   non-text byte. Touches neither the path nor the draft, so it is safe to call
   mid-session to pick up entries that other sessions appended. */
ITL_DEF bool itl_history_scan_fd(ITL_FILE file)
{
  char file_buffer[ITL_HISTORY_FILE_BUFFER_SIZE];
  bool escape_pending = false;
  size_t entry_start = 0;
  size_t file_pos = 0;
  uint8_t last_byte = (uint8_t) '\n';

  itl_g_history_head = 0;
  itl_g_history_count = 0;
  itl_g_history_total_count = 0;
  itl_history_read_fd_invalidate();

  if (!ITL_FILE_SEEK(file, 0)) {
    return false;
  }

  for (;;) {
    int read_amount =
        (int) ITL_READ(file, file_buffer, ITL_HISTORY_FILE_BUFFER_SIZE);
    size_t i;

    if (read_amount < 0) {
      return false; /* Read error. */
    }
    if (read_amount == 0) {
      break; /* End of file. */
    }

    /* Walk the bytes counting entries, where one entry is one physical line.
       The escape state carries across chunk boundaries so only an unescaped
       newline ends an entry. */
    for (i = 0; i < (size_t) read_amount; ++i, ++file_pos) {
      uint8_t ch = (uint8_t) file_buffer[i];

      last_byte = ch;

      if (escape_pending) {
        escape_pending = false;
        continue;
      }

      if (ch == '\\') {
        escape_pending = true;
        continue;
      }

      if (ch == '\n') {
        itl_history_push_offset(entry_start);
        itl_g_history_total_count += 1;
        entry_start = file_pos + 1;
        continue;
      }

      if (ch == '\r') {
        continue;
      }

      /* Loaded a binary file on accident? */
      if (iscntrl(ch) && !isspace(ch)) {
        ITL_TRACELN("non-text byte '%X' detected in history file at offset "
                    "%zu\n",
                    (uint8_t) ch, file_pos);
        errno = EINVAL;
        return false;
      }
    }
  }

  /* A trailing line without a final newline is ignored, since every written
     entry ends with a newline. Remember when the file did not end on a newline
     so the next append separates itself from that line. */
  itl_g_history_file_size = file_pos;
  itl_g_history_ends_with_newline =
      (file_pos == 0) || (last_byte == (uint8_t) '\n');

  return true;
}

/* Returns TL_SUCCESS or TL_ERROR, sets errno on failure. The loader scans the
   file once and keeps only the byte offset of each entry, capped to the most
   recent TL_HISTORY_MAX_SIZE, so it never holds entry text and a large history
   file stays cheap to load. */
ITL_DEF tl_status_code itl_history_load_from_file(const char *path)
{
  ITL_FILE file;
  size_t path_len;

  itl_g_history_free();
  itl_g_history_file_is_bad = false;

  /* Keep the path so entries can be read, appended, and searched on the file
     directly. */
  path_len = strlen(path);
  itl_g_history_path = (char *) itl_malloc(path_len + 1);
  memcpy(itl_g_history_path, path, path_len + 1);

  file = ITL_FILE_OPEN_FOR_READ(path);
  if (ITL_FILE_IS_BAD(file)) {
    ITL_TRACELN("could not open history file for load (%s): %s\n", path,
                strerror(errno));
    /* A missing file is not bad, the first append creates it. */
    if (errno != ENOENT) {
      itl_g_history_file_is_bad = true;
    }
    return TL_ERROR;
  }

  if (!itl_history_scan_fd(file)) {
    ITL_FILE_CLOSE(file);
    itl_g_history_free();
    itl_g_history_file_is_bad = true;
    return TL_ERROR;
  }

  ITL_FILE_CLOSE(file);
  ITL_TRACELN("loaded %zu history entries, file size %zu\n",
              itl_g_history_count, itl_g_history_file_size);

  return TL_SUCCESS;
}

/* Appends an accepted command to the history file and records its offset.
   Entries of length one or zero are skipped. Consecutive duplicates are
   skipped unless the caller requests them. Returns true on a successful write.
   Returns false when the entry is skipped or the write fails. */
ITL_DEF bool itl_history_append_to_file(const itl_string_t *str,
                                        bool should_require_terminal,
                                        bool should_allow_duplicate)
{
  ITL_FILE read_file;
  ITL_FILE append_file;
  itl_char_buf_t *buffer;
  long real_end;
  long actual_end;
  size_t new_offset;
  bool ok = true;
  bool is_duplicate = false;
  bool should_rescan = false;
  bool had_unterminated_tail;

  itl_g_last_history_event_number = 0;
  if (itl_g_history_path == NULL || itl_g_history_file_is_bad) {
    return false;
  }
  /* A non-interactive run reading from a pipe or a file leaves the history
     file untouched, so only a real terminal session records its commands the
     way bash skips history off a tty. */
  if (should_require_terminal && !ITL_TTY_IS_TTY()) {
    return false;
  }
  if (str->length <= 1) {
    return false;
  }
  /* An oversized line is dropped from history, so a pasted blob does not bloat
     the file or slow the ghost scan that reads each entry. The line itself
     still runs, only its recall is skipped. */
  if (str->size > ITL_HISTORY_ENTRY_MAX_BYTES) {
    ITL_TRACELN("skipping oversized history entry, %zu bytes\n", str->size);
    return false;
  }

  /* Reload the file when another session has grown it since we last looked, the
     way fish merges its history on save. This lets navigation, search, and the
     duplicate check below see commands other sessions wrote. */
  read_file = ITL_FILE_OPEN_FOR_READ(itl_g_history_path);
  if (!ITL_FILE_IS_BAD(read_file)) {
    long disk_size = ITL_FILE_SEEK_END(read_file);
    if (disk_size >= 0 && (size_t) disk_size != itl_g_history_file_size) {
      if (!itl_history_scan_fd(read_file)) {
        itl_g_history_file_is_bad = true;
        ITL_FILE_CLOSE(read_file);
        return false;
      }
    }
    /* Skip a command identical to the most recent entry. */
    if (itl_g_history_count > 0) {
      itl_string_t *newest = itl_string_alloc();
      if (itl_history_read_entry_fd(
              read_file, itl_history_index_to_offset(itl_g_history_count - 1),
              newest))
      {
        is_duplicate = itl_string_equal(newest, str);
      }
      ITL_STRING_FREE(newest);
    }
    ITL_FILE_CLOSE(read_file);
  }
  if (is_duplicate && !should_allow_duplicate) {
    return false;
  }

  buffer = itl_char_buf_alloc();
  /* When the file does not end on a newline, write a separator first so the new
     entry starts its own physical line instead of gluing onto the previous one.
   */
  had_unterminated_tail = !itl_g_history_ends_with_newline;
  if (had_unterminated_tail) {
    itl_char_buf_append_byte(buffer, '\n');
  }
  itl_char_buf_append_string_escaped(buffer, str);
  itl_char_buf_append_byte(buffer, '\n');

  append_file = ITL_FILE_OPEN_FOR_APPEND(itl_g_history_path);
  if (ITL_FILE_IS_BAD(append_file)) {
    ITL_TRACELN("could not open history file for append (%s): %s\n",
                itl_g_history_path, strerror(errno));
    itl_g_history_file_is_bad = true;
    ITL_CHAR_BUF_FREE(buffer);
    return false;
  }

  /* Take the offset from the real end of the file rather than a tracked size,
     so a concurrent append from another session does not misplace this entry.
     The entry itself starts after the optional separator byte. */
  real_end = ITL_FILE_SEEK_END(append_file);
  if (real_end < 0) {
    real_end = (long) itl_g_history_file_size;
  }
  new_offset = (size_t) real_end + (itl_g_history_ends_with_newline ? 0 : 1);

  {
    int written = (int) ITL_WRITE(append_file, buffer->data, buffer->size);
    if (written == -1 || (size_t) written != buffer->size) {
      /* A hard error or a short write would desync the recorded offsets from
         the real file, so mark the file bad and record nothing. */
      ITL_TRACELN("could not append to history file (%s): %s\n",
                  itl_g_history_path, strerror(errno));
      itl_g_history_file_is_bad = true;
      ok = false;
    } else {
      actual_end = ITL_FILE_TELL(append_file);
      if (actual_end < 0) actual_end = real_end + (long) buffer->size;
      new_offset = (size_t) actual_end - buffer->size +
                   (had_unterminated_tail ? 1 : 0);
      should_rescan = had_unterminated_tail ||
                      (size_t) actual_end !=
                          (size_t) real_end + buffer->size;
      itl_g_history_file_size = (size_t) actual_end;
      itl_g_history_ends_with_newline = true;
      if (!should_rescan) {
        itl_history_push_offset(new_offset);
        itl_g_history_total_count += 1;
        itl_g_last_history_event_number = itl_g_history_total_count;
      }
      itl_history_append_read_buffer((size_t) real_end, buffer->data,
                                     buffer->size);
      ITL_TRACELN("appended history entry at offset %zu, %zu entries now\n",
                  new_offset, itl_g_history_count);
    }
  }

  ITL_FILE_CLOSE(append_file);
  ITL_CHAR_BUF_FREE(buffer);

  if (ok && should_rescan) {
    size_t index;
    read_file = ITL_FILE_OPEN_FOR_READ(itl_g_history_path);
    if (ITL_FILE_IS_BAD(read_file) || !itl_history_scan_fd(read_file)) {
      if (!ITL_FILE_IS_BAD(read_file)) ITL_FILE_CLOSE(read_file);
      return false;
    }
    ITL_FILE_CLOSE(read_file);

    itl_g_last_history_event_number = 0;
    for (index = 0; index < itl_g_history_count; ++index) {
      if (itl_history_index_to_offset(index) == new_offset) {
        itl_g_last_history_event_number =
            itl_g_history_total_count - itl_g_history_count + index + 1;
        break;
      }
    }
    if (itl_g_last_history_event_number == 0) return false;
  }

  return ok;
}

/* History persists on append, so dump only flushes. When the target path is the
   active store there is nothing to do. A different path receives a byte copy of
   the store so the public contract still writes the history somewhere. Returns
   TL_SUCCESS or TL_ERROR, sets errno on failure. */
ITL_DEF tl_status_code itl_history_dump_to_file(const char *path)
{
  char file_buffer[ITL_HISTORY_FILE_BUFFER_SIZE];
  ITL_FILE in_file;
  ITL_FILE out_file;
  tl_status_code ret = TL_SUCCESS;

  TL_ASSERT(itl_g_is_active && "Dump history before calling tl_exit()!");

  if (itl_g_history_file_is_bad) {
    errno = EINVAL;
    return TL_ERROR;
  }

  /* Nothing was loaded or appended, so there is nothing to flush. */
  if (itl_g_history_path == NULL) {
    return TL_SUCCESS;
  }
  /* The active store already holds every entry. */
  if (strcmp(itl_g_history_path, path) == 0) {
    return TL_SUCCESS;
  }

  out_file = ITL_FILE_OPEN_FOR_WRITE(path);
  if (ITL_FILE_IS_BAD(out_file)) {
    ITL_TRACELN("could not open history file for dump (%s): %s\n", path,
                strerror(errno));
    return TL_ERROR;
  }

  in_file = ITL_FILE_OPEN_FOR_READ(itl_g_history_path);
  if (ITL_FILE_IS_BAD(in_file)) {
    int const open_error = errno;
    ITL_FILE_CLOSE(out_file);
    return (open_error == ENOENT) ? TL_SUCCESS : TL_ERROR;
  }

  for (;;) {
    int read_amount =
        (int) ITL_READ(in_file, file_buffer, ITL_HISTORY_FILE_BUFFER_SIZE);
    size_t total_written = 0;
    if (read_amount < 0) {
      ret = TL_ERROR;
      break;
    }
    if (read_amount == 0) break;

    while (total_written < (size_t) read_amount) {
      int write_amount =
          (int) ITL_WRITE(out_file, file_buffer + total_written,
                          (size_t) read_amount - total_written);
      if (write_amount <= 0) {
        ret = TL_ERROR;
        break;
      }
      total_written += (size_t) write_amount;
    }
    if (ret != TL_SUCCESS) break;
  }

  ITL_FILE_CLOSE(in_file);
  ITL_FILE_CLOSE(out_file);

  return ret;
}

ITL_DEF size_t itl_parse_size(const char *cstr, size_t *result)
{
  size_t i, number;

  for (i = 0, number = 0; cstr[i] != '\0'; ++i) {
    if (!isdigit((unsigned char) cstr[i])) {
      break;
    }
    number *= 10;
    number += (size_t) (cstr[i] - '0');
  }

  ITL_PTR_ASSIGN(result, number);

  return i;
}

#if defined ITL_POSIX || defined ITL_WIN32
ITL_DEF int itl_esc_parse_vt(uint8_t byte)
{
  int event = 0;
  bool read_mod = false;

  if (byte == 27) { /* esc */
    /* A lone ESC has no byte after it, so reading one here would block until
       the next keystroke and a search or a command mode would never see the
       cancel. An escape sequence arrives as one burst, so its next byte is
       already pending. A bare ESC is reported at once when nothing follows. */
    if (!itl_input_is_pending()) return TL_KEY_UNKN;

    ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);

    if (byte != '[' && byte != 'O') {
      switch (byte) {
      case 'b': return TL_KEY_LEFT | TL_MOD_CTRL;
      case 'f': return TL_KEY_RIGHT | TL_MOD_CTRL;

      case 'd': return TL_KEY_DELETE | TL_MOD_CTRL;
      case 'h': return TL_KEY_BACKSPACE | TL_MOD_CTRL;
      case 8:
      case 127: return TL_KEY_BACKSPACE | TL_MOD_CTRL;

      case '.':
      case '>': return TL_KEY_HISTORY_END;
      case ',':
      case '<': return TL_KEY_HISTORY_BEGINNING;

      case 13:
      case 10: return TL_KEY_ENTER | TL_MOD_ALT;

      default: return TL_KEY_CHAR | TL_MOD_ALT;
      }
    }

    ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);

    /* Bracketed paste opens with ESC [ 200 ~ and closes with ESC [ 201 ~. Any
       other ESC [ 2 ... ~ sequence, like Insert or its modified forms, is
       drained to the terminator so its trailing bytes do not leak as input. */
    if (byte == '2') {
      int paste_kind = 0;
      ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);
      if (byte == '0') {
        ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);
        if (byte == '0' || byte == '1') {
          paste_kind = (byte == '0') ? 1 : 2;
          ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);
        }
      }
      /* Consume the remaining CSI parameter and intermediate bytes (0x20-0x3F)
         up to the final byte, so a non-paste sequence does not leak bytes and a
         malformed one cannot drain past its terminator. */
      while (byte >= 0x20 && byte < 0x40) {
        ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);
      }
      return (paste_kind == 1) ? TL_KEY_PASTE_BEGIN : TL_KEY_UNKN;
    }

    if (byte == '1') {
      ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);
      if (byte == '~') {
        return event | TL_KEY_HOME;
      }
      if (byte != ';') {
        return TL_KEY_UNKN;
      }

      ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);
      switch (byte) {
      case '2': event |= TL_MOD_SHIFT; break;
      case '3': event |= TL_MOD_ALT; break;
      case '5': event |= TL_MOD_CTRL; break;
      }
      read_mod = true;

      ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);
    }

    switch (byte) {
    case 'A': return event | TL_KEY_UP;
    case 'B': return event | TL_KEY_DOWN;
    case 'C': return event | TL_KEY_RIGHT;
    case 'D': return event | TL_KEY_LEFT;

    case 'F': return event | TL_KEY_END;
    case 'H': return event | TL_KEY_HOME;

    case '3': event |= TL_KEY_DELETE; break;
    case '4': event |= TL_KEY_END; break;
    case '7': event |= TL_KEY_HOME; break;
    case '8': event |= TL_KEY_END; break;

    default: event |= TL_KEY_UNKN;
    }
  } else {
    ITL_TRY(!iscntrl(byte), return TL_KEY_UNKN);
    return TL_KEY_CHAR;
  }

  if (!read_mod) {
    if (byte >= 0x40) {
      return event;
    }

    ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);
    if (byte == ';') {
      ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);
      switch (byte) {
      case '2': event |= TL_MOD_SHIFT; break;
      case '3': event |= TL_MOD_ALT; break;
      case '5': event |= TL_MOD_CTRL; break;
      }
      ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);
    }

    ITL_TRY(byte == '~', return TL_KEY_UNKN);
  }

  return event;
}
#endif /* ITL_POSIX || ITL_WIN32 */

#ifdef ITL_WIN32
ITL_DEF int itl_esc_parse_win32(uint8_t byte)
{
  int event = 0;

  /* ENABLE_VIRTUAL_TERMINAL_INPUT reports arrows and navigation keys as the
     same CSI sequences as a POSIX terminal. Keep accepting the legacy _getch
     scan-code pairs for consoles that do not provide VT input. */
  if (byte == 27) return itl_esc_parse_vt(byte);

  /* https://learn.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-6.0/aa299374(v=vs.60)
   */
  if (byte == 224 || byte == 0) { /* esc */
    ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);

    switch (byte) {
    case 'H': event = TL_KEY_UP; break;
    case 'P': event = TL_KEY_DOWN; break;
    case 'K': event = TL_KEY_LEFT; break;
    case 'M': event = TL_KEY_RIGHT; break;

    case 's': event = TL_KEY_LEFT | TL_MOD_CTRL; break;
    case 't': event = TL_KEY_RIGHT | TL_MOD_CTRL; break;

    case 'G': event = TL_KEY_HOME; break;
    case 'O': event = TL_KEY_END; break;

    case 147: event = TL_KEY_DELETE | TL_MOD_CTRL; break;
    case 'S': event = TL_KEY_DELETE; break;

    default: event = TL_KEY_UNKN;
    }
  } else {
    ITL_TRY(!iscntrl(byte), return TL_KEY_UNKN);
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

  case 3: return TL_KEY_INTERRUPT; /* ctrl c */
  case 4: return TL_KEY_EOF;       /* ctrl d */
  case 26: return TL_KEY_SUSPEND;  /* ctrl z */

  case 9: return TL_KEY_TAB;
  case 12: return TL_KEY_CLEAR; /* ctrl l */

  case 18: return TL_KEY_HISTORY_SEARCH; /* ctrl r */

  case 14: return TL_KEY_DOWN; /* ctrl n */
  case 16: return TL_KEY_UP;   /* ctrl p */

  case 13: /* cr */
  case 10: return TL_KEY_ENTER;

  case 11: return TL_KEY_KILL_LINE;        /* ctrl k */
  case 21: return TL_KEY_KILL_LINE_BEFORE; /* ctrl u */
  case 23: return TL_KEY_BACKSPACE | TL_MOD_CTRL;

  case 8: /* old backspace */
  case 127: return TL_KEY_BACKSPACE;

  case 31: return TL_KEY_UNDO;
  case 30: return TL_KEY_REDO;
  }

#if defined ITL_WIN32
  return itl_esc_parse_win32(byte);
#elif defined ITL_POSIX
  return itl_esc_parse_vt(byte);
#endif /* ITL_POSIX */
}

ITL_DEF ITL_THREAD_LOCAL itl_char_buf_t itl_g_char_buffer = ITL_ZERO_INIT;
ITL_DEF ITL_THREAD_LOCAL bool itl_g_tty_is_dumb = true;

ITL_DEF int itl_term_supports_decorations(void);

/* *le, *rows, *cols can be NULL. */
ITL_DEF bool itl_tty_get_size(ITL_MAYBE_UNUSED itl_le_t *le, size_t *rows,
                              size_t *cols)
{
  size_t temp_rows, temp_cols;
  char *emacs_buf = NULL;
#if defined ITL_VT_SIZE
  bool correct_response;
  size_t i, parse_diff;
  char size_buf[32], *first;
  itl_char_buf_t *b;
#elif defined ITL_WIN32
  CONSOLE_SCREEN_BUFFER_INFO buffer_info;
#else /* ITL_WIN32 */
  struct winsize window;
#endif

  if (itl_g_tty_is_dumb && !itl_term_supports_decorations()) {
    if ((emacs_buf = getenv("COLUMNS")) == NULL) {
      itl_g_tty_is_dumb = false;
      goto next;
    }
    itl_parse_size(emacs_buf, &temp_cols);
    if ((emacs_buf = getenv("LINES")) == NULL) {
      itl_g_tty_is_dumb = false;
      goto next;
    }
    itl_parse_size(emacs_buf, &temp_rows);
    if (temp_cols > 0 && temp_rows > 0) {
      ITL_PTR_ASSIGN(rows, temp_rows);
      ITL_PTR_ASSIGN(cols, temp_cols);
      return true;
    }
  }

next:
#if defined ITL_VT_SIZE

  b = &itl_g_char_buffer;
  ITL_TTY_MOVE_FORWARD(b, 999);
  ITL_TTY_STATUS_REPORT(b);
  ITL_CHAR_BUF_DUMP(b);
  ITL_CHAR_BUF_CLEAR(b);

  /* There might be pasted input awaiting to be processed. Read and parse all
     bytes until escape is encountered. */
  first = &size_buf[0];
  while (true) {
    ITL_TRY_READ_BYTE((uint8_t *) first, return false);
    if (*first == '\x1b') {
      break;
    }
    /* don't print control sequences if they got pasted */
    if (itl_esc_parse((uint8_t) *first) != TL_KEY_CHAR) {
      continue;
    }
    if (le != NULL) {
      itl_le_insert(le, itl_utf8_parse((uint8_t) *first));
    }
  }

  i = 1; /* already read the escape */
  correct_response = false;
  while (i < sizeof(size_buf) - 2) {
    ITL_TRY_READ_BYTE((uint8_t *) &size_buf[i], return false);
    if (size_buf[i] == 'R') {
      correct_response = true;
      break;
    }
    i += 1;
  }
  size_buf[i + 1] = '\0';

  ITL_TRY(correct_response, return false);
  ITL_TRY(size_buf[0] == '\x1b' && size_buf[1] == '[', return false);

  parse_diff = 2; /* skip first two characters */
  parse_diff += itl_parse_size(size_buf + parse_diff, rows);
  ITL_TRY(size_buf[parse_diff] == ';', return false);
  itl_parse_size(size_buf + parse_diff + 1, cols);

  return true;

#elif defined ITL_WIN32
  (void) le;
  ITL_TRY(
      GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &buffer_info),
      return false);

  ITL_PTR_ASSIGN(cols, (size_t) (buffer_info.srWindow.Right -
                                 buffer_info.srWindow.Left + 1));
  ITL_PTR_ASSIGN(rows, (size_t) (buffer_info.srWindow.Bottom -
                                 buffer_info.srWindow.Top + 1));

  return true;
#else
  (void) le;
  ITL_TRY(ioctl(STDOUT_FILENO, TIOCGWINSZ, &window) == 0, return false);

  ITL_PTR_ASSIGN(rows, (size_t) window.ws_row);
  ITL_PTR_ASSIGN(cols, (size_t) window.ws_col);

  return true;
#endif
  return false;
}

ITL_DEF ITL_THREAD_LOCAL bool itl_g_tty_should_refresh_text = true;

/* Set while the empty-completion flash repaints. The full redraw then wraps the
   line in the flash SGR and skips the highlight spans. */
ITL_DEF ITL_THREAD_LOCAL bool itl_g_tty_flash_active = false;

/* Line editor's visual extent during the previous refresh() call. */
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_le_prev_total_rows = 1;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_le_prev_cursor_row = 1;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_le_prev_cursor_col = 0;
/* The bytes the previous text refresh drew, the base a plain append writes its
   tail onto. The length is zero before the first text refresh. */
ITL_DEF ITL_THREAD_LOCAL char itl_g_le_prev_render[ITL_STRING_MAX_LEN] = {0};
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_le_prev_render_len = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_le_prev_length = 0;
/* Whether the previous refresh left the cursor at the line end. The append fast
   path fires only then, otherwise a mid-line caret forces the full redraw. */
ITL_DEF ITL_THREAD_LOCAL bool itl_g_le_prev_cursor_at_end = false;
ITL_DEF ITL_THREAD_LOCAL bool itl_g_tty_plain_append_pending = false;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_tty_plain_append_width = 0;
#if !defined NDEBUG
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_debug_append_refresh_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_debug_full_refresh_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_debug_metrics_scan_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_debug_line_serialization_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_debug_ghost_history_scan_count = 0;
#endif

/* The ghost suggestion drawn dimmed after the cursor, and its byte length.
   Right or End accepts it. */
ITL_DEF ITL_THREAD_LOCAL char itl_g_ghost[ITL_STRING_MAX_LEN] = {0};
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_ghost_len = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_ghost_width = 0;

ITL_DEF ITL_THREAD_LOCAL char itl_g_ghost_case_fix[ITL_STRING_MAX_LEN] = {0};
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_ghost_case_fix_len = 0;

ITL_DEF int itl_ascii_prefix_matches_casefold(const char *entry,
                                              const char *typed, size_t length)
{
  size_t i;
  for (i = 0; i < length; i++) {
    unsigned char a = (unsigned char) entry[i];
    unsigned char b = (unsigned char) typed[i];
    if (itl_ascii_fold_byte(a) != itl_ascii_fold_byte(b)) return 0;
  }
  return 1;
}

/* The history autosuggestion scans at most this many recent entries per
   keystroke to bound the per-keystroke cost on a long history. */
#define ITL_GHOST_HISTORY_SCAN_MAX 1000

/* The host highlight callback, or NULL when highlighting is disabled. Only the
   interactive host registers one. The refresh reads it, so it is declared
   before the refresh. */
ITL_DEF ITL_THREAD_LOCAL tl_highlight_fn itl_g_highlight_callback = NULL;
ITL_DEF ITL_THREAD_LOCAL tl_wake_fn itl_g_wake_callback = NULL;

/* The reset that closes every colored span, matching the ghost text's own
   reset. Each span carries its own opening SGR from the host. */
#define ITL_HIGHLIGHT_RESET "\x1b[0m"

/* The empty-completion flash, bright grey over a grey tint. */
#define ITL_FLASH_TINT_ON  "\x1b[38;5;250;48;5;238m"
#define ITL_FLASH_TINT_OFF "\x1b[39;49m"

/* The fallback flash for a terminal without the 256-color set. */
#define ITL_FLASH_REVERSE_ON  "\x1b[7m"
#define ITL_FLASH_REVERSE_OFF "\x1b[27m"

/* The terminal color and decoration capabilities, probed once from the
   environment and cached. A negative value marks the capability unprobed. */
ITL_DEF ITL_THREAD_LOCAL int itl_g_supports_256_color = -1;
ITL_DEF ITL_THREAD_LOCAL int itl_g_supports_decorations = -1;

/* Whether the terminal supports the 256-color set, read from COLORTERM naming
   truecolor or 24bit, or from TERM naming 256color or direct. The check is the
   environment heuristic modern tools rely on, since reading the terminfo
   database would pull in a curses dependency shit does without. */
ITL_DEF int itl_term_supports_256_color(void)
{
  if (itl_g_supports_256_color < 0) {
    const char *colorterm = getenv("COLORTERM");
    const char *term = getenv("TERM");
    itl_g_supports_256_color =
        (colorterm != NULL && (strstr(colorterm, "truecolor") != NULL ||
                               strstr(colorterm, "24bit") != NULL)) ||
        (term != NULL &&
         (strstr(term, "256color") != NULL || strstr(term, "direct") != NULL));
  }
  return itl_g_supports_256_color;
}

/* Whether the terminal renders the colors and cursor moves the editor draws, so
   it is a real terminal rather than a dumb or an absent one. A dumb terminal
   turns off the ghost suggestion and the flash. */
ITL_DEF int itl_term_supports_decorations(void)
{
  if (itl_g_supports_decorations < 0) {
#if defined ITL_WIN32 && !defined ITL_NO_WIN_ESCAPES
    const char *term = getenv("TERM");
    /* Raw initialization enables VT output before the editor renders, while
       native Windows sessions commonly leave TERM unset. Keep dumb as an
       explicit opt-out. */
    itl_g_supports_decorations =
        term == NULL || term[0] == '\0' || strcmp(term, "dumb") != 0;
#else
    const char *term = getenv("TERM");
    itl_g_supports_decorations =
        term != NULL && term[0] != '\0' && strcmp(term, "dumb") != 0;
#endif
  }
  return itl_g_supports_decorations;
}

/* The flash on and off SGR for the current terminal, the bright white on black
   where the bright set is supported and the normal white on black otherwise. */
ITL_DEF const char *itl_flash_sgr_on(void)
{
  return itl_term_supports_256_color() ? ITL_FLASH_TINT_ON
                                       : ITL_FLASH_REVERSE_ON;
}
ITL_DEF const char *itl_flash_sgr_off(void)
{
  return itl_term_supports_256_color() ? ITL_FLASH_TINT_OFF
                                       : ITL_FLASH_REVERSE_OFF;
}

/* The flash hold, matching fish's 100ms, long enough to perceive and short
   enough not to feel like a stall on a deliberate TAB. */
#define ITL_FLASH_HOLD_MS 100

/* Sleep the flash hold without busy-waiting. A signal that cuts it short just
   ends the flash early, which is harmless. */
ITL_DEF void itl_flash_sleep(void)
{
#if defined ITL_POSIX
  struct pollfd unused_fd;
  unused_fd.fd = -1;
  unused_fd.events = 0;
  unused_fd.revents = 0;
  poll(&unused_fd, 0, ITL_FLASH_HOLD_MS);
#elif defined ITL_WIN32
  Sleep(ITL_FLASH_HOLD_MS);
#endif
}

/* The most spans one line carries. A span per token on a normal line stays well
   under this, and the host stops filling at capacity. */
#define ITL_HIGHLIGHT_MAX_SPANS 256

/* The longest SGR escape a saved span keeps, including the terminator. A span
   whose escape exceeds this cannot be compared across frames, so the append
   fast path stays off until a full redraw saves a comparable set. */
#define ITL_PREV_SPAN_SGR_MAX 24

/* One highlight span of the previously rendered frame. The sgr bytes are an
   owned copy, since the host's pointer is only stable for one render. */
typedef struct itl_prev_span_t
{
  size_t start;
  size_t end;
  char sgr[ITL_PREV_SPAN_SGR_MAX];
} itl_prev_span_t;

/* The highlight spans the previous frame drew. The append fast path compares
   the new frame's spans against these, since identical spans mean the colored
   regions did not move and the appended tail is uncolored, so the earlier
   bytes need no repaint. */
ITL_DEF ITL_THREAD_LOCAL itl_prev_span_t
    itl_g_le_prev_spans[ITL_HIGHLIGHT_MAX_SPANS];
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_le_prev_span_count = 0;
ITL_DEF ITL_THREAD_LOCAL bool itl_g_le_prev_spans_usable = false;

/* The reverse search builds its own highlight spans, the matched entry through
   the host callback and the prompt label and the hint as bold and yellow runs.
   The flag is set while the search block is on screen so the refresh draws
   those spans rather than highlighting the whole block as a command. */
ITL_DEF ITL_THREAD_LOCAL bool itl_g_search_spans_active = false;
ITL_DEF ITL_THREAD_LOCAL tl_highlight_span
    itl_g_search_spans[ITL_HIGHLIGHT_MAX_SPANS];
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_search_span_count = 0;
ITL_DEF ITL_THREAD_LOCAL bool itl_g_multicursor_active = false;

ITL_DEF void itl_le_save_prev_spans(const tl_highlight_span *spans,
                                    size_t count)
{
  size_t s;
  itl_g_le_prev_span_count = count;
  itl_g_le_prev_spans_usable = true;
  for (s = 0; s < count; ++s) {
    size_t sgr_len = strlen(spans[s].sgr);
    if (sgr_len >= ITL_PREV_SPAN_SGR_MAX) {
      itl_g_le_prev_spans_usable = false;
      return;
    }
    itl_g_le_prev_spans[s].start = spans[s].start;
    itl_g_le_prev_spans[s].end = spans[s].end;
    memcpy(itl_g_le_prev_spans[s].sgr, spans[s].sgr, sgr_len + 1);
  }
}

ITL_DEF bool itl_le_prev_spans_append_compatible(
    const tl_highlight_span *spans, size_t count, size_t current_length,
    const char **tail_sgr)
{
  size_t s;
  *tail_sgr = NULL;
  if (!itl_g_le_prev_spans_usable || count != itl_g_le_prev_span_count) {
    return false;
  }
  for (s = 0; s < count; ++s) {
    if (spans[s].start != itl_g_le_prev_spans[s].start ||
        strcmp(spans[s].sgr, itl_g_le_prev_spans[s].sgr) != 0)
    {
      return false;
    }
    if (spans[s].end == itl_g_le_prev_spans[s].end) {
      continue;
    }
    if (s + 1 != count ||
        itl_g_le_prev_spans[s].end != itl_g_le_prev_length ||
        spans[s].end != current_length)
    {
      return false;
    }
    *tail_sgr = spans[s].sgr;
  }
  return true;
}

ITL_DEF size_t itl_merge_visual_spans(
    const tl_highlight_span *syntax, size_t syntax_count,
    const tl_highlight_span *selection, size_t selection_count,
    size_t line_length, tl_highlight_span *out, size_t out_capacity)
{
  size_t count = 0;
  size_t position = 0;

  while (position < line_length && count < out_capacity) {
    const char *color = NULL;
    size_t color_end = line_length;
    const char *selection_sgr = NULL;
    size_t selection_end = line_length;
    size_t run_end;
    const char *sgr;
    size_t s;

    for (s = 0; s < syntax_count; ++s) {
      if (position >= syntax[s].start && position < syntax[s].end) {
        color = syntax[s].sgr;
        color_end = syntax[s].end;
        break;
      }
      if (syntax[s].start > position) {
        color_end = syntax[s].start;
        break;
      }
    }

    for (s = 0; s < selection_count; ++s) {
      if (position >= selection[s].start && position < selection[s].end) {
        selection_sgr = selection[s].sgr;
        selection_end = selection[s].end;
        break;
      }
      if (selection[s].start > position) {
        selection_end = selection[s].start;
        break;
      }
    }

    run_end = color_end < selection_end ? color_end : selection_end;
    if (run_end <= position) run_end = position + 1;

    sgr = selection_sgr != NULL ? selection_sgr : color;
    if (sgr != NULL) {
      if (count > 0 && out[count - 1].end == position &&
          out[count - 1].sgr == sgr)
      {
        out[count - 1].end = run_end;
      } else {
        out[count].start = position;
        out[count].end = run_end;
        out[count].sgr = sgr;
        count++;
      }
    }

    position = run_end;
  }

  return count;
}

/* $COLUMNS and $LINES, same as above. */
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_tty_prev_rows = 1;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_tty_prev_cols = 1;

#if defined ITL_WIN32
/* The console has no SIGWINCH, so a resize is found by polling the size and
   comparing it to the previous render. Best effort, used to redraw the line
   live as the window changes. */
ITL_DEF bool itl_win_console_resized(void)
{
  size_t rows = 0, cols = 0;
  if (!itl_tty_get_size(NULL, &rows, &cols)) {
    return false;
  }
  return (cols != itl_g_tty_prev_cols) || (rows != itl_g_tty_prev_rows);
}

#if !defined ITL_INJECT_KLEE
/* Blocks on the console input handle until a keystroke is queued, the Windows
   counterpart to the POSIX poll. The handle wakes the moment any record
   arrives, so a keypress is served without the latency of a fixed sleep, while
   the twenty millisecond timeout keeps polling the console size since a resize
   raises no signal on Windows. A non-character record, a key release, a lone
   modifier, a mouse move, or a resize, would keep the handle signaled and spin
   the wait, so it is consumed here. A resize record marks the line for a
   redraw the way SIGWINCH does. A real keystroke is left in the buffer for the
   _getch reader, told apart by _kbhit. */
ITL_DEF void itl_wait_for_input(void)
{
  HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
  for (;;) {
    if (itl_g_pushback_byte != -1 || _kbhit() != 0) {
      return;
    }
    if (WaitForSingleObject(handle, 20) != WAIT_OBJECT_0) {
      if (itl_win_console_resized()) {
        itl_g_tty_changed_size = 1;
      }
      continue;
    }
    INPUT_RECORD record;
    DWORD count = 0;
    if (!PeekConsoleInput(handle, &record, 1, &count) || count == 0) {
      continue;
    }
    if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown &&
        _kbhit() != 0)
    {
      return;
    }
    if (ReadConsoleInput(handle, &record, 1, &count) && count > 0 &&
        record.EventType == WINDOW_BUFFER_SIZE_EVENT)
    {
      itl_g_tty_changed_size = 1;
    }
  }
}
#endif /* !ITL_INJECT_KLEE */
#endif /* ITL_WIN32 */

typedef struct itl_le_metrics itl_le_metrics_t;

struct itl_le_metrics
{
  size_t total_rows; /* visual rows the whole buffer occupies, >= 1 */
  size_t cursor_row; /* 0-based visual row of the cursor */
  size_t cursor_col; /* 0-based visual column of the cursor */
};

/* Columns each wrapped or continuation row is padded by so the text lines up
   under the first row. Falls back to no padding when the prompt fills the row.
 */
/* The cells the rendered prompt occupies at this width, the clamped width
   once the prompt is at or past the terminal width, so the metrics, the
   reflow, and the refresh all wrap the same way the render does. */
ITL_DEF size_t itl_le_prompt_indent(const itl_le_t *le, size_t cols)
{
  size_t effective_width;
  itl_prompt_render_cut(le->prompt, le->prompt_width, cols, &effective_width);
  return effective_width;
}

#define ITL_LE_INDENT(le, cols) itl_le_prompt_indent((le), (cols))

/* Walks the buffer once and computes the cursor's visual row and column plus
   the total number of visual rows. It accounts for the prompt width on the
   first row, per-character display width, soft wrapping at tty_cols, a wide
   glyph that would straddle the right edge wrapping early, and embedded
   newlines. Both the renderer and the cursor metrics share this one pass. */
ITL_DEF itl_le_metrics_t itl_le_compute_metrics(const itl_le_t *le,
                                                size_t tty_cols)
{
  itl_le_metrics_t m = ITL_ZERO_INIT;
  size_t cols = ITL_MAX(tty_cols, 1);
  size_t indent = ITL_LE_INDENT(le, cols);
  /* A multi-row prompt places its trailing row, where the input begins, this
     many rows below the block's first row, so the cursor and the row total
     count from there. A single-row prompt keeps prompt_rows zero, so this is
     the unchanged starting row. */
  size_t row = le->prompt_rows;
  size_t col = indent;
  size_t i;

  for (i = 0; i <= le->line->length; ++i) {
    /* The caret sits to the left of chars[i], so record before consuming it. */
    if (i == le->cursor_position) {
      m.cursor_row = row;
      m.cursor_col = col;
    }
    if (i == le->line->length) {
      break;
    }

    if (ITL_LE_IS_NEWLINE(le->line->chars[i])) {
      row += 1;
      col = indent;
    } else {
      size_t char_width = itl_char_width(le->line->chars[i]);
      /* A double-width glyph is never split across the right edge. */
      if (char_width == 2 && col + 1 >= cols) {
        row += 1;
        col = indent;
      }
      col += char_width;
      if (col >= cols) {
        row += 1;
        col = indent;
      }
    }
  }

  m.total_rows = row + 1;
  return m;
}

/* On a resize the terminal reflows each row the previous render emitted to the
   new width independently, since each was terminated by our own newline. This
   returns how many reflowed rows sit above the caret, so the renderer can step
   the cursor, which the terminal left on the caret's reflowed row, up to the
   true top of the block before clearing. */
ITL_DEF size_t itl_le_reflow_rows_above_caret(const itl_le_t *le,
                                              size_t old_cols, size_t new_cols)
{
  size_t ocols = ITL_MAX(old_cols, 1);
  size_t ncols = ITL_MAX(new_cols, 1);
  size_t indent = ITL_LE_INDENT(le, ocols);
  size_t col = indent;
  size_t rows_above = 0;
  size_t i;

  for (i = 0; i <= le->line->length; ++i) {
    if (i == le->cursor_position) {
      /* The caret sits on sub-row col / ncols of its own emitted row. */
      return rows_above + col / ncols;
    }
    if (i == le->line->length) {
      break;
    }

    if (ITL_LE_IS_NEWLINE(le->line->chars[i])) {
      rows_above += ITL_MAX((size_t) 1, (col + ncols - 1) / ncols);
      col = indent;
    } else {
      size_t char_width = itl_char_width(le->line->chars[i]);
      if (char_width == 2 && col + 1 >= ocols) {
        rows_above += ITL_MAX((size_t) 1, (col + ncols - 1) / ncols);
        col = indent;
      }
      col += char_width;
      if (col >= ocols) {
        rows_above += ITL_MAX((size_t) 1, (col + ncols - 1) / ncols);
        col = indent;
      }
    }
  }

  return rows_above + col / ncols;
}

/* Returns the character index whose caret lands on target_row at or before
   goal_column. Used by visual up and down movement. */
ITL_DEF size_t itl_le_index_at_visual(const itl_le_t *le, size_t tty_cols,
                                      size_t target_row, size_t goal_column)
{
  size_t cols = ITL_MAX(tty_cols, 1);
  size_t indent = ITL_LE_INDENT(le, cols);
  /* The rows count from prompt_rows the way itl_le_compute_metrics counts them,
     so a target row from those metrics lands on the same input row under a
     multi-row prompt. */
  size_t row = le->prompt_rows;
  size_t col = indent;
  size_t i, best_index = 0;
  bool has_best = false;

  for (i = 0; i <= le->line->length; ++i) {
    if (row == target_row) {
      if (col <= goal_column) {
        best_index = i;
        has_best = true;
      } else {
        return has_best ? best_index : i;
      }
    } else if (has_best && row > target_row) {
      return best_index;
    }

    if (i == le->line->length) {
      break;
    }

    if (ITL_LE_IS_NEWLINE(le->line->chars[i])) {
      row += 1;
      col = indent;
    } else {
      size_t char_width = itl_char_width(le->line->chars[i]);
      if (char_width == 2 && col + 1 >= cols) {
        row += 1;
        col = indent;
      }
      col += char_width;
      if (col >= cols) {
        row += 1;
        col = indent;
      }
    }
  }

  return has_best ? best_index : le->line->length;
}

/* Returns the index of the first character of the logical line the cursor is
   on, where logical lines are split by newline characters. */
ITL_DEF size_t itl_le_logical_line_start(const itl_le_t *le)
{
  size_t p = le->cursor_position;
  while (p > 0 && !ITL_LE_IS_NEWLINE(le->line->chars[p - 1])) {
    p -= 1;
  }
  return p;
}

/* Returns the index one past the last character of the cursor's logical line.
 */
ITL_DEF size_t itl_le_logical_line_end(const itl_le_t *le)
{
  size_t q = le->cursor_position;
  while (q < le->line->length && !ITL_LE_IS_NEWLINE(le->line->chars[q])) {
    q += 1;
  }
  return q;
}

ITL_DEF size_t itl_le_line_start_of(const itl_le_t *le, size_t position)
{
  size_t p = position;
  while (p > 0 && !ITL_LE_IS_NEWLINE(le->line->chars[p - 1])) {
    p -= 1;
  }
  return p;
}

ITL_DEF size_t itl_le_line_end_of(const itl_le_t *le, size_t position)
{
  size_t q = position;
  while (q < le->line->length && !ITL_LE_IS_NEWLINE(le->line->chars[q])) {
    q += 1;
  }
  return q;
}

ITL_DEF size_t itl_le_line_index_of(const itl_le_t *le, size_t position)
{
  size_t p = 0;
  size_t line_count = 0;
  while (p < position && p < le->line->length) {
    if (ITL_LE_IS_NEWLINE(le->line->chars[p])) {
      line_count += 1;
    }
    p += 1;
  }
  return line_count;
}

ITL_DEF size_t itl_le_line_start_at_index(const itl_le_t *le, size_t line_index)
{
  size_t p = 0;
  size_t line_count = 0;
  while (p < le->line->length && line_count < line_index) {
    if (ITL_LE_IS_NEWLINE(le->line->chars[p])) {
      line_count += 1;
    }
    p += 1;
  }
  return p;
}

ITL_DEF void itl_vi_sync_cursor_shape(itl_char_buf_t *b)
{
  int desired;

  switch (itl_g_edit_mode) {
  case TL_EDIT_MODE_VI_INSERT: desired = ITL_VI_CURSOR_BAR_SHAPE; break;
  case TL_EDIT_MODE_VI_VISUAL: desired = ITL_VI_CURSOR_UNDERLINE_SHAPE; break;
  case TL_EDIT_MODE_VI_COMMAND:
    desired = (itl_g_vi_pending_operator != ITL_VI_OP_NONE)
                  ? ITL_VI_CURSOR_UNDERLINE_SHAPE
                  : ITL_VI_CURSOR_BLOCK_SHAPE;
    break;
  default: desired = ITL_VI_CURSOR_DEFAULT_SHAPE; break;
  }

  if (desired == itl_g_vi_cursor_shape) {
    return;
  }

  itl_g_vi_cursor_shape = desired;

  itl_char_buf_append_byte(b, 0x1b);
  itl_char_buf_append_byte(b, '[');
  itl_char_buf_append_byte(b, (uint8_t) ('0' + desired));
  itl_char_buf_append_byte(b, ' ');
  itl_char_buf_append_byte(b, 'q');
}

/* NOTE: Hottest function in the library. */
ITL_DEF bool itl_le_tty_refresh(itl_le_t *le)
{
  size_t i, j, tty_rows, tty_cols, cols, indent;
  size_t col, move_up;
  itl_le_metrics_t m;
  /* A genuine resize reflowed the previous render, so the stored row counts are
     stale and the clear below cannot trust them. The first render has no
     previous block, so it is never treated as a resize. */
  bool is_resize = (itl_g_tty_changed_size != 0) && !itl_g_tty_first_render;

  /* A resize that lands while an arrow key cleared the text-refresh flag must
     still reflow and repaint, so force the text path. Otherwise the cursor-only
     branch would swallow the resize, record the new size, and leave the stale
     row counts against a reflowed screen. */
  if (is_resize) {
    itl_g_tty_should_refresh_text = true;
  }

  /* Write everything into a buffer, then dump it all at once */
  itl_char_buf_t *b;

  TL_ASSERT(le->line);
  TL_ASSERT(le->line->chars);
  TL_ASSERT(le->line->size >= le->line->length);
  TL_ASSERT(le->line->length <= ITL_STRING_MAX_LEN);

  if (itl_g_tty_changed_size) {
    ITL_TRY(itl_tty_get_size(le, &tty_rows, &tty_cols), {
      /* Could not get terminal size? */
      tty_rows = 24;
      tty_cols = 80;
    });
  } else {
    tty_rows = itl_g_tty_prev_rows;
    tty_cols = itl_g_tty_prev_cols;
  }

  cols = ITL_MAX(tty_cols, 1);
  indent = ITL_LE_INDENT(le, cols);
  if (itl_g_tty_plain_append_pending && !is_resize &&
      !itl_g_tty_first_render && itl_g_le_prev_cursor_at_end &&
      itl_g_le_prev_cursor_col + itl_g_tty_plain_append_width < cols)
  {
    m.total_rows = itl_g_le_prev_total_rows;
    m.cursor_row = itl_g_le_prev_cursor_row - 1;
    m.cursor_col =
        itl_g_le_prev_cursor_col + itl_g_tty_plain_append_width;
  } else {
#if !defined NDEBUG
    itl_g_debug_metrics_scan_count += 1;
#endif
    m = itl_le_compute_metrics(le, tty_cols);
  }

  ITL_TRACELN("refresh: total %zu, crow %zu, ccol %zu, curp %zu\n",
              m.total_rows, m.cursor_row, m.cursor_col, le->cursor_position);

  /* The new frame's text and its highlight spans are computed once here and
     shared by the append fast path and the full redraw below, so the line is
     serialized and the host callback runs at most once per refresh. The host
     receives the line and fills colored codepoint spans, sorted by start and
     non-overlapping, so one left-to-right cursor opens and closes them. The
     escapes are emitted between codepoints, so they carry zero column width
     and the metrics pass that placed the cursor never sees them, the same
     zero-width handling the ghost text relies on. Out-of-bounds or empty spans
     are dropped here. */
  char itl_cur_render_storage[ITL_STRING_MAX_LEN];
  const char *itl_cur_render = itl_cur_render_storage;
  bool have_cur_render = false;
  tl_highlight_span itl_spans[ITL_HIGHLIGHT_MAX_SPANS];
  tl_highlight_span itl_syntax_spans[ITL_HIGHLIGHT_MAX_SPANS];
  size_t span_count = 0;
  const char *append_tail_sgr = NULL;
  if (itl_g_tty_should_refresh_text) {
    if (itl_g_serialized_line_ready) {
      itl_cur_render = itl_g_serialized_line;
      have_cur_render = true;
    } else {
#if !defined NDEBUG
      itl_g_debug_line_serialization_count += 1;
#endif
      have_cur_render =
          itl_string_to_cstr(le->line, itl_cur_render_storage,
                             sizeof(itl_cur_render_storage)) == TL_SUCCESS;
    }
    itl_g_serialized_line_ready = false;
    if (itl_g_search_spans_active &&
        (itl_g_edit_mode == TL_EDIT_MODE_VI_VISUAL ||
         itl_g_multicursor_active) &&
        have_cur_render && itl_g_highlight_callback != NULL)
    {
      tl_highlight hl;
      size_t syntax_count = 0;
      hl.spans = itl_syntax_spans;
      hl.count = 0;
      hl.capacity = ITL_HIGHLIGHT_MAX_SPANS;
      if (itl_g_highlight_callback(itl_cur_render, &hl)) {
        size_t s;
        for (s = 0; s < hl.count && s < ITL_HIGHLIGHT_MAX_SPANS; ++s) {
          if (itl_syntax_spans[s].start < itl_syntax_spans[s].end &&
              itl_syntax_spans[s].end <= le->line->length &&
              itl_syntax_spans[s].sgr != NULL)
          {
            itl_syntax_spans[syntax_count++] = itl_syntax_spans[s];
          }
        }
      }
      span_count = itl_merge_visual_spans(
          itl_syntax_spans, syntax_count, itl_g_search_spans,
          itl_g_search_span_count, le->line->length, itl_spans,
          ITL_HIGHLIGHT_MAX_SPANS);
    } else if (itl_g_search_spans_active) {
      /* The reverse search prebuilt its spans for the whole block, so the host
         callback is skipped and those spans are validated and drawn. */
      size_t s;
      for (s = 0; s < itl_g_search_span_count; ++s) {
        if (itl_g_search_spans[s].start < itl_g_search_spans[s].end &&
            itl_g_search_spans[s].end <= le->line->length &&
            itl_g_search_spans[s].sgr != NULL)
        {
          itl_spans[span_count++] = itl_g_search_spans[s];
        }
      }
    } else if (have_cur_render && itl_g_highlight_callback != NULL) {
      tl_highlight hl;
      hl.spans = itl_spans;
      hl.count = 0;
      hl.capacity = ITL_HIGHLIGHT_MAX_SPANS;
      if (itl_g_highlight_callback(itl_cur_render, &hl)) {
        size_t s;
        for (s = 0; s < hl.count && s < ITL_HIGHLIGHT_MAX_SPANS; ++s) {
          if (itl_spans[s].start < itl_spans[s].end &&
              itl_spans[s].end <= le->line->length && itl_spans[s].sgr != NULL)
          {
            itl_spans[span_count++] = itl_spans[s];
          }
        }
      }
    }
  }

  bool spans_are_append_compatible = itl_le_prev_spans_append_compatible(
      itl_spans, span_count, le->line->length, &append_tail_sgr);
  if (itl_g_tty_should_refresh_text && !is_resize && !itl_g_tty_first_render &&
      have_cur_render && m.total_rows == itl_g_le_prev_total_rows &&
      m.cursor_row + 1 == itl_g_le_prev_cursor_row &&
      le->cursor_position == le->line->length && itl_g_le_prev_cursor_at_end &&
      spans_are_append_compatible)
  {
    /* A successful itl_string_to_cstr wrote exactly the line's byte size, so
       the length is read off the line rather than recounted with strlen. */
    size_t cur_len = le->line->size;
    if (cur_len > itl_g_le_prev_render_len &&
        memcmp(itl_cur_render, itl_g_le_prev_render,
               itl_g_le_prev_render_len) == 0)
    {
      itl_char_buf_t *fb = &itl_g_char_buffer;
      size_t k;
      if (append_tail_sgr != NULL) {
        itl_char_buf_append_cstr(fb, append_tail_sgr);
      }
      for (k = itl_g_le_prev_render_len; k < cur_len; ++k) {
        itl_char_buf_append_byte(fb, (uint8_t) itl_cur_render[k]);
      }
      if (append_tail_sgr != NULL) {
        itl_char_buf_append_cstr(fb, ITL_HIGHLIGHT_RESET);
      }
      ITL_TTY_CLEAR_TO_END(fb);
      if (itl_g_ghost_len > 0 && m.cursor_col + itl_g_ghost_width < cols) {
        itl_char_buf_append_cstr(fb, "\x1b[90m");
        itl_char_buf_append_cstr(fb, itl_g_ghost);
        itl_char_buf_append_cstr(fb, "\x1b[0m");
        ITL_TTY_CLEAR_TO_END(fb);
        ITL_TTY_MOVE_TO_COLUMN(fb, m.cursor_col + 1);
      }
      memcpy(itl_g_le_prev_render, itl_cur_render, cur_len);
      itl_g_le_prev_render_len = cur_len;
      itl_g_le_prev_length = le->line->length;
      itl_g_le_prev_total_rows = m.total_rows;
      itl_g_le_prev_cursor_row = m.cursor_row + 1;
      itl_g_le_prev_cursor_col = m.cursor_col;
      itl_g_le_prev_cursor_at_end = true;
#if !defined NDEBUG
      itl_g_debug_append_refresh_count += 1;
#endif
      itl_g_tty_plain_append_pending = false;
      ITL_CHAR_BUF_DUMP(fb);
      ITL_CHAR_BUF_CLEAR(fb);
      return true;
    }
  }

  b = &itl_g_char_buffer;
#if !defined NDEBUG
  if (itl_g_tty_should_refresh_text) {
    itl_g_debug_full_refresh_count += 1;
  }
#endif
  itl_vi_sync_cursor_shape(b);
  ITL_TTY_HIDE_CURSOR(b);
  ITL_TTY_AUTOWRAP_OFF(b);

  if (itl_g_tty_should_refresh_text) {
    if (is_resize) {
      /* The terminal reflowed the previous render, so the stored row counts are
         stale. The cursor sits on the caret's reflowed row, so step up by the
         reflowed rows above it to reach the block top, then clear everything
         below. The input block is the last thing on screen, so nothing below it
         is lost. */
      size_t rows_above =
          itl_le_reflow_rows_above_caret(le, itl_g_tty_prev_cols, tty_cols);
      if (rows_above > 0) {
        ITL_TTY_MOVE_UP(b, rows_above);
      }
      ITL_TTY_MOVE_TO_COLUMN(b, 1);
      ITL_TTY_CLEAR_BELOW(b);
    } else {
      /* Park at the top-left of the previous render. */
      if (itl_g_le_prev_cursor_row > 1) {
        ITL_TTY_MOVE_UP(b, itl_g_le_prev_cursor_row - 1);
      }
      ITL_TTY_MOVE_TO_COLUMN(b, 1);

      /* Clear every row the previous render occupied, leaving rows we do not
         own untouched. */
      for (i = 0; i < itl_g_le_prev_total_rows; ++i) {
        ITL_TTY_CLEAR_WHOLE_LINE(b);
        if (i + 1 < itl_g_le_prev_total_rows) {
          ITL_TTY_MOVE_DOWN(b, 1);
        }
      }
      if (itl_g_le_prev_total_rows > 1) {
        ITL_TTY_MOVE_UP(b, itl_g_le_prev_total_rows - 1);
      }
      ITL_TTY_MOVE_TO_COLUMN(b, 1);
    }

    if (le->prompt != NULL) {
      /* A prompt at or past the terminal width renders as the ellipsis
         marker and its tail, the same cut the indent math uses, so the
         render and the cursor accounting agree. */
      size_t rendered_prompt_width;
      size_t prompt_cut = itl_prompt_render_cut(
          le->prompt, le->prompt_width, tty_cols, &rendered_prompt_width);
      if (prompt_cut == 0) {
        itl_char_buf_append_cstr(b, le->prompt);
      } else if (rendered_prompt_width > 0) {
        itl_char_buf_append_cstr(b, ITL_PROMPT_ELLIPSIS);
        itl_char_buf_append_cstr(b, le->prompt + prompt_cut);
      }
    }

    /* Emit the buffer, reproducing the metrics column accounting so our own
       line breaks stay in sync with the terminal. Continuation rows are padded
       so their text lines up under the first row. The span cursor closes a span
       that ends at this codepoint, then opens the one that starts here. */
    size_t next_span = 0;
    bool in_span = false;
    size_t open_end = 0;
    const char *open_sgr = NULL;
    bool suppress_pad = itl_g_edit_mode == TL_EDIT_MODE_VI_VISUAL;
    col = indent;
    /* The flash repaints the whole line in one tone. It opens that SGR once and
       the loop below skips the per-span color. */
    if (itl_g_tty_flash_active) {
      itl_char_buf_append_cstr(b, itl_flash_sgr_on());
    }
    for (i = 0; i < le->line->length; ++i) {
      itl_utf8_t ch = le->line->chars[i];

      if (!itl_g_tty_flash_active) {
        if (in_span && i == open_end) {
          itl_char_buf_append_cstr(b, ITL_HIGHLIGHT_RESET);
          in_span = false;
        }
        if (!in_span && next_span < span_count &&
            i == itl_spans[next_span].start)
        {
          itl_char_buf_append_cstr(b, itl_spans[next_span].sgr);
          in_span = true;
          open_end = itl_spans[next_span].end;
          open_sgr = itl_spans[next_span].sgr;
          next_span++;
        }
      }

      if (ITL_LE_IS_NEWLINE(ch)) {
        /* A block selection over an empty line carries a one-cell span on this
           newline, which has no character of its own to reverse. A reversed
           space is drawn in its place so the selected cell and the mock cursor
           still show, then the span is closed since it ends at this cell. */
        if (in_span && i + 1 == open_end) {
          itl_char_buf_append_byte(b, ' ');
          itl_char_buf_append_cstr(b, ITL_HIGHLIGHT_RESET);
          in_span = false;
        } else if (in_span && suppress_pad) {
          itl_char_buf_append_cstr(b, ITL_HIGHLIGHT_RESET);
        }
        itl_char_buf_append_cstr(b, ITL_LF);
        itl_char_buf_append_spaces(b, indent);
        col = indent;
        if (in_span && suppress_pad) {
          itl_char_buf_append_cstr(b, open_sgr);
        }
        continue;
      }

      {
        size_t char_width = itl_char_width(ch);
        if (char_width == 2 && col + 1 >= cols) {
          if (in_span && suppress_pad) {
            itl_char_buf_append_cstr(b, ITL_HIGHLIGHT_RESET);
          }
          itl_char_buf_append_cstr(b, ITL_LF);
          itl_char_buf_append_spaces(b, indent);
          col = indent;
          if (in_span && suppress_pad) {
            itl_char_buf_append_cstr(b, open_sgr);
          }
        }
        for (j = 0; j < ch.size; ++j) {
          itl_char_buf_append_byte(b, ch.bytes[j]);
        }
        col += char_width;
        if (col >= cols) {
          if (in_span && suppress_pad) {
            itl_char_buf_append_cstr(b, ITL_HIGHLIGHT_RESET);
          }
          itl_char_buf_append_cstr(b, ITL_LF);
          itl_char_buf_append_spaces(b, indent);
          col = indent;
          if (in_span && suppress_pad) {
            itl_char_buf_append_cstr(b, open_sgr);
          }
        }
      }
    }
    /* A span that runs to the end of the line never hit its close in the loop,
       so its reset is emitted here. */
    if (in_span) {
      itl_char_buf_append_cstr(b, ITL_HIGHLIGHT_RESET);
    }
    /* Close the flash SGR so the trailing clear and the ghost draw run normal.
     */
    if (itl_g_tty_flash_active) {
      itl_char_buf_append_cstr(b, itl_flash_sgr_off());
    }
    ITL_TTY_CLEAR_TO_END(b);

    /* Draw the ghost suggestion dimmed after the line. It is shown only when
       the cursor sits at the very end of the buffer and the suggestion fits on
       the current row without wrapping, so it never pushes a line break and the
       cursor restore below lands on the real caret. A second clear erases a
       longer ghost left from a previous frame, then the cursor is parked back
       on the real caret column. */
    if (itl_g_ghost_len > 0 && le->cursor_position == le->line->length &&
        col + itl_g_ghost_width < cols)
    {
      itl_char_buf_append_cstr(b, "\x1b[90m");
      itl_char_buf_append_cstr(b, itl_g_ghost);
      itl_char_buf_append_cstr(b, "\x1b[0m");
      ITL_TTY_CLEAR_TO_END(b);
    }

    /* Move from the end of the rendered text up to the cursor's row. */
    move_up = (m.total_rows - 1) - m.cursor_row;
    if (move_up > 0) {
      ITL_TTY_MOVE_UP(b, move_up);
    }
  } else {
    /* Only the caret moved, so step from the previously stored caret row. */
    size_t prev_row = itl_g_le_prev_cursor_row - 1;
    if (m.cursor_row < prev_row) {
      ITL_TTY_MOVE_UP(b, prev_row - m.cursor_row);
    } else if (m.cursor_row > prev_row) {
      ITL_TTY_MOVE_DOWN(b, m.cursor_row - prev_row);
    }
  }

  ITL_TTY_MOVE_TO_COLUMN(b, m.cursor_col + 1);

  itl_g_le_prev_total_rows = m.total_rows;
  itl_g_le_prev_cursor_row = m.cursor_row + 1;
  itl_g_le_prev_cursor_col = m.cursor_col;
  /* Record whether this frame, text or cursor-only, parked the caret at the
     line end, so the append fast path on the next keystroke knows the physical
     cursor sits at the append point. */
  itl_g_le_prev_cursor_at_end = (le->cursor_position == le->line->length);

  /* Remember the line and the spans this text refresh drew, so the next
     keystroke can take the append fast path. A cursor-only refresh leaves the
     line untouched and keeps the stored render. A failed conversion forces a
     full redraw next time. */
  if (itl_g_tty_should_refresh_text) {
    if (have_cur_render) {
      /* A successful itl_string_to_cstr wrote exactly the line's byte size, so
         the length is read off the line rather than recounted with strlen. */
      itl_g_le_prev_render_len = le->line->size;
      itl_g_le_prev_length = le->line->length;
      memcpy(itl_g_le_prev_render, itl_cur_render, itl_g_le_prev_render_len);
    } else {
      itl_g_le_prev_render_len = 0;
      itl_g_le_prev_length = 0;
    }
    itl_le_save_prev_spans(itl_spans, span_count);
  }

  itl_g_tty_prev_rows = tty_rows;
  itl_g_tty_prev_cols = tty_cols;
  itl_g_tty_first_render = false;
  itl_g_tty_plain_append_pending = false;

  itl_g_tty_changed_size = 0;
  ITL_TTY_AUTOWRAP_ON(b);
  ITL_TTY_SHOW_CURSOR(b);

  ITL_CHAR_BUF_DUMP(b);
  ITL_CHAR_BUF_CLEAR(b);

  return true;
}

ITL_DEF ITL_THREAD_LOCAL itl_le_t itl_g_le = ITL_ZERO_INIT;

#if defined ITL_POSIX
ITL_DEF ITL_THREAD_LOCAL void (*itl_g_prev_sigwinch)(int) = SIG_DFL;

ITL_DEF void itl_handle_sigwinch(int signal_number)
{
  if (signal_number != SIGWINCH) {
    return;
  }
  /* Only set the flag here. The main loop does the redraw in normal context. */
  itl_g_tty_changed_size = 1;
}
#endif

ITL_DEF ITL_THREAD_LOCAL int itl_g_last_control = TL_KEY_UNKN;

TL_DEF int tl_last_control_sequence(void) { return itl_g_last_control; }

/* The host completion callback, or NULL when completion is disabled. Only the
   interactive host registers one. */
ITL_DEF ITL_THREAD_LOCAL tl_complete_fn itl_g_complete_callback = NULL;

TL_DEF void tl_set_complete_callback(tl_complete_fn callback)
{
  itl_g_complete_callback = callback;
}

/* Whether the dimmed ghost suggestion is offered at all. A host that wants no
   inline hint, such as one started with a no-completion flag, turns it off so
   neither the completion nor the history source fills it. */
ITL_DEF ITL_THREAD_LOCAL int itl_g_ghost_enabled = 1;

TL_DEF void tl_set_ghost_enabled(int enabled)
{
  /* A dumb terminal cannot render the dimmed ghost suggestion, so the ghost
     stays off there whatever the host requests. */
  itl_g_ghost_enabled = enabled && itl_term_supports_decorations();
}

/* The host ghost validation callback, or NULL when every history entry is
   acceptable. Consulted by the history scan only. */
ITL_DEF ITL_THREAD_LOCAL tl_ghost_validate_fn itl_g_ghost_validate_callback =
    NULL;

TL_DEF void tl_set_ghost_validate_callback(tl_ghost_validate_fn callback)
{
  itl_g_ghost_validate_callback = callback;
}

TL_DEF void tl_set_wake_callback(tl_wake_fn callback)
{
  itl_g_wake_callback = callback;
}

TL_DEF void tl_set_highlight_callback(tl_highlight_fn callback)
{
  itl_g_highlight_callback = callback;
}

TL_DEF void tl_set_edit_mode(int mode)
{
  if (mode == TL_EDIT_MODE_VI_INSERT || mode == TL_EDIT_MODE_VI_COMMAND ||
      mode == TL_EDIT_MODE_VI_VISUAL)
  {
    itl_g_edit_mode_base = TL_EDIT_MODE_VI_INSERT;
  } else {
    itl_g_edit_mode_base = TL_EDIT_MODE_EMACS;
  }

  itl_g_edit_mode = itl_g_edit_mode_base;
}

/* Insert a UTF-8 C-string at the cursor, one decoded character at a time, so
   the line editor's character model stays intact. Returns false when the line
   buffer would overflow, leaving the part that fit in place. */
ITL_DEF bool itl_le_insert_cstr(itl_le_t *le, const char *text)
{
  size_t i = 0;
  while (text[i] != '\0') {
    uint8_t first = (uint8_t) text[i];
    uint8_t width = itl_utf8_width(first);
    itl_utf8_t ch;
    uint8_t k;

    if (width < 1) {
      width = 1;
    }
    ch.bytes[0] = first;
    ch.size = 1;
    for (k = 1; k < width && text[i + k] != '\0'; ++k) {
      ch.bytes[k] = (uint8_t) text[i + k];
      ch.size += 1;
    }
    if (!itl_le_insert(le, ch)) {
      return false;
    }
    i += ch.size;
  }
  return true;
}

/* Clear the recorded ghost text, so the next refresh draws none. */
ITL_DEF void itl_ghost_clear(void)
{
  itl_g_ghost_len = 0;
  itl_g_ghost_width = 0;
  itl_g_ghost[0] = '\0';
  itl_g_ghost_case_fix_len = 0;
  itl_g_ghost_case_fix[0] = '\0';
}

ITL_DEF void itl_ghost_accept(itl_le_t *le)
{
  if (itl_g_ghost_case_fix_len > 0) {
    /* The dispatch closed the insert run, so clear_line would otherwise erase
       the typed prefix before any snapshot captures it. Push the pre-accept
       line and open the run first, mirroring the plain accept path where the
       first insert snapshots, so undo restores the typed prefix. */
    itl_undo_push(le);
    itl_g_undo_insert_run_open = true;
    itl_le_clear_line(le);
    itl_le_insert_cstr(le, itl_g_ghost_case_fix);
  } else {
    itl_le_insert_cstr(le, itl_g_ghost);
  }
}

ITL_DEF bool itl_ghost_extends_completion_miss_plainly(
    const char *line_cstr, size_t line_byte_len)
{
  size_t position;
  if (itl_g_ghost_completion_miss_prefix_length == 0 ||
      line_byte_len < itl_g_ghost_completion_miss_prefix_length ||
      memcmp(line_cstr, itl_g_ghost_completion_miss_prefix,
             itl_g_ghost_completion_miss_prefix_length) != 0)
  {
    return false;
  }

  for (position = itl_g_ghost_completion_miss_prefix_length;
       position < line_byte_len; position++)
  {
    unsigned char byte = (unsigned char) line_cstr[position];
    bool is_plain = (byte >= 'a' && byte <= 'z') ||
                    (byte >= 'A' && byte <= 'Z') ||
                    (byte >= '0' && byte <= '9') || byte == '_' ||
                    byte == '-' || byte == '.' || byte >= 0x80;
    if (!is_plain) {
      return false;
    }
  }

  return true;
}

ITL_DEF void itl_ghost_record_completion_miss(const char *line_cstr,
                                               size_t line_byte_len)
{
  if (line_byte_len >= sizeof(itl_g_ghost_completion_miss_prefix)) {
    itl_g_ghost_completion_miss_prefix[0] = '\0';
    itl_g_ghost_completion_miss_prefix_length = 0;
    return;
  }

  memcpy(itl_g_ghost_completion_miss_prefix, line_cstr, line_byte_len + 1);
  itl_g_ghost_completion_miss_prefix_length = line_byte_len;
}

/* Ask the host for the top completion of the current line and fill the ghost
   suffix when the longest common prefix extends the token under the cursor. The
   ghost is the part of the common prefix past what the user already typed, so
   it only ever appends. Leaves the ghost cleared when completion offers
   nothing. */
ITL_DEF void itl_ghost_fill_from_completion(itl_le_t *le,
                                            const char *line_cstr,
                                            size_t line_byte_len)
{
  tl_completion result;

  if (itl_g_complete_callback == NULL) {
    return;
  }
  if (itl_ghost_extends_completion_miss_plainly(line_cstr, line_byte_len)) {
    return;
  }
  itl_g_ghost_completion_miss_prefix[0] = '\0';
  itl_g_ghost_completion_miss_prefix_length = 0;
  /* The cursor passed to the callback is a codepoint index, the unit toiletline
     edits in, and it equals the line length here since the ghost only fires at
     the end of the line. */
  if (!itl_g_complete_callback(line_cstr, le->line->length, &result, 0)) {
    itl_ghost_record_completion_miss(line_cstr, line_byte_len);
    return;
  }
  if (result.count == 0) {
    itl_ghost_record_completion_miss(line_cstr, line_byte_len);
    return;
  }
  if (result.longest_common_prefix == NULL) {
    return;
  }

  {
    /* The token under the cursor runs from token_start to the end of the line,
       so its length in codepoints is the line length minus the start. The ghost
       is the part of the common prefix past that typed length, the suffix the
       user has not typed yet. token_start is a codepoint index, so the prefix
       is measured in codepoints and then walked to its byte offset. */
    size_t typed_len = le->line->length - result.token_start;
    size_t lcp_len = tl_utf8_strlen(result.longest_common_prefix);
    if (lcp_len <= typed_len) {
      return;
    }
    {
      /* Skip the typed codepoints to find where the untyped byte suffix begins,
         since the prefix bytes the user already typed are not part of the
         ghost. */
      const char *lcp = result.longest_common_prefix;
      size_t skip_offset = 0;
      size_t skipped = 0;
      while (skipped < typed_len && lcp[skip_offset] != '\0') {
        if ((lcp[skip_offset] & 0xC0) != 0x80) {
          skipped += 1;
        }
        skip_offset += 1;
      }
      /* A continuation byte that belongs to the last skipped codepoint must not
         start the suffix, so advance past the whole codepoint. */
      while (lcp[skip_offset] != '\0' && (lcp[skip_offset] & 0xC0) == 0x80) {
        skip_offset += 1;
      }
      size_t suffix_len = strlen(lcp + skip_offset);
      if (suffix_len >= sizeof(itl_g_ghost)) {
        return;
      }
      memcpy(itl_g_ghost, lcp + skip_offset, suffix_len);
      itl_g_ghost[suffix_len] = '\0';
      itl_g_ghost_len = suffix_len;
      {
        size_t token_start_bytes = 0;
        size_t cp;
        for (cp = 0;
             cp < result.token_start && line_cstr[token_start_bytes] != '\0';
             cp++)
        {
          token_start_bytes += 1;
          while (line_cstr[token_start_bytes] != '\0' &&
                 (line_cstr[token_start_bytes] & 0xC0) == 0x80)
          {
            token_start_bytes += 1;
          }
        }
        {
          size_t typed_byte_len = line_byte_len - token_start_bytes;
          size_t lcp_full_len = strlen(lcp);
          size_t fixed_len = token_start_bytes + lcp_full_len;
          int differs =
              skip_offset != typed_byte_len ||
              memcmp(lcp, line_cstr + token_start_bytes, typed_byte_len) != 0;
          if (differs && fixed_len < sizeof(itl_g_ghost_case_fix)) {
            memcpy(itl_g_ghost_case_fix, line_cstr, token_start_bytes);
            memcpy(itl_g_ghost_case_fix + token_start_bytes, lcp,
                   lcp_full_len + 1);
            itl_g_ghost_case_fix_len = fixed_len;
          }
        }
      }
    }
  }
}

/* Fill the ghost from history before completion. The most recent history entry
   that begins with the whole typed line supplies the rest of that line as a
   dimmed suggestion. Leaves the ghost cleared when no entry matches. */
ITL_DEF void itl_ghost_fill_from_history(const char *line_cstr,
                                         size_t line_byte_len)
{
  size_t index;
  bool found_match = false;

  if (itl_g_history_path == NULL || itl_g_history_count == 0) {
    return;
  }

  if (line_byte_len >= itl_g_ghost_history_miss_prefix_length &&
      itl_g_ghost_history_miss_prefix_length > 0 &&
      itl_ascii_prefix_matches_casefold(
          line_cstr, itl_g_ghost_history_miss_prefix,
          itl_g_ghost_history_miss_prefix_length))
  {
    return;
  }

  /* The whole file is read into memory once and cached across keystrokes, so
     the newest-first scan below decodes each entry from the buffer rather than
     seeking and reading a fresh block per entry on every keystroke. The buffer
     is dropped by itl_history_read_fd_invalidate when the file changes. */
  if (!itl_history_ensure_read_buffer()) {
    return;
  }
  /* Newest first, so the most recent matching command wins, bounded to a recent
     window. */
  size_t scanned = 0;
  /* One scratch buffer for the whole scan. */
  static ITL_THREAD_LOCAL char entry_cstr[ITL_STRING_MAX_LEN];
  for (index = itl_g_history_count; index-- > 0;) {
    size_t offset = itl_history_index_to_offset(index);
    size_t entry_len;

    if (scanned >= ITL_GHOST_HISTORY_SCAN_MAX) {
      break;
    }
    scanned += 1;
#if !defined NDEBUG
    itl_g_debug_ghost_history_scan_count += 1;
#endif

    if (!itl_history_decode_entry_buffered(offset, entry_cstr,
                                           sizeof(entry_cstr), &entry_len))
    {
      continue;
    }
    if (entry_len <= line_byte_len) {
      continue;
    }
    if (!itl_ascii_prefix_matches_casefold(entry_cstr, line_cstr,
                                           line_byte_len))
    {
      continue;
    }
    /* The host vets the entry before it becomes the suggestion, so a command
       that no longer resolves is skipped and the scan keeps looking, the way
       fish validates its autosuggestions. */
    if (itl_g_ghost_validate_callback != NULL &&
        !itl_g_ghost_validate_callback(entry_cstr))
    {
      continue;
    }
    {
      size_t suffix_len = entry_len - line_byte_len;
      if (suffix_len >= sizeof(itl_g_ghost)) {
        continue;
      }
      memcpy(itl_g_ghost, entry_cstr + line_byte_len, suffix_len);
      itl_g_ghost[suffix_len] = '\0';
      itl_g_ghost_len = suffix_len;
      if (memcmp(entry_cstr, line_cstr, line_byte_len) != 0 &&
          entry_len < sizeof(itl_g_ghost_case_fix))
      {
        memcpy(itl_g_ghost_case_fix, entry_cstr, entry_len);
        itl_g_ghost_case_fix[entry_len] = '\0';
        itl_g_ghost_case_fix_len = entry_len;
      } else {
        itl_g_ghost_case_fix[0] = '\0';
        itl_g_ghost_case_fix_len = 0;
      }
      found_match = true;
      break;
    }
  }

  if (!found_match &&
      line_byte_len < sizeof(itl_g_ghost_history_miss_prefix))
  {
    memcpy(itl_g_ghost_history_miss_prefix, line_cstr, line_byte_len + 1);
    itl_g_ghost_history_miss_prefix_length = line_byte_len;
  }
  /* The read handle stays open and cached for the next keystroke. */
}

/* Update the dimmed ghost suggestion shown after the cursor. History is tried
   first, then completion. It is shown only when the cursor sits at the very end
   of the line, so it never splits the buffer. */
ITL_DEF void itl_ghost_update(itl_le_t *le)
{
  const char *line_cstr = itl_g_serialized_line;
  bool did_extend_serialized_line = false;
  size_t line_byte_len;

  itl_ghost_clear();
  itl_g_serialized_line_ready = false;
  if (!itl_g_tty_plain_append_pending) {
    itl_g_ghost_completion_miss_prefix[0] = '\0';
    itl_g_ghost_completion_miss_prefix_length = 0;
  }

  /* The host turned the ghost off, so no source fills it. */
  if (!itl_g_ghost_enabled) {
    return;
  }

  /* A ghost past the end of a multiline or mid-line cursor would corrupt the
     redraw, so it is offered only at the very end of the line. */
  if (le->cursor_position != le->line->length) {
    return;
  }
  if (itl_g_tty_plain_append_pending && itl_g_le_prev_cursor_at_end &&
      le->line->length > 0)
  {
    const itl_utf8_t *appended = &le->line->chars[le->line->length - 1];
    if (itl_g_le_prev_render_len + appended->size == le->line->size &&
        le->line->size < sizeof(itl_g_serialized_line))
    {
      size_t byte_position;
      memcpy(itl_g_serialized_line, itl_g_le_prev_render,
             itl_g_le_prev_render_len);
      for (byte_position = 0; byte_position < appended->size; byte_position++)
        itl_g_serialized_line[itl_g_le_prev_render_len + byte_position] =
            (char) appended->bytes[byte_position];
      itl_g_serialized_line[le->line->size] = '\0';
      did_extend_serialized_line = true;
    }
  }
  if (!did_extend_serialized_line) {
#if !defined NDEBUG
    itl_g_debug_line_serialization_count += 1;
#endif
    if (itl_string_to_cstr(le->line, itl_g_serialized_line,
                           sizeof(itl_g_serialized_line)) != TL_SUCCESS)
    {
      return;
    }
  }
  itl_g_serialized_line_ready = true;
  line_byte_len = le->line->size;
  /* An empty line has nothing to extend, and it ends any sticky suggestion so a
     line cleared back to empty does not keep the previous target. */
  if (line_cstr[0] == '\0') {
    itl_g_ghost_sticky_target[0] = '\0';
    return;
  }

  itl_g_ghost_case_fix_len = 0;
  itl_g_ghost_case_fix[0] = '\0';

  /* Stay on the target already suggested while the input is still a strict
     prefix of it, whichever source first produced it. Typing further into a
     suggestion keeps it rather than flipping as the candidate set shifts
     between the completion source and the history source. */
  {
    if (itl_g_ghost_sticky_target[0] != '\0') {
      size_t target_len = strlen(itl_g_ghost_sticky_target);
      /* The match is case-insensitive on the typed prefix, so a sticky target
         that corrected the case keeps its correction as the user types further
         into it, the way Tab does. */
      if (line_byte_len < target_len &&
          itl_ascii_prefix_matches_casefold(itl_g_ghost_sticky_target,
                                            line_cstr, line_byte_len))
      {
        size_t suffix_len = target_len - line_byte_len;
        if (suffix_len < sizeof(itl_g_ghost)) {
          memcpy(itl_g_ghost, itl_g_ghost_sticky_target + line_byte_len,
                 suffix_len);
          itl_g_ghost[suffix_len] = '\0';
          itl_g_ghost_len = suffix_len;
          /* The typed prefix differs in case from the target, so accepting the
             ghost rewrites the whole line to the target's casing. */
          if (memcmp(itl_g_ghost_sticky_target, line_cstr, line_byte_len) !=
                  0 &&
              target_len < sizeof(itl_g_ghost_case_fix))
          {
            memcpy(itl_g_ghost_case_fix, itl_g_ghost_sticky_target, target_len);
            itl_g_ghost_case_fix[target_len] = '\0';
            itl_g_ghost_case_fix_len = target_len;
          } else {
            itl_g_ghost_case_fix_len = 0;
            itl_g_ghost_case_fix[0] = '\0';
          }
          itl_g_ghost_width = itl_cstr_display_width(itl_g_ghost);
          return;
        }
      }
      /* The input no longer extends the sticky target, so the target is dropped
         and a fresh suggestion is picked below. */
      itl_g_ghost_sticky_target[0] = '\0';
    }
  }

  itl_ghost_fill_from_history(line_cstr, line_byte_len);
  if (itl_g_ghost_len == 0) {
    itl_ghost_fill_from_completion(le, line_cstr, line_byte_len);
  }

  /* A multiline suggestion drawn on the current line would push the caret onto
     the next row and leave the column accounting off when the suggestion is
     accepted, so the ghost is clipped to its first line. */
  {
    char *newline = (char *) memchr(itl_g_ghost, '\n', itl_g_ghost_len);
    if (newline != NULL) {
      *newline = '\0';
      itl_g_ghost_len = (size_t) (newline - itl_g_ghost);
      if (itl_g_ghost_len == 0) {
        itl_ghost_clear();
      }
    }
  }
  itl_g_ghost_width = itl_cstr_display_width(itl_g_ghost);

  /* A source that produced a suggestion records the whole line-plus-ghost as
     the sticky target, so the next keystroke keeps it while the input stays a
     prefix of it rather than re-running the sources and flipping. */
  if (itl_g_ghost_len > 0) {
    if (itl_g_ghost_case_fix_len > 0 &&
        itl_g_ghost_case_fix_len + 1 <= sizeof(itl_g_ghost_sticky_target))
    {
      memcpy(itl_g_ghost_sticky_target, itl_g_ghost_case_fix,
             itl_g_ghost_case_fix_len + 1);
    } else {
      if (line_byte_len + itl_g_ghost_len + 1 <=
          sizeof(itl_g_ghost_sticky_target))
      {
        memcpy(itl_g_ghost_sticky_target, line_cstr, line_byte_len);
        memcpy(itl_g_ghost_sticky_target + line_byte_len, itl_g_ghost,
               itl_g_ghost_len + 1);
      }
    }
  }
}

/* Print the candidate list below the input in columns, then leave the cursor on
   a fresh line so the next refresh redraws the prompt and line beneath the
   list. The previous-render row counts are reset so the refresh treats the spot
   below the list as untouched ground and does not clear the list it just
   printed. */
ITL_DEF void itl_completion_print_list(const tl_completion *result)
{
  itl_char_buf_t *b = &itl_g_char_buffer;
  size_t tty_cols = itl_g_tty_prev_cols > 0 ? itl_g_tty_prev_cols : 80;
  size_t longest = 0;
  size_t i, column_width, columns, column;

  /* Move below the whole input block, the same accounting tl_emit_newlines
     uses, so the list never lands on top of the line. */
  size_t move_down = itl_g_le_prev_total_rows - (itl_g_le_prev_cursor_row - 1);

  ITL_CHAR_BUF_CLEAR(b);
  ITL_TTY_SHOW_CURSOR(b);
  for (i = 0; i < move_down; ++i) {
    itl_char_buf_append_cstr(b, ITL_LF);
  }

  for (i = 0; i < result->count; ++i) {
    size_t len = strlen(result->candidates[i]);
    if (len > longest) {
      longest = len;
    }
  }

  /* Two spaces between columns, at least one column even when a name is wider
     than the terminal. */
  column_width = longest + 2;
  columns = column_width >= tty_cols ? 1 : tty_cols / column_width;
  if (columns < 1) {
    columns = 1;
  }

  /* With descriptions the list is one candidate per line, the description
     dimmed in a column after the name, the way fish shows them. Without them
     the names pack into columns. */
  if (result->descriptions != NULL) {
    /* The room left for a description after the name column, so a long one
       wraps onto continuation lines instead of running off the terminal. The
       wrap width is held to 80 columns even on a wider terminal so a line stays
       readable, and a little room is kept even when the names are very wide. */
    size_t wrap_cols = tty_cols < 80 ? tty_cols : 80;
    size_t desc_room =
        wrap_cols > column_width + 1 ? wrap_cols - column_width : 20;
    for (i = 0; i < result->count; ++i) {
      const char *name = result->candidates[i];
      const char *desc = result->descriptions[i];
      size_t len = strlen(name);
      size_t pad;
      itl_char_buf_append_cstr(b, name);
      if (desc != NULL && desc[0] != '\0') {
        size_t line_len = 0;
        const char *p = desc;
        for (pad = len; pad < column_width; ++pad) {
          itl_char_buf_append_byte(b, ' ');
        }
        itl_char_buf_append_cstr(b, "\x1b[90m");
        /* One word at a time. A word that no longer fits the line opens a
           continuation line indented under the description column. A word
           wider than the room is emitted whole and overflows rather than
           splitting mid-word. */
        while (*p != '\0') {
          const char *word;
          size_t word_len;
          while (*p == ' ' || *p == '\t')
            p++;
          if (*p == '\0') break;
          word = p;
          while (*p != '\0' && *p != ' ' && *p != '\t')
            p++;
          word_len = (size_t) (p - word);
          if (line_len > 0 && line_len + 1 + word_len > desc_room) {
            size_t k;
            itl_char_buf_append_cstr(b, ITL_LF);
            for (k = 0; k < column_width; ++k)
              itl_char_buf_append_byte(b, ' ');
            line_len = 0;
          }
          if (line_len > 0) {
            itl_char_buf_append_byte(b, ' ');
            line_len += 1;
          }
          {
            size_t k;
            for (k = 0; k < word_len; ++k)
              itl_char_buf_append_byte(b, (uint8_t) word[k]);
          }
          line_len += word_len;
        }
        itl_char_buf_append_cstr(b, ITL_HIGHLIGHT_RESET);
      }
      itl_char_buf_append_cstr(b, ITL_LF);
    }
  } else {
    column = 0;
    for (i = 0; i < result->count; ++i) {
      const char *name = result->candidates[i];
      size_t len = strlen(name);
      size_t pad;

      itl_char_buf_append_cstr(b, name);
      column += 1;
      if (column >= columns || i + 1 == result->count) {
        itl_char_buf_append_cstr(b, ITL_LF);
        column = 0;
      } else {
        for (pad = len; pad < column_width; ++pad) {
          itl_char_buf_append_byte(b, ' ');
        }
      }
    }
  }

  ITL_CHAR_BUF_DUMP(b);
  ITL_CHAR_BUF_CLEAR(b);

  /* The line is redrawn fresh below the list, so forget the old block. */
  itl_g_le_prev_total_rows = 1;
  itl_g_le_prev_cursor_row = 1;
  itl_g_le_prev_cursor_col = 0;
  itl_g_tty_should_refresh_text = true;
}

/* Replace the whole token span [token_start, token_end) with text, in codepoint
   units, leaving the cursor at the end of the inserted text. The span is erased
   first so a mid-word cursor does not keep the bytes to its right, then the
   replacement is inserted at the token start. */
ITL_DEF bool itl_completion_replace_token(itl_le_t *le,
                                          const tl_completion *result,
                                          const char *text)
{
  size_t line_length = le->line->length;
  size_t token_start = result->token_start;
  size_t token_end = result->token_end;
  size_t token_size = 0;
  size_t replacement_size;
  size_t remaining_size;
  size_t position;

  /* A host callback can return a span off the end of the line or inverted, the
     same off-by-one the highlight path validates and drops. Clamp both ends to
     the line and ignore the completion otherwise, so a bad span never traps the
     shift assert inside itl_string_insert. */
  if (token_start > line_length) {
    return false;
  }
  if (token_end > line_length) {
    token_end = line_length;
  }
  if (token_end < token_start) {
    return false;
  }

  for (position = token_start; position < token_end; ++position) {
    token_size += le->line->chars[position].size;
  }
  replacement_size = strlen(text);
  remaining_size = le->line->size - token_size;
  if (remaining_size >= le->out_size ||
      replacement_size >= le->out_size - remaining_size)
  {
    return false;
  }

  {
    size_t token_len = token_end - token_start;

    le->cursor_position = token_start;
    ITL_LE_ERASE_FORWARD(le, token_len);
    return itl_le_insert_cstr(le, text);
  }
}

/* Handle the TAB key when a completion callback is registered. Replace the
   whole token under the cursor with the longest common prefix when that prefix
   extends the token, and when several candidates remain and the prefix did not
   grow, print them below the prompt. A single candidate replaces the token
   outright, since it is the one full replacement. Returns true when it handled
   the key, false when the host should fall back to the default TAB behavior. */
ITL_DEF bool itl_completion_handle_tab(itl_le_t *le)
{
  char line_cstr[ITL_STRING_MAX_LEN];
  tl_completion result;
  size_t token_len, lcp_len;

  memset(&result, 0, sizeof(result));

  if (itl_g_complete_callback == NULL) {
    return false;
  }
  if (itl_string_to_cstr(le->line, line_cstr, sizeof(line_cstr)) != TL_SUCCESS)
  {
    return false;
  }
  int itl_completion_handled =
      itl_g_complete_callback(line_cstr, le->cursor_position, &result, 1);
  itl_ghost_clear();
  itl_g_ghost_sticky_target[0] = '\0';
  if (!itl_completion_handled || result.count == 0) {
    /* A dumb terminal cannot render the repaint the flash draws, so the key is
       still handled but no flash is attempted. */
    if (itl_term_supports_decorations()) {
      itl_g_tty_flash_active = true;
      itl_g_tty_should_refresh_text = true;
      itl_le_tty_refresh(le);
      itl_flash_sleep();
      itl_g_tty_flash_active = false;
      itl_g_tty_should_refresh_text = true;
      itl_le_tty_refresh(le);
    }
    return true;
  }

  /* A lone candidate is the full replacement for the token, so it goes in even
     when it is no longer than what the user typed, which covers a glob token
     that resolves to a single match. */
  if (result.count == 1) {
    itl_completion_replace_token(le, &result, result.candidates[0]);
    itl_g_tty_should_refresh_text = true;
    return true;
  }

  token_len = le->cursor_position >= result.token_start
                  ? le->cursor_position - result.token_start
                  : 0;
  lcp_len = result.longest_common_prefix != NULL
                ? tl_utf8_strlen(result.longest_common_prefix)
                : 0;

  /* Replace the token with the common prefix when that prefix is longer than
     the token, which grows the token toward the candidates. */
  if (lcp_len > token_len) {
    itl_completion_replace_token(le, &result, result.longest_common_prefix);
    itl_g_tty_should_refresh_text = true;
    return true;
  }

  /* The prefix did not grow the token, so a second TAB lists the candidates. */
  itl_completion_print_list(&result);
  return true;
}

ITL_DEF tl_status_code itl_le_key_handle(itl_le_t *le, int esc)
{
  int prev_control = itl_g_last_control;

  /* Remember the last control sequence. */
  itl_g_last_control = esc;

  /* Refresh text by default, avoid if we are only moving the cursor. */
  itl_g_tty_should_refresh_text = true;

  switch (esc & TL_MASK_KEY) {
  case TL_KEY_TAB: {
    /* A registered completion callback handles TAB in place, inserting the
       common prefix or listing the candidates. With no callback, TAB keeps its
       old contract of returning to the host. */
    if (itl_completion_handle_tab(le)) {
      return TL_SUCCESS;
    }
    ITL_TRY(itl_string_to_cstr(le->line, le->out_buf, le->out_size) ==
                TL_SUCCESS,
            return TL_ERROR_SIZE);
    return TL_PRESSED_TAB;
  } break;

  case TL_KEY_UP: {
    itl_le_metrics_t m = itl_le_compute_metrics(le, itl_g_tty_prev_cols);
    bool was_vertical = (prev_control & TL_MASK_KEY) == TL_KEY_UP ||
                        (prev_control & TL_MASK_KEY) == TL_KEY_DOWN;

    /* Move up a visual row while inside a multiline or wrapped buffer. The
       input's first row sits at prompt_rows in the metrics, since a multi-row
       prompt offsets the rows, so history is recalled there rather than at row
       zero, which would be inside the prompt. */
    if (m.cursor_row > le->prompt_rows) {
      if (!was_vertical) {
        le->goal_column = m.cursor_col;
      }
      le->cursor_position = itl_le_index_at_visual(
          le, itl_g_tty_prev_cols, m.cursor_row - 1, le->goal_column);
      itl_g_tty_should_refresh_text = false;
      break;
    }

    /* On the first visual row, navigate to the previous history entry. The
       draft line is saved inside get_prev on the first step up. */
    itl_g_history_get_prev(le);
  } break;

  case TL_KEY_DOWN: {
    itl_le_metrics_t m = itl_le_compute_metrics(le, itl_g_tty_prev_cols);
    bool was_vertical = (prev_control & TL_MASK_KEY) == TL_KEY_UP ||
                        (prev_control & TL_MASK_KEY) == TL_KEY_DOWN;

    /* Move down a visual row while the cursor is not on the last visual row. */
    if (m.cursor_row + 1 < m.total_rows) {
      if (!was_vertical) {
        le->goal_column = m.cursor_col;
      }
      le->cursor_position = itl_le_index_at_visual(
          le, itl_g_tty_prev_cols, m.cursor_row + 1, le->goal_column);
      itl_g_tty_should_refresh_text = false;
      break;
    }

    itl_g_history_get_next(le);
  } break;

  case TL_KEY_RIGHT: {
    bool cursor_was_on_space;
    /* At the end of the line, a Right with ghost text accepts the suggestion by
       inserting it, rather than moving the cursor nowhere. */
    if (le->cursor_position == le->line->length && itl_g_ghost_len > 0 &&
        !(esc & (TL_MOD_CTRL | TL_MOD_ALT)))
    {
      itl_ghost_accept(le);
      itl_ghost_clear();
      itl_g_tty_should_refresh_text = true;
      break;
    }
    if (le->cursor_position < le->line->length) {
      if (esc & (TL_MOD_CTRL | TL_MOD_ALT)) {
        cursor_was_on_space = ITL_LE_CURSOR_IS_ON_SPACE(le);
        itl_le_move_right(le, ITL_LE_STEPS_TO_TOKEN_FORWARD(le));
        if (cursor_was_on_space) {
          itl_le_move_right(le, ITL_LE_STEPS_TO_TOKEN_FORWARD(le));
        }
      } else {
        itl_le_move_right(le, 1);
      }
    }
    itl_g_tty_should_refresh_text = false;
  } break;
  case TL_KEY_LEFT: {
    size_t steps;
    bool cursor_was_on_space;
    if (le->cursor_position > 0 && le->cursor_position <= le->line->length) {
      if (esc & (TL_MOD_CTRL | TL_MOD_ALT)) {
        cursor_was_on_space = (le->cursor_position == le->line->length) ||
                              ITL_LE_CURSOR_IS_ON_SPACE(le);
        steps = ITL_LE_STEPS_TO_TOKEN_BACKWARD(le);
        if (steps > 0) {
          itl_le_move_left(le, steps - 1);
        }
        if (!cursor_was_on_space) {
          itl_le_move_left(le, ITL_LE_STEPS_TO_TOKEN_BACKWARD(le) - 1);
        }
      } else {
        itl_le_move_left(le, 1);
      }
    }
    itl_g_tty_should_refresh_text = false;
  } break;

  case TL_KEY_END: {
    size_t line_end = itl_le_logical_line_end(le);
    /* At the end of the line, End accepts the ghost suggestion the same way
       Right does. */
    if (le->cursor_position == le->line->length && itl_g_ghost_len > 0) {
      itl_ghost_accept(le);
      itl_ghost_clear();
      itl_g_tty_should_refresh_text = true;
      break;
    }
    itl_le_move_right(le, line_end - le->cursor_position);
    itl_g_tty_should_refresh_text = false;
  } break;

  case TL_KEY_HOME: {
    size_t line_start = itl_le_logical_line_start(le);
    itl_le_move_left(le, le->cursor_position - line_start);
    itl_g_tty_should_refresh_text = false;
  } break;
  case TL_KEY_ENTER: {
    bool insert_newline = (esc & TL_MOD_ALT) != 0;

    /* A trailing backslash at the end of the line continues it, fish-style. A
       backslash with text after it submits the line instead. */
    if (!insert_newline && le->cursor_position > 0 &&
        le->cursor_position == le->line->length &&
        ITL_LE_IS_BACKSLASH(le->line->chars[le->cursor_position - 1]))
    {
      insert_newline = true;
    }
    /* A bare newline always submits. Real pastes arrive inside the bracketed
       paste markers requested at raw enter, and a terminal without them
       degrades to one submit per pasted line, the same reading bash gives. A
       pending-input heuristic here would instead merge typed-ahead commands,
       tmux send-keys of "su user" then "cd dir", into one multiline buffer,
       running the second line in the outer shell after the first returns. */
    if (insert_newline) {
      itl_le_insert(le, itl_newline_char);
      break;
    }

    /* Submit. Join backslash continuations before handing the line back. */
    itl_string_join_continuations(le->line);
    if (le->cursor_position > le->line->length) {
      le->cursor_position = le->line->length;
    }
    ITL_TRY(itl_string_to_cstr(le->line, le->out_buf, le->out_size) ==
                TL_SUCCESS,
            return TL_ERROR_SIZE);
    /* Persist the accepted command and return to the draft state. */
    itl_g_last_history_event_number = 0;
    if (itl_g_history_enabled) {
      itl_history_append_to_file(le->line, true, false);
    }
    if (itl_g_history_draft != NULL) {
      itl_string_clear(itl_g_history_draft);
    }
    le->history_selected_index = ITL_HISTORY_NONE;
    return TL_PRESSED_ENTER;
  } break;

  case TL_KEY_BACKSPACE: {
    size_t steps;
    if (esc & TL_MOD_CTRL && le->line->length > 0) {
      steps = ITL_LE_STEPS_TO_TOKEN_BACKWARD(le);
      if (steps > 0) {
        if (le->cursor_position <= steps) {
          steps = le->cursor_position + 1;
        }
        ITL_LE_ERASE_BACKWARD(le, steps - 1);
      }
    } else {
      ITL_LE_ERASE_BACKWARD(le, 1);
    }
  } break;

  case TL_KEY_DELETE: {
    if (esc & TL_MOD_CTRL) {
      ITL_LE_ERASE_FORWARD(le, ITL_LE_STEPS_TO_TOKEN_FORWARD(le));
    } else {
      ITL_LE_ERASE_FORWARD(le, 1);
    }
  } break;

  case TL_KEY_KILL_LINE: {
    ITL_LE_ERASE_FORWARD(le, itl_le_logical_line_end(le) - le->cursor_position);
  } break;

  case TL_KEY_KILL_LINE_BEFORE: {
    ITL_LE_ERASE_BACKWARD(le, le->cursor_position - itl_le_logical_line_start(le));
  } break;

  case TL_KEY_SUSPEND: {
#if defined ITL_SUSPEND
    itl_raise_suspend();
#else
    ITL_TRY(itl_string_to_cstr(le->line, le->out_buf, le->out_size) ==
                TL_SUCCESS,
            {});
    return TL_PRESSED_SUSPEND;
#endif /* ITL_SUSPEND */
  } break;

  case TL_KEY_EOF: {
    if (le->line->length > 0) {
      ITL_LE_ERASE_FORWARD(le, 1);
    } else {
      ITL_TRY(itl_string_to_cstr(le->line, le->out_buf, le->out_size) ==
                  TL_SUCCESS,
              {});
      return TL_PRESSED_EOF;
    }
  } break;

  case TL_KEY_INTERRUPT: {
    ITL_TRY(itl_string_to_cstr(le->line, le->out_buf, le->out_size) ==
                TL_SUCCESS,
            {});
    return TL_PRESSED_INTERRUPT;
  } break;

  case TL_KEY_CLEAR: {
    itl_char_buf_t *b = &itl_g_char_buffer;
    ITL_TTY_GOTO_HOME(b);
    ITL_TTY_ERASE_SCREEN(b);
    ITL_CHAR_BUF_DUMP(b);
    ITL_CHAR_BUF_CLEAR(b);
  } break;

  case TL_KEY_HISTORY_END: {
    /* Jump to the most recent entry. */
    if (itl_g_history_count > 0) {
      itl_history_save_draft(le);
      le->history_selected_index = itl_g_history_count - 1;
      itl_history_show_selected(le);
    }
  } break;

  case TL_KEY_HISTORY_BEGINNING: {
    /* Jump to the oldest navigable entry. */
    if (itl_g_history_count > 0) {
      itl_history_save_draft(le);
      le->history_selected_index = 0;
      itl_history_show_selected(le);
    }
  } break;

  case TL_KEY_UNDO: {
    itl_undo_close_insert_run();
    if (!itl_undo_pop(le)) {
      itl_g_tty_should_refresh_text = false;
    }
  } break;

  case TL_KEY_REDO: {
    if (!itl_redo(le)) {
      itl_g_tty_should_refresh_text = false;
    }
  } break;
  }

  return TL_SUCCESS;
}

/* Inserts one byte of pasted content, turning newlines into newline chars,
   dropping carriage returns and other control bytes, and assembling a UTF-8
   sequence from a lead byte. */
ITL_DEF void itl_le_paste_insert_byte(itl_le_t *le, uint8_t byte)
{
  if (byte == '\r') {
    return; /* The following '\n' produces the break. */
  }
  if (byte == '\n') {
    itl_le_insert(le, itl_newline_char);
    return;
  }
  if (byte != '\t' && (byte < 0x20 || byte == 0x7F)) {
    return; /* Drop other control bytes. */
  }
  itl_le_insert(le, itl_utf8_parse(byte));
}

/* Reads a bracketed paste body up to the ESC [ 201 ~ terminator, inserting its
   content into the line. Multibyte characters are assembled from their own
   bytes instead of `itl_utf8_parse`, so a truncated sequence right before the
   terminator cannot swallow the terminator's ESC and hang. The terminator bytes
   are all ASCII, so they never collide with UTF-8 continuation bytes. */
ITL_DEF void itl_le_read_paste(itl_le_t *le)
{
  static const char end_sequence[] = {0x1B, '[', '2', '0', '1', '~'};
  size_t match = 0;
  uint8_t byte;
  bool have_byte = false;

  while (true) {
    if (!have_byte && !ITL_READ_BYTE(&byte)) {
      return; /* A read error or EOF ends the paste. */
    }
    have_byte = false;

    if (byte == (uint8_t) end_sequence[match]) {
      match += 1;
      if (match == ITL_COUNTOF(end_sequence)) {
        return;
      }
      continue;
    }

    /* The partial match was ordinary content after all. */
    {
      size_t k;
      for (k = 0; k < match; ++k) {
        itl_le_paste_insert_byte(le, (uint8_t) end_sequence[k]);
      }
    }
    match = 0;

    if (byte == (uint8_t) end_sequence[0]) {
      match = 1;
      continue;
    }

    {
      uint8_t rune_width = itl_utf8_width(byte);
      itl_utf8_t ch;
      uint8_t k;

      if (rune_width <= 1) {
        itl_le_paste_insert_byte(le, byte);
        continue;
      }

      /* Read the continuation bytes here. A byte that is not a continuation is
         left for the next iteration so the terminator is never lost. */
      ch.bytes[0] = byte;
      for (k = 1; k < rune_width; ++k) {
        if (!ITL_READ_BYTE(&byte)) {
          return;
        }
        if ((byte & 0xC0) != 0x80) {
          have_byte = true;
          break;
        }
        ch.bytes[k] = byte;
      }
      if (k == rune_width) {
        ch.size = rune_width;
        /* Reject UTF-16 surrogates, matching itl_utf8_parse. */
        if (ITL_UTF8_IS_SURROGATE(ch.bytes[0], ch.bytes[1])) {
          itl_le_insert(le, itl_replacement_character);
        } else {
          itl_le_insert(le, ch);
        }
      }
    }
  }
}

TL_DEF tl_status_code tl_init(void)
{
  TL_ASSERT(!(TL_HISTORY_MAX_SIZE & (TL_HISTORY_MAX_SIZE - 1)) &&
            "History size must be a power of 2");
  TL_ASSERT(TL_HISTORY_MAX_SIZE >= 0 && "History size must be positive");

  if (itl_g_is_active) {
    return TL_SUCCESS;
  }

#if defined ITL_POSIX
  itl_g_prev_sigwinch = signal(SIGWINCH, itl_handle_sigwinch);
#endif

  if (!itl_g_entered_raw_mode) {
    ITL_TRY(ITL_TTY_IS_TTY(), return TL_ERROR);
    ITL_TRY(tl_enter_raw_mode() == TL_SUCCESS, return TL_ERROR);
  }

  itl_string_init(&itl_g_line_buffer);
  itl_char_buf_init(&itl_g_char_buffer);

  itl_g_is_active = true;

  return TL_SUCCESS;
}

TL_DEF tl_status_code tl_exit(void)
{
  TL_ASSERT(itl_g_is_active && "tl_init() should be called");

  /* Restore the terminal before freeing, mirroring tl_init which enters raw
     mode before allocating. A failed restore leaves all state intact and
     tl_exit stays retryable, so a second call never frees the same block. */
  if (itl_g_entered_raw_mode) {
    ITL_TRY(tl_exit_raw_mode() == TL_SUCCESS, return TL_ERROR);
  }

  itl_g_history_free();
  itl_vi_free();
  ITL_FREE(itl_g_line_buffer.chars);
  ITL_FREE(itl_g_char_buffer.data);

  ITL_TRACELN("Exited, alloc count: %zu\n", itl_g_alloc_count);
  TL_ASSERT(itl_g_alloc_count == 0);

  itl_g_is_active = false;

#if defined ITL_POSIX
  signal(SIGWINCH, itl_g_prev_sigwinch);
  itl_g_prev_sigwinch = SIG_DFL;
#endif

  return TL_SUCCESS;
}

/* Walks history backward from start_index toward older entries, reading each
   one from the file into scratch, returning the index of the first that
   contains query as a substring or ITL_HISTORY_NONE when none match. */
ITL_DEF size_t itl_history_find_match(const itl_string_t *query,
                                      size_t start_index, itl_string_t *scratch)
{
  size_t i;
  size_t found = ITL_HISTORY_NONE;

  if (itl_g_history_count == 0 || start_index == ITL_HISTORY_NONE ||
      itl_g_history_path == NULL)
  {
    return ITL_HISTORY_NONE;
  }

  TL_ASSERT(start_index < itl_g_history_count);

  /* One search keystroke can walk every navigable entry, decoded from the
     in-memory file buffer rather than a read per entry. */
  if (!itl_history_ensure_read_buffer()) {
    return ITL_HISTORY_NONE;
  }

  /* Count down from start_index to zero inclusive without underflowing. */
  for (i = start_index + 1; i-- > 0;) {
    if (itl_history_read_entry_buffered(itl_history_index_to_offset(i),
                                        scratch) &&
        itl_string_find_substring_ascii_casefold(scratch, query))
    {
      found = i;
      break;
    }
  }

  return found;
}

/* Walks history forward from start_index toward newer entries, returning the
   index of the first that contains query as a substring or ITL_HISTORY_NONE
   when none match. The mirror of itl_history_find_match. */
ITL_DEF size_t itl_history_find_match_forward(const itl_string_t *query,
                                              size_t start_index,
                                              itl_string_t *scratch)
{
  size_t i;
  size_t found = ITL_HISTORY_NONE;

  if (itl_g_history_count == 0 || start_index == ITL_HISTORY_NONE ||
      start_index >= itl_g_history_count || itl_g_history_path == NULL)
  {
    return ITL_HISTORY_NONE;
  }

  if (!itl_history_ensure_read_buffer()) {
    return ITL_HISTORY_NONE;
  }

  for (i = start_index; i < itl_g_history_count; ++i) {
    if (itl_history_read_entry_buffered(itl_history_index_to_offset(i),
                                        scratch) &&
        itl_string_find_substring_ascii_casefold(scratch, query))
    {
      found = i;
      break;
    }
  }

  return found;
}

/* The newest navigable entry index, or ITL_HISTORY_NONE when history is empty.
 */
#define ITL_HISTORY_NEWEST()                                                   \
  (itl_g_history_count > 0 ? itl_g_history_count - 1 : ITL_HISTORY_NONE)

#define ITL_SEARCH_SGR_GREEN  "\x1b[32m"
#define ITL_SEARCH_SGR_YELLOW "\x1b[33m"
#define ITL_SEARCH_SGR_BOLD   "\x1b[1m"

/* Records one highlight span for the reverse search block, dropped once the
   span array is full. The start and end are codepoint indices into the display
   buffer, and the sgr is a static escape so the pointer stays valid until the
   refresh draws it. */
ITL_DEF void itl_search_push_span(size_t start, size_t end, const char *sgr)
{
  if (start < end && itl_g_search_span_count < ITL_HIGHLIGHT_MAX_SPANS) {
    itl_g_search_spans[itl_g_search_span_count].start = start;
    itl_g_search_spans[itl_g_search_span_count].end = end;
    itl_g_search_spans[itl_g_search_span_count].sgr = sgr;
    itl_g_search_span_count++;
  }
}

/* Appends a guide segment to the status buffer and bolds a key token. Returns
   the codepoint position past the segment. The guide is ASCII, so a byte is one
   codepoint. */
ITL_DEF size_t itl_search_append_guide(itl_char_buf_t *status, size_t position,
                                       const char *text, bool is_key)
{
  size_t text_length = strlen(text);

  itl_char_buf_append_cstr(status, text);
  if (is_key) {
    itl_search_push_span(position, position + text_length, ITL_SEARCH_SGR_BOLD);
  }

  return position + text_length;
}

/* Runs a reverse incremental history search. The live prompt stays on screen
   and the matched entry, the search term, and the hint are drawn below it as
   one multiline buffer swapped into the line editor. The block carries its own
   highlight spans. Returns the control key that ended the search so the caller
   can re-dispatch it, or TL_KEY_UNKN when the search was cancelled or accepted
   with no further action. */
ITL_DEF int itl_history_search(itl_le_t *le)
{
  itl_string_t *original = itl_string_alloc();
  itl_string_t *query = itl_string_alloc();
  itl_string_t *display = itl_string_alloc();
  itl_string_t *match_str = itl_string_alloc();
  itl_string_t *scratch = itl_string_alloc();
  itl_char_buf_t *status = itl_char_buf_alloc();

  /* Index of the matched entry, ITL_HISTORY_NONE while nothing matches. The
     matched entry text is kept in match_str for the preview. */
  size_t match = ITL_HISTORY_NONE;

  const char *saved_prompt = le->prompt;
  size_t saved_prompt_size = le->prompt_size;
  size_t saved_prompt_width = le->prompt_width;
  size_t saved_prompt_rows = le->prompt_rows;

  int result = TL_KEY_UNKN;
  bool accepted = false;
  /* When search starts on the draft, the current line is the user's draft and
     must be saved so a later step past the newest entry can restore it. */
  bool was_on_draft = (le->history_selected_index == ITL_HISTORY_NONE);
  uint8_t byte;

  itl_string_copy(original, le->line);

  /* A draft already typed becomes the initial query, so it moves into the
     search term instead of staying on the prompt line. */
  if (le->line->length > 0) {
    itl_string_copy(query, le->line);
    match = itl_history_find_match(query, ITL_HISTORY_NEWEST(), scratch);
    if (match != ITL_HISTORY_NONE) {
      itl_string_copy(match_str, scratch);
    }
  }

  while (true) {
    /* The matched entry shares line one with the live prompt, then the search
       term and the hint follow on their own rows. The whole block is one
       multiline buffer drawn through the normal refresh. */
    const itl_string_t *preview =
        (match != ITL_HISTORY_NONE) ? match_str : original;
    size_t tty_cols = ITL_MAX(itl_g_tty_prev_cols, 1);
    size_t budget = (tty_cols > saved_prompt_width + 1)
                        ? tty_cols - saved_prompt_width - 1
                        : tty_cols;
    char match_render[ITL_STRING_MAX_LEN];
    size_t match_bytes = 0;
    size_t match_length = 0;
    size_t match_width = 0;
    size_t line2_start, query_start, line3_start, guide_position;
    size_t pi, pj;

    /* Flatten newlines to spaces and clip to the prompt's row remainder so the
       match never wraps under the prompt. */
    for (pi = 0; pi < preview->length; ++pi) {
      itl_utf8_t pch = preview->chars[pi];
      bool is_newline = ITL_LE_IS_NEWLINE(pch);
      size_t char_width = is_newline ? 1 : itl_char_width(pch);

      if (match_width + char_width > budget) {
        break;
      }
      if (match_bytes + pch.size >= ITL_STRING_MAX_LEN) {
        break;
      }

      if (is_newline) {
        match_render[match_bytes++] = ' ';
      } else {
        for (pj = 0; pj < pch.size; ++pj) {
          match_render[match_bytes++] = (char) pch.bytes[pj];
        }
      }

      match_length += 1;
      match_width += char_width;
    }
    match_render[match_bytes] = '\0';

    itl_g_search_span_count = 0;

    /* Line one, the matched entry highlighted as the command it would become.
       The match sits at offset zero, so the host's codepoint spans index the
       display buffer unchanged. */
    if (itl_g_highlight_callback != NULL) {
      tl_highlight_span match_spans[ITL_HIGHLIGHT_MAX_SPANS];
      tl_highlight hl;
      hl.spans = match_spans;
      hl.count = 0;
      hl.capacity = ITL_HIGHLIGHT_MAX_SPANS;
      if (itl_g_highlight_callback(match_render, &hl)) {
        size_t s;
        for (s = 0; s < hl.count && s < ITL_HIGHLIGHT_MAX_SPANS; ++s) {
          itl_search_push_span(match_spans[s].start, match_spans[s].end,
                               match_spans[s].sgr);
        }
      }
    }

    ITL_CHAR_BUF_CLEAR(status);
    itl_char_buf_append_cstr(status, match_render);
    itl_char_buf_append_byte(status, '\n');

    /* Line two, the search label in green and the typed query in yellow. The
       label `(incremental search)` is 20 codepoints and the trailing ` '` is
       two more. */
    line2_start = match_length + 1;
    itl_char_buf_append_cstr(status, "(incremental search) '");
    itl_search_push_span(line2_start, line2_start + 20, ITL_SEARCH_SGR_GREEN);
    query_start = line2_start + 22;
    if (query->length > 0) {
      itl_char_buf_append_string(status, query);
      itl_search_push_span(query_start, query_start + query->length,
                           ITL_SEARCH_SGR_YELLOW);
    }
    itl_char_buf_append_byte(status, '\'');
    itl_char_buf_append_byte(status, '\n');

    /* Line three, the hint with its key tokens bolded. */
    line3_start = query_start + query->length + 2;
    guide_position = line3_start;
    guide_position =
        itl_search_append_guide(status, guide_position, "enter", true);
    guide_position =
        itl_search_append_guide(status, guide_position, " accept   ", false);
    guide_position =
        itl_search_append_guide(status, guide_position, "ctrl-r", true);
    guide_position =
        itl_search_append_guide(status, guide_position, "/", false);
    guide_position =
        itl_search_append_guide(status, guide_position, "ctrl-f", true);
    guide_position = itl_search_append_guide(status, guide_position,
                                             " back/forward   ", false);
    guide_position =
        itl_search_append_guide(status, guide_position, "ctrl-g", true);
    (void) itl_search_append_guide(status, guide_position, " cancel", false);

    itl_string_from_bytes(display, status->data, status->size);

    /* The prompt stays drawn with its rows counted, so the block row math
       starts from the prompt's trailing row exactly as the normal render path.
       Only the width is zeroed so the indent is zero and the term and hint rows
       sit flush left under the prompt. */
    le->prompt_width = 0;
    le->line = display;
    le->cursor_position = query_start + query->length;
    itl_g_search_spans_active = true;
    itl_g_tty_should_refresh_text = true;
    itl_le_tty_refresh(le);

    if (!ITL_READ_BYTE(&byte)) {
      break;
    }

    {
      int key = itl_esc_parse(byte);
      int kind = key & TL_MASK_KEY;

      if (byte == 6) {
        /* Ctrl-F steps forward to a newer match, the mirror of ctrl-r. It is
           caught by raw byte so the arrow keys keep their accept behavior. */
        size_t from = (match != ITL_HISTORY_NONE) ? match + 1 : 0;
        size_t next = itl_history_find_match_forward(query, from, scratch);
        if (next != ITL_HISTORY_NONE) {
          match = next;
          itl_string_copy(match_str, scratch);
        }
      } else if (kind == TL_KEY_CHAR) {
        size_t found;
        itl_string_insert(query, query->length, itl_utf8_parse(byte));
        found = itl_history_find_match(query, ITL_HISTORY_NEWEST(), scratch);
        match = found;
        if (found != ITL_HISTORY_NONE) {
          itl_string_copy(match_str, scratch);
        }
      } else if (kind == TL_KEY_HISTORY_SEARCH) {
        /* Search further back, starting just past the current match. */
        size_t from = (match != ITL_HISTORY_NONE)
                          ? (match > 0 ? match - 1 : ITL_HISTORY_NONE)
                          : ITL_HISTORY_NEWEST();
        size_t next = itl_history_find_match(query, from, scratch);
        if (next != ITL_HISTORY_NONE) {
          match = next;
          itl_string_copy(match_str, scratch);
        }
      } else if (kind == TL_KEY_BACKSPACE) {
        if (query->length > 0) {
          itl_string_erase(query, query->length, 1, true);
          if (query->length > 0) {
            size_t found =
                itl_history_find_match(query, ITL_HISTORY_NEWEST(), scratch);
            match = found;
            if (found != ITL_HISTORY_NONE) {
              itl_string_copy(match_str, scratch);
            }
          } else {
            match = ITL_HISTORY_NONE;
          }
        }
      } else if (kind == TL_KEY_UNKN || kind == TL_KEY_INTERRUPT ||
                 kind == TL_KEY_EOF || kind == TL_KEY_SUSPEND)
      {
        /* Ctrl-G, escape, and the interrupt keys cancel and restore the
           original line. The interrupt keys are still re-dispatched so they act
           on that line rather than on the matched entry. */
        if (kind != TL_KEY_UNKN) {
          result = key;
        }
        break;
      } else {
        accepted = true;
        result = (kind == TL_KEY_ENTER) ? TL_KEY_UNKN : key;
        break;
      }
    }
  }

  /* Restore the real prompt width and line editor buffer, and stop drawing the
     search spans so the next refresh highlights the line as a command again. */
  itl_g_search_spans_active = false;
  le->prompt = saved_prompt;
  le->prompt_size = saved_prompt_size;
  le->prompt_width = saved_prompt_width;
  le->prompt_rows = saved_prompt_rows;
  le->line = &itl_g_line_buffer;

  if (accepted && match != ITL_HISTORY_NONE) {
    /* Preserve the pre-search draft so stepping down past the newest entry
       restores it rather than a stale draft. */
    if (was_on_draft) {
      if (itl_g_history_draft == NULL) {
        itl_g_history_draft = itl_string_alloc();
      }
      itl_string_copy(itl_g_history_draft, original);
    }
    /* A match longer than the host buffer would fail itl_string_to_cstr on
       submit and abort the read, so fall back to the pre-search line when the
       match does not fit. The size plus the null terminator must stay within
       out_size. */
    if (match_str->size + 1 > le->out_size) {
      itl_string_copy(le->line, original);
    } else {
      itl_string_copy(le->line, match_str);
      le->history_selected_index = match;
    }
  } else {
    itl_string_copy(le->line, original);
  }
  le->cursor_position = le->line->length;

  ITL_STRING_FREE(original);
  ITL_STRING_FREE(query);
  ITL_STRING_FREE(display);
  ITL_STRING_FREE(match_str);
  ITL_STRING_FREE(scratch);
  ITL_CHAR_BUF_FREE(status);

  return result;
}

ITL_DEF size_t itl_vi_register_index(char name)
{
  if (name >= 'a' && name <= 'z') {
    return (size_t) (name - 'a');
  }
  if (name >= 'A' && name <= 'Z') {
    return (size_t) (name - 'A');
  }

  return ITL_VI_REGISTER_UNNAMED;
}

ITL_DEF itl_string_t *itl_vi_register_at(size_t index)
{
  if (itl_g_vi_registers[index] == NULL) {
    itl_g_vi_registers[index] = itl_string_alloc();
  }

  return itl_g_vi_registers[index];
}

ITL_DEF void itl_vi_register_set_span(itl_string_t *reg,
                                      const itl_string_t *src, size_t from,
                                      size_t to)
{
  size_t i;

  itl_string_clear(reg);
  for (i = from; i < to && i < src->length; ++i) {
    itl_string_insert(reg, reg->length, src->chars[i]);
  }
}

ITL_DEF int itl_vi_char_class(itl_utf8_t ch, bool is_big_word)
{
  uint8_t b = ch.bytes[0];

  if (ch.size == 1 && isspace(b)) {
    return 0;
  }
  if (is_big_word) {
    return 1;
  }
  if (ch.size > 1 || isalnum(b) || b == '_') {
    return 1;
  }

  return 2;
}

ITL_DEF size_t itl_vi_word_forward(const itl_string_t *str, size_t from,
                                   bool is_big_word)
{
  size_t i = from;
  int start_class;

  if (i >= str->length) {
    return str->length;
  }

  start_class = itl_vi_char_class(str->chars[i], is_big_word);

  if (start_class != 0) {
    while (i < str->length &&
           itl_vi_char_class(str->chars[i], is_big_word) == start_class)
    {
      i += 1;
    }
  }

  while (i < str->length && itl_vi_char_class(str->chars[i], is_big_word) == 0)
  {
    i += 1;
  }

  return i;
}

ITL_DEF size_t itl_vi_word_end(const itl_string_t *str, size_t from,
                               bool is_big_word)
{
  size_t i = from;
  int cls;

  if (str->length == 0) {
    return 0;
  }
  if (i >= str->length - 1) {
    return str->length - 1;
  }

  i += 1;
  while (i < str->length && itl_vi_char_class(str->chars[i], is_big_word) == 0)
  {
    i += 1;
  }
  if (i >= str->length) {
    return str->length - 1;
  }

  cls = itl_vi_char_class(str->chars[i], is_big_word);
  while (i + 1 < str->length &&
         itl_vi_char_class(str->chars[i + 1], is_big_word) == cls)
  {
    i += 1;
  }

  return i;
}

ITL_DEF size_t itl_vi_word_back(const itl_string_t *str, size_t from,
                                bool is_big_word)
{
  size_t i = from;
  int cls;

  if (i == 0) {
    return 0;
  }

  i -= 1;
  while (i > 0 && itl_vi_char_class(str->chars[i], is_big_word) == 0) {
    i -= 1;
  }
  if (itl_vi_char_class(str->chars[i], is_big_word) == 0) {
    return i;
  }

  cls = itl_vi_char_class(str->chars[i], is_big_word);
  while (i > 0 && itl_vi_char_class(str->chars[i - 1], is_big_word) == cls) {
    i -= 1;
  }

  return i;
}

ITL_DEF size_t itl_vi_find_char(const itl_string_t *str, size_t from,
                                itl_utf8_t target, bool is_forward,
                                bool is_till, size_t count, bool *is_found)
{
  size_t i = from;
  size_t remaining = count;

  ITL_PTR_ASSIGN(is_found, false);

  while (remaining > 0) {
    if (is_forward) {
      i += 1;
      while (i < str->length && !itl_utf8_equal(str->chars[i], target)) {
        i += 1;
      }
      if (i >= str->length) {
        return from;
      }
    } else {
      if (i == 0) {
        return from;
      }
      i -= 1;
      while (i > 0 && !itl_utf8_equal(str->chars[i], target)) {
        i -= 1;
      }
      if (!itl_utf8_equal(str->chars[i], target)) {
        return from;
      }
    }
    remaining -= 1;
  }

  ITL_PTR_ASSIGN(is_found, true);

  if (is_till) {
    i = is_forward ? i - 1 : i + 1;
  }

  return i;
}

ITL_DEF void itl_vi_clamp_command_cursor(itl_le_t *le)
{
  if (itl_g_edit_mode == TL_EDIT_MODE_VI_COMMAND && le->line->length > 0 &&
      le->cursor_position >= le->line->length)
  {
    le->cursor_position = le->line->length - 1;
  }
}

ITL_DEF size_t itl_vi_resolve_motion(itl_le_t *le, int motion_key,
                                     itl_utf8_t find_char, size_t count,
                                     bool is_for_operator, bool *is_inclusive,
                                     bool *is_valid)
{
  itl_string_t *line = le->line;
  size_t pos = le->cursor_position;
  size_t target = pos;
  size_t k;
  bool found;

  ITL_PTR_ASSIGN(is_inclusive, false);
  ITL_PTR_ASSIGN(is_valid, true);

  switch (motion_key) {
  case 'h': target = (count <= pos) ? pos - count : 0; break;

  case 'l':
  case ' ':
    target = pos + count;
    if (target > line->length) {
      target = line->length;
    }
    if (!is_for_operator && line->length > 0 && target >= line->length) {
      target = line->length - 1;
    }
    break;

  case '0': target = itl_le_logical_line_start(le); break;

  case '$':
    target = itl_le_logical_line_end(le);
    if (!is_for_operator && target > itl_le_logical_line_start(le)) {
      target -= 1;
    }
    break;

  case '^': {
    size_t i = itl_le_logical_line_start(le);
    while (i < line->length && !ITL_LE_IS_NEWLINE(line->chars[i]) &&
           itl_vi_char_class(line->chars[i], false) == 0)
    {
      i += 1;
    }
    target = i;
  } break;

  case 'w':
  case 'W':
    target = pos;
    for (k = 0; k < count; ++k) {
      target = itl_vi_word_forward(line, target, motion_key == 'W');
    }
    break;

  case 'b':
  case 'B':
    target = pos;
    for (k = 0; k < count; ++k) {
      target = itl_vi_word_back(line, target, motion_key == 'B');
    }
    break;

  case 'e':
  case 'E':
    target = pos;
    for (k = 0; k < count; ++k) {
      target = itl_vi_word_end(line, target, motion_key == 'E');
    }
    ITL_PTR_ASSIGN(is_inclusive, true);
    break;

  case 'f':
  case 'F':
  case 't':
  case 'T': {
    bool is_forward = (motion_key == 'f' || motion_key == 't');
    bool is_till = (motion_key == 't' || motion_key == 'T');

    target = itl_vi_find_char(line, pos, find_char, is_forward, is_till, count,
                              &found);
    if (!found) {
      ITL_PTR_ASSIGN(is_valid, false);
    } else if (is_forward) {
      ITL_PTR_ASSIGN(is_inclusive, true);
    }

    itl_g_vi_find.target_char = find_char;
    itl_g_vi_find.is_forward = is_forward;
    itl_g_vi_find.is_till = is_till;
    itl_g_vi_find.has_pending = true;
  } break;

  case ';':
  case ',': {
    bool is_forward;

    if (!itl_g_vi_find.has_pending) {
      ITL_PTR_ASSIGN(is_valid, false);
      break;
    }

    is_forward = itl_g_vi_find.is_forward;
    if (motion_key == ',') {
      is_forward = !is_forward;
    }

    target = itl_vi_find_char(line, pos, itl_g_vi_find.target_char, is_forward,
                              itl_g_vi_find.is_till, count, &found);
    if (!found) {
      ITL_PTR_ASSIGN(is_valid, false);
    } else if (is_forward) {
      ITL_PTR_ASSIGN(is_inclusive, true);
    }
  } break;

  default: ITL_PTR_ASSIGN(is_valid, false); break;
  }

  return target;
}

ITL_DEF void itl_vi_apply_operator(itl_le_t *le, itl_vi_operator_kind op,
                                   size_t from, size_t to, bool is_inclusive)
{
  size_t start = (from <= to) ? from : to;
  size_t end = (from <= to) ? to : from;
  itl_string_t *reg;
  size_t span_count;

  if (is_inclusive && end < le->line->length) {
    end += 1;
  }

  if (start == end) {
    if (op == ITL_VI_OP_CHANGE) {
      itl_g_edit_mode = TL_EDIT_MODE_VI_INSERT;
    }
    return;
  }

  {
    size_t reg_index = itl_vi_register_index(itl_g_vi_pending_register);
    reg = itl_vi_register_at(reg_index);
    itl_vi_register_set_span(reg, le->line, start, end);
    itl_g_vi_register_is_linewise[reg_index] = false;
  }
  if (itl_g_vi_pending_register != 0) {
    itl_vi_register_set_span(itl_vi_register_at(ITL_VI_REGISTER_UNNAMED),
                             le->line, start, end);
    itl_g_vi_register_is_linewise[ITL_VI_REGISTER_UNNAMED] = false;
  }

  span_count = end - start;
  le->cursor_position = start;

  if (op == ITL_VI_OP_YANK) {
    return;
  }

  ITL_LE_ERASE_FORWARD(le, span_count);

  if (op == ITL_VI_OP_CHANGE) {
    itl_g_edit_mode = TL_EDIT_MODE_VI_INSERT;
  }
}

ITL_DEF void itl_vi_record_operator(itl_vi_operator_kind op, int motion_key,
                                    itl_utf8_t find_char, size_t count)
{
  itl_g_vi_last_change.kind = ITL_VI_CHANGE_OPERATOR;
  itl_g_vi_last_change.operator_kind = op;
  itl_g_vi_last_change.motion_key = motion_key;
  itl_g_vi_last_change.find_char = find_char;
  itl_g_vi_last_change.repeat_count = count;
  itl_g_vi_last_change.did_enter_insert = (op == ITL_VI_OP_CHANGE);

  if (op != ITL_VI_OP_CHANGE && itl_g_vi_last_change.inserted_text != NULL) {
    itl_string_clear(itl_g_vi_last_change.inserted_text);
  }
}

ITL_DEF void itl_vi_start_insert_recording(void)
{
  if (itl_g_vi_last_change.inserted_text == NULL) {
    itl_g_vi_last_change.inserted_text = itl_string_alloc();
  } else {
    itl_string_clear(itl_g_vi_last_change.inserted_text);
  }

  itl_g_vi_is_recording_insert = true;
  itl_g_undo_insert_run_open = false;
}

ITL_DEF void itl_vi_begin_insert(bool should_record, int entry_key)
{
  itl_g_edit_mode = TL_EDIT_MODE_VI_INSERT;
  itl_g_undo_insert_run_open = false;

  if (should_record) {
    itl_g_vi_last_change.kind = ITL_VI_CHANGE_INSERT;
    itl_g_vi_last_change.motion_key = entry_key;
    itl_g_vi_last_change.repeat_count = 1;
    itl_g_vi_last_change.did_enter_insert = true;
    itl_vi_start_insert_recording();
  }
}

ITL_DEF void itl_vi_operator_motion(itl_le_t *le, itl_vi_operator_kind op,
                                    int motion_key, itl_utf8_t find_char,
                                    size_t count)
{
  bool is_inclusive, is_valid;
  size_t target;

  if (op == ITL_VI_OP_CHANGE && le->cursor_position < le->line->length &&
      itl_vi_char_class(le->line->chars[le->cursor_position], false) != 0)
  {
    if (motion_key == 'w') {
      motion_key = 'e';
    } else if (motion_key == 'W') {
      motion_key = 'E';
    }
  }

  target = itl_vi_resolve_motion(le, motion_key, find_char, count, true,
                                 &is_inclusive, &is_valid);

  if (!is_valid) {
    return;
  }

  itl_vi_record_operator(op, motion_key, find_char, count);
  itl_vi_apply_operator(le, op, le->cursor_position, target, is_inclusive);

  if (op == ITL_VI_OP_CHANGE) {
    itl_vi_start_insert_recording();
  }
}

ITL_DEF void itl_vi_operator_line(itl_le_t *le, itl_vi_operator_kind op,
                                  int doubled_key, size_t count)
{
  itl_utf8_t none = ITL_ZERO_INIT;
  size_t start = itl_le_logical_line_start(le);
  size_t end = itl_le_logical_line_end(le);
  size_t reg_index = itl_vi_register_index(itl_g_vi_pending_register);
  itl_string_t *reg = itl_vi_register_at(reg_index);
  size_t i;

  for (i = 1; i < count; ++i) {
    if (end >= le->line->length) {
      break;
    }
    end += 1;
    while (end < le->line->length && !ITL_LE_IS_NEWLINE(le->line->chars[end])) {
      end += 1;
    }
  }

  itl_vi_record_operator(op, doubled_key, none, count);

  itl_vi_register_set_span(reg, le->line, start, end);
  itl_string_insert(reg, reg->length, itl_newline_char);
  itl_g_vi_register_is_linewise[reg_index] = true;

  if (itl_g_vi_pending_register != 0) {
    itl_string_t *unnamed = itl_vi_register_at(ITL_VI_REGISTER_UNNAMED);
    itl_vi_register_set_span(unnamed, le->line, start, end);
    itl_string_insert(unnamed, unnamed->length, itl_newline_char);
    itl_g_vi_register_is_linewise[ITL_VI_REGISTER_UNNAMED] = true;
  }

  if (op == ITL_VI_OP_YANK) {
    le->cursor_position = start;
    itl_vi_clamp_command_cursor(le);
    return;
  }

  if (op == ITL_VI_OP_CHANGE) {
    le->cursor_position = start;
    ITL_LE_ERASE_FORWARD(le, end - start);
    itl_g_edit_mode = TL_EDIT_MODE_VI_INSERT;
    itl_vi_start_insert_recording();
    return;
  }

  {
    size_t erase_start = start;
    size_t erase_end = end;

    if (end < le->line->length && ITL_LE_IS_NEWLINE(le->line->chars[end])) {
      erase_end = end + 1;
    } else if (start > 0 && ITL_LE_IS_NEWLINE(le->line->chars[start - 1])) {
      erase_start = start - 1;
    }

    le->cursor_position = erase_start;
    ITL_LE_ERASE_FORWARD(le, erase_end - erase_start);
    itl_vi_clamp_command_cursor(le);
  }
}

ITL_DEF void itl_vi_apply_bare_motion(itl_le_t *le, int motion_key,
                                      itl_utf8_t find_char, size_t count)
{
  bool is_inclusive, is_valid;
  size_t target = itl_vi_resolve_motion(le, motion_key, find_char, count, false,
                                        &is_inclusive, &is_valid);

  if (is_valid) {
    le->cursor_position = target;
  }

  itl_vi_clamp_command_cursor(le);
}

ITL_DEF void itl_vi_do_replace(itl_le_t *le, itl_utf8_t ch, size_t count)
{
  size_t i;

  if (le->cursor_position + count > le->line->length) {
    return;
  }

  itl_undo_push(le);
  itl_g_undo_insert_run_open = false;

  for (i = 0; i < count; ++i) {
    itl_string_erase(le->line, le->cursor_position + i, 1, false);
    itl_string_insert(le->line, le->cursor_position + i, ch);
  }

  le->cursor_position += count - 1;
}

ITL_DEF void itl_vi_do_tilde(itl_le_t *le, size_t count)
{
  size_t i;

  /* A no-op tilde on an empty line or at the line end changes nothing, so it
     must not push an undo snapshot and clear the redo stack. */
  if (le->cursor_position >= le->line->length) {
    return;
  }

  itl_undo_push(le);
  itl_g_undo_insert_run_open = false;

  for (i = 0; i < count && le->cursor_position < le->line->length; ++i) {
    itl_utf8_t *ch = &le->line->chars[le->cursor_position];
    if (ch->size == 1) {
      uint8_t b = ch->bytes[0];
      if (isupper(b)) {
        ch->bytes[0] = (uint8_t) tolower(b);
      } else if (islower(b)) {
        ch->bytes[0] = (uint8_t) toupper(b);
      }
    }
    itl_le_move_right(le, 1);
  }

  itl_vi_clamp_command_cursor(le);
}

ITL_DEF void itl_vi_paste_lines(itl_le_t *le, const char *line_text,
                                bool is_before, size_t count)
{
  size_t first;
  size_t i;

  if (is_before) {
    first = itl_le_logical_line_start(le);
    le->cursor_position = first;
    for (i = 0; i < count; ++i) {
      itl_le_insert_cstr(le, line_text);
    }
    le->cursor_position = first;
    return;
  }

  {
    size_t line_end = itl_le_logical_line_end(le);

    if (line_end < le->line->length) {
      first = line_end + 1;
      le->cursor_position = first;
      for (i = 0; i < count; ++i) {
        itl_le_insert_cstr(le, line_text);
      }
      le->cursor_position = first;
      return;
    }

    le->cursor_position = line_end;
    itl_le_insert_cstr(le, "\n");
    first = le->cursor_position;
    for (i = 0; i < count; ++i) {
      itl_le_insert_cstr(le, line_text);
    }
    if (le->line->length > 0 &&
        ITL_LE_IS_NEWLINE(le->line->chars[le->line->length - 1]))
    {
      itl_string_erase(le->line, le->line->length, 1, true);
    }
    le->cursor_position = first;
  }
}

ITL_DEF void itl_vi_do_paste(itl_le_t *le, bool is_before, size_t count)
{
  size_t reg_index = itl_vi_register_index(itl_g_vi_pending_register);
  itl_string_t *reg = itl_vi_register_at(reg_index);
  bool is_linewise = itl_g_vi_register_is_linewise[reg_index];
  char text[ITL_STRING_MAX_LEN];
  size_t i;

  if (reg->length == 0) {
    return;
  }
  if (itl_string_to_cstr(reg, text, sizeof(text)) != TL_SUCCESS) {
    return;
  }

  itl_undo_push(le);
  itl_g_undo_insert_run_open = false;

  if (is_linewise) {
    itl_vi_paste_lines(le, text, is_before, count);
    return;
  }

  if (!is_before && le->line->length > 0) {
    itl_le_move_right(le, 1);
  }

  for (i = 0; i < count; ++i) {
    itl_le_insert_cstr(le, text);
  }

  if (le->cursor_position > 0) {
    le->cursor_position -= 1;
  }
}

ITL_DEF tl_status_code itl_vi_repeat_last_change(itl_le_t *le)
{
  itl_vi_change_kind kind = itl_g_vi_last_change.kind;
  size_t count = (itl_g_vi_last_change.repeat_count == 0)
                     ? 1
                     : itl_g_vi_last_change.repeat_count;
  char text[ITL_STRING_MAX_LEN];

  itl_g_vi_is_recording_insert = false;

  switch (kind) {
  case ITL_VI_CHANGE_INSERT:
    switch (itl_g_vi_last_change.motion_key) {
    case 'a':
      if (le->line->length > 0) {
        itl_le_move_right(le, 1);
      }
      break;
    case 'A': le->cursor_position = itl_le_logical_line_end(le); break;
    case 'I': {
      itl_utf8_t none = ITL_ZERO_INIT;
      bool is_inclusive, is_valid;
      le->cursor_position = itl_vi_resolve_motion(le, '^', none, 1, false,
                                                  &is_inclusive, &is_valid);
    } break;
    default: break;
    }

    if (itl_g_vi_last_change.inserted_text != NULL &&
        itl_string_to_cstr(itl_g_vi_last_change.inserted_text, text,
                           sizeof(text)) == TL_SUCCESS)
    {
      itl_le_insert_cstr(le, text);
    }
    break;

  case ITL_VI_CHANGE_OPERATOR: {
    itl_vi_operator_kind op =
        (itl_vi_operator_kind) itl_g_vi_last_change.operator_kind;
    int motion_key = itl_g_vi_last_change.motion_key;
    bool is_doubled =
        (motion_key == 'd' || motion_key == 'c' || motion_key == 'y');
    bool did_apply = false;
    bool have_insert_text =
        itl_g_vi_last_change.did_enter_insert &&
        itl_g_vi_last_change.inserted_text != NULL &&
        itl_string_to_cstr(itl_g_vi_last_change.inserted_text, text,
                           sizeof(text)) == TL_SUCCESS;

    itl_g_vi_pending_register = 0;

    if (is_doubled) {
      itl_vi_operator_line(le, op, motion_key, count);
      did_apply = true;
    } else {
      bool is_inclusive, is_valid;
      size_t target =
          itl_vi_resolve_motion(le, motion_key, itl_g_vi_last_change.find_char,
                                count, true, &is_inclusive, &is_valid);
      if (is_valid) {
        itl_vi_apply_operator(le, op, le->cursor_position, target,
                              is_inclusive);
        did_apply = true;
      }
    }

    itl_g_vi_is_recording_insert = false;

    if (did_apply && have_insert_text) {
      itl_le_insert_cstr(le, text);
    }
    if (did_apply && itl_g_vi_last_change.did_enter_insert) {
      itl_g_edit_mode = TL_EDIT_MODE_VI_COMMAND;
    }
  } break;

  case ITL_VI_CHANGE_REPLACE:
    itl_vi_do_replace(le, itl_g_vi_last_change.replace_char, count);
    break;

  case ITL_VI_CHANGE_TILDE: itl_vi_do_tilde(le, count); break;

  case ITL_VI_CHANGE_PASTE:
    itl_g_vi_pending_register = 0;
    itl_vi_do_paste(le, itl_g_vi_last_change.is_paste_before, count);
    break;

  default: break;
  }

  itl_vi_clamp_command_cursor(le);
  return TL_SUCCESS;
}

ITL_DEF void itl_vi_step_visual_row(itl_le_t *le, bool is_up)
{
  itl_le_metrics_t m = itl_le_compute_metrics(le, itl_g_tty_prev_cols);

  if (is_up && m.cursor_row > le->prompt_rows) {
    le->cursor_position = itl_le_index_at_visual(
        le, itl_g_tty_prev_cols, m.cursor_row - 1, m.cursor_col);
  } else if (!is_up && m.cursor_row + 1 < m.total_rows) {
    le->cursor_position = itl_le_index_at_visual(
        le, itl_g_tty_prev_cols, m.cursor_row + 1, m.cursor_col);
  }
}

ITL_DEF tl_status_code itl_vi_visual_loop(itl_le_t *le, bool is_linewise)
{
  uint8_t byte;

  itl_g_vi_visual_anchor = le->cursor_position;
  itl_g_edit_mode = TL_EDIT_MODE_VI_VISUAL;

  while (true) {
    size_t selection_start =
        ITL_MIN(itl_g_vi_visual_anchor, le->cursor_position);
    size_t selection_end = ITL_MAX(itl_g_vi_visual_anchor, le->cursor_position);
    int key, kind;

    if (le->line->length > 0) {
      size_t span_start = selection_start;
      size_t span_end;

      if (is_linewise) {
        span_start = itl_le_line_start_of(le, selection_start);
        span_end = itl_le_line_end_of(le, selection_end);
      } else {
        span_end = selection_end + 1;
        if (span_end > le->line->length) {
          span_end = le->line->length;
        }
      }
      itl_g_search_spans[0].start = span_start;
      itl_g_search_spans[0].end = span_end;
      itl_g_search_spans[0].sgr = ITL_VI_SGR_SELECT;
      itl_g_search_span_count = 1;
    } else {
      itl_g_search_span_count = 0;
    }
    itl_g_search_spans_active = true;
    itl_g_tty_should_refresh_text = true;
    itl_le_tty_refresh(le);

    if (!ITL_READ_BYTE(&byte)) {
      break;
    }

    if (byte == 27 && !itl_input_is_pending()) {
      break;
    }

    key = itl_esc_parse(byte);
    kind = key & TL_MASK_KEY;

    if (kind == TL_KEY_UNKN || byte == 'v') {
      break;
    }

    switch (kind) {
    case TL_KEY_ENTER:
    case TL_KEY_EOF:
    case TL_KEY_INTERRUPT:
    case TL_KEY_SUSPEND:
      itl_g_search_spans_active = false;
      itl_g_search_span_count = 0;
      itl_g_edit_mode = TL_EDIT_MODE_VI_COMMAND;
      itl_vi_clamp_command_cursor(le);
      itl_g_tty_should_refresh_text = true;
      itl_le_tty_refresh(le);
      return itl_le_key_handle(le, key);

    case TL_KEY_LEFT:
    case TL_KEY_RIGHT:
    case TL_KEY_HOME:
    case TL_KEY_END: itl_le_key_handle(le, key); continue;

    case TL_KEY_UP:
    case TL_KEY_DOWN: itl_vi_step_visual_row(le, kind == TL_KEY_UP); continue;

    default: break;
    }

    if (byte == 'd' || byte == 'x' || byte == 'c' || byte == 'y') {
      itl_vi_operator_kind op =
          (byte == 'y') ? ITL_VI_OP_YANK
                        : ((byte == 'c') ? ITL_VI_OP_CHANGE : ITL_VI_OP_DELETE);

      itl_g_search_spans_active = false;
      itl_g_search_span_count = 0;

      if (is_linewise) {
        size_t top_line = itl_le_line_index_of(le, selection_start);
        size_t bottom_line = itl_le_line_index_of(le, selection_end);
        le->cursor_position = itl_le_line_start_at_index(le, top_line);
        itl_vi_operator_line(le, op, (int) byte, bottom_line - top_line + 1);

        if (op != ITL_VI_OP_CHANGE) {
          itl_g_edit_mode = TL_EDIT_MODE_VI_COMMAND;
        }
      } else {
        itl_vi_apply_operator(le, op, selection_start, selection_end, true);

        if (op == ITL_VI_OP_CHANGE) {
          itl_g_vi_last_change.kind = ITL_VI_CHANGE_NONE;
          itl_vi_start_insert_recording();
        } else {
          itl_g_edit_mode = TL_EDIT_MODE_VI_COMMAND;
          itl_vi_clamp_command_cursor(le);
        }
      }

      itl_g_tty_should_refresh_text = true;
      return TL_SUCCESS;
    }

    {
      itl_utf8_t find_char = ITL_ZERO_INIT;
      bool is_inclusive, is_valid;
      size_t target;

      if (byte == 'f' || byte == 'F' || byte == 't' || byte == 'T') {
        uint8_t target_byte;
        if (ITL_READ_BYTE(&target_byte)) {
          find_char = itl_utf8_parse(target_byte);
        }
      }

      target = itl_vi_resolve_motion(le, (int) byte, find_char, 1, false,
                                     &is_inclusive, &is_valid);
      if (is_valid) {
        le->cursor_position = target;
      }
    }
  }

  itl_g_search_spans_active = false;
  itl_g_search_span_count = 0;
  itl_g_edit_mode = TL_EDIT_MODE_VI_COMMAND;
  itl_vi_clamp_command_cursor(le);
  itl_g_tty_should_refresh_text = true;
  itl_le_tty_refresh(le);

  return TL_SUCCESS;
}

ITL_DEF void itl_vi_block_insert_apply(itl_le_t *le)
{
  char text[ITL_STRING_MAX_LEN];
  size_t row;
  size_t top_start;

  itl_g_vi_block_insert_active = false;

  if (itl_g_vi_last_change.inserted_text == NULL ||
      itl_g_vi_last_change.inserted_text->length == 0)
  {
    return;
  }
  if (itl_string_to_cstr(itl_g_vi_last_change.inserted_text, text,
                         sizeof(text)) != TL_SUCCESS)
  {
    return;
  }

  if (strchr(text, '\n') != NULL) {
    return;
  }

  itl_g_undo_insert_run_open = true;

  for (row = 1; row < itl_g_vi_block_insert_row_count; ++row) {
    size_t line_start =
        itl_le_line_start_at_index(le, itl_g_vi_block_insert_top_line + row);
    size_t line_end = itl_le_line_end_of(le, line_start);

    if (line_end - line_start < itl_g_vi_block_insert_column) {
      continue;
    }

    le->cursor_position = line_start + itl_g_vi_block_insert_column;
    itl_le_insert_cstr(le, text);
  }

  top_start = itl_le_line_start_at_index(le, itl_g_vi_block_insert_top_line);
  le->cursor_position = top_start + itl_g_vi_block_insert_column;
  if (le->cursor_position > le->line->length) {
    le->cursor_position = le->line->length;
  }
}

ITL_DEF tl_status_code itl_vi_block_loop(itl_le_t *le, int return_mode)
{
  uint8_t byte;

  bool was_vertical = false;

  itl_g_vi_block_anchor = le->cursor_position;
  itl_g_vi_block_return_mode = return_mode;
  itl_g_edit_mode = TL_EDIT_MODE_VI_VISUAL;

  while (true) {
    size_t anchor_line = itl_le_line_index_of(le, itl_g_vi_block_anchor);
    size_t cursor_line = itl_le_line_index_of(le, le->cursor_position);
    size_t anchor_column =
        itl_g_vi_block_anchor - itl_le_line_start_of(le, itl_g_vi_block_anchor);
    size_t cursor_column =
        le->cursor_position - itl_le_line_start_of(le, le->cursor_position);
    size_t top_line = ITL_MIN(anchor_line, cursor_line);
    size_t bottom_line = ITL_MAX(anchor_line, cursor_line);
    size_t left_column = ITL_MIN(anchor_column, cursor_column);
    size_t right_column = ITL_MAX(anchor_column, cursor_column);
    int key, kind;
    size_t row;

    itl_g_search_span_count = 0;
    for (row = top_line; row <= bottom_line &&
                         itl_g_search_span_count < ITL_HIGHLIGHT_MAX_SPANS;
         ++row)
    {
      size_t line_start = itl_le_line_start_at_index(le, row);
      size_t line_end = itl_le_line_end_of(le, line_start);
      size_t span_start = line_start + left_column;
      size_t span_end = line_start + right_column + 1;

      if (span_start > line_end) {
        span_start = line_end;
      }
      if (span_end > line_end) {
        span_end = line_end;
      }
      if (span_start < span_end) {
        itl_g_search_spans[itl_g_search_span_count].start = span_start;
        itl_g_search_spans[itl_g_search_span_count].end = span_end;
        itl_g_search_spans[itl_g_search_span_count].sgr = ITL_VI_SGR_SELECT;
        itl_g_search_span_count += 1;
      } else if (line_end < le->line->length) {
        /* The block covers no column on this line, an empty line or one shorter
           than the left edge. A one-cell span on its newline draws a reversed
           space there so the selection and the mock cursor still show. */
        itl_g_search_spans[itl_g_search_span_count].start = line_end;
        itl_g_search_spans[itl_g_search_span_count].end = line_end + 1;
        itl_g_search_spans[itl_g_search_span_count].sgr = ITL_VI_SGR_SELECT;
        itl_g_search_span_count += 1;
      }
    }
    itl_g_search_spans_active = true;
    itl_g_tty_should_refresh_text = true;
    itl_le_tty_refresh(le);

    if (!ITL_READ_BYTE(&byte)) {
      break;
    }

    if (byte == 27 && !itl_input_is_pending()) {
      break;
    }

    key = itl_esc_parse(byte);
    kind = key & TL_MASK_KEY;

    if (kind == TL_KEY_UNKN || byte == 22) {
      break;
    }

    switch (kind) {
    case TL_KEY_ENTER:
    case TL_KEY_EOF:
    case TL_KEY_INTERRUPT:
    case TL_KEY_SUSPEND:
      itl_g_search_spans_active = false;
      itl_g_search_span_count = 0;
      itl_g_edit_mode = return_mode;
      itl_g_tty_should_refresh_text = true;
      itl_le_tty_refresh(le);
      return itl_le_key_handle(le, key);

    case TL_KEY_LEFT:
    case TL_KEY_RIGHT:
    case TL_KEY_HOME:
    case TL_KEY_END:
      was_vertical = false;
      itl_le_key_handle(le, key);
      continue;

    default: break;
    }

    if (kind == TL_KEY_UP || kind == TL_KEY_DOWN || byte == 'j' || byte == 'k')
    {
      itl_le_metrics_t m = itl_le_compute_metrics(le, itl_g_tty_prev_cols);
      bool is_up = (kind == TL_KEY_UP || byte == 'k');
      if (!was_vertical) {
        le->goal_column = m.cursor_col;
      }
      if (is_up && m.cursor_row > le->prompt_rows) {
        le->cursor_position = itl_le_index_at_visual(
            le, itl_g_tty_prev_cols, m.cursor_row - 1, le->goal_column);
      } else if (!is_up && m.cursor_row + 1 < m.total_rows) {
        le->cursor_position = itl_le_index_at_visual(
            le, itl_g_tty_prev_cols, m.cursor_row + 1, le->goal_column);
      }
      was_vertical = true;
      continue;
    }

    if (byte == 'd' || byte == 'x') {
      itl_undo_push(le);
      itl_g_undo_insert_run_open = false;

      row = bottom_line + 1;
      while (row > top_line) {
        size_t line_start, line_end, span_start, span_end;
        row -= 1;
        line_start = itl_le_line_start_at_index(le, row);
        line_end = itl_le_line_end_of(le, line_start);
        span_start = line_start + left_column;
        span_end = line_start + right_column + 1;
        if (span_start > line_end) {
          span_start = line_end;
        }
        if (span_end > line_end) {
          span_end = line_end;
        }
        if (span_start < span_end) {
          itl_string_erase(le->line, span_end, span_end - span_start, true);
        }
      }

      itl_g_search_spans_active = false;
      itl_g_search_span_count = 0;
      le->cursor_position =
          itl_le_line_start_at_index(le, top_line) + left_column;
      itl_g_edit_mode = return_mode;
      if (return_mode == TL_EDIT_MODE_VI_COMMAND) {
        itl_vi_clamp_command_cursor(le);
      }
      itl_g_tty_should_refresh_text = true;
      return TL_SUCCESS;
    }

    if (byte == 'I' || byte == 'c' || byte == 'C') {
      size_t top_start = itl_le_line_start_at_index(le, top_line);
      size_t top_length = itl_le_line_end_of(le, top_start) - top_start;
      size_t enter_column = ITL_MIN(left_column, top_length);

      itl_g_search_spans_active = false;
      itl_g_search_span_count = 0;
      itl_g_vi_block_insert_active = true;
      itl_g_vi_block_insert_top_line = top_line;
      itl_g_vi_block_insert_row_count = bottom_line - top_line + 1;
      itl_g_vi_block_insert_column = left_column;
      le->cursor_position = top_start + enter_column;
      itl_vi_begin_insert(true, 'i');
      itl_g_tty_should_refresh_text = true;
      return TL_SUCCESS;
    }

    {
      itl_utf8_t find_char = ITL_ZERO_INIT;
      bool is_inclusive, is_valid;
      size_t target;

      if (byte == 'f' || byte == 'F' || byte == 't' || byte == 'T') {
        uint8_t target_byte;
        if (ITL_READ_BYTE(&target_byte)) {
          find_char = itl_utf8_parse(target_byte);
        }
      }

      target = itl_vi_resolve_motion(le, (int) byte, find_char, 1, false,
                                     &is_inclusive, &is_valid);
      if (is_valid) {
        size_t line_start = itl_le_line_start_of(le, le->cursor_position);
        size_t line_end = itl_le_line_end_of(le, line_start);
        if (target < line_start) {
          target = line_start;
        }
        if (target > line_end) {
          target = line_end;
        }
        le->cursor_position = target;
      }
      was_vertical = false;
    }
  }

  itl_g_search_spans_active = false;
  itl_g_search_span_count = 0;
  itl_g_edit_mode = return_mode;
  if (return_mode == TL_EDIT_MODE_VI_COMMAND) {
    itl_vi_clamp_command_cursor(le);
  }
  itl_g_tty_should_refresh_text = true;
  itl_le_tty_refresh(le);

  return TL_SUCCESS;
}

ITL_DEF tl_status_code itl_emacs_multicursor_loop(itl_le_t *le)
{
  uint8_t byte;
  size_t anchor_line = itl_le_line_index_of(le, le->cursor_position);
  size_t active_line = anchor_line;
  size_t column =
      le->cursor_position - itl_le_line_start_of(le, le->cursor_position);
  bool did_push_undo = false;

  itl_g_multicursor_active = true;

  while (true) {
    size_t total_lines = itl_le_line_index_of(le, le->line->length) + 1;
    size_t top_line = ITL_MIN(anchor_line, active_line);
    size_t bottom_line = ITL_MAX(anchor_line, active_line);
    size_t active_start = itl_le_line_start_at_index(le, active_line);
    size_t active_end = itl_le_line_end_of(le, active_start);
    int key, kind;
    size_t row;

    itl_g_search_span_count = 0;
    for (row = top_line; row <= bottom_line &&
                         itl_g_search_span_count < ITL_HIGHLIGHT_MAX_SPANS;
         ++row)
    {
      size_t line_start = itl_le_line_start_at_index(le, row);
      size_t line_end = itl_le_line_end_of(le, line_start);
      size_t marker = line_start + column;

      if (marker > line_end) {
        marker = line_end;
      }
      if (row != active_line && marker < line_end) {
        itl_g_search_spans[itl_g_search_span_count].start = marker;
        itl_g_search_spans[itl_g_search_span_count].end = marker + 1;
        itl_g_search_spans[itl_g_search_span_count].sgr = ITL_VI_SGR_SELECT;
        itl_g_search_span_count += 1;
      } else if (row != active_line && line_end < le->line->length) {
        /* The line has no character under the marker, an empty line or one
           shorter than the column. A one-cell span on its newline draws a
           reversed space there to stand in for the mock cursor. */
        itl_g_search_spans[itl_g_search_span_count].start = line_end;
        itl_g_search_spans[itl_g_search_span_count].end = line_end + 1;
        itl_g_search_spans[itl_g_search_span_count].sgr = ITL_VI_SGR_SELECT;
        itl_g_search_span_count += 1;
      }
    }
    itl_g_search_spans_active = true;

    le->cursor_position = (column < active_end - active_start)
                              ? active_start + column
                              : active_end;
    itl_g_tty_should_refresh_text = true;
    itl_le_tty_refresh(le);

    if (!ITL_READ_BYTE(&byte)) {
      break;
    }

    if (byte == 7) {
      break;
    }
    if (byte == 27 && !itl_input_is_pending()) {
      break;
    }

    key = itl_esc_parse(byte);
    kind = key & TL_MASK_KEY;

    if (kind == TL_KEY_UNKN) {
      break;
    }

    if (kind == TL_KEY_ENTER || kind == TL_KEY_EOF ||
        kind == TL_KEY_INTERRUPT || kind == TL_KEY_SUSPEND)
    {
      itl_g_search_spans_active = false;
      itl_g_search_span_count = 0;
      itl_g_multicursor_active = false;
      itl_g_tty_should_refresh_text = true;
      itl_le_tty_refresh(le);
      return itl_le_key_handle(le, key);
    }

    if (kind == TL_KEY_UP) {
      if (active_line > 0) {
        active_line -= 1;
      }
      continue;
    }
    if (kind == TL_KEY_DOWN) {
      if (active_line + 1 < total_lines) {
        active_line += 1;
      }
      continue;
    }

    if (kind == TL_KEY_LEFT || kind == TL_KEY_RIGHT || kind == TL_KEY_HOME ||
        kind == TL_KEY_END)
    {
      itl_le_key_handle(le, key);
      if (le->cursor_position < active_start) {
        le->cursor_position = active_start;
      }
      if (le->cursor_position > active_end) {
        le->cursor_position = active_end;
      }
      column = le->cursor_position - active_start;
      continue;
    }

    if (kind == TL_KEY_BACKSPACE) {
      if (column > 0) {
        if (!did_push_undo) {
          itl_undo_push(le);
          did_push_undo = true;
        }
        row = bottom_line + 1;
        while (row > top_line) {
          size_t line_start, line_end, at;
          row -= 1;
          line_start = itl_le_line_start_at_index(le, row);
          line_end = itl_le_line_end_of(le, line_start);
          at = line_start + column;
          if (at > line_start && at <= line_end) {
            itl_string_erase(le->line, at, 1, true);
          }
        }
        column -= 1;
      }
      continue;
    }

    if (kind == TL_KEY_CHAR) {
      itl_utf8_t ch = itl_utf8_parse(byte);
      size_t row_count = bottom_line - top_line + 1;

      if (le->line->size + ch.size * row_count >= le->out_size) {
        continue;
      }
      if (!did_push_undo) {
        itl_undo_push(le);
        did_push_undo = true;
      }
      row = bottom_line + 1;
      while (row > top_line) {
        size_t line_start, line_end, at;
        row -= 1;
        line_start = itl_le_line_start_at_index(le, row);
        line_end = itl_le_line_end_of(le, line_start);
        at = line_start + column;
        if (at > line_end) {
          at = line_end;
        }
        itl_string_insert(le->line, at, ch);
      }
      column += 1;
      continue;
    }
  }

  itl_g_search_spans_active = false;
  itl_g_search_span_count = 0;
  itl_g_multicursor_active = false;

  {
    size_t active_start = itl_le_line_start_at_index(le, active_line);
    size_t active_end = itl_le_line_end_of(le, active_start);
    le->cursor_position = active_start + column;
    if (le->cursor_position > active_end) {
      le->cursor_position = active_end;
    }
  }
  itl_g_tty_should_refresh_text = true;
  itl_le_tty_refresh(le);

  return TL_SUCCESS;
}

ITL_DEF bool itl_vi_ex_is_quit(const char *command)
{
  return strcmp(command, "q") == 0 || strcmp(command, "q!") == 0 ||
         strcmp(command, "wq") == 0 || strcmp(command, "wq!") == 0 ||
         strcmp(command, "x") == 0 || strcmp(command, "quit") == 0;
}

ITL_DEF tl_status_code itl_vi_ex_command(itl_le_t *le)
{
  itl_string_t *original = itl_string_alloc();
  itl_string_t *display = itl_string_alloc();
  itl_char_buf_t *status = itl_char_buf_alloc();
  itl_string_t *preview = itl_string_alloc();

  const char *saved_prompt = le->prompt;
  size_t saved_prompt_size = le->prompt_size;
  size_t saved_prompt_width = le->prompt_width;
  size_t saved_prompt_rows = le->prompt_rows;

  char command[64];
  size_t command_length = 0;

  tl_status_code result = TL_SUCCESS;
  bool is_done = false;
  uint8_t byte;

  command[0] = '\0';
  itl_string_copy(original, le->line);

  while (!is_done) {
    int key, kind;

    ITL_CHAR_BUF_CLEAR(status);
    /* The display is original plus '\n', ':', and the command, so its length
       must stay under ITL_STRING_MAX_LEN or itl_string_from_bytes traps. Clip
       the preview of the original line to the remaining budget. */
    {
      size_t budget = ITL_STRING_MAX_LEN - 2 - command_length;
      itl_string_copy(preview, original);
      if (preview->length > budget) {
        preview->length = budget;
        itl_string_recalc_size(preview);
      }
      itl_char_buf_append_string(status, preview);
    }
    itl_char_buf_append_byte(status, '\n');
    itl_char_buf_append_byte(status, ':');
    itl_char_buf_append_cstr(status, command);

    itl_string_from_bytes(display, status->data, status->size);

    le->line = display;
    le->cursor_position = display->length;
    itl_g_search_span_count = 0;
    itl_g_search_spans_active = true;
    itl_g_tty_should_refresh_text = true;
    itl_le_tty_refresh(le);

    if (!ITL_READ_BYTE(&byte)) {
      break;
    }

    key = itl_esc_parse(byte);
    kind = key & TL_MASK_KEY;

    switch (kind) {
    case TL_KEY_ENTER:
      if (itl_vi_ex_is_quit(command)) {
        result = TL_PRESSED_QUIT;
      }
      is_done = true;
      break;

    case TL_KEY_BACKSPACE:
      if (command_length > 0) {
        command_length -= 1;
        command[command_length] = '\0';
      } else {
        is_done = true;
      }
      break;

    case TL_KEY_CHAR:
      if (command_length < sizeof(command) - 1) {
        command[command_length++] = (char) byte;
        command[command_length] = '\0';
      }
      break;

    case TL_KEY_UNKN:
    case TL_KEY_INTERRUPT:
    case TL_KEY_EOF:
    case TL_KEY_SUSPEND: is_done = true; break;

    default: break;
    }
  }

  le->prompt = saved_prompt;
  le->prompt_size = saved_prompt_size;
  le->prompt_width = saved_prompt_width;
  le->prompt_rows = saved_prompt_rows;
  le->line = &itl_g_line_buffer;
  itl_string_copy(le->line, original);
  le->cursor_position = le->line->length;
  itl_vi_clamp_command_cursor(le);
  itl_g_search_spans_active = false;
  itl_g_search_span_count = 0;
  itl_g_tty_should_refresh_text = true;

  ITL_STRING_FREE(original);
  ITL_STRING_FREE(display);
  ITL_STRING_FREE(preview);
  ITL_CHAR_BUF_FREE(status);

  return result;
}

ITL_DEF tl_status_code itl_vi_command_dispatch(itl_le_t *le, uint8_t byte,
                                                int key)
{
  int kind = key & TL_MASK_KEY;
  itl_utf8_t none = ITL_ZERO_INIT;
  size_t count;

  /* A pending operator resolves j and k as the linewise Up and Down motions,
     the same as the arrow keys, so dj, dk, cj, ck, yj, and yk act on whole
     lines instead of falling through to itl_vi_operator_motion which cannot
     resolve the letter and would silently drop the operator. */
  if (itl_g_vi_pending_operator != ITL_VI_OP_NONE) {
    if (byte == 'j') {
      kind = TL_KEY_DOWN;
    } else if (byte == 'k') {
      kind = TL_KEY_UP;
    }
  }

  itl_g_tty_should_refresh_text = true;

  switch (kind) {
  case TL_KEY_ENTER:
  case TL_KEY_EOF:
  case TL_KEY_INTERRUPT:
  case TL_KEY_SUSPEND:
  case TL_KEY_CLEAR: return itl_le_key_handle(le, key);

  case TL_KEY_UP:
  case TL_KEY_DOWN:
    /* An operator waits, so the arrow deletes or changes whole lines from the
       current one to the one the count steps onto, the linewise dk and dj. A
       step past the first or the last line is a failed motion and changes
       nothing. */
    if (itl_g_vi_pending_operator != ITL_VI_OP_NONE) {
      itl_vi_operator_kind op = itl_g_vi_pending_operator;
      size_t cursor_line = itl_le_line_index_of(le, le->cursor_position);
      size_t total_lines = itl_le_line_index_of(le, le->line->length) + 1;
      size_t step = (itl_g_vi_pending_count == 0) ? 1 : itl_g_vi_pending_count;
      int doubled = (op == ITL_VI_OP_DELETE)   ? 'd'
                    : (op == ITL_VI_OP_CHANGE) ? 'c'
                                               : 'y';
      size_t target_line = cursor_line;

      if (kind == TL_KEY_UP) {
        target_line = (cursor_line > step) ? cursor_line - step : 0;
      } else {
        target_line = cursor_line + step;
        if (target_line > total_lines - 1) {
          target_line = total_lines - 1;
        }
      }

      if (target_line != cursor_line) {
        size_t top_line = ITL_MIN(target_line, cursor_line);
        size_t bottom_line = ITL_MAX(target_line, cursor_line);
        le->cursor_position = itl_le_line_start_at_index(le, top_line);
        itl_vi_operator_line(le, op, doubled, bottom_line - top_line + 1);
      }

      itl_g_vi_pending_operator = ITL_VI_OP_NONE;
      itl_g_vi_pending_count = 0;
      itl_g_vi_pending_register = 0;
      if (itl_g_edit_mode == TL_EDIT_MODE_VI_COMMAND) {
        itl_vi_clamp_command_cursor(le);
      }
      return TL_SUCCESS;
    }

    /* Normal mode steps between buffer rows only, so the arrows never recall
       history the way insert mode does. */
    itl_vi_step_visual_row(le, kind == TL_KEY_UP);
    itl_vi_clamp_command_cursor(le);
    return TL_SUCCESS;

  case TL_KEY_LEFT:
  case TL_KEY_RIGHT:
  case TL_KEY_HOME:
  case TL_KEY_END: {
    tl_status_code code = itl_le_key_handle(le, key);
    itl_vi_clamp_command_cursor(le);
    return code;
  }

  case TL_KEY_UNDO:
    itl_undo_close_insert_run();
    if (itl_undo_pop(le)) {
      itl_vi_clamp_command_cursor(le);
    }
    return TL_SUCCESS;

  case TL_KEY_REDO:
  case TL_KEY_HISTORY_SEARCH:
    if (itl_redo(le)) {
      itl_vi_clamp_command_cursor(le);
    }
    return TL_SUCCESS;

  case TL_KEY_BACKSPACE: itl_le_move_left(le, 1); return TL_SUCCESS;

  case TL_KEY_UNKN:
    itl_g_vi_pending_operator = ITL_VI_OP_NONE;
    itl_g_vi_pending_count = 0;
    itl_g_vi_pending_register = 0;
    return TL_SUCCESS;

  default: break;
  }

  if (byte >= '1' && byte <= '9') {
    itl_g_vi_pending_count =
        itl_g_vi_pending_count * 10 + (size_t) (byte - '0');
    if (itl_g_vi_pending_count > ITL_STRING_MAX_LEN) {
      itl_g_vi_pending_count = ITL_STRING_MAX_LEN;
    }
    return TL_SUCCESS;
  }
  if (byte == '0' && itl_g_vi_pending_count > 0) {
    itl_g_vi_pending_count *= 10;
    if (itl_g_vi_pending_count > ITL_STRING_MAX_LEN) {
      itl_g_vi_pending_count = ITL_STRING_MAX_LEN;
    }
    return TL_SUCCESS;
  }

  count = (itl_g_vi_pending_count == 0) ? 1 : itl_g_vi_pending_count;

  if (byte == '"') {
    uint8_t register_byte;
    if (ITL_READ_BYTE(&register_byte)) {
      itl_g_vi_pending_register = (char) register_byte;
    }
    return TL_SUCCESS;
  }

  if (itl_g_vi_pending_operator != ITL_VI_OP_NONE) {
    itl_vi_operator_kind op = itl_g_vi_pending_operator;
    bool is_doubled = (op == ITL_VI_OP_DELETE && byte == 'd') ||
                      (op == ITL_VI_OP_CHANGE && byte == 'c') ||
                      (op == ITL_VI_OP_YANK && byte == 'y');

    if (is_doubled) {
      itl_vi_operator_line(le, op, (int) byte, count);
    } else {
      itl_utf8_t find_char = ITL_ZERO_INIT;
      if (byte == 'f' || byte == 'F' || byte == 't' || byte == 'T') {
        uint8_t target_byte;
        if (ITL_READ_BYTE(&target_byte)) {
          find_char = itl_utf8_parse(target_byte);
        }
      }
      itl_vi_operator_motion(le, op, (int) byte, find_char, count);
    }

    itl_g_vi_pending_operator = ITL_VI_OP_NONE;
    itl_g_vi_pending_count = 0;
    itl_g_vi_pending_register = 0;
    if (itl_g_edit_mode == TL_EDIT_MODE_VI_COMMAND) {
      itl_vi_clamp_command_cursor(le);
    }
    return TL_SUCCESS;
  }

  switch (byte) {
  case 'd': itl_g_vi_pending_operator = ITL_VI_OP_DELETE; return TL_SUCCESS;
  case 'c': itl_g_vi_pending_operator = ITL_VI_OP_CHANGE; return TL_SUCCESS;
  case 'y': itl_g_vi_pending_operator = ITL_VI_OP_YANK; return TL_SUCCESS;

  case 'i': itl_vi_begin_insert(true, 'i'); break;
  case 'I': {
    bool is_inclusive, is_valid;
    le->cursor_position = itl_vi_resolve_motion(le, '^', none, 1, false,
                                                &is_inclusive, &is_valid);
    itl_vi_begin_insert(true, 'I');
  } break;
  case 'a':
    if (le->line->length > 0) {
      itl_le_move_right(le, 1);
    }
    itl_vi_begin_insert(true, 'a');
    break;
  case 'A':
    le->cursor_position = itl_le_logical_line_end(le);
    itl_vi_begin_insert(true, 'A');
    break;

  case 'x':
    itl_vi_operator_motion(le, ITL_VI_OP_DELETE, 'l', none, count);
    break;
  case 'X':
    itl_vi_operator_motion(le, ITL_VI_OP_DELETE, 'h', none, count);
    break;
  case 'D':
    itl_vi_operator_motion(le, ITL_VI_OP_DELETE, '$', none, count);
    break;
  case 'C':
    itl_vi_operator_motion(le, ITL_VI_OP_CHANGE, '$', none, count);
    break;
  case 's':
    itl_vi_operator_motion(le, ITL_VI_OP_CHANGE, 'l', none, count);
    break;
  case 'S': itl_vi_operator_line(le, ITL_VI_OP_CHANGE, 'c', count); break;

  case 'r': {
    uint8_t replace_byte;
    if (ITL_READ_BYTE(&replace_byte) &&
        le->cursor_position + count <= le->line->length)
    {
      itl_utf8_t ch = itl_utf8_parse(replace_byte);
      itl_g_vi_last_change.kind = ITL_VI_CHANGE_REPLACE;
      itl_g_vi_last_change.replace_char = ch;
      itl_g_vi_last_change.repeat_count = count;
      itl_vi_do_replace(le, ch, count);
    }
  } break;

  case 'R': itl_vi_begin_insert(true, 'R'); break;

  case '~':
    itl_g_vi_last_change.kind = ITL_VI_CHANGE_TILDE;
    itl_g_vi_last_change.repeat_count = count;
    itl_vi_do_tilde(le, count);
    break;

  case 'p':
    itl_g_vi_last_change.kind = ITL_VI_CHANGE_PASTE;
    itl_g_vi_last_change.is_paste_before = false;
    itl_g_vi_last_change.repeat_count = count;
    itl_vi_do_paste(le, false, count);
    break;
  case 'P':
    itl_g_vi_last_change.kind = ITL_VI_CHANGE_PASTE;
    itl_g_vi_last_change.is_paste_before = true;
    itl_g_vi_last_change.repeat_count = count;
    itl_vi_do_paste(le, true, count);
    break;

  case 'u':
    itl_undo_close_insert_run();
    if (itl_undo_pop(le)) {
      itl_vi_clamp_command_cursor(le);
    }
    break;

  case '.': itl_vi_repeat_last_change(le); break;

  case 'v':
    itl_g_vi_pending_count = 0;
    itl_g_vi_pending_register = 0;
    return itl_vi_visual_loop(le, false);

  case 'V':
    itl_g_vi_pending_count = 0;
    itl_g_vi_pending_register = 0;
    return itl_vi_visual_loop(le, true);

  case ':':
    itl_g_vi_pending_count = 0;
    itl_g_vi_pending_register = 0;
    return itl_vi_ex_command(le);

  case '/': {
    int after_search = itl_history_search(le);
    itl_g_tty_should_refresh_text = true;
    itl_le_tty_refresh(le);
    itl_g_vi_pending_count = 0;
    itl_g_vi_pending_register = 0;
    if (after_search != TL_KEY_UNKN) {
      tl_status_code search_code = itl_le_key_handle(le, after_search);
      if (search_code != TL_SUCCESS) {
        return search_code;
      }
    }
    itl_vi_clamp_command_cursor(le);
    return TL_SUCCESS;
  }

  case 'k':
    itl_g_history_get_prev(le);
    itl_vi_clamp_command_cursor(le);
    break;
  case 'j':
    itl_g_history_get_next(le);
    itl_vi_clamp_command_cursor(le);
    break;

  case 'f':
  case 'F':
  case 't':
  case 'T': {
    uint8_t target_byte;
    itl_utf8_t find_char = ITL_ZERO_INIT;
    if (ITL_READ_BYTE(&target_byte)) {
      find_char = itl_utf8_parse(target_byte);
    }
    itl_vi_apply_bare_motion(le, (int) byte, find_char, count);
  } break;

  default: itl_vi_apply_bare_motion(le, (int) byte, none, count); break;
  }

  itl_vi_clamp_command_cursor(le);
  itl_g_vi_pending_count = 0;
  itl_g_vi_pending_register = 0;
  return TL_SUCCESS;
}

TL_DEF tl_status_code tl_get_input(char *buffer, size_t buffer_size,
                                   const char *prompt)
{
  itl_le_t *le = &itl_g_le;
  uint8_t input_byte;
  int input_type;

  tl_status_code code;

  TL_ASSERT(itl_g_is_active && "tl_init() should be called");
  TL_ASSERT(
      buffer_size > 1 &&
      "Size should be enough at least for one byte and a null terminator");
  TL_ASSERT(
      buffer_size <= ITL_STRING_MAX_LEN &&
      "Size should be less than platform's allowed maximum string length");
  TL_ASSERT(buffer != NULL);

  itl_le_init(le, &itl_g_line_buffer, buffer, buffer_size, prompt);

  /* A new line starts with no ghost, since the previous line's suggestion does
     not carry over, and predefined input is shown as the user's own text. */
  itl_ghost_clear();

  /* Avoid clearing lines that don't belong to us. */
  itl_g_le_prev_total_rows = 1;
  itl_g_le_prev_cursor_row = 1;
  /* The incremental-append fast path keys off the previous render, so its state
     is reset with the row counts, otherwise the first refresh of this line
     could compare against the previous command's render. */
  itl_g_le_prev_render_len = 0;
  itl_g_le_prev_length = 0;
  itl_g_le_prev_cursor_at_end = false;
  itl_le_tty_refresh(le);

  while (true) {
#if defined ITL_POSIX && !defined ITL_INJECT_KLEE
    /* Block for input, but wake on SIGWINCH to redraw the line live as the
       window resizes instead of waiting for the next keystroke. A SIGCHLD
       wakes the poll the same way, and the wake hook lets the host print a
       job report above the live prompt. */
    for (;;) {
      if (itl_g_tty_changed_size) {
        itl_g_tty_should_refresh_text = true;
        itl_le_tty_refresh(le);
      }
      if (itl_g_wake_callback != NULL && itl_g_wake_callback(0)) {
        /* Clear the current render block the way the full refresh does, the
           block top then everything below, so the host's rows land where
           the prompt began and the fresh render follows them. */
        itl_char_buf_t *wake_buf = &itl_g_char_buffer;
        if (itl_g_le_prev_cursor_row > 1) {
          ITL_TTY_MOVE_UP(wake_buf, itl_g_le_prev_cursor_row - 1);
        }
        ITL_TTY_MOVE_TO_COLUMN(wake_buf, 1);
        ITL_TTY_CLEAR_BELOW(wake_buf);
        ITL_CHAR_BUF_DUMP(wake_buf);
        ITL_CHAR_BUF_CLEAR(wake_buf);
        itl_g_wake_callback(1);
        itl_g_tty_first_render = true;
        itl_g_le_prev_total_rows = 1;
        itl_g_le_prev_cursor_row = 1;
        itl_g_le_prev_cursor_col = 0;
        itl_g_le_prev_render_len = 0;
        itl_g_le_prev_length = 0;
        itl_g_le_prev_cursor_at_end = false;
        itl_g_tty_should_refresh_text = true;
        itl_le_tty_refresh(le);
      }
      if (itl_input_is_pending()) {
        break;
      }
      itl_wait_for_input();
    }
#elif defined ITL_WIN32 && !defined ITL_INJECT_KLEE
    /* The console raises no resize signal, so the wait blocks on the input
       handle and polls the size on its timeout, redrawing live when it
       changes, the same shape as the POSIX branch above. */
    for (;;) {
      if (itl_g_tty_changed_size) {
        itl_g_tty_should_refresh_text = true;
        itl_le_tty_refresh(le);
      }
      if (itl_input_is_pending()) {
        break;
      }
      itl_wait_for_input();
    }
#endif /* ITL_POSIX && !ITL_INJECT_KLEE */

    ITL_TRY_READ_BYTE(&input_byte, return TL_ERROR);

#if defined TL_SEE_BYTES
    if (input_byte == 3) return -69; /* ctrl c */
    if (iscntrl(input_byte) || input_byte > 127) {
      printf("cntrl seq -> ");
    } else {
      printf("'%c' -> ", (char) input_byte);
    }
    printf("%d\n", input_byte);
    fflush(stdout);
    continue;
#endif /* TL_SEE_BYTES */

    if (input_byte == 27 && itl_g_edit_mode != TL_EDIT_MODE_EMACS &&
        !itl_input_is_pending())
    {
      if (itl_g_edit_mode == TL_EDIT_MODE_VI_INSERT) {
        itl_undo_close_insert_run();
        itl_g_vi_is_recording_insert = false;
        if (le->cursor_position > 0) {
          itl_le_move_left(le, 1);
        }
      }

      if (itl_g_vi_block_insert_active) {
        itl_vi_block_insert_apply(le);
        itl_g_edit_mode = itl_g_vi_block_return_mode;
      } else {
        itl_g_edit_mode = TL_EDIT_MODE_VI_COMMAND;
      }
      itl_g_vi_pending_operator = ITL_VI_OP_NONE;
      itl_g_vi_pending_count = 0;
      itl_g_vi_pending_register = 0;
      itl_ghost_clear();
      itl_g_tty_should_refresh_text = true;
      itl_le_tty_refresh(le);
      continue;
    }

    input_type = itl_esc_parse(input_byte);
    itl_g_tty_plain_append_pending = false;

    if (itl_g_vi_block_insert_active &&
        itl_g_edit_mode == TL_EDIT_MODE_VI_INSERT)
    {
      int leaving_kind = input_type & TL_MASK_KEY;
      if (leaving_kind == TL_KEY_ENTER || leaving_kind == TL_KEY_EOF ||
          leaving_kind == TL_KEY_INTERRUPT || leaving_kind == TL_KEY_SUSPEND)
      {
        itl_undo_close_insert_run();
        itl_g_vi_is_recording_insert = false;
        itl_vi_block_insert_apply(le);
        itl_g_edit_mode = itl_g_vi_block_return_mode;
      }
    }

    if ((input_type & TL_MASK_KEY) == TL_KEY_PASTE_BEGIN) {
      /* A paste replaces the token wholesale, so the stale ghost is dropped. */
      itl_ghost_clear();
      itl_le_read_paste(le);
      itl_g_tty_should_refresh_text = true;
    } else if ((itl_g_edit_mode == TL_EDIT_MODE_EMACS ||
                itl_g_edit_mode == TL_EDIT_MODE_VI_INSERT) &&
               ((input_type & TL_MASK_KEY) == TL_KEY_HISTORY_SEARCH ||
                input_byte == 6))
    {
      /* Ctrl-R and Ctrl-F both enable incremental search. Ctrl-F is caught by
         its raw byte so the right arrow, which also parses to TL_KEY_RIGHT,
         still moves the cursor. Once in search, ctrl-f and ctrl-r steer it
         forward and backward. */
      int after_search = itl_history_search(le);
      /* Redraw the restored line first so the search view is cleared even when
         the terminating key submits or only moves the cursor. */
      itl_g_tty_should_refresh_text = true;
      itl_le_tty_refresh(le);
      if (after_search != TL_KEY_UNKN) {
        code = itl_le_key_handle(le, after_search);
        if (code != TL_SUCCESS) {
          itl_le_clear_line(le);
          return code;
        }
      }
    } else if (input_byte == 22 && itl_g_edit_mode == TL_EDIT_MODE_EMACS) {
      itl_ghost_clear();
      code = itl_emacs_multicursor_loop(le);
      if (code != TL_SUCCESS) {
        itl_le_clear_line(le);
        return code;
      }
    } else if (input_byte == 22 && itl_g_edit_mode == TL_EDIT_MODE_VI_COMMAND) {
      itl_ghost_clear();
      code = itl_vi_block_loop(le, itl_g_edit_mode);
      if (code != TL_SUCCESS) {
        itl_le_clear_line(le);
        return code;
      }
    } else if (itl_g_edit_mode == TL_EDIT_MODE_VI_COMMAND ||
               itl_g_edit_mode == TL_EDIT_MODE_VI_VISUAL)
    {
      itl_ghost_clear();
      code = itl_vi_command_dispatch(le, input_byte, input_type);
      if (code != TL_SUCCESS) {
        itl_le_clear_line(le);
        return code;
      }
    } else if (input_type != TL_KEY_CHAR) {
      /* Any non-character key edits or moves, so the stale ghost is dropped
         before the key runs. Tab and the arrow keys that accept the ghost
         manage it themselves inside the handler. */
      bool is_tab = (input_type & TL_MASK_KEY) == TL_KEY_TAB;
      size_t line_length_before_key = le->line->length;
      itl_undo_close_insert_run();
      bool accepts_ghost = ((input_type & TL_MASK_KEY) == TL_KEY_RIGHT ||
                            (input_type & TL_MASK_KEY) == TL_KEY_END) &&
                           le->cursor_position == le->line->length &&
                           itl_g_ghost_len > 0;
      /* Whether a dimmed ghost is on screen before it is cleared below, so the
         terminating-key path knows if a repaint is needed to erase it. */
      bool ghost_was_on_screen = itl_g_ghost_len > 0;
      if (!is_tab && !accepts_ghost) {
        itl_ghost_clear();
      }
      code = itl_le_key_handle(le, input_type);
      if (code != TL_SUCCESS) {
        /* A terminating key such as Enter leaves the loop before the refresh at
           the bottom runs. When a dimmed ghost was on screen it must be erased,
           so one forced text refresh redraws the line without it. With no ghost
           the line on screen is already correct, so the repaint is skipped,
           which avoids a full-block flicker on a multiline submit. */
        if (ghost_was_on_screen) {
          itl_g_tty_should_refresh_text = true;
          itl_le_tty_refresh(le);
        }
        itl_le_clear_line(le);
        return code;
      }
      /* After tab grew the line, offer a fresh ghost for the new token. */
      if (is_tab && le->line->length > line_length_before_key) {
        itl_ghost_update(le);
      }
    } else {
      itl_utf8_t appended_character = itl_utf8_parse(input_byte);
      itl_g_tty_plain_append_pending =
          le->cursor_position == le->line->length;
      itl_g_tty_plain_append_width = itl_char_width(appended_character);
      itl_le_insert(le, appended_character);
      itl_g_tty_should_refresh_text = true;
      /* Recompute the ghost for the token the new character extended. */
      itl_ghost_update(le);
    }

    ITL_TRACELN("strlen: %zu, hist index: %zu\n", le->line->length,
                le->history_selected_index);
    itl_le_tty_refresh(le);
  }

  ITL_UNREACHABLE();
}

TL_DEF void tl_set_predefined_input(const char *str)
{
  TL_ASSERT(itl_g_is_active && "tl_init() should be called");
  itl_string_shrink(&itl_g_line_buffer);
  ITL_STRING_FROM_CSTR(&itl_g_line_buffer, str);
}

TL_DEF tl_status_code tl_get_character(char *char_buffer,
                                       size_t char_buffer_size,
                                       const char *prompt)
{
  itl_le_t *le = &itl_g_le;
  uint8_t input_byte = 0;
  int input_type = TL_KEY_UNKN;

  TL_ASSERT(itl_g_is_active && "tl_init() should be called");
  TL_ASSERT(
      char_buffer_size > 1 &&
      "Size should be enough at least for one byte and a null terminator");
  TL_ASSERT(char_buffer_size <= sizeof(char) * 5 &&
            "Size should be less or equal to size of 4 characters with a null "
            "terminator.");
  TL_ASSERT(char_buffer != NULL);

  itl_le_init(le, &itl_g_line_buffer, char_buffer, char_buffer_size, prompt);

  /* Clear leftover predefined input so a single character is read fresh. */
  if (itl_g_line_buffer.length != 0) {
    itl_string_clear(&itl_g_line_buffer);
  }

  itl_le_tty_refresh(le);
  ITL_TRY_READ_BYTE(&input_byte, return TL_ERROR);

  input_type = itl_esc_parse(input_byte);
  if (input_type != TL_KEY_CHAR) {
    itl_g_last_control = input_type;
    return TL_PRESSED_CONTROL_SEQUENCE;
  }

  itl_le_insert(le, itl_utf8_parse(input_byte));
  itl_g_tty_should_refresh_text = true;
  itl_le_tty_refresh(le);
  ITL_TRY(itl_string_to_cstr(le->line, char_buffer, char_buffer_size) ==
              TL_SUCCESS,
          return TL_ERROR_SIZE);
  itl_le_clear_line(le);

  return TL_SUCCESS;
}

TL_DEF tl_status_code tl_history_load(const char *file_path)
{
  return itl_history_load_from_file(file_path);
}

TL_DEF void tl_set_history_enabled(bool enabled)
{
  itl_g_history_enabled = enabled;
}

TL_DEF void tl_set_history_limit(size_t entry_count)
{
  size_t removed_count;

  if (entry_count > TL_HISTORY_MAX_SIZE) entry_count = TL_HISTORY_MAX_SIZE;
  if (entry_count == itl_g_history_limit) return;

  itl_g_history_limit = entry_count;
  if (itl_g_history_count <= entry_count) return;

  removed_count = itl_g_history_count - entry_count;
  itl_g_history_head =
      (itl_g_history_head + removed_count) % (TL_HISTORY_MAX_SIZE);
  itl_g_history_count = entry_count;
  itl_history_read_fd_invalidate();
}

TL_DEF tl_status_code tl_history_dump(const char *file_path)
{
  return itl_history_dump_to_file(file_path);
}

TL_DEF size_t tl_utf8_strlen(const char *utf8_str)
{
  size_t len = 0;
  while (*utf8_str != '\0') {
    if ((*utf8_str & 0xC0) != 0x80) {
      len += 1;
    }
    utf8_str += 1;
  }
  return len;
}

TL_DEF size_t tl_utf8_strnlen(const char *utf8_str, size_t byte_count)
{
  size_t len = 0;
  while (*utf8_str != '\0' && byte_count-- > 0) {
    /* The byte is read as unsigned before the mask, since a signed char sign
       extends a continuation byte such as 0x8E and a codegen that keeps the
       comparison narrow then misreads it as a lead byte, which miscounts a
       multibyte string. */
    if (((unsigned char) *utf8_str & 0xC0) != 0x80) {
      len += 1;
    }
    utf8_str += 1;
  }
  return len;
}

TL_DEF tl_status_code tl_emit_newlines(const char *char_buffer)
{
  size_t i, newlines_to_emit;

  (void) char_buffer;

  /* Move below the whole rendered input using the visual extent recorded by the
     last refresh, which already accounts for wrapping, wide glyphs, and
     embedded newlines. The cursor row is 1-based, so this stays at least 1. */
  newlines_to_emit = itl_g_le_prev_total_rows - (itl_g_le_prev_cursor_row - 1);

  for (i = 0; i < newlines_to_emit; ++i) {
    ITL_TRY(ITL_WRITE(ITL_STDOUT, "\n", 1) != -1, return TL_ERROR);
  }

  return TL_SUCCESS;
}

TL_DEF tl_status_code tl_set_title(const char *title)
{
  if (ITL_ISATTY(STDOUT_FILENO)) {
    ITL_TRY(ITL_WRITE(ITL_STDOUT, "\x1b]0;", 4) != -1, return TL_ERROR);
    ITL_TRY(ITL_WRITE(ITL_STDOUT, title, strlen(title)) != -1, return TL_ERROR);
    ITL_TRY(ITL_WRITE(ITL_STDOUT, "\x07", 1) != -1, return TL_ERROR);
    return TL_SUCCESS;
  }
  return TL_ERROR;
}

#if defined ITL_WIN32_DISABLED_WARNINGS
#undef _CRT_SECURE_NO_WARNINGS
#endif /* ITL_WIN32_DISABLED_WARNINGS */

#endif /* TOILETLINE_IMPLEMENTATION */

#if defined __cplusplus
}
#endif

/*
 * Later work, not soon.
 *  - Use Windows' console API instead of terminal sequences on Windows.
 */
