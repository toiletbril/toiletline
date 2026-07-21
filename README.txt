toiletline
----------
Small, crossplatform, single-header shell library in C99, meant to replace and
kill GNU Readline, and work on both macOS, Linux and Windows out of the box. It
uses native API on both platforms and relies on VT100 terminal escape
sequences.


Notes on Windows
----------------
The library uses the ENABLE_VIRTUAL_TERMINAL_PROCESSING switch to enable
native VT100 escape sequence processing for conhost.exe and such. That
requires a Windows 10 build of 10586 or later. This is not crucial. The
library can still be used with a VT100 compatible terminal with its own
terminal processing, like Windows Terminal or Alacritty.
Native Windows consoles do not need to set TERM for inline suggestions and
other decorations after virtual terminal processing has been enabled.

UTF-8 locale feature is required for proper multibyte character support.


Current features
----------------
* MIT License;
* UTF-8 support;
* Wide character (CJK and emoji) width handling;
* Line wrapping;
* Multiline editing;
* Reverse history search;
* Emacs and Vi controls;
* Block selection across lines (multiple cursors);
* Shared undo and redo;
* Rich configuration;
* Persistent history;


Notes on usage
--------------
Before you include this file in C or C++ file, define
`TOILETLINE_IMPLEMENTATION` to create the implementation.

If you want to use this library in other languages, you will need to make a .c
file which creates implementation and includes the library. Compile it to
object file (`-c` flag in gcc/clang) and link your program against it.

Assume that every function is thread-unsafe until stated otherwise. UI probably
shouldn't be modified by multiple threads anyway.


Keys and editing
----------------
Most Emacs style motions work, including Ctrl-A and Ctrl-E for the start and end
of a line, Ctrl-K and Ctrl-U to kill text, and the arrow keys with Ctrl held for
word motion.

Ctrl-R starts a reverse incremental history search. Enter accepts the
highlighted match into the line and leaves the search without running it.
Ctrl-G cancels the search, and the line is restored to its state from before
the search began.

A vi editing mode is selected through tl_set_edit_mode. Each line begins in
insert mode, where the Emacs controls and the history search stay available.
Esc enters command mode, which provides the motions, the operators d, c, and y
with counts, the named registers, the change repeat on the dot key, and a
visual selection started with v. The undo ring is shared by the vi u key and
the Emacs Ctrl-_ binding, and a redo runs on Ctrl-^ or on Ctrl-R in command
mode. The incremental history search is reached through the slash key in
command mode. An ex command is opened with the colon key, where q, q!, wq, wq!,
x, and quit end the input with TL_PRESSED_QUIT. The cursor shape follows the
submode, a bar in insert, a block in command, and an underline in visual or
while an operator waits for its motion.

Ctrl-V in command mode enters a block selection, a column rectangle across
several logical lines. The keys d and x delete the rectangle, and I, c, or C
insert the typed text at the left column of every selected line, so several
lines are indented at once. The arrows and h, j, k, and l move within the
rectangle.

Ctrl-V in emacs mode enters a multi-cursor edit and selects nothing. A cursor
sits at the same column on every spanned logical line, and Up or Down grows the
span by one line. A typed character is inserted at every cursor at once, and
Backspace erases at every cursor. Ctrl-G leaves the multi-cursor edit.

A line can span several rows. Alt-Enter inserts a newline and keeps editing. A
backslash at the end of a line followed by Enter also continues onto a new row,
fish style, without drawing a second prompt. When the line is submitted each
backslash that is followed by a newline is removed together with that newline,
so the two rows are joined like a POSIX shell. Newlines that come from
Alt-Enter or a paste stay in the returned string.

Pasted text is read through bracketed paste when the terminal supports it, so
the newlines inside a paste are inserted instead of submitting the line. A
terminal without bracketed paste submits each pasted line separately.


Configuration macros
--------------------
These should be defined before including, in the same file with implementation
macro.

* TL_USE_STDIO can be defined to use <stdio.h> functions instead of raw
  `read()`, `open()` and etc.
* TL_HISTORY_MAX_SIZE configures maximum history size;
* ITL_HISTORY_ENTRY_MAX_BYTES caps the byte length of a single history entry;
  longer entries are silently dropped from history;
* TL_NO_SUSPEND prevents Ctrl-Z from sending `SIGTSTP` to the terminal. Note
  that Windows does not have this signal, and if this macro is not defined,
  Ctrl-Z will call `exit(0)`;
* TL_SIZE_USE_ESCAPES forces to use escape codes instead of native API to
  retrieve terminal size;
* TL_DEF and ITL_DEF are put before every definition, public and internal
  respectively.
* TL_ASSERT configures function used for assertions;
* TL_MALLOC, TL_REALLOC, TL_FREE configure functions used for memory
  allocation;
* TL_ABORT sets function that will be called on a failed allocation;
* TL_NO_ABORT disables checks for failed memory allocations;
* TL_SEE_BYTES forces tl_get_input() to output terminal codes of pressed keys
  instead of processing and echoing them. This is useful for debugging.
* TL_DEBUG can be defined to output various debug information at runtime.


Definitions
-----------
Some functions return their status. Errors are always below 0. All statuses are
defined in the tl_status_code enum. Each function's return code is documented
here.


int tl_last_control_sequence(void);
-----------------------------------
Last pressed control sequence.

Related values:

Bit masks:
* TL_MASK_KEY;
* TL_MASK_MOD.

Possible mod values:
* TL_MOD_CTRL;
* TL_MOD_SHIFT;
* TL_MOD_ALT.

Possible key values:
* TL_KEY_CHAR;
* TL_KEY_UNKN;
* TL_KEY_UP;
* TL_KEY_DOWN;
* TL_KEY_RIGHT;
* TL_KEY_LEFT;
* TL_KEY_HISTORY_END (Alt->);
* TL_KEY_HISTORY_BEGINNING (Alt-<);
* TL_KEY_END;
* TL_KEY_HOME;
* TL_KEY_ENTER;
* TL_KEY_BACKSPACE;
* TL_KEY_DELETE;
* TL_KEY_KILL_LINE (Ctrl-K);
* TL_KEY_KILL_LINE_BEFORE (Ctrl-U);
* TL_KEY_TAB;
* TL_KEY_CLEAR (Ctrl-L);
* TL_KEY_SUSPEND (Ctrl-Z);
* TL_KEY_EOF (Ctrl-D);
* TL_KEY_INTERRUPT (Ctrl-C);
* TL_KEY_HISTORY_SEARCH (Ctrl-R);
* TL_KEY_UNDO (Ctrl-_);
* TL_KEY_REDO (Ctrl-^);
* TL_KEY_PASTE_BEGIN (bracketed paste marker, surfaced through
  tl_last_control_sequence when tl_get_character reads it).


tl_status_code tl_init(void);
-----------------------------
Initialize toiletline and put terminal in raw mode.

Returns `TL_SUCCESS` or `TL_ERROR` on errors.


tl_status_code tl_enter_raw_mode(void);
---------------------------------------
Put the terminal into raw mode without doing anything else.

Returns `TL_SUCCESS` or `TL_ERROR` on errors.


tl_status_code tl_exit(void);
-----------------------------
Exit toiletline, restore terminal state, and free internal memory.

Returns `TL_SUCCESS` or `TL_ERROR` on errors.


tl_status_code tl_exit_raw_mode(void);
--------------------------------------
Restore the terminal state without doing anything else.

Returns `TL_SUCCESS` or `TL_ERROR` on errors.


tl_status_code tl_get_input(char *line_buffer, size_t size, const char *prompt);
-------------------------------------------------------------------------------
Get input from user.

To support multi-byte characters and null at the end, size needs to be at least
2. It must not exceed the platform maximum string length, 4095 bytes on POSIX
and 8191 bytes on Windows, or the call traps. Submitted input will be written to
`*line_buffer` as a null-terminated string. After the size is exhausted,
character inputs will be ignored.

The submitted string can contain embedded newlines from multiline editing or a
paste, so the buffer is not always a single line.

All control sequences except Enter, EOF, Interrupt, Suspend, the vi :q quit,
and Tab without a completion callback will be handled internally.

Returns:
* TL_PRESSED_ENTER on Enter;
* TL_PRESSED_INTERRUPT on Ctrl-C;
* TL_PRESSED_EOF on Ctrl-D when there is no characters on the line;
* TL_PRESSED_SUSPEND on Ctrl-Z, only when TL_NO_SUSPEND is defined; the
  default build handles Ctrl-Z internally;
* TL_PRESSED_TAB on Tab when no completion callback is registered;
* TL_PRESSED_QUIT when the vi :q command quits the shell.
* `TL_ERROR` on errors.


void tl_set_predefined_input(const char *str);
----------------------------------------------
Predefine input for `tl_get_input()`. Does not work for `tl_get_character()`.


tl_status_code tl_get_character(char *char_buffer, size_t size, const char *prompt);
------------------------------------------------------------------------------------
Read a character without waiting and modify `tl_last_control_sequence`.

`size` must be between 2 and 5: one UTF-8 character of up to four bytes plus
the null terminator. A larger buffer traps the call.

Returns:
* TL_SUCCESS on a character;
* TL_PRESSED_CONTROL_SEQUENCE on a control sequence (`tl_last_control_sequence`
  to check which one).
* `TL_ERROR` on errors.


tl_status_code tl_history_load(const char *file_path);
------------------------------------------------------
Load history from a file.

If loading a history file fails for any reason other than a non-existent file,
`tl_history_dump()` will be a no-op to avoid overwriting wrong files. Finding a
control byte that is not whitespace while loading the file is treated as an
error as well (that means you accidentaly loaded a binary file T__T).

Returns:
* `TL_SUCCESS`;
* `TL_ERROR` on errors. Sets `errno` to `EINVAL` if a previous call to
  `tl_history_load()` was attempted on a binary file, sets `errno` to respective
  values on other failures.


tl_status_code tl_history_dump(const char *file_path);
------------------------------------------------------
Dump history to a file, overwriting it. Should be called before tl_exit()!

Returns:
* `TL_SUCCESS`;
* `TL_ERROR` on errors. Sets `errno` to `EINVAL` if a previous call to
  `tl_history_load()` was attempted on a binary file, sets `errno` to respective
  values on other failures.


size_t tl_utf8_strlen(const char *utf8_str);
--------------------------------------------
Get the amount of characters in a UTF-8 string.

Since number of bytes can be bigger than amount of characters, regular
`strlen()` will not work, and will only return the number of bytes before \0.


size_t tl_utf8_strnlen(const char *utf8_str, size_t byte_count);
-----------------------------------------------------------------
Same as above, except it stops after reading `byte_count` bytes from the string.


tl_status_code tl_emit_newlines(const char *buffer);
----------------------------------------------------
Emit newlines after getting the input.

*buffer should be the buffer used in tl_get_input().

Returns `TL_SUCCESS` or `TL_ERROR` on errors.


tl_status_code tl_set_title(const char *title);
-----------------------------------------------
Sets a new title for the terminal. Returns `TL_ERROR` and does nothing if stdout
is not a tty.

Returns `TL_SUCCESS` or `TL_ERROR` on other errors.


void tl_set_complete_callback(tl_complete_fn callback);
------------------------------------------------------
Registers the completion callback the editor calls on `Tab` and while building
the ghost suggestion. The callback receives the current line and the cursor
position in codepoints and fills a `tl_completion` with the candidates, their
count, the longest common prefix, and the token span to replace. Pass `NULL` to
turn completion off, which is the default.


void tl_set_highlight_callback(tl_highlight_fn callback);
--------------------------------------------------------
Registers the syntax-highlight callback the editor calls before each redraw. The
callback receives the current line and fills a `tl_highlight` with the colored
spans, each one a start and end in codepoints and an SGR escape. Pass `NULL` to
turn highlighting off, which is the default.


void tl_set_ghost_enabled(int enabled);
---------------------------------------
Turns the dimmed ghost suggestion shown ahead of the cursor on or off. It is on
by default. When it is off neither the completion callback nor the history fills
it, so a host that wants no inline hint passes 0.


void tl_set_edit_mode(int mode);
--------------------------------
Selects the line editing mode, one of the tl_edit_mode values. TL_EDIT_MODE_EMACS
is the default, and any vi value starts each fresh line in vi insert mode. The
choice is read on the next line, so a host toggles it before the prompt to match
a `set -o vi` or `set -o emacs` change.

The values are TL_EDIT_MODE_EMACS, TL_EDIT_MODE_VI_INSERT, TL_EDIT_MODE_VI_COMMAND,
and TL_EDIT_MODE_VI_VISUAL.


Examples
--------
For example usage, take a look at `example.c` and `example_character.c`
