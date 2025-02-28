/*
 *  toiletline 0.7.0
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
#define TL_MINOR_VERSION 6
#define TL_PATCH_VERSION 0

/* Compile with -Werror on Windows */
#if defined TOILETLINE_IMPLEMENTATION && !defined _CRT_SECURE_NO_WARNINGS
#define ITL_WIN32_DISABLED_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif /* !_CRT_SECURE_NO_WARNINGS */

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
#define TL_HISTORY_MAX_SIZE 256
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

  /**
   * Codes below 0 are errors.
   */
  TL_ERROR = -1,
  TL_ERROR_SIZE = -2,
  TL_ERROR_ALLOC = -3,
} TL_STATUS_CODE;

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

/**
 * Last pressed control sequence.
 */
TL_DEF int tl_last_control_sequence(void);
/**
 * Initialize toiletline and put terminal in raw mode.
 */
TL_DEF TL_STATUS_CODE tl_init(void);
/**
 * Put the terminal into raw mode without doing anything else.
 */
TL_DEF TL_STATUS_CODE tl_enter_raw_mode(void);
/**
 * Exit toiletline, restore terminal state, delete all completions, and free
 * internal memory.
 */
TL_DEF TL_STATUS_CODE tl_exit(void);
/**
 * Restore the terminal state without doing anything else.
 */
TL_DEF TL_STATUS_CODE tl_exit_raw_mode(void);
/**
 * Read input into the buffer.
 */
TL_DEF TL_STATUS_CODE tl_get_input(char *buffer, size_t buffer_size,
                                   const char *prompt);
/**
 * Predefine input for `tl_readline()`.
 */
TL_DEF void tl_set_predefined_input(const char *str);
/**
 * Read a character without waiting and modify `tl_last_control_sequence`.
 */
TL_DEF TL_STATUS_CODE tl_get_character(char *char_buffer,
                                       size_t char_buffer_size,
                                       const char *prompt);
/**
 * Load history from a file.
 *
 * Returns `TL_SUCCESS`, `-EINVAL` if file is invalid or `-errno` on other
 * failures.
 */
TL_DEF TL_STATUS_CODE tl_history_load(const char *file_path);
/**
 * Dump history to a file, overwriting it.
 *
 * Returns `TL_SUCCESS`, `-EINVAL` if file is invalid or `-errno` on other
 * failures.
 */
TL_DEF TL_STATUS_CODE tl_history_dump(const char *file_path);
/**
 * Returns the number of UTF-8 characters.
 *
 * Since number of bytes can be bigger than amount of characters, regular
 * `strlen()` will not work, and will only return the number of bytes before \0.
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
TL_DEF TL_STATUS_CODE tl_emit_newlines(const char *buffer);
/**
 * Sets a new title for the terminal. Returns -1 and does nothing if stdout is
 * not a tty or amount of bytes written.
 */
TL_DEF TL_STATUS_CODE tl_set_title(const char *title);

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

#define ITL_FILE_OPEN_FOR_READ(path) _open(path, O_RDONLY)
#define ITL_FILE_OPEN_FOR_WRITE(path)                                          \
  _open(path, O_WRONLY | O_CREAT | O_TRUNC, _S_IREAD | _S_IWRITE)
#define ITL_FILE_IS_BAD(file) (file < 0)
#define ITL_FILE_CLOSE        _close

#define ITL_WRITE(fd, buf, size) _write(fd, buf, (unsigned long) size)
#define ITL_READ(fd, buf, size)  _read(fd, buf, (unsigned long) size)
#endif /* !ITL_USE_STDIO */

/* <https://learn.microsoft.com/en-US/troubleshoot/windows-client/shell-experience/command-line-string-limitation>
 */
#define ITL_STRING_MAX_LEN 8191

#define ITL_TTY_IS_TTY() ITL_ISATTY(STDIN_FILENO)

#elif defined ITL_POSIX
#if !defined _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

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

#if !defined TL_USE_STDIO
#define ITL_STDIN  0
#define ITL_STDOUT 1
#define ITL_STDERR 2
#define ITL_FILE   int

#define ITL_ISATTY isatty

#define ITL_FILE_OPEN_FOR_READ(path) open(path, O_RDONLY)
#define ITL_FILE_OPEN_FOR_WRITE(path)                                          \
  open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)
#define ITL_FILE_IS_BAD(file) (file < 0)
#define ITL_FILE_CLOSE        close

#define ITL_WRITE(fd, buf, size) write(fd, buf, (unsigned long) size)
#define ITL_READ(fd, buf, size)  read(fd, buf, (unsigned long) size)
#endif /* !ITL_USE_STDIO */

/* <https://man7.org/linux/man-pages/man3/termios.3.html> */
#define ITL_STRING_MAX_LEN 4095

#define ITL_TTY_IS_TTY() ITL_ISATTY(STDIN_FILENO)
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

#define ITL_FILE_OPEN_FOR_READ(path)  fopen(path, "rb")
#define ITL_FILE_OPEN_FOR_WRITE(path) fopen(path, "wb")
#define ITL_FILE_IS_BAD(file)         (file == NULL)
#define ITL_FILE_CLOSE                fclose

ITL_DEF int
itl_write_impl(FILE *f, const void *buf, size_t size)
{
  size_t written_count = fwrite(buf, (unsigned long) size, 1, f);
  fflush(f);
  return ferror(f) ? -1 : (int) written_count;
}

#define ITL_WRITE(file, buf, size) itl_write_impl(file, buf, size)
#define ITL_READ(file, buf, size)  fread(buf, size, 1, file)
#endif /* ITL_USE_STDIO */

#if defined ITL_WIN32
/* Windows can't read arrow keys otherwise */
#define ITL_READ_BYTE_RAW _getch
#else /* ITL_WIN32 */
ITL_DEF int
ITL_READ_BYTE_RAW(void)
{
  int buf[1];
  return (ITL_READ(ITL_STDIN, buf, 1) != 1) ? -1 : *buf;
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
ITL_NO_RETURN ITL_DEF void
itl_unreachable_impl(const char *file, int line, const char *message)
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
ITL_DEF inline void
itl_do_nothing()
{}
#define ITL_TRACELN(...) itl_do_nothing()
#else                    /* ITL_C89 */
#define ITL_TRACELN(...) /* nothing */
#endif
#endif

#define ITL_MAX(i, j) (((i) > (j)) ? (i) : (j))
#define ITL_MIN(i, j) (((i) < (j)) ? (i) : (j))

/* Do `catch_` if `expr` is not true */
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

ITL_DEF bool
itl_enter_raw_mode_impl(void)
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

  /* TODO: Look at this later. */
  itl_g_original_tty_in_mode = tty_in_mode;
  tty_in_mode = (DWORD) 0;

  itl_g_original_tty_out_mode = tty_out_mode;
#if !defined ITL_NO_WIN_ESCAPES
  tty_out_mode = (DWORD) ENABLE_PROCESSED_INPUT |
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

  ITL_TRY(tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) == 0, return false);
#endif /* ITL_POSIX */
  return true;
}

ITL_DEF bool
itl_exit_raw_mode_impl(void)
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
    ITL_TRY(tcsetattr(STDIN_FILENO, TCSAFLUSH, &itl_g_original_tty_mode) == 0,
            return false);
  }

  return true;
#endif /* ITL_POSIX */
}

TL_DEF TL_STATUS_CODE
tl_enter_raw_mode(void)
{
  ITL_TRY(!itl_g_entered_raw_mode, return TL_SUCCESS);

  ITL_TRY(ITL_TTY_IS_TTY(), return TL_ERROR);

  /* If raw mode failed, restore terminal's state */
  ITL_TRY(itl_enter_raw_mode_impl(), {
    tl_exit_raw_mode();
    return TL_ERROR;
  });

  itl_g_entered_raw_mode = true;

  return TL_SUCCESS;
}

TL_DEF TL_STATUS_CODE
tl_exit_raw_mode(void)
{
  ITL_TRY(itl_g_entered_raw_mode, return TL_SUCCESS);

  ITL_TRY(ITL_TTY_IS_TTY(), return TL_ERROR);
  ITL_TRY(itl_exit_raw_mode_impl(), return TL_ERROR);

  itl_g_entered_raw_mode = false;

  return TL_SUCCESS;
}

ITL_DEF bool
ITL_READ_BYTE(uint8_t *buffer)
{
  int byte = ITL_READ_BYTE_RAW();
#if defined ITL_POSIX
  /* Catch `read()` errors. `_getch()` on Windows does not have error
     returns */
  ITL_TRY(byte != -1, return false);
#endif /* ITL_POSIX */
  ITL_PTR_ASSIGN(buffer, (uint8_t) byte);
  return true;
}

#define ITL_TRY_READ_BYTE(buffer, expr) ITL_TRY(ITL_READ_BYTE(buffer), expr)

#if defined ITL_SUSPEND
#if defined ITL_POSIX
ITL_DEF void
itl_handle_sigcont(int signal_number)
{
  (void) signal_number;
  tl_enter_raw_mode();
}

ITL_DEF void
itl_raise_suspend(void)
{
  tl_exit_raw_mode();
  signal(SIGCONT, itl_handle_sigcont);
  raise(SIGTSTP);
}

#else /* ITL_POSIX */
ITL_NO_RETURN ITL_DEF void
itl_raise_suspend(void)
{
  tl_exit();
  exit(0);
}
#endif
#endif /* ITL_SUSPEND */

ITL_DEF bool itl_g_tty_changed_size = true;

ITL_DEF ITL_THREAD_LOCAL size_t itl_g_alloc_count = 0;

ITL_DEF void *
itl_malloc(size_t size)
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

ITL_DEF void *
itl_realloc(void *block, size_t size)
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

ITL_DEF itl_utf8_t
itl_utf8_new(const uint8_t *bytes, uint8_t size)
{
  itl_utf8_t ch;

  TL_ASSERT(size <= 4);

  memcpy(ch.bytes, bytes, size);
  ch.size = size;

  return ch;
}

#define ITL_UTF8_COPY(dst, src) memcpy(dst, src, sizeof(itl_utf8_t))

ITL_DEF bool
itl_utf8_equal(itl_utf8_t ch1, itl_utf8_t ch2)
{
  TL_ASSERT(ch1.size <= 4 && ch2.size <= 4);

  if (ch1.size != ch2.size ||
      memcmp(ch1.bytes, ch2.bytes, ch1.size * sizeof(uint8_t)) != 0)
  {
    return false;
  }

  return true;
}

ITL_DEF uint8_t
itl_utf8_width(int byte)
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

ITL_DEF itl_utf8_t
itl_utf8_parse(uint8_t first_byte)
{
  uint8_t i, size;
  uint8_t bytes[4];

  if ((size = itl_utf8_width(first_byte)) == 0) { /* invalid character */
    ITL_TRACELN("Invalid UTF-8 sequence '%d'\n", (uint8_t) first_byte);
    return itl_replacement_character;
  }

  bytes[0] = first_byte;

  /* Consequent bytes. */
  for (i = 1; i < size; ++i) {
    ITL_TRY_READ_BYTE(&bytes[i], return itl_replacement_character);
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

ITL_DEF void
itl_string_init(itl_string_t *str)
{
  str->length = 0;
  str->size = 0;

  str->capacity = ITL_STRING_INIT_SIZE;
  str->chars = (itl_utf8_t *) itl_malloc(str->capacity * sizeof(itl_utf8_t));
}

ITL_DEF itl_string_t *
itl_string_alloc(void)
{
  itl_string_t *ptr = (itl_string_t *) itl_malloc(sizeof(itl_string_t));
  itl_string_init(ptr);
  return ptr;
}

ITL_DEF void
itl_string_extend(itl_string_t *str)
{
  str->capacity = ITL_STRING_REALLOC_CAPACITY(str->capacity);
  str->chars = (itl_utf8_t *) itl_realloc(str->chars,
                                          str->capacity * sizeof(itl_utf8_t));
}

/* Returns length of the matching prefix */
ITL_DEF size_t
itl_string_prefix_with_offset(const itl_string_t *str1, size_t start,
                              size_t end, const itl_string_t *str2)
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

ITL_DEF bool
itl_string_equal(const itl_string_t *str1, const itl_string_t *str2)
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

ITL_DEF void
itl_string_copy(itl_string_t *dst, const itl_string_t *src)
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

ITL_DEF void
itl_string_recalc_size(itl_string_t *str)
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

/* Shrinks string to capacity of ITL_STRING_INIT_SIZE */
ITL_DEF void
itl_string_shrink(itl_string_t *str)
{
  str->capacity = ITL_STRING_INIT_SIZE;
  str->chars = (itl_utf8_t *) itl_realloc(str->chars,
                                          str->capacity * sizeof(itl_utf8_t));

  if (str->length > str->capacity) {
    str->length = str->capacity;
  }

  itl_string_recalc_size(str);
}

ITL_DEF void
itl_string_clear(itl_string_t *str)
{
  str->size = 0;
  str->length = 0;
  itl_string_shrink(str);
}

/* Shifts all characters after `position`. When shifting forward, character on
   `position` is duplicated `shift_by` times. Does not recalculate the size */
ITL_DEF void
itl_string_shift(itl_string_t *str, size_t position, size_t shift_by,
                 bool backwards)
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

ITL_DEF void
itl_string_erase(itl_string_t *str, size_t position, size_t count,
                 bool backwards)
{
  ITL_TRACELN("string_erase: pos: %zu, count: %zu, backwards: %d, len %zu\n",
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
    if (position >= str->length) {
      return;
    }
    position += count;
  }

  /* Erase the characters by shifting */
  itl_string_shift(str, position, count, true);
  itl_string_recalc_size(str);
}

ITL_DEF void
itl_string_insert(itl_string_t *str, size_t position, itl_utf8_t ch)
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
  itl_string_recalc_size(str);
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

ITL_DEF TL_STATUS_CODE
itl_string_to_cstr(const itl_string_t *str, char *cstr, size_t cstr_size)
{
  size_t i, j, k;

  for (i = 0, k = 0; i < str->length; ++i) {
    /* FIXME: This sometimes explodes. */
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

ITL_DEF bool
itl_string_from_bytes(itl_string_t *str, const char *data, size_t size)
{
  size_t i, j, k;
  uint8_t rune_width;

  for (i = 0, k = 0; k < size; ++i) {
    while (str->capacity < i + 1) {
      itl_string_extend(str);
    }

    rune_width = itl_utf8_width(data[k]);
    if (rune_width == 0) {
      return false; /* Something went wrong. */
    }

    str->chars[i].size = rune_width;

    for (j = 0; j < rune_width && k < size; ++j, ++k) {
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

ITL_DEF const itl_utf8_t itl_space = {{0x20}, 1};

typedef struct itl_history_item itl_history_item_t;

struct itl_history_item
{
  itl_string_t *str;
  itl_history_item_t *next;
  itl_history_item_t *prev;
};

ITL_DEF ITL_THREAD_LOCAL itl_history_item_t *itl_g_history_last = NULL;
ITL_DEF ITL_THREAD_LOCAL itl_history_item_t *itl_g_history_first = NULL;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_history_length = 0;

ITL_DEF ITL_THREAD_LOCAL itl_string_t itl_g_line_buffer = ITL_ZERO_INIT;

typedef struct itl_le itl_le_t;

/* Line editor */
struct itl_le
{
  /* Contents of the line */
  itl_string_t *line;
  size_t cursor_position;

  /* Whether unsubmitted line was already appended to history */
  bool appended_to_history;
  itl_history_item_t *history_selected_item;

  char *out_buf;
  size_t out_size;

  const char *prompt;
  size_t prompt_size;
};

ITL_DEF itl_history_item_t *
itl_history_item_alloc(const itl_string_t *str)
{
  itl_history_item_t *item =
      (itl_history_item_t *) itl_malloc(sizeof(itl_history_item_t));

  item->next = NULL;
  item->prev = NULL;

  item->str = itl_string_alloc();
  itl_string_copy(item->str, str);

  return item;
}

#define ITL_HISTORY_ITEM_FREE(item)                                            \
  do {                                                                         \
    ITL_STRING_FREE((item)->str);                                              \
    ITL_FREE(item);                                                            \
  } while (0)

ITL_DEF void
itl_g_history_free(void)
{
  itl_history_item_t *item, *prev_item;

  if ((item = itl_g_history_last) == NULL) {
    return;
  }

  while (item->next) {
    item = item->next;
  }
  while (item) {
    prev_item = item->prev;
    ITL_HISTORY_ITEM_FREE(item);
    item = prev_item;
  }

  itl_g_history_length = 0;

  itl_g_history_last = NULL;
  itl_g_history_first = NULL;
}

ITL_DEF bool
itl_g_history_append(const itl_string_t *str)
{
  /* If history size was exceeded, release the last item first */
  if (itl_g_history_length >= TL_HISTORY_MAX_SIZE) {
    if (itl_g_history_first) {
      itl_history_item_t *next_item = itl_g_history_first->next;
      ITL_HISTORY_ITEM_FREE(itl_g_history_first);

      itl_g_history_first = next_item;

      if (itl_g_history_first) {
        itl_g_history_first->prev = NULL;
      }

      itl_g_history_length -= 1;
    }
  }

  if (itl_g_history_last == NULL) {
    itl_g_history_last = itl_history_item_alloc(str);
    itl_g_history_first = itl_g_history_last;
  } else {
    itl_history_item_t *item;

    /* Do not append the same string */
    if (itl_string_equal(itl_g_history_last->str, str)) {
      return false;
    }

    item = itl_history_item_alloc(str);
    item->prev = itl_g_history_last;
    itl_g_history_last->next = item;
    itl_g_history_last = item;
  }

  itl_g_history_length += 1;

  return true;
}

ITL_DEF void
itl_le_init(itl_le_t *le, itl_string_t *line_buf, char *out_buf,
            size_t out_size, const char *prompt)
{
  /* clang-format off */
  le->line                  = line_buf;
  le->cursor_position       = line_buf->length;
  le->appended_to_history   = false;
  le->history_selected_item = NULL;
  le->out_buf               = out_buf;
  le->out_size              = out_size;
  le->prompt                = prompt;
  le->prompt_size           = (prompt != NULL) ? strlen(prompt) : 0;
  /* clang-format on */
}

ITL_DEF void
itl_le_move_right(itl_le_t *le, size_t steps)
{
  if (le->cursor_position + steps >= le->line->length) {
    le->cursor_position = le->line->length;
  } else {
    le->cursor_position += steps;
  }
}

ITL_DEF void
itl_le_move_left(itl_le_t *le, size_t steps)
{
  if (steps <= le->cursor_position) {
    le->cursor_position -= steps;
  } else {
    le->cursor_position = 0;
  }
}

ITL_DEF void
itl_le_erase(itl_le_t *le, size_t count, bool backwards)
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

#define ITL_LE_ERASE_FORWARD(le, count)  itl_le_erase(le, count, false)
#define ITL_LE_ERASE_BACKWARD(le, count) itl_le_erase(le, count, true)

/* Inserts character at cursor position */
ITL_DEF bool
itl_le_insert(itl_le_t *le, itl_utf8_t ch)
{
  ITL_TRY(le->line->size + ch.size < le->out_size, return false);

  itl_string_insert(le->line, le->cursor_position, ch);
  itl_le_move_right(le, 1);

  return true;
}

#define ITL_CHAR_IS_DELIM(c) (ispunct(c))
#define ITL_CHAR_IS_SPACE(c) (isspace(c))

typedef enum
{
  ITL_TOKEN_DELIM = 0,
  ITL_TOKEN_WORD = 1,
  ITL_TOKEN_SPACE = 2,
} ITL_TOKEN_KIND;

/* Returns amount of steps required to reach next/previos token */
ITL_DEF size_t
itl_string_steps_to_token(const itl_string_t *str, size_t position,
                          bool backwards)
{
  uint8_t b;
  bool should_break = false;
  size_t i = position, steps = 0;

  ITL_TOKEN_KIND token_kind;

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

ITL_DEF void
itl_le_clear_line(itl_le_t *le)
{
  itl_string_clear(le->line);
  le->cursor_position = 0;
}

ITL_DEF void
itl_g_history_get_prev(itl_le_t *le)
{
  if (itl_g_history_last == NULL) {
    return;
  }

  if (le->history_selected_item) {
    if (le->history_selected_item->prev) {
      le->history_selected_item = le->history_selected_item->prev;
    }
  } else {
    le->history_selected_item = itl_g_history_last;
  }

  TL_ASSERT(le->history_selected_item);

  itl_le_clear_line(le);
  itl_string_copy(le->line, le->history_selected_item->str);
  le->cursor_position = le->line->length;
}

ITL_DEF void
itl_g_history_get_next(itl_le_t *le)
{
  if (le->history_selected_item && le->history_selected_item->next) {
    le->history_selected_item = le->history_selected_item->next;
    itl_le_clear_line(le);
    itl_string_copy(le->line, le->history_selected_item->str);
    le->cursor_position = le->line->length;
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

ITL_DEF void
itl_char_buf_init(itl_char_buf_t *cb)
{
  cb->size = 0;
  cb->capacity = ITL_CHAR_BUFFER_INIT_SIZE;
  cb->data = (char *) itl_malloc(sizeof(char) * cb->capacity);
}

ITL_DEF itl_char_buf_t *
itl_char_buf_alloc(void)
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

ITL_DEF void
itl_char_buf_extend(itl_char_buf_t *cb)
{
  cb->capacity = ITL_CHAR_BUF_REALLOC_CAPACITY(cb->capacity);
  cb->data = (char *) itl_realloc(cb->data, cb->capacity);
}

ITL_DEF void
itl_char_buf_append_cstr(itl_char_buf_t *cb, const char *cstr)
{
  size_t new_size, j;

  for (new_size = cb->size, j = 0; cstr[j] != '\0'; ++j, ++new_size) {
    while (cb->capacity <= new_size) {
      itl_char_buf_extend(cb);
    }
    cb->data[new_size] = cstr[j];
  }

  cb->size = new_size;
}

ITL_DEF void
itl_char_buf_append_size_t(itl_char_buf_t *cb, size_t n)
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
  for (i = new_size - 1; i >= cb->size; --i) {
    cb->data[i] = (char) (n % 10) + '0';
    n /= 10;
  }

  cb->size = new_size;
}

ITL_DEF TL_STATUS_CODE
itl_char_buf_append_string(itl_char_buf_t *cb, const itl_string_t *str)
{
  char *data;

  while (cb->capacity < cb->size + str->size) {
    itl_char_buf_extend(cb);
  }

  data = cb->data + (cb->size * sizeof(char));
  ITL_TRY(itl_string_to_cstr(str, data, str->size + 1) == TL_SUCCESS,
          return TL_ERROR_SIZE);
  cb->size += str->size; /* Ignore null at the end */

  return TL_SUCCESS;
}

ITL_DEF void
itl_char_buf_append_byte(itl_char_buf_t *cb, uint8_t data)
{
  while (cb->capacity < cb->size + 1) {
    itl_char_buf_extend(cb);
  }

  cb->data[cb->size] = (char) data;
  cb->size += 1;
}

#define ITL_CHAR_BUF_CLEAR(cb) (cb)->size = 0

#define ITL_CHAR_BUF_DUMP(cb) ITL_WRITE(ITL_STDOUT, (cb)->data, (cb)->size)

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
    itl_char_buf_append_byte(buffer, 'C')                                      \
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

#define ITL_TTY_GOTO_HOME(buffer) itl_char_buf_append_cstr(buffer, "\x1b[H")

#define ITL_TTY_ERASE_SCREEN(buffer) itl_char_buf_append_cstr(buffer, "\033[2J")

#define ITL_TTY_STATUS_REPORT(buffer)                                          \
  itl_char_buf_append_cstr(buffer, "\x1b[6n")

/* If this is true, do not overwrite file on `history_dump_to_file()` */
ITL_DEF ITL_THREAD_LOCAL bool itl_g_history_file_is_bad = false;

#define ITL_HISTORY_FILE_EXPLOSION()                                           \
  do {                                                                         \
    itl_g_history_free();                                                      \
    itl_g_history_file_is_bad = true;                                          \
    ret = TL_ERROR;                                                            \
    goto end;                                                                  \
  } while (0)

#define ITL_HISTORY_FILE_BUFFER_SIZE (1024 * 2)

/* Returns TL_SUCCESS, -EINVAL on invalid file, or -errno on other errors */
ITL_DEF TL_STATUS_CODE
itl_history_load_from_file(const char *path)
{
  ITL_FILE file;
  bool is_eof = false;

  char file_buffer[ITL_HISTORY_FILE_BUFFER_SIZE];

  itl_string_t *str = itl_string_alloc();
  itl_char_buf_t *cb = itl_char_buf_alloc();

  int read_amount = 0;
  TL_STATUS_CODE ret = TL_SUCCESS;

  size_t line = 1;
  (void) line;

  itl_g_history_free();
  itl_g_history_file_is_bad = false;

  file = ITL_FILE_OPEN_FOR_READ(path);
  if (ITL_FILE_IS_BAD(file)) {
    ITL_TRACELN("could not open history file for load (%s): %s\n", path,
                strerror(errno));
    /* Do not mark file as bad if it does not exist. `dump_to_file` will
       create it. */
    if (errno != ENOENT) {
      itl_g_history_file_is_bad = true;
    }
    ret = TL_ERROR;
    goto end;
  }

  is_eof = false;

  while (!is_eof) {
    size_t file_buffer_pos = 0;

    size_t pos = 0;
    (void) pos;

    read_amount =
        (int) ITL_READ(file, file_buffer, ITL_HISTORY_FILE_BUFFER_SIZE);

    if (read_amount <= 0) {
#if defined TL_USE_STDIO
      is_eof = feof(file);
#else /* TL_USE_STDIO */
      is_eof = (read_amount == 0);
#endif
      if (!is_eof) {
        ITL_HISTORY_FILE_EXPLOSION();
      }

      break;
    }

    TL_ASSERT(read_amount > 0);

    while (file_buffer_pos < (size_t) read_amount) {
      int ch = file_buffer[file_buffer_pos];

      if (ch == '\r') {
        /* TODO: Multiline support for history. */
        continue;
      } else if (ch == '\n') {
        /* TODO: Here long lines are silently truncated. */
        if (!itl_string_from_bytes(str, cb->data,
                                   ITL_MIN(cb->size, ITL_STRING_MAX_LEN)))
        {
          ITL_TRACELN("incorrect calculated string size in history file "
                      "at %zu:%zu\n",
                      line, pos);

          errno = EINVAL;
          ITL_HISTORY_FILE_EXPLOSION();
        }

        itl_g_history_append(str);
        ITL_TRACELN("loaded history entry: %.*s\n", (int) cb->size, cb->data);
        ITL_CHAR_BUF_CLEAR(cb);

        pos = 0;

        line++;
        /* Loaded a binary file on accident? */
      } else if (iscntrl(ch) && !isspace(ch)) {
        ITL_TRACELN("non-text byte '%X' detected in history file at %zu:%zu\n",
                    (uint8_t) ch, line, pos);

        errno = EINVAL;
        ITL_HISTORY_FILE_EXPLOSION();
      } else {
        itl_char_buf_append_byte(cb, (uint8_t) ch);
      }

      file_buffer_pos++;
      pos++;
    }
  }

end:
  if (!ITL_FILE_IS_BAD(file)) {
    ITL_FILE_CLOSE(file);
  }
  ITL_CHAR_BUF_FREE(cb);
  ITL_STRING_FREE(str);

  return ret;
}

/* Returns TL_SUCCESS, -EINVAL on invalid file, or -errno on other errors */
ITL_DEF TL_STATUS_CODE
itl_history_dump_to_file(const char *path)
{
  ITL_FILE file;
  itl_char_buf_t *buffer = NULL;
  itl_history_item_t *item = NULL, *next_item = NULL;
  TL_STATUS_CODE ret = TL_SUCCESS;

  TL_ASSERT(itl_g_is_active && "Dump history before calling tl_exit()!");

  if (itl_g_history_file_is_bad) {
    errno = EINVAL;
    return TL_ERROR;
  }

  buffer = itl_char_buf_alloc();

  file = ITL_FILE_OPEN_FOR_WRITE(path);
  if (ITL_FILE_IS_BAD(file)) {
    ITL_TRACELN("could not open history file for dump (%s): %s\n", path,
                strerror(errno));
    ret = TL_ERROR;
    goto end;
  }

  item = itl_g_history_first;
  if (item == NULL) {
    goto end;
  }

  while (item->prev != NULL) {
    item = item->prev;
  }
  while (item) {
    next_item = item->next;
    ITL_TRY(itl_char_buf_append_string(buffer, item->str) == TL_SUCCESS, {
      ret = TL_ERROR_SIZE;
      goto end;
    });
    if (item->str->length > 1) {
      if (ITL_WRITE(file, buffer->data, buffer->size) == -1 ||
          ITL_WRITE(file, "\n", 1) == -1)
      {
        ret = TL_ERROR;
        goto end;
      }
    }
    ITL_CHAR_BUF_CLEAR(buffer);
    item = next_item;
  }

end:
  if (!ITL_FILE_IS_BAD(file)) {
    ITL_FILE_CLOSE(file);
  }
  ITL_CHAR_BUF_FREE(buffer);

  return ret;
}

ITL_DEF size_t
itl_parse_size(const char *cstr, size_t *result)
{
  size_t i, number;

  for (i = 0, number = 0; i < strlen(cstr); ++i) {
    if (!isdigit(cstr[i])) {
      break;
    }
    number *= 10;
    number += (size_t) (cstr[i] - '0');
  }

  ITL_PTR_ASSIGN(result, number);

  return i;
}

#ifdef ITL_POSIX
ITL_DEF int
itl_esc_parse_posix(uint8_t byte)
{
  int event = 0;
  bool read_mod = false;

  if (byte == 27) { /* esc */
    ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);

    if (byte != '[' && byte != 'O') {
      switch (byte) {
      case 'b': return TL_KEY_LEFT | TL_MOD_CTRL;
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

    ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);

    if (byte == '1') {
      ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);
      if (byte != ';') {
        return TL_KEY_UNKN;
      }

      ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);
      switch (byte) {
      case '2': event |= TL_MOD_SHIFT; break;
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

    default: event |= TL_KEY_UNKN;
    }
  } else {
    ITL_TRY(!iscntrl(byte), return TL_KEY_UNKN);
    return TL_KEY_CHAR;
  }

  if (!read_mod) {
    ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);
    if (byte == ';') {
      ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);
      switch (byte) {
      case '3': event |= TL_MOD_SHIFT; break;
      case '5': event |= TL_MOD_CTRL; break;
      }
      ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);
    }

    ITL_TRY(byte == '~', return TL_KEY_UNKN);
  }

  return event;
}
#endif /* ITL_POSIX */

#ifdef ITL_WIN32
ITL_DEF int
itl_esc_parse_win32(uint8_t byte)
{
  int event = 0;

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

ITL_DEF int
itl_esc_parse(uint8_t byte)
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

ITL_DEF ITL_THREAD_LOCAL itl_char_buf_t itl_g_char_buffer = ITL_ZERO_INIT;
ITL_DEF ITL_THREAD_LOCAL bool itl_g_tty_is_dumb = true;

/* *le, *rows, *cols can be NULL. */
ITL_DEF bool
itl_tty_get_size(ITL_MAYBE_UNUSED itl_le_t *le, size_t *rows, size_t *cols)
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

  if (itl_g_tty_is_dumb) {
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
  itl_tty_move_forward(b, 999);
  itl_tty_status_report(b);
  itl_char_buf_dump(b);
  itl_char_buf_clear(b);

  /* There might be pasted input awaiting to be processed. Read and parse all
     bytes until escape is encountered. */
  first = &size_buf[0];
  while (true) {
    ITL_TRY_READ_BYTE((uint8_t *) first, return false);
    if (*first == '\x1b') {
      break;
    }
    /* don't print control sequences if they got pasted */
    if (itl_esc_parse(*first) != TL_KEY_CHAR) {
      continue;
    }
    if (le != NULL) {
      itl_le_insert(le, itl_utf8_parse(*first));
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

/* Line editor's contents during previous refresh() call. */
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_le_prev_rows = 1;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_le_prev_cursor_rows = 1;

ITL_DEF ITL_THREAD_LOCAL size_t itl_g_le_prev_length = 0;

/* $COLUMNS and $LINES, same as above. */
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_tty_prev_rows = 1;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_tty_prev_cols = 1;

/* NOTE: Hottest function in the library. */
ITL_DEF bool
itl_le_tty_refresh(itl_le_t *le)
{
  size_t i, j, tty_rows, tty_cols;
  size_t current_cols, le_row_amount, dirty_lines;
  size_t le_cursor_column, le_cursor_rows;
  size_t extra_rows_to_delete;

  /* Write everything into a buffer, then dump it all at once */
  itl_char_buf_t *b;

  TL_ASSERT(le->line);
  TL_ASSERT(le->line->chars);
  TL_ASSERT(le->line->size >= le->line->length);
  TL_ASSERT(le->line->length <= ITL_STRING_MAX_LEN);

  /* FIXME: Properly refresh on size change. */
  extra_rows_to_delete = 0;

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

  le_row_amount =
      (le->line->length + le->prompt_size) / ITL_MAX(tty_cols, 1) + 1;

  le_cursor_column =
      (le->cursor_position + le->prompt_size) % ITL_MAX(tty_cols, 1) + 1;
  le_cursor_rows =
      (le->cursor_position + le->prompt_size) / ITL_MAX(tty_cols, 1) + 1;

  ITL_TRACELN("sr: %d, er: %zu, wrow: %zu, prev: %zu, col: %zu, curp: %zu\n",
              itl_g_tty_should_refresh_text, extra_rows_to_delete,
              le_cursor_rows, itl_g_le_prev_rows, le_cursor_column,
              le->cursor_position);

  b = &itl_g_char_buffer;
  ITL_TTY_HIDE_CURSOR(b);

  /* FIXME: Fix refreshing artifacts after killing more than one line or
   * resizing the terminal. */

  if (itl_g_tty_should_refresh_text) {
    /* Move appropriate amount of lines back, while clearing previous output
     */
    for (i = 0; i < itl_g_le_prev_rows + extra_rows_to_delete; ++i) {
      ITL_TTY_CLEAR_WHOLE_LINE(b);
      if (i < itl_g_le_prev_cursor_rows + extra_rows_to_delete - 1) {
        ITL_TTY_MOVE_UP(b, 1);
      }
    }

    if (le->prompt) {
      itl_char_buf_append_cstr(b, le->prompt);
    }

    /* Print current contents of the line editor */
    for (i = 0; i < le->line->length; ++i) {
      for (j = 0; j < le->line->chars[i].size; ++j) {
        itl_char_buf_append_byte(b, le->line->chars[i].bytes[j]);
      }

      /* If line is full, wrap */
      current_cols = (le->prompt_size + i) % ITL_MAX(tty_cols, 1);
      if (current_cols == tty_cols - 1) {
        itl_char_buf_append_cstr(b, ITL_LF);
      }
    }

    /* If current amount of lines is less than previous amount of lines,
       then input was cleared by kill line or such. Clear each dirty line,
       then go back up */
    if (le_row_amount < itl_g_le_prev_rows) {
      dirty_lines = itl_g_le_prev_rows - le_row_amount;
      for (i = 0; i < dirty_lines; ++i) {
        ITL_TTY_MOVE_DOWN(b, 1);
        ITL_TTY_CLEAR_WHOLE_LINE(b);
      }
      ITL_TTY_MOVE_UP(b, dirty_lines);
    } else {
      /* Otherwise clear to the end of line */
      ITL_TTY_CLEAR_TO_END(b);
    }

    /* Move cursor to appropriate row and column. If row didn't change, stay
       on the same line */
    if (le_cursor_rows < le_row_amount) {
      ITL_TTY_MOVE_UP(b, le_row_amount - le_cursor_rows);
    }
  } else {
    if (le_cursor_rows < itl_g_le_prev_cursor_rows) {
      ITL_TTY_MOVE_UP(b, itl_g_le_prev_cursor_rows - le_cursor_rows);
    } else if (le_cursor_rows > itl_g_le_prev_cursor_rows) {
      ITL_TTY_MOVE_DOWN(b, le_cursor_rows - itl_g_le_prev_cursor_rows);
    }
  }

  ITL_TTY_MOVE_TO_COLUMN(b, le_cursor_column);

  itl_g_le_prev_rows = le_row_amount;
  itl_g_le_prev_cursor_rows = le_cursor_rows;
  itl_g_le_prev_length = le->line->length + le->prompt_size;

  itl_g_tty_prev_rows = tty_rows;
  itl_g_tty_prev_cols = tty_cols;

#if defined ITL_POSIX
  itl_g_tty_changed_size = false;
#else
  /* Windows does not present anything useful and is a bad OS. */
  itl_g_tty_changed_size = true;
#endif
  ITL_TTY_SHOW_CURSOR(b);

  ITL_CHAR_BUF_DUMP(b);
  ITL_CHAR_BUF_CLEAR(b);

  return true;
}

ITL_DEF ITL_THREAD_LOCAL itl_le_t itl_g_le = ITL_ZERO_INIT;

#if defined ITL_POSIX
ITL_DEF void
itl_handle_sigwinch(int signal_number)
{
  if (signal_number != SIGWINCH) {
    return;
  }
  itl_g_tty_changed_size = true;
  itl_g_tty_should_refresh_text = true;
}
#endif

ITL_DEF ITL_THREAD_LOCAL int itl_g_last_control = TL_KEY_UNKN;

TL_DEF int
tl_last_control_sequence(void)
{
  return itl_g_last_control;
}

ITL_DEF TL_STATUS_CODE
itl_le_key_handle(itl_le_t *le, int esc)
{
  /* Remember the last control sequence. */
  itl_g_last_control = esc;

  /* Refresh text by default, avoid if we are only moving the cursor. */
  itl_g_tty_should_refresh_text = true;

  switch (esc & TL_MASK_KEY) {
  case TL_KEY_TAB: {
    ITL_TRY(itl_string_to_cstr(le->line, le->out_buf, le->out_size) ==
                TL_SUCCESS,
            return TL_ERROR_SIZE);
    return TL_PRESSED_TAB;
  } break;

  case TL_KEY_UP: {
    itl_string_t *prev_line;

    if (!le->appended_to_history) {
      prev_line = itl_string_alloc();
      itl_string_copy(prev_line, le->line);
      itl_g_history_get_prev(le);
      /* Avoid appending same strings or empty strings */
      if (!itl_string_equal(le->line, prev_line) && prev_line->length > 0) {
        itl_g_history_append(prev_line);
      }
      ITL_STRING_FREE(prev_line);
      le->appended_to_history = true;
    } else if (itl_g_history_last != NULL &&
               itl_g_history_last == le->history_selected_item)
    {
      /* If some string was already appended, just update it */
      itl_string_copy(itl_g_history_last->str, le->line);
      itl_g_history_get_prev(le);
    } else {
      itl_g_history_get_prev(le);
    }
  } break;

  case TL_KEY_DOWN: {
    itl_g_history_get_next(le);
  } break;

  case TL_KEY_RIGHT: {
    bool cursor_was_on_space;
    if (le->cursor_position < le->line->length) {
      if (esc & TL_MOD_CTRL) {
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
      if (esc & TL_MOD_CTRL) {
        cursor_was_on_space = ITL_LE_CURSOR_IS_ON_SPACE(le) ||
                              (le->cursor_position == le->line->length);
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
    itl_le_move_right(le, le->line->length - le->cursor_position);
    itl_g_tty_should_refresh_text = false;
  } break;

  case TL_KEY_HOME: {
    itl_le_move_left(le, le->cursor_position);
    itl_g_tty_should_refresh_text = false;
  } break;
  case TL_KEY_ENTER: {
    ITL_TRY(itl_string_to_cstr(le->line, le->out_buf, le->out_size) ==
                TL_SUCCESS,
            return TL_ERROR_SIZE);
    itl_g_history_append(le->line);
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
    ITL_LE_ERASE_FORWARD(le, le->line->length - le->cursor_position);
  } break;

  case TL_KEY_KILL_LINE_BEFORE: {
    ITL_LE_ERASE_BACKWARD(le, le->cursor_position);
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
    size_t i;
    for (i = 0; i < itl_g_history_length; ++i) {
      itl_g_history_get_next(le);
    }
    itl_g_history_get_prev(le);
  } break;

  case TL_KEY_HISTORY_BEGINNING: {
    size_t i;
    for (i = 0; i < itl_g_history_length; ++i) {
      itl_g_history_get_prev(le);
    }
  } break;
  }

  return TL_SUCCESS;
}

TL_DEF TL_STATUS_CODE
tl_init(void)
{
  TL_ASSERT(!(TL_HISTORY_MAX_SIZE & (TL_HISTORY_MAX_SIZE - 1)) &&
            "History size must be a power of 2");
  TL_ASSERT(TL_HISTORY_MAX_SIZE >= 0 && "History size must be positive");

  if (itl_g_is_active) {
    return TL_SUCCESS;
  }

#if defined ITL_POSIX
  signal(SIGWINCH, itl_handle_sigwinch);
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

TL_DEF TL_STATUS_CODE
tl_exit(void)
{
  TL_ASSERT(itl_g_is_active && "tl_init() should be called");

  itl_g_history_free();
  ITL_FREE(itl_g_line_buffer.chars);
  ITL_FREE(itl_g_char_buffer.data);

  ITL_TRACELN("Exited, alloc count: %zu\n", itl_g_alloc_count);
  TL_ASSERT(itl_g_alloc_count == 0);

  if (itl_g_entered_raw_mode) {
    ITL_TRY(tl_exit_raw_mode() == TL_SUCCESS, return TL_ERROR);
  }

  itl_g_is_active = false;

  return TL_SUCCESS;
}

TL_DEF TL_STATUS_CODE
tl_get_input(char *buffer, size_t buffer_size, const char *prompt)
{
  itl_le_t *le = &itl_g_le;
  uint8_t input_byte;
  int input_type;

  TL_STATUS_CODE code;

  TL_ASSERT(itl_g_is_active && "tl_init() should be called");
  TL_ASSERT(
      buffer_size > 1 &&
      "Size should be enough at least for one byte and a null terminator");
  TL_ASSERT(
      buffer_size <= ITL_STRING_MAX_LEN &&
      "Size should be less than platform's allowed maximum string length");
  TL_ASSERT(buffer != NULL);

  itl_le_init(le, &itl_g_line_buffer, buffer, buffer_size, prompt);

  /* Avoid clearing lines that don't belong to us. */
  itl_g_le_prev_rows = 1;
  itl_g_le_prev_cursor_rows = 1;
  itl_le_tty_refresh(le);

  while (true) {
    ITL_TRY_READ_BYTE(&input_byte, return TL_ERROR);

#if defined ITL_POSIX
    /* A refresh may be pending due to SIGWINCH. */
    if (itl_g_tty_changed_size) {
      itl_le_tty_refresh(le);
    }
#endif /* ITL_POSIX */

#if defined TL_SEE_BYTES
    if (input_byte == 3) return -69; /* ctrl c */
    if (iscntrl((char) input_byte) || input_byte > 127) {
      printf("cntrl seq -> ");
    } else {
      printf("'%c' -> ", (char) input_byte);
    }
    printf("%d\n", input_byte);
    fflush(stdout);
    continue;
#endif /* TL_SEE_BYTES */

    input_type = itl_esc_parse(input_byte);
    if (input_type != TL_KEY_CHAR) {
      code = itl_le_key_handle(le, input_type);
      if (code != TL_SUCCESS) {
        itl_le_clear_line(le);
        return code;
      }
    } else {
      itl_le_insert(le, itl_utf8_parse(input_byte));
      itl_g_tty_should_refresh_text = true;
    }

    ITL_TRACELN("strlen: %zu, hist: %zu\n", le->line->length,
                (size_t) le->history_selected_item);
    itl_le_tty_refresh(le);
  }

  ITL_UNREACHABLE();
}

TL_DEF void
tl_set_predefined_input(const char *str)
{
  TL_ASSERT(itl_g_is_active && "tl_init() should be called");
  itl_string_shrink(&itl_g_line_buffer);
  ITL_STRING_FROM_CSTR(&itl_g_line_buffer, str);
}

TL_DEF TL_STATUS_CODE
tl_get_character(char *char_buffer, size_t char_buffer_size, const char *prompt)
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

  /* Avoid overriding buffer if tl_setline was used */
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

TL_DEF TL_STATUS_CODE
tl_history_load(const char *file_path)
{
  return itl_history_load_from_file(file_path);
}

TL_DEF TL_STATUS_CODE
tl_history_dump(const char *file_path)
{
  return itl_history_dump_to_file(file_path);
}

TL_DEF size_t
tl_utf8_strlen(const char *utf8_str)
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

TL_DEF size_t
tl_utf8_strnlen(const char *utf8_str, size_t byte_count)
{
  size_t len = 0;
  while (*utf8_str != '\0' && byte_count-- > 0) {
    if ((*utf8_str & 0xC0) != 0x80) {
      len += 1;
    }
    utf8_str += 1;
  }
  return len;
}

TL_DEF TL_STATUS_CODE
tl_emit_newlines(const char *char_buffer)
{
  size_t cols, i, newlines_to_emit;

  ITL_TRY(itl_tty_get_size(NULL, NULL, &cols), return TL_ERROR);

  newlines_to_emit =
      (tl_utf8_strlen(char_buffer) / cols + 1) - itl_g_le_prev_cursor_rows + 1;

  for (i = 0; i < newlines_to_emit; ++i) {
    ITL_TRY(ITL_WRITE(ITL_STDOUT, "\n", 1) != -1, return TL_ERROR);
  }

  return TL_SUCCESS;
}

TL_DEF TL_STATUS_CODE
tl_set_title(const char *title)
{
  if (ITL_ISATTY(ITL_STDOUT)) {
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
 * TODO (not soon):
 *  - History search.
 *  - Replace macros with enums.
 *  - Support multiple lines simultaneously.
 *  - Use Windows' console API instead of terminal sequences on Windows.
 */
