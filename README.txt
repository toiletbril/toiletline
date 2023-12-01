toiletline
----------------
Tiny, crossplatform, single-header shell library, meant to replace a small
subset of GNU Readline, and work on both Linux and Windows.

NOTE: On Windows, UTF-8 locale feature is required for proper multibyte
character support.


Current features
----------------
* UTF-8 support;
* Emacs controls;
* Multi-line input;
* In-memory history;
* MIT License.


Documentation
----------------
Before you include this file in C or C++ file, define
"TOILETLINE_IMPLEMENTATION" to create the implementation.

If you want to use this library in other languages, you will need to make a .c
file which creates implementation and includes the library. Compile it to object
file (`-c` flag in gcc/clang) and link your program against it.


Configuration macros
--------
These should be defined before including, in the same file with implementation
macro.

* TL_HISTORY_MAX_SIZE configures maximum history size;
* TL_NO_SUSPEND prevents Ctrl-Z from sending SIGTSTP to the terminal. Since
  Windows does not have this signal, exit(1) will be called instead;
* TL_SIZE_USE_ESCAPES forces to use escape codes instead of syscalls for
  retrieving terminal size;
* TL_ASSERT configures function used for assertions;
* TL_MALLOC, TL_REALLOC, TL_FREE configure functions used for memory
  allocation;
* TL_ABORT sets function that will be called on a failed allocation;
* TL_NO_ABORT disables checks for failed memory allocations.


Definitions
--------
Int functions can return error codes. They are always above 0, and defined as:
* TL_ERROR;
* TL_ERROR_SIZE;
* TL_ERROR_ALLOC.


int tl_last_control;
--------
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
* TL_KEY_INTERRUPT (Ctrl-C).


int tl_init(void);
--------
Initialize toiletline and put terminal in semi-raw mode.
Returns TL_SUCCESS.


int tl_exit(void);
--------
Exit toiletline and restore terminal state.
Returns TL_SUCCESS.


int tl_readline(char *line_buffer, size_t size, const char *prompt);
--------
Read a line.

To support multi-byte characters and null at the end, size needs to be at least
2 or more. Submitted input will be written to *line_buffer as a null-terminated
string. After the size is exhausted, character inputs will be ignored.

Beware of characters like `\t` (Tab) and such, as they may break cursor
position. Althrough they are considered as a single character, they occupy more
than one space.

All control sequences except Enter and Interrupt will be handled internally.

Returns:
* TL_PRESSED_ENTER on Enter;
* TL_PRESSED_INTERRUPT on Ctrl-C;
* TL_PRESSED_EOF on Ctrl-D when there is no characters on the line;
* TL_PRESSED_SUSPEND on Ctrl-Z.

void tl_setline(const char *str);
--------
Predefine input for `tl_readline()`. Does not work for `tl_getc()`.


int tl_getc(char *char_buffer, size_t size, const char *prompt);
--------
Read a character without waiting and modify `tl_last_control`.

Returns:
* TL_SUCCESS on a character;
* TL_PRESSED_CONTROL_SEQUENCE on a control sequence (tl_last_control to check
  which one).


size_t tl_utf8_strlen(const char *utf8_str);
--------
Get the amount of characters in a UTF-8 string.

Since number of bytes can be bigger than amount of characters, regular
`strlen()` will not work, and will only return the number of bytes before \0.


Examples
----------------
For example usage, take a look at example.c and example_getc.c
