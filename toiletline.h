/**
 *  toiletline 0.2.0
 *  Raw CLI shell implementation
 *  Meant to be a tiny replacement of GNU Readline :3
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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TOILETLINE_H_
#define TOILETLINE_H_

#include <stddef.h>

// for no asserts, define TL_ASSERT before including
#ifndef TL_ASSERT
    #include <assert.h>
    #define TL_ASSERT(boolval) assert(boolval)
#endif // TL_ASSERT

// To use different allocation functions, define these before including.
#ifndef TL_MALLOC
    #define TL_MALLOC(size) malloc(size)
    #define TL_CALLOC(count, size) calloc(count, size)
    #define TL_REALLOC(block, size) realloc(block, size)
#endif

/**
 *  Zero is reserved as a successful result.
 */
#define TL_SUCCESS 0
/**
 *  Codes which are returned from read functions.
 */
#define TL_PRESSED_ENTER -1
#define TL_PRESSED_CTRLC -2
#define TL_PRESSED_CONTROL_SEQUENCE -3
/**
 *  Codes above 0 are errors.
 */
#define TL_ERROR 1
#define TL_ERROR_SIZE 2
#define TL_ERROR_ALLOC 3
 *  Initialize toiletline, enter raw mode.
 *
 *  Returns 1 on success.
 */
int tl_init(void);
/**
 *  Exit toiletline and free history.
 *
 *  Returns 1 on success.
 */
int tl_exit(void);
/**
 *  Read one character without waiting for Enter.
 *  This does not append to history.
 *
 *  Returns:
 *     0 on success.
 *     1 when exited.
 *    -1 internal error.
 */
int tl_getc(char *char_buffer, size_t size, const char *prompt);
/**
 *  Read one line.
 *
 *  Returns:
 *     0 on success.
 *     1 when exited.
 *    -1 internal error.
 */
int tl_readline(char *line_buffer, size_t size, const char *prompt);
/**
 *  Returns the number of UTF-8 characters.
 *
 *  Since number of bytes can be bigger than amount of characters,
 *  regular strlen will not work, and will only return the number of bytes before \0.
 */
size_t tl_utf8_strlen(const char *utf8_str);

#endif // TOILETLINE_H_

#ifdef TOILETLINE_IMPLEMENTATION

// general debug messages
#ifdef TL_DEBUG
    #define ITL_DBG(message, val) \
        printf("%s: %d\n", message, val)
#else
    #define ITL_DBG(message, val)
#endif

inline static int itl_enter_raw_mode(void)
{
#ifdef _WIN32
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hInput == INVALID_HANDLE_VALUE)
        return 0;

    // NOTE:
    // ENABLE_VIRTUAL_TERMINAL_INPUT seems to not work on older versions of Windows
    DWORD mode = ENABLE_EXTENDED_FLAGS | ENABLE_VIRTUAL_TERMINAL_INPUT | ENABLE_QUICK_EDIT_MODE;

    if (!SetConsoleMode(hInput, mode))
        return 0;

    if (_setmode(STDIN_FILENO, _O_BINARY) == -1)
        return 0;

    if (SetConsoleCP(CP_UTF8) == 0)
        return 0;
#else // Linux
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) != 0)
        return 0;

    struct termios raw = term;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0)
        return 0;

#endif // Linux
    return 1;
}

inline static int itl_exit_raw_mode(void)
{
#ifdef _WIN32
    HANDLE h_input = GetStdHandle(STD_INPUT_HANDLE);

    if (h_input == INVALID_HANDLE_VALUE)
        return 0;

    DWORD mode;

    if (!GetConsoleMode(h_input, &mode))
        return 0;

    mode |= ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT;

    if (!SetConsoleMode(h_input, mode))
        return 0;
#else // Linux
    struct termios term;

    if (tcgetattr(STDIN_FILENO, &term) != 0)
        return 0;

    term.c_lflag |= ICANON | ECHO;
    term.c_oflag |= OPOST;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) != 0)
        return 0;
#endif // Linux
    return 1;
}

inline static void itl_handle_interrupt(int sign)
{
    signal(sign, SIG_IGN);

    fputc('\n', stdout);
    printf("Interrupted.\n");
    fflush(stdout);

    exit(0);
}

static int ITL_GLOBAL_ALLOC_COUNT = 0;

// TODO: Remove this, since allocations are mostly figured out
// Messages about allocations.
#ifdef TL_DEBUG_ALLOC
    #define ITL_ALLOC_DBG(message) \
        printf("\n%s, alloc: %d\n", message, ITL_GLOBAL_ALLOC_COUNT)
#else
    #define ITL_ALLOC_DBG(message)
#endif // TL_DEBUG_ALLOC

inline static void *itl_malloc(size_t size)
{
    ITL_GLOBAL_ALLOC_COUNT += 1;

    ITL_ALLOC_DBG("malloc");

    return malloc(size);
}

inline static void *itl_calloc(size_t count, size_t size)
{
    ITL_ALLOC_DBG("calloc");

    ITL_GLOBAL_ALLOC_COUNT += 1;

    return calloc(count, size);
}

inline static void *itl_realloc(void *block, size_t size)
{
    ITL_ALLOC_DBG("realloc");

    if (block == NULL)
        ITL_GLOBAL_ALLOC_COUNT += 1;

    return realloc(block, size);
}

#define ITL_FREE(ptr)                     \
    do {                                  \
        if (ptr != NULL) {                \
            ITL_GLOBAL_ALLOC_COUNT -= 1;  \
            ITL_ALLOC_DBG("free");        \
            free(ptr);                    \
        }                                 \
    } while (0)

#define ITL_MAX(type, i, j) ((((type)i) > ((type)j)) ? ((type)i) : ((type)j))

typedef struct itl_utf8 itl_utf8_t;

struct itl_utf8
{
    size_t size;
    uint8_t bytes[4];
};

static itl_utf8_t itl_utf8_new(const uint8_t *bytes, uint8_t length)
{
    itl_utf8_t utf8_char;
    utf8_char.size = length;

    for (uint8_t i = 0; i < length; i++)
        utf8_char.bytes[i] = bytes[i];

    return utf8_char;
}

typedef struct itl_char itl_char_t;

// UTF-8 string list node
struct itl_char
{
    itl_utf8_t c;
    itl_char_t *next;
    itl_char_t *prev;
};

static itl_char_t *itl_char_alloc(void)
{
    itl_char_t *ptr = (itl_char_t *)itl_malloc(sizeof(itl_char_t));

    ptr->next = NULL;
    ptr->prev = NULL;

    return ptr;
}

static void itl_char_copy(itl_char_t *dst, itl_char_t *src)
{
    memcpy(dst, src, sizeof(itl_char_t));
}

static void itl_char_free(itl_char_t *c)
{
    ITL_FREE(c);
}

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

//  0 if strings are the same,
// -1 if they are not.
static int itl_string_cmp(itl_string_t *str1, itl_string_t *str2)
{
    if (str1->size != str2->size)
        return -1;

    itl_char_t *str1_c = str1->begin;
    itl_char_t *str2_c = str2->begin;

    while (str1_c != NULL) {
        if (sizeof(str1_c->c) != sizeof(str1_c->c))
            return -1;

        int cmp_result = memcmp(str1_c->c.bytes, str2_c->c.bytes, 4);

        if (cmp_result != 0)
            return -1;

        str1_c = str1_c->next;
        str2_c = str2_c->next;
    }

    return 0;
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

inline static void itl_string_free(itl_string_t *str)
{
    itl_string_clear(str);

    ITL_FREE(str);
}

// 0 - success, -1 - *str did not fit in *c_str
// This function should not result in any errors.
static int itl_string_to_cstr(itl_string_t *str, char *c_str, size_t size)
{
    itl_char_t *c = str->begin;
    size_t i = 0;

    // NOTE: This can cause bugs if (size - i - 1) wraps around
    while (c && size - i > c->c.size) {
        for (size_t j = 0; j != c->c.size; ++j)
            c_str[i++] = c->c.bytes[j];

        if (c != c->next)
            c = c->next;
    }

    c_str[i] = '\0';

    // If *c exists, then size was exceeded. This should not happen ever.
    if (c)
        return -1;

    return 0;
}

// TODO: Inserts \n to fit multiple columns
inline static int
itl_string_to_tty_cstr(itl_string_t *str, char *c_str, size_t size,
                       size_t cols, size_t pr_len, size_t *cur_offset)
{
    (void)cols;
    (void)pr_len;
    (void)cur_offset;

    return itl_string_to_cstr(str, c_str, size);
}

static itl_string_t *ITL_LINE_BUF = NULL;

typedef struct itl_le itl_le_t;

// Line editor
struct itl_le
{
    itl_string_t *line;
    itl_char_t *cur_char;
    size_t cur_pos;
    size_t cur_col;
    int h_item_sel;
    char *out_buf;
    size_t out_size;
    const char *prompt;
};

static itl_le_t itl_le_new(itl_string_t *line_buf, char *out_buf, size_t out_size, const char *prompt)
{
    itl_le_t le = {
        .line = line_buf,
        .cur_char = NULL,
        .cur_pos = 0,
        .cur_col = 0,
        .h_item_sel = -1,
        .out_buf = out_buf,
        .out_size = out_size,
        .prompt = prompt,
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
            if (le->cur_pos == 0)
                return 0;
            else if (le->cur_char) {
                to_free = le->cur_char->prev;
                le->cur_pos -= 1;
            }
            else if (le->cur_pos == le->line->length) {
                to_free = le->line->end;
                le->line->end = le->line->end->prev;
                le->cur_pos -= 1;
            }
        }
        else {
            if (le->cur_pos == le->line->length)
                return 0;
            else if (le->cur_char) {
                to_free = le->cur_char;
                le->cur_char = le->cur_char->next;
                if (le->cur_pos == le->line->length - 1)
                    le->line->end = le->line->end->prev;
            }
        }

        if (le->cur_pos == 0)
            le->line->begin = le->line->begin->next;

        if (to_free->prev)
            to_free->prev->next = to_free->next;
        if (to_free->next)
            to_free->next->prev = to_free->prev;

        le->line->size -= to_free->c.size;
        le->line->length -= 1;

        itl_char_free(to_free);
    }

    return 1;
}

// Inserts character at cursor position
static int itl_le_putc(itl_le_t *le, const itl_utf8_t ch)
{
    if (le->line->size >= le->out_size - 1)
        return 0;

    if (le->out_size - 1 < le->line->size + ch.size)
        return 0;

    itl_char_t *new_c = itl_char_alloc();
    new_c->c = ch;

    if (le->cur_pos == 0) {
        new_c->next = le->line->begin;
        if (le->line->begin)
            le->line->begin->prev = new_c;
        le->line->begin = new_c;
        if (le->line->end == NULL)
            le->line->end = new_c;
    }
    else if (le->cur_pos == le->line->length) {
        if (le->line->end)
            le->line->end->next = new_c;
        new_c->prev = le->line->end;
        le->line->end = new_c;
    }
    else if (le->cur_char) {
        new_c->next = le->cur_char;
        if (le->cur_char->prev) {
            le->cur_char->prev->next = new_c;
            new_c->prev = le->cur_char->prev;
        }
        le->cur_char->prev = new_c;
    }

    le->line->length += 1;
    le->line->size += ch.size;
    le->cur_pos += 1;

    return 1;
}

static void itl_le_move_right(itl_le_t *le, size_t steps)
{
    size_t i = 0;
    while (i++ < steps) {
        if (le->cur_char) {
            le->cur_char = le->cur_char->next;
            le->cur_pos += 1;
        }
        else
            return;
    }
}

static void itl_le_move_left(itl_le_t *le, size_t steps)
{
    size_t i = 0;
    while (i++ < steps) {
        if (le->cur_pos == 0)
            return;
        else if (le->cur_char) {
            le->cur_char = le->cur_char->prev;
            le->cur_pos -= 1;
        }
        else if (le->cur_pos == le->line->length) {
            le->cur_char = le->line->end;
            le->cur_pos -= 1;
        }
        else
            return;
    }
}

#define ITL_IS_DELIM(c) (ispunct(c) || isspace(c))

// Number of characters to get to the next word
static size_t itl_le_next_word(itl_le_t *le, int behind)
{
    itl_char_t *ch = le->cur_char;
    size_t steps = 1;

    if (ch)
        if (behind)
            ch = ch->prev;
        else
            ch = ch->next;
    else
        if (le->cur_pos == le->line->size && behind)
            ch = le->line->end;
        else
            return 0;

    while (ch) {
        if (!ITL_IS_DELIM(ch->c.bytes[0]))
            break;

        steps += 1;

        if (behind)
            ch = ch->prev;
        else
            ch = ch->next;
    }

    return steps;
}

// Number of characters to get to the next whitespace
static size_t itl_le_next_whitespace(itl_le_t *le, int behind)
{
    itl_char_t *ch = le->cur_char;
    size_t steps = 1;

    if (ch)
        if (behind)
            ch = ch->prev;
        else
            ch = ch->next;
    else
        if (le->cur_pos == le->line->size && behind)
            ch = le->line->end;
        else
            return 0;

    while (ch) {
        if (ITL_IS_DELIM(ch->c.bytes[0]))
            break;

        steps += 1;

        if (behind)
            ch = ch->prev;
        else
            ch = ch->next;
    }

    return steps;
}

// Frees line buffer's string's characters, does not free the string itself
inline static void itl_le_clear(itl_le_t *le)
{
    itl_string_clear(le->line);

    le->line->length = 0;
    le->cur_pos = 0;
}

#ifdef _WIN32
    #define ITL_READ_BYTE() _getch()
#else // Linux
    #define ITL_READ_BYTE() fgetc(stdin)
#endif // Linux

typedef struct itl_pbuf itl_pbuf_t;

// Buffer output before writing it all to stdout
struct itl_pbuf
{
  char *string;
  int size;
};

static void itl_pbuf_append(itl_pbuf_t *pbuf, const char *s, size_t size)
{
    char *n_s = itl_realloc(pbuf->string, pbuf->size + size);
    if (n_s == NULL)
        return;

    memcpy(&n_s[pbuf->size], s, size);

    pbuf->string = n_s;
    pbuf->size += size;
}

inline static void itl_pbuf_free(itl_pbuf_t *pbuf)
{
    ITL_FREE(pbuf->string);
}

// Gets you maximum terminal column width and current row
int itl_tty_size(size_t *rows, size_t *cols) {
    char buf[32];
    size_t i = 0;

    fputs("\x1b[999C", stdout);
    fputs("\x1b[6n", stdout);

    // This does not work without flushing if setvbuf was called previously
    fflush(stdout);

    while (i < sizeof(buf) - 1) {
        buf[i] = ITL_READ_BYTE();
        if (buf[i] == 'R')
            break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[')
        return 1;

    if (sscanf(&buf[2], "%zu;%zu", rows, cols) != 2)
        return 1;

    return 0;
}

static int itl_le_update_tty(itl_le_t *le)
{
    fputs("\x1b[?25l", stdout);

    itl_pbuf_t pbuf = { .string = NULL, .size = 0 };

    size_t pr_len;

    // prompt = NULL is valid
    if (le->prompt != NULL)
        pr_len = strlen(le->prompt);
    else
        pr_len = 0;

    size_t cstr_size = le->line->size + 1;

    size_t buf_size =
        ITL_MAX(size_t, cstr_size, 8) * sizeof(char) + 2;

    size_t rows, cols;
    itl_tty_size(&rows, &cols);

    size_t wrap_value = wrap_value = (le->line->length + pr_len) / ITL_MAX(size_t, 1, cols);
    size_t wrap_cursor_pos = le->cur_pos + pr_len - wrap_value * cols + 1;

    ITL_DBG("len", le->line->length);
    ITL_DBG("cols", cols);
    ITL_DBG("wrap_value", wrap_value);
    ITL_DBG("wrap_cursor_pos", wrap_cursor_pos);

    char *buf = (char *)itl_malloc(buf_size);
    if (!buf)
        return 0;

    if (wrap_value > 0 && wrap_cursor_pos > 1) {
        snprintf(buf, buf_size, "\x1b[%zuF", wrap_value);
        itl_pbuf_append(&pbuf, buf, strlen(buf));
    }

    itl_pbuf_append(&pbuf, "\r", 1);
    itl_pbuf_append(&pbuf, "\x1b[0K", 4);

    itl_pbuf_append(&pbuf, le->prompt, pr_len);

    size_t cur_offset = 0;
    itl_string_to_tty_cstr(le->line, buf, buf_size, cols, pr_len, &cur_offset);

    itl_pbuf_append(&pbuf, buf, cstr_size + cur_offset);

    snprintf(buf, buf_size, "\x1b[%zuG", wrap_cursor_pos + cur_offset);
    itl_pbuf_append(&pbuf, buf, strlen(buf));

    write(STDOUT_FILENO, pbuf.string, pbuf.size);

    itl_pbuf_free(&pbuf);
    ITL_FREE(buf);

    fputs("\x1b[?25h", stdout);

    return fflush(stdout);
}

#define TL_HISTORY_INIT_SIZE 16
#define ITL_HISTORY_MAX_SIZE 128

static itl_string_t **ITL_HISTORY = NULL;
static int ITL_HIST_SIZE = TL_HISTORY_INIT_SIZE;
static int ITL_HIST_INDEX = 0;

static void itl_history_alloc(void)
{
    ITL_HISTORY = (itl_string_t **)
        itl_calloc(TL_HISTORY_INIT_SIZE, sizeof(itl_string_t *));
}

static void itl_history_free(void)
{
    for (int i = 0; i < ITL_HIST_INDEX; ++i)
        itl_string_free(ITL_HISTORY[i]);

    ITL_FREE(ITL_HISTORY);
}

// Copies string to global history.
// Allocates memory for a new string.
static int itl_history_append(itl_string_t *str)
{
    if (str->length <= 0)
        return 0;

    if (ITL_HIST_SIZE >= ITL_HISTORY_MAX_SIZE)
        return 0;

    // Allocate more memory if needed.
    if (ITL_HIST_INDEX >= ITL_HIST_SIZE && ITL_HIST_SIZE < ITL_HISTORY_MAX_SIZE) {
        ITL_HIST_SIZE *= 2;
        ITL_HISTORY = (itl_string_t **)
            itl_realloc(ITL_HISTORY, ITL_HIST_SIZE * sizeof(itl_string_t *));
    }

    if (ITL_HISTORY)

    // Avoid adding the same string to history.
    if (ITL_HIST_INDEX > 0) {
        int cmp_result = itl_string_cmp(ITL_HISTORY[ITL_HIST_INDEX - 1], str);

        if (cmp_result == 0)
            return 0;
    }

    itl_string_t *new_str = itl_string_alloc();

    itl_string_copy(new_str, str);

    ITL_HISTORY[ITL_HIST_INDEX++] = new_str;

    return 1;
}

// Copies string from history to line editor
// Does not free anything
static void itl_history_get(itl_le_t *le)
{
    if (le->h_item_sel >= ITL_HIST_INDEX)
        return;

    itl_string_t *h_entry = ITL_HISTORY[le->h_item_sel];

    itl_string_copy(ITL_LINE_BUF, h_entry);

    le->cur_pos = ITL_LINE_BUF->length;
}

// allocates memory for global history and line buffer
// adds signal handle for c^c
int tl_init(void)
{
    itl_history_alloc();

    ITL_LINE_BUF = itl_string_alloc();

    signal(SIGINT, itl_handle_interrupt);

    return itl_enter_raw_mode();
}

// Frees memory for global history and line buffer
// Removes signal handle for c^c
int tl_exit(void)
{
    itl_history_free();
    itl_string_free(ITL_LINE_BUF);

    signal(SIGINT, SIG_DFL);

    ITL_DBG("exit, alloc count", ITL_GLOBAL_ALLOC_COUNT);

    int code = itl_exit_raw_mode();

    TL_ASSERT(ITL_GLOBAL_ALLOC_COUNT == 0);

    return code;
}

// Returns the number of utf8 characters in a char array
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

typedef enum
{
    ITL_KEY_CHAR = 0,
    ITL_KEY_UNKN,
    ITL_KEY_UP,
    ITL_KEY_DOWN,
    ITL_KEY_RIGHT,
    ITL_KEY_LEFT,
    ITL_KEY_END,
    ITL_KEY_HOME,
    ITL_KEY_ENTER,
    ITL_KEY_BACKSPACE,
    ITL_KEY_DELETE,
    ITL_KEY_TAB,
    ITL_KEY_INTERRUPT,
} ITL_KEY_KIND;

#define ITL_CTRL_BIT  32
#define ITL_SHIFT_BIT 64
#define ITL_ALT_BIT   128

#define ITL_KEY_MASK  15
#define ITL_MOD_MASK  224

static int itl_parse_esc(int byte)
{
    ITL_KEY_KIND event = 0;

    switch (byte) { // plain bytes
        case 3:
            return ITL_KEY_INTERRUPT;
        case 9:
            return ITL_KEY_TAB;
        case 10: // newline
        case 13: // cr
            return ITL_KEY_ENTER;
        case 23:
            return ITL_KEY_BACKSPACE | ITL_CTRL_BIT;
        case 8:
        case 127:
            return ITL_KEY_BACKSPACE;
    }

#ifdef _WIN32
    if (byte == 224) { // escape
        switch (ITL_READ_BYTE()) {
            case 72: {
                event = ITL_KEY_UP;
            } break;

            case 75: {
                event = ITL_KEY_LEFT;
            } break;

            case 77: {
                event = ITL_KEY_RIGHT;
            } break;

            case 71: {
                event = ITL_KEY_HOME;
            } break;

            case 79: {
                event = ITL_KEY_END;
            } break;

            case 80: {
                event = ITL_KEY_DOWN;
            } break;

            case 83: {
                event = ITL_KEY_DELETE;
            } break;

            case 115: {
                event = ITL_KEY_LEFT | ITL_CTRL_BIT;
            } break;

            case 116: {
                event = ITL_KEY_RIGHT | ITL_CTRL_BIT;
            } break;

            case 147: { // ctrl del
                event = ITL_KEY_DELETE | ITL_CTRL_BIT;
            } break;

            default:
                event = ITL_KEY_UNKN;
        }
    } else {
        return ITL_KEY_CHAR;
    }
#else // Linux
    int read_mod = 0;

    if (byte == 27) { // \x1b
        byte = ITL_READ_BYTE();

        if (byte != 91 && byte != 79) { // [
            return ITL_KEY_CHAR | ITL_ALT_BIT;
        }

        byte = ITL_READ_BYTE();

        if (byte == 49) {
            if (ITL_READ_BYTE() != 59) // ;
                return ITL_KEY_UNKN;

            switch (ITL_READ_BYTE()) {
                case 50: {
                    event |= ITL_SHIFT_BIT;
                } break;

                case 53: {
                    event |= ITL_CTRL_BIT;
                } break;
            }

            read_mod = 1;

            byte = ITL_READ_BYTE();
        }

        switch (byte) { // escape codes based on xterm
            case 51: {
                event |= ITL_KEY_DELETE;
            } break;

            case 65: {
                event |= ITL_KEY_UP;
                return event;
            } break;

            case 66: {
                event |= ITL_KEY_DOWN;
                return event;
            } break;

            case 67: {
                event |= ITL_KEY_RIGHT;
                return event;
            } break;

            case 68: {
                event |= ITL_KEY_LEFT;
                return event;
            } break;

            case 70: {
                event |= ITL_KEY_END;
                return event;
            } break;

            case 72: {
                event |= ITL_KEY_HOME;
                return event;
            } break;

            default:
                event |= ITL_KEY_UNKN;
        }
    } else {
        if (!iscntrl(byte))
            return ITL_KEY_CHAR;
        else
            return ITL_KEY_UNKN;
    }

    if (!read_mod) {
        byte = ITL_READ_BYTE();

        if (byte == 59) { // ;
            switch (ITL_READ_BYTE()) {
                case 53: {
                    event |= ITL_CTRL_BIT;
                } break;

                case 51: {
                    event |= ITL_SHIFT_BIT;
                } break;
            }

            byte = ITL_READ_BYTE();
        }

        if (byte != 126) // ~
            event = ITL_KEY_UNKN;
    }
#endif // Linux
    return event;
}

// TODO: Codepoints U+D800 to U+DFFF (known as UTF-16 surrogates) are invalid
static itl_utf8_t itl_parse_utf8(int byte)
{
    uint8_t bytes[4] = {byte, 0, 0, 0};
    uint8_t len = 0;

    if ((byte & 0x80) == 0) // 1 byte
        len = 1;
    else if ((byte & 0xE0) == 0xC0) // 2 byte
        len = 2;
    else if ((byte & 0xF8) == 0xF0) // 3 byte
        len = 3;
    else if ((byte & 0xF8) == 0xF0) // 4 byte
        len = 4;

    for (uint8_t i = 1; i < len; ++i) // consequent bytes
        bytes[i] = ITL_READ_BYTE();

#ifdef TL_DEBUG
    printf("\nutf8 char bytes: '");

    for (uint8_t i = 0; i < len - 1; ++i)
        printf("%02X ", bytes[i]);

    printf("%02X'\n", bytes[len - 1]);
#endif

    return itl_utf8_new(bytes, len);
}

//  0 on enter
//  1 on interrupt
// -1 if should continue
static int itl_handle_esc(itl_le_t *le, int esc)
{
    switch (esc & ITL_KEY_MASK) {
        case ITL_KEY_UP: {
            if (le->h_item_sel == -1) {
                le->h_item_sel = ITL_HIST_INDEX;
                if (le->line->length > 0 && ITL_HIST_INDEX > 0) {
                    itl_history_append(le->line);
                }
            }

           if (le->h_item_sel > 0) {
                le->h_item_sel -= 1;

                itl_le_clear(le);

                itl_history_get(le);
            }
        } break;

        case ITL_KEY_DOWN: {
            if (le->h_item_sel < ITL_HIST_INDEX - 1 && le->h_item_sel >= 0) {
                le->h_item_sel += 1;

                itl_le_clear(le);
                itl_history_get(le);
            }
            else if (ITL_HIST_INDEX > 0) {
                itl_le_clear(le);
                le->h_item_sel = -1;
            }
        } break;

        case ITL_KEY_RIGHT: {
            if (le->cur_pos < le->line->length) {
                size_t count = 1;

                if (esc & ITL_CTRL_BIT) {
                    size_t next_ws = itl_le_next_whitespace(le, 0);

                    if (next_ws <= 1) {
                        count = itl_le_next_word(le, 0);
                    }
                    else
                        count = next_ws;
                }

                itl_le_move_right(le, count);
            }
        } break;

        case ITL_KEY_LEFT: {
            if (le->cur_pos > 0 && le->cur_pos <= le->line->length) {
                size_t count = 1;

                if (esc & ITL_CTRL_BIT) {
                    size_t next_ws = itl_le_next_whitespace(le, 1);

                    if (next_ws <= 1) {
                        count = itl_le_next_word(le, 1);

                        itl_le_move_left(le, count);

                        count = itl_le_next_whitespace(le, 1) - 1;
                    }
                    else
                        count = next_ws - 1;
                }

                itl_le_move_left(le, count);
            }
        } break;

        case ITL_KEY_END: {
            itl_le_move_right(le, le->line->length - le->cur_pos);
        } break;

        case ITL_KEY_HOME: {
            itl_le_move_left(le, le->cur_pos);
        } break;

        case ITL_KEY_ENTER: {
            int code = itl_string_to_cstr(le->line, le->out_buf, le->out_size);

            itl_history_append(le->line);

            itl_le_clear(le);

            if (code == 0)
                fputc('\n', stdout);

            fflush(stdout);

            return code;
        } break;

        case ITL_KEY_BACKSPACE: {
            size_t count = 1;

            if (esc & ITL_CTRL_BIT) {
                size_t next_ws = itl_le_next_whitespace(le, 1);

                if (next_ws <= 1) {
                    count = itl_le_next_word(le, 1);

                    itl_le_erase(le, count, 1);

                    count = itl_le_next_whitespace(le, 1) - 1;
                }
                else
                    count = next_ws - 1;
            }

            itl_le_erase(le, count, 1);
        } break;

        case ITL_KEY_DELETE: {
            size_t count = 1;

            if (esc & ITL_CTRL_BIT) {
                size_t next_ws = itl_le_next_whitespace(le, 0);

                if (next_ws <= 1) {
                    count = itl_le_next_word(le, 0);

                    itl_le_erase(le, count, 0);

                    count = itl_le_next_whitespace(le, 0);
                }
                else
                    count = next_ws;
            }

            itl_le_erase(le, count, 0);
        } break;

        case ITL_KEY_INTERRUPT: {
            itl_string_to_cstr(le->line, le->out_buf, le->out_size);

            return 1;
        } break;

        default: {
            ITL_DBG("key wasn't handled", esc & ITL_KEY_MASK);
            ITL_DBG("modifier", esc & ITL_MOD_MASK);
        }
    }

    return -1;
}

// Gets one character, does not wait for Enter
int tl_getc(char *char_buffer, size_t size, const char *prompt)
{
    TL_ASSERT(size > 1);
    TL_ASSERT(char_buffer != NULL);

    itl_le_t le = itl_le_new(ITL_LINE_BUF, char_buffer, size, prompt);

    itl_le_update_tty(&le);

    int esc;
    int in = ITL_READ_BYTE();

    if ((esc = itl_parse_esc(in)) != ITL_KEY_CHAR) {
            if ((esc = itl_handle_esc(&le, esc)) != -1) {
                return esc;
            }
        }
    else {
        itl_utf8_t ch = itl_parse_utf8(in);
        itl_le_putc(&le, ch);

        itl_string_to_cstr(le.line, char_buffer, size);
        itl_le_update_tty(&le);

        itl_le_clear(&le);

        fputc('\n', stdout);

        fflush(stdout);
    }

    return 0;
}

//  0 on success
//  1 on interrupt
// -1 internal error (reserved)
int tl_readline(char *line_buffer, size_t size, const char *prompt)
{
    // Size required for at least one 1-byte character and a null-terminator
    TL_ASSERT(size > 1);
    TL_ASSERT(line_buffer != NULL);

    itl_le_t le = itl_le_new(ITL_LINE_BUF, line_buffer, size, prompt);

    itl_le_update_tty(&le);

    int esc;
    int in;

    while (1) {
        in = ITL_READ_BYTE();

#ifdef TL_SEE_BYTES
        printf("%d\n", in);
        continue;
#endif

        if ((esc = itl_parse_esc(in)) != ITL_KEY_CHAR) {
            if ((esc = itl_handle_esc(&le, esc)) != -1) {
                return esc;
            }
        }
        else {
            itl_utf8_t ch = itl_parse_utf8(in);
            itl_le_putc(&le, ch);
        }

        itl_le_update_tty(&le);
    }

    return -1;
}

#endif // TOILETLINE_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

/*
 * TODO:
 *  - Refactoring to allow better handling of terminal properties.
 *  - itl_string_to_tty_cstr().
 *  - Replace history on limit.
 *  - Autocompletion.
 *  - Variadic utf8_t struct size.
 *  - Test this on old Windows.
 */
