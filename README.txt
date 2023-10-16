toiletline
----------------
Single-header library for immediate shell, meant to replace a small subset of
GNU Readline, and work on both Linux and Windows.

All features like character echo and key handling are implemented from scratch.
NOTE: On Windows, beta UTF-8 feature is required for proper multibyte character
support.

Any help finding bugs is appreciated.


Current features
----------------
* UTF-8 support;
* In-memory history;
* Familiar Emacs controls.

The only feature remaining for full shell experience is the multiline support,
so lines longer than terminal width might look oddly, but will still work.


Documentation
----------------
Before you include this file in C or C++ file, define
"TOILETLINE_IMPLEMENTATION" to create the implementation.

If you want to use this library in C++ or other languages, you will need to make
a .c file which creates implementation and includes the library. Compile it to
object file ('-c' flag in gcc/clang) and link your code against it.


Definitions
--------
Int functions can return error codes. They are always above 0, and defined as:
* TL_ERROR;
* TL_ERROR_SIZE;
* TL_ERROR_ALLOC.


int tl_last_control;
--------
Last pressed control sequence. Note that last control sequence for
tl_readline() can only be Enter or Interrupt.

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
* TL_KEY_END;
* TL_KEY_HOME;
* TL_KEY_ENTER;
* TL_KEY_BACKSPACE;
* TL_KEY_DELETE;
* TL_KEY_KILL_LINE;
* TL_KEY_TAB;
* TL_KEY_SUSPEND;
* TL_KEY_EOF;
* TL_KEY_INTERRUPT.

They can be tested with bitwise AND (&).


int tl_init(void);
--------
Initialize toiletline and put terminal in semi-raw mode.
Returns TL_SUCCESS.


int tl_readline(char *line_buffer, size_t size, const char *prompt);
--------
Read a line.

To support multi-byte characters and null at the end, size needs to be at least
2 or more. Submitted input will be written to *line_buffer as a null-terminated
string. After the size is exhausted, character inputs will be ignored.

Beware of characters like '\t' (Tab) and such, as they may break cursor
position. Althrough they are considered as a single character, they occupy more
than one space.

All control sequences except Enter and Interrupt will be handled internally.

Returns:
* TL_PRESSED_ENTER on Enter;
* TL_PRESSED_INTERRUPT on Ctrl-C;


int tl_getc(char *char_buffer, size_t size, const char *prompt);
--------
Read a character (without waiting for Enter).

Returns:
* TL_SUCCESS on a character;
* TL_PRESSED_CONTROL_SEQUENCE on a control sequence (tl_last_control to check
  which one).


int tl_exit(void);
--------
Exit and deallocate memory.
Returns TL_SUCCESS.


size_t tl_utf8_strlen(const char *utf8_str);
--------
Get the amount of characters in a UTF-8 string.

Since number of bytes can be bigger than amount of characters, regular strlen
will not work, and will only return the number of bytes before \0.


Examples
----------------
For example usage, take a look at example.c and example_getc.c