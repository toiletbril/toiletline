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

/* If defined, Ctrl-Z undoes while a line is read, the same as Ctrl-_, and
 * neither suspends nor returns `TL_PRESSED_SUSPEND`. Define it with
 * `TL_NO_SUSPEND` in a host that leaves job control to the terminal while its
 * own programs run. Ctrl-Shift-Z redoes either way when the terminal reports
 * it apart from Ctrl-Z, which tl_set_extended_keys() asks the terminal for. */

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
/* Will be called on failed allocation. */
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

  /* Ctrl-Y inserts the newest kill and Alt-Y replaces it with an older one. */
  TL_KEY_YANK,
  TL_KEY_YANK_POP,
  /* Ctrl-T transposes characters, and with TL_MOD_ALT, words. */
  TL_KEY_TRANSPOSE,
  /* Alt-. inserts the last word of a previous history entry. */
  TL_KEY_LAST_ARGUMENT,
  /* Ctrl-X Ctrl-E hands the line to the edit callback. */
  TL_KEY_EDIT_EXTERNAL,

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
 * Put the terminal back the way raw mode found it and withdraw every request
 * raw mode made, from a fatal signal handler or an exit hook. It calls only
 * async-signal-safe functions and changes no editor state, so the process is
 * expected to end after it. POSIX only.
 */
TL_DEF void tl_restore_terminal_for_exit(void);
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
  /* Nonzero when the host offered only the best of its exact prefix,
     smart-case prefix, and subsequence tiers. An open menu then narrows by the
     same tiers. Zero for a list the host did not rank, which the menu narrows
     by prefix and keeps whole. */
  int is_tier_ranked;
  /* Nonzero when an accepted candidate takes no trailing space even with
     tl_set_space_after_completion on, the way a bash spec with -o nospace
     completes a word the user keeps typing. */
  int is_space_suppressed;
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

/**
 * The history selector callback. The host receives the matching history
 * entries, newest first, and picks one with a program of its own. It returns 1
 * when it wrote the chosen entry to out_selected, 0 when it ran nothing and the
 * editor is to search on its own, and a negative value when its selector ran
 * and the user chose nothing. The chosen string stays valid until the next
 * call.
 */
typedef int (*tl_history_select_fn)(const char *const *entries, size_t count,
                                    const char **out_selected);

/**
 * Register the history selector callback, or NULL to keep ctrl-R inside the
 * editor.
 */
TL_DEF void tl_set_history_select_callback(tl_history_select_fn callback);

/**
 * The history search snapshot callback. The host returns a complete encoded
 * history snapshot whose bytes stay valid until the next callback. A nonzero
 * result uses that snapshot for ctrl-R without changing ordinary history.
 */
typedef int (*tl_history_search_snapshot_fn)(const char **out_contents,
                                             size_t *out_size);

/** Register the snapshot callback, or NULL to search ordinary history. */
TL_DEF void tl_set_history_search_snapshot_callback(
    tl_history_search_snapshot_fn callback);

/**
 * Temporarily hand the terminal to an interactive program while tl_get_input()
 * is active. The begin call clears the editor-owned input block and restores
 * cooked mode. The end call re-enters raw mode and invalidates the saved render
 * so the next editor refresh repaints the prompt and draft from scratch.
 */
TL_DEF tl_status_code tl_begin_external_screen(void);
TL_DEF tl_status_code tl_end_external_screen(void);

/**
 * The edit callback for Ctrl-X Ctrl-E. The host receives the current buffer
 * and edits it with a program of its own, handing the terminal over through
 * tl_begin_external_screen(). It returns 1 when it wrote the edited text to
 * out_edited and 0 to leave the buffer alone. The edited text replaces the
 * whole buffer and is not submitted. It stays valid until the next call.
 */
typedef int (*tl_edit_fn)(const char *buffer, const char **out_edited);

/** Register the edit callback, or NULL to make Ctrl-X Ctrl-E do nothing. */
TL_DEF void tl_set_edit_callback(tl_edit_fn callback);

/**
 * Let the terminal turn the interrupt key back into SIGINT while the editor
 * stays in raw mode. A host callback that runs a slow command turns this on for
 * its duration so the key reaches the command instead of queueing as input.
 */
TL_DEF tl_status_code tl_set_signal_keys(int enabled);

/*
 * Asks the terminal for distinct reports of modified keys while raw mode is on,
 * through the kitty keyboard protocol's disambiguate flag and xterm's
 * modifyOtherKeys level 1. A terminal without either ignores the requests. A
 * key reported in one of those forms that has a legacy encoding is read as the
 * legacy bytes, so every binding behaves the same, and a key without one, such
 * as Ctrl-Shift-Z, becomes distinct. The requests are withdrawn whenever raw
 * mode is left and while signal keys are on, since the kitty form of the
 * interrupt key raises no signal. POSIX only. Disabled by default.
 */
TL_DEF void tl_set_extended_keys(int enabled);

/*
 * Enables or disables the dimmed ghost suggestion shown ahead of the cursor.
 * Enabled by default. When disabled neither completion nor history fills it.
 */
TL_DEF void tl_set_ghost_enabled(int enabled);

/*
 * Makes Up and Down on a non-empty line recall only the history entries that
 * begin with the text typed before the first Up. Down walks back and restores
 * the typed text. An empty line navigates the whole history. Disabled by
 * default.
 */
TL_DEF void tl_set_history_prefix_search(int enabled);

/*
 * Makes a typed (, [, {, " or ' insert its closer after the caret. Typing that
 * closer steps over it, and Backspace between the empty pair deletes both.
 * Disabled by default.
 */
TL_DEF void tl_set_auto_pair(int enabled);

/* The typed byte is plain text at the caret, such as inside a quoted string. */
#define TL_PAIR_NONE 0
/* The typed byte opens a pair, so the editor may insert its closer. */
#define TL_PAIR_OPENS 1
/* The typed byte closes a pair, so it may step over an inserted closer. */
#define TL_PAIR_CLOSES 2

/*
 * The auto-pair callback. The host receives the buffer, the byte offset of the
 * caret, and the typed bracket or quote, and answers TL_PAIR_NONE,
 * TL_PAIR_OPENS, or TL_PAIR_CLOSES from its own syntax. Without a callback a
 * typed opener always opens and a typed closer always closes.
 */
typedef int (*tl_pair_role_fn)(const char *buffer, size_t cursor, int byte);

/**
 * Register the auto-pair callback, or NULL to pair without one.
 */
TL_DEF void tl_set_pair_role_callback(tl_pair_role_fn callback);

typedef enum
{
  TL_SPACE_AFTER_COMPLETION_OFF = 0,
  TL_SPACE_AFTER_COMPLETION_ON = 1,
  TL_SPACE_AFTER_COMPLETION_EXCEPT_AFTER_SLASH = 2,
} tl_space_after_completion;

/**
 * Chooses whether an accepted complete candidate takes a trailing space: never,
 * always, or unless it ends in a path separator. Off by default.
 */
TL_DEF void tl_set_space_after_completion(tl_space_after_completion mode);

/*
 * Enables or disables the selectable candidate menu opened under the prompt
 * when a second TAB finds several candidates. Disabled by default, which keeps
 * the plain printed column list.
 */
TL_DEF void tl_set_completion_menu_enabled(int enabled);

/*
 * Enables or disables the colored parts of what the editor draws. Enabled by
 * default. The host owns the decision, since an environment variable such as
 * NO_COLOR is read by the host and not by the editor. The reversed selection
 * band of the menu survives a disabled setting, since reverse video carries no
 * color of its own.
 */
TL_DEF void tl_set_colors_enabled(int enabled);

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
 * The cursor of a highlight request whose text carries no caret, such as a
 * menu candidate or a reverse search match.
 */
#define TL_HIGHLIGHT_NO_CURSOR ((size_t) -1)

/**
 * The highlight result. The editor provides spans, an array of capacity slots,
 * and the host fills the first count of them. The spans must be sorted by
 * start, non-overlapping, and within the line. cursor is the byte offset of
 * the caret in the buffer, or TL_HIGHLIGHT_NO_CURSOR.
 */
typedef struct tl_highlight
{
  tl_highlight_span *spans;
  size_t count;
  size_t capacity;
  size_t cursor;
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
 * Whether the highlight depends on the caret. When nonzero, a caret move asks
 * the highlight callback again and redraws the line when the spans changed.
 * The default is zero.
 */
TL_DEF void tl_set_highlight_follows_cursor(int follows_cursor);

/**
 * The hint callback. The host receives the buffer and the byte offset of the
 * caret. It returns the text for the rows directly under the input, or NULL or
 * an empty string for no hint. Text with a line break is a header row before
 * the break and a body after it, and text without one is a body alone. Every
 * row is indented by two columns. The header is cut with an ellipsis at the
 * terminal width, and the body wraps at spaces onto at most three rows, the
 * last of which is cut with an ellipsis. Later line breaks and other control
 * bytes in the body become spaces. A terminal too short for the input and
 * every row gets fewer body rows, then the body alone on one row, then no
 * hint, so the input never scrolls away. The host may set *sgr to the escape
 * sequence that styles the rows, and leaves it alone for the dimmed default.
 * Both pointers only need to stay valid until the call returns. The rows are
 * never drawn while a menu or history search is open and are erased when the
 * line is submitted.
 */
typedef const char *(*tl_hint_fn)(const char *buffer, size_t cursor,
                                  const char **sgr);

/**
 * Register the hint callback, or NULL to disable the hint rows.
 */
TL_DEF void tl_set_hint_callback(tl_hint_fn callback);

/**
 * Set the right prompt, or NULL for none. It is drawn right-aligned on the
 * first input row and ends one column short of the right edge. It is hidden
 * while the input and ghost on that row would come within one column of it,
 * while the prompt is cut to fit, while a history search is open, and when it
 * holds a line break. It is never part of the submitted line. Escape sequences
 * count as zero columns. The width is measured once here, and the host keeps
 * the string valid until the next call.
 */
TL_DEF void tl_set_right_prompt(const char *right_prompt);

/**
 * Set the prompt a submitted line is redrawn with, or NULL to leave the line
 * as it was drawn. When Enter submits or Ctrl-C interrupts the line,
 * everything from the top of the prompt
 * down is erased, which takes the right prompt, the ghost, the hint rows, and
 * any rows below the input, and the line is drawn again after this prompt. The
 * host keeps the string valid until the next call.
 */
TL_DEF void tl_set_transient_prompt(const char *transient_prompt);

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

/* The idle hook asks for the hint rows to be composed again and redrawn when
   they changed. */
#define TL_IDLE_REFRESH 1
/* The idle hook asks to run once more after the repeat interval while the
   pause lasts. */
#define TL_IDLE_AGAIN 2

/**
 * The idle hook. The input wait calls it once no key has arrived for the delay
 * given at registration, with the buffer and the byte offset of the caret. It
 * runs at most once per pause unless it answers TL_IDLE_AGAIN, and its answer
 * may add TL_IDLE_REFRESH. A key ends the pause and the next pause waits the
 * whole delay again. Pending input, a paste, an open menu, and a history
 * search never reach it. The hook runs with the wake signals unblocked, so it
 * may start and reap child processes, and it should return quickly since keys
 * wait while it runs.
 */
typedef int (*tl_idle_fn)(const char *buffer, size_t cursor);

/**
 * Register the idle hook with its delay and repeat interval in milliseconds,
 * or NULL to disable it.
 */
TL_DEF void tl_set_idle_callback(tl_idle_fn callback, int delay_ms,
                                 int repeat_ms);

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

#define ITL_STDIN  0
#define ITL_STDOUT 1
#define ITL_STDERR 2
#define ITL_FILE   int

/* Binary mode keeps byte offsets exact, the CRT text mode would translate each
   newline to a carriage return plus newline and desync the offset ring. */
#define ITL_FILE_OPEN_FOR_READ(path) _open(path, O_RDONLY | _O_BINARY)
#define ITL_FILE_OPEN_FOR_WRITE(path)                                          \
  _open(path, O_WRONLY | O_CREAT | O_TRUNC | _O_BINARY, _S_IREAD | _S_IWRITE)
#define ITL_FILE_OPEN_FOR_APPEND(path)                                         \
  _open(path, O_RDWR | O_CREAT | O_APPEND | _O_BINARY, _S_IREAD | _S_IWRITE)
#define ITL_FILE_IS_BAD(file) (file < 0)
#define ITL_FILE_CLOSE        _close
#define ITL_FILE_SEEK(file, offset)                                            \
  (_lseek(file, (long) (offset), SEEK_SET) == (long) (offset))
/* Seeks to the end and yields the resulting offset, the file size, or -1. */
#define ITL_FILE_SEEK_END(file) ((long) _lseek(file, 0L, SEEK_END))
#define ITL_FILE_TELL(file)     ((long) _lseek(file, 0L, SEEK_CUR))

#define ITL_WRITE(fd, buf, size) _write(fd, buf, (unsigned int) (size))
#define ITL_READ(fd, buf, size)  _read(fd, buf, (unsigned int) (size))

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
#include <sys/select.h>
#include <termios.h>
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define ITL_ISATTY isatty

#define ITL_STDIN  0
#define ITL_STDOUT 1
#define ITL_STDERR 2
#define ITL_FILE   int

#define ITL_FILE_OPEN_FOR_READ(path) open(path, O_RDONLY)
#define ITL_FILE_OPEN_FOR_WRITE(path)                                          \
  open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)
#define ITL_FILE_OPEN_FOR_APPEND(path)                                         \
  open(path, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR)
#define ITL_FILE_IS_BAD(file) (file < 0)
#define ITL_FILE_CLOSE        close
#define ITL_FILE_SEEK(file, offset)                                            \
  (lseek(file, (off_t) (offset), SEEK_SET) == (off_t) (offset))
/* Seeks to the end and yields the resulting offset, the file size, or -1. */
#define ITL_FILE_SEEK_END(file) ((long) lseek(file, (off_t) 0, SEEK_END))
#define ITL_FILE_TELL(file)     ((long) lseek(file, (off_t) 0, SEEK_CUR))

#define ITL_WRITE(fd, buf, size) write(fd, buf, (unsigned long) size)
#define ITL_READ(fd, buf, size)  read(fd, buf, (unsigned long) size)

/* <https://man7.org/linux/man-pages/man3/termios.3.html> */
#define ITL_STRING_MAX_LEN 4095

#if !defined ITL_TTY_IS_TTY
#define ITL_TTY_IS_TTY() ITL_ISATTY(STDIN_FILENO)
#endif /* ITL_TTY_IS_TTY */
#endif /* ITL_POSIX */

#if defined TL_DEBUG || defined TL_SEE_BYTES
#include <stdio.h>
#endif /* TL_DEBUG */

#if defined ITL_WIN32
/* Windows can't read arrow keys otherwise */
#define ITL_READ_BYTE_RAW _getch
#else /* ITL_WIN32 */
ITL_DEF int ITL_READ_BYTE_RAW(void)
{
  unsigned char byte_value;
  return (ITL_READ(ITL_STDIN, &byte_value, 1) != 1) ? -1 : (int) byte_value;
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
#endif /* !__STDC_VERSION__ || __STDC_VERSION__ >= 199409L */

#if defined ITL_C89 && defined __cplusplus
#define ITL_ZERO_INIT                                                          \
  {}
#else
#define ITL_ZERO_INIT {0}
#endif /* ITL_C89 && __cplusplus */

#if !defined __cplusplus
#include <stdbool.h>
#endif /* !__cplusplus */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h> /* sig_atomic_t for the terminal resize flag */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined ITL_POSIX || defined ITL_SUSPEND
#include <signal.h>
#endif

#define ITL_THREAD_LOCAL         __thread
#define ITL_NO_RETURN            __attribute__((noreturn))
#define ITL_MAYBE_UNUSED         __attribute__((unused))
#define ITL_UNREACHABLE_INTRIN() __builtin_unreachable()
#define itl_debug_trap()         __builtin_trap()

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
/* Set while the host lets the interrupt key raise a signal, so raw mode entered
   again meanwhile, such as after a program run in the middle, keeps it. */
ITL_DEF ITL_THREAD_LOCAL bool itl_g_signal_keys_enabled = false;
#if defined ITL_WIN32
ITL_DEF ITL_THREAD_LOCAL DWORD itl_g_original_tty_in_mode = 0;
ITL_DEF ITL_THREAD_LOCAL DWORD itl_g_original_tty_out_mode = 0;
ITL_DEF ITL_THREAD_LOCAL UINT itl_g_original_tty_cp = 0;
ITL_DEF ITL_THREAD_LOCAL UINT itl_g_original_tty_output_cp = 0;
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
#if defined ENABLE_VIRTUAL_TERMINAL_INPUT
  /* Virtual terminal input makes the console deliver special keys as the escape
     sequences the input parser already decodes. */
  tty_in_mode |= (DWORD) ENABLE_VIRTUAL_TERMINAL_INPUT;
#endif

  itl_g_original_tty_out_mode = tty_out_mode;
  tty_out_mode = (DWORD) ENABLE_PROCESSED_OUTPUT |
                 ENABLE_VIRTUAL_TERMINAL_PROCESSING |
                 DISABLE_NEWLINE_AUTO_RETURN;

  ITL_TRY(SetConsoleMode(stdin_handle, tty_in_mode), return false);
  ITL_TRY(SetConsoleMode(stdout_handle, tty_out_mode), return false);

  codepage = GetConsoleCP();
  ITL_TRY(codepage != 0, return false);

  itl_g_original_tty_cp = codepage;
  ITL_TRY(SetConsoleCP(CP_UTF8), return false);

  /* The editor draws UTF-8, so the output code page follows the input one
     while raw mode is active and is restored for the programs run between
     prompts. */
  codepage = GetConsoleOutputCP();
  ITL_TRY(codepage != 0, return false);

  itl_g_original_tty_output_cp = codepage;
  ITL_TRY(SetConsoleOutputCP(CP_UTF8), return false);

  mode = _setmode(STDIN_FILENO, _O_BINARY);
  ITL_TRY(mode != -1, return false);

  itl_g_original_mode = mode;
#elif defined ITL_POSIX
  struct termios term;
  ITL_TRY(tcgetattr(STDIN_FILENO, &term) == 0, return false);

  itl_g_original_tty_mode = term;
  cfmakeraw(&term);
  term.c_oflag = OPOST | ONLCR;
  if (itl_g_signal_keys_enabled) {
    term.c_lflag |= (tcflag_t) ISIG;
  }

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
  if (itl_g_original_tty_output_cp != 0) {
    ITL_TRY(SetConsoleOutputCP(itl_g_original_tty_output_cp),
            something_failed = true);
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

/* The kitty push of the disambiguate flag with xterm's modifyOtherKeys level 1,
   and the kitty pop with the modifyOtherKeys reset to the terminal's own
   setting. A terminal that knows neither sees private-marker sequences it
   discards. */
#define ITL_EXTENDED_KEYS_ON  "\x1b[>1u\x1b[>4;1m"
#define ITL_EXTENDED_KEYS_OFF "\x1b[<u\x1b[>4m"

ITL_DEF ITL_THREAD_LOCAL bool itl_g_extended_keys_enabled = false;
#if defined ITL_POSIX
ITL_DEF ITL_THREAD_LOCAL bool itl_g_extended_keys_active = false;
#endif /* ITL_POSIX */

ITL_DEF void itl_set_extended_keys_active(bool should_be_active)
{
#if defined ITL_POSIX
  if (should_be_active == itl_g_extended_keys_active) {
    return;
  }

  if (should_be_active) {
    ITL_TRY(ITL_WRITE(ITL_STDOUT, ITL_EXTENDED_KEYS_ON,
                      sizeof(ITL_EXTENDED_KEYS_ON) - 1) != -1,
            return);
  } else {
    ITL_TRY(ITL_WRITE(ITL_STDOUT, ITL_EXTENDED_KEYS_OFF,
                      sizeof(ITL_EXTENDED_KEYS_OFF) - 1) != -1,
            {});
  }
  itl_g_extended_keys_active = should_be_active;
#else
  (void) should_be_active;
#endif /* ITL_POSIX */
}

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
  itl_set_extended_keys_active(itl_g_extended_keys_enabled &&
                               !itl_g_signal_keys_enabled);

  return TL_SUCCESS;
}

TL_DEF tl_status_code tl_exit_raw_mode(void)
{
  bool did_restore_mode;

  ITL_TRY(itl_g_entered_raw_mode, return TL_SUCCESS);

  ITL_TRY(ITL_TTY_IS_TTY(), return TL_ERROR);

  /* The terminal requests are withdrawn even when the mode cannot be restored,
     so a failed exit leaves no key reporting behind for the next program. */
  did_restore_mode = itl_exit_raw_mode_impl();

#if defined ITL_POSIX
  ITL_TRY(ITL_WRITE(ITL_STDOUT, "\x1b[?2004l", 8) != -1, {});
#endif
  itl_set_extended_keys_active(false);

  if (itl_g_vi_cursor_shape != ITL_VI_CURSOR_DEFAULT_SHAPE) {
    ITL_TRY(ITL_WRITE(ITL_STDOUT, "\x1b[0 q", 5) != -1, {});
    itl_g_vi_cursor_shape = ITL_VI_CURSOR_DEFAULT_SHAPE;
  }

  ITL_TRY(did_restore_mode, return TL_ERROR);

  itl_g_entered_raw_mode = false;

  return TL_SUCCESS;
}

TL_DEF void tl_restore_terminal_for_exit(void)
{
#if defined ITL_POSIX
  static const char withdraw[] = "\x1b[?2004l" ITL_EXTENDED_KEYS_OFF;
  static const char default_cursor[] = "\x1b[0 q";
  ssize_t written;

  if (!itl_g_entered_raw_mode) {
    return;
  }

  written = write(STDOUT_FILENO, withdraw, sizeof(withdraw) - 1);
  if (itl_g_vi_cursor_shape != ITL_VI_CURSOR_DEFAULT_SHAPE) {
    written = write(STDOUT_FILENO, default_cursor, sizeof(default_cursor) - 1);
  }
  (void) written;
  (void) tcsetattr(STDIN_FILENO, TCSANOW, &itl_g_original_tty_mode);
#endif /* ITL_POSIX */
}

/* Holds at most one pushed-back byte, or -1 when empty. The read path drains
   it before the descriptor and the pending probe counts it as input. */
ITL_DEF ITL_THREAD_LOCAL int itl_g_pushback_byte = -1;

/* The rest of an escape sequence already read from the terminal, or the legacy
   bytes that replaced it. The read path drains it after the pushback byte and
   before the descriptor, and the pending probe counts it as input. */
#define ITL_KEY_QUEUE_SIZE 32
ITL_DEF ITL_THREAD_LOCAL uint8_t itl_g_key_queue[ITL_KEY_QUEUE_SIZE];
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_key_queue_index = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_key_queue_length = 0;

/* A bracketed paste body is text, so its bytes are never translated. */
ITL_DEF ITL_THREAD_LOCAL bool itl_g_is_reading_paste = false;

/* Returns true when the terminal has a byte to read without blocking. */
ITL_DEF bool itl_tty_input_is_pending(void)
{
#if defined ITL_WIN32
  return _kbhit() != 0;
#else  /* ITL_WIN32 */
  struct pollfd pfd;
  pfd.fd = STDIN_FILENO;
  pfd.events = POLLIN;
  pfd.revents = 0;
  return poll(&pfd, 1, 0) > 0;
#endif /* ITL_WIN32 */
}

ITL_DEF bool itl_read_tty_byte(uint8_t *buffer)
{
  int byte;
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

#if defined ITL_POSIX
#define ITL_LEGACY_KEY_SIZE 8

ITL_DEF size_t itl_utf8_encode_codepoint(uint32_t codepoint, uint8_t *out)
{
  if (codepoint < 0x80) {
    out[0] = (uint8_t) codepoint;
    return 1;
  }
  if (codepoint < 0x800) {
    out[0] = (uint8_t) (0xC0 | (codepoint >> 6));
    out[1] = (uint8_t) (0x80 | (codepoint & 0x3F));
    return 2;
  }
  if (codepoint < 0x10000) {
    out[0] = (uint8_t) (0xE0 | (codepoint >> 12));
    out[1] = (uint8_t) (0x80 | ((codepoint >> 6) & 0x3F));
    out[2] = (uint8_t) (0x80 | (codepoint & 0x3F));
    return 3;
  }
  out[0] = (uint8_t) (0xF0 | (codepoint >> 18));
  out[1] = (uint8_t) (0x80 | ((codepoint >> 12) & 0x3F));
  out[2] = (uint8_t) (0x80 | ((codepoint >> 6) & 0x3F));
  out[3] = (uint8_t) (0x80 | (codepoint & 0x3F));
  return 4;
}

/* The byte a legacy terminal sends for Ctrl with a character, or -1 when the
   pair has none. These are the xterm and kitty tables. */
ITL_DEF int itl_legacy_ctrl_byte(uint32_t key)
{
  if ((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z')) {
    return (int) (key & 0x1F);
  }

  switch (key) {
  case ' ':
  case '@':
  case '2': return 0;
  case '[':
  case '3': return 27;
  case '\\':
  case '4': return 28;
  case ']':
  case '5': return 29;
  case '^':
  case '~':
  case '6': return 30;
  case '_':
  case '/':
  case '7': return 31;
  case '?':
  case '8': return 127;
  }

  return -1;
}

/* The legacy bytes for a key the kitty keyboard protocol reports as CSI code ;
   modifier u, or xterm's modifyOtherKeys as CSI 27 ; modifier ; code ~. The
   modifier is one plus a bit set of shift 1, alt 2, ctrl 4, super 8, hyper 16,
   meta 32, and the lock bits 64 and 128, which are ignored. Kitty reports the
   unshifted key, so a shifted key comes from shifted_code, the case of a
   letter, or the US layout. Returns the byte count written to out, or 0 when
   the key has no legacy form of its own, such as Ctrl-Shift-Z, or would read
   as the start of another sequence, such as Alt-[. The sequence is then left
   to the key parser. */
ITL_DEF size_t itl_legacy_key_bytes(uint32_t code, uint32_t shifted_code,
                                    unsigned modifier, uint8_t *out)
{
  static const char unshifted_keys[] = "`1234567890-=[]\\;',./";
  static const char shifted_keys[] = "~!@#$%^&*()_+{}|:\"<>?";
  static const char keypad_text[] = "0123456789./*-+\r=,";
  static const char *const keypad_moves[] = {"D", "C", "A",  "B",  "5~", "6~",
                                             "H", "F", "2~", "3~", "E"};
  unsigned bits = (modifier > 0) ? modifier - 1 : 0;
  uint32_t key = code;
  size_t length = 0;
  bool is_shift, is_alt, is_ctrl;
  int ctrl_byte;

  if ((bits & 32u) != 0) {
    bits |= 2u;
  }
  if ((bits & (8u | 16u)) != 0) {
    return 0;
  }
  bits &= 7u;
  is_shift = (bits & 1u) != 0;
  is_alt = (bits & 2u) != 0;
  is_ctrl = (bits & 4u) != 0;

  /* Kitty reports the keypad apart from the main keys. Its text keys read as
     their characters and its moves as the main moves. */
  if (key >= 57399 && key < 57399 + sizeof(keypad_text) - 1) {
    key = (uint8_t) keypad_text[key - 57399];
  } else if (key >= 57417 &&
             key < 57417 + sizeof(keypad_moves) / sizeof(*keypad_moves))
  {
    const char *move = keypad_moves[key - 57417];
    size_t move_length = strlen(move);

    out[length++] = 0x1B;
    out[length++] = '[';
    if (move_length == 1 && bits != 0) {
      out[length++] = '1';
    }
    if (move_length == 2) {
      out[length++] = (uint8_t) move[0];
    }
    if (bits != 0) {
      out[length++] = ';';
      out[length++] = (uint8_t) ('1' + bits);
    }
    out[length++] = (uint8_t) move[move_length - 1];
    return length;
  }

  switch (key) {
  case 27:
  case 13:
    if (is_alt) {
      out[length++] = 0x1B;
    }
    out[length++] = (uint8_t) key;
    return length;

  case 9:
    if (is_shift) {
      memcpy(out, "\x1b[Z", 3);
      return 3;
    }
    if (is_alt) {
      out[length++] = 0x1B;
    }
    out[length++] = 9;
    return length;

  case 8:
  case 127:
    if (is_alt) {
      out[length++] = 0x1B;
    }
    out[length++] = (is_ctrl || key == 8) ? 8 : 127;
    return length;
  }

  if (key < 32 || (key >= 127 && key < 160) || key > 0x10FFFF ||
      (key >= 0xD800 && key < 0xE000) || (key >= 57344 && key < 63744))
  {
    return 0;
  }

  if (is_shift) {
    const char *unshifted =
        (key < 128) ? strchr(unshifted_keys, (int) key) : NULL;

    if (shifted_code >= 32 && shifted_code <= 0x10FFFF) {
      key = shifted_code;
    } else if (key >= 'a' && key <= 'z') {
      key -= 'a' - 'A';
    } else if (unshifted != NULL) {
      key = (uint8_t) shifted_keys[unshifted - unshifted_keys];
    }
  }

  if (is_alt) {
    out[length++] = 0x1B;
  }

  if (is_ctrl) {
    ctrl_byte = itl_legacy_ctrl_byte(key);
    if (key == 'Z' || ctrl_byte < 0) {
      return 0;
    }
    out[length++] = (uint8_t) ctrl_byte;
    return length;
  }

  if (is_alt && (key == '[' || key == 'O')) {
    return 0;
  }

  return length + itl_utf8_encode_codepoint(key, out + length);
}

#define ITL_CSI_FIELD_MAX 3

/* Decodes the parameters and final byte of a CSI sequence into the legacy
   bytes of the key it reports, returning 0 for any sequence that is not a
   key in the kitty or the modifyOtherKeys form. */
ITL_DEF size_t itl_legacy_csi_key_bytes(const uint8_t *sequence, size_t size,
                                        uint8_t *out)
{
  uint32_t fields[ITL_CSI_FIELD_MAX][ITL_CSI_FIELD_MAX] = {{0}};
  size_t field = 0, part = 0, i;
  uint8_t final_byte;

  if (size < 2 || sequence[0] < '0' || sequence[0] > '9') {
    return 0;
  }
  final_byte = sequence[size - 1];

  for (i = 0; i + 1 < size; ++i) {
    uint8_t byte = sequence[i];

    if (byte >= '0' && byte <= '9') {
      if (fields[field][part] > 0x10FFFF) {
        return 0;
      }
      fields[field][part] = fields[field][part] * 10 + (uint32_t) (byte - '0');
    } else if (byte == ';' && field + 1 < ITL_CSI_FIELD_MAX) {
      field += 1;
      part = 0;
    } else if (byte == ':' && part + 1 < ITL_CSI_FIELD_MAX) {
      part += 1;
    } else {
      return 0;
    }
  }

  if (final_byte == 'u') {
    if (fields[1][1] == 3) {
      return 0;
    }
    return itl_legacy_key_bytes(fields[0][0], fields[0][1],
                                (unsigned) fields[1][0], out);
  }
  if (final_byte == '~' && fields[0][0] == 27 && fields[0][1] == 0 &&
      field == 2)
  {
    return itl_legacy_key_bytes(fields[2][0], fields[2][0],
                                (unsigned) fields[1][0], out);
  }

  return 0;
}

/* Reads what follows an ESC from the terminal into the key queue. A key that
   the kitty keyboard protocol or modifyOtherKeys reports in a form of its own
   is replaced by the bytes a legacy terminal sends for it, so every reader of
   keys, from the parser to the vi, menu, and search loops, sees one encoding.
   Any other sequence is queued as it arrived. A lone ESC queues nothing. */
ITL_DEF void itl_queue_escape_tail(uint8_t *first_byte)
{
  uint8_t legacy[ITL_LEGACY_KEY_SIZE];
  size_t length = 0, legacy_length;
  uint8_t byte;

  itl_g_key_queue_index = 0;
  itl_g_key_queue_length = 0;
  if (itl_g_is_reading_paste || !itl_tty_input_is_pending() ||
      !itl_read_tty_byte(&byte))
  {
    return;
  }

  itl_g_key_queue[length++] = byte;
  if (byte == '[') {
    while (length < ITL_KEY_QUEUE_SIZE && itl_read_tty_byte(&byte)) {
      itl_g_key_queue[length++] = byte;
      if (byte < 0x20 || byte >= 0x40) {
        break;
      }
    }

    legacy_length =
        itl_legacy_csi_key_bytes(itl_g_key_queue + 1, length - 1, legacy);
    if (legacy_length > 0) {
      *first_byte = legacy[0];
      memcpy(itl_g_key_queue, legacy + 1, legacy_length - 1);
      length = legacy_length - 1;
    }
  }

  itl_g_key_queue_length = length;
}
#endif /* ITL_POSIX */

ITL_DEF bool ITL_READ_BYTE(uint8_t *buffer)
{
  uint8_t byte;
  if (itl_g_pushback_byte != -1) {
    ITL_PTR_ASSIGN(buffer, (uint8_t) itl_g_pushback_byte);
    itl_g_pushback_byte = -1;
    return true;
  }
  if (itl_g_key_queue_index < itl_g_key_queue_length) {
    ITL_PTR_ASSIGN(buffer, itl_g_key_queue[itl_g_key_queue_index]);
    itl_g_key_queue_index += 1;
    return true;
  }

  ITL_TRY(itl_read_tty_byte(&byte), return false);
#if defined ITL_POSIX
  if (byte == 0x1B) {
    itl_queue_escape_tail(&byte);
  }
#endif /* ITL_POSIX */
  ITL_PTR_ASSIGN(buffer, byte);
  return true;
}

#define ITL_TRY_READ_BYTE(buffer, expr) ITL_TRY(ITL_READ_BYTE(buffer), expr)

/* Returns true when a byte is already available without blocking, so the
   key wait loop skips its sleep when input is ready. */
ITL_DEF bool itl_input_is_pending(void)
{
  if (itl_g_pushback_byte != -1 ||
      itl_g_key_queue_index < itl_g_key_queue_length)
  {
    return true;
  }

  return itl_tty_input_is_pending();
}

#if defined ITL_POSIX
ITL_DEF bool itl_block_input_wake_signals(sigset_t *previous_signals)
{
  sigset_t blocked_signals;

  sigemptyset(&blocked_signals);
  sigaddset(&blocked_signals, SIGWINCH);
  sigaddset(&blocked_signals, SIGCHLD);
  return sigprocmask(SIG_BLOCK, &blocked_signals, previous_signals) == 0;
}

ITL_DEF bool itl_restore_input_wake_signals(const sigset_t *previous_signals)
{
  return sigprocmask(SIG_SETMASK, previous_signals, NULL) == 0;
}

/* Waits for input or a wake signal, and for at most timeout_ms when it is not
   negative. Returns 0 when the timeout passed with nothing to read, 1 on input
   or a signal, and -1 on an error. */
ITL_DEF int itl_wait_for_input_until(const sigset_t *previous_signals,
                                     int timeout_ms)
{
  fd_set readable;
  struct timespec timeout;
  int result;

  FD_ZERO(&readable);
  FD_SET(STDIN_FILENO, &readable);
  timeout.tv_sec = timeout_ms / 1000;
  timeout.tv_nsec = (long) (timeout_ms % 1000) * 1000000L;
  result = pselect(STDIN_FILENO + 1, &readable, NULL, NULL,
                   timeout_ms < 0 ? NULL : &timeout, previous_signals);
  if (result == 0) {
    return 0;
  }
  return result > 0 || errno == EINTR ? 1 : -1;
}

ITL_DEF bool itl_wait_for_input(const sigset_t *previous_signals)
{
  return itl_wait_for_input_until(previous_signals, -1) >= 0;
}
#endif /* ITL_POSIX */

/* A clock for the idle delay that never steps backwards. */
ITL_DEF uint64_t itl_monotonic_ms(void)
{
#if defined ITL_WIN32
  return (uint64_t) GetTickCount64();
#else  /* ITL_WIN32 */
  struct timespec now;
  if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
    return 0;
  }
  return (uint64_t) now.tv_sec * 1000u + (uint64_t) now.tv_nsec / 1000000u;
#endif /* ITL_WIN32 */
}

ITL_DEF volatile sig_atomic_t itl_g_tty_changed_size = 1;

/* True until the first render of an input session. The first render has no
   previous block on screen to reflow, so it must draw in place rather than
   moving up and clearing as a resize would. */
ITL_DEF ITL_THREAD_LOCAL bool itl_g_tty_first_render = true;

#if defined ITL_SUSPEND
#if defined ITL_POSIX
ITL_DEF void itl_raise_suspend(void)
{
  /* Leave raw mode, stop, and resume here when continued. raise() returns only
     after SIGCONT, so raw mode is restored in normal context without a handler
     calling unsafe terminal functions. */
  tl_exit_raw_mode();
  raise(SIGTSTP);
  tl_enter_raw_mode();
  itl_g_tty_changed_size = 1;
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

  ITL_TRY(allocated != NULL, TL_ABORT());

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

  ITL_TRY(allocated != NULL, TL_ABORT());

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

#define ITL_COUNTOF(a) (sizeof(a) / sizeof((a)[0]))

typedef struct itl_cp_interval itl_cp_interval_t;

struct itl_cp_interval
{
  uint32_t first;
  uint32_t last;
};

/* Sorted ranges of zero-width and combining codepoints. An emoji skin tone
   modifier draws on the cell of the emoji before it. */
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
    {0xFE00,  0xFE0F },
    {0xFE20,  0xFE2F },
    {0x1F3FB, 0x1F3FF},
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

ITL_DEF bool itl_utf8_is_plain_ascii(itl_utf8_t ch)
{
  return ch.size == 1 && ch.bytes[0] >= 0x20 && ch.bytes[0] < 0x7F;
}

/* Returns the terminal column width of a character, which is 0, 1, or 2. Tab is
   counted as a single column. A newline is handled by the renderer, not here.
 */
ITL_DEF size_t itl_char_width(itl_utf8_t ch)
{
  uint32_t cp;

  /* An ASCII rune is answered without a table search. The zero width table
     starts at U+0300 and the wide table at U+1100, so neither can hold one. */
  if (ch.size == 1 && ch.bytes[0] < 0x80) {
    if (ch.bytes[0] == 0x09) {
      return 1;
    }

    if (ch.bytes[0] < 0x20 || ch.bytes[0] == 0x7F) {
      return 0;
    }

    return 1;
  }

  cp = itl_utf8_codepoint(ch);

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

/* A C0 control, DEL, or a C1 control never reaches the terminal raw from
   edited or offered text, since a file name or a description could otherwise
   move the cursor or set the title. It draws as caret notation, such as ^[ for
   ESC and ^? for DEL, or as \x9b for a C1 control. */
ITL_DEF bool itl_char_has_visible_notation(itl_utf8_t ch)
{
  if (ch.size == 1) {
    return ch.bytes[0] < 0x20 || ch.bytes[0] == 0x7F;
  }

  return ch.size == 2 && ch.bytes[0] == 0xC2 && ch.bytes[1] < 0xA0;
}

#define ITL_CARET_NOTATION_WIDTH 2
#define ITL_HEX_NOTATION_WIDTH   4

/* The columns a character of the edited line takes as drawn. A tab is drawn
   raw as one column and a newline is handled by the renderer, so only the
   other controls take their notation width. */
ITL_DEF size_t itl_char_line_width(itl_utf8_t ch)
{
  if (ch.size == 1 && (ch.bytes[0] == 0x09 || ch.bytes[0] == 0x0A)) {
    return itl_char_width(ch);
  }

  if (itl_char_has_visible_notation(ch)) {
    return ch.size == 1 ? ITL_CARET_NOTATION_WIDTH : ITL_HEX_NOTATION_WIDTH;
  }

  return itl_char_width(ch);
}

#define ITL_ZERO_WIDTH_JOINER 0x200D

ITL_DEF bool itl_char_is_zero_width_joiner(itl_utf8_t ch)
{
  return ch.size == 3 && ch.bytes[0] == 0xE2 && ch.bytes[1] == 0x80 &&
         ch.bytes[2] == 0x8D;
}

/* The columns the character at position of the line takes as drawn. The
   character after a zero-width joiner draws on the cell of the emoji before
   the joiner, so a joined sequence takes the width of its first emoji. */
ITL_DEF size_t itl_line_char_width(const itl_utf8_t *chars, size_t position)
{
  if (position > 0 && chars[position].size > 1 &&
      itl_char_is_zero_width_joiner(chars[position - 1]) &&
      !itl_char_has_visible_notation(chars[position]))
  {
    return 0;
  }

  return itl_char_line_width(chars[position]);
}

/* Decodes the codepoint that starts the text. A false result means the first
   byte begins no valid sequence that fits in byte_length bytes. */
ITL_DEF bool itl_utf8_decode_at(const char *text, size_t byte_length,
                                itl_utf8_t *ch)
{
  uint8_t rune_width = itl_utf8_width((uint8_t) text[0]);
  uint8_t j;

  if (rune_width == 0 || rune_width > sizeof ch->bytes ||
      (size_t) rune_width > byte_length)
  {
    return false;
  }

  for (j = 1; j < rune_width; ++j) {
    if (((uint8_t) text[j] & 0xC0) != 0x80) {
      return false;
    }
  }

  for (j = 0; j < rune_width; ++j) {
    ch->bytes[j] = (uint8_t) text[j];
  }
  ch->size = rune_width;

  return true;
}

/* The walker consumes at most byte_length bytes and stops at a null byte. */
ITL_DEF size_t itl_strn_width_walk(const char *cstr, size_t byte_length,
                                   size_t stop_after, size_t *out_offset)
{
  size_t width = 0, i = 0;
  bool is_after_joiner = false;

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

    if ((uint8_t) cstr[i] >= 0x20 && (uint8_t) cstr[i] < 0x7F) {
      if (width >= stop_after) {
        break;
      }

      width += 1;
      i += 1;
      is_after_joiner = false;
      continue;
    }

    itl_utf8_t ch;

    if (!itl_utf8_decode_at(cstr + i, byte_length - i, &ch)) {
      if (width >= stop_after) break;
      width += 1;
      i += 1;
      is_after_joiner = false;
      continue;
    }
    {
      size_t character_width =
          is_after_joiner && ch.size > 1 ? 0 : itl_char_width(ch);
      if (character_width > 0 && width >= stop_after) break;
      width += character_width;
    }
    is_after_joiner = itl_char_is_zero_width_joiner(ch);
    i += ch.size;
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

ITL_DEF bool itl_string_equal(const itl_string_t *str1,
                              const itl_string_t *str2)
{
  size_t i;

  if (str1->size != str2->size) {
    return false;
  }
  if (str1->size == 0) {
    return true;
  }

  for (i = 0; i < str1->length && i < str2->length; ++i) {
    if (!itl_utf8_equal(str1->chars[i], str2->chars[i])) {
      return false;
    }
  }

  return i == str1->length;
}

/* Compares a string with the raw bytes of a decoded entry, so a caller that
   already holds bytes does not have to build a string to compare it. */
ITL_DEF bool itl_string_equal_bytes(const itl_string_t *str, const char *data,
                                    size_t size)
{
  size_t i;
  size_t position = 0;

  if (str->size != size) {
    return false;
  }

  for (i = 0; i < str->length; ++i) {
    if (memcmp(str->chars[i].bytes, data + position, str->chars[i].size) != 0) {
      return false;
    }

    position += str->chars[i].size;
  }

  return true;
}

ITL_DEF void itl_string_copy(itl_string_t *dst, const itl_string_t *src)
{
  TL_ASSERT(dst != NULL);
  TL_ASSERT(src != NULL);

  while (dst->capacity < src->length) {
    itl_string_extend(dst);
  }

  memcpy(dst->chars, src->chars, src->length * sizeof(itl_utf8_t));

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
  size_t moved_count;

  TL_ASSERT(position <= str->length);

  /* The tail after `position` keeps its order in both directions, so one
     overlapping move carries it. */
  if (backwards) {
    moved_count = str->length - position;

    memmove(str->chars + position - shift_by, str->chars + position,
            moved_count * sizeof(itl_utf8_t));

    TL_ASSERT(str->length >= shift_by);
    str->length -= shift_by;
  } else {
    str->length += shift_by;

    while (str->capacity < str->length) {
      itl_string_extend(str);
    }

    TL_ASSERT(str->length >= shift_by + 1);
    moved_count = str->length - shift_by - position;

    memmove(str->chars + position + shift_by, str->chars + position,
            moved_count * sizeof(itl_utf8_t));
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
  size_t i, k;

  /* A zero size buffer has no room for even the null terminator, so writing it
     would land past the end. */
  if (cstr_size == 0) {
    return TL_ERROR_SIZE;
  }

  for (i = 0, k = 0; i < str->length; ++i) {
    if (k + 1 >= cstr_size || cstr_size - k - 1 < str->chars[i].size) {
      break;
    }

    memcpy(cstr + k, str->chars[i].bytes, str->chars[i].size);
    k += str->chars[i].size;
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
  size_t rune_count = 0;
  uint8_t rune_width;
  uint32_t codepoint;

  /* Clamp the byte count to the platform cap, the same silent-truncation
     policy itl_history_read_entry_fd uses, so a host setter that feeds an
     oversized string never trips the length assert in
     itl_string_recalc_size. */
  if (size > ITL_STRING_MAX_LEN) {
    size = ITL_STRING_MAX_LEN;
    while (size > 0 && (data[size] & 0xC0) == 0x80) {
      --size;
    }
  }

  for (k = 0; k < size;) {
    rune_width = itl_utf8_width((uint8_t) data[k]);
    if (rune_width == 0 || k + rune_width > size) {
      return false;
    }

    codepoint = (uint8_t) data[k] &
                (rune_width == 1   ? 0x7FU
                 : rune_width == 2 ? 0x1FU
                 : rune_width == 3 ? 0x0FU
                                   : 0x07U);
    for (j = 1; j < rune_width; ++j) {
      uint8_t continuation_byte = (uint8_t) data[k + j];
      if ((continuation_byte & 0xC0) != 0x80) return false;
      codepoint = (codepoint << 6) | (continuation_byte & 0x3F);
    }
    if ((rune_width == 2 && codepoint < 0x80U) ||
        (rune_width == 3 && codepoint < 0x800U) ||
        (rune_width == 4 && codepoint < 0x10000U) ||
        codepoint > 0x10FFFFU ||
        (codepoint >= 0xD800U && codepoint <= 0xDFFFU))
    {
      return false;
    }
    k += rune_width;
    rune_count += 1;
  }

  while (str->capacity < rune_count) {
    itl_string_extend(str);
  }

  for (i = 0, k = 0; k < size; ++i) {
    rune_width = itl_utf8_width((uint8_t) data[k]);

    str->chars[i].size = rune_width;
    memcpy(str->chars[i].bytes, data + k, rune_width);

    k += rune_width;
  }

  str->length = rune_count;
  str->size = size;

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
ITL_DEF ITL_THREAD_LOCAL size_t
    itl_g_history_durable_offsets[TL_HISTORY_MAX_SIZE];
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_history_head = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_history_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_history_total_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_last_history_event_number = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_history_file_size = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_history_limit = TL_HISTORY_MAX_SIZE;
ITL_DEF ITL_THREAD_LOCAL bool itl_g_history_enabled = true;

typedef struct itl_history_search_snapshot
{
  const char *contents;
  size_t size;
  size_t offsets[TL_HISTORY_MAX_SIZE];
  size_t head;
  size_t count;
  bool is_active;
} itl_history_search_snapshot;

ITL_DEF ITL_THREAD_LOCAL tl_history_search_snapshot_fn
    itl_g_history_search_snapshot_callback = NULL;
ITL_DEF ITL_THREAD_LOCAL itl_history_search_snapshot
    itl_g_history_search_snapshot = {NULL, 0, {0}, 0, 0, false};

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

/* Whether Up and Down on a non-empty line recall only the entries that begin
   with the text typed before the first Up. */
ITL_DEF ITL_THREAD_LOCAL int itl_g_history_prefix_search_enabled = 0;

/* The typed text a prefix search matches entries against. */
ITL_DEF ITL_THREAD_LOCAL char itl_g_history_prefix[ITL_STRING_MAX_LEN + 1] = {
    0};
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_history_prefix_length = 0;

/* The entry the last prefix step selected. Navigation continues the prefix
   search only while the selected entry is still this one, so any other way of
   moving through history ends it. */
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_history_prefix_selected =
    ((size_t) -1);

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

/* Snapshots the line before a change that ends the current insert run. */
ITL_DEF void itl_undo_push_closed(itl_le_t *le)
{
  itl_undo_push(le);
  itl_g_undo_insert_run_open = false;
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

ITL_DEF void itl_vi_reset_pending(void)
{
  itl_g_vi_pending_operator = ITL_VI_OP_NONE;
  itl_g_vi_pending_count = 0;
  itl_g_vi_pending_register = 0;
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

/* Releases the private in-memory history snapshot, its path, draft, and offset
   ring counters. */
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

/* Decodes the entry that starts at byte offset in the history file into
   decoded, turning the backslash escapes the dumper wrote back into their
   bytes, a backslash n into a newline and a doubled backslash into one
   backslash. The decoded length is capped at capacity so one huge command
   cannot grow the read. Returns false when the file cannot be read. */
ITL_DEF bool itl_history_decode_entry_fd(ITL_FILE file, size_t offset,
                                         char *decoded, size_t capacity,
                                         size_t *decoded_size_out)
{
  char chunk[ITL_HISTORY_FILE_BUFFER_SIZE];
  size_t decoded_size = 0;
  bool escape_pending = false;
  bool carriage_return_pending = false;
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

      if (carriage_return_pending) {
        carriage_return_pending = false;
        if (ch == '\n') {
          line_done = true;
          continue;
        }

        /* The record was written with CRLF endings. A carriage return anywhere
           else is entry data. */
        if (decoded_size < capacity) {
          decoded[decoded_size++] = '\r';
        } else {
          was_truncated = true;
          line_done = true;
          continue;
        }
      }

      if (escape_pending) {
        escape_pending = false;
        if (ch == 'n') {
          ch = 0x0A;
        } else if (ch != '\\') {
          /* An unknown escape keeps the leading backslash, then the byte falls
             through to be appended on its own. */
          if (decoded_size < capacity) {
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
        carriage_return_pending = true;
        continue;
      }

      if (decoded_size < capacity) {
        decoded[decoded_size++] = (char) ch;
      } else {
        was_truncated = true;
        line_done = true;
      }
    }
  }

  if (carriage_return_pending) {
    if (decoded_size < capacity) {
      decoded[decoded_size++] = '\r';
    } else {
      was_truncated = true;
    }
  }

  if (was_truncated) {
    ITL_TRACELN("history entry at offset %zu truncated to %zu bytes\n", offset,
                capacity);
  }

  *decoded_size_out = decoded_size;

  return true;
}

/* Reads the entry that starts at byte offset from the history file into out.
   Returns false when the file cannot be read or the entry is not valid
   UTF-8. */
ITL_DEF bool itl_history_read_entry_fd(ITL_FILE file, size_t offset,
                                       itl_string_t *out)
{
  char decoded[ITL_STRING_MAX_LEN];
  size_t decoded_size;

  if (!itl_history_decode_entry_fd(file, offset, decoded, sizeof(decoded),
                                   &decoded_size))
  {
    return false;
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
  itl_vi_reset_pending();
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

/* A user edit turns a recalled entry into the current draft. Keep that draft
   available for Down, then make the next history navigation start fresh. */
ITL_DEF void itl_history_reset_after_edit(itl_le_t *le)
{
  if (le->history_selected_index == ITL_HISTORY_NONE) {
    return;
  }

  if (itl_g_history_draft == NULL) {
    itl_g_history_draft = itl_string_alloc();
  }
  itl_string_copy(itl_g_history_draft, le->line);
  le->history_selected_index = ITL_HISTORY_NONE;
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

  itl_history_reset_after_edit(le);
  itl_undo_push_closed(le);

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

  itl_history_reset_after_edit(le);
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

ITL_DEF bool itl_history_ensure_read_buffer(void);
ITL_DEF bool itl_history_decode_entry_buffered(size_t offset, char *decoded,
                                               size_t capacity,
                                               size_t *decoded_size_out);

/* Replaces the editor line with the currently selected entry from the private
   history snapshot. */
ITL_DEF void itl_history_show_selected(itl_le_t *le)
{
  size_t offset = itl_history_index_to_offset(le->history_selected_index);
  char decoded[ITL_STRING_MAX_LEN + 1];
  size_t decoded_size = 0;

  itl_le_clear_line(le);
  if (itl_history_ensure_read_buffer() &&
      itl_history_decode_entry_buffered(offset, decoded, sizeof(decoded),
                                        &decoded_size) &&
      itl_string_from_bytes(le->line, decoded, decoded_size))
  {
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

/* Scans the history from the given index toward older or newer entries and
   stores the first entry that begins with the typed prefix and differs from the
   line shown now. A start index outside the history finds nothing. */
ITL_DEF bool itl_history_find_prefixed(size_t from, bool is_backwards,
                                       const char *current,
                                       size_t *found_out)
{
  static ITL_THREAD_LOCAL char entry_cstr[ITL_STRING_MAX_LEN + 1];
  size_t index = from;
  size_t current_length = strlen(current);

  if (!itl_history_ensure_read_buffer()) {
    return false;
  }

  while (index < itl_g_history_count) {
    size_t entry_len = 0;

    if (itl_history_decode_entry_buffered(itl_history_index_to_offset(index),
                                          entry_cstr, sizeof(entry_cstr),
                                          &entry_len) &&
        entry_len >= itl_g_history_prefix_length &&
        memcmp(entry_cstr, itl_g_history_prefix,
               itl_g_history_prefix_length) == 0 &&
        !(entry_len == current_length &&
          memcmp(entry_cstr, current, entry_len) == 0))
    {
      *found_out = index;
      return true;
    }

    if (is_backwards) {
      if (index == 0) {
        break;
      }
      index -= 1;
    } else {
      index += 1;
    }
  }

  return false;
}

/* Up on a non-empty line recalls the next older entry that begins with the text
   typed before the first Up. Returns false when the line is not eligible, so
   the caller falls back to plain history navigation. */
ITL_DEF bool itl_g_history_prefix_get_prev(itl_le_t *le)
{
  char current[ITL_STRING_MAX_LEN + 1];
  bool is_continuing = le->history_selected_index != ITL_HISTORY_NONE &&
                       le->history_selected_index ==
                           itl_g_history_prefix_selected;
  size_t from;
  size_t found;

  if (itl_string_to_cstr(le->line, current, sizeof(current)) != TL_SUCCESS) {
    return false;
  }

  if (is_continuing) {
    from = le->history_selected_index - 1;
  } else {
    if (le->history_selected_index != ITL_HISTORY_NONE ||
        le->line->length == 0)
    {
      return false;
    }
    memcpy(itl_g_history_prefix, current, strlen(current) + 1);
    itl_g_history_prefix_length = strlen(current);
    from = itl_g_history_count - 1;
  }

  if (!itl_history_find_prefixed(from, true, current, &found)) {
    return true; /* No older match, so the line stays as it is. */
  }

  if (!is_continuing) {
    itl_history_save_draft(le);
  }
  le->history_selected_index = found;
  itl_g_history_prefix_selected = found;
  itl_history_show_selected(le);

  return true;
}

/* Down continues a prefix search toward newer entries and restores the typed
   draft once none is left. Returns false when no prefix search is under way. */
ITL_DEF bool itl_g_history_prefix_get_next(itl_le_t *le)
{
  char current[ITL_STRING_MAX_LEN + 1];
  size_t found;

  if (le->history_selected_index == ITL_HISTORY_NONE ||
      le->history_selected_index != itl_g_history_prefix_selected ||
      itl_string_to_cstr(le->line, current, sizeof(current)) != TL_SUCCESS)
  {
    return false;
  }

  if (itl_history_find_prefixed(le->history_selected_index + 1, false, current,
                                &found))
  {
    le->history_selected_index = found;
    itl_g_history_prefix_selected = found;
    itl_history_show_selected(le);
  } else {
    itl_history_restore_draft(le);
  }

  return true;
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

  if (cb->capacity == 0) {
    cb->capacity = ITL_CHAR_BUFFER_INIT_SIZE;
  }

  while (cb->capacity < needed) {
    cb->capacity = ITL_CHAR_BUF_REALLOC_CAPACITY(cb->capacity);
  }

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
   file per entry. A failure to open or seek is recorded and not retried until
   the next invalidation. A failure while reading or closing invalidates the
   buffer. The following call loads again. Returns true when the buffer holds
   the file. */
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
                        : (itl_g_history_ends_with_newline ? (size_t) file_size
                                                           : 0);
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

  /* The retained bytes and one spare are the whole load, so the cache is sized
     to them exactly. Doubling would hold up to twice the file. */
  itl_g_history_read_buffer->capacity = retained_size + 1;
  itl_g_history_read_buffer->data = (char *) itl_realloc(
      itl_g_history_read_buffer->data, itl_g_history_read_buffer->capacity);

  while (total_read < retained_size) {
    int read_amount =
        (int) ITL_READ(file, itl_g_history_read_buffer->data + total_read,
                       retained_size - total_read);
    if (read_amount < 0 && errno == EINTR) continue;
    if (read_amount <= 0) {
      ITL_FILE_CLOSE(file);
      itl_history_read_fd_invalidate();
      return false;
    }
    total_read += (size_t) read_amount;
  }
  if (ITL_FILE_CLOSE(file) != 0) {
    itl_history_read_fd_invalidate();
    return false;
  }

  itl_g_history_read_buffer->size = total_read;
  itl_g_history_read_buffer_offset = retained_offset;
  itl_g_history_read_buffer_start = 0;
  return true;
}

ITL_DEF bool itl_history_decode_entry_bytes(const char *buffer_data,
                                            size_t buffer_size, size_t offset,
                                            char *decoded, size_t capacity,
                                            size_t *decoded_size_out);

ITL_DEF bool itl_history_decode_entry_buffered(size_t offset, char *decoded,
                                               size_t capacity,
                                               size_t *decoded_size_out)
{
  size_t buffer_size = itl_g_history_read_buffer->size;
  const char *buffer_data = itl_g_history_read_buffer->data;

  if (offset < itl_g_history_read_buffer_offset) return false;
  offset -= itl_g_history_read_buffer_offset;
  if (offset < itl_g_history_read_buffer_start || offset > buffer_size)
    return false;

  return itl_history_decode_entry_bytes(buffer_data, buffer_size, offset,
                                        decoded, capacity, decoded_size_out);
}

ITL_DEF bool itl_history_decode_entry_bytes(const char *buffer_data,
                                            size_t buffer_size, size_t offset,
                                            char *decoded, size_t capacity,
                                            size_t *decoded_size_out)
{
  size_t decoded_size = 0;
  bool escape_pending = false;
  size_t i;

  if (capacity == 0) return false;
  if (offset > buffer_size) return false;

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
    } else if (ch == '\r' && i + 1 < buffer_size &&
               buffer_data[i + 1] == '\n')
    {
      /* The record was written with CRLF endings. A carriage return anywhere
         else is entry data. */
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

ITL_DEF void itl_char_buf_append_bytes(itl_char_buf_t *cb, const char *data,
                                       size_t size)
{
  itl_char_buf_reserve(cb, cb->size + size);

  memcpy(cb->data + cb->size, data, size);
  cb->size += size;
}

ITL_DEF void itl_char_buf_append_cstr(itl_char_buf_t *cb, const char *cstr)
{
  itl_char_buf_append_bytes(cb, cstr, strlen(cstr));
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
  itl_char_buf_reserve(cb, cb->size + count);

  memset(cb->data + cb->size, ' ', count);
  cb->size += count;
}

/* Append the visible notation of a control byte or codepoint: caret notation
   below 0x80 and a hex escape at or above it, which also stands for a byte
   that starts no valid UTF-8 sequence. */
ITL_DEF void itl_char_buf_append_notation(itl_char_buf_t *cb, uint32_t value)
{
  static const char hex_digits[] = "0123456789abcdef";

  if (value < 0x80) {
    itl_char_buf_append_byte(cb, '^');
    itl_char_buf_append_byte(cb, (uint8_t) (value ^ 0x40));
    return;
  }

  itl_char_buf_append_byte(cb, '\\');
  itl_char_buf_append_byte(cb, 'x');
  itl_char_buf_append_byte(cb, (uint8_t) hex_digits[(value >> 4) & 0x0F]);
  itl_char_buf_append_byte(cb, (uint8_t) hex_digits[value & 0x0F]);
}

/* Append one character of the edited line as itl_char_line_width counts it. */
ITL_DEF void itl_char_buf_append_line_char(itl_char_buf_t *cb, itl_utf8_t ch)
{
  bool is_raw_layout = ch.size == 1 &&
                       (ch.bytes[0] == 0x09 || ch.bytes[0] == 0x0A);

  if (!is_raw_layout && itl_char_has_visible_notation(ch)) {
    itl_char_buf_append_notation(cb, itl_utf8_codepoint(ch));
    return;
  }

  itl_char_buf_append_bytes(cb, (const char *) ch.bytes, ch.size);
}

/* Report the byte length and the drawn column width of the codepoint that
   starts the text. A byte that begins no valid sequence is one byte drawn as
   a hex escape, and a control draws in its notation, so every byte belongs to
   exactly one step and none reaches the terminal raw. The host counts its
   spans over the same raw bytes. */
ITL_DEF void itl_visible_step(const char *text, size_t byte_length,
                              size_t *out_bytes, size_t *out_width)
{
  itl_utf8_t ch;

  *out_bytes = 1;
  *out_width = ITL_HEX_NOTATION_WIDTH;

  if (!itl_utf8_decode_at(text, byte_length, &ch)) {
    return;
  }

  *out_bytes = ch.size;
  if (itl_char_has_visible_notation(ch)) {
    *out_width =
        ch.size == 1 ? ITL_CARET_NOTATION_WIDTH : ITL_HEX_NOTATION_WIDTH;
  } else {
    *out_width = itl_char_width(ch);
  }
}

/* Append one step of text that itl_visible_step measured. */
ITL_DEF void itl_char_buf_append_visible_step(itl_char_buf_t *cb,
                                              const char *text,
                                              size_t step_bytes)
{
  uint8_t first = (uint8_t) text[0];

  if (step_bytes == 1 && (first < 0x20 || first >= 0x7F)) {
    itl_char_buf_append_notation(cb, first);
    return;
  }

  if (step_bytes == 2 && first == 0xC2 && (uint8_t) text[1] < 0xA0) {
    itl_char_buf_append_notation(cb, (uint8_t) text[1]);
    return;
  }

  itl_char_buf_append_bytes(cb, text, step_bytes);
}

/* The drawn width of the first length bytes of text. */
ITL_DEF size_t itl_visible_width(const char *text, size_t length)
{
  size_t offset = 0;
  size_t width = 0;

  while (offset < length) {
    size_t step_bytes = 0;
    size_t step_width = 0;

    itl_visible_step(text + offset, length - offset, &step_bytes, &step_width);
    offset += step_bytes;
    width += step_width;
  }

  return width;
}

/* Append the leading part of the first length bytes of text that fits in
   width columns and return the columns drawn. */
ITL_DEF size_t itl_char_buf_append_visible(itl_char_buf_t *cb,
                                           const char *text, size_t length,
                                           size_t width)
{
  size_t offset = 0;
  size_t drawn = 0;

  while (offset < length) {
    size_t step_bytes = 0;
    size_t step_width = 0;

    itl_visible_step(text + offset, length - offset, &step_bytes, &step_width);
    if (step_width > 0 && drawn + step_width > width) {
      break;
    }
    itl_char_buf_append_visible_step(cb, text + offset, step_bytes);
    offset += step_bytes;
    drawn += step_width;
  }

  return drawn;
}

/* Appends a string with each newline written as backslash n and each backslash
   doubled, so a multiline entry survives the newline-delimited history file. */
ITL_DEF void itl_char_buf_append_string_escaped(itl_char_buf_t *cb,
                                                const itl_string_t *str)
{
  size_t i;
  size_t position;

  /* Only a single byte rune is ever escaped, so twice the byte count is the
     worst case and one reservation covers the whole append. */
  itl_char_buf_reserve(cb, cb->size + str->size * 2);
  position = cb->size;

  for (i = 0; i < str->length; ++i) {
    itl_utf8_t ch = str->chars[i];

    if (ITL_LE_IS_NEWLINE(ch)) {
      cb->data[position] = '\\';
      cb->data[position + 1] = 'n';
      position += 2;
    } else if (ITL_LE_IS_BACKSLASH(ch)) {
      cb->data[position] = '\\';
      cb->data[position + 1] = '\\';
      position += 2;
    } else {
      memcpy(cb->data + position, ch.bytes, ch.size);
      position += ch.size;
    }
  }

  cb->size = position;
}

#define ITL_CHAR_BUF_CLEAR(cb) (cb)->size = 0

/* Writes the whole span, retrying an interrupted call and resuming a partial
   write. The returned count is short only when the write failed. */
ITL_DEF size_t itl_write_all(ITL_FILE file, const char *data, size_t size)
{
  size_t total_written = 0;

  while (total_written < size) {
    int written =
        (int) ITL_WRITE(file, data + total_written, size - total_written);
    if (written < 0 && errno == EINTR) continue;
    if (written <= 0) break;
    total_written += (size_t) written;
  }

  return total_written;
}

#if !defined NDEBUG
typedef void (*itl_debug_frame_sink_fn)(const char *data, size_t size);
ITL_DEF ITL_THREAD_LOCAL itl_debug_frame_sink_fn itl_g_debug_frame_sink = NULL;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_debug_output_drain_count = 0;
#endif

/* Wait until a loading-only frame has left the terminal output queue before
   synchronous result gathering can enqueue the replacement frame. Terminal
   drain failures do not make completion fail. */
ITL_DEF void itl_terminal_drain_output(void)
{
#if !defined NDEBUG
  itl_g_debug_output_drain_count += 1;
  if (itl_g_debug_frame_sink != NULL) {
    return;
  }
#endif

#if defined ITL_POSIX
  int previous_errno = errno;
  int result;

  do {
    result = tcdrain(STDOUT_FILENO);
  } while (result < 0 && errno == EINTR);
  errno = previous_errno;
#endif
}

/* Sends one finished frame to the terminal. An empty frame writes nothing,
   because a zero-length write carries no meaning here. */
ITL_DEF bool itl_char_buf_flush(itl_char_buf_t *cb)
{
  if (cb->size == 0) {
    return true;
  }

#if !defined NDEBUG
  if (itl_g_debug_frame_sink != NULL) {
    itl_g_debug_frame_sink(cb->data, cb->size);
    return true;
  }
#endif

  return itl_write_all(ITL_STDOUT, cb->data, cb->size) == cb->size;
}

#define ITL_CHAR_BUF_DUMP(cb) (void) itl_char_buf_flush(cb)

#define ITL_TTY_HIDE_CURSOR(buffer)                                            \
  itl_char_buf_append_cstr(buffer, "\x1b[?25l")

#define ITL_TTY_SHOW_CURSOR(buffer)                                            \
  itl_char_buf_append_cstr(buffer, "\x1b[?25h")

ITL_DEF void itl_char_buf_append_csi(itl_char_buf_t *cb, size_t parameter,
                                     char final_byte)
{
  char   sequence[24];
  size_t begin = sizeof(sequence) - 1;

  sequence[begin] = final_byte;

  do {
    begin -= 1;
    sequence[begin] = (char) ('0' + parameter % 10);
    parameter /= 10;
  } while (parameter > 0);

  begin -= 2;
  sequence[begin] = '\x1b';
  sequence[begin + 1] = '[';

  itl_char_buf_append_bytes(cb, sequence + begin, sizeof(sequence) - begin);
}

#define ITL_TTY_MOVE_TO_COLUMN(buffer, col)                                    \
  itl_char_buf_append_csi(buffer, (size_t) (col), 'G')

#define ITL_TTY_MOVE_FORWARD(buffer, steps)                                    \
  itl_char_buf_append_csi(buffer, (size_t) (steps), 'C')

#define ITL_TTY_MOVE_UP(buffer, rows)                                          \
  itl_char_buf_append_csi(buffer, (size_t) (rows), 'A')

#define ITL_TTY_MOVE_DOWN(buffer, rows)                                        \
  itl_char_buf_append_csi(buffer, (size_t) (rows), 'B')

#define ITL_TTY_CLEAR_WHOLE_LINE(buffer)                                       \
  itl_char_buf_append_cstr(buffer, "\r\x1b[0K")

#define ITL_TTY_CLEAR_TO_END(buffer) itl_char_buf_append_cstr(buffer, "\x1b[K")

/* Erases from the cursor to the end of the display, used on resize where the
   reflowed row counts can no longer be trusted. */
#define ITL_TTY_CLEAR_BELOW(buffer) itl_char_buf_append_cstr(buffer, "\x1b[0J")

#define ITL_TTY_GOTO_HOME(buffer) itl_char_buf_append_cstr(buffer, "\x1b[H")

#define ITL_TTY_ERASE_SCREEN(buffer) itl_char_buf_append_cstr(buffer, "\033[2J")

/* Toggling autowrap lets the renderer place its own line breaks without the
   terminal also wrapping at the right edge, which would double the break. */
#define ITL_TTY_AUTOWRAP_OFF(buffer)                                           \
  itl_char_buf_append_cstr(buffer, "\x1b[?7l")
#define ITL_TTY_AUTOWRAP_ON(buffer) itl_char_buf_append_cstr(buffer, "\x1b[?7h")

/* If this is true, do not overwrite file on `history_dump_to_file()` */
ITL_DEF ITL_THREAD_LOCAL bool itl_g_history_file_is_bad = false;

/* Records the byte offset of one entry in the ring, evicting the oldest when
   the configured limit is full. */
ITL_DEF void itl_history_push_offset(size_t offset, size_t durable_offset)
{
  size_t slot;

  if (itl_g_history_limit == 0) return;

  slot = (itl_g_history_head + itl_g_history_count) % (TL_HISTORY_MAX_SIZE);
  itl_g_history_offsets[slot] = offset;
  itl_g_history_durable_offsets[slot] = durable_offset;

  if (itl_g_history_count < itl_g_history_limit) {
    itl_g_history_count += 1;
  } else {
    itl_g_history_head = (itl_g_history_head + 1) % (TL_HISTORY_MAX_SIZE);
  }
}

/* Drops the offset ring and the cached read descriptor. A scan calls this to
   start over, and an aborting scan calls it to abandon what it read. */
ITL_DEF void itl_history_offsets_reset(void)
{
  itl_g_history_head = 0;
  itl_g_history_count = 0;
  itl_g_history_total_count = 0;
  itl_history_read_fd_invalidate();
}

/* Accounts for a leading span that was dropped from the history file. The
   retained entries keep their order and event numbers, and every live offset
   shrinks by the same count. The cached read buffer maps the previous offsets
   and is dropped. */
ITL_DEF void itl_history_offsets_shift(size_t removed_byte_count)
{
  size_t index;

  TL_ASSERT(removed_byte_count <= itl_g_history_file_size);

  for (index = 0; index < itl_g_history_count; ++index) {
    size_t slot = (itl_g_history_head + index) % (TL_HISTORY_MAX_SIZE);

    TL_ASSERT(itl_g_history_offsets[slot] >= removed_byte_count);
    itl_g_history_offsets[slot] -= removed_byte_count;
  }

  itl_g_history_file_size -= removed_byte_count;
  itl_history_read_fd_invalidate();
}

/* Scans the open file from the start, rebuilding the offset ring, the recorded
   file size, and the trailing-newline flag. Returns false on a read error or a
   non-text byte. Touches neither the path nor the draft. */
ITL_DEF bool itl_history_scan_fd(ITL_FILE file)
{
  char file_buffer[ITL_HISTORY_FILE_BUFFER_SIZE];
  bool escape_pending = false;
  size_t entry_start = 0;
  size_t file_pos = 0;
  size_t retained_limit = itl_g_history_limit;
  uint8_t last_byte = (uint8_t) '\n';

  if (!ITL_FILE_SEEK(file, 0)) {
    return false;
  }

  itl_g_history_limit = TL_HISTORY_MAX_SIZE;
  itl_history_offsets_reset();

  for (;;) {
    int read_amount =
        (int) ITL_READ(file, file_buffer, ITL_HISTORY_FILE_BUFFER_SIZE);
    size_t i;

    if (read_amount < 0 && errno == EINTR) continue;
    if (read_amount < 0) {
      itl_history_offsets_reset();
      itl_g_history_limit = retained_limit;
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
        itl_history_push_offset(entry_start, entry_start);
        itl_g_history_total_count += 1;
        entry_start = file_pos + 1;
        continue;
      }

      if (ch == '\r') {
        continue;
      }

      /* Loaded a binary file on accident? The bytes are classified here without
         ctype, because a locale decides which of them are control bytes and a
         history file has to read the same way everywhere. */
      if ((ch < 0x20 && ch != '\t' && ch != '\v' && ch != '\f') || ch == 0x7f) {
        ITL_TRACELN("non-text byte '%X' detected in history file at offset "
                    "%zu\n",
                    (uint8_t) ch, file_pos);
        errno = EINVAL;
        itl_history_offsets_reset();
        itl_g_history_limit = retained_limit;
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
  if (itl_g_history_count > retained_limit) {
    size_t removed_count = itl_g_history_count - retained_limit;
    itl_g_history_head =
        (itl_g_history_head + removed_count) % (TL_HISTORY_MAX_SIZE);
    itl_g_history_count = retained_limit;
    itl_history_read_fd_invalidate();
  }
  itl_g_history_limit = retained_limit;

  return true;
}

ITL_DEF bool itl_history_search_snapshot_scan(const char *contents, size_t size)
{
  bool escape_pending = false;
  size_t entry_start = 0;
  size_t position;

  itl_g_history_search_snapshot.contents = contents;
  itl_g_history_search_snapshot.size = size;
  itl_g_history_search_snapshot.head = 0;
  itl_g_history_search_snapshot.count = 0;

  for (position = 0; position < size; ++position) {
    uint8_t ch = (uint8_t) contents[position];

    if (escape_pending) {
      escape_pending = false;
      continue;
    }
    if (ch == '\\') {
      escape_pending = true;
      continue;
    }
    if (ch == '\n') {
      if (itl_g_history_limit > 0) {
        size_t slot = (itl_g_history_search_snapshot.head +
                       itl_g_history_search_snapshot.count) %
                      TL_HISTORY_MAX_SIZE;
        itl_g_history_search_snapshot.offsets[slot] = entry_start;
        if (itl_g_history_search_snapshot.count < itl_g_history_limit) {
          itl_g_history_search_snapshot.count += 1;
        } else {
          itl_g_history_search_snapshot.head =
              (itl_g_history_search_snapshot.head + 1) % TL_HISTORY_MAX_SIZE;
        }
      }
      entry_start = position + 1;
      continue;
    }
    if (ch == '\r') continue;
    if ((ch < 0x20 && ch != '\t' && ch != '\v' && ch != '\f') || ch == 0x7f)
      return false;
  }

  return true;
}

ITL_DEF void itl_history_search_snapshot_begin(void)
{
  const char *contents = NULL;
  size_t size = 0;

  itl_g_history_search_snapshot.is_active = false;
  if (itl_g_history_search_snapshot_callback == NULL) return;
  if (!itl_g_history_search_snapshot_callback(&contents, &size)) return;
  if (contents == NULL && size != 0) return;
  if (!itl_history_search_snapshot_scan(contents, size)) return;

  itl_g_history_search_snapshot.is_active = true;
}

ITL_DEF void itl_history_search_snapshot_end(void)
{
  itl_g_history_search_snapshot.contents = NULL;
  itl_g_history_search_snapshot.size = 0;
  itl_g_history_search_snapshot.head = 0;
  itl_g_history_search_snapshot.count = 0;
  itl_g_history_search_snapshot.is_active = false;
}

ITL_DEF size_t itl_history_search_count(void)
{
  return itl_g_history_search_snapshot.is_active
             ? itl_g_history_search_snapshot.count
             : itl_g_history_count;
}

ITL_DEF size_t itl_history_search_index_to_offset(size_t index)
{
  if (!itl_g_history_search_snapshot.is_active)
    return itl_history_index_to_offset(index);

  TL_ASSERT(index < itl_g_history_search_snapshot.count);
  return itl_g_history_search_snapshot.offsets
      [(itl_g_history_search_snapshot.head + index) % TL_HISTORY_MAX_SIZE];
}

ITL_DEF bool itl_history_search_prepare(void)
{
  return itl_g_history_search_snapshot.is_active
             ? true
             : itl_history_ensure_read_buffer();
}

ITL_DEF bool itl_history_search_decode_entry(size_t offset, char *decoded,
                                             size_t capacity,
                                             size_t *decoded_size_out)
{
  if (!itl_g_history_search_snapshot.is_active)
    return itl_history_decode_entry_buffered(offset, decoded, capacity,
                                             decoded_size_out);

  return itl_history_decode_entry_bytes(
      itl_g_history_search_snapshot.contents,
      itl_g_history_search_snapshot.size, offset, decoded, capacity,
      decoded_size_out);
}

/* Returns TL_SUCCESS or TL_ERROR, sets errno on failure. The loader scans the
   file once, keeps the most recent entry offsets, and freezes the encoded bytes
   into the shell's private snapshot. */
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
   skipped unless the caller requests them. Returns true on a successful write
   or duplicate skip. Returns false when another entry is skipped or the write
   fails. */
ITL_DEF bool itl_history_append_to_file(const itl_string_t *str,
                                        bool should_require_terminal,
                                        bool should_allow_duplicate)
{
  ITL_FILE append_file;
  itl_char_buf_t buffer;
  itl_char_buf_t durable_buffer;
  const char *durable_data;
  size_t durable_size;
  long real_end;
  long actual_end;
  size_t private_end;
  size_t unterminated_offset;
  size_t new_offset;
  bool is_duplicate = false;
  bool had_private_unterminated_tail;
  bool has_durable_buffer = false;
  bool needs_durable_separator = false;

  itl_g_last_history_event_number = 0;
  if (itl_g_history_path == NULL || itl_g_history_file_is_bad ||
      itl_g_history_limit == 0)
  {
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

  /* The private snapshot, not the durable file tail, owns duplicate
     suppression and event numbering for this shell. */
  if (itl_g_history_count > 0) {
    char newest[ITL_STRING_MAX_LEN];
    size_t newest_size;

    if (!itl_history_ensure_read_buffer()) return false;
    if (itl_history_decode_entry_buffered(
            itl_history_index_to_offset(itl_g_history_count - 1), newest,
            sizeof(newest), &newest_size))
    {
      is_duplicate = itl_string_equal_bytes(str, newest, newest_size);
    }
  }
  if (is_duplicate && !should_allow_duplicate) {
    itl_g_last_history_event_number = itl_g_history_total_count;
    return true;
  }

  private_end =
      itl_g_history_read_buffer == NULL
          ? 0
          : itl_g_history_read_buffer_offset + itl_g_history_read_buffer->size;
  unterminated_offset = private_end;

  /* The escaped entry and its two separators are the whole record, so the
     encode grows the buffer at most once. */
  itl_char_buf_init(&buffer);
  itl_char_buf_reserve(&buffer, str->size * 2 + 2);

  /* When the file does not end on a newline, write a separator first so the new
     entry starts its own physical line instead of gluing onto the previous one.
   */
  had_private_unterminated_tail = !itl_g_history_ends_with_newline;
  if (had_private_unterminated_tail) {
    size_t position;
    bool escape_pending = false;

    unterminated_offset = itl_g_history_read_buffer_offset;
    for (position = 0; position < itl_g_history_read_buffer->size; ++position) {
      uint8_t ch = (uint8_t) itl_g_history_read_buffer->data[position];

      if (escape_pending) {
        escape_pending = false;
      } else if (ch == '\\') {
        escape_pending = true;
      } else if (ch == '\n') {
        unterminated_offset = itl_g_history_read_buffer_offset + position + 1;
      }
    }
    itl_char_buf_append_byte(&buffer, '\n');
  }

  itl_char_buf_append_string_escaped(&buffer, str);
  itl_char_buf_append_byte(&buffer, '\n');

  append_file = ITL_FILE_OPEN_FOR_APPEND(itl_g_history_path);
  if (ITL_FILE_IS_BAD(append_file)) {
    ITL_TRACELN("could not open history file for append (%s): %s\n",
                itl_g_history_path, strerror(errno));
    itl_g_history_file_is_bad = true;
    ITL_FREE(buffer.data);
    return false;
  }

  /* Persist at the real shared-file end, while the navigable offset belongs to
     this shell's private encoded snapshot. */
  real_end = ITL_FILE_SEEK_END(append_file);
  if (real_end < 0) {
    ITL_FILE_CLOSE(append_file);
    ITL_FREE(buffer.data);
    itl_g_history_file_is_bad = true;
    return false;
  }
  if (real_end > 0) {
    char last_byte;

    if (!ITL_FILE_SEEK(append_file, (size_t) real_end - 1) ||
        ITL_READ(append_file, &last_byte, 1) != 1 ||
        ITL_FILE_SEEK_END(append_file) != real_end)
    {
      ITL_FILE_CLOSE(append_file);
      ITL_FREE(buffer.data);
      itl_g_history_file_is_bad = true;
      return false;
    }
    needs_durable_separator = last_byte != '\n';
  }
  new_offset = private_end + (had_private_unterminated_tail ? 1 : 0);

  durable_data = buffer.data;
  durable_size = buffer.size;
  if (needs_durable_separator != had_private_unterminated_tail) {
    size_t private_record_offset = had_private_unterminated_tail ? 1 : 0;

    itl_char_buf_init(&durable_buffer);
    has_durable_buffer = true;
    itl_char_buf_reserve(&durable_buffer, buffer.size + 1);
    if (needs_durable_separator)
      itl_char_buf_append_byte(&durable_buffer, '\n');
    itl_char_buf_append_bytes(&durable_buffer,
                              buffer.data + private_record_offset,
                              buffer.size - private_record_offset);
    durable_data = durable_buffer.data;
    durable_size = durable_buffer.size;
  }

  if (itl_write_all(append_file, durable_data, durable_size) < durable_size) {
    ITL_TRACELN("could not append to history file (%s): %s\n",
                itl_g_history_path, strerror(errno));
    ITL_FILE_CLOSE(append_file);
    if (has_durable_buffer) ITL_FREE(durable_buffer.data);
    ITL_FREE(buffer.data);
    itl_g_history_file_is_bad = true;
    return false;
  }

  actual_end = ITL_FILE_TELL(append_file);
  if (actual_end < 0) actual_end = real_end + (long) durable_size;
  if (ITL_FILE_CLOSE(append_file) != 0) {
    if (has_durable_buffer) ITL_FREE(durable_buffer.data);
    ITL_FREE(buffer.data);
    itl_g_history_file_is_bad = true;
    return false;
  }

  itl_g_history_file_size = (size_t) actual_end;
  itl_g_history_ends_with_newline = true;
  if (had_private_unterminated_tail && private_end > unterminated_offset) {
    itl_history_push_offset(unterminated_offset, unterminated_offset);
    itl_g_history_total_count += 1;
  }
  itl_history_push_offset(new_offset, (size_t) real_end +
                                          (needs_durable_separator ? 1 : 0));
  itl_g_history_total_count += 1;
  itl_g_last_history_event_number = itl_g_history_total_count;
  itl_history_append_read_buffer(private_end, buffer.data, buffer.size);
  ITL_TRACELN("appended history entry at offset %zu, %zu entries now\n",
              new_offset, itl_g_history_count);

  if (has_durable_buffer) ITL_FREE(durable_buffer.data);
  ITL_FREE(buffer.data);

  return true;
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
    if (read_amount < 0 && errno == EINTR) continue;
    if (read_amount < 0) {
      ret = TL_ERROR;
      break;
    }
    if (read_amount == 0) break;

    total_written =
        itl_write_all(out_file, file_buffer, (size_t) read_amount);
    if (total_written < (size_t) read_amount) {
      ret = TL_ERROR;
      break;
    }
  }

  if (ITL_FILE_CLOSE(in_file) != 0) ret = TL_ERROR;
  if (ITL_FILE_CLOSE(out_file) != 0) ret = TL_ERROR;

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

/* The key Ctrl-Z reports. A host that keeps job control to itself, such as a
   shell, can make it undo at the prompt instead. */
ITL_DEF int itl_ctrl_z_key(void)
{
#if defined TL_CTRL_Z_UNDO
  return TL_KEY_UNDO;
#else
  return TL_KEY_SUSPEND;
#endif /* TL_CTRL_Z_UNDO */
}

#if defined ITL_POSIX || defined ITL_WIN32
/* The modifier parameter of a CSI key is one plus a bit set of shift 1, alt 2,
   and ctrl 4. The plain mapping below keeps the forms the editor always read:
   2 is shift, 3 is alt, and 5 is ctrl. */
ITL_DEF int itl_csi_modifier(unsigned value)
{
  switch (value) {
  case 2: return TL_MOD_SHIFT;
  case 3: return TL_MOD_ALT;
  case 5: return TL_MOD_CTRL;
  }

  return 0;
}

/* A character key with modifiers, reported by the kitty keyboard protocol as
   CSI code ; modifier u or by xterm's modifyOtherKeys as CSI 27 ; modifier ;
   code ~. The read path already turned every such key with a legacy form into
   its legacy bytes, so only Ctrl-Shift-Z reaches here bound, where the shifted
   key may be reported as the lower or the upper case letter. A direct parse
   also maps Ctrl-Z. The lock bits 64 and 128 are ignored. */
ITL_DEF int itl_esc_parse_modified_key(unsigned code, unsigned modifier)
{
  unsigned bits = (modifier > 0) ? modifier - 1 : 0;
  bool is_ctrl = (bits & 4) != 0;
  bool is_shift = (bits & 1) != 0 || code == 'Z';

  if ((bits & ~(1u | 4u | 64u | 128u)) != 0 || !is_ctrl) {
    return TL_KEY_UNKN;
  }
  if (code == 'z' || code == 'Z') {
    return is_shift ? TL_KEY_REDO : itl_ctrl_z_key();
  }

  return TL_KEY_UNKN;
}

#define ITL_CSI_PARAMETER_MAX 4

/* A CSI or SS3 sequence whose first byte after the introducer is a parameter
   byte. Every parameter and intermediate byte is consumed up to the final
   byte, so an unbound sequence never leaks its tail as typed text. A private
   marker such as '<' or '?' makes the whole sequence unbound, and a ':'
   sub-parameter is skipped up to the next ';'. */
ITL_DEF int itl_esc_parse_csi_parameters(uint8_t byte)
{
  unsigned parameters[ITL_CSI_PARAMETER_MAX] = {0};
  size_t index = 0;
  bool is_private = false;
  bool is_subparameter = false;
  int modifier;

  while (byte >= 0x20 && byte < 0x40) {
    if (byte >= '0' && byte <= '9') {
      if (!is_subparameter && index < ITL_CSI_PARAMETER_MAX &&
          parameters[index] < 100000)
      {
        parameters[index] = parameters[index] * 10 + (unsigned) (byte - '0');
      }
    } else if (byte == ';') {
      index += 1;
      is_subparameter = false;
    } else if (byte == ':') {
      is_subparameter = true;
    } else {
      is_private = true;
    }
    ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);
  }

  if (is_private) {
    return TL_KEY_UNKN;
  }

  modifier = itl_csi_modifier(parameters[1]);

  switch (byte) {
  case '~':
    switch (parameters[0]) {
    case 1:
    case 7: return modifier | TL_KEY_HOME;
    case 4:
    case 8: return modifier | TL_KEY_END;
    case 3: return modifier | TL_KEY_DELETE;
    /* Bracketed paste opens with ESC [ 200 ~. Its closing ESC [ 201 ~ outside
       a paste, Insert, and the other keys are unbound. */
    case 200: return TL_KEY_PASTE_BEGIN;
    case 27: return itl_esc_parse_modified_key(parameters[2], parameters[1]);
    }
    return TL_KEY_UNKN;

  case 'u': return itl_esc_parse_modified_key(parameters[0], parameters[1]);

  case 'A': return modifier | TL_KEY_UP;
  case 'B': return modifier | TL_KEY_DOWN;
  case 'C': return modifier | TL_KEY_RIGHT;
  case 'D': return modifier | TL_KEY_LEFT;
  case 'F': return modifier | TL_KEY_END;
  case 'H': return modifier | TL_KEY_HOME;
  case 'Z': return TL_MOD_SHIFT | TL_KEY_TAB;
  }

  return TL_KEY_UNKN;
}

ITL_DEF int itl_esc_parse_vt(uint8_t byte)
{
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

      case 'y': return TL_KEY_YANK_POP;
      case 't': return TL_KEY_TRANSPOSE | TL_MOD_ALT;

      case '.':
      case '_': return TL_KEY_LAST_ARGUMENT;

      case '>': return TL_KEY_HISTORY_END;
      case ',':
      case '<': return TL_KEY_HISTORY_BEGINNING;

      case 13:
      case 10: return TL_KEY_ENTER | TL_MOD_ALT;

      default: return TL_KEY_CHAR | TL_MOD_ALT;
      }
    }

    ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);

    if (byte >= 0x20 && byte < 0x40) {
      return itl_esc_parse_csi_parameters(byte);
    }

    switch (byte) {
    case 'A': return TL_KEY_UP;
    case 'B': return TL_KEY_DOWN;
    case 'C': return TL_KEY_RIGHT;
    case 'D': return TL_KEY_LEFT;

    case 'F': return TL_KEY_END;
    case 'H': return TL_KEY_HOME;
    case 'Z': return TL_MOD_SHIFT | TL_KEY_TAB;
    }

    return TL_KEY_UNKN;
  }

  ITL_TRY(!iscntrl(byte), return TL_KEY_UNKN);
  return TL_KEY_CHAR;
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
    case 15: event = TL_KEY_TAB | TL_MOD_SHIFT; break;

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

/* Ctrl-X is a prefix, so the key is the byte that follows it. Ctrl-X Ctrl-E
   edits the line in the host's editor and Ctrl-X Ctrl-U undoes. */
ITL_DEF int itl_esc_parse_ctrl_x(void)
{
  uint8_t byte;

  ITL_TRY_READ_BYTE(&byte, return TL_KEY_UNKN);

  switch (byte) {
  case 5: return TL_KEY_EDIT_EXTERNAL; /* ctrl e */
  case 21: return TL_KEY_UNDO;         /* ctrl u */
  }

  /* The chord is not bound, so the byte starts the next key and an arrow after
     ctrl x still moves rather than inserting the tail of its sequence. */
  itl_g_pushback_byte = byte;

  return TL_KEY_UNKN;
}

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
  case 26: return itl_ctrl_z_key(); /* ctrl z */

  case 9: return TL_KEY_TAB;
#if !defined ITL_WIN32
  /* A terminal sends a null byte for ctrl space. The Windows console gives the
     same byte as the lead of a legacy scan code pair. */
  case 0: return TL_KEY_TAB;
#endif
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

  case 20: return TL_KEY_TRANSPOSE; /* ctrl t */
  case 25: return TL_KEY_YANK;      /* ctrl y */
  case 24: return itl_esc_parse_ctrl_x();
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

/* *rows, *cols can be NULL. */
ITL_DEF bool itl_tty_get_size(size_t *rows, size_t *cols)
{
  size_t temp_rows, temp_cols;
  char *emacs_buf = NULL;
#if defined ITL_WIN32
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
#if defined ITL_WIN32
  ITL_TRY(
      GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &buffer_info),
      return false);

  ITL_PTR_ASSIGN(cols, (size_t) (buffer_info.srWindow.Right -
                                 buffer_info.srWindow.Left + 1));
  ITL_PTR_ASSIGN(rows, (size_t) (buffer_info.srWindow.Bottom -
                                 buffer_info.srWindow.Top + 1));

  return true;
#else
  ITL_TRY(ioctl(STDOUT_FILENO, TIOCGWINSZ, &window) == 0, return false);

  ITL_PTR_ASSIGN(rows, (size_t) window.ws_row);
  ITL_PTR_ASSIGN(cols, (size_t) window.ws_col);

  return true;
#endif
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
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_le_prev_ghost_len = 0;
/* Whether the previous frame drew the right prompt on the first input row, so
   a resize knows that row reached the right edge less one column. */
ITL_DEF ITL_THREAD_LOCAL bool itl_g_le_prev_right_prompt_is_shown = false;

/* The host right prompt and its display width, zero when nothing is drawn. A
   hold keeps it away while a history search or a transient redraw owns the
   first row. */
ITL_DEF ITL_THREAD_LOCAL const char *itl_g_right_prompt = NULL;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_right_prompt_width = 0;
ITL_DEF ITL_THREAD_LOCAL bool itl_g_right_prompt_is_held = false;

/* The prompt a submitted line is redrawn with, or NULL to keep the line. */
ITL_DEF ITL_THREAD_LOCAL const char *itl_g_transient_prompt = NULL;

/* The host hint callback, or NULL when the hint rows are off. */
ITL_DEF ITL_THREAD_LOCAL tl_hint_fn itl_g_hint_callback = NULL;

/* The most text bytes and style bytes of one hint the editor keeps. */
#define ITL_HINT_TEXT_MAX 512
#define ITL_HINT_SGR_MAX  32
/* The most rows one hint takes, the header and three body rows. */
#define ITL_HINT_ROWS_MAX 4
/* Every hint row starts with this indent. */
#define ITL_HINT_INDENT       "  "
#define ITL_HINT_INDENT_WIDTH 2
/* The bytes of one laid-out hint, every row with its style, indent, text,
   ellipsis, reset, and line feed. */
#define ITL_HINT_LAYOUT_MAX                                                    \
  (ITL_HINT_ROWS_MAX * (ITL_HINT_SGR_MAX + 16) + ITL_HINT_TEXT_MAX)

/* The host text of the hint with control bytes blanked. The header is its
   first header_len bytes and the body follows without the line break. The
   style is copied, since the host pointer dies with the call. The layout of
   the next frame was cut for these columns and this row budget. */
ITL_DEF ITL_THREAD_LOCAL char itl_g_hint_source[ITL_HINT_TEXT_MAX + 1] = {0};
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_hint_source_len = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_hint_header_len = 0;
ITL_DEF ITL_THREAD_LOCAL char itl_g_hint_sgr[ITL_HINT_SGR_MAX] = {0};
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_hint_layout_cols = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_hint_layout_rows = 0;

/* The rows the next frame wants and the rows the screen holds, each row its
   style, indent, text, and reset, with a line feed between rows. The shown
   length and row count are zero when nothing sits under the input. The two
   compare byte for byte, so an unchanged hint costs no output. */
ITL_DEF ITL_THREAD_LOCAL char itl_g_hint_next[ITL_HINT_LAYOUT_MAX] = {0};
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_hint_next_len = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_hint_next_rows = 0;
ITL_DEF ITL_THREAD_LOCAL char itl_g_hint_shown[ITL_HINT_LAYOUT_MAX] = {0};
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_hint_shown_len = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_hint_shown_rows = 0;

/* Record that no hint row sits under the input, after the rows were erased or
   the screen under the block was cleared. */
ITL_DEF void itl_hint_forget_shown(void)
{
  itl_g_hint_shown_len = 0;
  itl_g_hint_shown_rows = 0;
}

/* A menu or a history search holds the row away while it is open. A finished
   line keeps it away until the next line starts. */
ITL_DEF ITL_THREAD_LOCAL int itl_g_hint_hold_count = 0;
ITL_DEF ITL_THREAD_LOCAL bool itl_g_hint_is_closed = false;

/* A key that waits for the next one before it acts. While one is open the hint
   rows name it and the keys that complete it, in place of the host's hint. */
typedef enum
{
  ITL_PREFIX_NONE = 0,
  ITL_PREFIX_CTRL_X,
  ITL_PREFIX_VI_REGISTER,
  ITL_PREFIX_VI_FIND,
  ITL_PREFIX_VI_REPLACE,
  ITL_PREFIX_VI_EX
} itl_prefix_kind;

ITL_DEF ITL_THREAD_LOCAL itl_prefix_kind itl_g_prefix_kind = ITL_PREFIX_NONE;
/* The byte that opened a find chord, one of f, F, t, and T. */
ITL_DEF ITL_THREAD_LOCAL uint8_t itl_g_prefix_key = 0;

ITL_DEF ITL_THREAD_LOCAL bool itl_g_tty_plain_append_pending = false;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_tty_plain_append_width = 0;
#if !defined NDEBUG
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_debug_append_refresh_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_debug_full_refresh_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_debug_metrics_scan_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_debug_line_serialization_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_debug_ghost_history_scan_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_debug_history_candidate_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_debug_search_highlight_count = 0;
#endif

/* The ghost suggestion drawn dimmed after the cursor, and its byte length.
   Right or End accepts it. */
ITL_DEF ITL_THREAD_LOCAL char itl_g_ghost[ITL_STRING_MAX_LEN] = {0};
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_ghost_len = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_ghost_width = 0;

/* True when the sticky target corrects the case of what is typed, so accepting
   the ghost rewrites the whole line instead of appending the suffix. */
ITL_DEF ITL_THREAD_LOCAL bool itl_g_ghost_should_replace_line = false;

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
ITL_DEF ITL_THREAD_LOCAL bool itl_g_highlight_follows_cursor = false;
ITL_DEF ITL_THREAD_LOCAL tl_wake_fn itl_g_wake_callback = NULL;

/* The host idle hook, its delay and repeat interval, and the moment the next
   call is due, zero while no call is due. */
ITL_DEF ITL_THREAD_LOCAL tl_idle_fn itl_g_idle_callback = NULL;
ITL_DEF ITL_THREAD_LOCAL int itl_g_idle_delay_ms = 0;
ITL_DEF ITL_THREAD_LOCAL int itl_g_idle_repeat_ms = 0;
ITL_DEF ITL_THREAD_LOCAL uint64_t itl_g_idle_due_ms = 0;

/* The reset that closes every colored span, matching the ghost text's own
   reset. Each span carries its own opening SGR from the host. */
#define ITL_HIGHLIGHT_RESET "\x1b[0m"

/* The grey of every secondary text the editor writes for itself, the ghost
   suggestion, a candidate description, and the menu rows. */
#define ITL_DIM_SGR "\x1b[90m"

/* Whether the colored parts of the editor output are written. The host clears
   it for a session that refuses color. */
ITL_DEF ITL_THREAD_LOCAL int itl_g_colors_enabled = 1;

/* The escape sequence to write, or the empty string once color is refused. */
ITL_DEF const char *itl_color_sequence(const char *sequence)
{
  return itl_g_colors_enabled ? sequence : "";
}

ITL_DEF bool itl_should_run_highlight(void)
{
  return itl_g_colors_enabled != 0 && itl_g_highlight_callback != NULL;
}

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
#if defined ITL_WIN32
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
  return itl_g_colors_enabled && itl_term_supports_256_color()
             ? ITL_FLASH_TINT_ON
             : ITL_FLASH_REVERSE_ON;
}
ITL_DEF const char *itl_flash_sgr_off(void)
{
  return itl_g_colors_enabled && itl_term_supports_256_color()
             ? ITL_FLASH_TINT_OFF
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
  uint32_t start;
  uint32_t end;
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

ITL_DEF void itl_search_spans_clear(void)
{
  itl_g_search_spans_active = false;
  itl_g_search_span_count = 0;
}

ITL_DEF void itl_search_span_add(size_t start, size_t end, const char *sgr)
{
  itl_g_search_spans[itl_g_search_span_count].start = start;
  itl_g_search_spans[itl_g_search_span_count].end = end;
  itl_g_search_spans[itl_g_search_span_count].sgr = sgr;
  itl_g_search_span_count += 1;
}

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
    itl_g_le_prev_spans[s].start = (uint32_t) spans[s].start;
    itl_g_le_prev_spans[s].end = (uint32_t) spans[s].end;
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

/* Whether the spans are the ones the previous frame drew. A caret move that
   keeps them leaves the drawn line valid. */
ITL_DEF bool itl_le_prev_spans_equal(const tl_highlight_span *spans,
                                     size_t count)
{
  size_t s;
  if (!itl_g_le_prev_spans_usable || count != itl_g_le_prev_span_count) {
    return false;
  }
  for (s = 0; s < count; ++s) {
    if (spans[s].start != itl_g_le_prev_spans[s].start ||
        spans[s].end != itl_g_le_prev_spans[s].end ||
        strcmp(spans[s].sgr, itl_g_le_prev_spans[s].sgr) != 0)
    {
      return false;
    }
  }
  return true;
}

ITL_DEF size_t itl_spans_keep_valid(const tl_highlight_span *source,
                                    size_t source_count,
                                    tl_highlight_span *destination,
                                    size_t line_length)
{
  size_t kept_count = 0;
  size_t s;

  for (s = 0; s < source_count; ++s) {
    if (source[s].start < source[s].end && source[s].end <= line_length &&
        source[s].sgr != NULL)
    {
      destination[kept_count++] = source[s];
    }
  }

  return kept_count;
}

ITL_DEF size_t itl_le_collect_highlight(const char *render,
                                        tl_highlight_span *destination,
                                        size_t line_length, size_t cursor)
{
  tl_highlight hl;

  hl.spans = destination;
  hl.count = 0;
  hl.capacity = ITL_HIGHLIGHT_MAX_SPANS;
  hl.cursor = cursor;

  if (!itl_g_highlight_callback(render, &hl)) {
    return 0;
  }

  return itl_spans_keep_valid(destination,
                              ITL_MIN(hl.count, ITL_HIGHLIGHT_MAX_SPANS),
                              destination, line_length);
}

ITL_DEF size_t itl_merge_visual_spans(
    const tl_highlight_span *syntax, size_t syntax_count,
    const tl_highlight_span *selection, size_t selection_count,
    size_t line_length, tl_highlight_span *out, size_t out_capacity)
{
  size_t count = 0;
  size_t position = 0;
  size_t next_syntax = 0;
  size_t next_selection = 0;

  while (position < line_length && count < out_capacity) {
    const char *color = NULL;
    size_t color_end = line_length;
    const char *selection_sgr = NULL;
    size_t selection_end = line_length;
    size_t run_end;
    const char *sgr;

    while (next_syntax < syntax_count && syntax[next_syntax].end <= position) {
      next_syntax += 1;
    }

    if (next_syntax < syntax_count) {
      if (syntax[next_syntax].start <= position) {
        color = syntax[next_syntax].sgr;
        color_end = syntax[next_syntax].end;
      } else {
        color_end = syntax[next_syntax].start;
      }
    }

    while (next_selection < selection_count &&
           selection[next_selection].end <= position)
    {
      next_selection += 1;
    }

    if (next_selection < selection_count) {
      if (selection[next_selection].start <= position) {
        selection_sgr = selection[next_selection].sgr;
        selection_end = selection[next_selection].end;
      } else {
        selection_end = selection[next_selection].start;
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
  if (!itl_tty_get_size(&rows, &cols)) {
    return false;
  }
  return (cols != itl_g_tty_prev_cols) || (rows != itl_g_tty_prev_rows);
}

/* Blocks on the console input handle until a keystroke or resize is queued,
   the Windows counterpart to the POSIX poll. The handle wakes the moment any
   record arrives. A keypress is served without the latency of a fixed sleep.
   The twenty millisecond timeout keeps polling the console size since a resize
   raises no signal on Windows. A non-character record, a key release, a lone
   modifier, or a mouse move would keep the handle signaled and spin the wait.
   Such a record is consumed here. A resize record marks the line for a redraw
   and returns to the caller. A real keystroke is left in the buffer for the
   _getch reader, told apart by _kbhit. A timeout_ms that is not negative also
   ends the wait, and 0 is returned when it passed with nothing to read. */
ITL_DEF int itl_wait_for_input_until(int timeout_ms)
{
  HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
  uint64_t started_ms = itl_monotonic_ms();
  for (;;) {
    DWORD slice_ms = 20;
    if (itl_g_pushback_byte != -1 || _kbhit() != 0) {
      return 1;
    }
    if (timeout_ms >= 0) {
      uint64_t waited_ms = itl_monotonic_ms() - started_ms;
      if (waited_ms >= (uint64_t) timeout_ms) {
        return 0;
      }
      if ((uint64_t) timeout_ms - waited_ms < slice_ms) {
        slice_ms = (DWORD) ((uint64_t) timeout_ms - waited_ms);
      }
    }
    if (WaitForSingleObject(handle, slice_ms) != WAIT_OBJECT_0) {
      if (itl_win_console_resized()) {
        itl_g_tty_changed_size = 1;
        return 1;
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
      return 1;
    }
    if (ReadConsoleInput(handle, &record, 1, &count) && count > 0 &&
        record.EventType == WINDOW_BUFFER_SIZE_EVENT)
    {
      itl_g_tty_changed_size = 1;
      return 1;
    }
  }
}

ITL_DEF void itl_wait_for_input(void) { (void) itl_wait_for_input_until(-1); }
#endif /* ITL_WIN32 */

typedef struct itl_le_metrics itl_le_metrics_t;

struct itl_le_metrics
{
  size_t total_rows; /* visual rows the whole buffer occupies, >= 1 */
  size_t cursor_row; /* 0-based visual row of the cursor */
  size_t cursor_col; /* 0-based visual column of the cursor */
};

/* Record whether this frame, text or cursor-only, parked the caret at the line
   end, so the append fast path on the next keystroke knows the physical cursor
   sits at the append point. */
ITL_DEF void itl_le_commit_geometry(itl_le_metrics_t m, bool is_cursor_at_end)
{
  itl_g_le_prev_total_rows = m.total_rows;
  itl_g_le_prev_cursor_row = m.cursor_row + 1;
  itl_g_le_prev_cursor_col = m.cursor_col;
  itl_g_le_prev_cursor_at_end = is_cursor_at_end;
}

/* Remember the line and the spans this text refresh drew, so the next
   keystroke can take the append fast path. */
ITL_DEF void itl_le_commit_render(const char *render, size_t render_len,
                                  size_t line_length,
                                  const tl_highlight_span *spans,
                                  size_t span_count)
{
  memcpy(itl_g_le_prev_render, render, render_len);
  itl_g_le_prev_render_len = render_len;
  itl_g_le_prev_length = line_length;
  itl_le_save_prev_spans(spans, span_count);
}

ITL_DEF bool itl_le_serialize_line(itl_le_t *le)
{
  if (itl_g_serialized_line_ready) {
    return true;
  }

  if (itl_g_tty_plain_append_pending && itl_g_le_prev_cursor_at_end &&
      le->cursor_position == le->line->length && le->line->length > 0)
  {
    const itl_utf8_t *appended = &le->line->chars[le->line->length - 1];

    if (itl_g_le_prev_render_len + appended->size == le->line->size &&
        le->line->size < sizeof(itl_g_serialized_line))
    {
      memcpy(itl_g_serialized_line, itl_g_le_prev_render,
             itl_g_le_prev_render_len);
      memcpy(itl_g_serialized_line + itl_g_le_prev_render_len, appended->bytes,
             appended->size);
      itl_g_serialized_line[le->line->size] = '\0';
      itl_g_serialized_line_ready = true;

      return true;
    }
  }

#if !defined NDEBUG
  itl_g_debug_line_serialization_count += 1;
#endif

  if (itl_string_to_cstr(le->line, itl_g_serialized_line,
                         sizeof(itl_g_serialized_line)) != TL_SUCCESS)
  {
    return false;
  }

  itl_g_serialized_line_ready = true;

  return true;
}

ITL_DEF void itl_le_invalidate_prev_frame(void)
{
  itl_g_le_prev_total_rows = 1;
  itl_g_le_prev_cursor_row = 1;
  itl_g_le_prev_cursor_col = 0;
  itl_g_le_prev_cursor_at_end = false;
  itl_g_le_prev_render_len = 0;
  itl_g_le_prev_length = 0;
  itl_g_le_prev_ghost_len = 0;
  itl_g_le_prev_spans_usable = false;
  itl_g_le_prev_right_prompt_is_shown = false;
  itl_hint_forget_shown();
}

ITL_DEF void itl_le_tty_move_to_block_top(itl_char_buf_t *b)
{
  if (itl_g_le_prev_cursor_row > 1) {
    ITL_TTY_MOVE_UP(b, itl_g_le_prev_cursor_row - 1);
  }
}

/* The cells the rendered prompt occupies at this width, the clamped width
   once the prompt is at or past the terminal width, so the metrics, the
   reflow, and the refresh all wrap the same way the render does. */
ITL_DEF size_t itl_le_prompt_indent(const itl_le_t *le, size_t cols)
{
  size_t effective_width;
  itl_prompt_render_cut(le->prompt, le->prompt_width, cols, &effective_width);
  return effective_width;
}

/* The fewest columns a continuation row keeps for text under the prompt. A
   prompt that leaves fewer would wrap the input into a narrow strip, so its
   continuation rows start near the left edge instead. */
#define ITL_LE_MIN_CONTINUATION_COLUMNS 20
#define ITL_LE_NARROW_CONTINUATION      2

/* Columns each wrapped or continuation row is padded by. The text lines up
   under the first row when the prompt leaves enough room beside it. */
ITL_DEF size_t itl_le_continuation_indent(const itl_le_t *le, size_t cols)
{
  size_t prompt_indent = itl_le_prompt_indent(le, cols);

  if (prompt_indent > ITL_LE_NARROW_CONTINUATION &&
      prompt_indent + ITL_LE_MIN_CONTINUATION_COLUMNS > cols)
  {
    return ITL_LE_NARROW_CONTINUATION;
  }

  return prompt_indent;
}

#define ITL_LE_INDENT(le, cols) itl_le_continuation_indent((le), (cols))

/* A double-width glyph is never split across the right edge. */
ITL_DEF bool itl_wrap_is_early_break(size_t col, size_t char_width, size_t cols)
{
  return char_width == 2 && col + 1 >= cols;
}

ITL_DEF bool itl_wrap_is_break_after(size_t col, size_t cols)
{
  return col >= cols;
}

ITL_DEF size_t itl_wrap_advance_plain_run(size_t col, size_t run_length,
                                          size_t cols, size_t indent,
                                          size_t *row)
{
  size_t first_row_fit = col < cols ? cols - col : 0;
  size_t per_row = indent < cols ? cols - indent : 1;
  size_t remaining;

  if (run_length < first_row_fit) {
    return col + run_length;
  }

  remaining = run_length - first_row_fit;
  *row += 1 + remaining / per_row;

  return indent + remaining % per_row;
}

ITL_DEF void itl_wrap_walk_range(const itl_string_t *line, size_t from,
                                 size_t to, size_t cols, size_t indent,
                                 size_t *row, size_t *col)
{
  size_t i = from;

  while (i < to) {
    size_t run_start = i;
    size_t char_width;

    while (i < to && itl_utf8_is_plain_ascii(line->chars[i])) {
      i += 1;
    }

    if (i > run_start) {
      *col =
          itl_wrap_advance_plain_run(*col, i - run_start, cols, indent, row);
      continue;
    }

    if (ITL_LE_IS_NEWLINE(line->chars[i])) {
      *row += 1;
      *col = indent;
      i += 1;
      continue;
    }

    char_width = itl_line_char_width(line->chars, i);

    if (itl_wrap_is_early_break(*col, char_width, cols)) {
      *row += 1;
      *col = indent;
    }

    *col += char_width;

    if (itl_wrap_is_break_after(*col, cols)) {
      *row += 1;
      *col = indent;
    }

    i += 1;
  }
}

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
  size_t col = itl_le_prompt_indent(le, cols);
  size_t length = le->line->length;

  /* The caret sits to the left of chars[cursor_position], so the walk stops
     there and records before consuming it. */
  if (le->cursor_position <= length) {
    itl_wrap_walk_range(le->line, 0, le->cursor_position, cols, indent, &row,
                        &col);
    m.cursor_row = row;
    m.cursor_col = col;
    itl_wrap_walk_range(le->line, le->cursor_position, length, cols, indent,
                        &row, &col);
  } else {
    itl_wrap_walk_range(le->line, 0, length, cols, indent, &row, &col);
  }

  m.total_rows = row + 1;
  return m;
}

ITL_DEF size_t itl_reflow_row_count(size_t col, size_t ncols)
{
  return ITL_MAX((size_t) 1, (col + ncols - 1) / ncols);
}

ITL_DEF size_t itl_reflow_advance_plain_run(size_t col, size_t run_length,
                                            size_t ocols, size_t ncols,
                                            size_t indent, size_t *rows_above)
{
  size_t first_row_fit = col < ocols ? ocols - col : 0;
  size_t per_row = indent < ocols ? ocols - indent : 1;
  size_t remaining;

  if (run_length < first_row_fit) {
    return col + run_length;
  }

  remaining = run_length - first_row_fit;
  *rows_above += (1 + remaining / per_row) * itl_reflow_row_count(ocols, ncols);

  return indent + remaining % per_row;
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
  size_t col = itl_le_prompt_indent(le, ocols);
  /* The rows count from prompt_rows the way itl_le_compute_metrics counts them,
     so the caret is stepped past a multi-row prompt to the true top of the
     block. */
  size_t rows_above = le->prompt_rows;
  size_t stop = ITL_MIN(le->cursor_position, le->line->length);
  size_t i = 0;

  while (i < stop) {
    size_t run_start = i;
    size_t char_width;

    while (i < stop && itl_utf8_is_plain_ascii(le->line->chars[i])) {
      i += 1;
    }

    if (i > run_start) {
      col = itl_reflow_advance_plain_run(col, i - run_start, ocols, ncols,
                                         indent, &rows_above);
      continue;
    }

    if (ITL_LE_IS_NEWLINE(le->line->chars[i])) {
      /* A right prompt drawn on the first row reached one column short of the
         old right edge, and the terminal reflowed it with that row. */
      size_t row_extent = col;
      if (rows_above == le->prompt_rows && itl_g_le_prev_right_prompt_is_shown)
      {
        row_extent = ITL_MAX(col, ocols - 1);
      }
      rows_above += itl_reflow_row_count(row_extent, ncols);
      col = indent;
      i += 1;
      continue;
    }

    char_width = itl_line_char_width(le->line->chars, i);

    if (itl_wrap_is_early_break(col, char_width, ocols)) {
      rows_above += itl_reflow_row_count(col, ncols);
      col = indent;
    }

    col += char_width;

    if (itl_wrap_is_break_after(col, ocols)) {
      rows_above += itl_reflow_row_count(col, ncols);
      col = indent;
    }

    i += 1;
  }

  /* The caret sits on sub-row col / ncols of its own emitted row. */
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
  size_t col = itl_le_prompt_indent(le, cols);
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
      size_t char_width = itl_line_char_width(le->line->chars, i);

      if (itl_wrap_is_early_break(col, char_width, cols)) {
        row += 1;
        col = indent;
      }

      col += char_width;

      if (itl_wrap_is_break_after(col, cols)) {
        row += 1;
        col = indent;
      }
    }
  }

  return has_best ? best_index : le->line->length;
}

/* Returns the index of the first character of the logical line the position is
   on, where logical lines are split by newline characters. */
ITL_DEF size_t itl_le_line_start_of(const itl_le_t *le, size_t position)
{
  size_t p = position;
  while (p > 0 && !ITL_LE_IS_NEWLINE(le->line->chars[p - 1])) {
    p -= 1;
  }
  return p;
}

/* Returns the index one past the last character of the position's logical line.
 */
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

ITL_DEF size_t itl_le_tty_break_row(itl_char_buf_t *b, bool is_span_open,
                                    bool should_suppress_pad,
                                    const char *open_sgr, size_t indent,
                                    size_t *row)
{
  if (is_span_open && should_suppress_pad) {
    itl_char_buf_append_cstr(b, ITL_HIGHLIGHT_RESET);
  }

  itl_char_buf_append_cstr(b, ITL_LF);
  itl_char_buf_append_spaces(b, indent);

  if (is_span_open && should_suppress_pad) {
    itl_char_buf_append_cstr(b, open_sgr);
  }

  *row += 1;

  return indent;
}

/* Draw the ghost suggestion dimmed after the line. It is shown only when the
   cursor sits at the very end of the buffer and the suggestion fits on the
   current row without wrapping, so it never pushes a line break and the caller's
   cursor restore lands on the real caret. A second clear erases a longer ghost
   left from a previous frame. */
ITL_DEF bool itl_le_tty_draw_ghost(itl_char_buf_t *b, bool is_cursor_at_end,
                                   size_t cursor_col, size_t cols)
{
  itl_g_le_prev_ghost_len = 0;

  if (!is_cursor_at_end || itl_g_ghost_len == 0 ||
      cursor_col + itl_g_ghost_width >= cols)
  {
    return false;
  }

  itl_char_buf_append_cstr(b, itl_color_sequence(ITL_DIM_SGR));
  itl_char_buf_append_visible(b, itl_g_ghost, itl_g_ghost_len,
                              itl_g_ghost_width);
  itl_char_buf_append_cstr(b, itl_color_sequence(ITL_HIGHLIGHT_RESET));
  ITL_TTY_CLEAR_TO_END(b);
  itl_g_le_prev_ghost_len = itl_g_ghost_len;

  return true;
}

/* Append text to the hint being built in out, cut to fit out_size. */
ITL_DEF void itl_prefix_append(char *out, size_t out_size, size_t *length,
                               const char *text)
{
  size_t text_length = strlen(text);

  if (*length + text_length >= out_size) {
    text_length = out_size - 1 - *length;
  }
  memcpy(out + *length, text, text_length);
  *length += text_length;
  out[*length] = '\0';
}

/* Write the hint of the open prefix into out, or return false when no key
   waits. A vi count, register, or operator held between keys in normal mode
   counts as a prefix too. The keys typed so far are the header, and the keys
   that complete them are the body. */
ITL_DEF bool itl_prefix_hint(char *out, size_t out_size)
{
  char keys[48];
  char digits[24];
  size_t keys_length = 0;
  size_t out_length = 0;
  const char *waiting_for;
  bool is_vi_held = itl_g_edit_mode == TL_EDIT_MODE_VI_COMMAND &&
                    (itl_g_vi_pending_operator != ITL_VI_OP_NONE ||
                     itl_g_vi_pending_count > 0 ||
                     itl_g_vi_pending_register != 0);

  out[0] = '\0';
  if (itl_g_prefix_kind == ITL_PREFIX_CTRL_X) {
    itl_prefix_append(out, out_size, &out_length,
                      "pressed ctrl-x\nwaiting for ctrl-e (edit in $VISUAL), "
                      "ctrl-u (undo)");
    return true;
  }
  if (itl_g_prefix_kind == ITL_PREFIX_VI_EX) {
    itl_prefix_append(out, out_size, &out_length,
                      "pressed :\nwaiting for q, q!, quit, wq, wq!, or x "
                      "(quit), then enter");
    return true;
  }
  if (itl_g_prefix_kind == ITL_PREFIX_NONE && !is_vi_held) {
    return false;
  }

  if (is_vi_held && itl_g_vi_pending_register != 0) {
    keys[keys_length++] = '"';
    keys[keys_length++] = itl_g_vi_pending_register;
  }
  if (is_vi_held && itl_g_vi_pending_count > 0) {
    size_t count = itl_g_vi_pending_count;
    size_t digit_count = 0;

    while (count > 0 && digit_count < sizeof(digits)) {
      digits[digit_count++] = (char) ('0' + (count % 10));
      count /= 10;
    }
    while (digit_count > 0) {
      keys[keys_length++] = digits[--digit_count];
    }
  }
  if (is_vi_held && itl_g_vi_pending_operator != ITL_VI_OP_NONE) {
    keys[keys_length++] = (itl_g_vi_pending_operator == ITL_VI_OP_DELETE) ? 'd'
                          : (itl_g_vi_pending_operator == ITL_VI_OP_CHANGE)
                              ? 'c'
                              : 'y';
  }

  switch (itl_g_prefix_kind) {
  case ITL_PREFIX_VI_REGISTER:
    keys[keys_length++] = '"';
    waiting_for = "a register name: a-z";
    break;
  case ITL_PREFIX_VI_FIND:
    keys[keys_length++] = (char) itl_g_prefix_key;
    waiting_for = (itl_g_prefix_key == 'f')   ? "a character to find"
                  : (itl_g_prefix_key == 'F') ? "a character to find backward"
                  : (itl_g_prefix_key == 't') ? "a character to stop before"
                                              : "a character to stop after";
    break;
  case ITL_PREFIX_VI_REPLACE:
    keys[keys_length++] = 'r';
    waiting_for = "a replacement character";
    break;
  default:
    if (itl_g_vi_pending_operator == ITL_VI_OP_DELETE) {
      waiting_for = "a motion: w, b, e, $, 0, ^, d (line), f, t, F, T, h, l, "
                    "j, k, W, B, E, ; or , (repeat a find)";
    } else if (itl_g_vi_pending_operator == ITL_VI_OP_CHANGE) {
      waiting_for = "a motion: w, b, e, $, 0, ^, c (line), f, t, F, T, h, l, "
                    "j, k, W, B, E, ; or , (repeat a find)";
    } else if (itl_g_vi_pending_operator == ITL_VI_OP_YANK) {
      waiting_for = "a motion: w, b, e, $, 0, ^, y (line), f, t, F, T, h, l, "
                    "j, k, W, B, E, ; or , (repeat a find)";
    } else if (itl_g_vi_pending_count > 0) {
      waiting_for = "a command or a motion";
    } else {
      waiting_for = "a command: d, c, y, p, P, x, X, D, C, s, S";
    }
    break;
  }

  keys[keys_length] = '\0';
  itl_prefix_append(out, out_size, &out_length, "pressed ");
  itl_prefix_append(out, out_size, &out_length, keys);
  itl_prefix_append(out, out_size, &out_length, "\nwaiting for ");
  itl_prefix_append(out, out_size, &out_length, waiting_for);

  return true;
}

/* The rows free under an input block of block_rows rows on a terminal of
   tty_rows rows. The whole block is held back, as the menu holds it back,
   since the terminal scrolls the prompt away once the rows below it are
   overrun. */
ITL_DEF size_t itl_hint_row_budget(size_t tty_rows, size_t block_rows)
{
  return tty_rows > block_rows ? tty_rows - block_rows : 0;
}

/* The byte length of the longest start of text whose display width fits
   limit. A wide character that would cross the limit is left out. */
ITL_DEF size_t itl_hint_fit(const char *text, size_t length, size_t limit)
{
  size_t offset = 0;

  if (itl_strn_width_walk(text, length, limit, &offset) > limit && limit > 0) {
    itl_strn_width_walk(text, length, limit - 1, &offset);
  }

  return offset;
}

/* Append one row to the next layout: the style, the indent, length bytes of
   text, an ellipsis when is_cut, and the reset, after a line feed when a row
   came before it. */
ITL_DEF void itl_hint_append_row(const char *text, size_t length, bool is_cut)
{
  const char *open = itl_color_sequence(itl_g_hint_sgr);
  const char *reset = itl_color_sequence(ITL_HIGHLIGHT_RESET);
  size_t open_len = strlen(open);
  size_t reset_len = strlen(reset);
  char *out = itl_g_hint_next + itl_g_hint_next_len;

  while (length > 0 && text[length - 1] == ' ') {
    length -= 1;
  }

  if (itl_g_hint_next_rows > 0) {
    *out++ = '\n';
  }
  memcpy(out, open, open_len);
  out += open_len;
  memcpy(out, ITL_HINT_INDENT, ITL_HINT_INDENT_WIDTH);
  out += ITL_HINT_INDENT_WIDTH;
  memcpy(out, text, length);
  out += length;
  if (is_cut) {
    memcpy(out, ITL_PROMPT_ELLIPSIS, ITL_PROMPT_ELLIPSIS_WIDTH);
    out += ITL_PROMPT_ELLIPSIS_WIDTH;
  }
  memcpy(out, reset, reset_len);
  out += reset_len;
  *out = '\0';

  itl_g_hint_next_len = (size_t) (out - itl_g_hint_next);
  itl_g_hint_next_rows += 1;
}

/* Append text as one row of at most room columns, cut with an ellipsis when
   it is wider. */
ITL_DEF void itl_hint_append_cut_row(const char *text, size_t length,
                                     size_t room)
{
  if (itl_strn_width_walk(text, length, (size_t) -1, NULL) <= room) {
    itl_hint_append_row(text, length, false);
    return;
  }

  itl_hint_append_row(
      text, itl_hint_fit(text, length, room - ITL_PROMPT_ELLIPSIS_WIDTH), true);
}

/* Lay the hint source out for a terminal of cols columns with row_budget rows
   free under the input. Every row keeps the last column free. The header takes
   its own row while a body row still fits under it, and otherwise the body is
   shown alone. The body wraps at spaces, a word wider than a row is split, and
   the last row it may take is cut with an ellipsis when text remains. */
ITL_DEF void itl_hint_layout(size_t cols, size_t row_budget)
{
  size_t room = cols > 1 ? cols - 1 : 0;
  const char *body = itl_g_hint_source + itl_g_hint_header_len;
  size_t body_len = itl_g_hint_source_len - itl_g_hint_header_len;
  size_t text_room, body_rows;

  itl_g_hint_layout_cols = cols;
  itl_g_hint_layout_rows = row_budget;
  itl_g_hint_next_len = 0;
  itl_g_hint_next_rows = 0;
  itl_g_hint_next[0] = '\0';

  if (itl_g_hint_source_len == 0 || row_budget == 0 ||
      room <= ITL_HINT_INDENT_WIDTH + ITL_PROMPT_ELLIPSIS_WIDTH + 1)
  {
    return;
  }

  text_room = room - ITL_HINT_INDENT_WIDTH;
  if (row_budget > ITL_HINT_ROWS_MAX) {
    row_budget = ITL_HINT_ROWS_MAX;
  }

  while (body_len > 0 && body[0] == ' ') {
    body += 1;
    body_len -= 1;
  }

  if (itl_g_hint_header_len > 0 && (body_len == 0 || row_budget >= 2)) {
    itl_hint_append_cut_row(itl_g_hint_source, itl_g_hint_header_len,
                            text_room);
    row_budget -= 1;
  }
  body_rows = ITL_MIN(row_budget, ITL_HINT_ROWS_MAX - 1);

  while (body_len > 0 && body_rows > 0) {
    size_t fit, end;

    if (body_rows == 1) {
      itl_hint_append_cut_row(body, body_len, text_room);
      break;
    }

    fit = itl_hint_fit(body, body_len, text_room);
    if (fit >= body_len) {
      itl_hint_append_row(body, body_len, false);
      break;
    }
    if (fit == 0) {
      break;
    }

    end = fit;
    if (body[fit] != ' ') {
      while (end > 0 && body[end - 1] != ' ') {
        end -= 1;
      }
      if (end == 0) {
        end = fit;
      }
    }

    itl_hint_append_row(body, end, false);
    body += end;
    body_len -= end;
    while (body_len > 0 && body[0] == ' ') {
      body += 1;
      body_len -= 1;
    }
    body_rows -= 1;
  }
}

/* Drop the hint the next frame wants, so it draws none. */
ITL_DEF void itl_hint_drop_next(void)
{
  itl_g_hint_source_len = 0;
  itl_g_hint_header_len = 0;
  itl_g_hint_next_len = 0;
  itl_g_hint_next_rows = 0;
  itl_g_hint_next[0] = '\0';
}

/* Ask the host for the hint of this line and lay it out for the terminal. An
   open prefix takes the rows before the host is asked, and only while the
   rows are enabled, held by nothing, and not closed. The first line break ends
   the header, and control bytes, later line breaks among them, become spaces.
   The result lands in itl_g_hint_next, empty for no hint. */
ITL_DEF void itl_hint_compose(const char *line, size_t cursor_byte, size_t cols,
                              size_t row_budget)
{
  char prefix_text[160];
  const char *sgr = NULL;
  const char *returned = NULL;
  const char *line_break;
  size_t text_bytes = 0, kept_bytes = 0, header_end, i;

  itl_g_hint_source_len = 0;
  itl_g_hint_header_len = 0;
  itl_g_hint_source[0] = '\0';

  if (itl_g_hint_callback != NULL && itl_g_hint_hold_count == 0 &&
      !itl_g_hint_is_closed)
  {
    if (itl_prefix_hint(prefix_text, sizeof(prefix_text))) {
      returned = prefix_text;
    } else {
      returned = itl_g_hint_callback(line, cursor_byte, &sgr);
    }
  }

  if (returned != NULL) {
    while (text_bytes < ITL_HINT_TEXT_MAX && returned[text_bytes] != '\0') {
      text_bytes += 1;
    }
  }

  line_break = text_bytes > 0
                   ? (const char *) memchr(returned, '\n', text_bytes)
                   : NULL;
  header_end = line_break != NULL ? (size_t) (line_break - returned) : 0;
  if (header_end > 0 && returned[header_end - 1] == '\r') {
    header_end -= 1;
  }

  i = 0;
  while (i < text_bytes) {
    uint8_t byte = (uint8_t) returned[i];
    size_t step_bytes, step_width;
    bool is_notation;

    if (line_break != NULL && i == header_end) {
      itl_g_hint_header_len = kept_bytes;
      i = (size_t) (line_break - returned) + 1;
      continue;
    }

    itl_visible_step(returned + i, text_bytes - i, &step_bytes, &step_width);
    is_notation = step_bytes == 1 ? byte < 0x20 || byte >= 0x7F
                                  : byte == 0xC2 &&
                                        (uint8_t) returned[i + 1] < 0xA0;
    if (is_notation) {
      itl_g_hint_source[kept_bytes++] = ' ';
    } else {
      memcpy(itl_g_hint_source + kept_bytes, returned + i, step_bytes);
      kept_bytes += step_bytes;
    }
    i += step_bytes;
  }
  itl_g_hint_source_len = kept_bytes;
  itl_g_hint_source[kept_bytes] = '\0';

  if (sgr == NULL || strlen(sgr) >= ITL_HINT_SGR_MAX) {
    sgr = ITL_DIM_SGR;
  }
  memcpy(itl_g_hint_sgr, sgr, strlen(sgr) + 1);

  itl_hint_layout(cols, row_budget);
}

/* The caret's byte offset into the serialized line. */
ITL_DEF size_t itl_le_cursor_byte_offset(const itl_le_t *le)
{
  size_t stop = ITL_MIN(le->cursor_position, le->line->length);
  size_t offset = 0;
  size_t i;

  for (i = 0; i < stop; ++i) {
    offset += le->line->chars[i].size;
  }

  return offset;
}

ITL_DEF bool itl_hint_is_unchanged(void)
{
  return itl_g_hint_next_len == itl_g_hint_shown_len &&
         memcmp(itl_g_hint_next, itl_g_hint_shown, itl_g_hint_next_len) == 0;
}

/* Bring the rows under the last input row to the composed ones, laid out again
   first when the block now leaves a different number of rows free. The caret
   must sit on that last row. A line feed enters each row, which scrolls the
   screen when the block ends at the bottom, rows on screen past the new last
   row are erased, and steps up return to the caret's row, with the column left
   for the caller to restore. */
ITL_DEF void itl_le_tty_draw_hint(itl_char_buf_t *b, size_t row_budget)
{
  size_t row_count, row, offset = 0;

  if (row_budget != itl_g_hint_layout_rows) {
    itl_hint_layout(itl_g_hint_layout_cols, row_budget);
  }
  if (itl_hint_is_unchanged()) {
    return;
  }

  row_count = ITL_MAX(itl_g_hint_next_rows, itl_g_hint_shown_rows);
  for (row = 0; row < row_count; ++row) {
    itl_char_buf_append_cstr(b, ITL_LF);
    ITL_TTY_MOVE_TO_COLUMN(b, 1);

    if (row < itl_g_hint_next_rows) {
      const char *start = itl_g_hint_next + offset;
      const char *end = (const char *) memchr(start, '\n',
                                              itl_g_hint_next_len - offset);
      size_t length = end != NULL ? (size_t) (end - start)
                                  : itl_g_hint_next_len - offset;

      itl_char_buf_append_bytes(b, start, length);
      ITL_TTY_CLEAR_TO_END(b);
      offset += length + 1;
    } else {
      ITL_TTY_CLEAR_WHOLE_LINE(b);
    }
  }

  if (row_count > 0) {
    ITL_TTY_MOVE_UP(b, row_count);
  }

  memcpy(itl_g_hint_shown, itl_g_hint_next, itl_g_hint_next_len);
  itl_g_hint_shown_len = itl_g_hint_next_len;
  itl_g_hint_shown_rows = itl_g_hint_next_rows;
}

/* Erase the hint rows from the first of them, with the cursor in its first
   column, and come back to that row. */
ITL_DEF void itl_le_tty_erase_hint_rows(itl_char_buf_t *b)
{
  size_t row;

  for (row = 0; row < itl_g_hint_shown_rows; ++row) {
    if (row > 0) {
      ITL_TTY_MOVE_DOWN(b, 1);
    }
    ITL_TTY_CLEAR_WHOLE_LINE(b);
  }
  if (itl_g_hint_shown_rows > 1) {
    ITL_TTY_MOVE_UP(b, itl_g_hint_shown_rows - 1);
  }

  itl_hint_forget_shown();
}

/* The column the right prompt starts at when the first input row ends at
   row_end_col, or zero when it is not drawn. It ends one column short of the
   right edge and keeps one free column after the row. A prompt cut to fit
   leaves no room for it. */
ITL_DEF size_t itl_right_prompt_column(const itl_le_t *le, size_t row_end_col,
                                       size_t cols)
{
  if (itl_g_right_prompt_width == 0 || itl_g_right_prompt_is_held ||
      le->prompt_width >= cols ||
      row_end_col + itl_g_right_prompt_width + 2 > cols)
  {
    return 0;
  }

  return cols - 1 - itl_g_right_prompt_width;
}

/* Draw the right prompt on the current row from start_col. The caller moves
   the caret back afterwards. */
ITL_DEF void itl_le_tty_draw_right_prompt(itl_char_buf_t *b, size_t start_col)
{
  ITL_TTY_MOVE_TO_COLUMN(b, start_col + 1);
  itl_char_buf_append_cstr(b, itl_g_right_prompt);
  itl_char_buf_append_cstr(b, itl_color_sequence(ITL_HIGHLIGHT_RESET));
}

/* NOTE: Hottest function in the library. */
ITL_DEF bool itl_le_tty_refresh(itl_le_t *le)
{
  size_t i, tty_rows, tty_cols, cols, indent;
  size_t col, row, move_up;
  itl_le_metrics_t m = ITL_ZERO_INIT;
  bool is_metrics_ready = false;
  bool has_resize;
#if defined ITL_POSIX
  sigset_t previous_signals;

  if (!itl_block_input_wake_signals(&previous_signals)) {
    return false;
  }
  has_resize = itl_g_tty_changed_size != 0;
  itl_g_tty_changed_size = 0;
  if (!itl_restore_input_wake_signals(&previous_signals)) {
    if (has_resize) {
      itl_g_tty_changed_size = 1;
    }
    return false;
  }
#else
  has_resize = itl_g_tty_changed_size != 0;
  itl_g_tty_changed_size = 0;
#endif
  /* A genuine resize reflowed the previous render, so the stored row counts are
     stale and the clear below cannot trust them. The first render has no
     previous block, so it is never treated as a resize. */
  bool is_resize = has_resize && !itl_g_tty_first_render;

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

  if (has_resize) {
    ITL_TRY(itl_tty_get_size(&tty_rows, &tty_cols), {
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

  tl_highlight_span itl_spans[ITL_HIGHLIGHT_MAX_SPANS];
  size_t span_count = 0;

  /* A caret move can change the hint and a caret-dependent highlight, so the
     cursor-only frame asks for them here. A hint or spans that differ from
     the ones on screen turn the frame into a text refresh, which redraws the
     line and the row. */
  bool is_hint_ready = false;
  bool is_highlight_ready = false;
  bool should_follow_highlight =
      !itl_g_tty_should_refresh_text && itl_g_highlight_follows_cursor &&
      !itl_g_search_spans_active && itl_should_run_highlight();
  if (!itl_g_tty_should_refresh_text &&
      (itl_g_hint_callback != NULL || should_follow_highlight))
  {
    bool was_serialized = itl_g_serialized_line_ready;
    bool is_serialized = itl_le_serialize_line(le);
    size_t cursor_offset = itl_le_cursor_byte_offset(le);

    if (itl_g_hint_callback != NULL) {
      if (is_serialized) {
        itl_hint_compose(
            itl_g_serialized_line, cursor_offset, cols,
            itl_hint_row_budget(tty_rows, itl_g_le_prev_total_rows));
      } else {
        itl_hint_drop_next();
      }
      is_hint_ready = true;
    }

    if (should_follow_highlight && is_serialized) {
      span_count = itl_le_collect_highlight(
          itl_g_serialized_line, itl_spans, le->line->length, cursor_offset);
      is_highlight_ready = true;

      if (!itl_le_prev_spans_equal(itl_spans, span_count)) {
        itl_g_tty_should_refresh_text = true;
      }
    }
    itl_g_serialized_line_ready = was_serialized;

    if (is_hint_ready && !itl_hint_is_unchanged()) {
      itl_g_tty_should_refresh_text = true;
    }
  }

  if (itl_g_tty_plain_append_pending && !is_resize &&
      !itl_g_tty_first_render && itl_g_le_prev_cursor_at_end &&
      itl_g_le_prev_cursor_col + itl_g_tty_plain_append_width < cols)
  {
    m.total_rows = itl_g_le_prev_total_rows;
    m.cursor_row = itl_g_le_prev_cursor_row - 1;
    m.cursor_col =
        itl_g_le_prev_cursor_col + itl_g_tty_plain_append_width;
    is_metrics_ready = true;
  } else if (!itl_g_tty_should_refresh_text ||
             (!is_resize && !itl_g_tty_first_render &&
              itl_g_le_prev_cursor_at_end &&
              le->cursor_position == le->line->length))
  {
#if !defined NDEBUG
    itl_g_debug_metrics_scan_count += 1;
#endif
    m = itl_le_compute_metrics(le, tty_cols);
    is_metrics_ready = true;
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
  const char *itl_cur_render = itl_g_serialized_line;
  bool have_cur_render = false;
  tl_highlight_span itl_syntax_spans[ITL_HIGHLIGHT_MAX_SPANS];
  const char *append_tail_sgr = NULL;
  if (itl_g_tty_should_refresh_text) {
    have_cur_render = itl_le_serialize_line(le);
    itl_g_serialized_line_ready = false;
    if (itl_g_search_spans_active &&
        (itl_g_edit_mode == TL_EDIT_MODE_VI_VISUAL ||
         itl_g_multicursor_active) &&
        have_cur_render && itl_should_run_highlight())
    {
      size_t syntax_count = itl_le_collect_highlight(
          itl_cur_render, itl_syntax_spans, le->line->length,
          itl_le_cursor_byte_offset(le));

      span_count = itl_merge_visual_spans(
          itl_syntax_spans, syntax_count, itl_g_search_spans,
          itl_g_search_span_count, le->line->length, itl_spans,
          ITL_HIGHLIGHT_MAX_SPANS);
    } else if (itl_g_search_spans_active) {
      /* The reverse search prebuilt its spans for the whole block, so the host
         callback is skipped and those spans are validated and drawn. */
      span_count = itl_spans_keep_valid(itl_g_search_spans,
                                        itl_g_search_span_count, itl_spans,
                                        le->line->length);
    } else if (have_cur_render && !is_highlight_ready &&
               itl_should_run_highlight())
    {
      span_count = itl_le_collect_highlight(itl_cur_render, itl_spans,
                                            le->line->length,
                                            itl_le_cursor_byte_offset(le));
    }
  }

  if (itl_g_tty_should_refresh_text && !is_hint_ready) {
    if (have_cur_render) {
      itl_hint_compose(
          itl_cur_render, itl_le_cursor_byte_offset(le), cols,
          itl_hint_row_budget(tty_rows, itl_g_le_prev_total_rows));
    } else {
      itl_hint_drop_next();
    }
  }

  bool spans_are_append_compatible = itl_le_prev_spans_append_compatible(
      itl_spans, span_count, le->line->length, &append_tail_sgr);
  if (is_metrics_ready && itl_g_tty_should_refresh_text && !is_resize &&
      !itl_g_tty_first_render && have_cur_render &&
      m.total_rows == itl_g_le_prev_total_rows &&
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
      size_t tail_index;
      if (append_tail_sgr != NULL) {
        itl_char_buf_append_cstr(fb, append_tail_sgr);
      }
      for (tail_index = itl_g_le_prev_length; tail_index < le->line->length;
           ++tail_index)
      {
        itl_char_buf_append_line_char(fb, le->line->chars[tail_index]);
      }
      if (append_tail_sgr != NULL) {
        itl_char_buf_append_cstr(fb, ITL_HIGHLIGHT_RESET);
      }
      ITL_TTY_CLEAR_TO_END(fb);
      bool was_ghost_drawn = itl_le_tty_draw_ghost(fb, true, m.cursor_col, cols);
      bool should_restore_column = was_ghost_drawn;
      /* The clear above took a right prompt on the caret's row with it, so it is
         drawn again while the row still leaves room for it. */
      if (m.cursor_row == le->prompt_rows) {
        size_t row_end_col =
            m.cursor_col + (was_ghost_drawn ? itl_g_ghost_width : 0);
        size_t right_prompt_col =
            itl_right_prompt_column(le, row_end_col, cols);

        if (right_prompt_col > 0) {
          itl_le_tty_draw_right_prompt(fb, right_prompt_col);
          should_restore_column = true;
        }
        itl_g_le_prev_right_prompt_is_shown = right_prompt_col > 0;
      }
      if (should_restore_column) {
        ITL_TTY_MOVE_TO_COLUMN(fb, m.cursor_col + 1);
      }
      if (!itl_hint_is_unchanged()) {
        ITL_TTY_HIDE_CURSOR(fb);
        itl_le_tty_draw_hint(fb, itl_hint_row_budget(tty_rows, m.total_rows));
        ITL_TTY_MOVE_TO_COLUMN(fb, m.cursor_col + 1);
        ITL_TTY_SHOW_CURSOR(fb);
      }
      itl_le_commit_geometry(m, true);
      itl_le_commit_render(itl_cur_render, cur_len, le->line->length, itl_spans,
                           span_count);
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
      /* The terminal reflowed the previous render and the stored row counts are
         stale. The cursor sits on the caret's reflowed row. Step up by the
         reflowed rows above it to reach the block top, then clear everything
         below. Nothing under the block survives a resize, and an open menu
         draws its own rows again once the block is back. */
      size_t rows_above =
          itl_le_reflow_rows_above_caret(le, itl_g_tty_prev_cols, tty_cols);
      if (rows_above > 0) {
        ITL_TTY_MOVE_UP(b, rows_above);
      }
      ITL_TTY_MOVE_TO_COLUMN(b, 1);
      ITL_TTY_CLEAR_BELOW(b);
    } else {
      /* Park at the top-left of the previous render. */
      itl_le_tty_move_to_block_top(b);

      /* Clear every row the previous render occupied, and the hint rows under
         it, leaving rows we do not own untouched. */
      size_t clear_rows = itl_g_le_prev_total_rows + itl_g_hint_shown_rows;
      for (i = 0; i < clear_rows; ++i) {
        ITL_TTY_CLEAR_WHOLE_LINE(b);
        if (i + 1 < clear_rows) {
          ITL_TTY_MOVE_DOWN(b, 1);
        }
      }
      if (clear_rows > 1) {
        ITL_TTY_MOVE_UP(b, clear_rows - 1);
      }
      ITL_TTY_MOVE_TO_COLUMN(b, 1);
    }
    itl_hint_forget_shown();

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
       by the continuation indent. The span cursor closes a span
       that ends at this codepoint, then opens the one that starts here. */
    size_t next_span = 0;
    bool in_span = false;
    size_t open_end = 0;
    const char *open_sgr = NULL;
    bool suppress_pad = itl_g_edit_mode == TL_EDIT_MODE_VI_VISUAL;
    /* Where the first input row ends. A row that wrapped fills the width. */
    size_t first_row_end_col = cols;
    col = itl_le_prompt_indent(le, cols);
    row = le->prompt_rows;
    /* The flash repaints the whole line in one tone. It opens that SGR once and
       the loop below skips the per-span color. */
    if (itl_g_tty_flash_active) {
      itl_char_buf_append_cstr(b, itl_flash_sgr_on());
    }
    for (i = 0; i < le->line->length; ++i) {
      itl_utf8_t ch = le->line->chars[i];

      if (!is_metrics_ready && i == le->cursor_position) {
        m.cursor_row = row;
        m.cursor_col = col;
      }

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
        }

        if (row == le->prompt_rows) {
          first_row_end_col = col;
        }
        col = itl_le_tty_break_row(b, in_span, suppress_pad, open_sgr, indent,
                                   &row);

        continue;
      }

      {
        size_t char_width = itl_line_char_width(le->line->chars, i);

        if (itl_wrap_is_early_break(col, char_width, cols)) {
          col = itl_le_tty_break_row(b, in_span, suppress_pad, open_sgr, indent,
                                     &row);
        }

        itl_char_buf_append_line_char(b, ch);
        col += char_width;

        if (itl_wrap_is_break_after(col, cols)) {
          col = itl_le_tty_break_row(b, in_span, suppress_pad, open_sgr, indent,
                                     &row);
        }
      }
    }

    if (!is_metrics_ready) {
      if (le->cursor_position == le->line->length) {
        m.cursor_row = row;
        m.cursor_col = col;
      }

      m.total_rows = row + 1;
      is_metrics_ready = true;
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

    bool was_ghost_drawn = itl_le_tty_draw_ghost(
        b, le->cursor_position == le->line->length, col, cols);
    itl_le_tty_draw_hint(b, itl_hint_row_budget(tty_rows, m.total_rows));

    if (row == le->prompt_rows) {
      first_row_end_col = col + (was_ghost_drawn ? itl_g_ghost_width : 0);
    }
    size_t right_prompt_col =
        itl_right_prompt_column(le, first_row_end_col, cols);
    itl_g_le_prev_right_prompt_is_shown = right_prompt_col > 0;

    if (right_prompt_col > 0) {
      /* The right prompt goes on the first input row, and the caret steps down
         from there to its own row. */
      if (m.total_rows - 1 > le->prompt_rows) {
        ITL_TTY_MOVE_UP(b, m.total_rows - 1 - le->prompt_rows);
      }
      itl_le_tty_draw_right_prompt(b, right_prompt_col);
      if (m.cursor_row > le->prompt_rows) {
        ITL_TTY_MOVE_DOWN(b, m.cursor_row - le->prompt_rows);
      }
    } else {
      /* Move from the end of the rendered text up to the cursor's row. */
      move_up = (m.total_rows - 1) - m.cursor_row;
      if (move_up > 0) {
        ITL_TTY_MOVE_UP(b, move_up);
      }
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

  itl_le_commit_geometry(m, le->cursor_position == le->line->length);

  /* A cursor-only refresh leaves the line untouched and keeps the stored
     render. A failed conversion forces a full redraw next time. */
  if (itl_g_tty_should_refresh_text) {
    if (have_cur_render) {
      /* A successful itl_string_to_cstr wrote exactly the line's byte size, so
         the length is read off the line rather than recounted with strlen. */
      itl_le_commit_render(itl_cur_render, le->line->size, le->line->length,
                           itl_spans, span_count);
    } else {
      itl_le_commit_render(itl_cur_render, 0, 0, itl_spans, span_count);
    }
  }

  itl_g_tty_prev_rows = tty_rows;
  itl_g_tty_prev_cols = tty_cols;
  itl_g_tty_first_render = false;
  itl_g_tty_plain_append_pending = false;

  ITL_TTY_AUTOWRAP_ON(b);
  ITL_TTY_SHOW_CURSOR(b);

  ITL_CHAR_BUF_DUMP(b);
  ITL_CHAR_BUF_CLEAR(b);

  return true;
}

ITL_DEF ITL_THREAD_LOCAL itl_le_t itl_g_le = ITL_ZERO_INIT;

#if defined ITL_POSIX
ITL_DEF ITL_THREAD_LOCAL struct sigaction itl_g_prev_sigwinch = ITL_ZERO_INIT;
ITL_DEF ITL_THREAD_LOCAL bool itl_g_has_sigwinch_handler = false;

ITL_DEF void itl_handle_sigwinch(int signal_number)
{
  if (signal_number != SIGWINCH) {
    return;
  }
  /* Only set the flag here. The main loop does the redraw in normal context. */
  itl_g_tty_changed_size = 1;
}

ITL_DEF bool itl_install_sigwinch_handler(void)
{
  struct sigaction action = ITL_ZERO_INIT;

  action.sa_handler = itl_handle_sigwinch;
  if (sigemptyset(&action.sa_mask) != 0 ||
      sigaction(SIGWINCH, &action, &itl_g_prev_sigwinch) != 0)
  {
    return false;
  }

  itl_g_has_sigwinch_handler = true;
  return true;
}

ITL_DEF bool itl_restore_sigwinch_handler(void)
{
  if (!itl_g_has_sigwinch_handler) {
    return true;
  }
  if (sigaction(SIGWINCH, &itl_g_prev_sigwinch, NULL) != 0) {
    return false;
  }

  itl_g_has_sigwinch_handler = false;
  return true;
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

/* The host history selector callback, or NULL when ctrl-R stays inside the
   editor. Only a host that drives a program of its own registers one. */
ITL_DEF ITL_THREAD_LOCAL tl_history_select_fn itl_g_history_select_callback =
    NULL;

TL_DEF void tl_set_history_select_callback(tl_history_select_fn callback)
{
  itl_g_history_select_callback = callback;
}

/* The host edit callback for Ctrl-X Ctrl-E, or NULL when the key does
   nothing. */
ITL_DEF ITL_THREAD_LOCAL tl_edit_fn itl_g_edit_callback = NULL;

TL_DEF void tl_set_edit_callback(tl_edit_fn callback)
{
  itl_g_edit_callback = callback;
}

TL_DEF void tl_set_history_search_snapshot_callback(
    tl_history_search_snapshot_fn callback)
{
  itl_g_history_search_snapshot_callback = callback;
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

TL_DEF void tl_set_history_prefix_search(int enabled)
{
  itl_g_history_prefix_search_enabled = enabled;
}

/* Whether a typed opener inserts its closer, and how many closers the editor
   inserted that still sit right after the caret. A typed character lands
   before them and a backspace erases before them, so both keep the count.
   Any other key forgets it. */
ITL_DEF ITL_THREAD_LOCAL bool itl_g_auto_pair_enabled = false;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_auto_pair_count = 0;
ITL_DEF ITL_THREAD_LOCAL tl_pair_role_fn itl_g_pair_role_callback = NULL;

TL_DEF void tl_set_auto_pair(int enabled)
{
  itl_g_auto_pair_enabled = enabled != 0;
  itl_g_auto_pair_count = 0;
}

TL_DEF void tl_set_pair_role_callback(tl_pair_role_fn callback)
{
  itl_g_pair_role_callback = callback;
}

/* What the typed byte does at the caret, or the given fallback when no host
   callback answers. */
ITL_DEF int itl_le_pair_role(const itl_le_t *le, uint8_t byte, int fallback)
{
  if (itl_g_pair_role_callback == NULL ||
      itl_string_to_cstr(le->line, itl_g_serialized_line,
                         sizeof(itl_g_serialized_line)) != TL_SUCCESS)
  {
    return fallback;
  }

  itl_g_serialized_line_ready = false;
  return itl_g_pair_role_callback(itl_g_serialized_line,
                                  itl_le_cursor_byte_offset(le), byte);
}

/* The closer an opener pairs with, or zero for any other byte. */
ITL_DEF uint8_t itl_auto_pair_closer(uint8_t opener)
{
  switch (opener) {
  case '(': return ')';
  case '[': return ']';
  case '{': return '}';
  case '"': return '"';
  case '\'': return '\'';
  default: return 0;
  }
}

/* The byte of a single-byte character at position, or zero past the line
   end and for a multibyte character. */
ITL_DEF uint8_t itl_le_ascii_at(const itl_le_t *le, size_t position)
{
  if (position >= le->line->length || le->line->chars[position].size != 1) {
    return 0;
  }
  return le->line->chars[position].bytes[0];
}

/* An opener is paired only before the line end, a blank, a closing bracket, a
   quote, or a closer the editor inserted, and never after a backslash. So an
   opener typed inside a pair the editor made, such as $( inside a closed
   double quote, pairs too. A quote after a word character, or next to the same
   quote, more likely closes a string or sits inside a word, so it stays single
   there. */
ITL_DEF bool itl_le_auto_pair_fits(const itl_le_t *le, uint8_t opener)
{
  size_t caret = le->cursor_position;
  uint8_t next = itl_le_ascii_at(le, caret);
  uint8_t previous = caret > 0 ? itl_le_ascii_at(le, caret - 1) : ' ';
  bool is_previous_wide = caret > 0 && le->line->chars[caret - 1].size != 1;

  if (caret < le->line->length && itl_g_auto_pair_count == 0 && next != ' ' &&
      next != '\t' && next != '\n' && next != ')' && next != ']' &&
      next != '}' && next != '"' && next != '\'')
  {
    return false;
  }
  if (previous == '\\') {
    return false;
  }
  if (opener == '"' || opener == '\'') {
    return !is_previous_wide && !isalnum(previous) && previous != opener &&
           next != opener;
  }
  return true;
}

/* Steps over an inserted closer or inserts an opener with its closer, and
   returns whether the typed byte was handled. */
ITL_DEF bool itl_le_auto_pair_type(itl_le_t *le, uint8_t byte)
{
  uint8_t closer;

  if (!itl_g_auto_pair_enabled || itl_g_vi_block_insert_active) {
    return false;
  }

  if (itl_g_auto_pair_count > 0 &&
      itl_le_ascii_at(le, le->cursor_position) == byte &&
      (byte == ')' || byte == ']' || byte == '}' || byte == '"' ||
       byte == '\'') &&
      itl_le_pair_role(le, byte, TL_PAIR_CLOSES) == TL_PAIR_CLOSES)
  {
    itl_le_move_right(le, 1);
    itl_g_auto_pair_count -= 1;
    return true;
  }

  closer = itl_auto_pair_closer(byte);
  if (closer == 0 || !itl_le_auto_pair_fits(le, byte) ||
      itl_le_pair_role(le, byte, TL_PAIR_OPENS) != TL_PAIR_OPENS)
  {
    return false;
  }
  if (!itl_le_insert(le, itl_utf8_parse(byte))) {
    return false;
  }
  if (itl_le_insert(le, itl_utf8_parse(closer))) {
    itl_le_move_left(le, 1);
    itl_g_auto_pair_count += 1;
  }
  return true;
}

/* Deletes an empty pair around the caret whose closer the editor inserted,
   and returns whether it did. */
ITL_DEF bool itl_le_auto_pair_erase(itl_le_t *le)
{
  size_t caret = le->cursor_position;
  uint8_t closer;

  if (itl_g_auto_pair_count == 0 || caret == 0) {
    return false;
  }

  closer = itl_auto_pair_closer(itl_le_ascii_at(le, caret - 1));
  if (closer == 0 || itl_le_ascii_at(le, caret) != closer) {
    return false;
  }

  itl_le_move_right(le, 1);
  ITL_LE_ERASE_BACKWARD(le, 2);
  itl_g_auto_pair_count -= 1;
  return true;
}

/* Whether a second TAB opens the selectable menu. A host that wants the plain
   printed list, or that drives another selector of its own, leaves it off. */
ITL_DEF ITL_THREAD_LOCAL int itl_g_completion_menu_enabled = 0;

/* Whether accepted complete candidates receive a trailing separator. */
ITL_DEF ITL_THREAD_LOCAL tl_space_after_completion
    itl_g_space_after_completion = TL_SPACE_AFTER_COMPLETION_OFF;

TL_DEF void tl_set_completion_menu_enabled(int enabled)
{
  /* The menu draws a reversed selection band and dimmed text. A dumb terminal
     keeps the plain list whatever the host requests. */
  itl_g_completion_menu_enabled = enabled && itl_term_supports_decorations();
}

TL_DEF void tl_set_space_after_completion(tl_space_after_completion mode)
{
  itl_g_space_after_completion = mode;
}

TL_DEF void tl_set_colors_enabled(int enabled)
{
  itl_g_colors_enabled = enabled != 0;
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

TL_DEF void tl_set_idle_callback(tl_idle_fn callback, int delay_ms,
                                 int repeat_ms)
{
  itl_g_idle_callback = callback;
  itl_g_idle_delay_ms = delay_ms > 0 ? delay_ms : 0;
  itl_g_idle_repeat_ms = repeat_ms > 0 ? repeat_ms : 1;
  itl_g_idle_due_ms = 0;
}

TL_DEF void tl_set_highlight_follows_cursor(int follows_cursor)
{
  itl_g_highlight_follows_cursor = follows_cursor != 0;
}

TL_DEF void tl_set_highlight_callback(tl_highlight_fn callback)
{
  itl_g_highlight_callback = callback;
}

TL_DEF void tl_set_hint_callback(tl_hint_fn callback)
{
  itl_g_hint_callback = callback;
}

/* Whether every control in the text opens an SGR sequence, ESC [ parameters m.
   Any other control, such as a tab, a carriage return, a cursor movement, or a
   C1 control, moves the cursor by an amount the width walker cannot know. */
ITL_DEF bool itl_text_has_only_sgr_controls(const char *text)
{
  const uint8_t *p = (const uint8_t *) text;

  while (*p != '\0') {
    if (*p == 0x1B) {
      if (p[1] != '[') {
        return false;
      }
      p += 2;
      while (*p >= 0x30 && *p <= 0x3F) {
        p += 1;
      }
      if (*p != 'm') {
        return false;
      }
      p += 1;
      continue;
    }

    if (*p < 0x20 || *p == 0x7F) {
      return false;
    }
    if (p[0] == 0xC2 && p[1] >= 0x80 && p[1] < 0xA0) {
      return false;
    }

    p += 1;
  }

  return true;
}

TL_DEF void tl_set_right_prompt(const char *right_prompt)
{
  itl_g_right_prompt = right_prompt;
  itl_g_right_prompt_width = 0;

  if (right_prompt != NULL && itl_text_has_only_sgr_controls(right_prompt)) {
    itl_g_right_prompt_width = itl_cstr_display_width(right_prompt);
  }
}

TL_DEF void tl_set_transient_prompt(const char *transient_prompt)
{
  itl_g_transient_prompt = transient_prompt;
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
  itl_g_ghost_should_replace_line = false;
}

/* Keep the hint rows away while a menu or a search is open. Rows already on
   screen are erased with one forced text refresh. */
ITL_DEF void itl_hint_hold(itl_le_t *le)
{
  itl_g_hint_hold_count += 1;

  if (itl_g_hint_shown_len > 0) {
    itl_g_tty_should_refresh_text = true;
    itl_le_tty_refresh(le);
  }
}

ITL_DEF void itl_hint_release(void)
{
  itl_g_hint_hold_count -= 1;
}

/* Redraw a finished line after the transient prompt. The erase runs from the
   top of the block to the end of the screen, so the old prompt rows, the right
   prompt, the hint rows, and any rows under the input go with it, and the erase
   and the redraw leave in one write. */
ITL_DEF void itl_le_tty_draw_transient(itl_le_t *le)
{
  itl_char_buf_t *b = &itl_g_char_buffer;

  itl_le_tty_move_to_block_top(b);
  ITL_TTY_MOVE_TO_COLUMN(b, 1);
  ITL_TTY_CLEAR_BELOW(b);

  le->prompt = itl_g_transient_prompt;
  le->prompt_size = strlen(itl_g_transient_prompt);
  le->prompt_width =
      itl_prompt_last_row_width(itl_g_transient_prompt, &le->prompt_rows);

  itl_g_right_prompt_is_held = true;
  itl_g_tty_first_render = true;
  itl_le_invalidate_prev_frame();
  itl_g_tty_should_refresh_text = true;
  if (!itl_le_tty_refresh(le)) {
    ITL_CHAR_BUF_DUMP(b);
    ITL_CHAR_BUF_CLEAR(b);
  }
  itl_g_right_prompt_is_held = false;
}

/* Hand the line back to the host with no ghost left anywhere. itl_ghost_clear
   only drops the recorded text, so a ghost that reached the screen also needs
   one forced text refresh to erase it. With nothing drawn the line on screen is
   already correct and the repaint is skipped, which avoids a full-block flicker
   on a multiline submit. With a transient prompt, a submitted or interrupted
   line is redrawn after it instead, so the scrollback keeps the short prompt
   for a cancelled line too, as zsh transient prompts do. */
ITL_DEF tl_status_code itl_le_finish_input(itl_le_t *le, tl_status_code code)
{
  bool was_ghost_drawn = itl_g_le_prev_ghost_len > 0;
  bool was_hint_drawn = itl_g_hint_shown_len > 0;

  itl_ghost_clear();
  itl_g_hint_is_closed = true;

  if ((code == TL_PRESSED_ENTER || code == TL_PRESSED_INTERRUPT) &&
      itl_g_transient_prompt != NULL)
  {
    itl_le_tty_draw_transient(le);
  } else if (was_ghost_drawn || was_hint_drawn) {
    itl_g_tty_should_refresh_text = true;
    itl_le_tty_refresh(le);
  }

  itl_le_clear_line(le);

  return code;
}

TL_DEF tl_status_code tl_set_signal_keys(int enabled)
{
#if defined ITL_POSIX
  struct termios term;

  ITL_TRY(itl_g_entered_raw_mode, return TL_ERROR);
  ITL_TRY(tcgetattr(STDIN_FILENO, &term) == 0, return TL_ERROR);

  /* Raw mode cleared ISIG, which is why the interrupt key arrives as an
     ordinary byte. Setting it back makes the terminal raise SIGINT again
     without disturbing echo, canonical input, or the timing controls. */
  if (enabled) {
    term.c_lflag |= (tcflag_t) ISIG;
  } else {
    term.c_lflag &= (tcflag_t) ~ISIG;
  }

  ITL_TRY(tcsetattr(STDIN_FILENO, TCSANOW, &term) == 0, return TL_ERROR);
  itl_g_signal_keys_enabled = enabled != 0;

  /* The kitty form of the interrupt key is a sequence the terminal driver does
     not turn into SIGINT, so the extended keys are withdrawn meanwhile. */
  itl_set_extended_keys_active(!enabled && itl_g_extended_keys_enabled);
  return TL_SUCCESS;
#else
  /* The Windows console has no equivalent knob, and the host reads the key
     itself there. */
  (void) enabled;
  return TL_SUCCESS;
#endif /* ITL_POSIX */
}

TL_DEF void tl_set_extended_keys(int enabled)
{
  itl_g_extended_keys_enabled = enabled != 0;
  if (itl_g_entered_raw_mode) {
    itl_set_extended_keys_active(itl_g_extended_keys_enabled &&
                                 !itl_g_signal_keys_enabled);
  }
}

/* The height of the prompt block the picker was opened under, so the resume
   side knows how far back up the block starts. Zero when no handoff is open. */
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_external_screen_rows = 0;

TL_DEF tl_status_code tl_begin_external_screen(void)
{
  itl_char_buf_t *b = &itl_g_char_buffer;
  size_t move_down;
  size_t i;

  TL_ASSERT(itl_g_is_active && "tl_init() should be called");
  ITL_TRY(itl_g_entered_raw_mode, return TL_ERROR);

  /* The prompt and the line being edited stay on screen, so the picker opens
     under them and the user keeps reading what the completion is for. Only the
     cursor moves, down past the last row of the block, which is where a
     height-limited child starts drawing. */
  move_down = itl_g_le_prev_total_rows - (itl_g_le_prev_cursor_row - 1);
  itl_g_external_screen_rows = itl_g_le_prev_total_rows;

  ITL_CHAR_BUF_CLEAR(b);
  for (i = 0; i < move_down; ++i) {
    itl_char_buf_append_cstr(b, ITL_LF);
  }
  if (itl_g_hint_shown_rows > 0) {
    ITL_TTY_MOVE_TO_COLUMN(b, 1);
    itl_le_tty_erase_hint_rows(b);
  }
  ITL_TTY_AUTOWRAP_ON(b);
  ITL_TTY_SHOW_CURSOR(b);
  ITL_CHAR_BUF_DUMP(b);
  ITL_CHAR_BUF_CLEAR(b);

  return tl_exit_raw_mode();
}

TL_DEF tl_status_code tl_end_external_screen(void)
{
  itl_char_buf_t *b = &itl_g_char_buffer;

  TL_ASSERT(itl_g_is_active && "tl_init() should be called");
  ITL_TRY(!itl_g_entered_raw_mode, return TL_ERROR);
  ITL_TRY(tl_enter_raw_mode() == TL_SUCCESS, return TL_ERROR);

  /* A child that limits its own height restores the cursor where it started,
     which is one row past the block. Walking back to the top of the block and
     erasing from there drops both the stale prompt and whatever the child left
     behind, so the repaint below lands the new prompt in the same spot rather
     than under a copy of the old one. */
  ITL_CHAR_BUF_CLEAR(b);
  if (itl_g_external_screen_rows > 0) {
    ITL_TTY_MOVE_UP(b, itl_g_external_screen_rows);
  }
  ITL_TTY_MOVE_TO_COLUMN(b, 1);
  ITL_TTY_CLEAR_BELOW(b);
  ITL_CHAR_BUF_DUMP(b);
  ITL_CHAR_BUF_CLEAR(b);
  itl_g_external_screen_rows = 0;

  /* The child may also have moved the cursor, switched screen buffers, resized
     the window, or repainted anywhere. Every incremental-render shortcut is
     therefore dropped so the next refresh repaints from scratch. The line
     buffer, cursor, undo history, and history draft are untouched. */
  itl_g_tty_changed_size = 1;
  itl_g_tty_first_render = true;
  itl_g_tty_should_refresh_text = true;
  itl_g_tty_plain_append_pending = false;
  itl_le_invalidate_prev_frame();
  itl_ghost_clear();
  itl_g_ghost_sticky_target[0] = '\0';
  return TL_SUCCESS;
}

/* Byte length of the leading word of the ghost, counting the spaces before it.
   A ghost made only of spaces counts whole. */
ITL_DEF size_t itl_ghost_word_length(void)
{
  size_t length = 0;

  while (length < itl_g_ghost_len &&
         ITL_CHAR_IS_SPACE((unsigned char) itl_g_ghost[length]))
  {
    length += 1;
  }
  while (length < itl_g_ghost_len &&
         !ITL_CHAR_IS_SPACE((unsigned char) itl_g_ghost[length]))
  {
    length += 1;
  }

  return length;
}

/* Accepts the first byte_count bytes of the ghost. The whole ghost is accepted
   when byte_count covers it. */
ITL_DEF void itl_ghost_accept_bytes(itl_le_t *le, size_t byte_count)
{
  if (byte_count > itl_g_ghost_len) {
    byte_count = itl_g_ghost_len;
  }

  if (itl_g_ghost_should_replace_line) {
    /* itl_le_clear_line drops the sticky target, so the corrected line is taken
       aside before the line is cleared. */
    char target[ITL_STRING_MAX_LEN];
    size_t target_len = strlen(itl_g_ghost_sticky_target);

    memcpy(target, itl_g_ghost_sticky_target, target_len + 1);
    /* The target ends with the whole ghost, so a partial accept keeps the typed
       part and the accepted bytes of the target. */
    if (byte_count < itl_g_ghost_len && target_len >= itl_g_ghost_len) {
      target[target_len - itl_g_ghost_len + byte_count] = '\0';
    }

    /* The dispatch closed the insert run, so clear_line would otherwise erase
       the typed prefix before any snapshot captures it. Push the pre-accept
       line and open the run first, mirroring the plain accept path where the
       first insert snapshots, so undo restores the typed prefix. */
    itl_undo_push(le);
    itl_g_undo_insert_run_open = true;
    itl_le_clear_line(le);
    itl_le_insert_cstr(le, target);
  } else {
    char accepted[ITL_STRING_MAX_LEN];

    memcpy(accepted, itl_g_ghost, byte_count);
    accepted[byte_count] = '\0';
    itl_le_insert_cstr(le, accepted);
  }
}

ITL_DEF void itl_ghost_accept(itl_le_t *le)
{
  itl_ghost_accept_bytes(le, itl_g_ghost_len);
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
  /* A line that ends in a blank asked about an empty word, which names
     nothing, so its miss says nothing about the word typed next. */
  bool is_word_empty =
      line_byte_len == 0 || line_cstr[line_byte_len - 1] == ' ' ||
      line_cstr[line_byte_len - 1] == '\t';

  if (is_word_empty ||
      line_byte_len >= sizeof(itl_g_ghost_completion_miss_prefix))
  {
    itl_g_ghost_completion_miss_prefix[0] = '\0';
    itl_g_ghost_completion_miss_prefix_length = 0;
    return;
  }

  memcpy(itl_g_ghost_completion_miss_prefix, line_cstr, line_byte_len + 1);
  itl_g_ghost_completion_miss_prefix_length = line_byte_len;
}

/* Fill the ghost from a replacement for the token that starts at token_start.
   The ghost is the part of the replacement past what the user already typed, so
   it only ever appends. A replacement whose typed part differs from the line,
   in case or in spelling, also records the whole corrected line. */
ITL_DEF void itl_ghost_fill_from_token_text(itl_le_t *le, const char *line_cstr,
                                            size_t line_byte_len,
                                            size_t token_start, const char *text)
{
  if (text == NULL || token_start > le->line->length) {
    return;
  }

  {
    /* The token under the cursor runs from token_start to the end of the line.
       Its length in codepoints is the line length minus the start. token_start
       is a codepoint index. The replacement is measured in codepoints and then
       walked to its byte offset. */
    size_t typed_len = le->line->length - token_start;
    size_t text_len = tl_utf8_strlen(text);
    if (text_len <= typed_len) {
      return;
    }
    {
      /* Skip the typed codepoints to find where the untyped byte suffix begins,
         since the bytes the user already typed are not part of the ghost. */
      size_t skip_offset = 0;
      size_t skipped = 0;
      while (skipped < typed_len && text[skip_offset] != '\0') {
        if ((text[skip_offset] & 0xC0) != 0x80) {
          skipped += 1;
        }
        skip_offset += 1;
      }
      /* A continuation byte that belongs to the last skipped codepoint must not
         start the suffix, so advance past the whole codepoint. */
      while (text[skip_offset] != '\0' && (text[skip_offset] & 0xC0) == 0x80) {
        skip_offset += 1;
      }
      size_t suffix_len = strlen(text + skip_offset);
      if (suffix_len >= sizeof(itl_g_ghost)) {
        return;
      }
      memcpy(itl_g_ghost, text + skip_offset, suffix_len);
      itl_g_ghost[suffix_len] = '\0';
      itl_g_ghost_len = suffix_len;
      {
        size_t token_start_bytes = 0;
        size_t cp;
        for (cp = 0; cp < token_start && line_cstr[token_start_bytes] != '\0';
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
          size_t text_byte_len = strlen(text);
          size_t fixed_len = token_start_bytes + text_byte_len;
          int differs =
              skip_offset != typed_byte_len ||
              memcmp(text, line_cstr + token_start_bytes, typed_byte_len) != 0;
          if (differs && fixed_len < sizeof(itl_g_ghost_sticky_target)) {
            memcpy(itl_g_ghost_sticky_target, line_cstr, token_start_bytes);
            memcpy(itl_g_ghost_sticky_target + token_start_bytes, text,
                   text_byte_len + 1);
            itl_g_ghost_should_replace_line = true;
          }
        }
      }
    }
  }
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

  itl_ghost_fill_from_token_text(le, line_cstr, line_byte_len,
                                 result.token_start,
                                 result.longest_common_prefix);
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
          entry_len < sizeof(itl_g_ghost_sticky_target))
      {
        memcpy(itl_g_ghost_sticky_target, entry_cstr, entry_len);
        itl_g_ghost_sticky_target[entry_len] = '\0';
        itl_g_ghost_should_replace_line = true;
      } else {
        itl_g_ghost_should_replace_line = false;
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
  size_t line_byte_len;

  itl_ghost_clear();
  itl_g_serialized_line_ready = false;
  if (!itl_g_tty_plain_append_pending) {
    itl_g_ghost_completion_miss_prefix[0] = '\0';
    itl_g_ghost_completion_miss_prefix_length = 0;
  }

  if (!itl_le_serialize_line(le)) {
    return;
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

  line_byte_len = le->line->size;
  /* An empty line has nothing to extend, and it ends any sticky suggestion so a
     line cleared back to empty does not keep the previous target. */
  if (line_cstr[0] == '\0') {
    itl_g_ghost_sticky_target[0] = '\0';
    return;
  }

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
          itl_g_ghost_should_replace_line =
              memcmp(itl_g_ghost_sticky_target, line_cstr, line_byte_len) != 0;

          itl_g_ghost_width = itl_visible_width(itl_g_ghost, itl_g_ghost_len);
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
        itl_g_ghost_sticky_target[0] = '\0';
      }
    }
  }
  itl_g_ghost_width = itl_visible_width(itl_g_ghost, itl_g_ghost_len);

  /* A source that produced a suggestion records the whole line-plus-ghost as
     the sticky target, so the next keystroke keeps it while the input stays a
     prefix of it rather than re-running the sources and flipping. */
  if (itl_g_ghost_len > 0 && !itl_g_ghost_should_replace_line) {
    if (line_byte_len + itl_g_ghost_len + 1 <=
        sizeof(itl_g_ghost_sticky_target))
    {
      memcpy(itl_g_ghost_sticky_target, line_cstr, line_byte_len);
      memcpy(itl_g_ghost_sticky_target + line_byte_len, itl_g_ghost,
             itl_g_ghost_len + 1);
    }
  }
}

/* The widest description of the list as it is drawn. A list without
   descriptions has none. */
ITL_DEF size_t itl_menu_description_width(const tl_completion *result)
{
  size_t widest = 0;
  size_t i;

  if (result->descriptions == NULL) {
    return 0;
  }

  for (i = 0; i < result->count; ++i) {
    const char *desc = result->descriptions[i];
    size_t width = desc != NULL ? itl_visible_width(desc, strlen(desc)) : 0;

    if (width > widest) {
      widest = width;
    }
  }

  return widest;
}

/* Print the candidate list below the input in columns, then leave the cursor on
   a fresh line so the next refresh redraws the prompt and line beneath the
   list. The previous-render row counts are reset so the refresh treats the spot
   below the list as untouched ground and does not clear the list it just
   printed. */
ITL_DEF void itl_completion_print_list(const tl_completion *result,
                                       size_t anchor)
{
  itl_char_buf_t *b = &itl_g_char_buffer;
  size_t tty_cols = itl_g_tty_prev_cols > 0 ? itl_g_tty_prev_cols : 80;
  size_t longest = 0;
  size_t widest_desc;
  size_t i, column_width, columns, column;

  /* Move below the whole input block, the same accounting tl_emit_newlines
     uses, so the list never lands on top of the line. */
  size_t move_down = itl_g_le_prev_total_rows - (itl_g_le_prev_cursor_row - 1);

  ITL_CHAR_BUF_CLEAR(b);
  ITL_TTY_SHOW_CURSOR(b);
  for (i = 0; i < move_down; ++i) {
    itl_char_buf_append_cstr(b, ITL_LF);
  }
  if (itl_g_hint_shown_rows > 0) {
    ITL_TTY_MOVE_TO_COLUMN(b, 1);
    itl_le_tty_erase_hint_rows(b);
  }

  for (i = 0; i < result->count; ++i) {
    const char *name = result->candidates[i];
    size_t len = itl_visible_width(name, strlen(name));
    if (len > longest) {
      longest = len;
    }
  }

  /* The list starts under the token when its widest name, and the widest
     description beside it when there are descriptions, fits to the right of
     it, and at the leftmost column otherwise. */
  widest_desc = itl_menu_description_width(result);
  if (anchor + longest + (widest_desc > 0 ? 2 + widest_desc : 0) >= tty_cols) {
    anchor = 0;
  }
  tty_cols -= anchor;

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
      size_t len;
      size_t pad;
      itl_char_buf_append_spaces(b, anchor);
      len = itl_char_buf_append_visible(b, name, strlen(name), (size_t) -1);
      if (desc != NULL && desc[0] != '\0') {
        size_t line_len = 0;
        const char *p = desc;
        for (pad = len; pad < column_width; ++pad) {
          itl_char_buf_append_byte(b, ' ');
        }
        itl_char_buf_append_cstr(b, itl_color_sequence(ITL_DIM_SGR));
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
            for (k = 0; k < anchor + column_width; ++k)
              itl_char_buf_append_byte(b, ' ');
            line_len = 0;
          }
          if (line_len > 0) {
            itl_char_buf_append_byte(b, ' ');
            line_len += 1;
          }
          line_len += itl_char_buf_append_visible(b, word, word_len,
                                                  (size_t) -1);
        }
        itl_char_buf_append_cstr(b, itl_color_sequence(ITL_HIGHLIGHT_RESET));
      }
      itl_char_buf_append_cstr(b, ITL_LF);
    }
  } else {
    column = 0;
    for (i = 0; i < result->count; ++i) {
      const char *name = result->candidates[i];
      size_t len;
      size_t pad;

      if (column == 0) {
        itl_char_buf_append_spaces(b, anchor);
      }
      len = itl_char_buf_append_visible(b, name, strlen(name), (size_t) -1);
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
  itl_le_invalidate_prev_frame();
  itl_g_tty_should_refresh_text = true;
}

/* Replace the whole token span [token_start, token_end) with text, in codepoint
   units, leaving the cursor at the end of the inserted text. The span is erased
   first so a mid-word cursor does not keep the bytes to its right, then the
   replacement is inserted at the token start. Both make one undo step that
   restores the token and the caret. */
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
    bool is_inserted;

    itl_history_reset_after_edit(le);
    itl_undo_push(le);
    le->cursor_position = token_start;
    if (token_len > 0) {
      itl_string_erase(le->line, token_start, token_len, false);
    }
    itl_g_undo_insert_run_open = true;
    is_inserted = itl_le_insert_cstr(le, text);
    itl_g_undo_insert_run_open = false;
    return is_inserted;
  }
}

ITL_DEF bool itl_byte_is_path_separator(uint8_t byte)
{
#if defined ITL_WIN32
  if (byte == '\\') {
    return true;
  }
#endif

  return byte == '/';
}

/* Append the space the option asks for after a completed word. The space
   joins the undo step of the completion it follows. */
ITL_DEF void itl_completion_append_space(itl_le_t *le)
{
  if (itl_g_space_after_completion == TL_SPACE_AFTER_COMPLETION_OFF ||
      le->cursor_position != le->line->length || le->line->length == 0)
    return;

  itl_utf8_t last = le->line->chars[le->line->length - 1];
  if (last.size == 1 && isspace(last.bytes[0])) return;
  if (itl_g_space_after_completion ==
          TL_SPACE_AFTER_COMPLETION_EXCEPT_AFTER_SLASH &&
      last.size == 1 && itl_byte_is_path_separator((uint8_t) last.bytes[0]))
    return;
  itl_g_undo_insert_run_open = true;
  itl_le_insert(le, itl_utf8_parse(' '));
  itl_g_undo_insert_run_open = false;
}

/* The selectable candidate menu drawn under the input block. Its rows sit
   outside the rows the line editor tracks. The menu clears them itself before
   every repaint and before it returns. */
#define ITL_MENU_MAX_ROWS         16
#define ITL_MENU_ROW_PREFIX       "  "
#define ITL_MENU_ROW_PREFIX_WIDTH 2
/* The reversed block of a selected row opens one column before the entry and
   closes one column after it. The plain first column of the prefix keeps the
   row aligned with an unselected one. */
#define ITL_MENU_SELECTED_MARGIN       " "
#define ITL_MENU_SELECTED_MARGIN_WIDTH 1
#define ITL_MENU_SELECTED_SGR          "\x1b[7m"
#define ITL_MENU_DESCRIPTION_SGR       ITL_DIM_SGR
#define ITL_MENU_TITLE_SEPARATOR       ", "
#define ITL_MENU_TITLE_SEPARATOR_WIDTH 2
/* The row drawn in place of the candidates once the search has narrowed the
   list away. The menu stays open on it and a backspace brings the list back. */
#define ITL_MENU_EMPTY_TEXT      "no matches, erase to widen the search"
#define ITL_MENU_LOADING_TEXT    "loading..."

ITL_DEF tl_status_code itl_le_key_handle(itl_le_t *le, int esc);

/* Every row of one menu repaint. The help row and the count row are counted
   here beside the candidates, and a repaint never draws a row the layout did
   not grant. */
typedef struct itl_menu_layout
{
  size_t candidate_rows;
  size_t help_row_count;
  bool has_help_row;
  bool has_count_row;
} itl_menu_layout;

/* Divide the rows under the input block between the help rows, the candidates,
   and the count row. The whole block is held back first, since the terminal
   scrolls the prompt away once the rows below it are overrun. The help rows are
   the first to go and the count row is the second. A block that already fills
   the terminal leaves nothing, and the menu then draws no rows at all. The
   fixed ceiling keeps a long list from filling a tall terminal. The help text
   asks for help_rows rows, and zero asks for none. */
ITL_DEF itl_menu_layout itl_menu_measure(size_t tty_rows, size_t help_rows)
{
  itl_menu_layout layout;
  size_t available = tty_rows > itl_g_le_prev_total_rows
                         ? tty_rows - itl_g_le_prev_total_rows
                         : 0;

  layout.has_help_row = help_rows > 0 && available >= help_rows + 2;
  layout.help_row_count = layout.has_help_row ? help_rows : 0;
  layout.has_count_row = available >= 2;

  if (layout.has_help_row) {
    available -= help_rows;
  }

  if (layout.has_count_row) {
    available -= 1;
  }

  layout.candidate_rows =
      available > ITL_MENU_MAX_ROWS ? ITL_MENU_MAX_ROWS : available;

  return layout;
}

/* The index of the first candidate drawn. The origin is clamped against the end
   of the list, then moved by the least amount that brings the selection back
   into view. */
ITL_DEF size_t itl_menu_window_start(size_t count, size_t selected,
                                     size_t window_start, size_t rows)
{
  size_t last_start;

  if (count <= rows) {
    return 0;
  }

  last_start = count - rows;
  if (window_start > last_start) {
    window_start = last_start;
  }
  if (selected < window_start) {
    return selected;
  }
  if (selected >= window_start + rows) {
    return selected + 1 - rows;
  }

  return window_start;
}

#define ITL_MENU_CUT_MARK       "..."
#define ITL_MENU_CUT_MARK_WIDTH 3

/* The columns of a cell left for text before the mark that ends a cut cell.
   A cell too narrow to hold the mark and a character is cut bare. */
ITL_DEF size_t itl_menu_cell_text_width(const char *text, size_t text_bytes,
                                        size_t width)
{
  if (width <= ITL_MENU_CUT_MARK_WIDTH ||
      itl_visible_width(text, text_bytes) <= width)
  {
    return width;
  }

  return width - ITL_MENU_CUT_MARK_WIDTH;
}

/* Append at most width columns of text and pad the remainder with spaces when
   the caller asked for a fixed cell. Text cut to the width ends in an
   ellipsis. Returns the columns written, which can exceed the width by one
   when a double-width character straddles the edge. */
ITL_DEF size_t itl_menu_append_cell(itl_char_buf_t *b, const char *text,
                                    size_t width, bool should_pad)
{
  size_t text_bytes = strlen(text);
  size_t text_width = itl_menu_cell_text_width(text, text_bytes, width);
  size_t offset = 0;
  size_t drawn = 0;

  itl_char_buf_reserve(b, b->size + text_bytes + width);

  while (offset < text_bytes) {
    size_t step_bytes = 0;
    size_t step_width = 0;

    itl_visible_step(text + offset, text_bytes - offset, &step_bytes,
                     &step_width);
    if (step_width > 0 &&
        (drawn >= text_width ||
         (text_width < width && drawn + step_width > text_width)))
    {
      break;
    }
    itl_char_buf_append_visible_step(b, text + offset, step_bytes);
    offset += step_bytes;
    drawn += step_width;
  }

  if (text_width < width) {
    itl_char_buf_append_cstr(b, ITL_MENU_CUT_MARK);
    drawn += ITL_MENU_CUT_MARK_WIDTH;
  }

  if (!should_pad) {
    return drawn;
  }

  if (drawn < width) {
    itl_char_buf_append_spaces(b, width - drawn);
  }

  return drawn > width ? drawn : width;
}

/* The bytes the rows of a completion menu leave out of every candidate: the
   directory all of them share, so a path list shows only the last component
   of each path, the way bash lists one. The menu loop sets it for the draw
   and clears it after, so every other width reads whole names. */
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_menu_name_skip = 0;

/* The size of the directory every candidate shares, up to and including its
   last path separator, or zero when they share none. */
ITL_DEF size_t itl_menu_common_directory_size(const tl_completion *result)
{
  const char *first;
  size_t common_size;
  size_t directory_size = 0;
  size_t index;
  size_t position;

  if (result->count == 0 || result->candidates == NULL) {
    return 0;
  }

  first = result->candidates[0];
  common_size = strlen(first);
  for (index = 1; index < result->count && common_size > 0; ++index) {
    const char *candidate = result->candidates[index];

    position = 0;
    while (position < common_size && candidate[position] == first[position]) {
      position += 1;
    }
    common_size = position;
  }

  for (position = 0; position < common_size; ++position) {
    if (itl_byte_is_path_separator((uint8_t) first[position])) {
      directory_size = position + 1;
    }
  }

  return directory_size;
}

/* The part of a candidate a menu row shows. A candidate that is only the
   shared directory keeps it whole. */
ITL_DEF const char *itl_menu_shown_part(const char *candidate)
{
  if (itl_g_menu_name_skip == 0 ||
      strlen(candidate) <= itl_g_menu_name_skip)
  {
    return candidate;
  }

  return candidate + itl_g_menu_name_skip;
}

/* Return a single-line display copy without changing the candidate used for
   matching or acceptance. */
ITL_DEF const char *itl_menu_display_name(const char *name, char *storage,
                                         size_t storage_size)
{
  const char *line_break = strpbrk(name, "\r\n");
  size_t prefix_length;

  if (line_break == NULL) {
    return name;
  }
  if (storage_size == 0) {
    return "";
  }
  if (storage_size <= 3) {
    prefix_length = storage_size - 1;
    memcpy(storage, "...", prefix_length);
    storage[prefix_length] = '\0';
    return storage;
  }

  prefix_length = (size_t) (line_break - name);
  if (prefix_length + 3 >= storage_size) {
    prefix_length = storage_size > 4 ? storage_size - 4 : 0;
  }

  memcpy(storage, name, prefix_length);
  memcpy(storage + prefix_length, "...", 4);
  return storage;
}

/* Draw a cell whose text carries the colors the host chose for it. The spans
   are codepoint ranges over that same text, sorted and non-overlapping. Each
   colored run opens with its own sequence and closes with a reset. The cell
   keeps the width its column grants, ends cut text in an ellipsis, and pads
   the remainder for a caller that asked for a fixed cell. */
ITL_DEF void itl_menu_append_colored_cell(itl_char_buf_t *b, const char *text,
                                          size_t width,
                                          const tl_highlight_span *spans,
                                          size_t span_count, bool should_pad)
{
  size_t text_bytes = strlen(text);
  size_t text_width = itl_menu_cell_text_width(text, text_bytes, width);
  size_t byte_offset = 0;
  size_t codepoint_index = 0;
  size_t drawn = 0;
  size_t next_span = 0;
  size_t open_span = span_count;

  itl_char_buf_reserve(b, b->size + text_bytes + width);

  while (byte_offset < text_bytes && drawn < text_width) {
    size_t active = span_count;
    size_t step_bytes = 0;
    size_t step_width = 0;

    while (next_span < span_count && spans[next_span].end <= codepoint_index) {
      next_span += 1;
    }

    if (next_span < span_count && spans[next_span].start <= codepoint_index) {
      active = next_span;
    }

    if (active != open_span) {
      if (open_span != span_count) {
        itl_char_buf_append_cstr(b, itl_color_sequence(ITL_HIGHLIGHT_RESET));
      }

      if (active != span_count) {
        itl_char_buf_append_cstr(b, itl_color_sequence(spans[active].sgr));
      }

      open_span = active;
    }

    itl_visible_step(text + byte_offset, text_bytes - byte_offset, &step_bytes,
                     &step_width);

    if (step_width > 0 && drawn + step_width > text_width) {
      break;
    }

    itl_char_buf_append_visible_step(b, text + byte_offset, step_bytes);

    byte_offset += step_bytes;
    codepoint_index += 1;
    drawn += step_width;
  }

  if (open_span != span_count) {
    itl_char_buf_append_cstr(b, itl_color_sequence(ITL_HIGHLIGHT_RESET));
  }

  if (text_width < width) {
    itl_char_buf_append_cstr(b, ITL_MENU_CUT_MARK);
    drawn += ITL_MENU_CUT_MARK_WIDTH;
  }

  if (!should_pad) {
    return;
  }

  if (drawn < width) {
    itl_char_buf_append_spaces(b, width - drawn);
  }
}

/* Draw one candidate. The name keeps a fixed column when a description follows
   it. Every description starts at the same offset, and a row without one ends
   right after the name. The selected row reverses only the displayed entry.
   A source that asked for highlighting has its names colored by the host. The
   selected row stays plain. Reverse video already marks it, and the ghost
   preview shows it colored on the line above. */
ITL_DEF void itl_menu_append_row(itl_char_buf_t *b, const tl_completion *result,
                                 size_t index, size_t name_width,
                                 size_t desc_width, bool is_selected,
                                 bool should_highlight)
{
  char display_name_storage[ITL_STRING_MAX_LEN + 1];
  const char *name =
      itl_menu_display_name(itl_menu_shown_part(result->candidates[index]),
                            display_name_storage, sizeof(display_name_storage));
  const char *desc =
      result->descriptions != NULL ? result->descriptions[index] : NULL;
  bool has_description = desc != NULL && desc[0] != '\0' && desc_width > 0;
  bool was_name_drawn = false;

  if (is_selected) {
    itl_char_buf_append_cstr(b, ITL_MENU_SELECTED_MARGIN);
    itl_char_buf_append_cstr(b, ITL_MENU_SELECTED_SGR);
    itl_char_buf_append_cstr(b, ITL_MENU_SELECTED_MARGIN);
  } else {
    itl_char_buf_append_cstr(b, ITL_MENU_ROW_PREFIX);
  }

  if (should_highlight && !is_selected && itl_should_run_highlight()) {
    tl_highlight_span name_spans[ITL_HIGHLIGHT_MAX_SPANS];
    tl_highlight hl;

    hl.spans = name_spans;
    hl.count = 0;
    hl.capacity = ITL_HIGHLIGHT_MAX_SPANS;
    hl.cursor = TL_HIGHLIGHT_NO_CURSOR;

    if (itl_g_highlight_callback(name, &hl)) {
      size_t span_count = hl.count < ITL_HIGHLIGHT_MAX_SPANS
                              ? hl.count
                              : ITL_HIGHLIGHT_MAX_SPANS;

      itl_menu_append_colored_cell(b, name, name_width, name_spans, span_count,
                                   has_description);
      was_name_drawn = true;
    }
  }

  if (!was_name_drawn) {
    itl_menu_append_cell(b, name, name_width, has_description);
  }

  if (has_description) {
    itl_char_buf_append_byte(b, ' ');
    if (!is_selected) {
      itl_char_buf_append_cstr(b, itl_color_sequence(ITL_MENU_DESCRIPTION_SGR));
    }

    itl_menu_append_cell(b, desc, desc_width, false);

    if (!is_selected) {
      itl_char_buf_append_cstr(b, itl_color_sequence(ITL_HIGHLIGHT_RESET));
    }
  }

  if (is_selected) {
    itl_char_buf_append_cstr(b, ITL_MENU_SELECTED_MARGIN);
    itl_char_buf_append_cstr(b, ITL_HIGHLIGHT_RESET);
  }
}

/* Draw the dimmed count row shown while part of the list is out of view. */
ITL_DEF void itl_menu_append_summary(itl_char_buf_t *b, size_t first,
                                     size_t last, size_t count)
{
  itl_char_buf_append_cstr(b, ITL_MENU_ROW_PREFIX);
  itl_char_buf_append_cstr(b, itl_color_sequence(ITL_MENU_DESCRIPTION_SGR));
  itl_char_buf_append_cstr(b, "showing ");
  itl_char_buf_append_size_t(b, first);
  itl_char_buf_append_byte(b, '-');
  itl_char_buf_append_size_t(b, last);
  itl_char_buf_append_cstr(b, " of ");
  itl_char_buf_append_size_t(b, count);
  itl_char_buf_append_cstr(b, itl_color_sequence(ITL_HIGHLIGHT_RESET));
}

/* Draw the dimmed row that stands in for an empty list. The text is cut at the
   row width and never wraps. */
ITL_DEF void itl_menu_append_dimmed_row(itl_char_buf_t *b, const char *text,
                                        size_t width)
{
  itl_char_buf_append_cstr(b, ITL_MENU_ROW_PREFIX);
  itl_char_buf_append_cstr(b, itl_color_sequence(ITL_MENU_DESCRIPTION_SGR));
  itl_menu_append_cell(b, text, width, false);
  itl_char_buf_append_cstr(b, itl_color_sequence(ITL_HIGHLIGHT_RESET));
}

/* Park the caret on the column the rows start at. The leftmost column is where
   every row break already lands. */
ITL_DEF void itl_menu_move_to_anchor(itl_char_buf_t *b, size_t anchor)
{
  if (anchor > 0) {
    ITL_TTY_MOVE_TO_COLUMN(b, anchor + 1);
  }
}

/* Append the leading part of text that fits in width columns, ending in an
   ellipsis when something was cut. */
ITL_DEF void itl_menu_append_elided(itl_char_buf_t *b, const char *text,
                                    size_t length, size_t width)
{
  size_t keep = width > 3 ? width - 3 : width;
  size_t drawn = itl_char_buf_append_visible(b, text, length, keep);

  if (width > 3 && itl_visible_width(text, length) > drawn) {
    itl_char_buf_append_cstr(b, "...");
  }
}

/* Lay the help text out over rows of at most width columns, and draw it when b
   is not null. The phrase naming the active source and the keys it answers
   read as one list of items split at the item separator, and a row breaks only
   between two items. Every row opens with the row prefix, carries the dim of
   every other secondary text, and starts at the anchor column. An item wider
   than a row is cut and ends in an ellipsis. Items past max_rows are dropped.
   Returns the rows the text takes. */
ITL_DEF size_t itl_menu_layout_help(itl_char_buf_t *b, const char *title,
                                    const char *keys, size_t width,
                                    size_t anchor, size_t max_rows)
{
  const char *parts[2];
  size_t part_count = keys != NULL ? 2 : 1;
  size_t rows = 0;
  size_t used = 0;
  size_t part;
  bool is_full = false;

  parts[0] = title;
  parts[1] = keys;

  if (width == 0 || max_rows == 0) {
    return 0;
  }

  for (part = 0; part < part_count && !is_full; ++part) {
    const char *item = parts[part];

    while (*item != '\0') {
      const char *separator = strstr(item, ITL_MENU_TITLE_SEPARATOR);
      size_t length = separator != NULL ? (size_t) (separator - item)
                                        : strlen(item);
      bool has_next = separator != NULL || part + 1 < part_count;
      size_t item_width = itl_visible_width(item, length) + (has_next ? 1 : 0);
      bool is_row_open = rows > 0 && used > 0;
      size_t cut;

      if (is_row_open && used + 1 + item_width > width) {
        is_row_open = false;
      }

      if (!is_row_open) {
        if (rows >= max_rows) {
          is_full = true;
          break;
        }
        if (b != NULL && rows > 0) {
          itl_char_buf_append_cstr(b, itl_color_sequence(ITL_HIGHLIGHT_RESET));
        }
        if (b != NULL) {
          if (rows > 0) {
            itl_char_buf_append_cstr(b, ITL_LF);
            itl_menu_move_to_anchor(b, anchor);
          }
          itl_char_buf_append_cstr(b, ITL_MENU_ROW_PREFIX);
          itl_char_buf_append_cstr(b,
                                   itl_color_sequence(ITL_MENU_DESCRIPTION_SGR));
        }
        rows += 1;
        used = 0;
      } else {
        if (b != NULL) {
          itl_char_buf_append_byte(b, ' ');
        }
        used += 1;
      }

      cut = item_width > width ? width : item_width;
      if (b != NULL) {
        if (item_width > width) {
          itl_menu_append_elided(b, item, length, cut);
        } else {
          itl_char_buf_append_visible(b, item, length, width);
          if (has_next) {
            itl_char_buf_append_byte(b, ',');
          }
        }
      }
      used += cut;

      item = separator != NULL ? separator + ITL_MENU_TITLE_SEPARATOR_WIDTH
                               : item + length;
    }
  }

  if (b != NULL && rows > 0) {
    itl_char_buf_append_cstr(b, itl_color_sequence(ITL_HIGHLIGHT_RESET));
  }

  return rows;
}

/* Step to the first row below the input block and clear everything under it,
   the same accounting tl_emit_newlines uses. Returns the rows stepped over so
   the caller can walk back up to the caret. */
ITL_DEF size_t itl_menu_open_area(itl_char_buf_t *b)
{
  size_t move_down = itl_g_le_prev_total_rows - (itl_g_le_prev_cursor_row - 1);
  size_t i;

  for (i = 0; i < move_down; ++i) {
    itl_char_buf_append_cstr(b, ITL_LF);
  }
  ITL_TTY_MOVE_TO_COLUMN(b, 1);
  ITL_TTY_CLEAR_BELOW(b);

  return move_down;
}

/* Park the caret back on the input line after the menu rows were written. */
ITL_DEF void itl_menu_close_area(itl_char_buf_t *b, size_t rows_below)
{
  if (rows_below > 0) {
    ITL_TTY_MOVE_UP(b, rows_below);
  }
  ITL_TTY_MOVE_TO_COLUMN(b, itl_g_le_prev_cursor_col + 1);
  ITL_TTY_SHOW_CURSOR(b);
  ITL_CHAR_BUF_DUMP(b);
  ITL_CHAR_BUF_CLEAR(b);
}

ITL_DEF size_t itl_menu_name_width(const tl_completion *result)
{
  char display_name_storage[ITL_STRING_MAX_LEN + 1];
  size_t widest = 0;
  size_t i;

  for (i = 0; i < result->count; ++i) {
    const char *name =
        itl_menu_display_name(itl_menu_shown_part(result->candidates[i]),
                              display_name_storage,
                              sizeof(display_name_storage));
    size_t width = itl_visible_width(name, strlen(name));

    if (width > widest) {
      widest = width;
    }
  }

  return widest;
}

/* The display column, counted from zero, of the token being completed. The menu
   loop sets it. A draw moves the row prefix left of it so the candidate text
   sits under the token, and uses it only when the whole menu fits to its right.
   It falls back to the leftmost column when it does not. */
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_menu_anchor_column = 0;

/* The display column where the codepoint at index token_start of the line
   begins, wrapped the way the renderer wraps the line. */
ITL_DEF size_t itl_menu_anchor_column_of(const itl_le_t *le,
                                         size_t token_start)
{
  size_t cols = itl_g_tty_prev_cols > 0 ? itl_g_tty_prev_cols : 80;
  size_t indent = ITL_LE_INDENT(le, cols);
  size_t row = 0;
  size_t col = itl_le_prompt_indent(le, cols);

  if (token_start > le->line->length) {
    token_start = le->line->length;
  }
  itl_wrap_walk_range(le->line, 0, token_start, cols, indent, &row, &col);

  return col;
}

/* Where the rows of a menu start and how wide they are. The rows start under
   the token only when the whole row fits to its right, the prefix, the widest
   name, the widest description and the gap before it included, only when no
   item of the help text would be cut there, and only when the help text takes
   no more rows there than at the leftmost column. Otherwise the
   rows start at the leftmost column, and a description too wide for the
   terminal is cut there. */
typedef struct itl_menu_geometry
{
  size_t anchor;
  size_t row_cols;
  size_t text_width;
  size_t name_width;
  size_t desc_width;
} itl_menu_geometry;

/* The widest item of the help text, its trailing comma included. A row narrower
   than that would cut the item. */
ITL_DEF size_t itl_menu_widest_help_item(const char *title, const char *keys)
{
  const char *parts[2];
  size_t part_count = keys != NULL ? 2 : 1;
  size_t widest = 0;
  size_t part;

  parts[0] = title;
  parts[1] = keys;

  if (title == NULL) {
    return 0;
  }

  for (part = 0; part < part_count; ++part) {
    const char *item = parts[part];

    while (*item != '\0') {
      const char *separator = strstr(item, ITL_MENU_TITLE_SEPARATOR);
      size_t length = separator != NULL ? (size_t) (separator - item)
                                        : strlen(item);
      bool has_next = separator != NULL || part + 1 < part_count;
      size_t width = itl_visible_width(item, length) + (has_next ? 1 : 0);

      if (width > widest) {
        widest = width;
      }

      item = separator != NULL ? separator + ITL_MENU_TITLE_SEPARATOR_WIDTH
                               : item + length;
    }
  }

  return widest;
}

ITL_DEF itl_menu_geometry itl_menu_geometry_of(const tl_completion *result,
                                               size_t name_width,
                                               const char *empty_text,
                                               const char *help_title,
                                               const char *help_keys)
{
  itl_menu_geometry geometry;
  size_t tty_cols = itl_g_tty_prev_cols > 0 ? itl_g_tty_prev_cols : 80;
  size_t full_cols = tty_cols > 1 ? tty_cols - 1 : 1;
  size_t anchor = itl_g_menu_anchor_column >= ITL_MENU_ROW_PREFIX_WIDTH
                      ? itl_g_menu_anchor_column - ITL_MENU_ROW_PREFIX_WIDTH
                      : 0;
  size_t widest_desc = result->count == 0 ? 0 : itl_menu_description_width(result);
  size_t needed_cols = ITL_MENU_ROW_PREFIX_WIDTH;
  size_t help_cols = ITL_MENU_ROW_PREFIX_WIDTH +
                     itl_menu_widest_help_item(help_title, help_keys);

  if (result->count == 0) {
    needed_cols += itl_visible_width(empty_text, strlen(empty_text));
  } else {
    needed_cols += name_width + ITL_MENU_SELECTED_MARGIN_WIDTH;
    if (widest_desc > 0) {
      needed_cols += 1 + widest_desc;
    }
  }

  if (help_cols > needed_cols) {
    needed_cols = help_cols;
  }

  if (anchor + needed_cols > full_cols) {
    anchor = 0;
  }

  if (anchor > 0 && help_title != NULL &&
      itl_menu_layout_help(NULL, help_title, help_keys,
                           full_cols - anchor - ITL_MENU_ROW_PREFIX_WIDTH,
                           anchor, (size_t) -1) >
          itl_menu_layout_help(NULL, help_title, help_keys,
                               full_cols - ITL_MENU_ROW_PREFIX_WIDTH, 0,
                               (size_t) -1))
  {
    anchor = 0;
  }

  geometry.anchor = anchor;
  geometry.row_cols = full_cols - anchor;
  geometry.text_width = geometry.row_cols > ITL_MENU_ROW_PREFIX_WIDTH
                            ? geometry.row_cols - ITL_MENU_ROW_PREFIX_WIDTH
                            : 1;

  if (name_width + ITL_MENU_ROW_PREFIX_WIDTH + ITL_MENU_SELECTED_MARGIN_WIDTH >=
      geometry.row_cols)
  {
    name_width =
        geometry.row_cols >
                ITL_MENU_ROW_PREFIX_WIDTH + ITL_MENU_SELECTED_MARGIN_WIDTH
            ? geometry.row_cols - ITL_MENU_ROW_PREFIX_WIDTH -
                  ITL_MENU_SELECTED_MARGIN_WIDTH
            : 1;
  }
  geometry.name_width = name_width;
  geometry.desc_width =
      geometry.row_cols > ITL_MENU_ROW_PREFIX_WIDTH + name_width + 1 +
                              ITL_MENU_SELECTED_MARGIN_WIDTH
          ? geometry.row_cols - ITL_MENU_ROW_PREFIX_WIDTH - name_width - 1 -
                ITL_MENU_SELECTED_MARGIN_WIDTH
          : 0;

  return geometry;
}

/* Divide the rows under the input block for a list about to be drawn. The help
   text wraps at the width the rows will have, so its row count follows the
   anchor the same fit rule picks for the draw. */
ITL_DEF itl_menu_layout itl_menu_measure_for(const tl_completion *result,
                                             size_t tty_rows, size_t name_width,
                                             const char *empty_text,
                                             const char *help_title,
                                             const char *help_keys)
{
  size_t help_rows = 0;

  if (help_title != NULL) {
    itl_menu_geometry geometry =
        itl_menu_geometry_of(result, name_width, empty_text, help_title,
                             help_keys);

    help_rows = itl_menu_layout_help(NULL, help_title, help_keys,
                                     geometry.text_width, geometry.anchor,
                                     tty_rows);
  }

  return itl_menu_measure(tty_rows, help_rows);
}

/* Repaint the menu rows under the input block. The rows are written from the
   first row below the block downward, and the caret returns to the line. The
   editor's own render path never sees them. The layout owns which rows exist,
   and a help_title of null drops the help rows the layout granted. The help
   text wraps onto as many rows as the layout granted. An empty list draws the
   row that says so in place of the candidates. A layout with no candidate row
   leaves the screen untouched. */
ITL_DEF void itl_menu_draw(const tl_completion *result, size_t selected,
                           size_t window_start, itl_menu_layout layout,
                           const char *help_title, const char *help_keys,
                           bool should_highlight, size_t name_width,
                           const char *empty_text)
{
  itl_char_buf_t *b = &itl_g_char_buffer;
  itl_menu_geometry geometry;
  size_t anchor;
  size_t text_width;
  size_t window_end = window_start + layout.candidate_rows;
  size_t desc_width;
  size_t drawn_rows = 0;
  size_t move_down;
  size_t i;

  if (layout.candidate_rows == 0) {
    return;
  }

  geometry = itl_menu_geometry_of(result, name_width, empty_text, help_title,
                                  help_keys);
  anchor = geometry.anchor;
  text_width = geometry.text_width;
  name_width = geometry.name_width;
  desc_width = geometry.desc_width;

  if (window_end > result->count) {
    window_end = result->count;
  }

  ITL_CHAR_BUF_CLEAR(b);
  ITL_TTY_HIDE_CURSOR(b);
  move_down = itl_menu_open_area(b);
  itl_menu_move_to_anchor(b, anchor);

  if (layout.has_help_row && help_title != NULL) {
    drawn_rows += itl_menu_layout_help(b, help_title, help_keys, text_width,
                                       anchor, layout.help_row_count);
  }

  if (result->count == 0) {
    if (drawn_rows > 0) {
      itl_char_buf_append_cstr(b, ITL_LF);
      itl_menu_move_to_anchor(b, anchor);
    }
    itl_menu_append_dimmed_row(b, empty_text, text_width);
    drawn_rows += 1;
  }

  for (i = window_start; i < window_end; ++i) {
    if (drawn_rows > 0) {
      itl_char_buf_append_cstr(b, ITL_LF);
      itl_menu_move_to_anchor(b, anchor);
    }
    itl_menu_append_row(b, result, i, name_width, desc_width, i == selected,
                        should_highlight);
    drawn_rows += 1;
  }

  if (layout.has_count_row && (window_start > 0 || window_end < result->count))
  {
    if (drawn_rows > 0) {
      itl_char_buf_append_cstr(b, ITL_LF);
      itl_menu_move_to_anchor(b, anchor);
    }
    itl_menu_append_summary(b, window_start + 1, window_end, result->count);
    drawn_rows += 1;
  }

  itl_menu_close_area(b, drawn_rows > 0 ? move_down + drawn_rows - 1
                                        : move_down);
}

/* Wipe every row the menu drew and leave the caret on the input line. */
ITL_DEF void itl_menu_erase(void)
{
  itl_char_buf_t *b = &itl_g_char_buffer;
  size_t move_down;

  ITL_CHAR_BUF_CLEAR(b);
  ITL_TTY_HIDE_CURSOR(b);
  move_down = itl_menu_open_area(b);
  itl_menu_close_area(b, move_down);
}

ITL_DEF bool itl_refresh_after_wake(itl_le_t *le)
{
  itl_char_buf_t *wake_buf;

  if (itl_g_wake_callback == NULL || !itl_g_wake_callback(0)) {
    return false;
  }

  wake_buf = &itl_g_char_buffer;
  itl_le_tty_move_to_block_top(wake_buf);
  ITL_TTY_MOVE_TO_COLUMN(wake_buf, 1);
  ITL_TTY_CLEAR_BELOW(wake_buf);
  ITL_CHAR_BUF_DUMP(wake_buf);
  ITL_CHAR_BUF_CLEAR(wake_buf);
  itl_g_wake_callback(1);
  itl_g_tty_first_render = true;
  itl_le_invalidate_prev_frame();
  itl_g_tty_should_refresh_text = true;
  itl_le_tty_refresh(le);
  return true;
}

/* A handled key starts a new pause, so the idle hook is due a whole delay from
   now. */
ITL_DEF void itl_idle_arm(void)
{
  itl_g_idle_due_ms = itl_g_idle_callback == NULL
                          ? 0
                          : itl_monotonic_ms() + (uint64_t) itl_g_idle_delay_ms;
}

/* The wait the input loop may spend before the idle hook is due, or -1 when no
   call is due. */
ITL_DEF int itl_idle_wait_ms(void)
{
  uint64_t now_ms;

  if (itl_g_idle_due_ms == 0 || itl_g_idle_callback == NULL) {
    return -1;
  }

  now_ms = itl_monotonic_ms();
  if (now_ms >= itl_g_idle_due_ms) {
    return 0;
  }
  if (itl_g_idle_due_ms - now_ms > (uint64_t) INT_MAX) {
    return INT_MAX;
  }
  return (int) (itl_g_idle_due_ms - now_ms);
}

/* Call the idle hook once for the line as it stands. A refresh that asks only
   for the caret composes the hint again and turns into a text refresh only when
   the row changed, so an unchanged hint redraws nothing. */
ITL_DEF void itl_idle_run(itl_le_t *le)
{
  bool was_serialized = itl_g_serialized_line_ready;
  bool was_text_refresh = itl_g_tty_should_refresh_text;
  int outcome;

  itl_g_idle_due_ms = 0;
  if (itl_g_idle_callback == NULL || !itl_le_serialize_line(le)) {
    itl_g_serialized_line_ready = was_serialized;
    return;
  }

  outcome = itl_g_idle_callback(itl_g_serialized_line,
                                itl_le_cursor_byte_offset(le));
  itl_g_serialized_line_ready = was_serialized;

  if ((outcome & TL_IDLE_AGAIN) != 0) {
    itl_g_idle_due_ms = itl_monotonic_ms() + (uint64_t) itl_g_idle_repeat_ms;
  }
  if ((outcome & TL_IDLE_REFRESH) != 0) {
    itl_g_tty_should_refresh_text = false;
    itl_le_tty_refresh(le);
    itl_g_tty_should_refresh_text = was_text_refresh;
  }
}

/* Wait until a key is pending, redrawing for a resize or a wake report and
   calling the idle hook when a pause reaches its delay. Returns false on an
   error. */
ITL_DEF bool itl_le_wait_for_key(itl_le_t *le)
{
#if defined ITL_POSIX
  for (;;) {
    sigset_t previous_signals;
    bool is_idle_due = false;

    if (!itl_block_input_wake_signals(&previous_signals)) {
      return false;
    }

    for (;;) {
      int wait_result;

      if (itl_g_tty_changed_size) {
        itl_g_tty_should_refresh_text = true;
        itl_le_tty_refresh(le);
      }
      itl_refresh_after_wake(le);
      if (itl_input_is_pending()) {
        break;
      }
      wait_result =
          itl_wait_for_input_until(&previous_signals, itl_idle_wait_ms());
      if (wait_result < 0) {
        itl_restore_input_wake_signals(&previous_signals);
        return false;
      }
      if (wait_result == 0) {
        is_idle_due = true;
        break;
      }
    }

    if (!itl_restore_input_wake_signals(&previous_signals)) {
      return false;
    }
    if (!is_idle_due) {
      return true;
    }
    itl_idle_run(le);
  }
#else  /* ITL_POSIX */
  /* The console raises no resize signal, so the wait blocks on the input
     handle and polls the size on its timeout, redrawing live when it
     changes, the same shape as the POSIX branch above. */
  for (;;) {
    if (itl_g_tty_changed_size) {
      itl_g_tty_should_refresh_text = true;
      itl_le_tty_refresh(le);
    }
    if (itl_input_is_pending()) {
      return true;
    }
    if (itl_wait_for_input_until(itl_idle_wait_ms()) == 0) {
      itl_idle_run(le);
    }
  }
#endif /* ITL_POSIX */
}

/* Open a prefix and wait for the key that completes it. The hint rows name
   the prefix while the editor waits, and a key already pending is read at once
   without drawing it. The prefix is closed again before this returns, so the
   next frame drops its hint. Returns false on an error. */
ITL_DEF bool itl_le_await_chord(itl_le_t *le, itl_prefix_kind kind, uint8_t key)
{
  bool is_ready;

  itl_g_prefix_kind = kind;
  itl_g_prefix_key = key;

  if (!itl_input_is_pending()) {
    itl_g_tty_plain_append_pending = false;
    itl_g_tty_should_refresh_text = true;
    itl_le_tty_refresh(le);
    itl_idle_arm();
  }
  is_ready = itl_le_wait_for_key(le);

  itl_g_prefix_kind = ITL_PREFIX_NONE;
  itl_g_prefix_key = 0;

  return is_ready;
}

/* Refills the menu candidates for the line as it stands now, returning false
   when the source has nothing to offer, which leaves the result untouched. The
   menu takes one of these so a second list, such as history, drives the same
   loop the completion candidates do. */
typedef bool (*itl_menu_gather_fn)(itl_le_t *le, tl_completion *result);

/* Everything that separates one menu source from another. gather refills the
   list as the line changes. can_descend belongs to a list of paths and reopens
   the menu inside an accepted directory. should_regather_new_words belongs to
   a list whose meaning changes at word boundaries, and the source is asked
   again for the first byte of a word and for a byte that moves the token.
   should_show_loading belongs to a source whose gather may take noticeable
   time. should_submit_on_enter belongs to a list that only extends the line,
   and Enter then closes the menu and submits what the line already holds.
   should_highlight belongs to a list whose entries are whole commands, and
   the host colors them the way it colors the line. help_title names the
   source on the first row and help_keys lists the keys it answers beside it.
   should_anchor_to_token belongs to a list of replacements for a token, and
   the rows then start under the token instead of the leftmost column.
   should_keep_best_tier belongs to a completion host. Typing narrows a list
   the host marked as tier ranked by the same tiers, and any other list by
   prefix. */
typedef struct itl_menu_source
{
  itl_menu_gather_fn gather;
  bool can_descend;
  bool should_regather_new_words;
  bool should_show_loading;
  bool should_submit_on_enter;
  bool should_highlight;
  bool restore_on_escape;
  const char *help_title;
  const char *help_keys;
  bool should_anchor_to_token;
  bool should_keep_best_tier;
} itl_menu_source;

/* Ask the host for the candidates of the line as it stands now. The host keeps
   its storage valid until the next call. A fresh result replaces the one the
   menu held and the previous candidates are dropped. */
ITL_DEF bool itl_menu_regather(itl_le_t *le, tl_completion *result)
{
  char line_cstr[ITL_STRING_MAX_LEN];
  tl_completion fresh;

  memset(&fresh, 0, sizeof(fresh));

  if (itl_g_complete_callback == NULL) {
    return false;
  }
  if (itl_string_to_cstr(le->line, line_cstr, sizeof(line_cstr)) != TL_SUCCESS)
  {
    return false;
  }
  if (!itl_g_complete_callback(line_cstr, le->cursor_position, &fresh, 1)) {
    return false;
  }
  if (fresh.count == 0) {
    return false;
  }

  *result = fresh;
  return true;
}

/* The history rows the selector may list, and the pool that holds their bytes
   back to back. One static block keeps a gather free of allocation, and both
   ceilings bound what a long history can put on screen. */
#define ITL_HISTORY_MENU_MAX_ENTRIES 128
#define ITL_HISTORY_MENU_POOL_SIZE   16384

ITL_DEF ITL_THREAD_LOCAL char
    itl_g_history_menu_pool[ITL_HISTORY_MENU_POOL_SIZE];
ITL_DEF ITL_THREAD_LOCAL const char
    *itl_g_history_menu_entries[ITL_HISTORY_MENU_MAX_ENTRIES];

/* The same rows in the order the menu lists them, and the rank each row was
   found with. The scan runs newest first and the order is settled after it. */
ITL_DEF ITL_THREAD_LOCAL const char
    *itl_g_history_menu_ordered[ITL_HISTORY_MENU_MAX_ENTRIES];
ITL_DEF ITL_THREAD_LOCAL unsigned char
    itl_g_history_menu_ranks[ITL_HISTORY_MENU_MAX_ENTRIES];

/* True when needle appears in haystack at or after start, comparing ASCII
   letters without case. An empty needle matches every entry. */
ITL_DEF bool itl_ascii_contains_casefold(const char *haystack,
                                         size_t haystack_len,
                                         const char *needle, size_t needle_len,
                                         size_t start)
{
  if (needle_len == 0) {
    return true;
  }
  if (needle_len > haystack_len) {
    return false;
  }

  for (; start + needle_len <= haystack_len; ++start) {
    if (itl_ascii_prefix_matches_casefold(haystack + start, needle, needle_len))
    {
      return true;
    }
  }

  return false;
}

/* True when every byte of needle appears in haystack in order, comparing ASCII
   letters without case. The bytes between the matched ones are free. An empty
   needle matches every entry. */
ITL_DEF bool itl_ascii_subsequence_casefold(const char *haystack,
                                            size_t haystack_len,
                                            const char *needle,
                                            size_t needle_len)
{
  size_t taken = 0;
  size_t position;

  for (position = 0; position < haystack_len && taken < needle_len; ++position)
  {
    if (itl_ascii_fold_byte((unsigned char) haystack[position]) ==
        itl_ascii_fold_byte((unsigned char) needle[taken]))
    {
      taken += 1;
    }
  }

  return taken == needle_len;
}

/* How well an entry answers the query. A smaller rank draws earlier. */
#define ITL_MENU_RANK_PREFIX      0
#define ITL_MENU_RANK_CONTAINS    1
#define ITL_MENU_RANK_SUBSEQUENCE 2
#define ITL_MENU_RANK_NONE        3

/* Rank one entry against the query by where the query sits inside it. An entry
   opening with the query comes first, one holding it whole comes next, and one
   whose bytes merely appear in order comes last. An empty query ranks every
   entry first. */
ITL_DEF unsigned itl_menu_match_rank(const char *entry, size_t entry_len,
                                     const char *query, size_t query_len)
{
  if (query_len == 0) {
    return ITL_MENU_RANK_PREFIX;
  }
  if (query_len > entry_len) {
    return ITL_MENU_RANK_NONE;
  }
  if (itl_ascii_prefix_matches_casefold(entry, query, query_len)) {
    return ITL_MENU_RANK_PREFIX;
  }
  if (itl_ascii_contains_casefold(entry, entry_len, query, query_len, 1)) {
    return ITL_MENU_RANK_CONTAINS;
  }
  if (itl_ascii_subsequence_casefold(entry, entry_len, query, query_len)) {
    return ITL_MENU_RANK_SUBSEQUENCE;
  }

  return ITL_MENU_RANK_NONE;
}

/* Fill the menu candidates with the history entries the line matches, best
   match first and newest first inside each group. The line is the search
   query. The whole of it is replaced when a row is accepted. An entry already
   gathered is dropped. A command run many times takes one row. */
ITL_DEF bool itl_history_menu_gather(itl_le_t *le, tl_completion *result)
{
  char query[ITL_STRING_MAX_LEN];
  char entry[ITL_STRING_MAX_LEN + 1];
  size_t query_len;
  size_t pool_used = 0;
  size_t found_count = 0;
  size_t placed_count = 0;
  size_t history_count = itl_history_search_count();
  size_t index;
  unsigned rank;

  if (history_count == 0 ||
      (!itl_g_history_search_snapshot.is_active && itl_g_history_path == NULL))
  {
    return false;
  }
  if (itl_string_to_cstr(le->line, query, sizeof(query)) != TL_SUCCESS) {
    return false;
  }
  if (!itl_history_search_prepare()) {
    return false;
  }

  query_len = le->line->size;

  for (index = history_count; index-- > 0;) {
    size_t entry_len;
    size_t gathered;
    unsigned entry_rank;
    bool is_repeat = false;

    if (found_count >= ITL_HISTORY_MENU_MAX_ENTRIES) {
      break;
    }

    if (!itl_history_search_decode_entry(
            itl_history_search_index_to_offset(index), entry, sizeof(entry),
            &entry_len))
    {
      continue;
    }
    if (entry_len == 0) {
      continue;
    }

    entry_rank = itl_menu_match_rank(entry, entry_len, query, query_len);
    if (entry_rank >= ITL_MENU_RANK_NONE) {
      continue;
    }
    if (pool_used + entry_len + 1 > sizeof(itl_g_history_menu_pool)) {
      break;
    }

    for (gathered = 0; gathered < found_count; ++gathered) {
      if (strcmp(itl_g_history_menu_entries[gathered], entry) == 0) {
        is_repeat = true;
        break;
      }
    }

    if (is_repeat) {
      continue;
    }

    memcpy(itl_g_history_menu_pool + pool_used, entry, entry_len + 1);
    itl_g_history_menu_entries[found_count] =
        itl_g_history_menu_pool + pool_used;
    itl_g_history_menu_ranks[found_count] = (unsigned char) entry_rank;
    pool_used += entry_len + 1;
    found_count += 1;
  }

  if (found_count == 0) {
    return false;
  }

  /* The rows the query opens come first, then the ones holding it whole, then
     the ones whose bytes merely appear in order. Each group keeps the newest
     first order the scan gave it. */
  for (rank = 0; rank < ITL_MENU_RANK_NONE; ++rank) {
    for (index = 0; index < found_count; ++index) {
      if (itl_g_history_menu_ranks[index] == rank) {
        itl_g_history_menu_ordered[placed_count] =
            itl_g_history_menu_entries[index];
        placed_count += 1;
      }
    }
  }

  result->candidates = itl_g_history_menu_ordered;
  result->count = placed_count;
  result->descriptions = NULL;
  result->longest_common_prefix = NULL;
  result->token_start = 0;
  result->token_end = le->line->length;

  return true;
}

/* Show the highlighted candidate as ghost text on the line the menu opened on.
   The line above the rows reads as the line that accepting it produces. The
   ghost only appends. A candidate that does not extend the typed token leaves
   the line bare. */
/* True when name opens with the typed token in either case, so the ghost of
   its rest reads as the name. A row matched elsewhere in its text, such as a
   subsequence, would draw the typed bytes followed by a suffix that does not
   continue them. token_start is a codepoint index. */
ITL_DEF bool itl_menu_name_extends_token(const itl_le_t *le,
                                         const char *line_cstr,
                                         size_t token_start, const char *name)
{
  size_t token_start_byte = 0;
  size_t typed_bytes;
  size_t i;

  if (token_start > le->line->length) {
    return false;
  }
  for (i = 0; i < token_start; ++i) {
    token_start_byte += le->line->chars[i].size;
  }
  typed_bytes = le->line->size - token_start_byte;

  return strlen(name) >= typed_bytes &&
         itl_ascii_prefix_matches_casefold(name, line_cstr + token_start_byte,
                                           typed_bytes);
}

ITL_DEF void itl_menu_ghost_preview(itl_le_t *le, const tl_completion *result,
                                    size_t selected)
{
  char display_name_storage[ITL_STRING_MAX_LEN + 1];
  char line_cstr[ITL_STRING_MAX_LEN];
  const char *name;

  itl_ghost_clear();

  if (!itl_g_ghost_enabled || selected >= result->count) {
    return;
  }
  if (le->cursor_position != le->line->length) {
    return;
  }
  if (itl_string_to_cstr(le->line, line_cstr, sizeof(line_cstr)) != TL_SUCCESS) {
    return;
  }

  name = itl_menu_display_name(result->candidates[selected],
                               display_name_storage,
                               sizeof(display_name_storage));
  if (!itl_menu_name_extends_token(le, line_cstr, result->token_start, name)) {
    return;
  }
  itl_ghost_fill_from_token_text(le, line_cstr, le->line->size,
                                 result->token_start,
                                 name);
  itl_g_ghost_width = itl_visible_width(itl_g_ghost, itl_g_ghost_len);
}

/* Drop every candidate while the menu stays open. A failed gather has already
   released the storage the host lent for the previous list. The pointers go
   with the count. The token span is kept, and a backspace still reaches the
   character that narrowed the list away. */
ITL_DEF void itl_menu_empty_candidates(tl_completion *result)
{
  result->candidates = NULL;
  result->count = 0;
  result->descriptions = NULL;
  result->longest_common_prefix = NULL;
}

/* The entries one narrowing reads, and the rows it may keep. A source hands the
   menu its whole list once. Every key that follows is answered from that list,
   and it costs a scan of these bounds and no work from the host. */
#define ITL_MENU_FILTER_SCAN_MAX 4096
#define ITL_MENU_FILTER_MAX      512

ITL_DEF ITL_THREAD_LOCAL const char *itl_g_menu_filtered[ITL_MENU_FILTER_MAX];
ITL_DEF ITL_THREAD_LOCAL const char
    *itl_g_menu_filtered_descriptions[ITL_MENU_FILTER_MAX];
ITL_DEF ITL_THREAD_LOCAL unsigned char
    itl_g_menu_ranks[ITL_MENU_FILTER_SCAN_MAX];

/* The list the source last gave and the query it answered. Typing narrows this
   list in place. The source is asked again only when the line no longer
   extends the query, or when the local list has no row for it. base_tier is
   the best tier any base row reaches against that query. */
typedef struct itl_menu_filter_state
{
  tl_completion base;
  char query[ITL_STRING_MAX_LEN];
  size_t query_len;
  size_t name_width;
  unsigned base_tier;
  bool should_regather;
} itl_menu_filter_state;

/* The tiers a shell completion host ranks its candidates by, best first. It
   offers only the rows of the best tier any entry reaches. */
#define ITL_MENU_TIER_EXACT_PREFIX 0
#define ITL_MENU_TIER_PREFIX       1
#define ITL_MENU_TIER_SUBSEQUENCE  2
#define ITL_MENU_TIER_NONE         3

/* True when the query holds an ASCII capital, which makes every tier compare
   case sensitively. */
ITL_DEF bool itl_menu_query_is_case_sensitive(const char *query,
                                              size_t query_len)
{
  size_t position;

  for (position = 0; position < query_len; ++position) {
    if (query[position] >= 'A' && query[position] <= 'Z') {
      return true;
    }
  }

  return false;
}

/* True when a query may match as a subsequence. A single byte or one opening
   with a byte such as an option dash would match almost every entry. */
ITL_DEF bool itl_menu_query_allows_subsequence(const char *query,
                                               size_t query_len)
{
  uint8_t first;

  if (query_len < 2) {
    return false;
  }

  first = (uint8_t) query[0];

  return (first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z') ||
         (first >= '0' && first <= '9') || first == '_';
}

/* Rank one entry against the query the way a completion host does. An entry
   opening with the query byte for byte comes first. One opening with it in
   another case comes next, unless the query holds a capital. One holding the
   query bytes in order comes last, for a query that allows it. */
ITL_DEF unsigned itl_menu_tier_rank(const char *entry, size_t entry_len,
                                    const char *query, size_t query_len,
                                    bool is_case_sensitive)
{
  size_t taken = 0;
  size_t position;

  if (query_len > entry_len) {
    return ITL_MENU_TIER_NONE;
  }
  if (memcmp(entry, query, query_len) == 0) {
    return ITL_MENU_TIER_EXACT_PREFIX;
  }
  if (!is_case_sensitive &&
      itl_ascii_prefix_matches_casefold(entry, query, query_len))
  {
    return ITL_MENU_TIER_PREFIX;
  }
  if (!itl_menu_query_allows_subsequence(query, query_len)) {
    return ITL_MENU_TIER_NONE;
  }
  if (!is_case_sensitive) {
    return itl_ascii_subsequence_casefold(entry, entry_len, query, query_len)
               ? ITL_MENU_TIER_SUBSEQUENCE
               : ITL_MENU_TIER_NONE;
  }

  for (position = 0; position < entry_len && taken < query_len; ++position) {
    if (entry[position] == query[taken]) {
      taken += 1;
    }
  }

  return taken == query_len ? ITL_MENU_TIER_SUBSEQUENCE : ITL_MENU_TIER_NONE;
}

/* The best tier any of the first rows reaches against the query. */
ITL_DEF unsigned itl_menu_best_tier(const tl_completion *base,
                                    size_t scanned_count, const char *query,
                                    size_t query_len)
{
  bool is_case_sensitive = itl_menu_query_is_case_sensitive(query, query_len);
  unsigned best = ITL_MENU_TIER_NONE;
  size_t index;

  for (index = 0; index < scanned_count && best > 0; ++index) {
    const char *entry = base->candidates[index];
    unsigned tier = itl_menu_tier_rank(entry, strlen(entry), query, query_len,
                                       is_case_sensitive);

    if (tier < best) {
      best = tier;
    }
  }

  return best;
}

/* Copy the token bytes a candidate replaces into out. The span is given in
   codepoints and the line is walked once. Returns false when the span is off
   the line or the bytes do not fit. */
ITL_DEF bool itl_menu_query_text(itl_le_t *le, const tl_completion *result,
                                 char *out, size_t out_size, size_t *out_len)
{
  size_t token_end = result->token_end;
  size_t position;
  size_t used = 0;

  if (out_size == 0 || result->token_start > le->line->length) {
    return false;
  }
  if (token_end > le->line->length) {
    token_end = le->line->length;
  }
  if (token_end < result->token_start) {
    return false;
  }

  for (position = result->token_start; position < token_end; ++position) {
    itl_utf8_t ch = le->line->chars[position];

    if (used + ch.size >= out_size) {
      return false;
    }

    memcpy(out + used, ch.bytes, ch.size);
    used += ch.size;
  }

  out[used] = '\0';
  *out_len = used;

  return true;
}

/* Hand the menu the scanned base rows whose rank in itl_g_menu_ranks lies from
   first_rank up to but not including end_rank. The groups are drawn best rank
   first and each group keeps the order the base gave it. The token span of
   the result is left alone. Returns false when no row is kept. */
ITL_DEF bool itl_menu_keep_ranks(const tl_completion *base,
                                 size_t scanned_count, unsigned first_rank,
                                 unsigned end_rank, tl_completion *result)
{
  size_t kept_count = 0;
  size_t index;
  unsigned rank;

  for (rank = first_rank; rank < end_rank && kept_count < ITL_MENU_FILTER_MAX;
       ++rank)
  {
    for (index = 0; index < scanned_count && kept_count < ITL_MENU_FILTER_MAX;
         ++index)
    {
      if (itl_g_menu_ranks[index] != rank) {
        continue;
      }

      itl_g_menu_filtered[kept_count] = base->candidates[index];
      itl_g_menu_filtered_descriptions[kept_count] =
          base->descriptions != NULL ? base->descriptions[index] : NULL;
      kept_count += 1;
    }
  }

  if (kept_count == 0) {
    return false;
  }

  result->candidates = itl_g_menu_filtered;
  result->descriptions =
      base->descriptions != NULL ? itl_g_menu_filtered_descriptions : NULL;
  result->longest_common_prefix = NULL;
  result->count = kept_count;

  return true;
}

/* Narrow the base list to the entries the query matches and hand the rows to
   the menu. The groups are drawn best match first and each group keeps the
   order the base gave it. The token span of the result is left alone. Returns
   false when nothing matches. */
ITL_DEF bool itl_menu_filter(const tl_completion *base, const char *query,
                             size_t query_len, tl_completion *result)
{
  size_t scanned_count = base->count < ITL_MENU_FILTER_SCAN_MAX
                             ? base->count
                             : ITL_MENU_FILTER_SCAN_MAX;
  size_t index;

  for (index = 0; index < scanned_count; ++index) {
    const char *entry = base->candidates[index];

    itl_g_menu_ranks[index] = (unsigned char) itl_menu_match_rank(
        entry, strlen(entry), query, query_len);
  }

  return itl_menu_keep_ranks(base, scanned_count, 0, ITL_MENU_RANK_NONE,
                             result);
}

/* Narrow the base list the way a completion host answers the query, keeping
   only the rows of the best tier. The host offered only the rows of
   base_tier for its own query, so a narrowing whose best tier differs would
   miss rows the base never held. Returns false then and when nothing
   matches. */
ITL_DEF bool itl_menu_filter_tier(const tl_completion *base, const char *query,
                                  size_t query_len, unsigned base_tier,
                                  tl_completion *result)
{
  size_t scanned_count = base->count < ITL_MENU_FILTER_SCAN_MAX
                             ? base->count
                             : ITL_MENU_FILTER_SCAN_MAX;
  bool is_case_sensitive = itl_menu_query_is_case_sensitive(query, query_len);
  unsigned best = ITL_MENU_TIER_NONE;
  size_t index;

  for (index = 0; index < scanned_count; ++index) {
    const char *entry = base->candidates[index];
    unsigned tier = itl_menu_tier_rank(entry, strlen(entry), query, query_len,
                                       is_case_sensitive);

    itl_g_menu_ranks[index] = (unsigned char) tier;
    if (tier < best) {
      best = tier;
    }
  }

  if (best == ITL_MENU_TIER_NONE || best != base_tier) {
    return false;
  }

  return itl_menu_keep_ranks(base, scanned_count, best, best + 1, result);
}

/* Narrow a list the host did not rank to the entries that open with the query
   in either case, in the order the base gave them. Every such row stays, since
   the host may offer any of them for the longer query. Returns false when
   nothing matches. */
ITL_DEF bool itl_menu_filter_prefix(const tl_completion *base,
                                    const char *query, size_t query_len,
                                    tl_completion *result)
{
  size_t scanned_count = base->count < ITL_MENU_FILTER_SCAN_MAX
                             ? base->count
                             : ITL_MENU_FILTER_SCAN_MAX;
  size_t index;

  for (index = 0; index < scanned_count; ++index) {
    const char *entry = base->candidates[index];
    bool is_prefix = strlen(entry) >= query_len &&
                     itl_ascii_prefix_matches_casefold(entry, query, query_len);

    itl_g_menu_ranks[index] =
        (unsigned char) (is_prefix ? ITL_MENU_RANK_PREFIX : ITL_MENU_RANK_NONE);
  }

  return itl_menu_keep_ranks(base, scanned_count, ITL_MENU_RANK_PREFIX,
                             ITL_MENU_RANK_PREFIX + 1, result);
}

/* Take the list the source just gave as the base the narrowing reads, together
   with the query it answered. A token the line cannot hand back leaves an
   empty base and the next key reaches the source. */
ITL_DEF void itl_menu_adopt_base(itl_le_t *le, itl_menu_filter_state *state,
                                 const tl_completion *result)
{
  state->base = *result;
  state->name_width = itl_menu_name_width(result);
  state->should_regather = false;

  if (!itl_menu_query_text(le, result, state->query, sizeof(state->query),
                           &state->query_len))
  {
    state->query[0] = '\0';
    state->query_len = 0;
    state->base.count = 0;
  }

  state->base_tier = itl_menu_best_tier(
      &state->base,
      state->base.count < ITL_MENU_FILTER_SCAN_MAX ? state->base.count
                                                   : ITL_MENU_FILTER_SCAN_MAX,
      state->query, state->query_len);
}

/* Ask the source for the line as it stands and adopt what it gives. Returns
   false when the source has nothing, and the base is emptied. */
ITL_DEF bool itl_menu_rebase(itl_le_t *le, const itl_menu_source *source,
                             itl_menu_filter_state *state,
                             tl_completion *result)
{
  if (source->should_show_loading) {
    tl_completion loading = ITL_ZERO_INIT;
    size_t tty_rows = itl_g_tty_prev_rows > 0 ? itl_g_tty_prev_rows : 24;
    itl_menu_layout layout;

    itl_g_tty_should_refresh_text = true;
    itl_le_tty_refresh(le);
    itl_g_menu_anchor_column = 0;
    layout = itl_menu_measure_for(&loading, tty_rows, 0, ITL_MENU_LOADING_TEXT,
                                  source->help_title, source->help_keys);
    itl_menu_draw(&loading, 0, 0, layout, source->help_title,
                  source->help_keys, source->should_highlight, 0,
                  ITL_MENU_LOADING_TEXT);
    itl_terminal_drain_output();
  }

  if (!source->gather(le, result)) {
    if (!itl_menu_query_text(le, result, state->query,
                             sizeof(state->query), &state->query_len))
    {
      state->query[0] = '\0';
      state->query_len = 0;
    }
    state->base.count = 0;
    state->name_width = 0;
    state->should_regather = false;

    return false;
  }

  itl_menu_adopt_base(le, state, result);

  return true;
}

/* Narrow the base list the way its source answers a longer query. A host that
   ranked the list by tiers keeps the best one, a host that did not keeps every
   row the query opens, and any other source ranks by where the query sits. */
ITL_DEF bool itl_menu_filter_for(const itl_menu_source *source,
                                 const itl_menu_filter_state *state,
                                 const char *query, size_t query_len,
                                 tl_completion *result)
{
  if (!source->should_keep_best_tier) {
    return itl_menu_filter(&state->base, query, query_len, result);
  }
  if (state->base.is_tier_ranked) {
    return itl_menu_filter_tier(&state->base, query, query_len,
                                state->base_tier, result);
  }

  return itl_menu_filter_prefix(&state->base, query, query_len, result);
}

/* Answer the line as it stands from the base list, and fall back to the source
   when the base cannot answer. The base holds every row the source offered for
   a shorter query. A line that only grew is narrowed without touching the host,
   and an erase that keeps the gathered query widens it the same way. An erase
   below that query, a line with no local match, a source that keeps the best
   tier when the local best tier is not the one it answered with, and a base
   longer than one scan reaches all go to the source. Returns false when
   neither has a row. */
ITL_DEF bool itl_menu_narrow(itl_le_t *le, const itl_menu_source *source,
                             itl_menu_filter_state *state,
                             tl_completion *result)
{
  char query[ITL_STRING_MAX_LEN];
  size_t query_len = 0;

  if (state->should_regather) {
    return itl_menu_rebase(le, source, state, result);
  }

  if (itl_menu_query_text(le, result, query, sizeof(query), &query_len) &&
      query_len >= state->query_len &&
      itl_ascii_prefix_matches_casefold(query, state->query, state->query_len))
  {
    if (state->base.count == 0) {
      return false;
    }
    if (state->base.count <= ITL_MENU_FILTER_SCAN_MAX &&
        itl_menu_filter_for(source, state, query, query_len, result))
    {
      state->name_width = itl_menu_name_width(result);

      return true;
    }
  }

  return itl_menu_rebase(le, source, state, result);
}

/* True when the next key starts a word or, in a list of paths, a component
   after a separator. Its first byte selects what the source lists, such as a
   dash for flags, a dollar for variables, or a dot for hidden files, so the
   list gathered before it cannot answer it. The caret sits at the token end
   while the menu is open. */
ITL_DEF bool itl_menu_opens_component(const itl_le_t *le,
                                      const tl_completion *result,
                                      bool can_descend)
{
  itl_utf8_t previous;

  if (result->token_end <= result->token_start || le->cursor_position == 0) {
    return true;
  }

  previous = le->line->chars[le->cursor_position - 1];

  return can_descend && previous.size == 1 &&
         itl_byte_is_path_separator(previous.bytes[0]);
}

/* True when a typed byte moves where the token starts. A blank opens the next
   word, a quote changes how the token is read, and an equals sign opens the
   value of an assignment or a flag. The source then lists a different span. */
ITL_DEF bool itl_menu_byte_moves_token(uint8_t byte)
{
  return ITL_CHAR_IS_SPACE(byte) || byte == '\'' || byte == '"' ||
         byte == '=';
}

/* Run the candidate menu until the user accepts a candidate, dismisses it, or
   presses a key the menu does not own. The down arrow steps forward, the up
   arrow and shift tab step back, Tab accepts and closes, Right accepts and
   continues completing. Escape closes the menu with the current edit, while
   ctrl-g cancels and puts back the line the menu opened on. A printable key
   and backspace narrow and widen the list in place. A search that
   matches nothing keeps the menu open on a row that says so, and a backspace
   brings the list back. Right keeps the menu open while the host still offers
   candidates. Any other key closes the menu and then does its own work on the
   line. The returned status is the one the caller must return, and it carries a
   terminating key back to the host. The source holds everything that separates
   one candidate list from another. */
ITL_DEF tl_status_code itl_completion_menu_run(itl_le_t *le,
                                               const tl_completion *initial,
                                               const itl_menu_source *source)
{
  tl_completion result = *initial;
  itl_menu_filter_state state;
  size_t selected = 0;
  size_t window_start = 0;

  /* The row the ghost was filled from. It starts outside the candidate range so
     the first pass fills the preview, and a regather puts it back there. */
  size_t previewed = (size_t) -1;

  /* Typing and walking into a directory both edit the line. The line as it
     stands is kept for ctrl-g to restore. A line too long for the buffer keeps
     no copy and cancels in place. */
  char original_line[ITL_STRING_MAX_LEN];
  size_t original_line_size = le->line->size;
  size_t original_cursor = le->cursor_position;
  bool has_original_line =
      itl_string_to_cstr(le->line, original_line, sizeof(original_line)) ==
      TL_SUCCESS;

  itl_menu_adopt_base(le, &state, &result);
  itl_g_tty_should_refresh_text = true;

  for (;;) {
    size_t tty_rows;
    size_t name_width;
    itl_menu_layout layout;
    uint8_t byte;
    int key, kind;
    bool is_escape;
    bool should_continue_completion;

    /* A resize invalidates the block the rows are measured against. The line is
       repainted first and the new size feeds the layout. */
    if (itl_g_tty_changed_size != 0) {
      itl_g_tty_should_refresh_text = true;
      itl_le_tty_refresh(le);
    }

    /* The ghost follows the highlighted row. The preview is refilled and the
       line above the rows is repainted whenever the highlight moves. The first
       pass also takes the ghost of the typed line off the screen. */
    if (previewed != selected) {
      itl_menu_ghost_preview(le, &result, selected);
      previewed = selected;
      itl_g_tty_should_refresh_text = true;
      itl_le_tty_refresh(le);
    }

    tty_rows = itl_g_tty_prev_rows > 0 ? itl_g_tty_prev_rows : 24;
    itl_g_menu_anchor_column =
        source->should_anchor_to_token
            ? itl_menu_anchor_column_of(le, result.token_start)
            : 0;
    itl_g_menu_name_skip = source->should_anchor_to_token
                               ? itl_menu_common_directory_size(&result)
                               : 0;
    name_width = itl_g_menu_name_skip > 0 ? itl_menu_name_width(&result)
                                          : state.name_width;
    layout = itl_menu_measure_for(&result, tty_rows, name_width,
                                  ITL_MENU_EMPTY_TEXT, source->help_title,
                                  source->help_keys);

    window_start = itl_menu_window_start(result.count, selected, window_start,
                                         layout.candidate_rows);
    itl_menu_draw(&result, selected, window_start, layout, source->help_title,
                  source->help_keys, source->should_highlight, name_width,
                  ITL_MENU_EMPTY_TEXT);
    itl_g_menu_name_skip = 0;

#if defined ITL_POSIX
    {
      sigset_t previous_signals;
      bool should_redraw = false;

      if (!itl_block_input_wake_signals(&previous_signals)) {
        return TL_ERROR;
      }

      for (;;) {
        if (itl_g_tty_changed_size != 0) {
          should_redraw = true;
          break;
        }
        if (itl_refresh_after_wake(le)) {
          should_redraw = true;
          break;
        }
        if (itl_input_is_pending()) {
          break;
        }
        if (!itl_wait_for_input(&previous_signals)) {
          itl_restore_input_wake_signals(&previous_signals);
          return TL_ERROR;
        }
      }

      if (!itl_restore_input_wake_signals(&previous_signals)) {
        return TL_ERROR;
      }
      if (should_redraw) {
        continue;
      }
    }
#else  /* ITL_POSIX */
    while (!itl_input_is_pending()) {
      itl_wait_for_input();
    }
#endif /* ITL_POSIX */

    if (!ITL_READ_BYTE(&byte)) {
      break;
    }

    is_escape = byte == 27;
    key = itl_esc_parse(byte);
    kind = key & TL_MASK_KEY;
    should_continue_completion =
        kind == TL_KEY_RIGHT &&
        (key & (TL_MOD_CTRL | TL_MOD_SHIFT | TL_MOD_ALT)) == 0 &&
        source->should_submit_on_enter;

    if (kind == TL_KEY_DOWN) {
      if (result.count > 0) {
        selected = selected + 1 < result.count ? selected + 1 : 0;
      }

      continue;
    }

    if (kind == TL_KEY_UP ||
        (kind == TL_KEY_TAB && (key & TL_MOD_SHIFT) != 0))
    {
      if (result.count > 0) {
        selected = selected > 0 ? selected - 1 : result.count - 1;
      }

      continue;
    }

    /* An empty list has no row to accept. The keys that would take one are
       swallowed and the menu waits for the erase that brings the list back. */
    if (result.count == 0 &&
        (kind == TL_KEY_TAB ||
         (kind == TL_KEY_ENTER && !source->should_submit_on_enter)))
    {
      continue;
    }

    itl_menu_erase();
    itl_g_tty_should_refresh_text = true;
    /* The preview belongs to the rows that were just erased. No path out of
       the menu keeps it on the line. */
    itl_ghost_clear();

    if (kind == TL_KEY_TAB || should_continue_completion ||
        (kind == TL_KEY_ENTER && !source->should_submit_on_enter))
    {
      const char *candidate = result.candidates[selected];
      size_t candidate_length = strlen(candidate);
      bool should_descend =
          source->can_descend && candidate_length > 0 &&
          itl_g_space_after_completion != TL_SPACE_AFTER_COMPLETION_ON &&
          itl_byte_is_path_separator(
              (uint8_t) candidate[candidate_length - 1]);

      if (!itl_completion_replace_token(le, &result, candidate)) {
        return TL_SUCCESS;
      }

      if ((should_continue_completion || should_descend) &&
          itl_menu_rebase(le, source, &state, &result))
      {
        selected = 0;
        window_start = 0;
        previewed = (size_t) -1;
        continue;
      }

      if (should_descend) {
        itl_menu_erase();
        itl_g_tty_should_refresh_text = true;
        return TL_SUCCESS;
      }

      if (kind == TL_KEY_TAB && !result.is_space_suppressed)
        itl_completion_append_space(le);

      return TL_SUCCESS;
    }

    if (kind == TL_KEY_UNKN) {
      if ((!is_escape || source->restore_on_escape) && has_original_line) {
        itl_string_from_bytes(le->line, original_line, original_line_size);
        le->cursor_position = original_cursor <= le->line->length
                                  ? original_cursor
                                  : le->line->length;
        itl_le_tty_refresh(le);
      }

      return TL_SUCCESS;
    }

    if (kind == TL_KEY_CHAR) {
      bool should_regather =
          source->should_regather_new_words &&
          (itl_menu_opens_component(le, &result, source->can_descend) ||
           itl_menu_byte_moves_token(byte));

      itl_le_insert(le, itl_utf8_parse(byte));
      result.token_end += 1;

      if (should_regather ||
          (source->can_descend && itl_byte_is_path_separator(byte)))
      {
        if (!itl_menu_rebase(le, source, &state, &result)) {
          itl_menu_empty_candidates(&result);
        }
      } else if (!itl_menu_narrow(le, source, &state, &result)) {
        itl_menu_empty_candidates(&result);
      }

      selected = 0;
      window_start = 0;
      previewed = (size_t) -1;
      continue;
    }

    if (kind == TL_KEY_BACKSPACE) {
      size_t cursor_before;
      size_t erased_count;
      size_t token_length;
      bool did_cross_token_start;
      tl_status_code erase_code;

      if (le->cursor_position == 0) {
        previewed = (size_t) -1;
        continue;
      }

      cursor_before = le->cursor_position;
      erase_code = itl_le_key_handle(le, key);

      if (erase_code != TL_SUCCESS) {
        return erase_code;
      }

      erased_count = cursor_before - le->cursor_position;
      token_length = result.token_end > result.token_start
                         ? result.token_end - result.token_start
                         : 0;
      did_cross_token_start = cursor_before <= result.token_start ||
                              le->cursor_position < result.token_start;

      if (did_cross_token_start) {
        if (!itl_menu_rebase(le, source, &state, &result)) {
          itl_menu_empty_candidates(&result);
          state.should_regather = true;
        }
      } else {
        result.token_end = erased_count >= token_length
                               ? result.token_start
                               : result.token_end - erased_count;
        if (!itl_menu_narrow(le, source, &state, &result)) {
          itl_menu_empty_candidates(&result);
        }
      }

      selected = 0;
      window_start = 0;
      previewed = (size_t) -1;
      continue;
    }

    itl_undo_close_insert_run();
    return itl_le_key_handle(le, key);
  }

  itl_menu_erase();
  itl_g_tty_should_refresh_text = true;

  return TL_SUCCESS;
}

/* Run the menu with the hint rows held away, so the rows it draws under the
   block never share the screen with it. */
ITL_DEF tl_status_code itl_completion_menu(itl_le_t *le,
                                           const tl_completion *initial,
                                           const itl_menu_source *source)
{
  tl_status_code status;

  itl_hint_hold(le);
  status = itl_completion_menu_run(le, initial, source);
  itl_hint_release();

  return status;
}

/* True when the common prefix that replaced a token leaves the list the host
   gave for that token whole, so the menu can open on it without asking the
   host again. The prefix extends the replaced text byte for byte and every
   candidate opens with it, even when the token was empty: the host may have
   answered an empty line from a list that a word no longer reaches. The bytes
   the prefix adds follow the rules of typing into an open menu: a byte that
   moves the token and a path separator send the menu back to the host. */
ITL_DEF bool itl_completion_prefix_keeps_list(const tl_completion *result,
                                              const char *replaced,
                                              size_t replaced_len)
{
  const char *prefix = result->longest_common_prefix;
  size_t prefix_len;
  size_t position;
  size_t index;

  if (prefix == NULL || result->candidates == NULL) {
    return false;
  }

  prefix_len = strlen(prefix);
  if (prefix_len <= replaced_len || memcmp(prefix, replaced, replaced_len) != 0)
  {
    return false;
  }

  for (position = replaced_len; position < prefix_len; ++position) {
    uint8_t byte = (uint8_t) prefix[position];

    if (itl_menu_byte_moves_token(byte) || itl_byte_is_path_separator(byte)) {
      return false;
    }
  }

  for (index = 0; index < result->count; ++index) {
    if (strncmp(result->candidates[index], prefix, prefix_len) != 0) {
      return false;
    }
  }

  return true;
}

/* Handle the TAB key when a completion callback is registered. Replace the
   whole token under the cursor with the longest common prefix when that prefix
   extends the token, and when several candidates remain and the prefix did not
   grow, present them below the prompt. A single candidate replaces the token
   outright, since it is the one full replacement. Returns true when it handled
   the key, false when the host should fall back to the default TAB behavior.
   out_code receives the status the caller must return, which is TL_SUCCESS
   unless the menu re-dispatched a key that terminates the line. */
ITL_DEF bool itl_completion_handle_tab(itl_le_t *le, tl_status_code *out_code)
{
  static const itl_menu_source completion_source = {
      itl_menu_regather, true, true, true, true, false, false,
      "selecting completions",
      "enter to run, tab to accept, esc to close, ctrl-g to restore", true,
      true};
  char line_cstr[ITL_STRING_MAX_LEN];
  tl_completion result;
  size_t token_len, lcp_len;
  bool is_loading_drawn = false;

  memset(&result, 0, sizeof(result));
  *out_code = TL_SUCCESS;

  if (itl_g_complete_callback == NULL) {
    return false;
  }
  if (itl_string_to_cstr(le->line, line_cstr, sizeof(line_cstr)) != TL_SUCCESS)
  {
    return false;
  }
  if (itl_g_completion_menu_enabled) {
    tl_completion loading = ITL_ZERO_INIT;
    size_t tty_rows = itl_g_tty_prev_rows > 0 ? itl_g_tty_prev_rows : 24;
    static const char loading_title[] = "selecting completions";
    static const char loading_keys[] =
        "enter to run, tab to accept, esc to close, ctrl-g to restore";
    itl_menu_layout layout;

    itl_g_menu_anchor_column = 0;
    layout = itl_menu_measure_for(&loading, tty_rows, 0, ITL_MENU_LOADING_TEXT,
                                  loading_title, loading_keys);
    itl_menu_draw(&loading, 0, 0, layout, loading_title, loading_keys, false, 0,
                  ITL_MENU_LOADING_TEXT);
    itl_terminal_drain_output();
    is_loading_drawn = true;
  }
  int itl_completion_handled =
      itl_g_complete_callback(line_cstr, le->cursor_position, &result, 1);
  itl_ghost_clear();
  itl_g_ghost_sticky_target[0] = '\0';
  if (!itl_completion_handled || result.count == 0) {
    if (is_loading_drawn) {
      itl_menu_erase();
    }
    /* A dumb terminal cannot render the repaint the flash draws. The key is
       still handled and no flash is attempted. */
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

  /* A lone candidate is the full replacement for the token. It goes in even
     when it is no longer than what the user typed. A glob token that resolves
     to a single match reaches this path. The key stops there: the next word
     or the inside of a completed directory is listed by the next TAB. */
  if (result.count == 1) {
    if (is_loading_drawn) {
      itl_menu_erase();
    }
    if (!itl_completion_replace_token(le, &result, result.candidates[0])) {
      return true;
    }
    if (!result.is_space_suppressed) {
      itl_completion_append_space(le);
    }
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
     the token, which grows the token toward the candidates. The menu then
     opens on the list already gathered when the prefix only narrows it, and
     asks the host again otherwise. */
  if (lcp_len > token_len) {
    char replaced[ITL_STRING_MAX_LEN];
    size_t replaced_len = 0;
    bool is_list_kept =
        itl_menu_query_text(le, &result, replaced, sizeof(replaced),
                            &replaced_len) &&
        itl_completion_prefix_keeps_list(&result, replaced, replaced_len);

    if (is_loading_drawn) {
      itl_menu_erase();
    }
    if (!itl_completion_replace_token(le, &result,
                                      result.longest_common_prefix))
    {
      is_list_kept = false;
    }
    itl_g_tty_should_refresh_text = true;
    if (itl_g_completion_menu_enabled) {
      tl_completion remaining = ITL_ZERO_INIT;

      if (is_list_kept) {
        remaining = result;
        remaining.token_end = le->cursor_position;
      } else if (!itl_menu_regather(le, &remaining)) {
        remaining.count = 0;
      }
      if (remaining.count > 1) {
        *out_code = itl_completion_menu(le, &remaining, &completion_source);
      }
    }
    return true;
  }

  /* The prefix did not grow the token. A second TAB presents the candidates.
     The menu owns the keys until it closes. Without it the candidates are
     printed as a static column list. */
  if (itl_g_completion_menu_enabled) {
    *out_code = itl_completion_menu(le, &result, &completion_source);
    return true;
  }

  itl_completion_print_list(
      &result, itl_menu_anchor_column_of(le, result.token_start));
  return true;
}

/* List the history entries that match the line and let the menu keys pick one.
   The line is the search query. Typing narrows the list in place and accepting
   replaces the whole line with the entry. An empty history leaves the line
   alone, and a query nothing matches opens the menu on its no-match row, so
   an erase can still widen the search. Returns the status the caller must
   return. */
ITL_DEF tl_status_code itl_history_menu(itl_le_t *le)
{
  static const itl_menu_source history_source = {
      itl_history_menu_gather, false, false, false, false, true, true,
      "incremental history search",
      "enter/tab to accept, esc/ctrl-g to cancel", false, false};

  tl_completion result;
  char original_line[ITL_STRING_MAX_LEN];
  char resulting_line[ITL_STRING_MAX_LEN];
  size_t original_size = le->line->size;
  bool has_original_line =
      itl_string_to_cstr(le->line, original_line, sizeof(original_line)) ==
      TL_SUCCESS;
  tl_status_code status;

  memset(&result, 0, sizeof(result));

  if (!itl_history_menu_gather(le, &result)) {
    if (le->line->length == 0 || itl_history_search_count() == 0 ||
        (!itl_g_history_search_snapshot.is_active &&
         itl_g_history_path == NULL))
    {
      return TL_SUCCESS;
    }
    itl_menu_empty_candidates(&result);
    result.token_start = 0;
    result.token_end = le->line->length;
  }

  status = itl_completion_menu(le, &result, &history_source);
  if (itl_g_history_search_snapshot.is_active && has_original_line &&
      itl_string_to_cstr(le->line, resulting_line, sizeof(resulting_line)) ==
          TL_SUCCESS &&
      (le->line->size != original_size ||
       memcmp(original_line, resulting_line, original_size) != 0))
  {
    le->history_selected_index = ITL_HISTORY_NONE;
  }

  return status;
}

#define ITL_KILL_RING_SIZE 16

/* The editing command a key performed, so the next key knows whether it
   continues a kill, a yank, or a last-argument walk. */
typedef enum
{
  ITL_LE_ACTION_NONE = 0,
  ITL_LE_ACTION_KILL,
  ITL_LE_ACTION_YANK,
  ITL_LE_ACTION_LAST_ARGUMENT
} itl_le_action_kind;

/* The kill ring keeps the newest kills across lines. kill_ring_newest is the
   slot of the newest entry once kill_ring_count is nonzero. */
ITL_DEF ITL_THREAD_LOCAL itl_string_t *itl_g_kill_ring[ITL_KILL_RING_SIZE] =
    ITL_ZERO_INIT;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_kill_ring_count = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_kill_ring_newest = 0;
/* How many entries back from the newest the yanked text came from. */
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_kill_ring_yank_distance = 0;

ITL_DEF ITL_THREAD_LOCAL itl_le_action_kind itl_g_le_action =
    ITL_LE_ACTION_NONE;
/* The span a yank or a last-argument insertion put into the line, which the
   next Alt-Y or Alt-. replaces. */
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_le_inserted_start = 0;
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_le_inserted_length = 0;
/* How many entries back from the newest the inserted last argument came
   from. */
ITL_DEF ITL_THREAD_LOCAL size_t itl_g_last_argument_distance = 0;

ITL_DEF void itl_kill_ring_free(void)
{
  size_t i;

  for (i = 0; i < ITL_KILL_RING_SIZE; ++i) {
    if (itl_g_kill_ring[i] != NULL) {
      ITL_STRING_FREE(itl_g_kill_ring[i]);
      itl_g_kill_ring[i] = NULL;
    }
  }

  itl_g_kill_ring_count = 0;
  itl_g_kill_ring_newest = 0;
  itl_g_kill_ring_yank_distance = 0;
  itl_g_le_action = ITL_LE_ACTION_NONE;
}

ITL_DEF itl_string_t *itl_kill_ring_at_distance(size_t distance)
{
  TL_ASSERT(distance < itl_g_kill_ring_count);
  return itl_g_kill_ring[(itl_g_kill_ring_newest + ITL_KILL_RING_SIZE -
                          distance) %
                         ITL_KILL_RING_SIZE];
}

/* Copies the span [from, to) of the line into the kill ring. A kill that
   follows another kill grows the newest entry, at its end for a forward kill
   and at its start for a backward one, so the entry reads as the line did. */
ITL_DEF void itl_kill_ring_save(const itl_string_t *line, size_t from,
                                size_t to, bool is_backward,
                                bool should_append)
{
  itl_string_t *entry;
  size_t i;

  if (from >= to) {
    return;
  }

  if (!should_append || itl_g_kill_ring_count == 0) {
    if (itl_g_kill_ring_count > 0) {
      itl_g_kill_ring_newest =
          (itl_g_kill_ring_newest + 1) % ITL_KILL_RING_SIZE;
    }
    if (itl_g_kill_ring_count < ITL_KILL_RING_SIZE) {
      itl_g_kill_ring_count += 1;
    }
    if (itl_g_kill_ring[itl_g_kill_ring_newest] == NULL) {
      itl_g_kill_ring[itl_g_kill_ring_newest] = itl_string_alloc();
    }
    itl_g_kill_ring[itl_g_kill_ring_newest]->length = 0;
    itl_g_kill_ring[itl_g_kill_ring_newest]->size = 0;
  }

  entry = itl_g_kill_ring[itl_g_kill_ring_newest];
  for (i = from; i < to; ++i) {
    itl_string_insert(entry, is_backward ? i - from : entry->length,
                      line->chars[i]);
  }
}

/* Erases count characters next to the cursor and saves them in the kill ring.
   The previous action decides whether the kill grows the newest entry. */
ITL_DEF void itl_le_kill(itl_le_t *le, size_t count, bool backwards,
                         itl_le_action_kind previous_action)
{
  size_t from;
  size_t to;

  if (le->cursor_position > le->line->length) {
    le->cursor_position = le->line->length;
  }

  if (backwards) {
    if (count > le->cursor_position) {
      count = le->cursor_position;
    }
    from = le->cursor_position - count;
    to = le->cursor_position;
  } else {
    if (count > le->line->length - le->cursor_position) {
      count = le->line->length - le->cursor_position;
    }
    from = le->cursor_position;
    to = le->cursor_position + count;
  }

  itl_kill_ring_save(le->line, from, to, backwards,
                     previous_action == ITL_LE_ACTION_KILL);
  itl_le_erase(le, count, backwards);
  itl_g_le_action = ITL_LE_ACTION_KILL;
}

/* Opens one undo step for an edit that is not a typed insertion. */
ITL_DEF void itl_le_begin_edit(itl_le_t *le)
{
  itl_history_reset_after_edit(le);
  itl_undo_push_closed(le);
}

/* Replaces the span the previous yank or last-argument insertion put into the
   line with text, keeping as much of it as the output buffer holds. */
ITL_DEF void itl_le_replace_inserted(itl_le_t *le, const itl_string_t *text)
{
  size_t i;

  if (itl_g_le_inserted_start > le->line->length) {
    itl_g_le_inserted_start = le->line->length;
  }
  if (itl_g_le_inserted_length > le->line->length - itl_g_le_inserted_start) {
    itl_g_le_inserted_length = le->line->length - itl_g_le_inserted_start;
  }

  itl_string_erase(le->line, itl_g_le_inserted_start,
                   itl_g_le_inserted_length, false);
  le->cursor_position = itl_g_le_inserted_start;
  itl_g_le_inserted_length = 0;

  for (i = 0; i < text->length; ++i) {
    if (le->line->size + text->chars[i].size >= le->out_size) {
      break;
    }
    itl_string_insert(le->line, le->cursor_position, text->chars[i]);
    le->cursor_position += 1;
    itl_g_le_inserted_length += 1;
  }
}

ITL_DEF void itl_le_yank(itl_le_t *le)
{
  if (itl_g_kill_ring_count == 0) {
    itl_g_tty_should_refresh_text = false;
    return;
  }

  itl_le_begin_edit(le);
  itl_g_kill_ring_yank_distance = 0;
  itl_g_le_inserted_start = le->cursor_position;
  itl_g_le_inserted_length = 0;
  itl_le_replace_inserted(le, itl_kill_ring_at_distance(0));
  itl_g_le_action = ITL_LE_ACTION_YANK;
}

/* Alt-Y right after a yank swaps the yanked text for the next older kill and
   wraps around to the newest after the oldest. The whole yank stays one undo
   step. */
ITL_DEF void itl_le_yank_pop(itl_le_t *le, itl_le_action_kind previous_action)
{
  if (previous_action != ITL_LE_ACTION_YANK || itl_g_kill_ring_count == 0) {
    itl_g_tty_should_refresh_text = false;
    return;
  }

  itl_g_kill_ring_yank_distance =
      (itl_g_kill_ring_yank_distance + 1) % itl_g_kill_ring_count;
  itl_le_replace_inserted(
      le, itl_kill_ring_at_distance(itl_g_kill_ring_yank_distance));
  itl_g_le_action = ITL_LE_ACTION_YANK;
}

ITL_DEF bool itl_char_is_regional_indicator(itl_utf8_t ch)
{
  uint32_t cp;

  if (ch.size != 4) {
    return false;
  }

  cp = itl_utf8_codepoint(ch);
  return cp >= 0x1F1E6 && cp <= 0x1F1FF;
}

/* Whether the character at position draws on the cell of the one before it,
   so the two form one grapheme. That holds for a combining mark, a variation
   selector, a zero-width joiner, an emoji skin tone modifier, the character
   after a zero-width joiner, and the second regional indicator of a flag. */
ITL_DEF bool itl_line_char_joins_previous(const itl_string_t *line,
                                          size_t position)
{
  itl_utf8_t ch = line->chars[position];
  size_t indicator_count = 0;

  if (position == 0 || ch.size == 1 || itl_char_has_visible_notation(ch)) {
    return false;
  }
  if (itl_char_width(ch) == 0 ||
      itl_char_is_zero_width_joiner(line->chars[position - 1]))
  {
    return true;
  }
  if (!itl_char_is_regional_indicator(ch)) {
    return false;
  }

  while (indicator_count < position &&
         itl_char_is_regional_indicator(
             line->chars[position - indicator_count - 1]))
  {
    indicator_count += 1;
  }

  return indicator_count % 2 == 1;
}

/* The start of the grapheme that holds the character at position. */
ITL_DEF size_t itl_le_grapheme_start(const itl_string_t *line, size_t position)
{
  while (position > 0 && itl_line_char_joins_previous(line, position)) {
    position -= 1;
  }

  return position;
}

/* The end of the grapheme that starts at position. */
ITL_DEF size_t itl_le_grapheme_end(const itl_string_t *line, size_t position)
{
  position += 1;
  while (position < line->length &&
         itl_line_char_joins_previous(line, position))
  {
    position += 1;
  }

  return position;
}

ITL_DEF void itl_chars_reverse(itl_utf8_t *chars, size_t start, size_t end)
{
  itl_utf8_t held;

  while (start + 1 < end) {
    end -= 1;
    held = chars[start];
    chars[start] = chars[end];
    chars[end] = held;
    start += 1;
  }
}

/* Swaps the grapheme before the cursor with the one under it and steps past
   both, so a combining mark stays on its base character. At the end of the
   line the last two graphemes are swapped. */
ITL_DEF void itl_le_transpose_chars(itl_le_t *le)
{
  itl_utf8_t *chars = le->line->chars;
  size_t at = le->cursor_position;
  size_t first_start, second_start, second_end;

  if (le->line->length < 2 || at == 0) {
    itl_g_tty_should_refresh_text = false;
    return;
  }
  if (at >= le->line->length) {
    at = le->line->length - 1;
  }

  second_start = itl_le_grapheme_start(le->line, at);
  if (second_start == 0) {
    itl_g_tty_should_refresh_text = false;
    return;
  }
  second_end = itl_le_grapheme_end(le->line, second_start);
  first_start = itl_le_grapheme_start(le->line, second_start - 1);

  itl_le_begin_edit(le);
  itl_chars_reverse(chars, first_start, second_start);
  itl_chars_reverse(chars, second_start, second_end);
  itl_chars_reverse(chars, first_start, second_end);
  le->cursor_position = second_end;
}

#define ITL_LE_IS_WORD_CHAR(ch) ((ch).size > 1 || isalnum((ch).bytes[0]))

ITL_DEF size_t itl_le_word_end_from(const itl_string_t *line, size_t i)
{
  while (i < line->length && !ITL_LE_IS_WORD_CHAR(line->chars[i])) {
    i += 1;
  }
  while (i < line->length && ITL_LE_IS_WORD_CHAR(line->chars[i])) {
    i += 1;
  }

  return i;
}

ITL_DEF size_t itl_le_word_start_from(const itl_string_t *line, size_t i)
{
  while (i > 0 && !ITL_LE_IS_WORD_CHAR(line->chars[i - 1])) {
    i -= 1;
  }
  while (i > 0 && ITL_LE_IS_WORD_CHAR(line->chars[i - 1])) {
    i -= 1;
  }

  return i;
}

/* Swaps the word before the cursor with the word after it and leaves the
   cursor after both, the way readline does. At the end of the line the last
   two words are swapped and trailing blanks stay where they are. */
ITL_DEF void itl_le_transpose_words(itl_le_t *le)
{
  itl_string_t *line = le->line;
  size_t second_start = itl_le_word_start_from(
      line, itl_le_word_end_from(line, le->cursor_position));
  size_t second_end = itl_le_word_end_from(line, second_start);
  size_t first_start = itl_le_word_start_from(line, second_start);
  size_t first_end = itl_le_word_end_from(line, first_start);
  size_t first_length = first_end - first_start;
  size_t gap_length;
  size_t second_length;
  itl_utf8_t *swapped;

  if (first_start == second_start || second_start < first_end) {
    itl_g_tty_should_refresh_text = false;
    return;
  }

  gap_length = second_start - first_end;
  second_length = second_end - second_start;
  swapped = (itl_utf8_t *) itl_malloc((second_end - first_start) *
                                      sizeof(itl_utf8_t));
  memcpy(swapped, line->chars + second_start,
         second_length * sizeof(itl_utf8_t));
  memcpy(swapped + second_length, line->chars + first_end,
         gap_length * sizeof(itl_utf8_t));
  memcpy(swapped + second_length + gap_length, line->chars + first_start,
         first_length * sizeof(itl_utf8_t));

  itl_le_begin_edit(le);
  memcpy(line->chars + first_start, swapped,
         (second_end - first_start) * sizeof(itl_utf8_t));
  ITL_FREE(swapped);
  le->cursor_position = second_end;
}

/* Finds the last word of a history entry. Quotes and backslashes keep their
   blanks inside the word, so a quoted argument comes back whole. */
ITL_DEF bool itl_last_word_span(const char *text, size_t size,
                                size_t *out_start, size_t *out_end)
{
  size_t i;
  size_t word_start = 0;
  bool is_in_word = false;
  bool has_word = false;
  char quote = 0;

  for (i = 0; i < size; ++i) {
    char ch = text[i];
    bool is_blank = ch == ' ' || ch == '\t' || ch == '\n';

    if (!is_in_word) {
      if (is_blank) {
        continue;
      }
      is_in_word = true;
      word_start = i;
    }

    if (quote != 0) {
      if (ch == quote) {
        quote = 0;
      } else if (ch == '\\' && quote == '"' && i + 1 < size) {
        i += 1;
      }
      continue;
    }

    if (ch == '\\') {
      if (i + 1 < size) {
        i += 1;
      }
      continue;
    }
    if (ch == '\'' || ch == '"') {
      quote = ch;
      continue;
    }
    if (is_blank) {
      *out_start = word_start;
      *out_end = i;
      has_word = true;
      is_in_word = false;
    }
  }

  if (is_in_word) {
    *out_start = word_start;
    *out_end = size;
    has_word = true;
  }

  return has_word;
}

/* Forget the span the last yank or last-argument insertion put into the line,
   so no later Alt-Y or Alt-. replaces text it did not insert. */
ITL_DEF void itl_le_end_insertion_walk(void)
{
  itl_g_le_action = ITL_LE_ACTION_NONE;
  itl_g_le_inserted_start = 0;
  itl_g_le_inserted_length = 0;
}

/* Alt-. inserts the last word of the newest history entry. Pressed again right
   away, it replaces that word with the last word of the entry before, and an
   entry with no word or with a word that is not UTF-8 is skipped. Past the
   oldest entry the line stays and the walk stays open. */
ITL_DEF void itl_le_insert_last_argument(itl_le_t *le,
                                         itl_le_action_kind previous_action)
{
  char decoded[ITL_STRING_MAX_LEN + 1];
  size_t decoded_size = 0;
  size_t word_start = 0;
  size_t word_end = 0;
  size_t distance = 0;
  bool is_repeat = previous_action == ITL_LE_ACTION_LAST_ARGUMENT;
  bool has_word = false;
  itl_string_t word;

  if (is_repeat) {
    itl_g_le_action = ITL_LE_ACTION_LAST_ARGUMENT;
    distance = itl_g_last_argument_distance + 1;
  }

  if (!itl_history_ensure_read_buffer()) {
    itl_g_tty_should_refresh_text = false;
    return;
  }

  itl_string_init(&word);
  for (; distance < itl_g_history_count; ++distance) {
    size_t offset =
        itl_history_index_to_offset(itl_g_history_count - 1 - distance);
    if (itl_history_decode_entry_buffered(offset, decoded, sizeof(decoded),
                                          &decoded_size) &&
        itl_last_word_span(decoded, decoded_size, &word_start, &word_end) &&
        itl_string_from_bytes(&word, decoded + word_start,
                              word_end - word_start))
    {
      has_word = true;
      break;
    }
  }

  if (!has_word) {
    ITL_FREE(word.chars);
    itl_g_tty_should_refresh_text = false;
    return;
  }

  itl_g_le_action = ITL_LE_ACTION_LAST_ARGUMENT;
  if (!is_repeat) {
    itl_le_begin_edit(le);
    itl_g_le_inserted_start = le->cursor_position;
    itl_g_le_inserted_length = 0;
  }
  itl_g_last_argument_distance = distance;
  itl_le_replace_inserted(le, &word);
  ITL_FREE(word.chars);
}

/* Copies the line into the host's output buffer, false when it does not fit. */
ITL_DEF bool itl_le_copy_out(itl_le_t *le)
{
  return itl_string_to_cstr(le->line, le->out_buf, le->out_size) == TL_SUCCESS;
}

/* Ctrl-X Ctrl-E hands the line to the host's editor. The edited text replaces
   the whole line as one undo step, and text that is not UTF-8 or does not fit
   the output buffer leaves the line alone. */
ITL_DEF void itl_le_edit_external(itl_le_t *le)
{
  const char *edited = NULL;
  size_t edited_size;
  itl_string_t replacement;

  if (itl_g_edit_callback == NULL) {
    itl_g_tty_should_refresh_text = false;
    return;
  }
  if (!itl_le_copy_out(le)) {
    return;
  }

  itl_hint_hold(le);
  if (itl_g_edit_callback(le->out_buf, &edited) <= 0 || edited == NULL) {
    itl_hint_release();
    return;
  }
  itl_hint_release();

  edited_size = strlen(edited);
  if (edited_size >= le->out_size) {
    return;
  }

  itl_string_init(&replacement);
  if (itl_string_from_bytes(&replacement, edited, edited_size)) {
    itl_le_begin_edit(le);
    itl_string_copy(le->line, &replacement);
    le->cursor_position = le->line->length;
  }
  ITL_FREE(replacement.chars);
}

ITL_DEF tl_status_code itl_le_key_handle(itl_le_t *le, int esc)
{
  int prev_control = itl_g_last_control;
  itl_le_action_kind previous_action = itl_g_le_action;

  /* A key continues a kill, a yank, or a last-argument walk only when it
     directly follows one. */
  itl_g_le_action = ITL_LE_ACTION_NONE;

  /* Remember the last control sequence. */
  itl_g_last_control = esc;

  /* Refresh text by default, avoid if we are only moving the cursor. */
  itl_g_tty_should_refresh_text = true;

  switch (esc & TL_MASK_KEY) {
  case TL_KEY_TAB: {
    tl_status_code completion_code = TL_SUCCESS;

    /* A registered completion callback handles TAB in place, inserting the
       common prefix, opening the menu, or listing the candidates. With no
       callback, TAB keeps its old contract of returning to the host. */
    if (itl_completion_handle_tab(le, &completion_code)) {
      return completion_code;
    }
    ITL_TRY(itl_le_copy_out(le), return TL_ERROR_SIZE);
    return TL_PRESSED_TAB;
  } break;

  case TL_KEY_UP:
  case TL_KEY_DOWN: {
    itl_le_metrics_t m = itl_le_compute_metrics(le, itl_g_tty_prev_cols);
    bool is_up = (esc & TL_MASK_KEY) == TL_KEY_UP;
    bool was_vertical = (prev_control & TL_MASK_KEY) == TL_KEY_UP ||
                        (prev_control & TL_MASK_KEY) == TL_KEY_DOWN;

    /* Move a visual row while inside a multiline or wrapped buffer. The input's
       first row sits at prompt_rows in the metrics, since a multi-row prompt
       offsets the rows, so history is recalled there rather than at row zero,
       which would be inside the prompt. */
    if (is_up ? m.cursor_row > le->prompt_rows
              : m.cursor_row + 1 < m.total_rows)
    {
      if (!was_vertical) {
        le->goal_column = m.cursor_col;
      }
      le->cursor_position = itl_le_index_at_visual(
          le, itl_g_tty_prev_cols, is_up ? m.cursor_row - 1 : m.cursor_row + 1,
          le->goal_column);
      itl_g_tty_should_refresh_text = false;
      break;
    }

    /* A non-empty line on its last visual row recalls only entries that begin
       with the typed text when prefix search is on. The draft line is saved
       inside get_prev on the first step up. */
    if (is_up) {
      if (itl_g_history_prefix_search_enabled &&
          m.cursor_row + 1 >= m.total_rows && itl_g_history_prefix_get_prev(le))
      {
        break;
      }
      itl_g_history_get_prev(le);
    } else {
      if (itl_g_history_prefix_search_enabled &&
          itl_g_history_prefix_get_next(le))
      {
        break;
      }
      itl_g_history_get_next(le);
    }
  } break;

  case TL_KEY_RIGHT: {
    bool cursor_was_on_space;
    /* At the end of the line, a Right with ghost text accepts the suggestion by
       inserting it, rather than moving the cursor nowhere. */
    if (le->cursor_position == le->line->length && itl_g_ghost_len > 0) {
      /* Ctrl-Right and Alt-F accept only the next word of the suggestion. */
      if (esc & (TL_MOD_CTRL | TL_MOD_ALT)) {
        itl_ghost_accept_bytes(le, itl_ghost_word_length());
      } else {
        itl_ghost_accept(le);
      }
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
        itl_le_move_right(le, itl_le_grapheme_end(le->line,
                                                  le->cursor_position) -
                                  le->cursor_position);
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
        itl_le_move_left(le, le->cursor_position -
                                 itl_le_grapheme_start(
                                     le->line, le->cursor_position - 1));
      }
    }
    itl_g_tty_should_refresh_text = false;
  } break;

  case TL_KEY_END: {
    size_t line_end = itl_le_line_end_of(le, le->cursor_position);
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
    size_t line_start = itl_le_line_start_of(le, le->cursor_position);
    itl_le_move_left(le, le->cursor_position - line_start);
    itl_g_tty_should_refresh_text = false;
  } break;
  case TL_KEY_ENTER: {
    bool insert_newline = (esc & TL_MOD_ALT) != 0;

    /* A trailing backslash at the end of the line continues it, fish-style. A
       backslash with text after it submits the line instead, and so does an
       even run, where the last backslash is itself escaped. */
    if (!insert_newline && le->cursor_position == le->line->length) {
      size_t backslash_count = 0;

      while (backslash_count < le->cursor_position &&
             ITL_LE_IS_BACKSLASH(
                 le->line->chars[le->cursor_position - backslash_count - 1]))
      {
        backslash_count += 1;
      }
      insert_newline = backslash_count % 2 == 1;
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

    /* Submit the line as typed. The shell joins backslash continuations
       itself, and only where they are not quoted, so history keeps the
       physical lines. */
    ITL_TRY(itl_le_copy_out(le), return TL_ERROR_SIZE);
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
        itl_le_kill(le, steps - 1, true, previous_action);
      }
    } else if (le->cursor_position > 0) {
      ITL_LE_ERASE_BACKWARD(le, le->cursor_position -
                                    itl_le_grapheme_start(
                                        le->line, le->cursor_position - 1));
    }
  } break;

  case TL_KEY_DELETE: {
    if (esc & TL_MOD_CTRL) {
      itl_le_kill(le, ITL_LE_STEPS_TO_TOKEN_FORWARD(le), false,
                  previous_action);
    } else if (le->cursor_position < le->line->length) {
      ITL_LE_ERASE_FORWARD(le, itl_le_grapheme_end(le->line,
                                                   le->cursor_position) -
                                   le->cursor_position);
    }
  } break;

  case TL_KEY_KILL_LINE: {
    size_t line_end = itl_le_line_end_of(le, le->cursor_position);
    itl_le_kill(le, line_end - le->cursor_position, false, previous_action);
  } break;

  case TL_KEY_KILL_LINE_BEFORE: {
    size_t line_start = itl_le_line_start_of(le, le->cursor_position);
    itl_le_kill(le, le->cursor_position - line_start, true, previous_action);
  } break;

  case TL_KEY_YANK: itl_le_yank(le); break;

  case TL_KEY_YANK_POP: itl_le_yank_pop(le, previous_action); break;

  case TL_KEY_TRANSPOSE: {
    if (esc & TL_MOD_ALT) {
      itl_le_transpose_words(le);
    } else {
      itl_le_transpose_chars(le);
    }
  } break;

  case TL_KEY_LAST_ARGUMENT:
    itl_le_insert_last_argument(le, previous_action);
    break;

  case TL_KEY_EDIT_EXTERNAL: itl_le_edit_external(le); break;

  case TL_KEY_SUSPEND: {
#if defined ITL_SUSPEND
    itl_raise_suspend();
#else
    (void) itl_le_copy_out(le);
    return TL_PRESSED_SUSPEND;
#endif /* ITL_SUSPEND */
  } break;

  case TL_KEY_EOF: {
    if (le->line->length > 0) {
      ITL_LE_ERASE_FORWARD(le, 1);
    } else {
      (void) itl_le_copy_out(le);
      return TL_PRESSED_EOF;
    }
  } break;

  case TL_KEY_INTERRUPT: {
    (void) itl_le_copy_out(le);
    return TL_PRESSED_INTERRUPT;
  } break;

  case TL_KEY_CLEAR: {
    itl_char_buf_t *b = &itl_g_char_buffer;
    ITL_TTY_GOTO_HOME(b);
    ITL_TTY_ERASE_SCREEN(b);
    ITL_CHAR_BUF_DUMP(b);
    ITL_CHAR_BUF_CLEAR(b);
  } break;

  case TL_KEY_HISTORY_END:
  case TL_KEY_HISTORY_BEGINNING: {
    /* Jump to the most recent or the oldest navigable entry. */
    if (itl_g_history_count > 0) {
      itl_history_save_draft(le);
      le->history_selected_index =
          ((esc & TL_MASK_KEY) == TL_KEY_HISTORY_END) ? itl_g_history_count - 1
                                                      : 0;
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
#if defined ITL_POSIX
  bool did_enter_raw_mode = false;
#endif

  TL_ASSERT(!(TL_HISTORY_MAX_SIZE & (TL_HISTORY_MAX_SIZE - 1)) &&
            "History size must be a power of 2");
  TL_ASSERT(TL_HISTORY_MAX_SIZE >= 0 && "History size must be positive");

  if (itl_g_is_active) {
    return TL_SUCCESS;
  }

  if (!itl_g_entered_raw_mode) {
    ITL_TRY(ITL_TTY_IS_TTY(), return TL_ERROR);
    ITL_TRY(tl_enter_raw_mode() == TL_SUCCESS, return TL_ERROR);
#if defined ITL_POSIX
    did_enter_raw_mode = true;
#endif
  }

#if defined ITL_POSIX
  if (!itl_install_sigwinch_handler()) {
    if (did_enter_raw_mode) {
      tl_exit_raw_mode();
    }
    return TL_ERROR;
  }
#endif

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

#if defined ITL_POSIX
  ITL_TRY(itl_restore_sigwinch_handler(), return TL_ERROR);
#endif

  itl_g_history_free();
  itl_vi_free();
  itl_kill_ring_free();
  ITL_FREE(itl_g_line_buffer.chars);
  ITL_FREE(itl_g_char_buffer.data);

  ITL_TRACELN("Exited, alloc count: %zu\n", itl_g_alloc_count);
  TL_ASSERT(itl_g_alloc_count == 0);

  itl_g_is_active = false;

  return TL_SUCCESS;
}

/* Decodes the entry at index and reports whether its bytes hold the query with
   ASCII letters folded. The match is converted into out only after the bytes
   match, so a rejected candidate costs one decode and one byte scan. An entry
   that is not valid UTF-8 is rejected. */
ITL_DEF bool itl_history_candidate_matches(size_t index, const char *query,
                                           size_t query_size,
                                           itl_string_t *out)
{
  char decoded[ITL_STRING_MAX_LEN + 1];
  size_t decoded_size;

#if !defined NDEBUG
  itl_g_debug_history_candidate_count += 1;
#endif

  if (!itl_history_search_decode_entry(itl_history_search_index_to_offset(index),
                                       decoded, sizeof(decoded), &decoded_size))
  {
    return false;
  }

  if (!itl_ascii_contains_casefold(decoded, decoded_size, query, query_size, 0))
  {
    return false;
  }

  return itl_string_from_bytes(out, decoded, decoded_size);
}

/* Walks history from start_index toward older entries, or toward newer ones
   when is_forward is set, returning the index of the first that contains query
   as a substring or ITL_HISTORY_NONE when none match. The matched entry is
   written into out. */
ITL_DEF size_t itl_history_scan_match(const char *query, size_t query_size,
                                      size_t start_index, bool is_forward,
                                      itl_string_t *out)
{
  size_t count = itl_history_search_count();
  size_t i;

  if (count == 0 || start_index == ITL_HISTORY_NONE ||
      (is_forward && start_index >= count) ||
      (!itl_g_history_search_snapshot.is_active && itl_g_history_path == NULL))
  {
    return ITL_HISTORY_NONE;
  }

  TL_ASSERT(is_forward || start_index < count);

  /* One search keystroke can walk every navigable entry, decoded from the
     in-memory file buffer rather than a read per entry. */
  if (!itl_history_search_prepare()) {
    return ITL_HISTORY_NONE;
  }

  if (is_forward) {
    for (i = start_index; i < count; ++i) {
      if (itl_history_candidate_matches(i, query, query_size, out)) {
        return i;
      }
    }
    return ITL_HISTORY_NONE;
  }

  /* Count down from start_index to zero inclusive without underflowing. */
  for (i = start_index + 1; i-- > 0;) {
    if (itl_history_candidate_matches(i, query, query_size, out)) {
      return i;
    }
  }

  return ITL_HISTORY_NONE;
}

ITL_DEF size_t itl_history_find_match(const char *query, size_t query_size,
                                      size_t start_index, itl_string_t *out)
{
  return itl_history_scan_match(query, query_size, start_index, false, out);
}

ITL_DEF size_t itl_history_find_match_forward(const char *query,
                                              size_t query_size,
                                              size_t start_index,
                                              itl_string_t *out)
{
  return itl_history_scan_match(query, query_size, start_index, true, out);
}

/* The newest navigable entry index, or ITL_HISTORY_NONE when history is empty.
 */
#define ITL_HISTORY_NEWEST()                                                   \
  (itl_history_search_count() > 0 ? itl_history_search_count() - 1             \
                                  : ITL_HISTORY_NONE)

/* An entry rejected by the shorter query cannot hold its extension, so a
   narrowed match only has to test itself and the entries below it. A narrowed
   miss stays a miss and needs no scan at all. */
ITL_DEF size_t itl_history_narrow_match(const char *query, size_t query_size,
                                        size_t match, bool is_narrowable,
                                        itl_string_t *out)
{
  size_t from;

  if (!is_narrowable) {
    return itl_history_find_match(query, query_size, ITL_HISTORY_NEWEST(), out);
  }

  if (match == ITL_HISTORY_NONE) {
    return ITL_HISTORY_NONE;
  }

  if (!itl_history_search_prepare()) {
    return ITL_HISTORY_NONE;
  }

  if (itl_history_candidate_matches(match, query, query_size, out)) {
    return match;
  }

  from = (match > 0) ? match - 1 : ITL_HISTORY_NONE;

  return itl_history_find_match(query, query_size, from, out);
}

#define ITL_SEARCH_SGR_GREEN  "\x1b[32m"
#define ITL_SEARCH_SGR_YELLOW "\x1b[33m"
#define ITL_SEARCH_SGR_BOLD   "\x1b[1m"

/* Records one highlight span for the reverse search block, dropped once the
   span array is full. The start and end are codepoint indices into the display
   buffer, and the sgr is a static escape so the pointer stays valid until the
   refresh draws it. */
ITL_DEF void itl_search_push_span(size_t start, size_t end, const char *sgr)
{
  if (!itl_g_colors_enabled) {
    return;
  }

  if (start < end && itl_g_search_span_count < ITL_HIGHLIGHT_MAX_SPANS) {
    itl_search_span_add(start, end, sgr);
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

/* The hint row of the search block, with its key tokens bolded. */
typedef struct
{
  const char *text;
  bool is_key;
} itl_search_guide_part;

ITL_DEF const itl_search_guide_part itl_search_guide[] = {
    {"up", true},         {"/", false},
    {"down", true},       {" to move, ", false},
    {"enter", true},      {"/", false},
    {"tab", true},        {" to accept, ", false},
    {"esc", true},        {"/", false},
    {"ctrl-g", true},     {" to cancel", false},
};

/* Flattens the query runes into bytes so one scan can match on bytes. A rune
   that would not fit is dropped whole, keeping the result valid UTF-8. Returns
   the byte count written, and the result is always terminated. */
ITL_DEF size_t itl_search_query_bytes(const itl_string_t *query, char *out,
                                      size_t capacity)
{
  size_t size = 0;
  size_t i;

  for (i = 0; i < query->length; ++i) {
    itl_utf8_t ch = query->chars[i];

    if (size + ch.size >= capacity) {
      break;
    }

    memcpy(out + size, ch.bytes, ch.size);
    size += ch.size;
  }

  out[size] = '\0';

  return size;
}

/* Follows the search term while no entry holds it. */
#define ITL_SEARCH_NO_MATCH        " no match"
#define ITL_SEARCH_NO_MATCH_LENGTH 9

/* Runs a reverse incremental history search. The live prompt stays on screen
   and the matched entry, the search term, and the hint are drawn below it as
   one multiline buffer swapped into the line editor. The block carries its own
   highlight spans. Returns the control key that ended the search so the caller
   can re-dispatch it, or TL_KEY_UNKN when the search was cancelled or accepted
   with no further action. */
ITL_DEF int itl_history_search(itl_le_t *le)
{
  itl_string_t query;
  itl_string_t display;
  itl_string_t match_str;
  itl_char_buf_t status;

  /* The draft the search started on. The editor points le->line at the search
     block for the whole loop, so the draft stays untouched in the line
     buffer. */
  const itl_string_t *draft = &itl_g_line_buffer;

  /* Index of the matched entry, ITL_HISTORY_NONE while nothing matches. The
     matched entry text is kept in match_str for the preview. */
  size_t match = ITL_HISTORY_NONE;

  /* The query flattened to bytes, refreshed once for each query change so a
     scan never re-serializes it per candidate. */
  char query_bytes[ITL_STRING_MAX_LEN + 1];
  size_t query_size = 0;

  /* True while the current match is the newest entry holding the query, which
     is what lets a longer query resume from the match instead of the newest
     entry. Any directional key moves off the newest match and clears it. */
  bool is_narrowable = false;

  /* The clipped preview text and the host spans over it, kept while the match
     index and the terminal width stay put so a query keystroke redraws without
     calling the highlighter again. */
  char match_render[ITL_STRING_MAX_LEN];
  size_t match_bytes = 0;
  size_t match_length = 0;
  tl_highlight_span cached_spans[ITL_HIGHLIGHT_MAX_SPANS];
  size_t cached_span_count = 0;
  size_t rendered_match = ITL_HISTORY_NONE;
  size_t rendered_cols = 0;
  bool has_rendered_preview = false;

  size_t saved_prompt_width = le->prompt_width;

  int result = TL_KEY_UNKN;
  bool accepted = false;
  /* When search starts on the draft, the current line is the user's draft and
     must be saved so a later step past the newest entry can restore it. */
  bool was_on_draft = (le->history_selected_index == ITL_HISTORY_NONE);
  uint8_t byte;

  itl_string_init(&query);
  itl_string_init(&display);
  itl_string_init(&match_str);
  itl_char_buf_init(&status);

  query_bytes[0] = '\0';

  /* A draft already typed becomes the initial query, so it moves into the
     search term instead of staying on the prompt line. */
  if (le->line->length > 0) {
    itl_string_copy(&query, le->line);
    query_size =
        itl_search_query_bytes(&query, query_bytes, sizeof(query_bytes));
    match = itl_history_find_match(query_bytes, query_size,
                                   ITL_HISTORY_NEWEST(), &match_str);
    is_narrowable = true;
  }

  while (true) {
    /* The matched entry shares line one with the live prompt, then the search
       term and the hint follow on their own rows. The whole block is one
       multiline buffer drawn through the normal refresh. */
    size_t tty_cols = ITL_MAX(itl_g_tty_prev_cols, 1);
    size_t line2_start, query_start, line3_start, guide_position;
    size_t s;

    if (!has_rendered_preview || rendered_match != match ||
        rendered_cols != tty_cols)
    {
      const itl_string_t *preview =
          (match != ITL_HISTORY_NONE) ? &match_str : draft;
      size_t budget = (tty_cols > saved_prompt_width + 1)
                          ? tty_cols - saved_prompt_width - 1
                          : tty_cols;
      size_t match_width = 0;
      size_t full_width = 0;
      size_t pi, pj;
      bool is_cut;

      match_bytes = 0;
      match_length = 0;

      for (pi = 0; pi < preview->length; ++pi) {
        full_width += ITL_LE_IS_NEWLINE(preview->chars[pi])
                          ? 1
                          : itl_line_char_width(preview->chars, pi);
      }
      is_cut = full_width > budget && budget > ITL_MENU_CUT_MARK_WIDTH;
      if (is_cut) {
        budget -= ITL_MENU_CUT_MARK_WIDTH;
      }

      /* Flatten newlines to spaces and clip to the prompt's row remainder so
         the match never wraps under the prompt. A clipped match ends in an
         ellipsis. */
      for (pi = 0; pi < preview->length; ++pi) {
        itl_utf8_t pch = preview->chars[pi];
        bool is_newline = ITL_LE_IS_NEWLINE(pch);
        size_t char_width =
            is_newline ? 1 : itl_line_char_width(preview->chars, pi);

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
      cached_span_count = 0;

      /* Line one, the matched entry highlighted as the command it would
         become. The match sits at offset zero, so the host's codepoint spans
         index the display buffer unchanged. */
      if (itl_should_run_highlight()) {
        tl_highlight hl;
        hl.spans = cached_spans;
        hl.count = 0;
        hl.capacity = ITL_HIGHLIGHT_MAX_SPANS;
        hl.cursor = TL_HIGHLIGHT_NO_CURSOR;

#if !defined NDEBUG
        itl_g_debug_search_highlight_count += 1;
#endif

        if (itl_g_highlight_callback(match_render, &hl)) {
          cached_span_count = ITL_MIN(hl.count, ITL_HIGHLIGHT_MAX_SPANS);
        }
      }

      if (is_cut &&
          match_bytes + ITL_MENU_CUT_MARK_WIDTH < ITL_STRING_MAX_LEN)
      {
        memcpy(match_render + match_bytes, ITL_MENU_CUT_MARK,
               ITL_MENU_CUT_MARK_WIDTH + 1);
        match_bytes += ITL_MENU_CUT_MARK_WIDTH;
        match_length += ITL_MENU_CUT_MARK_WIDTH;
      }

      rendered_match = match;
      rendered_cols = tty_cols;
      has_rendered_preview = true;
    }

    itl_g_search_span_count = 0;

    for (s = 0; s < cached_span_count; ++s) {
      itl_search_push_span(cached_spans[s].start, cached_spans[s].end,
                           cached_spans[s].sgr);
    }

    ITL_CHAR_BUF_CLEAR(&status);
    itl_char_buf_append_cstr(&status, match_render);
    itl_char_buf_append_byte(&status, '\n');

    /* Line two, the search label in green and the typed query in yellow. The
       label `(incremental search)` is 20 codepoints and the trailing ` '` is
       two more. */
    line2_start = match_length + 1;
    itl_char_buf_append_cstr(&status, "(incremental search) '");
    itl_search_push_span(line2_start, line2_start + 20, ITL_SEARCH_SGR_GREEN);
    query_start = line2_start + 22;
    if (query.length > 0) {
      itl_char_buf_append_string(&status, &query);
      itl_search_push_span(query_start, query_start + query.length,
                           ITL_SEARCH_SGR_YELLOW);
    }
    itl_char_buf_append_byte(&status, '\'');
    line3_start = query_start + query.length + 2;
    if (query.length > 0 && match == ITL_HISTORY_NONE) {
      itl_char_buf_append_cstr(&status, ITL_SEARCH_NO_MATCH);
      line3_start += ITL_SEARCH_NO_MATCH_LENGTH;
    }
    itl_char_buf_append_byte(&status, '\n');

    /* Line three, the hint with its key tokens bolded. */
    guide_position = line3_start;
    for (s = 0; s < ITL_COUNTOF(itl_search_guide); ++s) {
      guide_position = itl_search_append_guide(
          &status, guide_position, itl_search_guide[s].text,
          itl_search_guide[s].is_key);
    }

    itl_string_from_bytes(&display, status.data, status.size);

    /* The prompt stays drawn with its rows counted, so the block row math
       starts from the prompt's trailing row exactly as the normal render path.
       Only the width is zeroed so the indent is zero and the term and hint rows
       sit flush left under the prompt. */
    le->prompt_width = 0;
    le->line = &display;
    le->cursor_position = query_start + query.length;
    itl_g_right_prompt_is_held = true;
    itl_g_search_spans_active = true;
    itl_g_tty_should_refresh_text = true;
    itl_le_tty_refresh(le);

    if (!ITL_READ_BYTE(&byte)) {
      break;
    }

    {
      int key = itl_esc_parse(byte);
      int kind = key & TL_MASK_KEY;
      bool is_newer_key = (byte == 6) || (kind == TL_KEY_UP) ||
                          (kind == TL_KEY_TAB && (key & TL_MOD_SHIFT) != 0);
      bool is_older_key =
          (kind == TL_KEY_HISTORY_SEARCH) || (kind == TL_KEY_DOWN);

      if (is_newer_key || is_older_key) {
        size_t from;
        size_t next;

        if (is_newer_key) {
          from = (match != ITL_HISTORY_NONE) ? match + 1 : 0;
        } else if (match != ITL_HISTORY_NONE) {
          from = (match > 0) ? match - 1 : ITL_HISTORY_NONE;
        } else {
          from = ITL_HISTORY_NEWEST();
        }
        next = itl_history_scan_match(query_bytes, query_size, from,
                                      is_newer_key, &match_str);

        /* A step off the newest match leaves entries above it unexamined for
           the next longer query, so the narrowed scan is no longer valid. */
        is_narrowable = false;

        if (next != ITL_HISTORY_NONE) {
          match = next;
        }
      } else if (kind == TL_KEY_CHAR) {
        itl_string_insert(&query, query.length, itl_utf8_parse(byte));
        query_size =
            itl_search_query_bytes(&query, query_bytes, sizeof(query_bytes));

        match = itl_history_narrow_match(query_bytes, query_size, match,
                                         is_narrowable, &match_str);
        is_narrowable = true;
      } else if (kind == TL_KEY_BACKSPACE) {
        if (query.length > 0) {
          itl_string_erase(&query, query.length, 1, true);
          query_size =
              itl_search_query_bytes(&query, query_bytes, sizeof(query_bytes));

          /* A shorter query can match entries its longer form rejected, so the
             scan restarts from the newest entry. */
          if (query.length > 0) {
            match = itl_history_find_match(query_bytes, query_size,
                                           ITL_HISTORY_NEWEST(), &match_str);
            is_narrowable = true;
          } else {
            match = ITL_HISTORY_NONE;
            is_narrowable = false;
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
        result =
            (kind == TL_KEY_ENTER || kind == TL_KEY_TAB) ? TL_KEY_UNKN : key;
        break;
      }
    }
  }

  /* Restore the real prompt width and line editor buffer, and stop drawing the
     search spans so the next refresh highlights the line as a command again. */
  itl_g_search_spans_active = false;
  itl_g_right_prompt_is_held = false;
  le->prompt_width = saved_prompt_width;
  le->line = &itl_g_line_buffer;

  /* The draft was never written while the search block was displayed, so a
     cancelled search needs no restoring copy. */
  if (accepted && match != ITL_HISTORY_NONE) {
    /* Preserve the pre-search draft so stepping down past the newest entry
       restores it rather than a stale draft. */
    if (was_on_draft) {
      if (itl_g_history_draft == NULL) {
        itl_g_history_draft = itl_string_alloc();
      }

      itl_string_copy(itl_g_history_draft, draft);
    }

    /* A match longer than the host buffer would fail itl_string_to_cstr on
       submit and abort the read, so the pre-search line is kept when the match
       does not fit. The size plus the null terminator must stay within
       out_size. */
    if (match_str.size + 1 <= le->out_size) {
      itl_string_copy(le->line, &match_str);
      le->history_selected_index = itl_g_history_search_snapshot.is_active
                                       ? ITL_HISTORY_NONE
                                       : match;
    }
  }

  le->cursor_position = le->line->length;

  ITL_FREE(query.chars);
  ITL_FREE(display.chars);
  ITL_FREE(match_str.chars);
  ITL_FREE(status.data);

  return result;
}

/* Reach history through whichever selector is configured. The host is asked
   first, since its own program runs outside the editor. The menu lists the
   matching entries under the prompt when it is enabled, and the incremental
   block runs when neither is. Returns the key that ended the search for the
   caller to dispatch, and writes the status the caller must return. */
ITL_DEF int itl_history_select(itl_le_t *le, tl_status_code *out_code)
{
  int result = TL_KEY_UNKN;

  *out_code = TL_SUCCESS;
  itl_hint_hold(le);
  itl_history_search_snapshot_begin();

  if (itl_g_history_select_callback != NULL) {
    tl_completion entries;
    const char *chosen = NULL;

    memset(&entries, 0, sizeof(entries));

    if (itl_history_menu_gather(le, &entries)) {
      int host_result = itl_g_history_select_callback(
          entries.candidates, entries.count, &chosen);

      if (host_result != 0) {
        if (host_result > 0 && chosen != NULL &&
            itl_completion_replace_token(le, &entries, chosen) &&
            itl_g_history_search_snapshot.is_active)
        {
          le->history_selected_index = ITL_HISTORY_NONE;
        }

        itl_g_tty_should_refresh_text = true;
        goto done;
      }
    }
  }

  if (itl_g_completion_menu_enabled) {
    *out_code = itl_history_menu(le);
    goto done;
  }

  result = itl_history_search(le);

done:
  itl_history_search_snapshot_end();
  itl_hint_release();
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

/* Stores a yanked or deleted span in the pending register and, when one was
   named, in the unnamed register too. A linewise span ends with a newline. */
ITL_DEF void itl_vi_register_store(const itl_string_t *src, size_t from,
                                   size_t to, bool is_linewise)
{
  size_t indices[2];
  size_t target_count = (itl_g_vi_pending_register != 0) ? 2 : 1;
  size_t i;

  indices[0] = itl_vi_register_index(itl_g_vi_pending_register);
  indices[1] = ITL_VI_REGISTER_UNNAMED;

  for (i = 0; i < target_count; ++i) {
    itl_string_t *reg = itl_vi_register_at(indices[i]);

    itl_vi_register_set_span(reg, src, from, to);
    if (is_linewise) {
      itl_string_insert(reg, reg->length, itl_newline_char);
    }
    itl_g_vi_register_is_linewise[indices[i]] = is_linewise;
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

  case '0': target = itl_le_line_start_of(le, le->cursor_position); break;

  case '$': {
    size_t line_start = itl_le_line_start_of(le, le->cursor_position);
    target = itl_le_line_end_of(le, le->cursor_position);
    if (!is_for_operator && target > line_start) {
      target -= 1;
    }
  } break;

  case '^': {
    size_t i = itl_le_line_start_of(le, le->cursor_position);
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

  itl_vi_register_store(le->line, start, end, false);

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

/* A yank changes nothing, so the dot command keeps repeating the change
   before it, as in vi. */
ITL_DEF void itl_vi_record_operator(itl_vi_operator_kind op, int motion_key,
                                    itl_utf8_t find_char, size_t count)
{
  if (op == ITL_VI_OP_YANK) {
    return;
  }

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
  size_t start = itl_le_line_start_of(le, le->cursor_position);
  size_t end = itl_le_line_end_of(le, le->cursor_position);
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

  itl_vi_register_store(le->line, start, end, true);

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

  itl_undo_push_closed(le);

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

  itl_undo_push_closed(le);

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

ITL_DEF void itl_vi_insert_repeated(itl_le_t *le, const char *text,
                                    size_t count)
{
  size_t i;

  for (i = 0; i < count; ++i) {
    itl_le_insert_cstr(le, text);
  }
}

ITL_DEF void itl_vi_paste_lines(itl_le_t *le, const char *line_text,
                                bool is_before, size_t count)
{
  size_t line_end = itl_le_line_end_of(le, le->cursor_position);
  bool is_after_last_line = !is_before && line_end >= le->line->length;
  size_t first;

  if (is_before) {
    first = itl_le_line_start_of(le, le->cursor_position);
  } else if (!is_after_last_line) {
    first = line_end + 1;
  } else {
    le->cursor_position = line_end;
    itl_le_insert_cstr(le, "\n");
    first = le->cursor_position;
  }

  le->cursor_position = first;
  itl_vi_insert_repeated(le, line_text, count);
  if (is_after_last_line && le->line->length > 0 &&
      ITL_LE_IS_NEWLINE(le->line->chars[le->line->length - 1]))
  {
    itl_string_erase(le->line, le->line->length, 1, true);
  }
  le->cursor_position = first;
}

ITL_DEF void itl_vi_do_paste(itl_le_t *le, bool is_before, size_t count)
{
  size_t reg_index = itl_vi_register_index(itl_g_vi_pending_register);
  itl_string_t *reg = itl_vi_register_at(reg_index);
  bool is_linewise = itl_g_vi_register_is_linewise[reg_index];
  char text[ITL_STRING_MAX_LEN];

  if (reg->length == 0) {
    return;
  }
  if (itl_string_to_cstr(reg, text, sizeof(text)) != TL_SUCCESS) {
    return;
  }

  itl_undo_push_closed(le);

  if (is_linewise) {
    itl_vi_paste_lines(le, text, is_before, count);
    return;
  }

  if (!is_before && le->line->length > 0) {
    itl_le_move_right(le, 1);
  }

  itl_vi_insert_repeated(le, text, count);

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
    case 'A':
      le->cursor_position = itl_le_line_end_of(le, le->cursor_position);
      break;
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

/* Reads the target of an f, F, t or T motion. Any other key, or a find the
   terminal never completes, yields the zero character. */
ITL_DEF itl_utf8_t itl_vi_read_find_char(itl_le_t *le, uint8_t motion_byte)
{
  itl_utf8_t find_char = ITL_ZERO_INIT;
  uint8_t target_byte;

  if (motion_byte != 'f' && motion_byte != 'F' && motion_byte != 't' &&
      motion_byte != 'T')
  {
    return find_char;
  }
  if (itl_le_await_chord(le, ITL_PREFIX_VI_FIND, motion_byte) &&
      ITL_READ_BYTE(&target_byte))
  {
    find_char = itl_utf8_parse(target_byte);
  }

  return find_char;
}

/* Reads the next key of a selection loop. A false result ends the loop: the
   input ended, a bare escape arrived, or the key is unknown. */
ITL_DEF bool itl_modal_read_key(uint8_t *byte, int *key)
{
  if (!ITL_READ_BYTE(byte)) {
    return false;
  }
  if (*byte == 27 && !itl_input_is_pending()) {
    return false;
  }

  *key = itl_esc_parse(*byte);

  return (*key & TL_MASK_KEY) != TL_KEY_UNKN;
}

/* Hands a selection loop's screen state back to the caller's edit mode. */
ITL_DEF void itl_modal_leave(itl_le_t *le, int mode, bool should_clamp)
{
  itl_search_spans_clear();
  itl_g_edit_mode = mode;
  if (should_clamp) {
    itl_vi_clamp_command_cursor(le);
  }
  itl_g_tty_should_refresh_text = true;
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
      itl_g_search_span_count = 0;
      itl_search_span_add(span_start, span_end, ITL_VI_SGR_SELECT);
    } else {
      itl_g_search_span_count = 0;
    }
    itl_g_search_spans_active = true;
    itl_g_tty_should_refresh_text = true;
    itl_le_tty_refresh(le);

    if (!itl_modal_read_key(&byte, &key) || byte == 'v') {
      break;
    }
    kind = key & TL_MASK_KEY;

    switch (kind) {
    case TL_KEY_ENTER:
    case TL_KEY_EOF:
    case TL_KEY_INTERRUPT:
    case TL_KEY_SUSPEND:
      itl_modal_leave(le, TL_EDIT_MODE_VI_COMMAND, true);
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

      itl_search_spans_clear();

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
      itl_utf8_t find_char = itl_vi_read_find_char(le, byte);
      bool is_inclusive, is_valid;
      size_t target;

      target = itl_vi_resolve_motion(le, (int) byte, find_char, 1, false,
                                     &is_inclusive, &is_valid);
      if (is_valid) {
        le->cursor_position = target;
      }
    }
  }

  itl_modal_leave(le, TL_EDIT_MODE_VI_COMMAND, true);
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
      size_t span_start = ITL_MIN(line_start + left_column, line_end);
      size_t span_end = ITL_MIN(line_start + right_column + 1, line_end);

      if (span_start < span_end) {
        itl_search_span_add(span_start, span_end, ITL_VI_SGR_SELECT);
      } else if (line_end < le->line->length) {
        /* The block covers no column on this line, an empty line or one shorter
           than the left edge. A one-cell span on its newline draws a reversed
           space there so the selection and the mock cursor still show. */
        itl_search_span_add(line_end, line_end + 1, ITL_VI_SGR_SELECT);
      }
    }
    itl_g_search_spans_active = true;
    itl_g_tty_should_refresh_text = true;
    itl_le_tty_refresh(le);

    if (!itl_modal_read_key(&byte, &key) || byte == 22) {
      break;
    }
    kind = key & TL_MASK_KEY;

    switch (kind) {
    case TL_KEY_ENTER:
    case TL_KEY_EOF:
    case TL_KEY_INTERRUPT:
    case TL_KEY_SUSPEND:
      itl_modal_leave(le, return_mode, false);
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
      itl_undo_push_closed(le);

      row = bottom_line + 1;
      while (row > top_line) {
        size_t line_start, line_end, span_start, span_end;
        row -= 1;
        line_start = itl_le_line_start_at_index(le, row);
        line_end = itl_le_line_end_of(le, line_start);
        span_start = ITL_MIN(line_start + left_column, line_end);
        span_end = ITL_MIN(line_start + right_column + 1, line_end);
        if (span_start < span_end) {
          itl_string_erase(le->line, span_end, span_end - span_start, true);
        }
      }

      le->cursor_position =
          itl_le_line_start_at_index(le, top_line) + left_column;
      itl_modal_leave(le, return_mode, return_mode == TL_EDIT_MODE_VI_COMMAND);
      return TL_SUCCESS;
    }

    if (byte == 'I' || byte == 'c' || byte == 'C') {
      size_t top_start = itl_le_line_start_at_index(le, top_line);
      size_t top_length = itl_le_line_end_of(le, top_start) - top_start;
      size_t enter_column = ITL_MIN(left_column, top_length);

      itl_search_spans_clear();
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
      itl_utf8_t find_char = itl_vi_read_find_char(le, byte);
      bool is_inclusive, is_valid;
      size_t target;

      target = itl_vi_resolve_motion(le, (int) byte, find_char, 1, false,
                                     &is_inclusive, &is_valid);
      if (is_valid) {
        size_t line_start = itl_le_line_start_of(le, le->cursor_position);
        size_t line_end = itl_le_line_end_of(le, line_start);
        le->cursor_position = ITL_MIN(ITL_MAX(target, line_start), line_end);
      }
      was_vertical = false;
    }
  }

  itl_modal_leave(le, return_mode, return_mode == TL_EDIT_MODE_VI_COMMAND);
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
        itl_search_span_add(marker, marker + 1, ITL_VI_SGR_SELECT);
      } else if (row != active_line && line_end < le->line->length) {
        /* The line has no character under the marker, an empty line or one
           shorter than the column. A one-cell span on its newline draws a
           reversed space there to stand in for the mock cursor. */
        itl_search_span_add(line_end, line_end + 1, ITL_VI_SGR_SELECT);
      }
    }
    itl_g_search_spans_active = true;

    le->cursor_position = (column < active_end - active_start)
                              ? active_start + column
                              : active_end;
    itl_g_tty_should_refresh_text = true;
    itl_le_tty_refresh(le);

    if (!itl_modal_read_key(&byte, &key) || byte == 7) {
      break;
    }
    kind = key & TL_MASK_KEY;

    if (kind == TL_KEY_ENTER || kind == TL_KEY_EOF ||
        kind == TL_KEY_INTERRUPT || kind == TL_KEY_SUSPEND)
    {
      itl_search_spans_clear();
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

  itl_search_spans_clear();
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
  itl_string_t *saved_line = le->line;
  size_t saved_cursor = le->cursor_position;

  char command[64];
  size_t command_length = 0;

  tl_status_code result = TL_SUCCESS;
  bool is_done = false;
  uint8_t byte;

  command[0] = '\0';
  itl_string_copy(original, le->line);
  itl_g_prefix_kind = ITL_PREFIX_VI_EX;

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

  itl_g_prefix_kind = ITL_PREFIX_NONE;
  le->prompt = saved_prompt;
  le->prompt_size = saved_prompt_size;
  le->prompt_width = saved_prompt_width;
  le->prompt_rows = saved_prompt_rows;
  le->line = saved_line;
  itl_string_copy(le->line, original);
  le->cursor_position = saved_cursor <= le->line->length ? saved_cursor
                                                         : le->line->length;
  itl_vi_clamp_command_cursor(le);
  itl_search_spans_clear();
  itl_g_tty_should_refresh_text = true;

  ITL_STRING_FREE(original);
  ITL_STRING_FREE(display);
  ITL_STRING_FREE(preview);
  ITL_CHAR_BUF_FREE(status);

  return result;
}

/* The one-key forms of an operator and a motion, such as x for dl. */
typedef struct
{
  uint8_t byte;
  itl_vi_operator_kind op;
  int motion;
} itl_vi_shortcut;

static const itl_vi_shortcut itl_vi_shortcuts[] = {
    {'x', ITL_VI_OP_DELETE, 'l'}, {'X', ITL_VI_OP_DELETE, 'h'},
    {'D', ITL_VI_OP_DELETE, '$'}, {'C', ITL_VI_OP_CHANGE, '$'},
    {'s', ITL_VI_OP_CHANGE, 'l'},
};

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

      itl_vi_reset_pending();
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
    itl_vi_reset_pending();
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
    if (itl_le_await_chord(le, ITL_PREFIX_VI_REGISTER, byte) &&
        ITL_READ_BYTE(&register_byte))
    {
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
      itl_utf8_t find_char = itl_vi_read_find_char(le, byte);
      itl_vi_operator_motion(le, op, (int) byte, find_char, count);
    }

    itl_vi_reset_pending();
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
    le->cursor_position = itl_le_line_end_of(le, le->cursor_position);
    itl_vi_begin_insert(true, 'A');
    break;

  case 'S': itl_vi_operator_line(le, ITL_VI_OP_CHANGE, 'c', count); break;

  case 'r': {
    uint8_t replace_byte;
    if (itl_le_await_chord(le, ITL_PREFIX_VI_REPLACE, byte) &&
        ITL_READ_BYTE(&replace_byte) &&
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
  case 'P':
    itl_g_vi_last_change.kind = ITL_VI_CHANGE_PASTE;
    itl_g_vi_last_change.is_paste_before = (byte == 'P');
    itl_g_vi_last_change.repeat_count = count;
    itl_vi_do_paste(le, byte == 'P', count);
    break;

  case 'u':
    itl_undo_close_insert_run();
    if (itl_undo_pop(le)) {
      itl_vi_clamp_command_cursor(le);
    }
    break;

  case '.': itl_vi_repeat_last_change(le); break;

  case 'v':
    itl_vi_reset_pending();
    return itl_vi_visual_loop(le, false);

  case 'V':
    itl_vi_reset_pending();
    return itl_vi_visual_loop(le, true);

  case ':':
    itl_vi_reset_pending();
    return itl_vi_ex_command(le);

  case '/': {
    tl_status_code select_code = TL_SUCCESS;
    int after_search = itl_history_select(le, &select_code);
    itl_g_tty_should_refresh_text = true;
    itl_le_tty_refresh(le);
    itl_vi_reset_pending();
    if (select_code != TL_SUCCESS) {
      return select_code;
    }
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
  case 'T':
    itl_vi_apply_bare_motion(le, (int) byte, itl_vi_read_find_char(le, byte),
                             count);
    break;

  default: {
    size_t s;
    for (s = 0; s < ITL_COUNTOF(itl_vi_shortcuts); ++s) {
      if (itl_vi_shortcuts[s].byte == byte) {
        itl_vi_operator_motion(le, itl_vi_shortcuts[s].op,
                               itl_vi_shortcuts[s].motion, none, count);
        break;
      }
    }
    if (s == ITL_COUNTOF(itl_vi_shortcuts)) {
      itl_vi_apply_bare_motion(le, (int) byte, none, count);
    }
  } break;
  }

  itl_vi_clamp_command_cursor(le);
  itl_vi_reset_pending();
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
  itl_le_end_insertion_walk();
  itl_g_auto_pair_count = 0;

  /* A new line starts with no ghost, since the previous line's suggestion does
     not carry over, and predefined input is shown as the user's own text. */
  itl_ghost_clear();
  itl_g_hint_is_closed = false;

  /* Avoid clearing lines that don't belong to us. The incremental-append fast
     path keys off the previous render, so its state is reset with the row
     counts, otherwise the first refresh of this line could compare against the
     previous command's render. */
  itl_le_invalidate_prev_frame();
  itl_le_tty_refresh(le);

  while (true) {
    itl_idle_arm();
    if (!itl_le_wait_for_key(le)) {
      return TL_ERROR;
    }

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
      itl_vi_reset_pending();
      itl_ghost_clear();
      itl_g_tty_should_refresh_text = true;
      itl_le_tty_refresh(le);
      continue;
    }

    /* Ctrl-X waits for its chord in the editor's own wait, so the hint rows can
       name the chord and a resize or the idle hook is still served. The parser
       then reads the chord, and an unbound key starts the next key. */
    if (input_byte == 24 && !itl_le_await_chord(le, ITL_PREFIX_CTRL_X, 24)) {
      return TL_ERROR;
    }

    input_type = itl_esc_parse(input_byte);
    itl_g_tty_plain_append_pending = false;
    bool is_auto_pair_kept = false;

    /* Only a key that reaches itl_le_key_handle directly can continue a kill,
       a yank, or a last-argument walk. */
    if (input_type == TL_KEY_CHAR ||
        (input_type & TL_MASK_KEY) == TL_KEY_PASTE_BEGIN ||
        (input_type & TL_MASK_KEY) == TL_KEY_HISTORY_SEARCH ||
        input_byte == 6 || itl_g_edit_mode == TL_EDIT_MODE_VI_COMMAND ||
        itl_g_edit_mode == TL_EDIT_MODE_VI_VISUAL || input_byte == 22)
    {
      itl_g_le_action = ITL_LE_ACTION_NONE;
    }

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
      itl_le_end_insertion_walk();
      itl_g_is_reading_paste = true;
      itl_le_read_paste(le);
      itl_g_is_reading_paste = false;
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
      tl_status_code select_code = TL_SUCCESS;
      int after_search = itl_history_select(le, &select_code);
      /* Redraw the restored line first so the search view is cleared even when
         the terminating key submits or only moves the cursor. */
      itl_g_tty_should_refresh_text = true;
      itl_le_tty_refresh(le);
      if (select_code != TL_SUCCESS) {
        return itl_le_finish_input(le, select_code);
      }
      if (after_search != TL_KEY_UNKN) {
        code = itl_le_key_handle(le, after_search);
        if (code != TL_SUCCESS) {
          return itl_le_finish_input(le, code);
        }
      }
    } else if (input_byte == 22 && itl_g_edit_mode == TL_EDIT_MODE_EMACS) {
      itl_ghost_clear();
      code = itl_emacs_multicursor_loop(le);
      if (code != TL_SUCCESS) {
        return itl_le_finish_input(le, code);
      }
    } else if (input_byte == 22 && itl_g_edit_mode == TL_EDIT_MODE_VI_COMMAND) {
      itl_ghost_clear();
      code = itl_vi_block_loop(le, itl_g_edit_mode);
      if (code != TL_SUCCESS) {
        return itl_le_finish_input(le, code);
      }
    } else if (itl_g_edit_mode == TL_EDIT_MODE_VI_COMMAND ||
               itl_g_edit_mode == TL_EDIT_MODE_VI_VISUAL)
    {
      itl_ghost_clear();
      code = itl_vi_command_dispatch(le, input_byte, input_type);
      if (code != TL_SUCCESS) {
        return itl_le_finish_input(le, code);
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
      if (!is_tab && !accepts_ghost) {
        itl_ghost_clear();
      }
      /* A backspace erases before the inserted closers, so they stay counted.
         Between an empty pair it deletes both halves. */
      is_auto_pair_kept = (input_type & TL_MASK_KEY) == TL_KEY_BACKSPACE;
      if (input_type == TL_KEY_BACKSPACE && itl_le_auto_pair_erase(le)) {
        itl_g_le_action = ITL_LE_ACTION_NONE;
        itl_g_last_control = input_type;
        itl_g_tty_should_refresh_text = true;
        code = TL_SUCCESS;
      } else {
        code = itl_le_key_handle(le, input_type);
      }
      if (code != TL_SUCCESS) {
        return itl_le_finish_input(le, code);
      }
      /* After tab or a word-wise accept grew the line, offer a fresh ghost for
         the rest of the suggestion. */
      bool is_word_accept =
          accepts_ghost && (input_type & (TL_MOD_CTRL | TL_MOD_ALT)) != 0;
      if ((is_tab || is_word_accept) &&
          le->line->length > line_length_before_key)
      {
        itl_ghost_update(le);
      }
    } else if (itl_le_auto_pair_type(le, input_byte)) {
      is_auto_pair_kept = true;
      itl_le_end_insertion_walk();
      itl_g_tty_should_refresh_text = true;
      itl_ghost_update(le);
    } else {
      is_auto_pair_kept = true;
      itl_le_end_insertion_walk();
      itl_utf8_t appended_character = itl_utf8_parse(input_byte);
      itl_g_tty_plain_append_pending =
          le->cursor_position == le->line->length;
      itl_g_tty_plain_append_width = itl_char_line_width(appended_character);
      if (itl_le_insert(le, appended_character) && le->cursor_position > 0) {
        itl_g_tty_plain_append_width =
            itl_line_char_width(le->line->chars, le->cursor_position - 1);
      }
      itl_g_tty_should_refresh_text = true;
      /* Recompute the ghost for the token the new character extended. */
      itl_ghost_update(le);
    }

    if (!is_auto_pair_kept) {
      itl_g_auto_pair_count = 0;
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
  tl_status_code status = itl_history_load_from_file(file_path);

  if (status != TL_SUCCESS) return status;
  if (!itl_history_ensure_read_buffer()) {
    int history_errno = errno == ENOENT ? ENOENT : EIO;
    itl_history_offsets_reset();
    itl_g_last_history_event_number = 0;
    itl_g_history_file_size = 0;
    itl_g_history_ends_with_newline = true;
    itl_g_history_file_is_bad = history_errno != ENOENT;
    errno = history_errno;
    return TL_ERROR;
  }

  return TL_SUCCESS;
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
}

TL_DEF tl_status_code tl_history_dump(const char *file_path)
{
  return itl_history_dump_to_file(file_path);
}

TL_DEF size_t tl_utf8_strlen(const char *utf8_str)
{
  return tl_utf8_strnlen(utf8_str, (size_t) -1);
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
