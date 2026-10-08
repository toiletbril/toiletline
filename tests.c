#define ITL_TTY_IS_TTY() (1)
#define TL_CTRL_Z_UNDO
#define TOILETLINE_IMPLEMENTATION
#include "toiletline.h"

#include <stdio.h>

#if defined ITL_POSIX
#include <sys/wait.h>
#endif

#define BUFFER_SIZE 128

#define TEST_PRINTF(...)                                                       \
  do {                                                                         \
    fputs(__func__, stdout);                                                   \
    printf(": "__VA_ARGS__);                                                   \
  } while (0)

#define countof(a) (sizeof(a) / sizeof((a)[0]))

static bool test_bytes_have(const char *data, size_t size, const char *needle);

typedef struct string_test_case string_test_case_t;

struct string_test_case
{
  const char *original;
  const char *should_be;
};

typedef struct from_cstr_test_case from_cstr_test_case_t;

struct from_cstr_test_case
{
  const char *original;
  size_t      length;
  size_t      size;
};

static bool
test_string_from_cstr(void)
{
  size_t                i;
  int                   result;
  from_cstr_test_case_t test;
  char                  out_buffer[BUFFER_SIZE];

  itl_string_t *str = itl_string_alloc();

  /* clang-format off */
  const from_cstr_test_case_t tests[] = {
  /* original, length, size */
      {"hello, world", 12, 12},
      {"привет, мир",  11, 20},
      {"你好世界",     4,  12}
  };
  /* clang-format on */

  for (i = 0; i < countof(tests); ++i) {
    test = tests[i];

    ITL_STRING_FROM_CSTR(str, test.original);
    itl_string_to_cstr(str, out_buffer, BUFFER_SIZE);

    result = strcmp(out_buffer, test.original);

    if (result != 0 || str->length != test.length || str->size != test.size) {
      TEST_PRINTF("Result %zu: '%s', should be: '%s'. Length: %zu/%zu, "
                  "size: %zu/%zu\n",
                  i, out_buffer, test.original, str->length, test.length,
                  str->size, test.size);
      ITL_STRING_FREE(str);
      return false;
    }
  }

  ITL_STRING_FREE(str);

  return true;
}

typedef struct shift_test_case shift_test_case_t;

struct shift_test_case
{
  size_t pos;
  size_t count;
  bool   backwards;
};

static bool
test_string_shift(void)
{
  size_t             i;
  int                result;
  string_test_case_t test;
  shift_test_case_t  shift;
  char               out_buffer[BUFFER_SIZE];

  itl_string_t *str = itl_string_alloc();

  /* clang-format off */
  const string_test_case_t tests[] = {
      /* original, should_be */
      {"hello world sailor", "hello sailor"},
      {"это строка",         "то строка"},
  };
  /* clang-format on */

  const shift_test_case_t settings[] = {
      /*  pos, count, backwards */
      {12, 6, true},
      {1,  1, true}
  };

  for (i = 0; i < countof(tests); ++i) {
    test = tests[i];
    shift = settings[i];

    ITL_STRING_FROM_CSTR(str, test.original);
    itl_string_shift(str, shift.pos, shift.count, shift.backwards);
    itl_string_to_cstr(str, out_buffer, BUFFER_SIZE);

    result = strcmp(out_buffer, test.should_be);
    if (result != 0) {
      TEST_PRINTF("Result %zu: '%s', should be: '%s'\n", i, out_buffer,
                  test.should_be);
      ITL_STRING_FREE(str);
      return false;
    }
  }

  ITL_STRING_FREE(str);

  return true;
}

static bool
test_string_erase(void)
{
  size_t             i;
  int                result;
  string_test_case_t test;
  shift_test_case_t  erase;
  char               out_buffer[BUFFER_SIZE];

  itl_string_t *str = itl_string_alloc();

  /* clang-format off */ 
  const string_test_case_t tests[] = {
  /*  original               should_be */
      {"hello world sailor", "hello sailor"},
      {"это строка",         "то строка"},
      {"это строка",         "это стр"},
      {"это строка",         "это строка"},
      {"это строка",         "это строка"},
      {"hello",              "ello"}
  };
  /* clang-format on */

  const shift_test_case_t settings[] = {
      /*  pos, count, backwards */
      {12, 6, true },
      {0,  1, false},
      {10, 3, true },
      {10, 3, false},
      {0,  0, true },
      {1,  3, true }
  };

  for (i = 0; i < countof(tests); ++i) {
    test = tests[i];
    erase = settings[i];

    ITL_STRING_FROM_CSTR(str, test.original);
    itl_string_erase(str, erase.pos, erase.count, erase.backwards);
    itl_string_to_cstr(str, out_buffer, BUFFER_SIZE);

    result = strcmp(out_buffer, test.should_be);
    if (result != 0 || str->size != strlen(test.should_be)) {
      TEST_PRINTF("Result %zu: '%s', should be: '%s'\n", i, out_buffer,
                  test.should_be);
      ITL_STRING_FREE(str);
      return false;
    }
  }

  ITL_STRING_FREE(str);

  return true;
}

static bool
test_string_insert(void)
{
  int                result;
  size_t             i, pos;
  string_test_case_t test;
  char               out_buffer[BUFFER_SIZE];

  itl_string_t *str = itl_string_alloc();
  itl_utf8_t    A = itl_utf8_new((uint8_t[4]){0x41}, 1);
  itl_utf8_t    two_byte = itl_utf8_new((uint8_t[4]){0xC3, 0xA9}, 2);

  /* clang-format off */
  const string_test_case_t tests[] = {
  /* original, should_be */
      {"hello, wrld",  "hello, wArld" },
      {"hello, wrld",  "hello, wrldA" },
      {"hello, world", "Ahello, world"}
  };
  /* clang-format on*/

  const size_t positions[] = {8, 11, 0};

  for (i = 0; i < countof(tests); ++i) {
    test = tests[i];
    pos = positions[i];

    ITL_STRING_FROM_CSTR(str, test.original);
    itl_string_insert(str, pos, A);
    itl_string_to_cstr(str, out_buffer, BUFFER_SIZE);

    result = strcmp(out_buffer, test.should_be);
    if (result != 0 || str->size != strlen(test.should_be)) {
      TEST_PRINTF("Result %zu: '%s', should be: '%s'\n", i, out_buffer,
                  test.should_be);
      ITL_STRING_FREE(str);
      return false;
    }
  }

  ITL_STRING_FROM_CSTR(str, "ab");
  itl_string_insert(str, 1, two_byte);
  if (str->size != 4) {
    ITL_STRING_FREE(str);
    return false;
  }

  ITL_STRING_FREE(str);

  return true;
}

typedef struct split_test_case split_test_case_t;

struct split_test_case
{
  size_t start;
  size_t end;
};

static bool
test_char_buf(void)
{
  itl_string_t   *str = itl_string_alloc();
  itl_char_buf_t *cb = itl_char_buf_alloc();

  const char *should_be = "привет, мир help me3912033312 ЛОЛ";

  ITL_STRING_FROM_CSTR(str, "привет, ");
  itl_char_buf_append_string(cb, str);
  itl_char_buf_append_cstr(cb, "мир ");
  ITL_STRING_FROM_CSTR(str, "help");
  itl_char_buf_append_string(cb, str);
  itl_char_buf_append_byte(cb, ' ');
  itl_char_buf_append_byte(cb, 'm');
  itl_char_buf_append_byte(cb, 'e');
  itl_char_buf_append_size_t(cb, 3912033312);
  itl_char_buf_append_cstr(cb, " ЛОЛ");

  /* null-terminate cb->data */
  while (cb->capacity < cb->size + 1) {
    itl_char_buf_extend(cb);
  }
  cb->data[cb->size] = '\0';

  if (cb->size != strlen(should_be) || strcmp(should_be, cb->data) != 0) {
    TEST_PRINTF("Result: '%s', should be: '%s', len: %zu/%zu\n", cb->data,
                should_be, cb->size, strlen(should_be));
    ITL_STRING_FREE(str);
    ITL_CHAR_BUF_FREE(cb);
    return false;
  }

  ITL_STRING_FREE(str);
  ITL_CHAR_BUF_FREE(cb);

  return true;
}

static bool
test_string_shift_directions(void)
{
  size_t        i;
  char          filler[65];
  char          out_buffer[BUFFER_SIZE];
  itl_string_t *str = itl_string_alloc();

  ITL_STRING_FROM_CSTR(str, "abcdefgh");
  itl_string_shift(str, 3, 2, true);
  itl_string_recalc_size(str);

  if (itl_string_to_cstr(str, out_buffer, BUFFER_SIZE) != TL_SUCCESS ||
      strcmp(out_buffer, "adefgh") != 0 || str->length != 6)
  {
    TEST_PRINTF("overlapping backward shift gave '%s', length %zu\n",
                out_buffer, str->length);
    ITL_STRING_FREE(str);
    return false;
  }

  ITL_STRING_FROM_CSTR(str, "abcdef");
  itl_string_shift(str, 2, 3, false);
  itl_string_recalc_size(str);

  if (itl_string_to_cstr(str, out_buffer, BUFFER_SIZE) != TL_SUCCESS ||
      strcmp(out_buffer, "abcdecdef") != 0 || str->length != 9)
  {
    TEST_PRINTF("overlapping forward shift gave '%s', length %zu\n",
                out_buffer, str->length);
    ITL_STRING_FREE(str);
    return false;
  }

  ITL_STRING_FROM_CSTR(str, "x\xC3\xA9y");
  itl_string_shift(str, 1, 1, false);
  itl_string_recalc_size(str);

  if (itl_string_to_cstr(str, out_buffer, BUFFER_SIZE) != TL_SUCCESS ||
      strcmp(out_buffer, "x\xC3\xA9\xC3\xA9y") != 0 || str->size != 6)
  {
    TEST_PRINTF("multibyte forward shift gave '%s', size %zu\n", out_buffer,
                str->size);
    ITL_STRING_FREE(str);
    return false;
  }

  for (i = 0; i < ITL_STRING_INIT_SIZE; ++i) {
    filler[i] = 'a';
  }
  filler[ITL_STRING_INIT_SIZE] = '\0';

  ITL_STRING_FROM_CSTR(str, filler);
  itl_string_shift(str, 0, 1, false);
  itl_string_recalc_size(str);

  if (str->length != ITL_STRING_INIT_SIZE + 1 ||
      str->capacity < ITL_STRING_INIT_SIZE + 1 ||
      str->size != ITL_STRING_INIT_SIZE + 1)
  {
    TEST_PRINTF("boundary forward shift gave length %zu, capacity %zu\n",
                str->length, str->capacity);
    ITL_STRING_FREE(str);
    return false;
  }

  for (i = 0; i < str->length; ++i) {
    if (str->chars[i].size != 1 || str->chars[i].bytes[0] != 'a') {
      TEST_PRINTF("boundary forward shift corrupted rune %zu\n", i);
      ITL_STRING_FREE(str);
      return false;
    }
  }

  ITL_STRING_FREE(str);

  return true;
}

static bool
test_string_copy_uses_live_range(void)
{
  size_t        i;
  size_t        source_capacity;
  char          filler[201];
  char          out_buffer[BUFFER_SIZE];
  itl_string_t *src = itl_string_alloc();
  itl_string_t *dst = itl_string_alloc();

  for (i = 0; i < 200; ++i) {
    filler[i] = 'a';
  }
  filler[200] = '\0';

  ITL_STRING_FROM_CSTR(src, filler);
  itl_string_erase(src, src->length, 195, true);
  source_capacity = src->capacity;

  itl_string_copy(dst, src);

  if (dst->length != 5 || dst->size != 5 ||
      itl_string_to_cstr(dst, out_buffer, BUFFER_SIZE) != TL_SUCCESS ||
      strcmp(out_buffer, "aaaaa") != 0 || dst->capacity >= source_capacity)
  {
    TEST_PRINTF("copy from a sparse source gave '%s', capacity %zu/%zu\n",
                out_buffer, dst->capacity, source_capacity);
    ITL_STRING_FREE(src);
    ITL_STRING_FREE(dst);
    return false;
  }

  ITL_STRING_FROM_CSTR(src, "\xD0\xBF\xD1\x80\xD0\xB8");
  itl_string_copy(dst, src);

  if (dst->length != 3 || dst->size != 6 ||
      itl_string_to_cstr(dst, out_buffer, BUFFER_SIZE) != TL_SUCCESS ||
      strcmp(out_buffer, "\xD0\xBF\xD1\x80\xD0\xB8") != 0)
  {
    TEST_PRINTF("multibyte copy gave '%s', length %zu, size %zu\n", out_buffer,
                dst->length, dst->size);
    ITL_STRING_FREE(src);
    ITL_STRING_FREE(dst);
    return false;
  }

  ITL_STRING_FREE(src);
  ITL_STRING_FREE(dst);

  return true;
}

static bool
test_string_to_cstr_limits(void)
{
  char          exact[7];
  char          split[3];
  char          empty[1];
  itl_string_t *str = itl_string_alloc();

  ITL_STRING_FROM_CSTR(str, "h\xC3\xA9llo");

  if (itl_string_to_cstr(str, exact, sizeof(exact)) != TL_SUCCESS ||
      strcmp(exact, "h\xC3\xA9llo") != 0)
  {
    TEST_PRINTF("exact output buffer gave '%s'\n", exact);
    ITL_STRING_FREE(str);
    return false;
  }

  ITL_STRING_FROM_CSTR(str, "h\xC3\xA9");

  if (itl_string_to_cstr(str, split, sizeof(split)) != TL_ERROR_SIZE ||
      strcmp(split, "h") != 0)
  {
    TEST_PRINTF("short output buffer gave '%s'\n", split);
    ITL_STRING_FREE(str);
    return false;
  }

  empty[0] = 'Z';

  if (itl_string_to_cstr(str, empty, 0) != TL_ERROR_SIZE || empty[0] != 'Z') {
    TEST_PRINTF("zero-size output buffer was written\n");
    ITL_STRING_FREE(str);
    return false;
  }

  ITL_STRING_FREE(str);

  return true;
}

static bool
test_char_buf_growth_boundary(void)
{
  size_t          i;
  size_t          escaped_position;
  size_t          spaces_position;
  char            run[250];
  itl_char_buf_t  zero_capacity = ITL_ZERO_INIT;
  itl_string_t   *str = itl_string_alloc();
  itl_char_buf_t *cb = itl_char_buf_alloc();

  for (i = 0; i < sizeof(run); ++i) {
    run[i] = 'a';
  }

  itl_char_buf_append_bytes(cb, run, sizeof(run));
  escaped_position = cb->size;

  ITL_STRING_FROM_CSTR(str, "a\nb\\c");
  itl_char_buf_append_string_escaped(cb, str);
  spaces_position = cb->size;

  itl_char_buf_append_spaces(cb, 300);
  itl_char_buf_append_cstr(cb, "end");

  if (cb->size != sizeof(run) + 7 + 300 + 3 || cb->capacity < cb->size ||
      memcmp(cb->data, run, sizeof(run)) != 0 ||
      memcmp(cb->data + escaped_position, "a\\nb\\\\c", 7) != 0 ||
      memcmp(cb->data + cb->size - 3, "end", 3) != 0)
  {
    TEST_PRINTF("character buffer boundary gave size %zu, capacity %zu\n",
                cb->size, cb->capacity);
    ITL_STRING_FREE(str);
    ITL_CHAR_BUF_FREE(cb);
    return false;
  }

  for (i = spaces_position; i < spaces_position + 300; ++i) {
    if (cb->data[i] != ' ') {
      TEST_PRINTF("padding byte %zu is not a space\n", i);
      ITL_STRING_FREE(str);
      ITL_CHAR_BUF_FREE(cb);
      return false;
    }
  }

  itl_char_buf_append_spaces(&zero_capacity, 3);

  if (zero_capacity.size != 3 || memcmp(zero_capacity.data, "   ", 3) != 0) {
    TEST_PRINTF("padding an empty buffer gave size %zu\n", zero_capacity.size);
    ITL_FREE(zero_capacity.data);
    ITL_STRING_FREE(str);
    ITL_CHAR_BUF_FREE(cb);
    return false;
  }

  ITL_FREE(zero_capacity.data);
  ITL_STRING_FREE(str);
  ITL_CHAR_BUF_FREE(cb);

  return true;
}

static bool
test_parse_size(void)
{
  size_t     i, diff, offset, result = 0;
  const char test_string[] = "123;7788a88891231231hello!";

  const size_t should_be[] = {123, 7788, 88891231231, 0};

  for (i = 0, offset = 0; i < countof(should_be); ++i) {
    diff = itl_parse_size(test_string + offset, &result);
    if (result != should_be[i]) {
      TEST_PRINTF("Result: '%zu', should be: '%zu', diff: %zu, "
                  "offset %zu\n",
                  result, should_be[i], diff, offset);
      return false;
    }
    offset += diff + 1;
  }

  return true;
}

static bool
test_utf8_strlen(void)
{
  size_t i;

  const char  *input[] = {"привет", "world", "你好世界", "hel№lo"};
  const size_t should_be[] = {6, 5, 4, 6};
  const size_t should_be_chopped[] = {2, 3, 1, 3};

  for (i = 0; i < countof(should_be); ++i) {
    size_t length = tl_utf8_strlen(input[i]);
    size_t length_chopped = tl_utf8_strnlen(input[i], 3);
    if (length != should_be[i]) {
      TEST_PRINTF("Length: '%zu', should be: '%zu', string: '%s'\n",
                  length, should_be[i], input[i]);
      return false;
    } else if (length_chopped != should_be_chopped[i]) {
      TEST_PRINTF("Chopped length: '%zu', should be: '%zu', string: '%s'\n",
                  length_chopped, should_be_chopped[i], input[i]);
      return false;
    }
  }

  return true;
}

static bool
test_string_from_bytes_truncates_at_rune_boundary(void)
{
  static char  bytes[ITL_STRING_MAX_LEN + 3];
  char         last_character[2];
  size_t       i;
  itl_string_t *string = itl_string_alloc();

  for (i = 0; i < ITL_STRING_MAX_LEN - 1; ++i) {
    bytes[i] = 'x';
  }
  bytes[i++] = (char) 0xC3;
  bytes[i++] = (char) 0xA9;
  bytes[i] = 'y';

  if (!itl_string_from_bytes(string, bytes, sizeof(bytes)) ||
      string->size != ITL_STRING_MAX_LEN - 1)
  {
    TEST_PRINTF("multibyte truncation failed, size %zu\n", string->size);
    ITL_STRING_FREE(string);
    return false;
  }
  last_character[0] = (char) string->chars[string->length - 1].bytes[0];
  last_character[1] = '\0';
  if (strcmp(last_character, "x") != 0) {
    TEST_PRINTF("multibyte truncation kept a partial rune\n");
    ITL_STRING_FREE(string);
    return false;
  }

  ITL_STRING_FREE(string);
  return true;
}

static bool
test_string_from_bytes_rejects_malformed_utf8(void)
{
  static const unsigned char lone_continuation[] = {0x80};
  static const unsigned char invalid_lead[] = {0xF8};
  static const unsigned char truncated[] = {0xE2, 0x82};
  static const unsigned char invalid_continuation[] = {0xE2, 0x41, 0xAC};
  static const unsigned char valid_prefix_then_invalid[] = {'x', 0x80};
  static const unsigned char overlong_two[] = {0xC0, 0x80};
  static const unsigned char overlong_three[] = {0xE0, 0x80, 0xAF};
  static const unsigned char overlong_four[] = {0xF0, 0x80, 0x80, 0xAF};
  static const unsigned char surrogate[] = {0xED, 0xA0, 0x80};
  static const unsigned char out_of_range[] = {0xF4, 0x90, 0x80, 0x80};
  static const struct {
    const unsigned char *bytes;
    size_t               size;
  } cases[] = {
      {lone_continuation, sizeof(lone_continuation)},
      {invalid_lead, sizeof(invalid_lead)},
      {truncated, sizeof(truncated)},
      {invalid_continuation, sizeof(invalid_continuation)},
      {valid_prefix_then_invalid, sizeof(valid_prefix_then_invalid)},
      {overlong_two, sizeof(overlong_two)},
      {overlong_three, sizeof(overlong_three)},
      {overlong_four, sizeof(overlong_four)},
      {surrogate, sizeof(surrogate)},
      {out_of_range, sizeof(out_of_range)},
  };
  itl_string_t *string = itl_string_alloc();
  size_t        i;

  if (!itl_string_from_bytes(string, "kept", 4)) {
    ITL_STRING_FREE(string);
    return false;
  }

  for (i = 0; i < countof(cases); ++i) {
    char unchanged[5];

    if (itl_string_from_bytes(string, (const char *) cases[i].bytes,
                              cases[i].size))
    {
      TEST_PRINTF("malformed UTF-8 case %zu was accepted\n", i);
      ITL_STRING_FREE(string);
      return false;
    }
    if (itl_string_to_cstr(string, unchanged, sizeof(unchanged)) !=
            TL_SUCCESS ||
        strcmp(unchanged, "kept") != 0)
    {
      TEST_PRINTF("malformed UTF-8 case %zu changed the destination\n", i);
      ITL_STRING_FREE(string);
      return false;
    }
  }

  ITL_STRING_FREE(string);
  return true;
}

static bool
test_char_width(void)
{
  itl_utf8_t ascii = itl_utf8_new((uint8_t[4]){0x41}, 1);
  itl_utf8_t cjk = itl_utf8_new((uint8_t[4]){0xE4, 0xBD, 0xA0}, 3);
  itl_utf8_t combining = itl_utf8_new((uint8_t[4]){0xCC, 0x81}, 2);
  itl_utf8_t emoji = itl_utf8_new((uint8_t[4]){0xF0, 0x9F, 0x98, 0x80}, 4);
  const char invalid[] = {(char) 0xE2, 'A', 'B'};
  const char truncated[] = {(char) 0xE2, (char) 0x80};
  const char combined[] = {'A', (char) 0xCC, (char) 0x81, 'B'};
  const char escaped[] = {'A', 0x1B, '[', '0', 'm', 'B'};
  size_t offset = 0;

  if (itl_char_width(ascii) != 1 || itl_char_width(cjk) != 2 ||
      itl_char_width(combining) != 0 || itl_char_width(emoji) != 2)
  {
    TEST_PRINTF("widths: ascii %zu cjk %zu comb %zu emoji %zu\n",
                itl_char_width(ascii), itl_char_width(cjk),
                itl_char_width(combining), itl_char_width(emoji));
    return false;
  }

  if (itl_strn_width_walk(invalid, sizeof(invalid), (size_t) -1, NULL) != 3 ||
      itl_strn_width_walk(truncated, sizeof(truncated), (size_t) -1, NULL) != 2)
  {
    return false;
  }

  if (itl_strn_width_walk(combined, sizeof(combined), 1, &offset) != 1 ||
      offset != 3)
  {
    return false;
  }
  if (itl_strn_width_walk(escaped, sizeof(escaped), 1, &offset) != 1 ||
      offset != 5)
  {
    return false;
  }

  memcpy(itl_g_ghost, "\xC3\xA9", 3);
  itl_g_ghost_len = 2;
  itl_g_ghost_width = itl_cstr_display_width(itl_g_ghost);
  if (itl_g_ghost_len != 2 || itl_g_ghost_width != 1) return false;
  itl_ghost_clear();
  if (itl_g_ghost_len != 0 || itl_g_ghost_width != 0) return false;

  return true;
}

static bool
test_metrics(void)
{
  itl_le_t le = ITL_ZERO_INIT;
  itl_le_metrics_t m;

  itl_string_t *line = itl_string_alloc();

  le.line = line;

  /* An empty buffer is a single row. */
  le.prompt_width = 0;
  le.cursor_position = 0;
  m = itl_le_compute_metrics(&le, 80);
  if (m.total_rows != 1 || m.cursor_row != 0 || m.cursor_col != 0) {
    TEST_PRINTF("empty: rows %zu crow %zu ccol %zu\n", m.total_rows,
                m.cursor_row, m.cursor_col);
    ITL_STRING_FREE(line);
    return false;
  }

  /* An embedded newline makes a second row. */
  ITL_STRING_FROM_CSTR(line, "ab\ncd");
  le.cursor_position = line->length;
  m = itl_le_compute_metrics(&le, 80);
  if (m.total_rows != 2 || m.cursor_row != 1 || m.cursor_col != 2) {
    TEST_PRINTF("newline: rows %zu crow %zu ccol %zu\n", m.total_rows,
                m.cursor_row, m.cursor_col);
    ITL_STRING_FREE(line);
    return false;
  }

  /* Continuation rows are padded by the prompt width. */
  le.prompt = ">> ";
  le.prompt_width = 3;
  m = itl_le_compute_metrics(&le, 80);
  if (m.total_rows != 2 || m.cursor_row != 1 || m.cursor_col != 5) {
    TEST_PRINTF("padded: rows %zu crow %zu ccol %zu\n", m.total_rows,
                m.cursor_row, m.cursor_col);
    ITL_STRING_FREE(line);
    return false;
  }

  /* A soft wrap with padding wraps to the indent column. */
  ITL_STRING_FROM_CSTR(line, "abcde");
  le.prompt = "> ";
  le.prompt_width = 2;
  le.cursor_position = line->length;
  m = itl_le_compute_metrics(&le, 5);
  if (m.total_rows != 2 || m.cursor_row != 1 || m.cursor_col != 4) {
    TEST_PRINTF("wrapped: rows %zu crow %zu ccol %zu\n", m.total_rows,
                m.cursor_row, m.cursor_col);
    ITL_STRING_FREE(line);
    return false;
  }

  ITL_STRING_FREE(line);
  return true;
}

/* Expands a string literal into the pointer and byte count the byte level
   search helpers take. The terminator is excluded. */
#define SEARCH_QUERY(literal) (literal), (sizeof(literal) - 1)

static bool
test_find_substring(void)
{
  bool ok = true;

  if (!itl_ascii_contains_casefold(SEARCH_QUERY("Hello WORLD"),
                                   SEARCH_QUERY("o wo"), 0))
  {
    ok = false;
  }
  if (!itl_ascii_contains_casefold(SEARCH_QUERY("Hello WORLD"),
                                   SEARCH_QUERY("world"), 0))
  {
    ok = false;
  }
  if (itl_ascii_contains_casefold(SEARCH_QUERY("Hello WORLD"),
                                  SEARCH_QUERY("xyz"), 0))
  {
    ok = false;
  }
  if (!itl_ascii_contains_casefold(SEARCH_QUERY("Hello WORLD"),
                                   SEARCH_QUERY(""), 0))
  {
    ok = false;
  }
  if (itl_ascii_contains_casefold(SEARCH_QUERY("Hello WORLD"),
                                  SEARCH_QUERY("hello"), 1))
  {
    ok = false;
  }

  /* Case folding covers ASCII letters alone, so a non ASCII rune only matches
     itself. */
  if (itl_ascii_contains_casefold(SEARCH_QUERY("echo \xC3\x84"),
                                  SEARCH_QUERY("\xC3\xA4"), 0))
  {
    ok = false;
  }

  if (!ok) {
    TEST_PRINTF("substring match mismatch\n");
  }

  return ok;
}

static bool
test_history_multiline_file(void)
{
  bool ok = true;

  const char *path = "tl_test_history.txt";

  itl_string_t *entry = itl_string_alloc();
  itl_string_t *got = itl_string_alloc();

  const char multiline[] = {'l', 's', 0x0A, 'p', 'w', 'd'};

  itl_g_is_active = true;

  /* Start from a clean file so the append count is deterministic. */
  remove(path);

  itl_string_from_bytes(entry, multiline, sizeof multiline);

  /* A missing file is fine here, the load still records the store path. */
  tl_history_load(path);

  if (!itl_history_append_to_file(entry, true, false)) {
    TEST_PRINTF("append failed\n");
    ok = false;
  }

  /* Reload from disk to prove the entry was persisted to the file. */
  if (tl_history_load(path) != TL_SUCCESS) {
    TEST_PRINTF("load failed\n");
    ok = false;
  }

  if (ok && itl_g_history_count != 1) {
    TEST_PRINTF("expected one entry, got %zu\n", itl_g_history_count);
    ok = false;
  }

  if (ok && (!itl_history_read_entry(itl_history_index_to_offset(0), got) ||
             !itl_string_equal(got, entry)))
  {
    TEST_PRINTF("multiline entry did not survive the roundtrip\n");
    ok = false;
  }

  itl_g_history_free();
  ITL_STRING_FREE(entry);
  ITL_STRING_FREE(got);
  itl_g_is_active = false;
  remove(path);
  return ok;
}

/* Appends one command to the active history file, returning whether it was
   actually written. */
static bool
hist_append_cstr(const char *command)
{
  itl_string_t *str = itl_string_alloc();
  bool was_written;

  ITL_STRING_FROM_CSTR(str, command);
  was_written = itl_history_append_to_file(str, true, false);
  ITL_STRING_FREE(str);

  return was_written;
}

/* Reads the navigable entry at index and compares it to expected. */
static bool
hist_entry_is(size_t index, const char *expected)
{
  itl_string_t *got = itl_string_alloc();
  itl_string_t *want = itl_string_alloc();
  bool is_equal;

  ITL_STRING_FROM_CSTR(want, expected);
  is_equal = itl_history_read_entry(itl_history_index_to_offset(index), got) &&
             itl_string_equal(got, want);

  ITL_STRING_FREE(got);
  ITL_STRING_FREE(want);

  return is_equal;
}

static int
reject_ghost_history_entry(const char *entry)
{
  (void) entry;
  return 0;
}

static bool
test_rejected_ghost_history_prefix_is_cached(void)
{
  const char *path = "tl_test_rejected_ghost.txt";
  bool ok = true;

  itl_g_is_active = true;
  remove(path);
  tl_history_load(path);
  if (!hist_append_cstr("zzzz-invalid-history-command")) {
    ok = false;
  }
  tl_set_ghost_validate_callback(reject_ghost_history_entry);
  itl_ghost_fill_from_history("z", 1);
  if (itl_g_ghost_history_miss_prefix_length != 1 ||
      itl_g_ghost_history_miss_prefix[0] != 'z')
  {
    TEST_PRINTF("rejected prefix was not cached\n");
    ok = false;
  }

  tl_set_ghost_validate_callback(NULL);
  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}

#if defined ITL_WIN32 && !defined ITL_NO_WIN_ESCAPES
static bool
test_windows_ghost_does_not_require_term(void)
{
  const char *term = getenv("TERM");
  char       *saved_term = term != NULL ? _strdup(term) : NULL;
  int         previous_ghost_enabled = itl_g_ghost_enabled;
  bool        enabled_without_term;
  bool        dumb_term_disables;
  bool        explicit_disable_works;

  if (term != NULL && saved_term == NULL) return false;

  _putenv_s("TERM", "");
  itl_g_supports_decorations = -1;
  tl_set_ghost_enabled(1);
  enabled_without_term = itl_g_ghost_enabled != 0;

  _putenv_s("TERM", "dumb");
  itl_g_supports_decorations = -1;
  tl_set_ghost_enabled(1);
  dumb_term_disables = itl_g_ghost_enabled == 0;

  tl_set_ghost_enabled(0);
  explicit_disable_works = itl_g_ghost_enabled == 0;

  _putenv_s("TERM", saved_term != NULL ? saved_term : "");
  free(saved_term);
  itl_g_supports_decorations = -1;
  itl_g_ghost_enabled = previous_ghost_enabled;

  return enabled_without_term && dumb_term_disables && explicit_disable_works;
}
#endif

static bool
test_history_ring_cap(void)
{
  const char *path = "tl_test_ring.txt";
  bool   ok = true;
  size_t total = (size_t) TL_HISTORY_MAX_SIZE + 44;
  size_t i;
  char   line[32];
  char   expected[32];

  itl_g_is_active = true;
  remove(path);
  tl_history_load(path);

  /* Append more than the ring holds so the oldest entries are evicted. */
  for (i = 0; i < total; ++i) {
    snprintf(line, sizeof(line), "cmd %zu", i);
    if (!hist_append_cstr(line)) {
      TEST_PRINTF("append %zu failed\n", i);
      ok = false;
      break;
    }
  }

  if (ok && (itl_g_history_total_count != total ||
             itl_g_last_history_event_number != total))
  {
    TEST_PRINTF("total %zu and last %zu, expected %zu\n",
                itl_g_history_total_count,
                itl_g_last_history_event_number, total);
    ok = false;
  }

  if (ok) {
    tl_history_load(path);
    if (itl_g_history_count != TL_HISTORY_MAX_SIZE) {
      TEST_PRINTF("count %zu, expected %d after eviction\n",
                  itl_g_history_count, TL_HISTORY_MAX_SIZE);
      ok = false;
    }
  }

  /* The oldest surviving entry is the one total - MAX commands in. */
  snprintf(expected, sizeof(expected), "cmd %zu",
           total - (size_t) TL_HISTORY_MAX_SIZE);
  if (ok && !hist_entry_is(0, expected)) {
    TEST_PRINTF("oldest surviving entry is wrong\n");
    ok = false;
  }
  snprintf(expected, sizeof(expected), "cmd %zu", total - 1);
  if (ok && !hist_entry_is(itl_g_history_count - 1, expected)) {
    TEST_PRINTF("newest entry is wrong\n");
    ok = false;
  }

  tl_set_history_limit(2);
  snprintf(expected, sizeof(expected), "cmd %zu", total - 2);
  if (ok && (itl_g_history_count != 2 || !hist_entry_is(0, expected))) {
    TEST_PRINTF("shrunk history limit retained the wrong entries\n");
    ok = false;
  }
  snprintf(expected, sizeof(expected), "cmd %zu", total - 1);
  if (ok && !hist_entry_is(1, expected)) {
    TEST_PRINTF("shrunk history limit retained the wrong entries\n");
    ok = false;
  }

  tl_set_history_limit(TL_HISTORY_MAX_SIZE);
  if (ok && itl_g_history_count != 2) {
    TEST_PRINTF("grown history limit resurrected %zu entries\n",
                itl_g_history_count);
    ok = false;
  }

  tl_set_history_limit(0);
  if (ok && itl_g_history_count != 0) {
    TEST_PRINTF("zero history limit retained %zu entries\n",
                itl_g_history_count);
    ok = false;
  }
  tl_set_history_limit(TL_HISTORY_MAX_SIZE);
  if (ok && !hist_append_cstr("after growth")) {
    TEST_PRINTF("append after growing the history limit failed\n");
    ok = false;
  }
  if (ok && (itl_g_history_count != 1 ||
             !hist_entry_is(0, "after growth")))
  {
    TEST_PRINTF("grown history limit retained the wrong new entry\n");
    ok = false;
  }

  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}

static bool
test_history_dedup(void)
{
  const char *path = "tl_test_dedup.txt";
  bool ok = true;

  itl_g_is_active = true;
  remove(path);
  tl_history_load(path);

  hist_append_cstr("make test");
  hist_append_cstr("make test"); /* Consecutive duplicate, must be skipped. */
  if (itl_g_history_count != 1) {
    TEST_PRINTF("duplicate not skipped, count %zu\n", itl_g_history_count);
    ok = false;
  }

  hist_append_cstr("make run");
  if (ok && itl_g_history_count != 2) {
    TEST_PRINTF("distinct entry not added, count %zu\n", itl_g_history_count);
    ok = false;
  }

  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}

static bool
test_history_unterminated_line(void)
{
  const char *path = "tl_test_unterminated.txt";
  bool  ok = true;
  FILE *f;

  itl_g_is_active = true;
  remove(path);

  /* A file whose last line lacks a terminating newline, as a crash or a hand
     edit could leave it. */
  f = fopen(path, "wb");
  if (f == NULL) {
    TEST_PRINTF("could not create file\n");
    itl_g_is_active = false;
    return false;
  }
  fputs("ls", f);
  fclose(f);

  tl_history_load(path);
  hist_append_cstr("pwd");
  if (itl_g_history_total_count != 2 ||
      itl_g_last_history_event_number != 2)
  {
    TEST_PRINTF("unterminated append total %zu and last %zu\n",
                itl_g_history_total_count,
                itl_g_last_history_event_number);
    ok = false;
  }
  tl_history_load(path);

  if (itl_g_history_count != 2) {
    TEST_PRINTF("expected 2 entries, got %zu\n", itl_g_history_count);
    ok = false;
  }
  if (ok && (!hist_entry_is(0, "ls") || !hist_entry_is(1, "pwd"))) {
    TEST_PRINTF("append glued onto the unterminated line\n");
    ok = false;
  }

  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}

/* Both readers decode a record the same way. A lone carriage return survives as
   data and a CRLF ending is dropped on either path. */
static bool
test_history_carriage_return_rule(void)
{
  const char *path = "tl_test_carriage_return.txt";
  bool  ok = true;
  FILE *f;

  itl_string_t *buffered = itl_string_alloc();
  itl_string_t *unbuffered = itl_string_alloc();

  itl_g_is_active = true;
  remove(path);

  f = fopen(path, "wb");
  if (f == NULL) {
    TEST_PRINTF("could not create file\n");
    ITL_STRING_FREE(buffered);
    ITL_STRING_FREE(unbuffered);
    itl_g_is_active = false;
    return false;
  }

  /* The first record carries a lone carriage return. The second record ends
     with a carriage return and a line feed. */
  fwrite("a\rb\nc\r\n", 1, 7, f);
  fclose(f);

  if (tl_history_load(path) != TL_SUCCESS) {
    TEST_PRINTF("load failed\n");
    ok = false;
  }

  if (ok && itl_g_history_count != 2) {
    TEST_PRINTF("expected 2 entries, got %zu\n", itl_g_history_count);
    ok = false;
  }

  if (ok && (!hist_entry_is(0, "a\rb") || !hist_entry_is(1, "c"))) {
    TEST_PRINTF("the unbuffered reader mishandled a carriage return\n");
    ok = false;
  }

  if (ok && !itl_history_ensure_read_buffer()) {
    TEST_PRINTF("could not fill the read buffer\n");
    ok = false;
  }

  if (ok) {
    size_t index;

    for (index = 0; index < itl_g_history_count; ++index) {
      size_t offset = itl_history_index_to_offset(index);

      if (!itl_history_read_entry_buffered(offset, buffered) ||
          !itl_history_read_entry(offset, unbuffered) ||
          !itl_string_equal(buffered, unbuffered))
      {
        TEST_PRINTF("entry %zu differs between the readers\n", index);
        ok = false;
      }
    }
  }

  ITL_STRING_FREE(buffered);
  ITL_STRING_FREE(unbuffered);
  itl_g_history_free();
  itl_g_is_active = false;
  remove(path);
  return ok;
}

/* A history write drops the entries the limit no longer reaches and shifts the
   offsets that remain. The resulting state equals a scan of the shortened
   file. */
static bool
test_history_offset_shift_matches_scan(void)
{
  const char *path = "tl_test_offset_shift.txt";
  bool   ok = true;
  size_t removed_byte_count = 0;
  size_t shifted_offsets[2] = {0, 0};
  size_t shifted_count = 0;
  size_t original_total_count = 0;
  size_t shifted_total_count = 0;
  size_t shifted_file_size = 0;
  size_t index;
  FILE  *f;

  itl_g_is_active = true;
  remove(path);

  f = fopen(path, "wb");
  if (f == NULL) {
    TEST_PRINTF("could not create file\n");
    itl_g_is_active = false;
    return false;
  }
  fwrite("one\ntwo\nthree\n", 1, 14, f);
  fclose(f);

  tl_set_history_limit(2);
  if (tl_history_load(path) != TL_SUCCESS || itl_g_history_count != 2) {
    TEST_PRINTF("the limited load did not retain two entries\n");
    ok = false;
  }

  if (ok) {
    original_total_count = itl_g_history_total_count;
    removed_byte_count = itl_history_index_to_offset(0);
    if (removed_byte_count != 4) {
      TEST_PRINTF("expected the oldest retained entry at offset 4, got %zu\n",
                  removed_byte_count);
      ok = false;
    }
  }

  if (ok) {
    /* Leave behind what the replacement writes, which is the file without the
       dropped span. */
    f = fopen(path, "wb");
    if (f == NULL) {
      TEST_PRINTF("could not rewrite file\n");
      ok = false;
    } else {
      fwrite("two\nthree\n", 1, 10, f);
      fclose(f);
    }
  }

  if (ok) {
    itl_history_offsets_shift(removed_byte_count);
    shifted_count = itl_g_history_count;
    shifted_total_count = itl_g_history_total_count;
    shifted_file_size = itl_g_history_file_size;

    for (index = 0; index < shifted_count && index < 2; ++index) {
      shifted_offsets[index] = itl_history_index_to_offset(index);
    }

    if (!hist_entry_is(0, "two") || !hist_entry_is(1, "three")) {
      TEST_PRINTF("the shifted offsets do not read the retained entries\n");
      ok = false;
    }
  }

  if (ok && tl_history_load(path) != TL_SUCCESS) {
    TEST_PRINTF("the reload of the shortened file failed\n");
    ok = false;
  }

  if (ok && shifted_total_count != original_total_count) {
    TEST_PRINTF("the shift changed the total from %zu to %zu\n",
                original_total_count, shifted_total_count);
    ok = false;
  }

  if (ok && (shifted_count != itl_g_history_count ||
             shifted_file_size != itl_g_history_file_size))
  {
    TEST_PRINTF("the shift left %zu entries and %zu bytes, the scan left "
                "%zu and %zu\n",
                shifted_count, shifted_file_size, itl_g_history_count,
                itl_g_history_file_size);
    ok = false;
  }

  if (ok) {
    for (index = 0; index < itl_g_history_count; ++index) {
      if (shifted_offsets[index] != itl_history_index_to_offset(index)) {
        TEST_PRINTF("offset %zu differs between the shift and the scan\n",
                    index);
        ok = false;
      }
    }
  }

  tl_set_history_limit(TL_HISTORY_MAX_SIZE);
  itl_g_history_free();
  itl_g_is_active = false;
  remove(path);
  return ok;
}

static bool
test_history_search(void)
{
  const char *path = "tl_test_search.txt";
  bool ok = true;

  itl_string_t *scratch = itl_string_alloc();
  size_t        match;

  itl_g_is_active = true;
  remove(path);
  tl_history_load(path);

  hist_append_cstr("Git Status");
  hist_append_cstr("GIT commit");
  hist_append_cstr("make test");

  /* Searching backward from the newest finds "git commit" at index 1. */
  match = itl_history_find_match(SEARCH_QUERY("gIt"), itl_g_history_count - 1,
                                 scratch);
  if (match != 1) {
    TEST_PRINTF("expected match at index 1, got %zu\n", match);
    ok = false;
  }

  /* The next older match is "git status" at index 0. */
  if (ok) {
    match = itl_history_find_match(SEARCH_QUERY("gIt"), match - 1, scratch);
    if (match != 0) {
      TEST_PRINTF("expected older match at index 0, got %zu\n", match);
      ok = false;
    }
  }

  /* Searching forward from the oldest finds "git status" at index 0, and the
     next forward step finds "git commit" at index 1, the mirror of the
     backward walk. */
  if (ok) {
    match = itl_history_find_match_forward(SEARCH_QUERY("GiT"), 0, scratch);
    if (match != 0) {
      TEST_PRINTF("expected forward match at index 0, got %zu\n", match);
      ok = false;
    }
  }
  if (ok) {
    match =
        itl_history_find_match_forward(SEARCH_QUERY("GiT"), match + 1, scratch);
    if (match != 1) {
      TEST_PRINTF("expected newer forward match at index 1, got %zu\n", match);
      ok = false;
    }
  }

  /* A query that matches nothing returns the sentinel either way. */
  if (ok) {
    match = itl_history_find_match(SEARCH_QUERY("zzz"), itl_g_history_count - 1,
                                   scratch);
    if (match != ITL_HISTORY_NONE) {
      TEST_PRINTF("no match should return the sentinel, got %zu\n", match);
      ok = false;
    }
  }
  if (ok) {
    match = itl_history_find_match_forward(SEARCH_QUERY("zzz"), 0, scratch);
    if (match != ITL_HISTORY_NONE) {
      TEST_PRINTF("no forward match should return the sentinel, got %zu\n",
                  match);
      ok = false;
    }
  }

  remove(path);
  ITL_STRING_FREE(scratch);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}

static bool
search_match_is(const itl_string_t *found, const char *expected)
{
  char buffer[BUFFER_SIZE];

  return itl_string_to_cstr(found, buffer, sizeof(buffer)) == TL_SUCCESS &&
         strcmp(buffer, expected) == 0;
}

static const char *history_search_snapshot_contents;
static size_t      history_search_snapshot_size;

static int
provide_history_search_snapshot(const char **out_contents, size_t *out_size)
{
  *out_contents = history_search_snapshot_contents;
  *out_size = history_search_snapshot_size;
  return 1;
}

static bool
test_history_search_snapshot(void)
{
  static const char shared[] = "peer old\npeer new\nunfinished";
  static const char invalid[] = "bad\001entry\n";
  const char       *path = "tl_test_search_snapshot.txt";
  itl_string_t     *found = itl_string_alloc();
  itl_string_t     *query = itl_string_alloc();
  itl_le_t          le;
  tl_completion     entries;
  size_t            private_count;
  size_t            private_offset;
  size_t            match;
  bool              ok = true;

  itl_g_is_active = true;
  remove(path);
  tl_history_load(path);
  hist_append_cstr("private only");
  private_count = itl_g_history_count;
  private_offset = itl_history_index_to_offset(0);

  history_search_snapshot_contents = shared;
  history_search_snapshot_size = sizeof(shared) - 1;
  tl_set_history_search_snapshot_callback(provide_history_search_snapshot);
  itl_history_search_snapshot_begin();

  memset(&le, 0, sizeof(le));
  memset(&entries, 0, sizeof(entries));
  ITL_STRING_FROM_CSTR(query, "peer");
  le.line = query;

  match = itl_history_find_match(SEARCH_QUERY("peer"), ITL_HISTORY_NEWEST(),
                                 found);
  if (!itl_g_history_search_snapshot.is_active ||
      itl_history_search_count() != 2 || match != 1 ||
      !search_match_is(found, "peer new") ||
      itl_g_history_count != private_count ||
      itl_history_index_to_offset(0) != private_offset ||
      !hist_entry_is(0, "private only"))
  {
    TEST_PRINTF("the shared snapshot changed private history\n");
    ok = false;
  }
  if (ok && (!itl_history_menu_gather(&le, &entries) || entries.count != 2 ||
             strcmp(entries.candidates[0], "peer new") != 0 ||
             strcmp(entries.candidates[1], "peer old") != 0))
  {
    TEST_PRINTF("the history menu did not use the shared snapshot\n");
    ok = false;
  }

  itl_history_search_snapshot_end();
  match = itl_history_find_match(SEARCH_QUERY("private"),
                                 ITL_HISTORY_NEWEST(), found);
  if (ok && (match != 0 || !search_match_is(found, "private only"))) {
    TEST_PRINTF("private search was not restored\n");
    ok = false;
  }

  history_search_snapshot_contents = invalid;
  history_search_snapshot_size = sizeof(invalid) - 1;
  itl_history_search_snapshot_begin();
  if (ok && itl_g_history_search_snapshot.is_active) {
    TEST_PRINTF("an invalid shared snapshot became active\n");
    ok = false;
  }

  itl_history_search_snapshot_end();
  tl_set_history_search_snapshot_callback(NULL);
  remove(path);
  ITL_STRING_FREE(found);
  ITL_STRING_FREE(query);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}

static bool
test_history_search_matching(void)
{
  const char *path = "tl_test_search_matching.txt";
  bool ok = true;

  itl_string_t *found = itl_string_alloc();
  size_t        match;

  itl_g_is_active = true;
  remove(path);
  tl_history_load(path);

  hist_append_cstr("echo alpha");
  hist_append_cstr("line one\nline two");
  hist_append_cstr("echo \xC3\x84");
  hist_append_cstr("ECHO BETA");

  match =
      itl_history_find_match(SEARCH_QUERY("echo"), ITL_HISTORY_NEWEST(), found);
  if (match != 3 || !search_match_is(found, "ECHO BETA")) {
    TEST_PRINTF("the newest folded match was %zu\n", match);
    ok = false;
  }

  if (ok) {
    match = itl_history_find_match(SEARCH_QUERY("echo a"), ITL_HISTORY_NEWEST(),
                                   found);
    if (match != 0 || !search_match_is(found, "echo alpha")) {
      TEST_PRINTF("the oldest match was %zu\n", match);
      ok = false;
    }
  }

  if (ok) {
    match = itl_history_find_match(SEARCH_QUERY("one\nline"),
                                   ITL_HISTORY_NEWEST(), found);
    if (match != 1 || !search_match_is(found, "line one\nline two")) {
      TEST_PRINTF("the multiline match was %zu\n", match);
      ok = false;
    }
  }

  if (ok) {
    match = itl_history_find_match(SEARCH_QUERY("\xC3\x84"),
                                   ITL_HISTORY_NEWEST(), found);
    if (match != 2) {
      TEST_PRINTF("the non-ASCII match was %zu\n", match);
      ok = false;
    }
  }

  if (ok) {
    match = itl_history_find_match(SEARCH_QUERY("\xC3\xA4"),
                                   ITL_HISTORY_NEWEST(), found);
    if (match != ITL_HISTORY_NONE) {
      TEST_PRINTF("a folded non-ASCII query matched %zu\n", match);
      ok = false;
    }
  }

  if (ok) {
    match = itl_history_find_match(SEARCH_QUERY(""), ITL_HISTORY_NEWEST(),
                                   found);
    if (match != 3) {
      TEST_PRINTF("the empty backward query matched %zu\n", match);
      ok = false;
    }
  }

  if (ok) {
    match = itl_history_find_match_forward(SEARCH_QUERY(""), 0, found);
    if (match != 0) {
      TEST_PRINTF("the empty forward query matched %zu\n", match);
      ok = false;
    }
  }

  if (ok) {
    match = itl_history_find_match_forward(SEARCH_QUERY("echo"), 1, found);
    if (match != 2) {
      TEST_PRINTF("the forward folded match was %zu\n", match);
      ok = false;
    }
  }

  ITL_STRING_FREE(found);
  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}

static bool
hist_write_raw(const char *path, const char *bytes, size_t size)
{
  FILE *file = fopen(path, "wb");
  bool  was_written;

  if (file == NULL) {
    return false;
  }

  was_written = fwrite(bytes, 1, size, file) == size;
  fclose(file);

  return was_written;
}

static bool
test_history_search_rejects_malformed_entry(void)
{
  static const char content[] = "echo \xC3 broken\nplain entry\n";
  const char       *path = "tl_test_search_malformed.txt";
  bool              ok = true;

  itl_string_t *found = itl_string_alloc();
  size_t        match;

  itl_g_is_active = true;
  remove(path);
  if (!hist_write_raw(path, content, sizeof(content) - 1)) {
    TEST_PRINTF("could not write the malformed history file\n");
    ok = false;
  }

  if (ok) {
    tl_history_load(path);
    if (itl_g_history_count != 2) {
      TEST_PRINTF("the scan found %zu entries\n", itl_g_history_count);
      ok = false;
    }
  }

  if (ok) {
    match = itl_history_find_match(SEARCH_QUERY("echo"), ITL_HISTORY_NEWEST(),
                                   found);
    if (match != ITL_HISTORY_NONE) {
      TEST_PRINTF("the malformed entry matched at %zu\n", match);
      ok = false;
    }
  }

  if (ok) {
    match = itl_history_find_match(SEARCH_QUERY("plain"), ITL_HISTORY_NEWEST(),
                                   found);
    if (match != 1 || !search_match_is(found, "plain entry")) {
      TEST_PRINTF("the valid neighbour matched at %zu\n", match);
      ok = false;
    }
  }

  ITL_STRING_FREE(found);
  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}

static bool
test_history_search_narrowing(void)
{
  const char *path = "tl_test_search_narrowing.txt";
  bool ok = true;

  itl_string_t *found = itl_string_alloc();
  size_t        match;
  size_t        rescan;

  itl_g_is_active = true;
  remove(path);
  tl_history_load(path);

  hist_append_cstr("aaa target");
  hist_append_cstr("aaa filler one");
  hist_append_cstr("aaa filler two");
  hist_append_cstr("aaa filler three");
  hist_append_cstr("aaa filler four");

  itl_g_debug_history_candidate_count = 0;
  match = itl_history_narrow_match(SEARCH_QUERY("z"), ITL_HISTORY_NONE, false,
                                   found);
  match = itl_history_narrow_match(SEARCH_QUERY("zz"), match, true, found);
  match = itl_history_narrow_match(SEARCH_QUERY("zzz"), match, true, found);

  if (match != ITL_HISTORY_NONE || itl_g_debug_history_candidate_count != 5) {
    TEST_PRINTF("the growing miss ended at %zu after %zu candidates\n", match,
                itl_g_debug_history_candidate_count);
    ok = false;
  }

  if (ok) {
    itl_g_debug_history_candidate_count = 0;
    match = itl_history_narrow_match(SEARCH_QUERY("aaa"), ITL_HISTORY_NONE,
                                     false, found);
    match = itl_history_narrow_match(SEARCH_QUERY("aaa t"), match, true, found);
    match = itl_history_narrow_match(SEARCH_QUERY("aaa ta"), match, true, found);
    match =
        itl_history_narrow_match(SEARCH_QUERY("aaa tar"), match, true, found);

    if (match != 0 || !search_match_is(found, "aaa target")) {
      TEST_PRINTF("the growing hit ended at %zu\n", match);
      ok = false;
    }
    if (ok && itl_g_debug_history_candidate_count != 8) {
      TEST_PRINTF("the growing hit decoded %zu candidates\n",
                  itl_g_debug_history_candidate_count);
      ok = false;
    }
  }

  if (ok) {
    itl_g_debug_history_candidate_count = 0;
    rescan = itl_history_narrow_match(SEARCH_QUERY("aaa targ"), match, false,
                                      found);

    if (rescan != 0 || itl_g_debug_history_candidate_count != 5) {
      TEST_PRINTF("the rescan ended at %zu after %zu candidates\n", rescan,
                  itl_g_debug_history_candidate_count);
      ok = false;
    }
  }

  if (ok) {
    match = itl_history_narrow_match(SEARCH_QUERY("aaa"), ITL_HISTORY_NONE,
                                     false, found);
    match = itl_history_narrow_match(SEARCH_QUERY("aaa t"), match, true, found);
    rescan = itl_history_find_match(SEARCH_QUERY("aaa t"), ITL_HISTORY_NEWEST(),
                                    found);

    if (match != rescan) {
      TEST_PRINTF("narrowing gave %zu and the rescan gave %zu\n", match,
                  rescan);
      ok = false;
    }
  }

  ITL_STRING_FREE(found);
  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}

#if defined ITL_POSIX
static int
test_search_highlight_callback(const char *buffer, tl_highlight *out)
{
  (void) buffer;

  if (out->capacity == 0) {
    return 0;
  }

  out->spans[0].start = 0;
  out->spans[0].end = 1;
  out->spans[0].sgr = "\x1b[31m";
  out->count = 1;

  return 1;
}

static size_t
search_highlight_calls(const char *keys, size_t key_count)
{
  char out_buffer[BUFFER_SIZE];
  int  pipe_descriptors[2] = {-1, -1};
  int  null_descriptor = -1;
  int  saved_stdin = -1;
  int  saved_stdout = -1;
  size_t calls = (size_t) -1;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  if (pipe(pipe_descriptors) != 0) goto cleanup;
  if (write(pipe_descriptors[1], keys, key_count) != (ssize_t) key_count) {
    goto cleanup;
  }
  close(pipe_descriptors[1]);
  pipe_descriptors[1] = -1;

  null_descriptor = open("/dev/null", O_WRONLY);
  if (null_descriptor < 0) goto cleanup;

  saved_stdin = dup(STDIN_FILENO);
  saved_stdout = dup(STDOUT_FILENO);
  if (saved_stdin < 0 || saved_stdout < 0) goto cleanup;
  if (dup2(pipe_descriptors[0], STDIN_FILENO) < 0 ||
      dup2(null_descriptor, STDOUT_FILENO) < 0)
  {
    goto cleanup;
  }

  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "> ");
  itl_g_tty_changed_size = 0;
  itl_g_tty_prev_rows = 24;
  itl_g_tty_prev_cols = 80;
  itl_g_debug_search_highlight_count = 0;
  tl_set_highlight_callback(test_search_highlight_callback);

  (void) itl_history_search(&le);

  calls = itl_g_debug_search_highlight_count;
  tl_set_highlight_callback(NULL);
  itl_g_search_spans_active = false;

cleanup:
  if (saved_stdin >= 0) {
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdin);
  }
  if (saved_stdout >= 0) {
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
  }
  if (null_descriptor >= 0) close(null_descriptor);
  if (pipe_descriptors[0] >= 0) close(pipe_descriptors[0]);
  if (pipe_descriptors[1] >= 0) close(pipe_descriptors[1]);

  ITL_STRING_FREE(line);
  itl_g_tty_changed_size = 1;

  return calls;
}

static bool
test_history_search_preview_cache(void)
{
  const char *path = "tl_test_search_preview.txt";
  bool ok = true;

  size_t query_only_calls;
  size_t moving_calls;

  itl_g_is_active = true;
  remove(path);
  tl_history_load(path);

  hist_append_cstr("aaa older entry");
  hist_append_cstr("aaa newer entry");

  query_only_calls = search_highlight_calls("aaa", 3);
  if (query_only_calls != 2) {
    TEST_PRINTF("a query-only redraw highlighted %zu times\n",
                query_only_calls);
    ok = false;
  }

  if (ok) {
    moving_calls = search_highlight_calls("aaa\x12", 4);
    if (moving_calls != 3) {
      TEST_PRINTF("a moving redraw highlighted %zu times\n", moving_calls);
      ok = false;
    }
  }

  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}
#endif

static bool
test_history_short_entry_skipped(void)
{
  const char *path = "tl_test_short.txt";
  bool ok = true;

  itl_g_is_active = true;
  remove(path);
  tl_history_load(path);

  /* Entries of length one or zero are not persisted, matching the dumper. */
  if (hist_append_cstr("q") || hist_append_cstr("")) {
    TEST_PRINTF("a short entry was appended\n");
    ok = false;
  }
  if (ok && itl_g_history_count != 0) {
    TEST_PRINTF("count %zu, expected 0\n", itl_g_history_count);
    ok = false;
  }

  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}

static bool
test_history_alloc_balance(void)
{
  const char *path = "tl_test_alloc.txt";
  bool   ok = true;
  size_t before;

  itl_string_t *scratch;

  itl_g_is_active = true;
  remove(path);
  before = itl_g_alloc_count;

  /* Exercise load, append, read, and search, then release everything and check
     that the allocation count returns to where it started. */
  tl_history_load(path);
  hist_append_cstr("alpha one");
  hist_append_cstr("beta two");
  hist_append_cstr("alpha three");

  scratch = itl_string_alloc();
  (void) itl_history_find_match(SEARCH_QUERY("alpha"), itl_g_history_count - 1,
                                scratch);
  ITL_STRING_FREE(scratch);

  itl_g_history_free();

  if (itl_g_alloc_count != before) {
    TEST_PRINTF("alloc count %zu, expected %zu, a history path leaked\n",
                itl_g_alloc_count, before);
    ok = false;
  }

  remove(path);
  itl_g_is_active = false;
  return ok;
}

static bool
test_history_private_branch(void)
{
  const char *path = "tl_test_merge.txt";
  bool  ok = true;
  FILE *other;

  itl_g_is_active = true;
  remove(path);
  tl_history_load(path);

  hist_append_cstr("first one");

  /* Simulate another session appending straight to the file. */
  other = fopen(path, "ab");
  if (other == NULL) {
    TEST_PRINTF("could not open the file as another session\n");
    itl_g_is_active = false;
    return false;
  }
  fputs("second two\n", other);
  fclose(other);

  /* Our next append persists without merging the other session's entry. */
  hist_append_cstr("third three");

  if (itl_g_history_total_count != 2 ||
      itl_g_last_history_event_number != 2)
  {
    TEST_PRINTF("private total %zu and last %zu\n",
                itl_g_history_total_count,
                itl_g_last_history_event_number);
    ok = false;
  }

  if (itl_g_history_count != 2) {
    TEST_PRINTF("expected 2 private entries, got %zu\n", itl_g_history_count);
    ok = false;
  }
  if (ok && (!hist_entry_is(0, "first one") ||
             !hist_entry_is(1, "third three")))
  {
    TEST_PRINTF("private entries are out of order or wrong\n");
    ok = false;
  }

  if (ok && tl_history_load(path) != TL_SUCCESS) {
    TEST_PRINTF("could not reload the durable history\n");
    ok = false;
  }
  if (ok &&
      (itl_g_history_count != 3 || !hist_entry_is(0, "first one") ||
       !hist_entry_is(1, "second two") ||
       !hist_entry_is(2, "third three")))
  {
    TEST_PRINTF("reloaded entries are out of order or wrong\n");
    ok = false;
  }

  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}

/* Selects the entry at index through the editor recall path and compares the
   resulting line to expected. */
static bool
recall_line_is(size_t index, const char *expected)
{
  char          out_buffer[BUFFER_SIZE];
  char          line_buffer[BUFFER_SIZE];
  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();
  bool          is_equal;

  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  le.history_selected_index = index;
  itl_history_show_selected(&le);
  is_equal = itl_string_to_cstr(line, line_buffer, sizeof(line_buffer)) ==
                 TL_SUCCESS &&
             strcmp(line_buffer, expected) == 0;
  ITL_STRING_FREE(line);

  return is_equal;
}

static bool
test_history_recall_after_peer_write(void)
{
  const char *path = "tl_test_recall_peer.txt";
  bool        ok = true;
  FILE       *other;

  itl_g_is_active = true;
  remove(path);
  tl_history_load(path);
  hist_append_cstr("mine one");
  hist_append_cstr("mine two");

  other = fopen(path, "ab");
  if (other == NULL) {
    TEST_PRINTF("could not open the file as another session\n");
    itl_g_history_free();
    itl_g_is_active = false;
    return false;
  }
  fputs("peer entry that is long enough to shift offsets\n", other);
  fclose(other);

  if (!recall_line_is(0, "mine one") || !recall_line_is(1, "mine two")) {
    TEST_PRINTF("recall after a peer append left the private branch\n");
    ok = false;
  }

  other = fopen(path, "wb");
  if (other == NULL) {
    TEST_PRINTF("could not truncate the file as another session\n");
    remove(path);
    itl_g_history_free();
    itl_g_is_active = false;
    return false;
  }
  fputs("p\n", other);
  fclose(other);

  if (!recall_line_is(0, "mine one") || !recall_line_is(1, "mine two")) {
    TEST_PRINTF("recall after a peer truncation left the private branch\n");
    ok = false;
  }

  hist_append_cstr("mine three");
  if (ok && (itl_g_history_count != 3 || !recall_line_is(0, "mine one") ||
             !recall_line_is(1, "mine two") ||
             !recall_line_is(2, "mine three")))
  {
    TEST_PRINTF("append after a peer truncation changed the branch\n");
    ok = false;
  }

  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}

static bool
test_completion_replacement_is_atomic(void)
{
  char out_buffer[6];
  char line_buffer[BUFFER_SIZE];
  bool was_replaced;
  itl_le_t le = ITL_ZERO_INIT;
  tl_completion completion = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  ITL_STRING_FROM_CSTR(line, "abcde");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  completion.token_start = 4;
  completion.token_end = 5;
  was_replaced = itl_completion_replace_token(&le, &completion, "xyz");
  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));

  ITL_STRING_FREE(line);
  return !was_replaced && strcmp(line_buffer, "abcde") == 0;
}

static bool
test_alt_arrows_use_word_movement(void)
{
  char out_buffer[BUFFER_SIZE];
  bool left_matches;
  bool right_matches;
  bool ghost_word_was_accepted;
  char word_buffer[BUFFER_SIZE];
  itl_le_t ctrl_le = ITL_ZERO_INIT;
  itl_le_t alt_le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  ITL_STRING_FROM_CSTR(line, "alpha beta");
  itl_le_init(&ctrl_le, line, out_buffer, sizeof(out_buffer), "");
  itl_le_init(&alt_le, line, out_buffer, sizeof(out_buffer), "");
  itl_le_key_handle(&ctrl_le, TL_KEY_LEFT | TL_MOD_CTRL);
  itl_le_key_handle(&alt_le, TL_KEY_LEFT | TL_MOD_ALT);
  left_matches = ctrl_le.cursor_position == alt_le.cursor_position;

  ctrl_le.cursor_position = 0;
  alt_le.cursor_position = 0;
  itl_le_key_handle(&ctrl_le, TL_KEY_RIGHT | TL_MOD_CTRL);
  itl_le_key_handle(&alt_le, TL_KEY_RIGHT | TL_MOD_ALT);
  right_matches = ctrl_le.cursor_position == alt_le.cursor_position;

  alt_le.cursor_position = line->length;
  memcpy(itl_g_ghost, " tail end", 10);
  itl_g_ghost_len = 9;
  itl_le_key_handle(&alt_le, TL_KEY_RIGHT | TL_MOD_ALT);
  itl_string_to_cstr(line, word_buffer, sizeof(word_buffer));
  ghost_word_was_accepted = strcmp(word_buffer, "alpha beta tail") == 0 &&
                            itl_g_ghost_len == 0;
  itl_ghost_clear();

  ITL_STRING_FREE(line);
  return left_matches && right_matches && ghost_word_was_accepted;
}

static bool
test_ctrl_right_accepts_one_ghost_word(void)
{
  char out_buffer[BUFFER_SIZE];
  char line_buffer[BUFFER_SIZE];
  bool ok = true;
  itl_le_t le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  ITL_STRING_FROM_CSTR(line, "git");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  le.cursor_position = line->length;
  memcpy(itl_g_ghost, " commit --amend", 16);
  itl_g_ghost_len = 15;

  itl_le_key_handle(&le, TL_KEY_RIGHT | TL_MOD_CTRL);
  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));
  if (strcmp(line_buffer, "git commit") != 0) {
    TEST_PRINTF("word accept gave '%s'\n", line_buffer);
    ok = false;
  }

  memcpy(itl_g_ghost, " --amend", 9);
  itl_g_ghost_len = 8;
  itl_le_key_handle(&le, TL_KEY_RIGHT);
  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));
  if (strcmp(line_buffer, "git commit --amend") != 0) {
    TEST_PRINTF("full accept gave '%s'\n", line_buffer);
    ok = false;
  }

  itl_ghost_clear();
  ITL_STRING_FREE(line);
  return ok;
}

static bool
test_ctrl_right_accepts_one_case_corrected_word(void)
{
  char out_buffer[BUFFER_SIZE];
  char line_buffer[BUFFER_SIZE];
  bool ok = true;
  itl_le_t le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  ITL_STRING_FROM_CSTR(line, "git");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  le.cursor_position = line->length;
  memcpy(itl_g_ghost, " Status --Json", 15);
  itl_g_ghost_len = 14;
  itl_g_ghost_should_replace_line = true;
  memcpy(itl_g_ghost_sticky_target, "Git Status --Json", 18);

  itl_le_key_handle(&le, TL_KEY_RIGHT | TL_MOD_ALT);
  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));
  if (strcmp(line_buffer, "Git Status") != 0) {
    TEST_PRINTF("corrected word accept gave '%s'\n", line_buffer);
    ok = false;
  }

  itl_ghost_clear();
  itl_g_ghost_sticky_target[0] = '\0';
  ITL_STRING_FREE(line);
  return ok;
}

static bool
test_line_is(const itl_string_t *line, const char *expected, const char *step)
{
  char line_buffer[BUFFER_SIZE];

  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));
  if (strcmp(line_buffer, expected) != 0) {
    printf("%s gave '%s', expected '%s'\n", step, line_buffer, expected);
    return false;
  }

  return true;
}

static bool
test_kill_ring_appends_yanks_and_cycles(void)
{
  char out_buffer[BUFFER_SIZE];
  bool ok = true;
  itl_le_t le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  ITL_STRING_FROM_CSTR(line, "one two three");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  itl_g_le_action = ITL_LE_ACTION_NONE;
  itl_le_key_handle(&le, TL_KEY_KILL_LINE_BEFORE);
  ok &= test_line_is(line, "", "ctrl-u");

  ITL_STRING_FROM_CSTR(line, "four five");
  le.cursor_position = line->length;
  itl_le_key_handle(&le, TL_KEY_HOME);
  itl_le_key_handle(&le, TL_KEY_KILL_LINE);
  ok &= test_line_is(line, "", "ctrl-k");

  ITL_STRING_FROM_CSTR(line, "a b c");
  le.cursor_position = line->length;
  itl_le_key_handle(&le, TL_KEY_END);
  itl_le_key_handle(&le, TL_KEY_BACKSPACE | TL_MOD_CTRL);
  itl_le_key_handle(&le, TL_KEY_BACKSPACE | TL_MOD_CTRL);
  itl_le_key_handle(&le, TL_KEY_BACKSPACE | TL_MOD_CTRL);
  ok &= test_line_is(line, "a ", "three ctrl-w");

  itl_le_key_handle(&le, TL_KEY_YANK);
  ok &= test_line_is(line, "a b c", "yank of the appended kill");
  itl_le_key_handle(&le, TL_KEY_YANK_POP);
  ok &= test_line_is(line, "a four five", "first alt-y");
  itl_le_key_handle(&le, TL_KEY_YANK_POP);
  ok &= test_line_is(line, "a one two three", "second alt-y");
  itl_le_key_handle(&le, TL_KEY_YANK_POP);
  ok &= test_line_is(line, "a b c", "alt-y past the oldest");
  if (le.cursor_position != line->length) {
    TEST_PRINTF("cursor after alt-y is %zu\n", le.cursor_position);
    ok = false;
  }

  itl_le_key_handle(&le, TL_KEY_UNDO);
  ok &= test_line_is(line, "a ", "undo of the yank");

  itl_le_key_handle(&le, TL_KEY_YANK_POP);
  ok &= test_line_is(line, "a ", "alt-y without a yank");

  le.cursor_position = 0;
  itl_le_key_handle(&le, TL_KEY_DELETE | TL_MOD_CTRL);
  itl_le_key_handle(&le, TL_KEY_YANK);
  ok &= test_line_is(line, "a ", "alt-d then yank");

  ITL_STRING_FREE(line);
  itl_kill_ring_free();
  return ok;
}

static bool
test_ghost_miss_skips_an_empty_word(void)
{
  bool ok = true;

  itl_ghost_record_completion_miss("cd ", 3);
  ok &= itl_g_ghost_completion_miss_prefix_length == 0;
  ok &= !itl_ghost_extends_completion_miss_plainly("cd zz", 5);
  itl_ghost_record_completion_miss("cd zq", 5);
  ok &= itl_ghost_extends_completion_miss_plainly("cd zqx", 6);

  itl_g_ghost_completion_miss_prefix[0] = '\0';
  itl_g_ghost_completion_miss_prefix_length = 0;
  return ok;
}

static bool
test_vi_dot_skips_a_yank(void)
{
  char out_buffer[BUFFER_SIZE];
  bool ok = true;
  itl_le_t le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();
  itl_utf8_t none = ITL_ZERO_INIT;

  ITL_STRING_FROM_CSTR(line, "one two three four");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  le.cursor_position = 0;
  itl_vi_operator_motion(&le, ITL_VI_OP_DELETE, 'w', none, 1);
  ok &= test_line_is(line, "two three four", "dw");
  itl_vi_operator_motion(&le, ITL_VI_OP_YANK, 'w', none, 1);
  itl_vi_operator_line(&le, ITL_VI_OP_YANK, 'y', 1);
  le.cursor_position = 0;
  ok &= itl_vi_repeat_last_change(&le) == TL_SUCCESS;
  ok &= test_line_is(line, "three four", "dot after yw and yy repeats dw");

  itl_g_vi_last_change.kind = ITL_VI_CHANGE_NONE;
  ITL_STRING_FREE(line);
  return ok;
}

#if defined ITL_POSIX
/* Leaving the vi : command line by Escape, by Backspace on an empty command,
   or by Enter on an unknown command keeps the line and the caret where they
   were before the colon. */
static bool
test_vi_ex_command_keeps_the_caret(void)
{
  static const char *const keys[] = {"\x1b", "\x7f", "ab\x7f\x7f\x7f",
                                     "zz\r"};
  char   out_buffer[BUFFER_SIZE];
  bool   ok = true;
  size_t i;

  for (i = 0; i < countof(keys); ++i) {
    int           pipe_descriptors[2] = {-1, -1};
    int           null_descriptor = open("/dev/null", O_WRONLY);
    int           saved_stdin = dup(STDIN_FILENO);
    int           saved_stdout = dup(STDOUT_FILENO);
    size_t        key_size = strlen(keys[i]);
    itl_le_t      le = ITL_ZERO_INIT;
    itl_string_t *line = itl_string_alloc();

    if (pipe(pipe_descriptors) != 0 || null_descriptor < 0 ||
        saved_stdin < 0 || saved_stdout < 0 ||
        write(pipe_descriptors[1], keys[i], key_size) != (ssize_t) key_size ||
        dup2(pipe_descriptors[0], STDIN_FILENO) < 0 ||
        dup2(null_descriptor, STDOUT_FILENO) < 0)
    {
      TEST_PRINTF("could not feed case %zu\n", i);
      ok = false;
    } else {
      close(pipe_descriptors[1]);
      pipe_descriptors[1] = -1;
      ITL_STRING_FROM_CSTR(line, "one two three");
      itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
      le.cursor_position = 4;
      (void) itl_vi_ex_command(&le);
      dup2(saved_stdout, STDOUT_FILENO);
      ok &= test_line_is(line, "one two three", "leaving the : line");
      if (le.line != line || le.cursor_position != 4) {
        TEST_PRINTF("case %zu left the caret at %zu\n", i,
                    le.cursor_position);
        ok = false;
      }
    }

    dup2(saved_stdin, STDIN_FILENO);
    dup2(saved_stdout, STDOUT_FILENO);
    if (saved_stdin >= 0) close(saved_stdin);
    if (saved_stdout >= 0) close(saved_stdout);
    if (null_descriptor >= 0) close(null_descriptor);
    if (pipe_descriptors[0] >= 0) close(pipe_descriptors[0]);
    if (pipe_descriptors[1] >= 0) close(pipe_descriptors[1]);
    itl_g_key_queue_index = 0;
    itl_g_key_queue_length = 0;
    itl_g_pushback_byte = -1;
    itl_g_tty_changed_size = 1;
    itl_g_tty_first_render = true;
    ITL_STRING_FREE(line);
  }

  return ok;
}
#endif /* ITL_POSIX */

/* A completion and the space after it are one undo step, and undoing it gives
   back the token and the caret as they were before the TAB. */
static bool
test_completion_is_one_undo_step(void)
{
  char                      out_buffer[BUFFER_SIZE];
  bool                      ok = true;
  itl_le_t                  le = ITL_ZERO_INIT;
  itl_string_t             *line = itl_string_alloc();
  tl_completion             result = ITL_ZERO_INIT;
  tl_space_after_completion previous_space_after =
      itl_g_space_after_completion;

  tl_set_space_after_completion(TL_SPACE_AFTER_COMPLETION_ON);
  ITL_STRING_FROM_CSTR(line, "ls loc x");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  le.cursor_position = 5;
  result.token_start = 3;
  result.token_end = 6;
  ok &= itl_completion_replace_token(&le, &result, "local");
  ok &= test_line_is(line, "ls local x", "a completion inside the line");

  ITL_STRING_FROM_CSTR(line, "ls fi");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  le.cursor_position = 5;
  result.token_end = 5;
  ok &= itl_completion_replace_token(&le, &result, "file1.txt");
  itl_completion_append_space(&le);
  ok &= test_line_is(line, "ls file1.txt ", "a completion with its space");
  ok &= itl_undo_pop(&le);
  ok &= test_line_is(line, "ls fi", "undo of the completion");
  if (le.cursor_position != 5 || itl_undo_pop(&le)) {
    TEST_PRINTF("undo left the caret at %zu or took two steps\n",
                le.cursor_position);
    ok = false;
  }

  tl_set_space_after_completion(previous_space_after);
  ITL_STRING_FREE(line);
  return ok;
}

#define TEST_FLAG_US   "\xF0\x9F\x87\xBA\xF0\x9F\x87\xB8"
#define TEST_WAVE_DARK "\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBF"
#define TEST_FAMILY                                                            \
  "\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7"

/* Left, Right, Backspace, and Delete step over a whole emoji sequence, and
   the line, a prompt, and a C string all count it as two columns, the width
   a terminal draws it with. */
static bool
test_emoji_sequences_move_and_measure_whole(void)
{
  static const struct
  {
    const char *text;
    size_t      columns;
  } widths[] = {
      {TEST_FLAG_US,   2},
      {TEST_WAVE_DARK, 2},
      {TEST_FAMILY,    2},
  };
  char              out_buffer[BUFFER_SIZE];
  bool              ok = true;
  itl_le_t          le = ITL_ZERO_INIT;
  itl_le_metrics_t  metrics;
  itl_string_t     *line = itl_string_alloc();
  size_t            i;

  for (i = 0; i < countof(widths); ++i) {
    size_t columns = itl_cstr_display_width(widths[i].text);

    ITL_STRING_FROM_CSTR(line, widths[i].text);
    itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
    le.cursor_position = line->length;
    metrics = itl_le_compute_metrics(&le, 80);
    if (columns != widths[i].columns ||
        metrics.cursor_col != widths[i].columns)
    {
      TEST_PRINTF("sequence %zu measured %zu and %zu columns\n", i, columns,
                  metrics.cursor_col);
      ok = false;
    }
  }

  ITL_STRING_FROM_CSTR(line, "echo " TEST_WAVE_DARK " end");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  le.cursor_position = line->length;
  metrics = itl_le_compute_metrics(&le, 80);
  if (metrics.cursor_col != 11) {
    TEST_PRINTF("caret after a skin tone is at column %zu\n",
                metrics.cursor_col);
    ok = false;
  }

  ITL_STRING_FROM_CSTR(line, "a" TEST_FAMILY "b");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  le.cursor_position = line->length - 1;
  itl_le_key_handle(&le, TL_KEY_LEFT);
  if (le.cursor_position != 1) {
    TEST_PRINTF("left over a joined sequence reached %zu\n",
                le.cursor_position);
    ok = false;
  }
  itl_le_key_handle(&le, TL_KEY_RIGHT);
  if (le.cursor_position != line->length - 1) {
    TEST_PRINTF("right over a joined sequence reached %zu\n",
                le.cursor_position);
    ok = false;
  }
  itl_le_key_handle(&le, TL_KEY_BACKSPACE);
  ok &= test_line_is(line, "ab", "backspace after a joined sequence");

  ITL_STRING_FROM_CSTR(line, "a" TEST_FLAG_US TEST_FLAG_US);
  le.cursor_position = 1;
  itl_le_key_handle(&le, TL_KEY_DELETE);
  ok &= test_line_is(line, "a" TEST_FLAG_US, "delete before two flags");

  ITL_STRING_FREE(line);
  return ok;
}

static bool
test_transpose_characters_and_words(void)
{
  char out_buffer[BUFFER_SIZE];
  bool ok = true;
  itl_le_t le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  ITL_STRING_FROM_CSTR(line, "abc");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  le.cursor_position = 1;
  itl_le_key_handle(&le, TL_KEY_TRANSPOSE);
  ok &= test_line_is(line, "bac", "ctrl-t inside the line");
  if (le.cursor_position != 2) {
    TEST_PRINTF("cursor after ctrl-t is %zu\n", le.cursor_position);
    ok = false;
  }

  le.cursor_position = line->length;
  itl_le_key_handle(&le, TL_KEY_TRANSPOSE);
  ok &= test_line_is(line, "bca", "ctrl-t at the end");

  le.cursor_position = 0;
  itl_le_key_handle(&le, TL_KEY_TRANSPOSE);
  ok &= test_line_is(line, "bca", "ctrl-t at the start");

  ITL_STRING_FROM_CSTR(line, "привет");
  le.cursor_position = line->length;
  itl_le_key_handle(&le, TL_KEY_TRANSPOSE);
  ok &= test_line_is(line, "привте", "ctrl-t on UTF-8");

  ITL_STRING_FROM_CSTR(line, "xae\xcc\x81");
  le.cursor_position = line->length;
  itl_le_key_handle(&le, TL_KEY_TRANSPOSE);
  ok &= test_line_is(line, "xe\xcc\x81" "a", "ctrl-t keeps a trailing mark");
  if (le.cursor_position != line->length) {
    TEST_PRINTF("cursor after ctrl-t over a mark is %zu\n", le.cursor_position);
    ok = false;
  }

  le.cursor_position = 2;
  itl_le_key_handle(&le, TL_KEY_TRANSPOSE);
  ok &= test_line_is(line, "e\xcc\x81xa", "ctrl-t moves a marked base");
  if (le.cursor_position != 3) {
    TEST_PRINTF("cursor after ctrl-t on a marked base is %zu\n",
                le.cursor_position);
    ok = false;
  }

  ITL_STRING_FROM_CSTR(line, "\xcc\x81" "a");
  le.cursor_position = 1;
  itl_le_key_handle(&le, TL_KEY_TRANSPOSE);
  ok &= test_line_is(line, "a\xcc\x81", "ctrl-t on a leading mark");

  ITL_STRING_FROM_CSTR(line, "x" TEST_FLAG_US);
  le.cursor_position = line->length;
  itl_le_key_handle(&le, TL_KEY_TRANSPOSE);
  ok &= test_line_is(line, TEST_FLAG_US "x", "ctrl-t keeps a flag whole");

  ITL_STRING_FROM_CSTR(line, TEST_WAVE_DARK "z");
  le.cursor_position = line->length;
  itl_le_key_handle(&le, TL_KEY_TRANSPOSE);
  ok &= test_line_is(line, "z" TEST_WAVE_DARK, "ctrl-t keeps a skin tone");

  ITL_STRING_FROM_CSTR(line, TEST_FAMILY "z");
  le.cursor_position = line->length;
  itl_le_key_handle(&le, TL_KEY_TRANSPOSE);
  ok &= test_line_is(line, "z" TEST_FAMILY, "ctrl-t keeps a joined sequence");

  ITL_STRING_FROM_CSTR(line, "cp src dst");
  le.cursor_position = 3;
  itl_le_key_handle(&le, TL_KEY_TRANSPOSE | TL_MOD_ALT);
  ok &= test_line_is(line, "src cp dst", "alt-t inside the line");
  if (le.cursor_position != 6) {
    TEST_PRINTF("cursor after alt-t is %zu\n", le.cursor_position);
    ok = false;
  }

  le.cursor_position = line->length;
  itl_le_key_handle(&le, TL_KEY_TRANSPOSE | TL_MOD_ALT);
  ok &= test_line_is(line, "src dst cp", "alt-t at the end");

  ITL_STRING_FROM_CSTR(line, "  один  ");
  le.cursor_position = line->length;
  itl_le_key_handle(&le, TL_KEY_TRANSPOSE | TL_MOD_ALT);
  ok &= test_line_is(line, "  один  ", "alt-t with one word");

  ITL_STRING_FROM_CSTR(line, "a b ");
  le.cursor_position = line->length;
  itl_le_key_handle(&le, TL_KEY_TRANSPOSE | TL_MOD_ALT);
  ok &= test_line_is(line, "b a ", "alt-t before trailing blanks");
  if (le.cursor_position != 3) {
    TEST_PRINTF("cursor after alt-t before blanks is %zu\n",
                le.cursor_position);
    ok = false;
  }

  ITL_STRING_FROM_CSTR(line, "один два");
  le.cursor_position = line->length;
  itl_le_key_handle(&le, TL_KEY_TRANSPOSE | TL_MOD_ALT);
  ok &= test_line_is(line, "два один", "alt-t on UTF-8");

  itl_le_key_handle(&le, TL_KEY_UNDO);
  ok &= test_line_is(line, "один два", "undo of alt-t");

  ITL_STRING_FREE(line);
  return ok;
}

static bool
test_last_argument_walks_history(void)
{
  const char *path = "tl_test_last_argument.txt";
  char out_buffer[BUFFER_SIZE];
  bool ok = true;
  itl_le_t le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  itl_g_is_active = true;
  remove(path);
  tl_history_load(path);
  if (!hist_append_cstr("cat 'a b'")) ok = false;
  if (!hist_append_cstr("touch file1 ")) ok = false;
  if (!hist_append_cstr("git commit -m \"x y\"")) ok = false;

  ITL_STRING_FROM_CSTR(line, "vim ");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  itl_g_le_action = ITL_LE_ACTION_NONE;
  itl_le_key_handle(&le, TL_KEY_LAST_ARGUMENT);
  ok &= test_line_is(line, "vim \"x y\"", "first alt-.");
  itl_le_key_handle(&le, TL_KEY_LAST_ARGUMENT);
  ok &= test_line_is(line, "vim file1", "second alt-.");
  itl_le_key_handle(&le, TL_KEY_LAST_ARGUMENT);
  ok &= test_line_is(line, "vim 'a b'", "third alt-.");
  itl_le_key_handle(&le, TL_KEY_LAST_ARGUMENT);
  ok &= test_line_is(line, "vim 'a b'", "alt-. past the oldest");

  itl_le_key_handle(&le, TL_KEY_UNDO);
  ok &= test_line_is(line, "vim ", "undo of the alt-. walk");

  itl_le_key_handle(&le, TL_KEY_LAST_ARGUMENT);
  itl_le_key_handle(&le, TL_KEY_HOME);
  itl_le_key_handle(&le, TL_KEY_LAST_ARGUMENT);
  ok &= test_line_is(line, "\"x y\"vim \"x y\"", "alt-. after a motion");

  ITL_STRING_FREE(line);
  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}

static const char *test_edit_seen_buffer = NULL;
static char test_edit_seen_copy[BUFFER_SIZE];

static int
test_edit_callback(const char *buffer, const char **out_edited)
{
  snprintf(test_edit_seen_copy, sizeof(test_edit_seen_copy), "%s", buffer);
  test_edit_seen_buffer = test_edit_seen_copy;
  *out_edited = "edited\nline";
  return 1;
}

static int
test_edit_declining_callback(const char *buffer, const char **out_edited)
{
  (void) buffer;
  (void) out_edited;
  return 0;
}

static bool
test_edit_external_replaces_the_line(void)
{
  char out_buffer[BUFFER_SIZE];
  bool ok = true;
  itl_le_t le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  ITL_STRING_FROM_CSTR(line, "draft");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  itl_le_key_handle(&le, TL_KEY_EDIT_EXTERNAL);
  ok &= test_line_is(line, "draft", "ctrl-x ctrl-e without a callback");

  tl_set_edit_callback(test_edit_declining_callback);
  itl_le_key_handle(&le, TL_KEY_EDIT_EXTERNAL);
  ok &= test_line_is(line, "draft", "declined edit");

  tl_set_edit_callback(test_edit_callback);
  le.cursor_position = 2;
  itl_le_key_handle(&le, TL_KEY_EDIT_EXTERNAL);
  ok &= test_line_is(line, "edited\nline", "ctrl-x ctrl-e");
  if (test_edit_seen_buffer == NULL ||
      strcmp(test_edit_seen_buffer, "draft") != 0)
  {
    TEST_PRINTF("the callback did not receive the line\n");
    ok = false;
  }
  if (le.cursor_position != line->length) {
    TEST_PRINTF("cursor after the edit is %zu\n", le.cursor_position);
    ok = false;
  }

  itl_le_key_handle(&le, TL_KEY_UNDO);
  ok &= test_line_is(line, "draft", "undo of the edit");

  tl_set_edit_callback(NULL);
  ITL_STRING_FREE(line);
  return ok;
}

static bool
test_editing_key_sequences(void)
{
  bool ok = true;

  itl_g_pushback_byte = 'y';
  ok &= itl_esc_parse(27) == TL_KEY_YANK_POP;
  itl_g_pushback_byte = 't';
  ok &= itl_esc_parse(27) == (TL_KEY_TRANSPOSE | TL_MOD_ALT);
  itl_g_pushback_byte = '.';
  ok &= itl_esc_parse(27) == TL_KEY_LAST_ARGUMENT;
  itl_g_pushback_byte = '_';
  ok &= itl_esc_parse(27) == TL_KEY_LAST_ARGUMENT;
  ok &= itl_esc_parse(20) == TL_KEY_TRANSPOSE;
  ok &= itl_esc_parse(25) == TL_KEY_YANK;
  itl_g_pushback_byte = 5;
  ok &= itl_esc_parse(24) == TL_KEY_EDIT_EXTERNAL;
  itl_g_pushback_byte = 21;
  ok &= itl_esc_parse(24) == TL_KEY_UNDO;
  itl_g_pushback_byte = 'q';
  ok &= itl_esc_parse(24) == TL_KEY_UNKN;
  ok &= itl_g_pushback_byte == 'q';
  itl_g_pushback_byte = 27;
  ok &= itl_esc_parse(24) == TL_KEY_UNKN;
  ok &= itl_g_pushback_byte == 27;
  itl_g_pushback_byte = -1;

  ok &= itl_esc_parse(26) == TL_KEY_UNDO;
  ok &= itl_esc_parse(30) == TL_KEY_REDO;
  ok &= itl_esc_parse(31) == TL_KEY_UNDO;

  return ok;
}

#if defined ITL_POSIX
typedef struct key_sequence_case key_sequence_case_t;

struct key_sequence_case
{
  const char *bytes;
  int         key;
};

/* Parse one key from bytes and report whether any of them was left unread. */
static int
test_parse_key_bytes(const char *bytes, bool *was_drained)
{
  int    pipe_descriptors[2] = {-1, -1};
  int    saved_stdin = -1;
  int    key = -1;
  size_t tail_size = strlen(bytes) - 1;

  *was_drained = false;
  if (pipe(pipe_descriptors) != 0) goto cleanup;
  if (write(pipe_descriptors[1], bytes + 1, tail_size) != (ssize_t) tail_size)
    goto cleanup;
  saved_stdin = dup(STDIN_FILENO);
  if (saved_stdin < 0 || dup2(pipe_descriptors[0], STDIN_FILENO) < 0)
    goto cleanup;

  key = itl_esc_parse((uint8_t) bytes[0]);
  *was_drained = !itl_input_is_pending();

cleanup:
  if (saved_stdin >= 0) {
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdin);
  }
  if (pipe_descriptors[0] >= 0) close(pipe_descriptors[0]);
  if (pipe_descriptors[1] >= 0) close(pipe_descriptors[1]);
  return key;
}

static bool
test_modified_key_sequences(void)
{
  /* clang-format off */
  const key_sequence_case_t cases[] = {
      {"\x1b[122;6u",     TL_KEY_REDO},
      {"\x1b[90;6u",      TL_KEY_REDO},
      {"\x1b[90;5u",      TL_KEY_REDO},
      {"\x1b[27;6;90~",   TL_KEY_REDO},
      {"\x1b[27;6;122~",  TL_KEY_REDO},
      {"\x1b[122;5u",     TL_KEY_UNDO},
      {"\x1b[27;5;122~",  TL_KEY_UNDO},
      {"\x1b[122;70u",    TL_KEY_REDO},
      {"\x1b[122;7u",     TL_KEY_UNKN},
      {"\x1b[97;5u",      TL_KEY_UNKN},
      {"\x1b[122:90;6u",  TL_KEY_REDO},
      {"\x1b[A",          TL_KEY_UP},
      {"\x1bOD",          TL_KEY_LEFT},
      {"\x1b[1;5C",       TL_KEY_RIGHT | TL_MOD_CTRL},
      {"\x1b[1;3D",       TL_KEY_LEFT | TL_MOD_ALT},
      {"\x1b[1;2A",       TL_KEY_UP | TL_MOD_SHIFT},
      {"\x1b[1~",         TL_KEY_HOME},
      {"\x1b[4~",         TL_KEY_END},
      {"\x1b[7~",         TL_KEY_HOME},
      {"\x1b[8~",         TL_KEY_END},
      {"\x1b[3~",         TL_KEY_DELETE},
      {"\x1b[3;5~",       TL_KEY_DELETE | TL_MOD_CTRL},
      {"\x1b[Z",          TL_KEY_TAB | TL_MOD_SHIFT},
      {"\x1b[200~",       TL_KEY_PASTE_BEGIN},
      {"\x1b[201~",       TL_KEY_UNKN},
      {"\x1b[2~",         TL_KEY_UNKN},
      {"\x1b[2;5~",       TL_KEY_UNKN},
      {"\x1b[5~",         TL_KEY_UNKN},
      {"\x1b[11~",        TL_KEY_UNKN},
      {"\x1b[1;10A",      TL_KEY_UP},
      {"\x1b[<0;1;1M",    TL_KEY_UNKN},
  };
  /* clang-format on */
  size_t i;
  bool   ok = true;

  for (i = 0; i < countof(cases); ++i) {
    bool was_drained;
    int  key = test_parse_key_bytes(cases[i].bytes, &was_drained);

    if (key != cases[i].key || !was_drained) {
      TEST_PRINTF("case %zu (ESC%s): key %d, want %d, drained %d\n", i,
                  cases[i].bytes + 1, key, cases[i].key, (int) was_drained);
      ok = false;
    }
  }

  return ok;
}

#define TEST_KEY_BYTES_MAX 32

typedef struct test_key_reading test_key_reading_t;

/* What the read path makes of bytes the terminal sent: every byte it yields
   while input is pending, and separately the key the parser reads from them. */
struct test_key_reading
{
  uint8_t bytes[TEST_KEY_BYTES_MAX];
  size_t  size;
  int     key;
  bool    was_drained;
};

static bool
test_read_terminal_bytes(const char *bytes, size_t size, bool should_parse,
                         test_key_reading_t *reading)
{
  int     pipe_descriptors[2] = {-1, -1};
  int     saved_stdin = -1;
  bool    ok = false;
  uint8_t byte;

  reading->size = 0;
  reading->key = -1;
  reading->was_drained = false;
  if (pipe(pipe_descriptors) != 0) goto cleanup;
  if (write(pipe_descriptors[1], bytes, size) != (ssize_t) size) goto cleanup;
  saved_stdin = dup(STDIN_FILENO);
  if (saved_stdin < 0 || dup2(pipe_descriptors[0], STDIN_FILENO) < 0)
    goto cleanup;

  if (should_parse) {
    if (ITL_READ_BYTE(&byte)) {
      reading->key = itl_esc_parse(byte);
    }
  } else {
    while (itl_input_is_pending() && reading->size < TEST_KEY_BYTES_MAX &&
           ITL_READ_BYTE(&byte))
    {
      reading->bytes[reading->size++] = byte;
    }
  }
  reading->was_drained = !itl_input_is_pending();
  ok = true;

cleanup:
  if (saved_stdin >= 0) {
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdin);
  }
  if (pipe_descriptors[0] >= 0) close(pipe_descriptors[0]);
  if (pipe_descriptors[1] >= 0) close(pipe_descriptors[1]);
  itl_g_key_queue_index = 0;
  itl_g_key_queue_length = 0;
  itl_g_pushback_byte = -1;
  return ok;
}

typedef struct extended_key_case extended_key_case_t;

/* An extended encoding and the legacy bytes the same key sends, or NULL when
   the encoding has no legacy form and must reach the parser as it arrived. A
   legacy_size of zero means the legacy string's length. */
struct extended_key_case
{
  const char *extended;
  const char *legacy;
  size_t      legacy_size;
};

static bool
test_extended_key_case(const char *extended, const char *legacy,
                       size_t legacy_size)
{
  test_key_reading_t extended_bytes, extended_key, legacy_bytes, legacy_key;
  const char        *expected = legacy != NULL ? legacy : extended;
  size_t             expected_size = legacy == NULL  ? strlen(extended)
                                     : legacy_size > 0 ? legacy_size
                                                       : strlen(legacy);

  if (!test_read_terminal_bytes(extended, strlen(extended), false,
                                &extended_bytes) ||
      !test_read_terminal_bytes(extended, strlen(extended), true,
                                &extended_key) ||
      !test_read_terminal_bytes(expected, expected_size, false,
                                &legacy_bytes) ||
      !test_read_terminal_bytes(expected, expected_size, true, &legacy_key))
  {
    TEST_PRINTF("ESC%s: could not feed the bytes\n", extended + 1);
    return false;
  }

  if (extended_bytes.size != expected_size ||
      memcmp(extended_bytes.bytes, expected, expected_size) != 0 ||
      (legacy != NULL && (legacy_bytes.size != expected_size ||
                          memcmp(legacy_bytes.bytes, expected,
                                 expected_size) != 0)))
  {
    TEST_PRINTF("ESC%s: read %zu bytes, want %zu\n", extended + 1,
                extended_bytes.size, expected_size);
    return false;
  }
  if (extended_key.key != legacy_key.key ||
      extended_key.was_drained != legacy_key.was_drained)
  {
    TEST_PRINTF("ESC%s: key %d, legacy key %d, drained %d and %d\n",
                extended + 1, extended_key.key, legacy_key.key,
                (int) extended_key.was_drained, (int) legacy_key.was_drained);
    return false;
  }

  return true;
}

static bool
test_extended_keys_read_as_legacy_bytes(void)
{
  /* clang-format off */
  const extended_key_case_t cases[] = {
      {"\x1b[99;5u",        "\x03",       0},
      {"\x1b[100;5u",       "\x04",       0},
      {"\x1b[27;5;99~",     "\x03",       0},
      {"\x1b[27;5;100~",    "\x04",       0},
      {"\x1b[99;69u",       "\x03",       0},
      {"\x1b[27u",          "\x1b",       0},
      {"\x1b[27;3u",        "\x1b\x1b",   0},
      {"\x1b[27;5;27~",     "\x1b",       0},
      {"\x1b[13;2u",        "\r",         0},
      {"\x1b[13;5u",        "\r",         0},
      {"\x1b[13;3u",        "\x1b\r",     0},
      {"\x1b[27;5;13~",     "\r",         0},
      {"\x1b[9;5u",         "\t",         0},
      {"\x1b[9;2u",         "\x1b[Z",     0},
      {"\x1b[9;3u",         "\x1b\t",     0},
      {"\x1b[27;5;9~",      "\t",         0},
      {"\x1b[127u",         "\x7f",       0},
      {"\x1b[127;3u",       "\x1b\x7f",   0},
      {"\x1b[127;5u",       "\x08",       0},
      {"\x1b[127;7u",       "\x1b\x08",   0},
      {"\x1b[27;5;8~",      "\x08",       0},
      {"\x1b[32;5u",        "\0",         1},
      {"\x1b[50;6u",        "\0",         1},
      {"\x1b[27;6;64~",     "\0",         1},
      {"\x1b[45;6u",        "\x1f",       0},
      {"\x1b[45:95;6u",     "\x1f",       0},
      {"\x1b[47;5u",        "\x1f",       0},
      {"\x1b[27;6;95~",     "\x1f",       0},
      {"\x1b[95;6u",        "\x1f",       0},
      {"\x1b[54;6u",        "\x1e",       0},
      {"\x1b[27;6;94~",     "\x1e",       0},
      {"\x1b[91;5u",        "\x1b",       0},
      {"\x1b[120;5u\x1b[101;5u", "\x18\x05", 0},
      {"\x1b[120;5u\x1b[117;5u", "\x18\x15", 0},
      {"\x1b[46;3u",        "\x1b.",      0},
      {"\x1b[45;4u",        "\x1b_",      0},
      {"\x1b[44;4u",        "\x1b<",      0},
      {"\x1b[46;4u",        "\x1b>",      0},
      {"\x1b[46:62;4u",     "\x1b>",      0},
      {"\x1b[27;3;98~",     "\x1b" "b",   0},
      {"\x1b[98;33u",       "\x1b" "b",   0},
      {"\x1b[98;7u",        "\x1b\x02",   0},
      {"\x1b[1076;3u",      "\x1b\xd0\xb4", 0},
      {"\x1b[97;5uX",       "\x01X",      0},
      {"\x1b[57414u",       "\r",         0},
      {"\x1b[57399u",       "0",          0},
      {"\x1b[57413;2u",     "+",          0},
      {"\x1b[57417;5u",     "\x1b[1;5D",  0},
      {"\x1b[57421u",       "\x1b[5~",    0},
      {"\x1b[57426;3u",     "\x1b[3;3~",  0},
      {"\x1b[122;6u",       NULL,         0},
      {"\x1b[90;5u",        NULL,         0},
      {"\x1b[27;6;90~",     NULL,         0},
      {"\x1b[91;3u",        NULL,         0},
      {"\x1b[79;4u",        NULL,         0},
      {"\x1b[97;9u",        NULL,         0},
      {"\x1b[97;5:3u",      NULL,         0},
      {"\x1b[49;5u",        NULL,         0},
      {"\x1b[57376u",       NULL,         0},
      {"\x1b[A",            NULL,         0},
      {"\x1b[1;5C",         NULL,         0},
      {"\x1b[1;3D",         NULL,         0},
      {"\x1b[1;2A",         NULL,         0},
      {"\x1b[1;6B",         NULL,         0},
      {"\x1bOD",            NULL,         0},
      {"\x1b[H",            NULL,         0},
      {"\x1b[1;5F",         NULL,         0},
      {"\x1b[1~",           NULL,         0},
      {"\x1b[4~",           NULL,         0},
      {"\x1b[5~",           NULL,         0},
      {"\x1b[6;5~",         NULL,         0},
      {"\x1b[2~",           NULL,         0},
      {"\x1b[3;5~",         NULL,         0},
      {"\x1b[Z",            NULL,         0},
      {"\x1bOP",            NULL,         0},
      {"\x1b[P",            NULL,         0},
      {"\x1b[1;5P",         NULL,         0},
      {"\x1b[13~",          NULL,         0},
      {"\x1b[15;2~",        NULL,         0},
      {"\x1b[24~",          NULL,         0},
      {"\x1b[200~",         NULL,         0},
      {"\x1b[<0;1;1M",      NULL,         0},
      {"\x1b" "b",          NULL,         0},
      {"\x1b",              NULL,         0},
  };
  /* clang-format on */
  char   extended[TEST_KEY_BYTES_MAX];
  char   legacy[TEST_KEY_BYTES_MAX];
  size_t i;
  bool   ok = true;
  int    letter;

  for (i = 0; i < countof(cases); ++i) {
    ok &= test_extended_key_case(cases[i].extended, cases[i].legacy,
                                 cases[i].legacy_size);
  }

  /* Every Ctrl and Alt letter in both encodings, apart from Ctrl-X, which
     waits for its chord and is covered above. */
  for (letter = 'a'; letter <= 'z'; ++letter) {
    if (letter != 'x') {
      snprintf(legacy, sizeof(legacy), "%c", letter & 0x1F);
      snprintf(extended, sizeof(extended), "\x1b[%d;5u", letter);
      ok &= test_extended_key_case(extended, legacy, 0);
      snprintf(extended, sizeof(extended), "\x1b[27;5;%d~", letter);
      ok &= test_extended_key_case(extended, legacy, 0);
      snprintf(extended, sizeof(extended), "\x1b[%d;6u", letter);
      ok &= test_extended_key_case(extended, letter == 'z' ? NULL : legacy, 0);
    }

    snprintf(legacy, sizeof(legacy), "\x1b%c", letter);
    snprintf(extended, sizeof(extended), "\x1b[%d;3u", letter);
    ok &= test_extended_key_case(extended, legacy, 0);
    snprintf(extended, sizeof(extended), "\x1b[27;3;%d~", letter);
    ok &= test_extended_key_case(extended, legacy, 0);
    snprintf(legacy, sizeof(legacy), "\x1b%c", letter - 'a' + 'A');
    snprintf(extended, sizeof(extended), "\x1b[%d;4u", letter);
    ok &= test_extended_key_case(extended, letter == 'o' ? NULL : legacy, 0);
  }

  return ok;
}

static bool
test_extended_keys_map_signal_keys(void)
{
  test_key_reading_t reading;
  bool               ok = true;

  ok &= test_read_terminal_bytes("\x1b[99;5u", 8, true, &reading) &&
        reading.key == TL_KEY_INTERRUPT;
  ok &= test_read_terminal_bytes("\x1b[100;5u", 9, true, &reading) &&
        reading.key == TL_KEY_EOF;
  ok &= test_read_terminal_bytes("\x1b[27;5;99~", 11, true, &reading) &&
        reading.key == TL_KEY_INTERRUPT;
  ok &= test_read_terminal_bytes("\x1b[122;5u", 9, true, &reading) &&
        reading.key == TL_KEY_UNDO;
  ok &= test_read_terminal_bytes("\x1b[122;6u", 9, true, &reading) &&
        reading.key == TL_KEY_REDO;
  ok &= test_read_terminal_bytes("\x1b[45;6u", 8, true, &reading) &&
        reading.key == TL_KEY_UNDO;
  ok &= test_read_terminal_bytes("\x1b[54;6u", 8, true, &reading) &&
        reading.key == TL_KEY_REDO;
  ok &= test_read_terminal_bytes("\x1b[32;5u", 8, true, &reading) &&
        reading.key == TL_KEY_TAB;
  ok &= test_read_terminal_bytes("\x1b[13;3u", 8, true, &reading) &&
        reading.key == (TL_KEY_ENTER | TL_MOD_ALT);
  ok &= test_read_terminal_bytes("\x1b[27u", 5, true, &reading) &&
        reading.key == TL_KEY_UNKN && reading.was_drained;

  /* A paste body is text, so a key form inside it stays as it was sent. */
  itl_g_is_reading_paste = true;
  ok &= test_read_terminal_bytes("\x1b[99;5u", 8, false, &reading) &&
        reading.size == 8 && memcmp(reading.bytes, "\x1b[99;5u", 8) == 0;
  itl_g_is_reading_paste = false;

  if (!ok) {
    TEST_PRINTF("a signal key read as another key\n");
  }
  return ok;
}

/* Declared here because the strict C99 build hides the XSI terminal calls. */
extern int   posix_openpt(int flags);
extern int   grantpt(int descriptor);
extern int   unlockpt(int descriptor);
extern char *ptsname(int descriptor);

static bool
test_extended_keys_follow_raw_mode(void)
{
  static const char expected[] = "\x1b[?2004h" ITL_EXTENDED_KEYS_ON
      ITL_EXTENDED_KEYS_OFF ITL_EXTENDED_KEYS_ON ITL_EXTENDED_KEYS_OFF
          ITL_EXTENDED_KEYS_ON "\x1b[?2004l" ITL_EXTENDED_KEYS_OFF;
  struct termios saved_mode = itl_g_original_tty_mode;
  bool           was_raw = itl_g_entered_raw_mode;
  int            master = -1, slave = -1;
  int            saved_stdin = -1, saved_stdout = -1;
  char           written[256];
  size_t         written_size = 0;
  bool           ok = false;

  master = posix_openpt(O_RDWR | O_NOCTTY);
  if (master < 0 || grantpt(master) != 0 || unlockpt(master) != 0 ||
      ptsname(master) == NULL)
  {
    TEST_PRINTF("could not open a terminal pair\n");
    goto cleanup;
  }
  slave = open(ptsname(master), O_RDWR | O_NOCTTY);
  saved_stdin = dup(STDIN_FILENO);
  saved_stdout = dup(STDOUT_FILENO);
  if (slave < 0 || saved_stdin < 0 || saved_stdout < 0 ||
      dup2(slave, STDIN_FILENO) < 0 || dup2(slave, STDOUT_FILENO) < 0)
  {
    goto cleanup;
  }

  itl_g_entered_raw_mode = false;
  tl_set_extended_keys(1);
  ok = tl_enter_raw_mode() == TL_SUCCESS;
  ok &= tl_set_signal_keys(1) == TL_SUCCESS;
  ok &= tl_set_signal_keys(0) == TL_SUCCESS;
  tl_set_extended_keys(0);
  tl_set_extended_keys(1);
  ok &= tl_exit_raw_mode() == TL_SUCCESS;
  tl_set_extended_keys(0);
  tl_set_extended_keys(1);
  ok &= tl_set_signal_keys(0) == TL_ERROR;
  tl_set_extended_keys(0);

cleanup:
  if (saved_stdin >= 0) {
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdin);
  }
  if (saved_stdout >= 0) {
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
  }
  if (slave >= 0) close(slave);
  if (master >= 0) {
    struct pollfd pfd;
    ssize_t       chunk;

    pfd.fd = master;
    pfd.events = POLLIN;
    while (written_size < sizeof(written) && poll(&pfd, 1, 100) > 0 &&
           (pfd.revents & POLLIN) != 0 &&
           (chunk = read(master, written + written_size,
                         sizeof(written) - written_size)) > 0)
    {
      written_size += (size_t) chunk;
    }
    close(master);
  }
  itl_g_entered_raw_mode = was_raw;
  itl_g_original_tty_mode = saved_mode;
  itl_g_extended_keys_active = false;

  if (!ok || written_size != sizeof(expected) - 1 ||
      memcmp(written, expected, written_size) != 0)
  {
    TEST_PRINTF("raw mode wrote %zu bytes, want %zu\n", written_size,
                sizeof(expected) - 1);
    return false;
  }
  return true;
}

/* Raw mode entered again while the host has the signal keys on keeps the
   interrupt key a signal and asks for no extended keys. The exit restore puts
   the terminal back and withdraws every request without touching the editor
   state. */
static bool
test_raw_mode_keeps_signal_keys(void)
{
  static const char expected[] = "\x1b[?2004h" ITL_EXTENDED_KEYS_ON
      ITL_EXTENDED_KEYS_OFF "\x1b[?2004l\x1b[?2004h" ITL_EXTENDED_KEYS_ON
          "\x1b[?2004l" ITL_EXTENDED_KEYS_OFF;
  struct termios saved_mode = itl_g_original_tty_mode;
  struct termios term;
  bool           was_raw = itl_g_entered_raw_mode;
  int            master = -1, slave = -1;
  int            saved_stdin = -1, saved_stdout = -1;
  char           written[256];
  size_t         written_size = 0;
  bool           ok = false;

  master = posix_openpt(O_RDWR | O_NOCTTY);
  if (master < 0 || grantpt(master) != 0 || unlockpt(master) != 0 ||
      ptsname(master) == NULL)
  {
    TEST_PRINTF("could not open a terminal pair\n");
    goto cleanup;
  }
  slave = open(ptsname(master), O_RDWR | O_NOCTTY);
  saved_stdin = dup(STDIN_FILENO);
  saved_stdout = dup(STDOUT_FILENO);
  if (slave < 0 || saved_stdin < 0 || saved_stdout < 0 ||
      dup2(slave, STDIN_FILENO) < 0 || dup2(slave, STDOUT_FILENO) < 0)
  {
    goto cleanup;
  }

  itl_g_entered_raw_mode = false;
  tl_set_extended_keys(1);
  ok = tl_enter_raw_mode() == TL_SUCCESS;
  ok &= tl_set_signal_keys(1) == TL_SUCCESS;
  ok &= tl_exit_raw_mode() == TL_SUCCESS;
  ok &= tl_enter_raw_mode() == TL_SUCCESS;
  ok &= tcgetattr(STDIN_FILENO, &term) == 0 &&
        (term.c_lflag & (tcflag_t) ISIG) != 0 &&
        (term.c_lflag & (tcflag_t) ICANON) == 0;
  ok &= tl_set_signal_keys(0) == TL_SUCCESS;
  tl_restore_terminal_for_exit();
  ok &= itl_g_entered_raw_mode;
  ok &= tcgetattr(STDIN_FILENO, &term) == 0 &&
        (term.c_lflag & (tcflag_t) ICANON) != 0;

cleanup:
  if (saved_stdin >= 0) {
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdin);
  }
  if (saved_stdout >= 0) {
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
  }
  if (slave >= 0) close(slave);
  if (master >= 0) {
    struct pollfd pfd;
    ssize_t       chunk;

    pfd.fd = master;
    pfd.events = POLLIN;
    while (written_size < sizeof(written) && poll(&pfd, 1, 100) > 0 &&
           (pfd.revents & POLLIN) != 0 &&
           (chunk = read(master, written + written_size,
                         sizeof(written) - written_size)) > 0)
    {
      written_size += (size_t) chunk;
    }
    close(master);
  }
  itl_g_entered_raw_mode = was_raw;
  itl_g_original_tty_mode = saved_mode;
  itl_g_extended_keys_active = false;
  itl_g_extended_keys_enabled = false;
  itl_g_signal_keys_enabled = false;

  if (!ok || written_size != sizeof(expected) - 1 ||
      memcmp(written, expected, written_size) != 0)
  {
    TEST_PRINTF("raw mode wrote %zu bytes, want %zu\n", written_size,
                sizeof(expected) - 1);
    return false;
  }
  return true;
}
#endif

static bool
test_prefix_history_search_walks_matches(void)
{
  const char *path = "tl_test_prefix_search.txt";
  char out_buffer[BUFFER_SIZE];
  char line_buffer[BUFFER_SIZE];
  bool ok = true;
  itl_le_t le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  itl_g_is_active = true;
  remove(path);
  tl_history_load(path);
  if (!hist_append_cstr("git status")) ok = false;
  if (!hist_append_cstr("ls -l")) ok = false;
  if (!hist_append_cstr("git commit")) ok = false;
  if (!hist_append_cstr("echo hi")) ok = false;
  tl_set_history_prefix_search(1);

  ITL_STRING_FROM_CSTR(line, "git");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  le.cursor_position = line->length;

  itl_le_key_handle(&le, TL_KEY_UP);
  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));
  if (strcmp(line_buffer, "git commit") != 0) {
    TEST_PRINTF("first up gave '%s'\n", line_buffer);
    ok = false;
  }
  itl_le_key_handle(&le, TL_KEY_UP);
  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));
  if (strcmp(line_buffer, "git status") != 0) {
    TEST_PRINTF("second up gave '%s'\n", line_buffer);
    ok = false;
  }
  itl_le_key_handle(&le, TL_KEY_UP);
  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));
  if (strcmp(line_buffer, "git status") != 0) {
    TEST_PRINTF("up past the oldest match gave '%s'\n", line_buffer);
    ok = false;
  }
  itl_le_key_handle(&le, TL_KEY_DOWN);
  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));
  if (strcmp(line_buffer, "git commit") != 0) {
    TEST_PRINTF("down gave '%s'\n", line_buffer);
    ok = false;
  }
  itl_le_key_handle(&le, TL_KEY_DOWN);
  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));
  if (strcmp(line_buffer, "git") != 0 ||
      le.history_selected_index != ITL_HISTORY_NONE)
  {
    TEST_PRINTF("down past the newest match gave '%s'\n", line_buffer);
    ok = false;
  }

  ITL_STRING_FROM_CSTR(line, "");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  itl_le_key_handle(&le, TL_KEY_UP);
  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));
  if (strcmp(line_buffer, "echo hi") != 0) {
    TEST_PRINTF("empty line up gave '%s'\n", line_buffer);
    ok = false;
  }

  tl_set_history_prefix_search(0);
  ITL_STRING_FROM_CSTR(line, "git");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  le.cursor_position = line->length;
  itl_le_key_handle(&le, TL_KEY_UP);
  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));
  if (strcmp(line_buffer, "echo hi") != 0) {
    TEST_PRINTF("disabled up gave '%s'\n", line_buffer);
    ok = false;
  }

  ITL_STRING_FREE(line);
  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}

typedef struct menu_rank_test_case menu_rank_test_case_t;

struct menu_rank_test_case
{
  const char *entry;
  const char *query;
  unsigned    rank;
};

static bool
test_menu_match_rank(void)
{
  static const menu_rank_test_case_t cases[] = {
      {"alpha", "", ITL_MENU_RANK_PREFIX},
      {"al", "alpha", ITL_MENU_RANK_NONE},
      {"ALPHA", "al", ITL_MENU_RANK_PREFIX},
      {"aab", "ab", ITL_MENU_RANK_CONTAINS},
      {"unALPHAed", "alpha", ITL_MENU_RANK_CONTAINS},
      {"abcdefg", "adg", ITL_MENU_RANK_SUBSEQUENCE},
      {"abc", "xyz", ITL_MENU_RANK_NONE},
      {"\xC3\x84nder", "\xC3\xA4n", ITL_MENU_RANK_NONE},
      {"\xC3\xA4nder", "\xC3\xA4n", ITL_MENU_RANK_PREFIX},
  };
  size_t i;

  for (i = 0; i < countof(cases); ++i) {
    unsigned rank = itl_menu_match_rank(cases[i].entry, strlen(cases[i].entry),
                                        cases[i].query, strlen(cases[i].query));

    if (rank != cases[i].rank) {
      TEST_PRINTF("'%s' against '%s' ranked %u, expected %u\n", cases[i].entry,
                  cases[i].query, rank, cases[i].rank);
      return false;
    }
  }

  return true;
}

static bool
test_menu_filter_groups(void)
{
  static const char *candidates[] = {
      "alpha", "beta",  "malt",
      "abcl",  "ALIEN", "\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E"};
  static const char *descriptions[] = {"one",  "two",  "three",
                                       "four", "five", "six"};
  static const char *expected_names[] = {"alpha", "ALIEN", "malt", "abcl"};
  static const char *expected_descriptions[] = {"one", "five", "three", "four"};
  size_t        i;
  tl_completion base = ITL_ZERO_INIT;
  tl_completion result = ITL_ZERO_INIT;

  base.candidates = candidates;
  base.descriptions = descriptions;
  base.count = countof(candidates);

  if (!itl_menu_filter(&base, "al", 2, &result)) {
    TEST_PRINTF("a matching query kept nothing\n");
    return false;
  }

  if (result.count != countof(expected_names) ||
      result.longest_common_prefix != NULL)
  {
    TEST_PRINTF("filtering kept %zu rows\n", result.count);
    return false;
  }

  for (i = 0; i < result.count; ++i) {
    if (strcmp(result.candidates[i], expected_names[i]) != 0 ||
        strcmp(result.descriptions[i], expected_descriptions[i]) != 0)
    {
      TEST_PRINTF("row %zu is '%s' with '%s'\n", i, result.candidates[i],
                  result.descriptions[i]);
      return false;
    }
  }

  if (itl_menu_name_width(&result) != 5) {
    TEST_PRINTF("narrowed name width is %zu\n", itl_menu_name_width(&result));
    return false;
  }

  if (itl_menu_filter(&base, "zzq", 3, &result)) {
    TEST_PRINTF("a query matching nothing kept %zu rows\n", result.count);
    return false;
  }

  if (!itl_menu_filter(&base, "", 0, &result) || result.count != base.count) {
    TEST_PRINTF("an empty query kept %zu rows\n", result.count);
    return false;
  }

  for (i = 0; i < result.count; ++i) {
    if (strcmp(result.candidates[i], candidates[i]) != 0) {
      TEST_PRINTF("an empty query moved row %zu to '%s'\n", i,
                  result.candidates[i]);
      return false;
    }
  }

  if (itl_menu_name_width(&result) != 6) {
    TEST_PRINTF("wide name width is %zu\n", itl_menu_name_width(&result));
    return false;
  }

  base.descriptions = NULL;

  if (!itl_menu_filter(&base, "al", 2, &result) || result.descriptions != NULL)
  {
    TEST_PRINTF("a base without descriptions produced some\n");
    return false;
  }

  return true;
}

static size_t test_menu_gather_calls;
static bool   test_menu_gather_has_rows = true;

static bool
test_menu_gather(itl_le_t *le, tl_completion *result)
{
  static const char *candidates[] = {"alpha", "album", "beta"};

  test_menu_gather_calls += 1;

  if (!test_menu_gather_has_rows) {
    return false;
  }

  result->candidates = candidates;
  result->descriptions = NULL;
  result->longest_common_prefix = NULL;
  result->count = countof(candidates);
  result->token_start = 0;
  result->token_end = le->line->length;

  return true;
}

static bool
test_menu_narrow_reuses_base(void)
{
  char                   out_buffer[BUFFER_SIZE];
  itl_le_t               le = ITL_ZERO_INIT;
  itl_menu_source        source = ITL_ZERO_INIT;
  itl_menu_filter_state  state = ITL_ZERO_INIT;
  tl_completion          result = ITL_ZERO_INIT;
  itl_string_t          *line = itl_string_alloc();

  source.gather = test_menu_gather;
  test_menu_gather_calls = 0;
  test_menu_gather_has_rows = true;

  ITL_STRING_FROM_CSTR(line, "al");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");

  if (!itl_menu_rebase(&le, &source, &state, &result) || result.count != 3 ||
      state.query_len != 2 || state.name_width != 5)
  {
    TEST_PRINTF("the first gather kept %zu rows at width %zu\n", result.count,
                state.name_width);
    ITL_STRING_FREE(line);
    return false;
  }

  itl_le_insert(&le, itl_utf8_parse('b'));
  result.token_end += 1;

  if (!itl_menu_narrow(&le, &source, &state, &result) || result.count != 1 ||
      strcmp(result.candidates[0], "album") != 0 ||
      test_menu_gather_calls != 1)
  {
    TEST_PRINTF("a grown query kept %zu rows after %zu gathers\n", result.count,
                test_menu_gather_calls);
    ITL_STRING_FREE(line);
    return false;
  }

  ITL_LE_ERASE_BACKWARD(&le, 1);
  itl_le_insert(&le, itl_utf8_parse('z'));

  if (!itl_menu_narrow(&le, &source, &state, &result) || result.count != 3 ||
      test_menu_gather_calls != 2 || state.query_len != 3)
  {
    TEST_PRINTF("a diverged query kept %zu rows after %zu gathers\n",
                result.count, test_menu_gather_calls);
    ITL_STRING_FREE(line);
    return false;
  }

  ITL_LE_ERASE_BACKWARD(&le, 1);
  result.token_end -= 1;

  if (!itl_menu_narrow(&le, &source, &state, &result) || result.count != 3 ||
      test_menu_gather_calls != 3 || state.query_len != 2)
  {
    TEST_PRINTF("a widened query kept %zu rows after %zu gathers\n",
                result.count, test_menu_gather_calls);
    ITL_STRING_FREE(line);
    return false;
  }

  test_menu_gather_has_rows = false;

  if (itl_menu_rebase(&le, &source, &state, &result) || state.base.count != 0 ||
      state.name_width != 0 || state.query_len != 2)
  {
    TEST_PRINTF("an empty source left %zu rows at width %zu for %zu bytes\n",
                state.base.count, state.name_width, state.query_len);
    ITL_STRING_FREE(line);
    return false;
  }

  itl_le_insert(&le, itl_utf8_parse('z'));
  result.token_end += 1;

  if (itl_menu_narrow(&le, &source, &state, &result) ||
      test_menu_gather_calls != 4 || state.query_len != 2)
  {
    TEST_PRINTF("an extended empty query ran %zu gathers for %zu bytes\n",
                test_menu_gather_calls, state.query_len);
    ITL_STRING_FREE(line);
    return false;
  }

  ITL_STRING_FREE(line);

  return true;
}

#if defined ITL_POSIX
static size_t test_word_gather_calls;
static bool   test_word_gather_is_unranked;

/* A source whose token is the last word of the line. It offers the words the
   token starts, each with a description, as a list ranked by tiers. The
   unranked variant offers the words the token starts in either case, the way
   a completion function may, and does not rank them. */
static bool
test_word_gather(itl_le_t *le, tl_completion *result)
{
  static const char *ranked_words[] = {"alpha", "album", "alcove", "beta"};
  static const char *unranked_words[] = {"alpha", "Album", "alcove", "beta"};
  static const char *notes[] = {"first", "second", "third", "fourth"};
  const char *const *words =
      test_word_gather_is_unranked ? unranked_words : ranked_words;
  static const char *candidates[countof(ranked_words)];
  static const char *descriptions[countof(ranked_words)];
  char               token[BUFFER_SIZE];
  size_t             token_len = 0;
  size_t             kept_count = 0;
  size_t             start = le->line->length;
  size_t             i;

  test_word_gather_calls += 1;

  while (start > 0 && !ITL_CHAR_IS_SPACE(le->line->chars[start - 1].bytes[0]))
  {
    start -= 1;
  }
  for (i = start; i < le->line->length && token_len + 1 < sizeof(token); ++i) {
    token[token_len++] = (char) le->line->chars[i].bytes[0];
  }
  token[token_len] = '\0';

  for (i = 0; i < countof(ranked_words); ++i) {
    bool does_match = test_word_gather_is_unranked
                          ? strncasecmp(words[i], token, token_len) == 0
                          : strncmp(words[i], token, token_len) == 0;

    if (does_match) {
      candidates[kept_count] = words[i];
      descriptions[kept_count] = notes[i];
      kept_count += 1;
    }
  }

  if (kept_count == 0) {
    return false;
  }

  result->candidates = candidates;
  result->descriptions = descriptions;
  result->longest_common_prefix = NULL;
  result->count = kept_count;
  result->token_start = start;
  result->token_end = le->line->length;
  result->is_tier_ranked = !test_word_gather_is_unranked;

  return true;
}

/* Open the menu on text with one gather, feed it keys until the input ends,
   and return how many more gathers the keys cost. The line the keys left is
   written to out_line. */
static size_t
menu_keys_gathers(const char *text, const char *keys, char *out_line,
                  size_t out_size)
{
  static const itl_menu_source source = {
      test_word_gather, true, true, false, true, false, false, NULL, NULL,
      false, true};
  char          out_buffer[BUFFER_SIZE];
  int           pipe_descriptors[2] = {-1, -1};
  int           null_descriptor = -1;
  int           saved_stdin = -1;
  int           saved_stdout = -1;
  size_t        calls = (size_t) -1;
  size_t        key_count = strlen(keys);
  tl_completion initial = ITL_ZERO_INIT;
  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  out_line[0] = '\0';
  if (pipe(pipe_descriptors) != 0) goto cleanup;
  if (write(pipe_descriptors[1], keys, key_count) != (ssize_t) key_count) {
    goto cleanup;
  }
  close(pipe_descriptors[1]);
  pipe_descriptors[1] = -1;

  null_descriptor = open("/dev/null", O_WRONLY);
  if (null_descriptor < 0) goto cleanup;

  saved_stdin = dup(STDIN_FILENO);
  saved_stdout = dup(STDOUT_FILENO);
  if (saved_stdin < 0 || saved_stdout < 0) goto cleanup;
  if (dup2(pipe_descriptors[0], STDIN_FILENO) < 0 ||
      dup2(null_descriptor, STDOUT_FILENO) < 0)
  {
    goto cleanup;
  }

  ITL_STRING_FROM_CSTR(line, text);
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  itl_g_tty_changed_size = 0;
  itl_g_tty_prev_rows = 24;
  itl_g_tty_prev_cols = 80;

  test_word_gather_calls = 0;
  (void) test_word_gather(&le, &initial);
  test_word_gather_calls = 0;
  (void) itl_completion_menu_run(&le, &initial, &source);
  calls = test_word_gather_calls;
  itl_string_to_cstr(line, out_line, out_size);

cleanup:
  if (saved_stdin >= 0) {
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdin);
  }
  if (saved_stdout >= 0) {
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
  }
  if (null_descriptor >= 0) close(null_descriptor);
  if (pipe_descriptors[0] >= 0) close(pipe_descriptors[0]);
  if (pipe_descriptors[1] >= 0) close(pipe_descriptors[1]);

  ITL_STRING_FREE(line);
  itl_ghost_clear();
  itl_g_tty_changed_size = 1;
  itl_g_tty_first_render = true;

  return calls;
}

typedef struct menu_keys_case menu_keys_case_t;

struct menu_keys_case
{
  const char *text;
  const char *keys;
  const char *line;
  size_t      gathers;
};

/* Typing into an open menu narrows the gathered list without asking the source
   again, and an erase widens it while the gathered query stands. The source is
   asked again for a new word, a new path component, a byte that moves the
   token, and an erase below the gathered query. */
static bool
test_menu_keys_reuse_gathered_list(void)
{
  static const menu_keys_case_t cases[] = {
      {"cat al",  "bu",               "cat albu",    0},
      {"cat al",  "c",                "cat alc",     0},
      {"cat al",  "bu\x7f\x7f",       "cat al",      0},
      {"cat al",  "bu\x7f\x7f\x7f",   "cat a",       1},
      {"cat al",  " ",                "cat al ",     1},
      {"cat al",  " b",               "cat al b",    2},
      {"cat al",  "/",                "cat al/",     1},
      {"cat al",  "/a",               "cat al/a",    2},
      {"cat al",  "'",                "cat al'",     1},
      {"cat al",  "=",                "cat al=",     1},
      {"cat ",    "al",               "cat al",      1},
      {"cat al",  "zz",               "cat alzz",    1},
      {"cat al",  "ee",               "cat alee",    1},
      {"cat al",  "P",                "cat alP",     1},
  };
  bool   ok = true;
  char   line[BUFFER_SIZE];
  size_t i;

  if (itl_menu_tier_rank("alpha", 5, "al", 2, false) !=
          ITL_MENU_TIER_EXACT_PREFIX ||
      itl_menu_tier_rank("Alpha", 5, "al", 2, false) != ITL_MENU_TIER_PREFIX ||
      itl_menu_tier_rank("Alpha", 5, "aL", 2, true) != ITL_MENU_TIER_NONE ||
      itl_menu_tier_rank("foo_bar_baz", 11, "fbb", 3, false) !=
          ITL_MENU_TIER_SUBSEQUENCE ||
      itl_menu_tier_rank("fOo_Bar", 7, "fB", 2, true) !=
          ITL_MENU_TIER_SUBSEQUENCE ||
      itl_menu_tier_rank("cab", 3, "b", 1, false) != ITL_MENU_TIER_NONE ||
      itl_menu_tier_rank("a-b-c", 5, "-c", 2, false) != ITL_MENU_TIER_NONE)
  {
    TEST_PRINTF("a completion tier was ranked wrong\n");
    ok = false;
  }

  test_word_gather_is_unranked = true;
  if (menu_keys_gathers("cat al", "b", line, sizeof(line)) != 0 ||
      strcmp(line, "cat alb") != 0 ||
      strcmp(itl_g_menu_filtered[0], "Album") != 0)
  {
    TEST_PRINTF("an unranked list lost a row a fresh gather keeps\n");
    ok = false;
  }
  test_word_gather_is_unranked = false;

  for (i = 0; i < countof(cases); ++i) {
    size_t gathers =
        menu_keys_gathers(cases[i].text, cases[i].keys, line, sizeof(line));

    if (gathers != cases[i].gathers || strcmp(line, cases[i].line) != 0) {
      TEST_PRINTF("case %zu left '%s' after %zu gathers\n", i, line, gathers);
      ok = false;
    }
  }

  if (menu_keys_gathers("cat al", "bu", line, sizeof(line)) != 0 ||
      strcmp(itl_g_menu_filtered[0], "album") != 0 ||
      strcmp(itl_g_menu_filtered_descriptions[0], "second") != 0)
  {
    TEST_PRINTF("a narrowed row lost its description\n");
    ok = false;
  }

  return ok;
}

static const char *const *test_tab_words;
static size_t             test_tab_word_count;
static bool               test_tab_should_fold_prefix;
static size_t             test_tab_callback_calls;

/* A host whose token is the last word of the line. It offers the words that
   open with the token, ignoring case when asked to, and reports their common
   prefix the same way. */
static int
test_tab_counting_callback(const char *buffer, size_t cursor,
                           tl_completion *completion, int for_listing)
{
  static const char *candidates[8];
  static char        prefix[BUFFER_SIZE];
  size_t             start = cursor;
  size_t             token_len;
  size_t             prefix_len = 0;
  size_t             kept_count = 0;
  size_t             i;

  (void) for_listing;
  test_tab_callback_calls += 1;

  while (start > 0 && buffer[start - 1] != ' ') {
    start -= 1;
  }
  token_len = cursor - start;

  for (i = 0; i < test_tab_word_count && kept_count < countof(candidates); ++i)
  {
    const char *word = test_tab_words[i];
    bool        does_match =
        test_tab_should_fold_prefix
                   ? strncasecmp(word, buffer + start, token_len) == 0
                   : strncmp(word, buffer + start, token_len) == 0;

    if (!does_match) {
      continue;
    }
    if (kept_count == 0) {
      prefix_len = strlen(word);
      memcpy(prefix, word, prefix_len);
    }
    while (prefix_len > 0 &&
           (test_tab_should_fold_prefix
                ? strncasecmp(prefix, word, prefix_len) != 0
                : strncmp(prefix, word, prefix_len) != 0))
    {
      prefix_len -= 1;
    }
    candidates[kept_count] = word;
    kept_count += 1;
  }
  prefix[prefix_len] = '\0';

  completion->candidates = candidates;
  completion->descriptions = NULL;
  completion->longest_common_prefix = prefix;
  completion->count = kept_count;
  completion->token_start = start;
  completion->token_end = cursor;

  return 1;
}

/* Press TAB once on text with the menu enabled, feed the menu keys until the
   input ends, and return how many times the host was asked. The line the keys
   left is written to out_line. */
static size_t
tab_completion_calls(const char *text, const char *keys, char *out_line,
                     size_t out_size)
{
  char           out_buffer[BUFFER_SIZE];
  int            pipe_descriptors[2] = {-1, -1};
  int            null_descriptor = -1;
  int            saved_stdin = -1;
  int            saved_stdout = -1;
  int            was_menu_enabled = itl_g_completion_menu_enabled;
  size_t         calls = (size_t) -1;
  size_t         key_count = strlen(keys);
  tl_status_code completion_code = TL_SUCCESS;
  itl_le_t       le = ITL_ZERO_INIT;
  itl_string_t  *line = itl_string_alloc();

  out_line[0] = '\0';
  if (pipe(pipe_descriptors) != 0) goto cleanup;
  if (write(pipe_descriptors[1], keys, key_count) != (ssize_t) key_count) {
    goto cleanup;
  }
  close(pipe_descriptors[1]);
  pipe_descriptors[1] = -1;

  null_descriptor = open("/dev/null", O_WRONLY);
  if (null_descriptor < 0) goto cleanup;

  saved_stdin = dup(STDIN_FILENO);
  saved_stdout = dup(STDOUT_FILENO);
  if (saved_stdin < 0 || saved_stdout < 0) goto cleanup;
  if (dup2(pipe_descriptors[0], STDIN_FILENO) < 0 ||
      dup2(null_descriptor, STDOUT_FILENO) < 0)
  {
    goto cleanup;
  }

  ITL_STRING_FROM_CSTR(line, text);
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  itl_g_tty_changed_size = 0;
  itl_g_tty_prev_rows = 24;
  itl_g_tty_prev_cols = 80;
  itl_g_completion_menu_enabled = 1;

  test_tab_callback_calls = 0;
  tl_set_complete_callback(test_tab_counting_callback);
  (void) itl_completion_handle_tab(&le, &completion_code);
  tl_set_complete_callback(NULL);
  calls = test_tab_callback_calls;
  itl_string_to_cstr(line, out_line, out_size);

cleanup:
  itl_g_completion_menu_enabled = was_menu_enabled;
  if (saved_stdin >= 0) {
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdin);
  }
  if (saved_stdout >= 0) {
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
  }
  if (null_descriptor >= 0) close(null_descriptor);
  if (pipe_descriptors[0] >= 0) close(pipe_descriptors[0]);
  if (pipe_descriptors[1] >= 0) close(pipe_descriptors[1]);

  ITL_STRING_FREE(line);
  itl_ghost_clear();
  itl_g_tty_changed_size = 1;
  itl_g_tty_first_render = true;

  return calls;
}

typedef struct tab_calls_case tab_calls_case_t;

struct tab_calls_case
{
  const char *const *words;
  size_t             word_count;
  bool               should_fold_prefix;
  const char        *text;
  const char        *keys;
  const char        *line;
  size_t             calls;
};

/* A TAB that grows the token to the common prefix opens the menu on the list
   it already gathered, and typing into that menu narrows it without asking the
   host, even when the token was empty. The host is asked again when the
   prefix opens a new path component, moves the token, or does not extend the
   token byte for byte. A second TAB that cannot grow the token asks the host
   once. */
static bool
test_tab_prefix_menu_reuses_gather(void)
{
  static const char *const tools[] = {"tool-alpha", "tool-beta", "other"};
  static const char *const paths[] = {"dir/a", "dir/b"};
  static const char *const values[] = {"key=one", "key=two"};
  static const char *const capitals[] = {"Alpha", "Album"};
  static const tab_calls_case_t cases[] = {
      {tools,    3, false, "run to",       "",   "run tool-",       1},
      {tools,    3, false, "run to",       "a",  "run tool-a",      1},
      {tools,    3, false, "run tool-",    "",   "run tool-",       1},
      {tools,    3, false, "run ",         "",   "run ",            1},
      {paths,    2, false, "run d",        "",   "run dir/",        2},
      {values,   2, false, "run k",        "",   "run key=",        2},
      {capitals, 2, true,  "run a",        "",   "run Al",          2},
      {tools,    2, false, "run ",         "",   "run tool-",       1},
      {tools,    2, false, "run ",         "b",  "run tool-b",      1},
  };
  bool   ok = true;
  char   line[BUFFER_SIZE];
  size_t i;

  for (i = 0; i < countof(cases); ++i) {
    size_t calls;

    test_tab_words = cases[i].words;
    test_tab_word_count = cases[i].word_count;
    test_tab_should_fold_prefix = cases[i].should_fold_prefix;
    calls = tab_completion_calls(cases[i].text, cases[i].keys, line,
                                 sizeof(line));

    if (calls != cases[i].calls || strcmp(line, cases[i].line) != 0) {
      TEST_PRINTF("tab case %zu left '%s' after %zu calls\n", i, line, calls);
      ok = false;
    }
  }

  test_tab_words = NULL;
  test_tab_word_count = 0;
  test_tab_should_fold_prefix = false;

  return ok;
}

/* A sole candidate goes in and the TAB stops, with or without the space after
   it. No menu opens for the next word or the inside of a completed directory,
   so the keys after the TAB stay unread and the host is asked once. A second
   TAB on the completed directory lists what it holds. */
static bool
test_tab_sole_candidate_stops(void)
{
  static const char *const files[] = {"file1.txt", "other"};
  static const char *const directories[] = {"local/", "other"};
  static const char *const inside[] = {"local/a", "local/b"};
  static const struct
  {
    const char *const        *words;
    size_t                    word_count;
    tl_space_after_completion space_after;
    const char               *text;
    const char               *line;
  } cases[] = {
      {files,       2, TL_SPACE_AFTER_COMPLETION_ON,  "cat fi",
       "cat file1.txt "                                                    },
      {files,       2, TL_SPACE_AFTER_COMPLETION_OFF, "cat fi",
       "cat file1.txt"                                                     },
      {directories, 2, TL_SPACE_AFTER_COMPLETION_ON,  "cd lo",  "cd local/ "},
      {directories, 2, TL_SPACE_AFTER_COMPLETION_OFF, "cd lo",  "cd local/" },
      {inside,      2, TL_SPACE_AFTER_COMPLETION_OFF, "cd local/",
       "cd local/"                                                         },
  };
  tl_space_after_completion previous_space_after =
      itl_g_space_after_completion;
  bool   ok = true;
  char   line[BUFFER_SIZE];
  size_t i;

  for (i = 0; i < countof(cases); ++i) {
    size_t calls;

    test_tab_words = cases[i].words;
    test_tab_word_count = cases[i].word_count;
    tl_set_space_after_completion(cases[i].space_after);
    calls = tab_completion_calls(cases[i].text, "\x1b", line, sizeof(line));

    if (calls != 1 || strcmp(line, cases[i].line) != 0) {
      TEST_PRINTF("sole case %zu left '%s' after %zu calls\n", i, line, calls);
      ok = false;
    }
  }

  tl_set_space_after_completion(previous_space_after);
  test_tab_words = NULL;
  test_tab_word_count = 0;

  return ok;
}
#endif /* ITL_POSIX */

static bool
test_menu_cells(void)
{
  static const char *red = "\x1b[31m";
  size_t             drawn;
  tl_highlight_span  spans[1];
  int                was_colors_enabled = itl_g_colors_enabled;
  itl_char_buf_t    *b = itl_char_buf_alloc();

  itl_g_colors_enabled = 1;

  drawn = itl_menu_append_cell(b, "ab", 5, true);

  if (drawn != 5 || b->size != 5 || memcmp(b->data, "ab   ", 5) != 0) {
    TEST_PRINTF("a padded cell drew %zu columns in %zu bytes\n", drawn,
                b->size);
    goto failed;
  }

  b->size = 0;
  drawn = itl_menu_append_cell(b, "ab", 5, false);

  if (drawn != 2 || b->size != 2 || memcmp(b->data, "ab", 2) != 0) {
    TEST_PRINTF("an unpadded cell drew %zu columns in %zu bytes\n", drawn,
                b->size);
    goto failed;
  }

  b->size = 0;
  drawn = itl_menu_append_cell(b, "abcdef", 3, true);

  if (drawn != 3 || b->size != 3 || memcmp(b->data, "abc", 3) != 0) {
    TEST_PRINTF("a clipped cell drew %zu columns in %zu bytes\n", drawn,
                b->size);
    goto failed;
  }

  b->size = 0;
  drawn = itl_menu_append_cell(b, "\xE6\x97\xA5\xE6\x9C\xAC", 3, true);

  if (drawn != 4 || b->size != 6 ||
      memcmp(b->data, "\xE6\x97\xA5\xE6\x9C\xAC", 6) != 0)
  {
    TEST_PRINTF("a straddling cell drew %zu columns in %zu bytes\n", drawn,
                b->size);
    goto failed;
  }

  b->size = 0;
  drawn = itl_menu_append_cell(b, "\xE6\x97\xA5\xE6\x9C\xAC", 6, true);

  if (drawn != 6 || b->size != 8 ||
      memcmp(b->data, "\xE6\x97\xA5\xE6\x9C\xAC  ", 8) != 0)
  {
    TEST_PRINTF("a wide padded cell drew %zu columns in %zu bytes\n", drawn,
                b->size);
    goto failed;
  }

  b->size = 0;
  spans[0].start = 1;
  spans[0].end = 3;
  spans[0].sgr = red;
  itl_menu_append_colored_cell(b, "abcdef", 6, spans, 1, false);

  if (b->size != 15 || memcmp(b->data, "a\x1b[31mbc\x1b[0mdef", 15) != 0) {
    TEST_PRINTF("a colored cell wrote %zu bytes\n", b->size);
    goto failed;
  }

  b->size = 0;
  spans[0].end = 6;
  itl_menu_append_colored_cell(b, "abcdef", 3, spans, 1, true);

  if (b->size != 12 || memcmp(b->data, "a\x1b[31mbc\x1b[0m", 12) != 0) {
    TEST_PRINTF("a clipped colored cell wrote %zu bytes\n", b->size);
    goto failed;
  }

  b->size = 0;
  itl_menu_append_colored_cell(b, "a\xE6\x97\xA5\xE6\x9C\xAC", 4, NULL, 0,
                               true);

  if (b->size != 5 || memcmp(b->data, "a\xE6\x97\xA5 ", 5) != 0) {
    TEST_PRINTF("a straddling colored cell wrote %zu bytes\n", b->size);
    goto failed;
  }

  itl_g_colors_enabled = 0;
  b->size = 0;
  spans[0].end = 3;
  itl_menu_append_colored_cell(b, "abcdef", 6, spans, 1, false);

  if (b->size != 6 || memcmp(b->data, "abcdef", 6) != 0) {
    TEST_PRINTF("a colorless cell wrote %zu bytes\n", b->size);
    goto failed;
  }

  itl_g_colors_enabled = was_colors_enabled;
  ITL_CHAR_BUF_FREE(b);

  return true;

failed:
  itl_g_colors_enabled = was_colors_enabled;
  ITL_CHAR_BUF_FREE(b);

  return false;
}

static bool
merged_spans_are(const tl_highlight_span *out, size_t count,
                 const tl_highlight_span *expected, size_t expected_count)
{
  size_t i;

  if (count != expected_count) {
    return false;
  }

  for (i = 0; i < count; ++i) {
    if (out[i].start != expected[i].start || out[i].end != expected[i].end ||
        out[i].sgr != expected[i].sgr)
    {
      return false;
    }
  }

  return true;
}

static bool
test_history_menu_multiline_display(void)
{
  static const char *const names[] = {"line one\nline two", "plain\rnext"};
  tl_completion result = ITL_ZERO_INIT;
  char          display[32];
  itl_char_buf_t *b = itl_char_buf_alloc();
  bool            ok = true;

  result.candidates = names;
  result.count = countof(names);

  if (strcmp(itl_menu_display_name(names[0], display, sizeof(display)),
             "line one...") != 0 ||
      strcmp(names[0], "line one\nline two") != 0 ||
      strcmp(itl_menu_display_name(names[1], display, sizeof(display)),
             "plain...") != 0 ||
      itl_menu_name_width(&result) != 11)
  {
    TEST_PRINTF("multiline menu display was not truncated correctly\n");
    ok = false;
  }

  itl_menu_append_row(b, &result, 0, 11, 0, false, false);
  if (!test_bytes_have(b->data, b->size, "line one...") ||
      test_bytes_have(b->data, b->size, "line two"))
  {
    TEST_PRINTF("multiline menu row still carried a second line\n");
    ok = false;
  }

  ITL_CHAR_BUF_FREE(b);
  return ok;
}

static bool
test_merge_visual_spans(void)
{
  static const char *first = "\x1b[31m";
  static const char *second = "\x1b[32m";
  static const char *inverse = "\x1b[7m";
  tl_highlight_span  syntax[3];
  tl_highlight_span  selection[1];
  tl_highlight_span  expected[3];
  tl_highlight_span  out[8];
  size_t             count;

  syntax[0].start = 0;
  syntax[0].end = 3;
  syntax[0].sgr = first;
  syntax[1].start = 3;
  syntax[1].end = 6;
  syntax[1].sgr = second;
  count = itl_merge_visual_spans(syntax, 2, NULL, 0, 6, out, countof(out));
  expected[0] = syntax[0];
  expected[1] = syntax[1];

  if (!merged_spans_are(out, count, expected, 2)) {
    TEST_PRINTF("two syntax runs merged into %zu spans\n", count);
    return false;
  }

  syntax[1].sgr = first;
  count = itl_merge_visual_spans(syntax, 2, NULL, 0, 6, out, countof(out));
  expected[0].start = 0;
  expected[0].end = 6;
  expected[0].sgr = first;

  if (!merged_spans_are(out, count, expected, 1)) {
    TEST_PRINTF("two adjacent runs merged into %zu spans\n", count);
    return false;
  }

  selection[0].start = 2;
  selection[0].end = 4;
  selection[0].sgr = inverse;
  count = itl_merge_visual_spans(NULL, 0, selection, 1, 6, out, countof(out));
  expected[0] = selection[0];

  if (!merged_spans_are(out, count, expected, 1)) {
    TEST_PRINTF("a lone selection merged into %zu spans\n", count);
    return false;
  }

  syntax[0].start = 0;
  syntax[0].end = 6;
  count = itl_merge_visual_spans(syntax, 1, selection, 1, 6, out, countof(out));
  expected[0].start = 0;
  expected[0].end = 2;
  expected[0].sgr = first;
  expected[1] = selection[0];
  expected[2].start = 4;
  expected[2].end = 6;
  expected[2].sgr = first;

  if (!merged_spans_are(out, count, expected, 3)) {
    TEST_PRINTF("a selection over syntax merged into %zu spans\n", count);
    return false;
  }

  syntax[0].start = 1;
  syntax[0].end = 2;
  count = itl_merge_visual_spans(syntax, 1, NULL, 0, 4, out, countof(out));
  expected[0] = syntax[0];

  if (!merged_spans_are(out, count, expected, 1)) {
    TEST_PRINTF("a gapped run merged into %zu spans\n", count);
    return false;
  }

  syntax[0].start = 0;
  syntax[0].end = 1;
  syntax[1].start = 1;
  syntax[1].end = 2;
  syntax[1].sgr = second;
  syntax[2].start = 2;
  syntax[2].end = 3;
  syntax[2].sgr = first;
  count = itl_merge_visual_spans(syntax, 3, NULL, 0, 3, out, 2);
  expected[0] = syntax[0];
  expected[1] = syntax[1];

  if (!merged_spans_are(out, count, expected, 2)) {
    TEST_PRINTF("an exhausted output holds %zu spans\n", count);
    return false;
  }

  count = itl_merge_visual_spans(NULL, 0, NULL, 0, 5, out, countof(out));

  if (count != 0) {
    TEST_PRINTF("an uncolored line merged into %zu spans\n", count);
    return false;
  }

  return true;
}

#if defined ITL_POSIX
static bool
test_write_all_resumes_a_partial_write(void)
{
  const size_t span_length = (size_t) 1 << 20;
  char        *data;
  int          pipe_fds[2];
  pid_t        child;
  size_t       written;
  int          child_status = 0;
  bool         ok;

  data = (char *) malloc(span_length);

  if (data == NULL) {
    TEST_PRINTF("could not allocate %zu bytes\n", span_length);
    return false;
  }

  memset(data, 'x', span_length);

  if (pipe(pipe_fds) != 0) {
    TEST_PRINTF("could not create a pipe\n");
    free(data);
    return false;
  }

  child = fork();

  if (child < 0) {
    TEST_PRINTF("could not fork a reader\n");
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    free(data);
    return false;
  }

  if (child == 0) {
    char sink[4096];

    close(pipe_fds[1]);

    while (read(pipe_fds[0], sink, sizeof(sink)) > 0) {
      continue;
    }

    close(pipe_fds[0]);
    _exit(0);
  }

  close(pipe_fds[0]);
  written = itl_write_all(pipe_fds[1], data, span_length);
  close(pipe_fds[1]);
  waitpid(child, &child_status, 0);
  free(data);

  ok = written == span_length;

  if (!ok) {
    TEST_PRINTF("wrote %zu of %zu bytes\n", written, span_length);
  }

  return ok;
}

static volatile sig_atomic_t test_alarm_fired;

static void
test_alarm_handler(int signal_number)
{
  (void) signal_number;
  test_alarm_fired = 1;
}

static bool
test_alt_backspace_sequences(void)
{
  int delete_event;
  int backspace_event;

  itl_g_pushback_byte = 127;
  delete_event = itl_esc_parse(27);
  itl_g_pushback_byte = 8;
  backspace_event = itl_esc_parse(27);

  return delete_event == (TL_KEY_BACKSPACE | TL_MOD_CTRL) &&
         backspace_event == (TL_KEY_BACKSPACE | TL_MOD_CTRL);
}

static bool
test_pending_resize_wakes_input_wait(void)
{
  int pipe_descriptors[2] = {-1, -1};
  int saved_stdin = -1;
  bool did_block_signals = false;
  bool result = false;
  sigset_t saved_mask;
  sigset_t wait_mask;
  struct sigaction saved_alarm_action = ITL_ZERO_INIT;
  struct sigaction saved_resize_action = ITL_ZERO_INIT;
  struct sigaction alarm_action = ITL_ZERO_INIT;
  struct sigaction resize_action = ITL_ZERO_INIT;

  if (sigprocmask(SIG_SETMASK, NULL, &saved_mask) != 0) goto cleanup;
  wait_mask = saved_mask;
  if (sigdelset(&wait_mask, SIGALRM) != 0 ||
      sigdelset(&wait_mask, SIGWINCH) != 0 ||
      sigprocmask(SIG_SETMASK, &wait_mask, NULL) != 0)
  {
    goto cleanup;
  }
  if (sigaction(SIGALRM, NULL, &saved_alarm_action) != 0 ||
      sigaction(SIGWINCH, NULL, &saved_resize_action) != 0)
  {
    goto cleanup;
  }

  alarm_action.sa_handler = test_alarm_handler;
  resize_action.sa_handler = itl_handle_sigwinch;
  if (sigemptyset(&alarm_action.sa_mask) != 0 ||
      sigemptyset(&resize_action.sa_mask) != 0 ||
      sigaction(SIGALRM, &alarm_action, NULL) != 0 ||
      sigaction(SIGWINCH, &resize_action, NULL) != 0)
  {
    goto cleanup;
  }
  if (pipe(pipe_descriptors) != 0) goto cleanup;
  saved_stdin = dup(STDIN_FILENO);
  if (saved_stdin < 0 || dup2(pipe_descriptors[0], STDIN_FILENO) < 0)
    goto cleanup;
  if (!itl_block_input_wake_signals(&wait_mask)) goto cleanup;
  did_block_signals = true;

  itl_g_tty_changed_size = 0;
  test_alarm_fired = 0;
  if (raise(SIGWINCH) != 0) goto cleanup;
  alarm(1);
  result = itl_wait_for_input(&wait_mask) && test_alarm_fired == 0 &&
           itl_g_tty_changed_size != 0;

cleanup:
  alarm(0);
  if (did_block_signals) itl_restore_input_wake_signals(&wait_mask);
  if (saved_stdin >= 0) {
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdin);
  }
  if (pipe_descriptors[0] >= 0) close(pipe_descriptors[0]);
  if (pipe_descriptors[1] >= 0) close(pipe_descriptors[1]);
  sigaction(SIGALRM, &saved_alarm_action, NULL);
  sigaction(SIGWINCH, &saved_resize_action, NULL);
  sigprocmask(SIG_SETMASK, &saved_mask, NULL);
  test_alarm_fired = 0;
  itl_g_tty_changed_size = 1;
  return result;
}

static int test_idle_wait_call_count = 0;
static int test_idle_wait_writer = -1;

static int
test_idle_wait_callback(const char *buffer, size_t cursor)
{
  (void) buffer;
  (void) cursor;
  test_idle_wait_call_count += 1;
  if (test_idle_wait_call_count == 1) {
    return TL_IDLE_AGAIN;
  }
  if (write(test_idle_wait_writer, "k", 1) != 1) {
    return 0;
  }
  return 0;
}

static bool
test_idle_hook_runs_in_the_input_wait(void)
{
  int           pipe_descriptors[2] = {-1, -1};
  int           saved_stdin = -1;
  int           timeout_result = -1;
  int           pending_result = -1;
  bool          did_return = false;
  bool          result = false;
  uint64_t      started_ms;
  uint64_t      waited_ms = 0;
  uint8_t       byte = 0;
  char          out_buffer[BUFFER_SIZE];
  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();
  sigset_t      wait_mask;

  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "> ");
  if (pipe(pipe_descriptors) != 0) goto cleanup;
  saved_stdin = dup(STDIN_FILENO);
  if (saved_stdin < 0 || dup2(pipe_descriptors[0], STDIN_FILENO) < 0)
    goto cleanup;

  if (sigprocmask(SIG_SETMASK, NULL, &wait_mask) != 0) goto cleanup;
  timeout_result = itl_wait_for_input_until(&wait_mask, 10);
  if (write(pipe_descriptors[1], "x", 1) != 1) goto cleanup;
  pending_result = itl_wait_for_input_until(&wait_mask, 1000);
  if (read(STDIN_FILENO, &byte, 1) != 1) goto cleanup;

  itl_g_tty_changed_size = 0;
  test_idle_wait_call_count = 0;
  test_idle_wait_writer = pipe_descriptors[1];
  tl_set_idle_callback(test_idle_wait_callback, 30, 10);
  itl_idle_arm();
  started_ms = itl_monotonic_ms();
  did_return = itl_le_wait_for_key(&le);
  waited_ms = itl_monotonic_ms() - started_ms;

  result = timeout_result == 0 && pending_result == 1 && did_return &&
           test_idle_wait_call_count == 2 && waited_ms >= 40 &&
           itl_input_is_pending() && read(STDIN_FILENO, &byte, 1) == 1 &&
           byte == 'k';

cleanup:
  tl_set_idle_callback(NULL, 0, 0);
  test_idle_wait_writer = -1;
  if (saved_stdin >= 0) {
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdin);
  }
  if (pipe_descriptors[0] >= 0) close(pipe_descriptors[0]);
  if (pipe_descriptors[1] >= 0) close(pipe_descriptors[1]);
  ITL_STRING_FREE(line);
  itl_g_tty_changed_size = 1;

  if (!result) {
    TEST_PRINTF("timeout %d, pending %d, returned %d, calls %d, waited %llu "
                "ms\n",
                timeout_result, pending_result, (int) did_return,
                test_idle_wait_call_count, (unsigned long long) waited_ms);
  }

  return result;
}
#endif

static int
test_completion_callback(const char *buffer, size_t cursor,
                         tl_completion *completion, int for_listing)
{
  static const char *candidates[] = {"alpha"};
  static size_t call_count;

  (void) buffer;
  (void) cursor;
  (void) for_listing;
  completion->candidates = candidates;
  completion->count = call_count++ == 0 ? 1 : 0;
  completion->token_start = 0;
  completion->token_end = 2;
  return 1;
}

static int
test_tailscale_completion_callback(const char *buffer, size_t cursor,
                                   tl_completion *completion, int for_listing)
{
  static const char *candidates[] = {"tailscale"};

  (void) buffer;
  (void) cursor;
  (void) for_listing;
  completion->candidates = candidates;
  completion->count = 1;
  completion->longest_common_prefix = "tailscale";
  completion->token_start = 0;
  completion->token_end = 4;
  return 1;
}

static bool
test_ghost_prefers_recent_history(void)
{
  const char *path = "tl_test_ghost_history_priority.txt";
  char out_buffer[BUFFER_SIZE];
  char line_buffer[BUFFER_SIZE];
  bool ok = true;
  itl_le_t le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  itl_g_is_active = true;
  remove(path);
  tl_history_load(path);
  if (!hist_append_cstr("tailscale up --older")) ok = false;
  if (!hist_append_cstr("tailscale status --json")) ok = false;

  ITL_STRING_FROM_CSTR(line, "tail");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  le.cursor_position = line->length;
  tl_set_complete_callback(test_tailscale_completion_callback);
  itl_ghost_update(&le);
  if (strcmp(itl_g_ghost, "scale status --json") != 0) {
    TEST_PRINTF("ghost was '%s'\n", itl_g_ghost);
    ok = false;
  }
  itl_ghost_accept(&le);
  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));
  if (strcmp(line_buffer, "tailscale status --json") != 0) {
    TEST_PRINTF("accepted line was '%s'\n", line_buffer);
    ok = false;
  }

  remove(path);
  itl_g_history_free();
  tl_history_load(path);
  if (!hist_append_cstr("unrelated command")) ok = false;
  ITL_STRING_FROM_CSTR(line, "tail");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  itl_ghost_update(&le);
  if (strcmp(itl_g_ghost, "scale") != 0) {
    TEST_PRINTF("fallback ghost was '%s'\n", itl_g_ghost);
    ok = false;
  }

  tl_set_complete_callback(NULL);
  itl_ghost_clear();
  itl_g_ghost_sticky_target[0] = '\0';
  ITL_STRING_FREE(line);
  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}

static int
test_cased_completion_callback(const char *buffer, size_t cursor,
                               tl_completion *completion, int for_listing)
{
  static const char *candidates[] = {"Tailscale"};

  (void) buffer;
  (void) cursor;
  (void) for_listing;
  completion->candidates = candidates;
  completion->count = 1;
  completion->longest_common_prefix = "Tailscale";
  completion->token_start = 0;
  completion->token_end = 4;

  return 1;
}

static bool
test_ghost_history_corrects_case(void)
{
  const char   *path = "tl_test_ghost_case.txt";
  char          out_buffer[BUFFER_SIZE];
  char          line_buffer[BUFFER_SIZE];
  bool          ok = true;
  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  itl_g_is_active = true;
  itl_g_tty_plain_append_pending = false;
  remove(path);
  tl_history_load(path);

  if (!hist_append_cstr("Tailscale Status --Json")) ok = false;

  ITL_STRING_FROM_CSTR(line, "tail");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  itl_undo_reset();
  itl_ghost_update(&le);

  if (strcmp(itl_g_ghost, "scale Status --Json") != 0 ||
      !itl_g_ghost_should_replace_line ||
      strcmp(itl_g_ghost_sticky_target, "Tailscale Status --Json") != 0)
  {
    TEST_PRINTF("case-correcting ghost was '%s', target '%s'\n", itl_g_ghost,
                itl_g_ghost_sticky_target);
    ok = false;
  }

  itl_ghost_accept(&le);
  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));

  if (strcmp(line_buffer, "Tailscale Status --Json") != 0) {
    TEST_PRINTF("accepted line was '%s'\n", line_buffer);
    ok = false;
  }

  itl_undo_close_insert_run();

  if (!itl_undo_pop(&le)) {
    TEST_PRINTF("the accept pushed no undo snapshot\n");
    ok = false;
  }

  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));

  if (strcmp(line_buffer, "tail") != 0 || le.cursor_position != 4) {
    TEST_PRINTF("undo restored '%s' at %zu\n", line_buffer,
                le.cursor_position);
    ok = false;
  }

  itl_undo_reset();
  itl_ghost_clear();
  itl_g_ghost_sticky_target[0] = '\0';
  ITL_STRING_FREE(line);
  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;

  return ok;
}

static bool
test_ghost_completion_corrects_case(void)
{
  const char   *path = "tl_test_ghost_completion_case.txt";
  char          out_buffer[BUFFER_SIZE];
  char          line_buffer[BUFFER_SIZE];
  bool          ok = true;
  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  itl_g_is_active = true;
  itl_g_tty_plain_append_pending = false;
  remove(path);
  tl_history_load(path);

  ITL_STRING_FROM_CSTR(line, "tail");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  itl_undo_reset();
  tl_set_complete_callback(test_cased_completion_callback);
  itl_ghost_update(&le);

  if (strcmp(itl_g_ghost, "scale") != 0 || !itl_g_ghost_should_replace_line ||
      strcmp(itl_g_ghost_sticky_target, "Tailscale") != 0)
  {
    TEST_PRINTF("completion ghost was '%s', target '%s'\n", itl_g_ghost,
                itl_g_ghost_sticky_target);
    ok = false;
  }

  itl_ghost_accept(&le);
  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));

  if (strcmp(line_buffer, "Tailscale") != 0) {
    TEST_PRINTF("accepted completion line was '%s'\n", line_buffer);
    ok = false;
  }

  itl_undo_close_insert_run();

  if (!itl_undo_pop(&le)) {
    TEST_PRINTF("the completion accept pushed no undo snapshot\n");
    ok = false;
  }

  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));

  if (strcmp(line_buffer, "tail") != 0) {
    TEST_PRINTF("undo after a completion accept restored '%s'\n", line_buffer);
    ok = false;
  }

  tl_set_complete_callback(NULL);
  itl_undo_reset();
  itl_ghost_clear();
  itl_g_ghost_sticky_target[0] = '\0';
  ITL_STRING_FREE(line);
  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;

  return ok;
}

/* The menu previews a highlighted row as a ghost only when the row continues
   the typed token, so a row matched inside its text draws no ghost. */
static bool
test_menu_preview_needs_an_extending_row(void)
{
  static const char *names[] = {"apple", "Pear"};
  char               out_buffer[BUFFER_SIZE];
  bool               is_inner_match_hidden;
  bool               is_extending_shown;
  tl_completion      result = ITL_ZERO_INIT;
  itl_le_t           le = ITL_ZERO_INIT;
  itl_string_t      *line = itl_string_alloc();

  ITL_STRING_FROM_CSTR(line, "cat p");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  result.candidates = names;
  result.count = countof(names);
  result.token_start = 4;
  result.token_end = 5;

  itl_menu_ghost_preview(&le, &result, 0);
  is_inner_match_hidden = itl_g_ghost_len == 0;
  itl_menu_ghost_preview(&le, &result, 1);
  is_extending_shown = strcmp(itl_g_ghost, "ear") == 0;

  itl_ghost_clear();
  itl_g_ghost_sticky_target[0] = '\0';
  ITL_STRING_FREE(line);

  if (!is_inner_match_hidden || !is_extending_shown) {
    TEST_PRINTF("inner match hidden %d, extending row shown %d\n",
                (int) is_inner_match_hidden, (int) is_extending_shown);
    return false;
  }

  return true;
}

static bool
test_ghost_sticky_target_continues(void)
{
  const char   *path = "tl_test_ghost_sticky.txt";
  char          out_buffer[BUFFER_SIZE];
  bool          ok = true;
  int           previous_enabled = itl_g_ghost_enabled;
  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  itl_g_is_active = true;
  itl_g_tty_plain_append_pending = false;
  remove(path);
  tl_history_load(path);

  ITL_STRING_FROM_CSTR(line, "tails");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  memcpy(itl_g_ghost_sticky_target, "Tailscale Status", 17);
  itl_ghost_update(&le);

  if (strcmp(itl_g_ghost, "cale Status") != 0 ||
      !itl_g_ghost_should_replace_line)
  {
    TEST_PRINTF("sticky continuation gave '%s'\n", itl_g_ghost);
    ok = false;
  }

  ITL_STRING_FROM_CSTR(line, "tailX");
  le.cursor_position = line->length;
  itl_ghost_update(&le);

  if (itl_g_ghost_len != 0 || itl_g_ghost_sticky_target[0] != '\0') {
    TEST_PRINTF("a diverging line kept the target '%s'\n",
                itl_g_ghost_sticky_target);
    ok = false;
  }

  ITL_STRING_FROM_CSTR(line, "tail");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  memcpy(itl_g_ghost_sticky_target, "Tailscale Status", 17);
  le.cursor_position = 1;
  itl_ghost_update(&le);

  if (itl_g_ghost_len != 0) {
    TEST_PRINTF("a mid-line cursor still drew '%s'\n", itl_g_ghost);
    ok = false;
  }

  le.cursor_position = line->length;
  itl_g_ghost_enabled = 0;
  itl_ghost_update(&le);
  itl_g_ghost_enabled = previous_enabled;

  if (itl_g_ghost_len != 0) {
    TEST_PRINTF("a disabled ghost still drew '%s'\n", itl_g_ghost);
    ok = false;
  }

  itl_string_clear(line);
  le.cursor_position = 0;
  itl_ghost_update(&le);

  if (itl_g_ghost_len != 0 || itl_g_ghost_sticky_target[0] != '\0') {
    TEST_PRINTF("an emptied line kept the target '%s'\n",
                itl_g_ghost_sticky_target);
    ok = false;
  }

  itl_ghost_clear();
  itl_g_ghost_sticky_target[0] = '\0';
  ITL_STRING_FREE(line);
  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;

  return ok;
}

static bool
test_ghost_clips_multiline_suggestion(void)
{
  const char   *path = "tl_test_ghost_multiline.txt";
  char          out_buffer[BUFFER_SIZE];
  bool          ok = true;
  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  itl_g_is_active = true;
  itl_g_tty_plain_append_pending = false;
  remove(path);
  tl_history_load(path);

  if (!hist_append_cstr("echo one\necho two")) ok = false;

  ITL_STRING_FROM_CSTR(line, "echo o");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  itl_ghost_update(&le);

  if (strcmp(itl_g_ghost, "ne") != 0 || itl_g_ghost_len != 2 ||
      strcmp(itl_g_ghost_sticky_target, "echo one") != 0)
  {
    TEST_PRINTF("clipped ghost was '%s', target '%s'\n", itl_g_ghost,
                itl_g_ghost_sticky_target);
    ok = false;
  }

  if (!hist_append_cstr("date\nsleep 1")) ok = false;

  ITL_STRING_FROM_CSTR(line, "date");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  itl_ghost_update(&le);

  if (itl_g_ghost_len != 0 || itl_g_ghost_sticky_target[0] != '\0') {
    TEST_PRINTF("a suggestion starting with a newline gave '%s'\n",
                itl_g_ghost);
    ok = false;
  }

  itl_ghost_clear();
  itl_g_ghost_sticky_target[0] = '\0';
  ITL_STRING_FREE(line);
  remove(path);
  itl_g_history_free();
  itl_g_is_active = false;

  return ok;
}

static bool
test_tab_clears_stale_ghost_target(void)
{
  char out_buffer[BUFFER_SIZE];
  char line_buffer[BUFFER_SIZE];
  bool replacement_ok;
  bool was_handled;
  tl_status_code completion_code = TL_SUCCESS;
  int previous_supports_decorations = itl_g_supports_decorations;
  itl_le_t le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  itl_g_supports_decorations = 0;
  ITL_STRING_FROM_CSTR(line, "ab");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  memcpy(itl_g_ghost_sticky_target, "stale", 6);
  tl_set_complete_callback(test_completion_callback);
  was_handled = itl_completion_handle_tab(&le, &completion_code);
  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));
  replacement_ok = was_handled && strcmp(line_buffer, "alpha") == 0;

  memcpy(itl_g_ghost_sticky_target, "stale", 6);
  was_handled = itl_completion_handle_tab(&le, &completion_code);
  tl_set_complete_callback(NULL);
  itl_g_supports_decorations = previous_supports_decorations;

  ITL_STRING_FREE(line);
  return replacement_ok && was_handled && completion_code == TL_SUCCESS &&
         itl_g_ghost_sticky_target[0] == '\0';
}

static int test_space_suppressed_flag;

static int
test_space_suppressing_callback(const char *buffer, size_t cursor,
                                tl_completion *completion, int for_listing)
{
  static const char *candidates[] = {"alpha"};

  (void) buffer;
  (void) cursor;
  (void) for_listing;
  completion->candidates = candidates;
  completion->count = 1;
  completion->token_start = 0;
  completion->token_end = 2;
  completion->is_space_suppressed = test_space_suppressed_flag;
  return 1;
}

/* A host that suppresses the space keeps the caret on the completed word even
   with the space-after option on, and the same single candidate takes the
   space when the host leaves the flag clear. */
static bool
test_tab_honors_suppressed_space(void)
{
  static const int flags[] = {1, 0};
  static const char *expected[] = {"alpha", "alpha "};
  char out_buffer[BUFFER_SIZE];
  char line_buffer[BUFFER_SIZE];
  int previous_supports_decorations = itl_g_supports_decorations;
  tl_space_after_completion previous_space_after =
      itl_g_space_after_completion;
  bool ok = true;
  size_t index;

  itl_g_supports_decorations = 0;
  tl_set_space_after_completion(TL_SPACE_AFTER_COMPLETION_ON);
  tl_set_complete_callback(test_space_suppressing_callback);
  for (index = 0; index < 2; ++index) {
    tl_status_code completion_code = TL_SUCCESS;
    itl_le_t le = ITL_ZERO_INIT;
    itl_string_t *line = itl_string_alloc();

    ITL_STRING_FROM_CSTR(line, "al");
    itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
    test_space_suppressed_flag = flags[index];
    if (!itl_completion_handle_tab(&le, &completion_code)) {
      ok = false;
    }
    itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));
    if (strcmp(line_buffer, expected[index]) != 0) {
      TEST_PRINTF("flag %d completed to '%s'\n", flags[index], line_buffer);
      ok = false;
    }
    ITL_STRING_FREE(line);
  }
  tl_set_complete_callback(NULL);
  tl_set_space_after_completion(previous_space_after);
  itl_g_supports_decorations = previous_supports_decorations;

  return ok;
}

/* The external-screen pair is only meaningful around a live raw-mode session,
   so each half refuses the state the other one owns. Driving a real handoff
   needs a terminal and is covered by the interactive pty harness instead. */
static bool
test_external_screen_requires_raw_mode(void)
{
  bool begin_refused;
  bool end_refused;
  bool previous_active = itl_g_is_active;
  bool previous_raw = itl_g_entered_raw_mode;

  itl_g_is_active = true;

  itl_g_entered_raw_mode = false;
  begin_refused = tl_begin_external_screen() == TL_ERROR;

  itl_g_entered_raw_mode = true;
  end_refused = tl_end_external_screen() == TL_ERROR;

  itl_g_entered_raw_mode = previous_raw;
  itl_g_is_active = previous_active;

  return begin_refused && end_refused;
}

#if defined ITL_POSIX && !defined NDEBUG
static int
test_whole_line_highlight_callback(const char *buffer, tl_highlight *out)
{
  if (out->capacity == 0) {
    return 0;
  }

  out->spans[0].start = 0;
  out->spans[0].end = tl_utf8_strlen(buffer);
  out->spans[0].sgr = "\x1b[32m";
  out->count = out->spans[0].end > 0 ? 1 : 0;

  return 1;
}

static bool
test_append_path_keeps_spans(void)
{
  const char *keys = "abcd";
  char        out_buffer[BUFFER_SIZE];
  int         null_descriptor = -1;
  int         saved_stdout = -1;
  size_t      key_index;
  bool        ok = false;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  null_descriptor = open("/dev/null", O_WRONLY);
  if (null_descriptor < 0) goto cleanup;

  saved_stdout = dup(STDOUT_FILENO);
  if (saved_stdout < 0) goto cleanup;
  if (dup2(null_descriptor, STDOUT_FILENO) < 0) goto cleanup;

  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "> ");
  itl_g_tty_changed_size = 0;
  itl_g_tty_prev_rows = 24;
  itl_g_tty_prev_cols = 80;
  itl_g_tty_first_render = true;
  itl_g_le_prev_total_rows = 1;
  itl_g_le_prev_cursor_row = 1;
  itl_g_le_prev_cursor_col = 0;
  itl_g_le_prev_render_len = 0;
  itl_g_le_prev_length = 0;
  itl_g_le_prev_cursor_at_end = false;
  itl_g_le_prev_spans_usable = false;
  itl_g_le_prev_ghost_len = 0;
  itl_g_tty_plain_append_pending = false;
  itl_g_debug_append_refresh_count = 0;
  itl_g_debug_full_refresh_count = 0;
  tl_set_highlight_callback(test_whole_line_highlight_callback);

  for (key_index = 0; keys[key_index] != '\0'; ++key_index) {
    itl_utf8_t appended_character =
        itl_utf8_parse((uint8_t) keys[key_index]);

    itl_g_tty_plain_append_pending = le.cursor_position == le.line->length;
    itl_g_tty_plain_append_width = itl_char_width(appended_character);
    itl_le_insert(&le, appended_character);
    itl_g_tty_should_refresh_text = true;
    itl_le_tty_refresh(&le);
  }

  ok = itl_g_debug_append_refresh_count == 3 &&
       itl_g_debug_full_refresh_count == 1;

cleanup:
  if (saved_stdout >= 0) {
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
  }
  if (null_descriptor >= 0) close(null_descriptor);

  tl_set_highlight_callback(NULL);
  itl_g_tty_changed_size = 1;
  itl_g_tty_first_render = true;
  ITL_STRING_FREE(line);

  if (!ok) {
    TEST_PRINTF("append %zu, full %zu, expected 3 and 1\n",
                itl_g_debug_append_refresh_count,
                itl_g_debug_full_refresh_count);
  }

  return ok;
}

static bool
test_drawn_metrics_match_the_walk(void)
{
  static const char wide[] = {(char) 0xE4, (char) 0xBD, (char) 0xA0};
  char              text[16];
  char              out_buffer[BUFFER_SIZE];
  int               null_descriptor = -1;
  int               saved_stdout = -1;
  size_t            length = 0;
  size_t            cols;
  size_t            failed_cols = 0;
  size_t            failed_position = 0;
  itl_le_metrics_t  failed_expected = ITL_ZERO_INIT;
  bool              ok = true;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  null_descriptor = open("/dev/null", O_WRONLY);
  if (null_descriptor < 0) {
    ok = false;
    goto cleanup;
  }

  saved_stdout = dup(STDOUT_FILENO);
  if (saved_stdout < 0) {
    ok = false;
    goto cleanup;
  }
  if (dup2(null_descriptor, STDOUT_FILENO) < 0) {
    ok = false;
    goto cleanup;
  }

  text[length++] = 'a';
  text[length++] = 'b';
  memcpy(text + length, wide, sizeof(wide));
  length += sizeof(wide);
  text[length++] = '\n';
  text[length++] = 'c';
  text[length++] = 'd';
  text[length] = '\0';

  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "> ");
  ITL_STRING_FROM_CSTR(line, text);

  for (cols = 4; cols <= 12 && ok; ++cols) {
    size_t position;

    for (position = 0; position <= line->length && ok; ++position) {
      itl_le_metrics_t expected;

      itl_le_invalidate_prev_frame();
      itl_g_tty_changed_size = 0;
      itl_g_tty_prev_rows = 24;
      itl_g_tty_prev_cols = cols;
      itl_g_tty_first_render = false;
      itl_g_tty_plain_append_pending = false;
      itl_g_tty_should_refresh_text = true;
      itl_g_debug_metrics_scan_count = 0;

      le.cursor_position = position;
      expected = itl_le_compute_metrics(&le, cols);
      itl_le_tty_refresh(&le);

      if (itl_g_debug_metrics_scan_count != 0 ||
          itl_g_le_prev_total_rows != expected.total_rows ||
          itl_g_le_prev_cursor_row != expected.cursor_row + 1 ||
          itl_g_le_prev_cursor_col != expected.cursor_col)
      {
        failed_cols = cols;
        failed_position = position;
        failed_expected = expected;
        ok = false;
      }
    }
  }

cleanup:
  if (saved_stdout >= 0) {
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
  }
  if (null_descriptor >= 0) close(null_descriptor);

  itl_g_tty_changed_size = 1;
  itl_g_tty_first_render = true;
  ITL_STRING_FREE(line);

  if (!ok) {
    TEST_PRINTF("%zu cols, caret %zu, scans %zu, drew %zu %zu %zu against "
                "%zu %zu %zu\n",
                failed_cols, failed_position, itl_g_debug_metrics_scan_count,
                itl_g_le_prev_total_rows, itl_g_le_prev_cursor_row,
                itl_g_le_prev_cursor_col, failed_expected.total_rows,
                failed_expected.cursor_row + 1, failed_expected.cursor_col);
  }

  return ok;
}
#endif

static bool
test_reflow_agrees_with_metrics(void)
{
  const char *prompt = "one\ntwo\n> ";
  const char *text = "alpha\nbeta gamma delta epsilon zeta\nomega";
  char        out_buffer[BUFFER_SIZE];
  size_t      position;
  bool        ok = true;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), prompt);
  ITL_STRING_FROM_CSTR(line, text);

  for (position = 0; position <= line->length; ++position) {
    size_t rows_above;
    itl_le_metrics_t m;

    le.cursor_position = position;
    m = itl_le_compute_metrics(&le, 20);
    rows_above = itl_le_reflow_rows_above_caret(&le, 20, 20);

    if (rows_above != m.cursor_row) {
      TEST_PRINTF("at %zu reflow gave %zu, metrics gave %zu\n", position,
                  rows_above, m.cursor_row);
      ok = false;
      break;
    }
  }

  ITL_STRING_FREE(line);
  return ok;
}

static size_t
test_reference_reflow(const itl_le_t *le, size_t old_cols, size_t new_cols)
{
  size_t ocols = old_cols > 1 ? old_cols : 1;
  size_t ncols = new_cols > 1 ? new_cols : 1;
  size_t indent = itl_le_prompt_indent(le, ocols);
  size_t col = indent;
  size_t rows_above = le->prompt_rows;
  size_t i;

  for (i = 0; i <= le->line->length; ++i) {
    size_t char_width;

    if (i == le->cursor_position) {
      return rows_above + col / ncols;
    }

    if (i == le->line->length) {
      break;
    }

    if (ITL_LE_IS_NEWLINE(le->line->chars[i])) {
      rows_above += (col + ncols - 1) / ncols > 0 ? (col + ncols - 1) / ncols : 1;
      col = indent;
      continue;
    }

    char_width = itl_char_width(le->line->chars[i]);

    if (char_width == 2 && col + 1 >= ocols) {
      rows_above +=
          (col + ncols - 1) / ncols > 0 ? (col + ncols - 1) / ncols : 1;
      col = indent;
    }

    col += char_width;

    if (col >= ocols) {
      rows_above +=
          (col + ncols - 1) / ncols > 0 ? (col + ncols - 1) / ncols : 1;
      col = indent;
    }
  }

  return rows_above + col / ncols;
}

static bool
test_reflow_agrees_with_reference(void)
{
  static const char        wide[] = {(char) 0xE4, (char) 0xBD, (char) 0xA0};
  static const char *const PROMPTS[] = {"", "> ", "one\ntwo\nkosh> "};
  char                     text[160];
  char                     out_buffer[BUFFER_SIZE];
  size_t                   prompt_index;
  bool                     ok = true;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  for (prompt_index = 0;
       prompt_index < sizeof(PROMPTS) / sizeof(PROMPTS[0]) && ok; ++prompt_index)
  {
    size_t wide_position;

    itl_le_init(&le, line, out_buffer, sizeof(out_buffer),
                PROMPTS[prompt_index]);

    for (wide_position = 0; wide_position <= 24 && ok; ++wide_position) {
      size_t length = 0;
      size_t filler;
      size_t old_cols;

      for (filler = 0; filler < wide_position; ++filler) {
        text[length++] = 'a';
      }

      memcpy(text + length, wide, sizeof(wide));
      length += sizeof(wide);
      text[length++] = '\n';

      for (filler = wide_position; filler < 24; ++filler) {
        text[length++] = 'b';
      }

      text[length] = '\0';
      ITL_STRING_FROM_CSTR(line, text);

      for (old_cols = 1; old_cols <= 16 && ok; ++old_cols) {
        size_t new_cols;

        for (new_cols = 1; new_cols <= 16 && ok; ++new_cols) {
          size_t position;

          for (position = 0; position <= line->length + 1; ++position) {
            size_t expected;
            size_t actual;

            le.cursor_position = position;
            expected = test_reference_reflow(&le, old_cols, new_cols);
            actual = itl_le_reflow_rows_above_caret(&le, old_cols, new_cols);

            if (expected != actual) {
              TEST_PRINTF("prompt %zu, wide %zu, %zu to %zu cols, caret %zu "
                          "gave %zu against %zu\n",
                          prompt_index, wide_position, old_cols, new_cols,
                          position, actual, expected);
              ok = false;
              break;
            }
          }
        }
      }
    }
  }

  ITL_STRING_FREE(line);

  return ok;
}

static itl_le_metrics_t
test_reference_metrics(const itl_le_t *le, size_t tty_cols)
{
  itl_le_metrics_t m = ITL_ZERO_INIT;
  size_t           cols = tty_cols > 1 ? tty_cols : 1;
  size_t           indent = itl_le_prompt_indent(le, cols);
  size_t           row = le->prompt_rows;
  size_t           col = indent;
  size_t           i;

  for (i = 0; i <= le->line->length; ++i) {
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

static bool
test_ascii_runs_agree_with_reference(void)
{
  static const char        wide[] = {(char) 0xE4, (char) 0xBD, (char) 0xA0};
  static const char *const PROMPTS[] = {"", "> ", "kosh rather long prompt> "};
  char                     text[128];
  char                     out_buffer[BUFFER_SIZE];
  size_t                   prompt_index;
  bool                     ok = true;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  for (prompt_index = 0;
       prompt_index < sizeof(PROMPTS) / sizeof(PROMPTS[0]) && ok; ++prompt_index)
  {
    size_t break_position;

    itl_le_init(&le, line, out_buffer, sizeof(out_buffer),
                PROMPTS[prompt_index]);

    for (break_position = 0; break_position <= 30 && ok; ++break_position) {
      size_t length = 0;
      size_t filler;
      size_t cols;

      for (filler = 0; filler < break_position; ++filler) {
        text[length++] = 'a';
      }

      text[length++] = '\n';
      memcpy(text + length, wide, sizeof(wide));
      length += sizeof(wide);

      for (filler = break_position; filler < 30; ++filler) {
        text[length++] = 'b';
      }

      text[length] = '\0';
      ITL_STRING_FROM_CSTR(line, text);

      for (cols = 1; cols <= 20 && ok; ++cols) {
        size_t position;

        for (position = 0; position <= line->length + 1; ++position) {
          itl_le_metrics_t expected;
          itl_le_metrics_t actual;

          le.cursor_position = position;
          expected = test_reference_metrics(&le, cols);
          actual = itl_le_compute_metrics(&le, cols);

          if (expected.cursor_row != actual.cursor_row ||
              expected.cursor_col != actual.cursor_col ||
              expected.total_rows != actual.total_rows)
          {
            TEST_PRINTF("prompt %zu, break %zu, %zu cols, caret %zu gave "
                        "%zu %zu %zu against %zu %zu %zu\n",
                        prompt_index, break_position, cols, position,
                        actual.cursor_row, actual.cursor_col, actual.total_rows,
                        expected.cursor_row, expected.cursor_col,
                        expected.total_rows);
            ok = false;
            break;
          }
        }
      }
    }
  }

  ITL_STRING_FREE(line);

  return ok;
}

static bool
test_wrap_predicates_agree_with_reference(void)
{
  static const char wide[] = {(char) 0xE4, (char) 0xBD, (char) 0xA0};
  char              text[32];
  char              out_buffer[BUFFER_SIZE];
  size_t            insert_position;
  size_t            cols;
  bool              ok = true;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "ab> ");

  for (insert_position = 0; insert_position <= 12 && ok; ++insert_position) {
    size_t length = 0;
    size_t filler;

    for (filler = 0; filler < insert_position; ++filler) {
      text[length++] = 'a';
    }

    memcpy(text + length, wide, sizeof(wide));
    length += sizeof(wide);

    for (filler = insert_position; filler < 12; ++filler) {
      text[length++] = 'b';
    }

    text[length] = '\0';
    ITL_STRING_FROM_CSTR(line, text);

    for (cols = 1; cols <= 16 && ok; ++cols) {
      size_t position;

      for (position = 0; position <= line->length; ++position) {
        itl_le_metrics_t expected;
        itl_le_metrics_t actual;

        le.cursor_position = position;
        expected = test_reference_metrics(&le, cols);
        actual = itl_le_compute_metrics(&le, cols);

        if (expected.cursor_row != actual.cursor_row ||
            expected.cursor_col != actual.cursor_col ||
            expected.total_rows != actual.total_rows)
        {
          TEST_PRINTF("wide at %zu, %zu cols, caret %zu gave %zu %zu %zu "
                      "against %zu %zu %zu\n",
                      insert_position, cols, position, actual.cursor_row,
                      actual.cursor_col, actual.total_rows, expected.cursor_row,
                      expected.cursor_col, expected.total_rows);
          ok = false;
          break;
        }
      }
    }
  }

  ITL_STRING_FREE(line);

  return ok;
}

static bool
test_bytes_have(const char *data, size_t size, const char *needle)
{
  size_t needle_length = strlen(needle);
  size_t start;

  if (needle_length > size) {
    return false;
  }

  for (start = 0; start + needle_length <= size; ++start) {
    if (memcmp(data + start, needle, needle_length) == 0) {
      return true;
    }
  }

  return false;
}

#if defined ITL_POSIX && !defined NDEBUG
static char   test_frame_capture[8192];
static size_t test_frame_capture_size = 0;

static void
test_frame_capture_sink(const char *data, size_t size)
{
  size_t room = sizeof(test_frame_capture) - test_frame_capture_size;
  size_t taken = size < room ? size : room;

  memcpy(test_frame_capture + test_frame_capture_size, data, taken);
  test_frame_capture_size += taken;
}

static bool
test_frame_capture_has(const char *needle)
{
  return test_bytes_have(test_frame_capture, test_frame_capture_size, needle);
}

static void
test_frame_capture_refresh(itl_le_t *le)
{
  itl_g_tty_first_render = true;
  itl_g_tty_plain_append_pending = false;
  itl_le_invalidate_prev_frame();
  itl_g_tty_should_refresh_text = true;
  test_frame_capture_size = 0;
  itl_le_tty_refresh(le);
}

static size_t test_loading_gather_drain_count;

static bool
test_loading_gather(itl_le_t *le, tl_completion *result)
{
  test_loading_gather_drain_count = itl_g_debug_output_drain_count;
  return test_menu_gather(le, result);
}

static bool
test_loading_frame_drains_before_gather(void)
{
  char out_buffer[BUFFER_SIZE];
  bool did_rebase;
  bool ok;
  int previous_errno;
  int preserved_errno;
  itl_le_t le = ITL_ZERO_INIT;
  itl_menu_source source = ITL_ZERO_INIT;
  itl_menu_filter_state state = ITL_ZERO_INIT;
  tl_completion result = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  source.gather = test_loading_gather;
  source.should_show_loading = true;
  test_menu_gather_calls = 0;
  test_menu_gather_has_rows = true;
  ITL_STRING_FROM_CSTR(line, "al");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");

  itl_g_tty_changed_size = 0;
  itl_g_tty_prev_rows = 24;
  itl_g_tty_prev_cols = 80;
  itl_g_debug_frame_sink = test_frame_capture_sink;
  itl_g_debug_output_drain_count = 0;
  test_loading_gather_drain_count = 0;
  test_frame_capture_size = 0;

  did_rebase = itl_menu_rebase(&le, &source, &state, &result);
  ok = did_rebase && test_menu_gather_calls == 1 &&
       itl_g_debug_output_drain_count == 1 &&
       test_loading_gather_drain_count == 1 &&
       test_frame_capture_has(ITL_MENU_LOADING_TEXT);

  if (!ok) {
    TEST_PRINTF("loading=%d gathers=%zu drains=%zu observed=%zu bytes=%zu\n",
                test_frame_capture_has(ITL_MENU_LOADING_TEXT),
                test_menu_gather_calls, itl_g_debug_output_drain_count,
                test_loading_gather_drain_count, test_frame_capture_size);
  }

  itl_g_debug_frame_sink = NULL;
  previous_errno = errno;
  errno = EDOM;
  itl_terminal_drain_output();
  preserved_errno = errno;
  errno = previous_errno;
  ok = ok && preserved_errno == EDOM;
  itl_g_debug_output_drain_count = 0;
  itl_g_tty_changed_size = 1;
  itl_g_tty_first_render = true;
  ITL_STRING_FREE(line);

  return ok;
}

static bool
test_menu_rows_start_under_the_token(void)
{
  static const char *candidates[] = {"alpha", "album"};
  char               out_buffer[BUFFER_SIZE];
  bool               ok;
  size_t             anchor_column;
  itl_le_t           le = ITL_ZERO_INIT;
  tl_completion      result = ITL_ZERO_INIT;
  itl_menu_layout    layout;
  itl_string_t      *line = itl_string_alloc();

  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "> ");
  ITL_STRING_FROM_CSTR(line, "echo al");
  le.cursor_position = line->length;

  itl_g_tty_changed_size = 0;
  itl_g_tty_prev_rows = 24;
  itl_g_tty_prev_cols = 80;
  itl_g_le_prev_total_rows = 1;
  itl_g_le_prev_cursor_row = 1;
  result.candidates = candidates;
  result.count = countof(candidates);
  result.token_start = 5;
  result.token_end = 7;
  layout = itl_menu_measure(itl_g_tty_prev_rows, false);

  anchor_column = itl_menu_anchor_column_of(&le, result.token_start);
  ok = anchor_column == 7;

  itl_g_debug_frame_sink = test_frame_capture_sink;
  test_frame_capture_size = 0;
  itl_g_menu_anchor_column = anchor_column;
  itl_menu_draw(&result, 0, 0, layout, NULL, NULL, false, 5, "");
  ok = ok && test_frame_capture_has("\x1b[6G");

  itl_g_tty_prev_cols = 12;
  test_frame_capture_size = 0;
  itl_menu_draw(&result, 0, 0, layout, NULL, NULL, false, 5, "");
  ok = ok && !test_frame_capture_has("\x1b[6G");

  if (!ok) {
    TEST_PRINTF("anchor=%zu bytes=%zu\n", anchor_column, test_frame_capture_size);
  }

  itl_g_menu_anchor_column = 0;
  itl_g_debug_frame_sink = NULL;
  itl_g_tty_prev_cols = 80;
  itl_g_tty_changed_size = 1;
  itl_g_tty_first_render = true;
  ITL_STRING_FREE(line);

  return ok;
}

static bool
test_menu_help_wraps_between_items(void)
{
  static const char *candidates[] = {"alpha", "album"};
  static const char  title[] = "selecting completions";
  static const char  keys[] =
      "enter to run, tab to accept, esc to close, ctrl-g to restore";
  tl_completion      result = ITL_ZERO_INIT;
  itl_menu_layout    layout;
  size_t             rows_wide;
  size_t             rows_mid;
  size_t             rows_narrow;
  size_t             rows_cut;
  bool               ok;

  itl_g_tty_changed_size = 0;
  itl_g_tty_prev_rows = 24;
  itl_g_tty_prev_cols = 40;
  itl_g_le_prev_total_rows = 1;
  itl_g_le_prev_cursor_row = 1;
  itl_g_menu_anchor_column = 0;
  result.candidates = candidates;
  result.count = countof(candidates);

  rows_wide = itl_menu_layout_help(NULL, title, keys, 100, 0, 24);
  rows_mid = itl_menu_layout_help(NULL, title, keys, 57, 0, 24);
  rows_narrow = itl_menu_layout_help(NULL, title, keys, 37, 0, 24);
  rows_cut = itl_menu_layout_help(NULL, title, keys, 37, 0, 2);
  ok = rows_wide == 1 && rows_mid == 2 && rows_narrow == 3 && rows_cut == 2;

  layout = itl_menu_measure_for(&result, itl_g_tty_prev_rows, 5, "", title,
                                keys);
  ok = ok && layout.has_help_row && layout.help_row_count == 3 &&
       layout.has_count_row && layout.candidate_rows == ITL_MENU_MAX_ROWS;

  itl_g_debug_frame_sink = test_frame_capture_sink;
  test_frame_capture_size = 0;
  itl_menu_draw(&result, 0, 0, layout, title, keys, false, 5, "");
  ok = ok && test_frame_capture_has("selecting completions, enter to run,") &&
       test_frame_capture_has("tab to accept, esc to close,") &&
       test_frame_capture_has("ctrl-g to restore") &&
       !test_frame_capture_has("run, tab");

  /* A tall block leaves no room, so the help text is dropped whole. */
  itl_g_le_prev_total_rows = 21;
  layout = itl_menu_measure_for(&result, itl_g_tty_prev_rows, 5, "", title,
                                keys);
  ok = ok && !layout.has_help_row && layout.help_row_count == 0;

  /* One item wider than the row is cut with an ellipsis. */
  itl_g_le_prev_total_rows = 1;
  itl_g_tty_prev_cols = 16;
  layout = itl_menu_measure_for(&result, itl_g_tty_prev_rows, 5, "",
                                "a title longer than the row", NULL);
  test_frame_capture_size = 0;
  itl_menu_draw(&result, 0, 0, layout, "a title longer than the row", NULL,
                false, 5, "");
  ok = ok && layout.help_row_count == 1 && test_frame_capture_has("...") &&
       !test_frame_capture_has("than the row");

  if (!ok) {
    TEST_PRINTF("rows wide=%zu mid=%zu narrow=%zu cut=%zu help=%zu\n",
                rows_wide, rows_mid, rows_narrow, rows_cut,
                layout.help_row_count);
  }

  itl_g_debug_frame_sink = NULL;
  itl_g_tty_prev_cols = 80;
  itl_g_le_prev_total_rows = 1;
  itl_g_tty_changed_size = 1;
  itl_g_tty_first_render = true;

  return ok;
}

static bool
test_menu_anchor_accounts_for_descriptions(void)
{
  static const char *candidates[] = {"-a", "-all"};
  static const char *short_descriptions[] = {"all", "everything"};
  static const char *long_descriptions[] = {
      "do not ignore entries starting with a dot",
      "list every entry including the implied dot entries"};
  tl_completion      result = ITL_ZERO_INIT;
  itl_menu_layout    layout;
  bool               ok;

  itl_g_tty_changed_size = 0;
  itl_g_tty_prev_rows = 24;
  itl_g_tty_prev_cols = 60;
  itl_g_le_prev_total_rows = 1;
  itl_g_le_prev_cursor_row = 1;
  result.candidates = candidates;
  result.count = countof(candidates);
  result.descriptions = short_descriptions;
  layout = itl_menu_measure(itl_g_tty_prev_rows, 0);

  itl_g_debug_frame_sink = test_frame_capture_sink;
  itl_g_menu_anchor_column = 30;

  /* Names and descriptions fit to the right of the token. */
  test_frame_capture_size = 0;
  itl_menu_draw(&result, 0, 0, layout, NULL, NULL, false, 4, "");
  ok = test_frame_capture_has("\x1b[29G");

  /* The names alone would fit, the descriptions do not. */
  result.descriptions = long_descriptions;
  test_frame_capture_size = 0;
  itl_menu_draw(&result, 0, 0, layout, NULL, NULL, false, 4, "");
  ok = ok && !test_frame_capture_has("\x1b[29G");

  /* Without descriptions the same names stay under the token. */
  result.descriptions = NULL;
  test_frame_capture_size = 0;
  itl_menu_draw(&result, 0, 0, layout, NULL, NULL, false, 4, "");
  ok = ok && test_frame_capture_has("\x1b[29G");

  /* The names fit, a help item would be cut beside them. */
  layout = itl_menu_measure_for(&result, itl_g_tty_prev_rows, 4, "",
                                "selecting completions",
                                "a help item far too wide for that row");
  test_frame_capture_size = 0;
  itl_menu_draw(&result, 0, 0, layout, "selecting completions",
                "a help item far too wide for that row", false, 4, "");
  ok = ok && !test_frame_capture_has("\x1b[29G") &&
       test_frame_capture_has("a help item far too wide for that row");

  if (!ok) {
    TEST_PRINTF("bytes=%zu\n", test_frame_capture_size);
  }

  itl_g_menu_anchor_column = 0;
  itl_g_debug_frame_sink = NULL;
  itl_g_tty_prev_cols = 80;
  itl_g_tty_changed_size = 1;
  itl_g_tty_first_render = true;

  return ok;
}

static bool
test_colors_disabled_drop_span_escapes(void)
{
  char out_buffer[BUFFER_SIZE];
  bool was_colored_seen;
  bool is_colored_seen;
  bool is_text_seen;
  bool ok;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "> ");
  ITL_STRING_FROM_CSTR(line, "abc");
  le.cursor_position = line->length;

  itl_g_tty_changed_size = 0;
  itl_g_tty_prev_rows = 24;
  itl_g_tty_prev_cols = 80;
  itl_g_debug_frame_sink = test_frame_capture_sink;
  tl_set_highlight_callback(test_whole_line_highlight_callback);

  tl_set_colors_enabled(1);
  test_frame_capture_refresh(&le);
  was_colored_seen = test_frame_capture_has("\x1b[32m");

  tl_set_colors_enabled(0);
  test_frame_capture_refresh(&le);
  is_colored_seen = test_frame_capture_has("\x1b[32m");
  is_text_seen = test_frame_capture_has("abc");

  itl_g_debug_frame_sink = NULL;
  tl_set_highlight_callback(NULL);
  tl_set_colors_enabled(1);
  itl_g_tty_changed_size = 1;
  itl_g_tty_first_render = true;
  ITL_STRING_FREE(line);

  ok = was_colored_seen && !is_colored_seen && is_text_seen;

  if (!ok) {
    TEST_PRINTF("colored %d, still colored %d, text %d\n",
                (int) was_colored_seen, (int) is_colored_seen,
                (int) is_text_seen);
  }

  return ok;
}

static bool
test_submit_erases_the_drawn_ghost(void)
{
  static const char GHOST[] = "hoing";
  char              out_buffer[BUFFER_SIZE];
  bool              was_ghost_drawn;
  bool              was_ghost_recorded;
  bool              was_ghost_erased;
  bool              is_ghost_cleared;
  bool              was_quiet_without_ghost;
  bool              ok;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "> ");
  ITL_STRING_FROM_CSTR(line, "ec");
  le.cursor_position = line->length;

  itl_g_tty_changed_size = 0;
  itl_g_tty_prev_rows = 24;
  itl_g_tty_prev_cols = 80;
  itl_g_debug_frame_sink = test_frame_capture_sink;

  memcpy(itl_g_ghost, GHOST, sizeof(GHOST));
  itl_g_ghost_len = sizeof(GHOST) - 1;
  itl_g_ghost_width = sizeof(GHOST) - 1;

  test_frame_capture_refresh(&le);
  was_ghost_drawn = test_frame_capture_has(GHOST);
  was_ghost_recorded = itl_g_le_prev_ghost_len == sizeof(GHOST) - 1;

  test_frame_capture_size = 0;
  itl_le_finish_input(&le, TL_PRESSED_ENTER);
  was_ghost_erased = test_frame_capture_size > 0 && !test_frame_capture_has(GHOST);
  is_ghost_cleared = itl_g_ghost_len == 0;

  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "> ");
  ITL_STRING_FROM_CSTR(line, "ec");
  le.cursor_position = line->length;

  test_frame_capture_refresh(&le);
  test_frame_capture_size = 0;
  itl_le_finish_input(&le, TL_PRESSED_ENTER);
  was_quiet_without_ghost = test_frame_capture_size == 0;

  itl_g_debug_frame_sink = NULL;
  itl_g_tty_changed_size = 1;
  itl_g_tty_first_render = true;
  ITL_STRING_FREE(line);

  ok = was_ghost_drawn && was_ghost_recorded && was_ghost_erased &&
       is_ghost_cleared && was_quiet_without_ghost;

  if (!ok) {
    TEST_PRINTF("drawn %d, recorded %d, erased %d, cleared %d, quiet %d\n",
                (int) was_ghost_drawn, (int) was_ghost_recorded,
                (int) was_ghost_erased, (int) is_ghost_cleared,
                (int) was_quiet_without_ghost);
  }

  return ok;
}
#endif

static bool
test_csi_sequences(void)
{
  static const char *const expected[] = {"\x1b[1G", "\x1b[12C", "\x1b[7A",
                                         "\x1b[4096B"};
  itl_char_buf_t          *b = itl_char_buf_alloc();
  size_t                   index;
  bool                     ok = true;

  ITL_TTY_MOVE_TO_COLUMN(b, 1);
  ITL_TTY_MOVE_FORWARD(b, 12);
  ITL_TTY_MOVE_UP(b, 7);
  ITL_TTY_MOVE_DOWN(b, 4096);

  for (index = 0; index < countof(expected); ++index) {
    if (!test_bytes_have(b->data, b->size, expected[index])) {
      TEST_PRINTF("%s is missing from %zu bytes\n", expected[index] + 1,
                  b->size);
      ok = false;
    }
  }

  if (ok && b->size != 4 + 5 + 4 + 7) {
    TEST_PRINTF("four sequences wrote %zu bytes\n", b->size);
    ok = false;
  }

  ITL_CHAR_BUF_FREE(b);

  return ok;
}

static bool
test_menu_band_survives_disabled_colors(void)
{
  static const char *const names[] = {"alpha"};
  static const char *const descriptions[] = {"the first letter"};
  tl_completion            result = ITL_ZERO_INIT;
  int                      was_colors_enabled = itl_g_colors_enabled;
  itl_char_buf_t          *b = itl_char_buf_alloc();
  bool                     is_band_seen;
  bool                     is_description_seen;
  bool                     ok;

  result.candidates = names;
  result.descriptions = descriptions;
  result.count = 1;

  itl_g_colors_enabled = 0;
  itl_menu_append_row(b, &result, 0, 8, 20, true, true);
  is_band_seen = test_bytes_have(b->data, b->size, ITL_MENU_SELECTED_SGR);

  ITL_CHAR_BUF_CLEAR(b);
  itl_menu_append_row(b, &result, 0, 8, 20, false, true);
  is_description_seen =
      test_bytes_have(b->data, b->size, ITL_MENU_DESCRIPTION_SGR);

  itl_g_colors_enabled = was_colors_enabled;
  ITL_CHAR_BUF_FREE(b);

  ok = is_band_seen && !is_description_seen;

  if (!ok) {
    TEST_PRINTF("band %d, description %d\n", (int) is_band_seen,
                (int) is_description_seen);
  }

  return ok;
}

static const char *test_hint_text = "";

static const char *
test_hint_callback(const char *buffer, size_t cursor, const char **sgr)
{
  (void) buffer;
  (void) cursor;
  (void) sgr;

  return test_hint_text;
}

static const char *
test_hint_cursor_callback(const char *buffer, size_t cursor, const char **sgr)
{
  (void) sgr;

  return cursor == strlen(buffer) ? "hint at end" : "hint in middle";
}

static bool
test_hint_row_is_cut_to_the_width(void)
{
  static const char CJK[] = "\xE4\xBD\xA0\xE5\xA5\xBD\xE4\xBD\xA0\xE5\xA5\xBD"
                            "\xE4\xBD\xA0\xE5\xA5\xBD\xE4\xBD\xA0\xE5\xA5\xBD";
  int               was_colors_enabled = itl_g_colors_enabled;
  bool              is_ascii_cut;
  bool              is_wide_cut;
  bool              is_short_kept;
  bool              is_tiny_dropped;
  bool              is_header_split;
  bool              ok;

  tl_set_colors_enabled(0);
  tl_set_hint_callback(test_hint_callback);

  test_hint_text = "usage: a very long synopsis that cannot fit the row";
  itl_hint_compose("x", 1, 20, 1);
  is_ascii_cut = strcmp(itl_g_hint_next, "  usage: a very...") == 0 &&
                 itl_g_hint_next_rows == 1;

  test_hint_text = CJK;
  itl_hint_compose("x", 1, 10, 1);
  is_wide_cut = itl_g_hint_next_len > 5 &&
                strncmp(itl_g_hint_next, "  ", 2) == 0 &&
                strcmp(itl_g_hint_next + itl_g_hint_next_len - 3, "...") == 0 &&
                itl_cstr_display_width(itl_g_hint_next) <= 9;

  test_hint_text = "short";
  itl_hint_compose("x", 1, 20, 24);
  is_short_kept = strcmp(itl_g_hint_next, "  short") == 0 &&
                  itl_g_hint_next_rows == 1;

  itl_hint_compose("x", 1, 7, 24);
  is_tiny_dropped = itl_g_hint_next_len == 0 && itl_g_hint_next_rows == 0;

  test_hint_text = "first\nsecond";
  itl_hint_compose("x", 1, 20, 24);
  is_header_split = strcmp(itl_g_hint_next, "  first\n  second") == 0 &&
                    itl_g_hint_next_rows == 2;

  tl_set_hint_callback(NULL);
  tl_set_colors_enabled(was_colors_enabled);
  itl_hint_drop_next();

  ok = is_ascii_cut && is_wide_cut && is_short_kept && is_tiny_dropped &&
       is_header_split;

  if (!ok) {
    TEST_PRINTF("ascii %d, wide %d, short %d, tiny %d, header %d\n",
                (int) is_ascii_cut, (int) is_wide_cut, (int) is_short_kept,
                (int) is_tiny_dropped, (int) is_header_split);
  }

  return ok;
}

static bool
test_hint_rows_are(const char *expected, size_t cols, size_t row_budget,
                   const char *step)
{
  itl_hint_compose("x", 1, cols, row_budget);
  if (itl_g_hint_next_len != strlen(expected) ||
      (itl_g_hint_next_len > 0 && strcmp(itl_g_hint_next, expected) != 0))
  {
    TEST_PRINTF("%s: hint '%s', want '%s'\n", step, itl_g_hint_next, expected);
    return false;
  }
  return true;
}

static bool
test_hint_body_wraps_at_words(void)
{
  static const char WRAPPED[] =
      "builtin synopsis\ncd [-L|-P] [-e] [dir] and some more words here";
  int  was_colors_enabled = itl_g_colors_enabled;
  bool ok = true;

  tl_set_colors_enabled(0);
  tl_set_hint_callback(test_hint_callback);

  test_hint_text = WRAPPED;
  ok &= test_hint_rows_are("  builtin synopsis\n  cd [-L|-P] [-e]\n"
                           "  [dir] and some\n  more words here",
                           21, 24, "word wrap");
  ok &= itl_g_hint_next_rows == 4;

  test_hint_text = "builtin synopsis\ncd [-L|-P] [-e] [dir] and some more "
                   "words here and beyond";
  ok &= test_hint_rows_are("  builtin synopsis\n  cd [-L|-P] [-e]\n"
                           "  [dir] and some\n  more words here...",
                           21, 24, "row cap");

  test_hint_text = "header\nabcdefghijklmnopqrstuvwxyz0123 tail";
  ok &= test_hint_rows_are("  header\n  abcdefghij\n  klmnopqrst\n"
                           "  uvwxyz0...",
                           13, 24, "long word");

  test_hint_text = "a header much wider than the row\nbody";
  ok &= test_hint_rows_are("  a header muc...\n  body", 18, 24, "header cut");

  test_hint_text = "hdr\x01\nab\x1b" "cd\xC2\x9B" "ef\x7f\ngh ij";
  ok &= test_hint_rows_are("  hdr\n  ab cd\n  ef  gh\n  ij", 9, 24,
                           "control bytes");

  tl_set_hint_callback(NULL);
  tl_set_colors_enabled(was_colors_enabled);
  itl_hint_drop_next();

  return ok;
}

static bool
test_hint_rows_fit_a_short_terminal(void)
{
  int  was_colors_enabled = itl_g_colors_enabled;
  bool ok = true;

  tl_set_colors_enabled(0);
  tl_set_hint_callback(test_hint_callback);

  test_hint_text =
      "builtin synopsis\ncd [-L|-P] [-e] [dir] and some more words here";
  ok &= test_hint_rows_are("  builtin synopsis\n  cd [-L|-P] [-e]\n"
                           "  [dir] and some...",
                           21, 3, "three rows");
  ok &= test_hint_rows_are("  builtin synopsis\n  cd [-L|-P] [-e]...", 21, 2,
                           "two rows");
  ok &= test_hint_rows_are("  cd [-L|-P] [-e]...", 21, 1, "body alone");
  ok &= test_hint_rows_are("", 21, 0, "no room");

  test_hint_text = "pressed x\n";
  ok &= test_hint_rows_are("  pressed x", 21, 1, "header alone");

  tl_set_hint_callback(NULL);
  tl_set_colors_enabled(was_colors_enabled);
  itl_hint_drop_next();

  return ok;
}

static bool
test_hint_rows_erase_when_shrinking(void)
{
  int             was_colors_enabled = itl_g_colors_enabled;
  itl_char_buf_t *b = itl_char_buf_alloc();
  bool            was_three_drawn;
  bool            was_shrunk_to_one;
  bool            was_shrunk_to_none;
  bool            was_relaid_out;
  bool            ok;

  tl_set_colors_enabled(0);
  tl_set_hint_callback(test_hint_callback);
  itl_hint_forget_shown();

  test_hint_text = "kind\nalpha beta gamma delta";
  itl_hint_compose("x", 1, 16, 24);
  itl_le_tty_draw_hint(b, 24);
  was_three_drawn = itl_g_hint_shown_rows == 3 &&
                    test_bytes_have(b->data, b->size, "  alpha beta") &&
                    test_bytes_have(b->data, b->size, "  gamma delta") &&
                    test_bytes_have(b->data, b->size, "\x1b[3A");

  b->size = 0;
  test_hint_text = "one";
  itl_hint_compose("x", 1, 16, 24);
  itl_le_tty_draw_hint(b, 24);
  was_shrunk_to_one =
      itl_g_hint_shown_rows == 1 &&
      test_bytes_have(b->data, b->size,
                      ITL_LF "\x1b[1G  one\x1b[K" ITL_LF "\x1b[1G\r\x1b[0K"
                      ITL_LF "\x1b[1G\r\x1b[0K\x1b[3A");

  b->size = 0;
  test_hint_text = "";
  itl_hint_compose("x", 1, 16, 24);
  itl_le_tty_draw_hint(b, 24);
  was_shrunk_to_none =
      itl_g_hint_shown_rows == 0 && itl_g_hint_shown_len == 0 &&
      test_bytes_have(b->data, b->size, ITL_LF "\x1b[1G\r\x1b[0K\x1b[1A");

  b->size = 0;
  test_hint_text = "kind\nalpha beta gamma delta";
  itl_hint_compose("x", 1, 16, 24);
  itl_le_tty_draw_hint(b, 2);
  was_relaid_out = itl_g_hint_shown_rows == 2 &&
                   test_bytes_have(b->data, b->size, "  alpha beta...");

  itl_hint_forget_shown();
  tl_set_hint_callback(NULL);
  tl_set_colors_enabled(was_colors_enabled);
  itl_hint_drop_next();
  ITL_CHAR_BUF_FREE(b);

  ok = was_three_drawn && was_shrunk_to_one && was_shrunk_to_none &&
       was_relaid_out;

  if (!ok) {
    TEST_PRINTF("three %d, one %d, none %d, relaid %d\n",
                (int) was_three_drawn, (int) was_shrunk_to_one,
                (int) was_shrunk_to_none, (int) was_relaid_out);
  }

  return ok;
}

#if defined ITL_POSIX && !defined NDEBUG
static void
test_hint_prepare_frame(itl_le_t *le, itl_string_t *line, char *out_buffer,
                        size_t out_size, const char *text)
{
  itl_le_init(le, line, out_buffer, out_size, "> ");
  ITL_STRING_FROM_CSTR(line, "ec");
  le->cursor_position = line->length;

  itl_g_tty_changed_size = 0;
  itl_g_tty_prev_rows = 24;
  itl_g_tty_prev_cols = 80;
  itl_g_debug_frame_sink = test_frame_capture_sink;
  itl_g_hint_is_closed = false;
  itl_g_hint_hold_count = 0;
  test_hint_text = text;
  tl_set_hint_callback(test_hint_callback);
}

static void
test_hint_finish_frame(itl_string_t *line)
{
  itl_g_debug_frame_sink = NULL;
  tl_set_hint_callback(NULL);
  itl_g_hint_is_closed = false;
  itl_g_hint_hold_count = 0;
  itl_hint_forget_shown();
  itl_hint_drop_next();
  itl_g_tty_changed_size = 1;
  itl_g_tty_first_render = true;
  ITL_STRING_FREE(line);
}

static bool
test_hint_row_draws_holds_and_erases(void)
{
  static const char HINT[] = "usage: ec [-n] string";
  char              out_buffer[BUFFER_SIZE];
  bool              was_hint_drawn;
  bool              was_block_unchanged;
  bool              is_cursor_move_quiet;
  bool              was_hint_held_away;
  bool              was_hint_back;
  bool              was_hint_erased;
  bool              ok;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  test_hint_prepare_frame(&le, line, out_buffer, sizeof(out_buffer), HINT);

  test_frame_capture_refresh(&le);
  was_hint_drawn = test_frame_capture_has(HINT) && itl_g_hint_shown_len > 0;
  was_block_unchanged = itl_g_le_prev_total_rows == 1;

  test_frame_capture_size = 0;
  itl_g_tty_should_refresh_text = false;
  le.cursor_position = 1;
  itl_le_tty_refresh(&le);
  is_cursor_move_quiet = !test_frame_capture_has(HINT);

  le.cursor_position = line->length;
  itl_g_tty_should_refresh_text = true;
  test_frame_capture_size = 0;
  itl_hint_hold(&le);
  was_hint_held_away = !test_frame_capture_has(HINT) &&
                       test_frame_capture_size > 0 && itl_g_hint_shown_len == 0;

  itl_g_tty_should_refresh_text = true;
  test_frame_capture_size = 0;
  itl_le_tty_refresh(&le);
  was_hint_held_away = was_hint_held_away && !test_frame_capture_has(HINT);

  itl_hint_release();
  itl_g_tty_should_refresh_text = true;
  test_frame_capture_size = 0;
  itl_le_tty_refresh(&le);
  was_hint_back = test_frame_capture_has(HINT);

  test_frame_capture_size = 0;
  itl_le_finish_input(&le, TL_PRESSED_ENTER);
  was_hint_erased = test_frame_capture_size > 0 &&
                    !test_frame_capture_has(HINT) && itl_g_hint_shown_len == 0 &&
                    itl_g_hint_is_closed;

  test_hint_finish_frame(line);

  ok = was_hint_drawn && was_block_unchanged && is_cursor_move_quiet &&
       was_hint_held_away && was_hint_back && was_hint_erased;

  if (!ok) {
    TEST_PRINTF("drawn %d, block %d, cursor quiet %d, held %d, back %d, "
                "erased %d\n",
                (int) was_hint_drawn, (int) was_block_unchanged,
                (int) is_cursor_move_quiet, (int) was_hint_held_away,
                (int) was_hint_back, (int) was_hint_erased);
  }

  return ok;
}

static size_t
test_frame_capture_count(const char *needle)
{
  size_t needle_length = strlen(needle);
  size_t start;
  size_t found_count = 0;

  for (start = 0; start + needle_length <= test_frame_capture_size; ++start) {
    if (memcmp(test_frame_capture + start, needle, needle_length) == 0) {
      found_count += 1;
    }
  }

  return found_count;
}

static bool
test_hint_rows_follow_the_frame(void)
{
  static const char HINT[] =
      "builtin synopsis\nec [-n] [-e] [-E] [--first-long-option] "
      "[--second-long-option] [--third-long-option] [string ...]";
  char out_buffer[BUFFER_SIZE];
  int  was_colors_enabled = itl_g_colors_enabled;
  bool was_wrapped;
  bool was_held_back;
  bool was_erased;
  bool ok;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  test_hint_prepare_frame(&le, line, out_buffer, sizeof(out_buffer), HINT);
  tl_set_colors_enabled(0);

  test_frame_capture_refresh(&le);
  was_wrapped = itl_g_hint_shown_rows == 3 &&
                test_frame_capture_has("  builtin synopsis") &&
                test_frame_capture_has("  [--third-long-option]") &&
                test_frame_capture_has("\x1b[3A");

  itl_g_tty_prev_rows = 3;
  itl_g_tty_should_refresh_text = true;
  test_frame_capture_size = 0;
  itl_le_tty_refresh(&le);
  was_held_back = itl_g_hint_shown_rows == 2 &&
                  test_frame_capture_count("\r\x1b[0K") >= 4 &&
                  test_frame_capture_has("...");
  itl_g_tty_prev_rows = 24;

  test_frame_capture_size = 0;
  itl_le_finish_input(&le, TL_PRESSED_ENTER);
  was_erased = itl_g_hint_shown_rows == 0 &&
               test_frame_capture_count("\r\x1b[0K") >= 3 &&
               !test_frame_capture_has("synopsis");

  tl_set_colors_enabled(was_colors_enabled);
  test_hint_finish_frame(line);

  ok = was_wrapped && was_held_back && was_erased;

  if (!ok) {
    TEST_PRINTF("wrapped %d, held back %d, erased %d\n", (int) was_wrapped,
                (int) was_held_back, (int) was_erased);
  }

  return ok;
}

static bool
test_hint_is(const char *expected, size_t cols, const char *step)
{
  return test_hint_rows_are(expected, cols, 24, step);
}

static void
test_prefix_reset(void)
{
  itl_g_prefix_kind = ITL_PREFIX_NONE;
  itl_g_prefix_key = 0;
  itl_g_vi_pending_operator = ITL_VI_OP_NONE;
  itl_g_vi_pending_count = 0;
  itl_g_vi_pending_register = 0;
  itl_g_edit_mode = TL_EDIT_MODE_EMACS;
}

static bool
test_prefix_hint_names_the_waiting_keys(void)
{
  int  was_colors_enabled = itl_g_colors_enabled;
  bool ok = true;

  tl_set_colors_enabled(0);
  tl_set_hint_callback(test_hint_callback);
  test_hint_text = "usage";
  test_prefix_reset();

  ok &= test_hint_is("  usage", 200, "no prefix");

  itl_g_prefix_kind = ITL_PREFIX_CTRL_X;
  ok &= test_hint_is("  pressed ctrl-x\n  waiting for ctrl-e (edit in "
                     "$VISUAL), ctrl-u (undo)",
                     200, "ctrl-x");
  ok &= test_hint_is("  pressed ctrl-x\n  waiting for ctrl-e (edit\n"
                     "  in $VISUAL), ctrl-u\n  (undo)",
                     27, "ctrl-x wrap");
  tl_set_hint_callback(NULL);
  ok &= test_hint_is("", 200, "ctrl-x with the row off");
  tl_set_hint_callback(test_hint_callback);
  test_prefix_reset();

  itl_g_vi_pending_operator = ITL_VI_OP_DELETE;
  ok &= test_hint_is("  usage", 200, "vi state outside normal mode");

  itl_g_edit_mode = TL_EDIT_MODE_VI_COMMAND;
  itl_g_vi_pending_count = 2;
  itl_g_vi_pending_register = 'a';
  ok &= test_hint_is("  pressed \"a2d\n  waiting for a motion: w, b, e, $, 0, "
                     "^, d (line), f, t, F, T, h, l, j, k, W, B, E, ;, ,",
                     200, "operator");

  itl_g_prefix_kind = ITL_PREFIX_VI_FIND;
  itl_g_prefix_key = 't';
  ok &= test_hint_is("  pressed \"a2dt\n  waiting for a character to stop "
                     "before",
                     200, "operator find");
  test_prefix_reset();

  itl_g_edit_mode = TL_EDIT_MODE_VI_COMMAND;
  itl_g_vi_pending_operator = ITL_VI_OP_CHANGE;
  ok &= test_hint_is("  pressed c\n  waiting for a motion: w, b, e, $, 0, ^, "
                     "c (line), f, t, F, T, h, l, j, k, W, B, E, ;, ,",
                     200, "change");
  itl_g_vi_pending_operator = ITL_VI_OP_YANK;
  ok &= test_hint_is("  pressed y\n  waiting for a\n  motion: w, b, e, $,\n"
                     "  0, ^, y (line),...",
                     22, "yank cut");
  itl_g_vi_pending_operator = ITL_VI_OP_NONE;

  itl_g_vi_pending_count = 3;
  ok &= test_hint_is("  pressed 3\n  waiting for a command or a motion", 200,
                     "count");
  itl_g_vi_pending_count = 0;

  itl_g_vi_pending_register = 'b';
  ok &= test_hint_is("  pressed \"b\n  waiting for a command: d, c, y, p, P, "
                     "x, X, D, C, s, S",
                     200, "register");
  itl_g_vi_pending_register = 0;

  itl_g_prefix_kind = ITL_PREFIX_VI_REGISTER;
  ok &= test_hint_is("  pressed \"\n  waiting for a register name: a-z", 200,
                     "register name");
  itl_g_prefix_kind = ITL_PREFIX_VI_REPLACE;
  ok &= test_hint_is("  pressed r\n  waiting for a replacement character", 200,
                     "replace");
  itl_g_prefix_kind = ITL_PREFIX_VI_EX;
  ok &= test_hint_is("  pressed :\n  waiting for q, q!, quit, wq, wq!, or x "
                     "(quit), then enter",
                     200, "ex");

  itl_g_edit_mode = TL_EDIT_MODE_VI_VISUAL;
  itl_g_prefix_kind = ITL_PREFIX_VI_FIND;
  itl_g_prefix_key = 'F';
  ok &= test_hint_is("  pressed F\n  waiting for a character to find backward",
                     200, "visual find");
  itl_g_prefix_key = 'T';
  ok &= test_hint_is("  pressed T\n  waiting for a character to stop after",
                     200, "visual till");
  itl_g_prefix_key = 'f';
  ok &= test_hint_is("  pressed f\n  waiting for a character to find", 200,
                     "visual find forward");

  test_prefix_reset();
  ok &= test_hint_is("  usage", 200, "closed prefix");

  tl_set_hint_callback(NULL);
  tl_set_colors_enabled(was_colors_enabled);
  itl_hint_drop_next();

  return ok;
}

static int test_chord_writer = -1;
static bool test_chord_hint_was_drawn = false;

static int
test_chord_idle_callback(const char *buffer, size_t cursor)
{
  static const char CTRL_U = 21;

  (void) buffer;
  (void) cursor;
  test_chord_hint_was_drawn =
      test_frame_capture_has("  pressed ctrl-x") &&
      test_frame_capture_has("  waiting for ctrl-e") &&
      !test_frame_capture_has("usage") &&
      itl_g_prefix_kind == ITL_PREFIX_CTRL_X;
  if (write(test_chord_writer, &CTRL_U, 1) != 1) {
    return 0;
  }
  return 0;
}

static bool
test_ctrl_x_chord_shows_and_drops_its_hint(void)
{
  char          out_buffer[BUFFER_SIZE];
  int           pipe_descriptors[2] = {-1, -1};
  int           saved_stdin = -1;
  bool          did_wait = false;
  bool          was_resolved = false;
  bool          was_hint_dropped = false;
  bool          was_pending_quiet = false;
  bool          was_unbound_pushed = false;
  bool          ok;
  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  test_hint_prepare_frame(&le, line, out_buffer, sizeof(out_buffer), "usage");
  test_prefix_reset();
  test_chord_hint_was_drawn = false;

  if (pipe(pipe_descriptors) != 0) goto cleanup;
  saved_stdin = dup(STDIN_FILENO);
  if (saved_stdin < 0 || dup2(pipe_descriptors[0], STDIN_FILENO) < 0)
    goto cleanup;

  test_chord_writer = pipe_descriptors[1];
  tl_set_idle_callback(test_chord_idle_callback, 10, 10);
  test_frame_capture_size = 0;
  did_wait = itl_le_await_chord(&le, ITL_PREFIX_CTRL_X, 24);
  was_resolved = itl_g_prefix_kind == ITL_PREFIX_NONE &&
                 itl_esc_parse(24) == TL_KEY_UNDO;
  tl_set_idle_callback(NULL, 0, 0);

  test_frame_capture_size = 0;
  itl_g_tty_should_refresh_text = true;
  itl_le_tty_refresh(&le);
  was_hint_dropped = test_frame_capture_has("usage") &&
                     !test_frame_capture_has("pressed ctrl-x");

  if (write(pipe_descriptors[1], "q", 1) != 1) goto cleanup;
  test_frame_capture_size = 0;
  was_pending_quiet = itl_le_await_chord(&le, ITL_PREFIX_CTRL_X, 24) &&
                      test_frame_capture_size == 0;
  was_unbound_pushed =
      itl_esc_parse(24) == TL_KEY_UNKN && itl_g_pushback_byte == 'q';
  itl_g_pushback_byte = -1;

cleanup:
  tl_set_idle_callback(NULL, 0, 0);
  test_chord_writer = -1;
  if (saved_stdin >= 0) {
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdin);
  }
  if (pipe_descriptors[0] >= 0) close(pipe_descriptors[0]);
  if (pipe_descriptors[1] >= 0) close(pipe_descriptors[1]);
  test_prefix_reset();
  test_hint_finish_frame(line);

  ok = did_wait && test_chord_hint_was_drawn && was_resolved &&
       was_hint_dropped && was_pending_quiet && was_unbound_pushed;
  if (!ok) {
    TEST_PRINTF("waited %d, drawn %d, resolved %d, dropped %d, quiet %d, "
                "pushed %d\n",
                (int) did_wait, (int) test_chord_hint_was_drawn,
                (int) was_resolved, (int) was_hint_dropped,
                (int) was_pending_quiet, (int) was_unbound_pushed);
  }

  return ok;
}

static bool
test_hint_row_follows_the_caret_and_clears(void)
{
  char out_buffer[BUFFER_SIZE];
  bool was_end_hint_drawn;
  bool was_middle_hint_drawn;
  bool was_row_cleared;
  bool ok;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  test_hint_prepare_frame(&le, line, out_buffer, sizeof(out_buffer), "");
  tl_set_hint_callback(test_hint_cursor_callback);

  test_frame_capture_refresh(&le);
  was_end_hint_drawn = test_frame_capture_has("hint at end");

  test_frame_capture_size = 0;
  itl_g_tty_should_refresh_text = false;
  le.cursor_position = 0;
  itl_le_tty_refresh(&le);
  was_middle_hint_drawn = test_frame_capture_has("hint in middle") &&
                          !test_frame_capture_has("hint at end");

  tl_set_hint_callback(test_hint_callback);
  test_hint_text = "";
  test_frame_capture_size = 0;
  itl_g_tty_should_refresh_text = false;
  le.cursor_position = line->length;
  itl_le_tty_refresh(&le);
  was_row_cleared = test_frame_capture_size > 0 &&
                    !test_frame_capture_has("hint") &&
                    itl_g_hint_shown_len == 0;

  test_hint_finish_frame(line);

  ok = was_end_hint_drawn && was_middle_hint_drawn && was_row_cleared;

  if (!ok) {
    TEST_PRINTF("end %d, middle %d, cleared %d\n", (int) was_end_hint_drawn,
                (int) was_middle_hint_drawn, (int) was_row_cleared);
  }

  return ok;
}

static bool
test_hint_row_keeps_multiline_rows_and_append_path(void)
{
  const char *keys = "abcd";
  char        out_buffer[BUFFER_SIZE];
  size_t      key_index;
  bool        was_multiline_counted;
  bool        was_last_hint_drawn;
  bool        ok;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  test_hint_prepare_frame(&le, line, out_buffer, sizeof(out_buffer), "tip");
  ITL_STRING_FROM_CSTR(line, "a\nb");
  le.cursor_position = line->length;

  test_frame_capture_refresh(&le);
  was_multiline_counted = itl_g_le_prev_total_rows == 2 &&
                          test_frame_capture_has("tip");

  itl_string_clear(line);
  le.cursor_position = 0;
  test_frame_capture_refresh(&le);

  itl_g_debug_append_refresh_count = 0;
  itl_g_debug_full_refresh_count = 0;
  for (key_index = 0; keys[key_index] != '\0'; ++key_index) {
    static const char *const texts[] = {"tip-a", "tip-b", "tip-c", "tip-d"};
    itl_utf8_t               appended_character =
        itl_utf8_parse((uint8_t) keys[key_index]);

    test_hint_text = texts[key_index];
    itl_g_tty_plain_append_pending = le.cursor_position == le.line->length;
    itl_g_tty_plain_append_width = itl_char_width(appended_character);
    itl_le_insert(&le, appended_character);
    itl_g_tty_should_refresh_text = true;
    test_frame_capture_size = 0;
    itl_le_tty_refresh(&le);
  }
  was_last_hint_drawn = test_frame_capture_has("tip-d") &&
                        !test_frame_capture_has("tip-c");

  ok = was_multiline_counted && was_last_hint_drawn &&
       itl_g_debug_append_refresh_count == 4;

  test_hint_finish_frame(line);

  if (!ok) {
    TEST_PRINTF("multiline %d, last hint %d, appends %zu\n",
                (int) was_multiline_counted, (int) was_last_hint_drawn,
                itl_g_debug_append_refresh_count);
  }

  return ok;
}

static int         test_idle_outcome = 0;
static int         test_idle_call_count = 0;
static char        test_idle_line[BUFFER_SIZE];
static size_t      test_idle_cursor = 0;
static const char *test_idle_next_hint = NULL;

static int
test_idle_callback(const char *buffer, size_t cursor)
{
  test_idle_call_count += 1;
  snprintf(test_idle_line, sizeof(test_idle_line), "%s", buffer);
  test_idle_cursor = cursor;
  if (test_idle_next_hint != NULL) {
    test_hint_text = test_idle_next_hint;
  }
  return test_idle_outcome;
}

static bool
test_idle_hook_refreshes_the_hint_row(void)
{
  char out_buffer[BUFFER_SIZE];
  bool is_due_at_once;
  bool did_receive_line;
  bool was_new_hint_drawn;
  bool is_settled_after_refresh;
  bool is_unchanged_row_quiet;
  bool is_repeat_due;
  bool is_plain_answer_silent;
  bool ok;
  int  repeat_wait_ms;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  test_hint_prepare_frame(&le, line, out_buffer, sizeof(out_buffer),
                          "hint before");
  test_frame_capture_refresh(&le);

  test_idle_call_count = 0;
  test_idle_line[0] = '\0';
  tl_set_idle_callback(test_idle_callback, 0, 50);
  itl_idle_arm();
  is_due_at_once = itl_idle_wait_ms() == 0;

  test_idle_outcome = TL_IDLE_REFRESH;
  test_idle_next_hint = "hint after";
  test_frame_capture_size = 0;
  itl_idle_run(&le);
  did_receive_line = test_idle_call_count == 1 &&
                     strcmp(test_idle_line, "ec") == 0 && test_idle_cursor == 2;
  was_new_hint_drawn = test_frame_capture_has("hint after") &&
                       !test_frame_capture_has("hint before");
  is_settled_after_refresh = itl_idle_wait_ms() == -1;

  test_frame_capture_size = 0;
  itl_idle_run(&le);
  is_unchanged_row_quiet = !test_frame_capture_has("hint after");

  test_idle_outcome = TL_IDLE_AGAIN;
  test_frame_capture_size = 0;
  itl_idle_run(&le);
  repeat_wait_ms = itl_idle_wait_ms();
  is_repeat_due = repeat_wait_ms >= 0 && repeat_wait_ms <= 50;

  test_idle_outcome = 0;
  test_idle_next_hint = "hint ignored";
  test_frame_capture_size = 0;
  itl_idle_run(&le);
  is_plain_answer_silent = test_frame_capture_size == 0 &&
                           itl_idle_wait_ms() == -1 &&
                           test_idle_call_count == 4;

  tl_set_idle_callback(NULL, 0, 0);
  test_idle_next_hint = NULL;
  test_hint_finish_frame(line);

  ok = is_due_at_once && did_receive_line && was_new_hint_drawn &&
       is_settled_after_refresh && is_unchanged_row_quiet && is_repeat_due &&
       is_plain_answer_silent;

  if (!ok) {
    TEST_PRINTF("due %d, line %d '%s' %zu, drawn %d, settled %d, quiet %d, "
                "repeat %d (%d ms), silent %d\n",
                (int) is_due_at_once, (int) did_receive_line, test_idle_line,
                test_idle_cursor, (int) was_new_hint_drawn,
                (int) is_settled_after_refresh, (int) is_unchanged_row_quiet,
                (int) is_repeat_due, repeat_wait_ms,
                (int) is_plain_answer_silent);
  }

  return ok;
}

static bool
test_auto_pair_line_is(itl_le_t *le, const char *expected, size_t caret)
{
  char text[BUFFER_SIZE];

  if (itl_string_to_cstr(le->line, text, sizeof(text)) != TL_SUCCESS) {
    return false;
  }
  if (strcmp(text, expected) != 0 || le->cursor_position != caret) {
    TEST_PRINTF("line '%s' caret %zu, expected '%s' caret %zu\n", text,
                le->cursor_position, expected, caret);
    return false;
  }
  return true;
}

/* Brackets and quotes after an odd count of single quotes are plain text. */
static int
test_pair_role_inside_quote(const char *buffer, size_t cursor, int byte)
{
  size_t quote_count = 0;
  size_t i;

  for (i = 0; i < cursor; ++i) {
    quote_count += buffer[i] == '\'';
  }
  if (quote_count % 2 == 1) {
    return TL_PAIR_NONE;
  }
  return byte == ')' ? TL_PAIR_CLOSES : TL_PAIR_OPENS;
}

static bool
test_auto_pair_types_steps_and_erases(void)
{
  char out_buffer[BUFFER_SIZE];
  bool ok = true;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "> ");

  ok = ok && !itl_le_auto_pair_type(&le, '(');

  tl_set_auto_pair(1);
  ok = ok && itl_le_auto_pair_type(&le, '(') &&
       test_auto_pair_line_is(&le, "()", 1);
  ok = ok && itl_le_insert(&le, itl_utf8_parse('a')) &&
       test_auto_pair_line_is(&le, "(a)", 2);
  ok = ok && itl_le_auto_pair_type(&le, ')') &&
       test_auto_pair_line_is(&le, "(a)", 3) && itl_g_auto_pair_count == 0;
  ok = ok && !itl_le_auto_pair_type(&le, ')');

  itl_le_clear_line(&le);
  ok = ok && itl_le_auto_pair_type(&le, '[') &&
       itl_le_auto_pair_type(&le, '{') &&
       test_auto_pair_line_is(&le, "[{}]", 2) && itl_g_auto_pair_count == 2;
  ok = ok && itl_le_auto_pair_erase(&le) &&
       test_auto_pair_line_is(&le, "[]", 1);
  ok = ok && itl_le_auto_pair_erase(&le) && test_auto_pair_line_is(&le, "", 0);
  ok = ok && !itl_le_auto_pair_erase(&le);

  itl_le_clear_line(&le);
  ok = ok && itl_le_auto_pair_type(&le, '"') &&
       test_auto_pair_line_is(&le, "\"\"", 1);
  ok = ok && itl_le_auto_pair_type(&le, '"') &&
       test_auto_pair_line_is(&le, "\"\"", 2);

  ITL_STRING_FROM_CSTR(line, "don");
  le.cursor_position = line->length;
  itl_g_auto_pair_count = 0;
  ok = ok && !itl_le_auto_pair_type(&le, '\'');

  ITL_STRING_FROM_CSTR(line, "\\");
  le.cursor_position = line->length;
  ok = ok && !itl_le_auto_pair_type(&le, '(');

  ITL_STRING_FROM_CSTR(line, "x");
  le.cursor_position = 0;
  ok = ok && !itl_le_auto_pair_type(&le, '(');

  ITL_STRING_FROM_CSTR(line, "()");
  le.cursor_position = 1;
  ok = ok && !itl_le_auto_pair_type(&le, ')') && !itl_le_auto_pair_erase(&le);

  tl_set_pair_role_callback(test_pair_role_inside_quote);
  ITL_STRING_FROM_CSTR(line, "' ");
  le.cursor_position = line->length;
  itl_g_auto_pair_count = 0;
  ok = ok && !itl_le_auto_pair_type(&le, '(') &&
       !itl_le_auto_pair_type(&le, '"');
  ITL_STRING_FROM_CSTR(line, "a ");
  le.cursor_position = line->length;
  ok = ok && itl_le_auto_pair_type(&le, '(') &&
       test_auto_pair_line_is(&le, "a ()", 3);
  ok = ok && itl_le_insert(&le, itl_utf8_parse('\'')) &&
       !itl_le_auto_pair_type(&le, ')') &&
       test_auto_pair_line_is(&le, "a (')", 4) && itl_g_auto_pair_count == 1;
  tl_set_pair_role_callback(NULL);

  itl_le_clear_line(&le);
  itl_g_auto_pair_count = 0;
  ok = ok && itl_le_auto_pair_type(&le, '"') &&
       itl_le_insert(&le, itl_utf8_parse('$')) &&
       itl_le_auto_pair_type(&le, '(') &&
       test_auto_pair_line_is(&le, "\"$()\"", 3) && itl_g_auto_pair_count == 2;

  ITL_STRING_FROM_CSTR(line, "$\"");
  le.cursor_position = 1;
  itl_g_auto_pair_count = 0;
  ok = ok && itl_le_auto_pair_type(&le, '{') &&
       test_auto_pair_line_is(&le, "${}\"", 2);

  ITL_STRING_FROM_CSTR(line, "\"x\"");
  le.cursor_position = 0;
  itl_g_auto_pair_count = 0;
  ok = ok && !itl_le_auto_pair_type(&le, '"');

  tl_set_auto_pair(0);
  ITL_STRING_FREE(line);

  if (!ok) {
    TEST_PRINTF("auto pair count %zu\n", itl_g_auto_pair_count);
  }

  return ok;
}

#define TEST_CARET_SGR "\x1b[4m"

static size_t test_highlight_cursor = 0;
static size_t test_highlight_call_count = 0;

static int
test_caret_cell_highlight_callback(const char *buffer, tl_highlight *out)
{
  test_highlight_cursor = out->cursor;
  test_highlight_call_count += 1;

  if (out->capacity == 0 || out->cursor == TL_HIGHLIGHT_NO_CURSOR ||
      out->cursor >= strlen(buffer))
  {
    return 0;
  }

  out->spans[0].start = out->cursor;
  out->spans[0].end = out->cursor + 1;
  out->spans[0].sgr = TEST_CARET_SGR;
  out->count = 1;

  return 1;
}

static void
test_caret_highlight_move(itl_le_t *le, size_t position)
{
  test_frame_capture_size = 0;
  itl_g_tty_should_refresh_text = false;
  le->cursor_position = position;
  itl_le_tty_refresh(le);
}

static bool
test_highlight_follows_the_caret(void)
{
  char   out_buffer[BUFFER_SIZE];
  size_t full_count;
  size_t call_count;
  bool   was_end_cursor_passed;
  bool   was_move_redrawn;
  bool   was_same_spans_quiet;
  bool   was_unfollowed_quiet;
  bool   was_byte_offset_passed;
  bool   ok;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "> ");
  ITL_STRING_FROM_CSTR(line, "abc");
  le.cursor_position = line->length;

  itl_g_tty_changed_size = 0;
  itl_g_tty_prev_rows = 24;
  itl_g_tty_prev_cols = 80;
  itl_g_debug_frame_sink = test_frame_capture_sink;
  tl_set_highlight_callback(test_caret_cell_highlight_callback);
  tl_set_highlight_follows_cursor(1);

  test_frame_capture_refresh(&le);
  was_end_cursor_passed =
      test_highlight_cursor == 3 && !test_frame_capture_has(TEST_CARET_SGR);

  full_count = itl_g_debug_full_refresh_count;
  test_caret_highlight_move(&le, 1);
  was_move_redrawn = test_highlight_cursor == 1 &&
                     itl_g_debug_full_refresh_count == full_count + 1 &&
                     test_frame_capture_has(TEST_CARET_SGR "b");

  full_count = itl_g_debug_full_refresh_count;
  call_count = test_highlight_call_count;
  test_caret_highlight_move(&le, 1);
  was_same_spans_quiet = test_highlight_call_count == call_count + 1 &&
                         itl_g_debug_full_refresh_count == full_count &&
                         !test_frame_capture_has(TEST_CARET_SGR);

  tl_set_highlight_follows_cursor(0);
  full_count = itl_g_debug_full_refresh_count;
  call_count = test_highlight_call_count;
  test_caret_highlight_move(&le, 2);
  was_unfollowed_quiet = test_highlight_call_count == call_count &&
                         itl_g_debug_full_refresh_count == full_count &&
                         !test_frame_capture_has(TEST_CARET_SGR);

  ITL_STRING_FROM_CSTR(line, "\xd0\xb0" "bc");
  le.cursor_position = 1;
  test_frame_capture_refresh(&le);
  was_byte_offset_passed = test_highlight_cursor == 2;

  itl_g_debug_frame_sink = NULL;
  tl_set_highlight_callback(NULL);
  itl_g_tty_changed_size = 1;
  itl_g_tty_first_render = true;
  ITL_STRING_FREE(line);

  ok = was_end_cursor_passed && was_move_redrawn && was_same_spans_quiet &&
       was_unfollowed_quiet && was_byte_offset_passed;

  if (!ok) {
    TEST_PRINTF("end %d, moved %d, same quiet %d, unfollowed quiet %d, "
                "bytes %d\n",
                (int) was_end_cursor_passed, (int) was_move_redrawn,
                (int) was_same_spans_quiet, (int) was_unfollowed_quiet,
                (int) was_byte_offset_passed);
  }

  return ok;
}

#define TEST_RIGHT_PROMPT "\x1b[32mRP\x1b[0m"

static void
test_right_prompt_prepare_frame(itl_le_t *le, itl_string_t *line,
                                char *out_buffer, size_t out_size,
                                const char *text, size_t cols)
{
  itl_le_init(le, line, out_buffer, out_size, "> ");
  ITL_STRING_FROM_CSTR(line, text);
  le->cursor_position = line->length;

  itl_g_tty_changed_size = 0;
  itl_g_tty_prev_rows = 24;
  itl_g_tty_prev_cols = cols;
  itl_g_debug_frame_sink = test_frame_capture_sink;
  tl_set_right_prompt(TEST_RIGHT_PROMPT);
}

static void
test_right_prompt_finish_frame(itl_string_t *line)
{
  itl_g_debug_frame_sink = NULL;
  tl_set_right_prompt(NULL);
  tl_set_transient_prompt(NULL);
  itl_g_hint_is_closed = false;
  itl_g_tty_changed_size = 1;
  itl_g_tty_first_render = true;
  itl_le_invalidate_prev_frame();
  ITL_STRING_FREE(line);
}

/* The right prompt ends one column short of the edge, and it needs one free
   column after the first input row. At 20 columns with a two-column prompt it
   starts at column 17, so input that ends at column 16 keeps it and input that
   reaches column 17 hides it. */
static bool
test_right_prompt_fits_the_first_row(void)
{
  char out_buffer[BUFFER_SIZE];
  bool is_width_measured;
  bool was_drawn_short;
  bool was_drawn_at_the_gap;
  bool was_hidden_at_overlap;
  bool was_drawn_above_rows;
  bool was_hidden_when_wrapped;
  bool was_hidden_when_narrow;
  bool was_hidden_when_held;
  bool ok;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  test_right_prompt_prepare_frame(&le, line, out_buffer, sizeof(out_buffer),
                                  "ab", 20);
  is_width_measured = itl_g_right_prompt_width == 2;

  test_frame_capture_refresh(&le);
  was_drawn_short = test_frame_capture_has("\x1b[18G" TEST_RIGHT_PROMPT) &&
                    itl_g_le_prev_right_prompt_is_shown;

  ITL_STRING_FROM_CSTR(line, "abcdefghijklmn");
  le.cursor_position = line->length;
  test_frame_capture_refresh(&le);
  was_drawn_at_the_gap = test_frame_capture_has(TEST_RIGHT_PROMPT);

  ITL_STRING_FROM_CSTR(line, "abcdefghijklmno");
  le.cursor_position = line->length;
  test_frame_capture_refresh(&le);
  was_hidden_at_overlap = !test_frame_capture_has("RP") &&
                          !itl_g_le_prev_right_prompt_is_shown;

  ITL_STRING_FROM_CSTR(line, "ab\ncdefghijklmnopq");
  le.cursor_position = line->length;
  test_frame_capture_refresh(&le);
  was_drawn_above_rows =
      test_frame_capture_has("\x1b[1A\x1b[18G" TEST_RIGHT_PROMPT) &&
      itl_g_le_prev_total_rows == 2;

  ITL_STRING_FROM_CSTR(line, "abcdefghijklmnopqrstuvwxyz\nab");
  le.cursor_position = line->length;
  test_frame_capture_refresh(&le);
  was_hidden_when_wrapped = !test_frame_capture_has("RP");

  ITL_STRING_FROM_CSTR(line, "");
  le.cursor_position = 0;
  itl_g_tty_prev_cols = 5;
  test_frame_capture_refresh(&le);
  was_hidden_when_narrow = !test_frame_capture_has("RP");

  itl_g_tty_prev_cols = 20;
  itl_g_right_prompt_is_held = true;
  test_frame_capture_refresh(&le);
  was_hidden_when_held = !test_frame_capture_has("RP");
  itl_g_right_prompt_is_held = false;

  test_right_prompt_finish_frame(line);

  ok = is_width_measured && was_drawn_short && was_drawn_at_the_gap &&
       was_hidden_at_overlap && was_drawn_above_rows &&
       was_hidden_when_wrapped && was_hidden_when_narrow &&
       was_hidden_when_held;

  if (!ok) {
    TEST_PRINTF("width %d, short %d, gap %d, overlap %d, rows %d, wrapped %d, "
                "narrow %d, held %d\n",
                (int) is_width_measured, (int) was_drawn_short,
                (int) was_drawn_at_the_gap, (int) was_hidden_at_overlap,
                (int) was_drawn_above_rows, (int) was_hidden_when_wrapped,
                (int) was_hidden_when_narrow, (int) was_hidden_when_held);
  }

  return ok;
}

/* Typed characters stay on the append path. Each one redraws the right prompt
   after the clear, until the input reaches it, and the prompt comes back on a
   full redraw once the input shrinks. A prompt with a line break is never
   drawn. */
static bool
test_right_prompt_follows_appends(void)
{
  const char *keys = "cdefghijklmno";
  char        out_buffer[BUFFER_SIZE];
  size_t      key_index;
  size_t      drawn_count = 0;
  bool        was_hidden_at_end;
  bool        was_back_after_erase;
  bool        was_line_break_rejected;
  bool        ok;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  test_right_prompt_prepare_frame(&le, line, out_buffer, sizeof(out_buffer),
                                  "ab", 20);
  test_frame_capture_refresh(&le);

  itl_g_debug_append_refresh_count = 0;
  for (key_index = 0; keys[key_index] != '\0'; ++key_index) {
    itl_utf8_t appended_character = itl_utf8_parse((uint8_t) keys[key_index]);

    itl_g_tty_plain_append_pending = le.cursor_position == le.line->length;
    itl_g_tty_plain_append_width = itl_char_width(appended_character);
    itl_le_insert(&le, appended_character);
    itl_g_tty_should_refresh_text = true;
    test_frame_capture_size = 0;
    itl_le_tty_refresh(&le);
    if (test_frame_capture_has(TEST_RIGHT_PROMPT)) {
      drawn_count += 1;
    }
  }
  was_hidden_at_end = !itl_g_le_prev_right_prompt_is_shown;

  ITL_LE_ERASE_BACKWARD(&le, 1);
  itl_g_tty_should_refresh_text = true;
  test_frame_capture_size = 0;
  itl_le_tty_refresh(&le);
  was_back_after_erase = test_frame_capture_has(TEST_RIGHT_PROMPT) &&
                         itl_g_le_prev_right_prompt_is_shown;

  tl_set_right_prompt("one\ntwo");
  was_line_break_rejected = itl_g_right_prompt_width == 0;

  ok = drawn_count == 12 && was_hidden_at_end && was_back_after_erase &&
       was_line_break_rejected &&
       itl_g_debug_append_refresh_count == strlen(keys);

  test_right_prompt_finish_frame(line);

  if (!ok) {
    TEST_PRINTF("drawn %zu, hidden %d, back %d, line break %d, appends %zu\n",
                drawn_count, (int) was_hidden_at_end,
                (int) was_back_after_erase, (int) was_line_break_rejected,
                itl_g_debug_append_refresh_count);
  }

  return ok;
}

/* Enter or Ctrl-C with a transient prompt erases from the top of the block
   down and draws the line after that prompt alone. A submit without one
   leaves the block standing. */
static bool
test_transient_prompt_redraws_the_submitted_line(void)
{
  static const char HINT[] = "usage: ab";
  char              out_buffer[BUFFER_SIZE];
  bool              was_block_erased;
  bool              was_line_redrawn;
  bool              were_prompts_dropped;
  bool              is_prompt_restored;
  bool              was_interrupt_redrawn;
  bool              was_plain_submit_left;
  bool              ok;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  test_right_prompt_prepare_frame(&le, line, out_buffer, sizeof(out_buffer),
                                  "ab\ncd", 40);
  le.prompt = "top\n> ";
  le.prompt_size = strlen(le.prompt);
  le.prompt_width = itl_prompt_last_row_width(le.prompt, &le.prompt_rows);
  test_hint_text = HINT;
  itl_g_hint_is_closed = false;
  itl_g_hint_hold_count = 0;
  tl_set_hint_callback(test_hint_callback);
  tl_set_transient_prompt("$ ");
  test_frame_capture_refresh(&le);

  test_frame_capture_size = 0;
  itl_le_finish_input(&le, TL_PRESSED_ENTER);
  was_block_erased = test_frame_capture_has("\x1b[2A\x1b[1G\x1b[0J");
  was_line_redrawn = test_frame_capture_has("$ ab") &&
                     itl_g_le_prev_total_rows == 2;
  were_prompts_dropped = !test_frame_capture_has("RP") &&
                         !test_frame_capture_has("top") &&
                         !test_frame_capture_has(HINT);
  is_prompt_restored = !itl_g_right_prompt_is_held;

  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "> ");
  ITL_STRING_FROM_CSTR(line, "ab");
  le.cursor_position = line->length;
  itl_g_hint_is_closed = false;
  test_frame_capture_refresh(&le);
  test_frame_capture_size = 0;
  itl_le_finish_input(&le, TL_PRESSED_INTERRUPT);
  was_interrupt_redrawn = test_frame_capture_has("\x1b[0J") &&
                          test_frame_capture_has("$ ab");

  tl_set_transient_prompt(NULL);
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "> ");
  ITL_STRING_FROM_CSTR(line, "ab");
  le.cursor_position = line->length;
  itl_g_hint_is_closed = false;
  test_frame_capture_refresh(&le);
  test_frame_capture_size = 0;
  itl_le_finish_input(&le, TL_PRESSED_ENTER);
  was_plain_submit_left = !test_frame_capture_has("\x1b[0J") &&
                          !test_frame_capture_has("$ ");

  tl_set_hint_callback(NULL);
  itl_hint_forget_shown();
  itl_hint_drop_next();
  test_right_prompt_finish_frame(line);

  ok = was_block_erased && was_line_redrawn && were_prompts_dropped &&
       is_prompt_restored && was_interrupt_redrawn && was_plain_submit_left;

  if (!ok) {
    TEST_PRINTF("erased %d, redrawn %d, dropped %d, restored %d, interrupt %d, "
                "plain %d\n",
                (int) was_block_erased, (int) was_line_redrawn,
                (int) were_prompts_dropped, (int) is_prompt_restored,
                (int) was_interrupt_redrawn, (int) was_plain_submit_left);
  }

  return ok;
}
#endif

#define TEST_OSC_NAME    "a\x1b]0;PWN\x07" "z"
#define TEST_OSC_VISIBLE "a^[]0;PWN^Gz"

/* A control byte from a file name, a description, or a suggestion draws as its
   notation in a menu cell, the ghost, and the hint row, and the widths count
   the notation. */
static bool
test_control_bytes_draw_visibly(void)
{
  static const char *names[] = {TEST_OSC_NAME, "c\xC2\x9B" "d"};
  int                was_colors_enabled = itl_g_colors_enabled;
  tl_completion      result = ITL_ZERO_INIT;
  itl_char_buf_t    *b = itl_char_buf_alloc();
  size_t             drawn;
  bool               is_cell_visible;
  bool               is_c1_visible;
  bool               is_colored_visible;
  bool               is_name_width_counted;
  bool               is_invalid_byte_visible;
  bool               is_ghost_visible;
  bool               is_hint_blanked;
  bool               ok;

  tl_set_colors_enabled(0);

  drawn = itl_menu_append_cell(b, TEST_OSC_NAME, 20, false);
  is_cell_visible = drawn == 12 && b->size == 12 &&
                    memcmp(b->data, TEST_OSC_VISIBLE, 12) == 0;

  b->size = 0;
  drawn = itl_menu_append_cell(b, names[1], 20, false);
  is_c1_visible = drawn == 6 && b->size == 6 &&
                  memcmp(b->data, "c\\x9bd", 6) == 0;

  b->size = 0;
  itl_menu_append_colored_cell(b, TEST_OSC_NAME, 5, NULL, 0, true);
  is_colored_visible = b->size == 5 && memcmp(b->data, "a^[]0", 5) == 0;

  result.candidates = names;
  result.count = countof(names);
  is_name_width_counted = itl_menu_name_width(&result) == 12;

  b->size = 0;
  drawn = itl_char_buf_append_visible(b, "x\xFF", 2, 10);
  is_invalid_byte_visible = drawn == 5 && itl_visible_width("x\xFF", 2) == 5 &&
                            b->size == 5 && memcmp(b->data, "x\\xff", 5) == 0;

  b->size = 0;
  memcpy(itl_g_ghost, TEST_OSC_NAME, sizeof(TEST_OSC_NAME));
  itl_g_ghost_len = sizeof(TEST_OSC_NAME) - 1;
  itl_g_ghost_width = itl_visible_width(itl_g_ghost, itl_g_ghost_len);
  is_ghost_visible = itl_g_ghost_width == 12 &&
                     itl_le_tty_draw_ghost(b, true, 0, 80) &&
                     test_bytes_have(b->data, b->size, TEST_OSC_VISIBLE) &&
                     !test_bytes_have(b->data, b->size, "\x07") &&
                     !test_bytes_have(b->data, b->size, "\x1b]");
  itl_ghost_clear();
  itl_g_le_prev_ghost_len = 0;

  tl_set_hint_callback(test_hint_callback);
  test_hint_text = "a\xC2\x9B" "b\x1b" "c";
  itl_hint_compose("x", 1, 20, 24);
  is_hint_blanked = strcmp(itl_g_hint_next, "  a b c") == 0;
  test_hint_text = "a\x9B" "b\xE2\x82" "c\xC3\xA9";
  itl_hint_compose("x", 1, 20, 24);
  is_hint_blanked &= strcmp(itl_g_hint_next, "  a b  c\xC3\xA9") == 0;
  tl_set_hint_callback(NULL);
  itl_hint_drop_next();

  tl_set_colors_enabled(was_colors_enabled);
  ITL_CHAR_BUF_FREE(b);

  ok = is_cell_visible && is_c1_visible && is_colored_visible &&
       is_name_width_counted && is_invalid_byte_visible && is_ghost_visible &&
       is_hint_blanked;

  if (!ok) {
    TEST_PRINTF("cell %d, c1 %d, colored %d, width %d, invalid %d, ghost %d, "
                "hint %d\n",
                (int) is_cell_visible, (int) is_c1_visible,
                (int) is_colored_visible, (int) is_name_width_counted,
                (int) is_invalid_byte_visible, (int) is_ghost_visible,
                (int) is_hint_blanked);
  }

  return ok;
}

/* A right prompt keeps SGR colors and is hidden by any other control, since
   the width walker cannot know where that control moves the cursor. */
static bool
test_right_prompt_rejects_controls(void)
{
  static const char *rejected[] = {"R\tP", "R\rP", "\x1b]0;t\x07RP",
                                   "\x1b[2CRP", "\xC2\x9BRP"};
  bool               ok;
  size_t             i;

  tl_set_right_prompt("\x1b[1;32mRP\x1b[0m");
  ok = itl_g_right_prompt_width == 2;

  for (i = 0; i < countof(rejected); ++i) {
    tl_set_right_prompt(rejected[i]);
    if (itl_g_right_prompt_width != 0) {
      TEST_PRINTF("right prompt %zu kept width %zu\n", i,
                  itl_g_right_prompt_width);
      ok = false;
    }
  }

  tl_set_right_prompt(NULL);
  return ok;
}

/* Alt-. skips a last word that is not UTF-8 and only marks the walk when it
   inserted a word, so the next press never replaces a span it did not put
   into the line. */
static bool
test_last_argument_skips_invalid_words(void)
{
  static const char content[] = "echo first\necho caf\xE9\n";
  static const char invalid_only[] = "echo caf\xE9\n";
  const char       *path = "tl_test_last_argument_invalid.txt";
  char              out_buffer[BUFFER_SIZE];
  bool              ok = true;
  itl_le_t          le = ITL_ZERO_INIT;
  itl_string_t     *line = itl_string_alloc();

  itl_g_is_active = true;
  remove(path);
  ok &= hist_write_raw(path, content, sizeof(content) - 1);
  tl_history_load(path);

  ITL_STRING_FROM_CSTR(line, "ls ");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  itl_le_end_insertion_walk();
  itl_le_key_handle(&le, TL_KEY_LAST_ARGUMENT);
  ok &= test_line_is(line, "ls first", "alt-. past a word that is not UTF-8");
  itl_le_key_handle(&le, TL_KEY_LAST_ARGUMENT);
  ok &= test_line_is(line, "ls first", "alt-. past the oldest valid word");

  itl_g_history_free();
  ok &= hist_write_raw(path, invalid_only, sizeof(invalid_only) - 1);
  tl_history_load(path);
  ITL_STRING_FROM_CSTR(line, "ls ");
  le.cursor_position = line->length;
  itl_le_end_insertion_walk();
  itl_le_key_handle(&le, TL_KEY_LAST_ARGUMENT);
  ok &= itl_g_le_action != ITL_LE_ACTION_LAST_ARGUMENT;
  itl_le_key_handle(&le, TL_KEY_LAST_ARGUMENT);
  ok &= test_line_is(line, "ls ", "alt-. with no valid word");

  ITL_STRING_FREE(line);
  remove(path);
  itl_g_history_free();
  itl_le_end_insertion_walk();
  itl_g_is_active = false;
  return ok;
}

/* Once the help rows are full, every later item is dropped, including the keys
   that would still fit on the last row. */
static bool
test_menu_help_drops_items_past_the_last_row(void)
{
  itl_char_buf_t *b = itl_char_buf_alloc();
  size_t          rows;
  bool            ok;

  rows = itl_menu_layout_help(b, "abcdefgh, ijklmnop", "x", 12, 0, 1);
  ok = rows == 1 && test_bytes_have(b->data, b->size, "abcdefgh,") &&
       !test_bytes_have(b->data, b->size, "ijkl") &&
       !test_bytes_have(b->data, b->size, "x");

  if (!ok) {
    TEST_PRINTF("rows %zu\n", rows);
  }

  ITL_CHAR_BUF_FREE(b);
  return ok;
}

#if defined ITL_POSIX && !defined NDEBUG
/* The line renderer draws control bytes as notation on a full render and on
   the append path, and the caret column counts the notation. */
static bool
test_line_render_shows_control_bytes(void)
{
  char             out_buffer[BUFFER_SIZE];
  int              was_colors_enabled = itl_g_colors_enabled;
  itl_le_metrics_t expected;
  itl_utf8_t       escape = itl_utf8_parse(0x1b);
  bool             is_full_render_visible;
  bool             is_append_visible;
  bool             ok;

  itl_le_t      le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  tl_set_colors_enabled(0);
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "> ");
  ITL_STRING_FROM_CSTR(line, TEST_OSC_NAME "\xC2\x9B");
  le.cursor_position = line->length;
  itl_g_tty_changed_size = 0;
  itl_g_tty_prev_rows = 24;
  itl_g_tty_prev_cols = 40;
  itl_g_debug_frame_sink = test_frame_capture_sink;

  expected = itl_le_compute_metrics(&le, 40);
  test_frame_capture_refresh(&le);
  is_full_render_visible = test_frame_capture_has(TEST_OSC_VISIBLE "\\x9b") &&
                           !test_frame_capture_has("\x07") &&
                           !test_frame_capture_has("\x1b]") &&
                           !test_frame_capture_has("\xC2\x9B") &&
                           expected.cursor_col == 18 &&
                           itl_g_le_prev_cursor_col == 18;

  itl_g_tty_plain_append_pending = true;
  itl_g_tty_plain_append_width = itl_char_line_width(escape);
  itl_le_insert(&le, escape);
  itl_g_tty_should_refresh_text = true;
  itl_g_debug_append_refresh_count = 0;
  test_frame_capture_size = 0;
  itl_le_tty_refresh(&le);
  is_append_visible = itl_g_debug_append_refresh_count == 1 &&
                      test_frame_capture_has("^[") &&
                      !test_frame_capture_has("\x1b\x1b") &&
                      itl_g_le_prev_cursor_col == 20;

  itl_g_debug_frame_sink = NULL;
  itl_g_tty_changed_size = 1;
  itl_g_tty_first_render = true;
  itl_le_invalidate_prev_frame();
  tl_set_colors_enabled(was_colors_enabled);
  ITL_STRING_FREE(line);

  ok = is_full_render_visible && is_append_visible;

  if (!ok) {
    TEST_PRINTF("full %d, append %d, caret %zu\n", (int) is_full_render_visible,
                (int) is_append_visible, itl_g_le_prev_cursor_col);
  }

  return ok;
}
#endif

typedef bool (*test_func)(void);

typedef struct test_case test_case_t;

struct test_case
{
  const char *name;
  test_func   func;
};

#define DEFINE_TEST_CASE(fn)                                                   \
  {                                                                            \
    .name = #fn, .func = fn,                                                   \
  }

static test_case_t test_cases[] = {DEFINE_TEST_CASE(test_string_from_cstr),
                                   DEFINE_TEST_CASE(test_string_shift),
                                   DEFINE_TEST_CASE(test_string_erase),
                                   DEFINE_TEST_CASE(test_string_insert),
                                   DEFINE_TEST_CASE(test_char_buf),
                                   DEFINE_TEST_CASE(
                                       test_string_shift_directions),
                                   DEFINE_TEST_CASE(
                                       test_string_copy_uses_live_range),
                                   DEFINE_TEST_CASE(test_string_to_cstr_limits),
                                   DEFINE_TEST_CASE(
                                       test_char_buf_growth_boundary),
                                   DEFINE_TEST_CASE(test_parse_size),
                                   DEFINE_TEST_CASE(test_utf8_strlen),
                                   DEFINE_TEST_CASE(
                                       test_string_from_bytes_truncates_at_rune_boundary),
                                   DEFINE_TEST_CASE(
                                       test_string_from_bytes_rejects_malformed_utf8),
                                   DEFINE_TEST_CASE(test_char_width),
                                   DEFINE_TEST_CASE(test_metrics),
                                   DEFINE_TEST_CASE(test_find_substring),
                                   DEFINE_TEST_CASE(test_history_multiline_file),
                                   DEFINE_TEST_CASE(
                                       test_rejected_ghost_history_prefix_is_cached),
#if defined ITL_WIN32 && !defined ITL_NO_WIN_ESCAPES
                                   DEFINE_TEST_CASE(
                                       test_windows_ghost_does_not_require_term),
#endif
                                   DEFINE_TEST_CASE(test_history_ring_cap),
                                   DEFINE_TEST_CASE(test_history_dedup),
                                   DEFINE_TEST_CASE(test_history_unterminated_line),
                                   DEFINE_TEST_CASE(
                                       test_history_carriage_return_rule),
                                   DEFINE_TEST_CASE(
                                       test_history_offset_shift_matches_scan),
                                   DEFINE_TEST_CASE(test_history_search),
                                   DEFINE_TEST_CASE(
                                       test_history_search_snapshot),
                                   DEFINE_TEST_CASE(
                                       test_history_search_matching),
                                   DEFINE_TEST_CASE(
                                       test_history_search_rejects_malformed_entry),
                                   DEFINE_TEST_CASE(
                                       test_history_search_narrowing),
#if defined ITL_POSIX
                                   DEFINE_TEST_CASE(
                                       test_history_search_preview_cache),
#endif
                                   DEFINE_TEST_CASE(test_history_short_entry_skipped),
                                   DEFINE_TEST_CASE(test_history_alloc_balance),
                                   DEFINE_TEST_CASE(test_history_private_branch),
                                   DEFINE_TEST_CASE(test_history_recall_after_peer_write),
                                   DEFINE_TEST_CASE(
                                       test_completion_replacement_is_atomic),
                                   DEFINE_TEST_CASE(
                                       test_alt_arrows_use_word_movement),
                                   DEFINE_TEST_CASE(test_menu_match_rank),
                                   DEFINE_TEST_CASE(test_menu_filter_groups),
                                   DEFINE_TEST_CASE(
                                       test_menu_narrow_reuses_base),
                                   DEFINE_TEST_CASE(test_menu_cells),
                                   DEFINE_TEST_CASE(
                                       test_history_menu_multiline_display),
                                   DEFINE_TEST_CASE(test_merge_visual_spans),
#if defined ITL_POSIX
                                   DEFINE_TEST_CASE(
                                       test_menu_keys_reuse_gathered_list),
                                   DEFINE_TEST_CASE(
                                       test_tab_prefix_menu_reuses_gather),
                                   DEFINE_TEST_CASE(
                                       test_tab_sole_candidate_stops),
                                   DEFINE_TEST_CASE(
                                       test_vi_ex_command_keeps_the_caret),
                                   DEFINE_TEST_CASE(
                                       test_alt_backspace_sequences),
                                   DEFINE_TEST_CASE(
                                       test_pending_resize_wakes_input_wait),
                                   DEFINE_TEST_CASE(
                                       test_idle_hook_runs_in_the_input_wait),
                                   DEFINE_TEST_CASE(
                                       test_write_all_resumes_a_partial_write),
#endif
                                   DEFINE_TEST_CASE(
                                       test_ctrl_right_accepts_one_ghost_word),
                                   DEFINE_TEST_CASE(
                                       test_ctrl_right_accepts_one_case_corrected_word),
                                   DEFINE_TEST_CASE(
                                       test_kill_ring_appends_yanks_and_cycles),
                                   DEFINE_TEST_CASE(
                                       test_transpose_characters_and_words),
                                   DEFINE_TEST_CASE(
                                       test_emoji_sequences_move_and_measure_whole),
                                   DEFINE_TEST_CASE(
                                       test_completion_is_one_undo_step),
                                   DEFINE_TEST_CASE(test_vi_dot_skips_a_yank),
                                   DEFINE_TEST_CASE(
                                       test_ghost_miss_skips_an_empty_word),
                                   DEFINE_TEST_CASE(
                                       test_last_argument_walks_history),
                                   DEFINE_TEST_CASE(
                                       test_edit_external_replaces_the_line),
                                   DEFINE_TEST_CASE(test_editing_key_sequences),
#if defined ITL_POSIX
                                   DEFINE_TEST_CASE(test_modified_key_sequences),
                                   DEFINE_TEST_CASE(
                                       test_extended_keys_read_as_legacy_bytes),
                                   DEFINE_TEST_CASE(
                                       test_extended_keys_map_signal_keys),
                                   DEFINE_TEST_CASE(
                                       test_extended_keys_follow_raw_mode),
                                   DEFINE_TEST_CASE(
                                       test_raw_mode_keeps_signal_keys),
#endif
                                   DEFINE_TEST_CASE(
                                       test_prefix_history_search_walks_matches),
                                   DEFINE_TEST_CASE(
                                       test_ghost_prefers_recent_history),
                                   DEFINE_TEST_CASE(
                                       test_ghost_history_corrects_case),
                                   DEFINE_TEST_CASE(
                                       test_ghost_completion_corrects_case),
                                   DEFINE_TEST_CASE(
                                       test_menu_preview_needs_an_extending_row),
                                   DEFINE_TEST_CASE(
                                       test_ghost_sticky_target_continues),
                                   DEFINE_TEST_CASE(
                                       test_ghost_clips_multiline_suggestion),
                                   DEFINE_TEST_CASE(
                                       test_tab_clears_stale_ghost_target),
                                   DEFINE_TEST_CASE(
                                       test_tab_honors_suppressed_space),
                                   DEFINE_TEST_CASE(
                                       test_external_screen_requires_raw_mode),
                                   DEFINE_TEST_CASE(
                                       test_reflow_agrees_with_metrics),
                                   DEFINE_TEST_CASE(
                                       test_reflow_agrees_with_reference),
                                   DEFINE_TEST_CASE(
                                       test_wrap_predicates_agree_with_reference),
                                   DEFINE_TEST_CASE(
                                       test_ascii_runs_agree_with_reference),
                                   DEFINE_TEST_CASE(test_csi_sequences),
                                   DEFINE_TEST_CASE(
                                       test_menu_band_survives_disabled_colors),
#if defined ITL_POSIX && !defined NDEBUG
                                   DEFINE_TEST_CASE(
                                       test_loading_frame_drains_before_gather),
                                   DEFINE_TEST_CASE(
                                       test_menu_rows_start_under_the_token),
                                   DEFINE_TEST_CASE(
                                       test_menu_help_wraps_between_items),
                                   DEFINE_TEST_CASE(
                                       test_menu_anchor_accounts_for_descriptions),
                                   DEFINE_TEST_CASE(
                                       test_append_path_keeps_spans),
                                   DEFINE_TEST_CASE(
                                       test_drawn_metrics_match_the_walk),
                                   DEFINE_TEST_CASE(
                                       test_colors_disabled_drop_span_escapes),
                                   DEFINE_TEST_CASE(
                                       test_submit_erases_the_drawn_ghost),
                                   DEFINE_TEST_CASE(
                                       test_hint_row_draws_holds_and_erases),
                                   DEFINE_TEST_CASE(
                                       test_hint_rows_follow_the_frame),
                                   DEFINE_TEST_CASE(
                                       test_prefix_hint_names_the_waiting_keys),
                                   DEFINE_TEST_CASE(
                                       test_ctrl_x_chord_shows_and_drops_its_hint),
                                   DEFINE_TEST_CASE(
                                       test_hint_row_follows_the_caret_and_clears),
                                   DEFINE_TEST_CASE(
                                       test_hint_row_keeps_multiline_rows_and_append_path),
                                   DEFINE_TEST_CASE(
                                       test_idle_hook_refreshes_the_hint_row),
                                   DEFINE_TEST_CASE(
                                       test_highlight_follows_the_caret),
                                   DEFINE_TEST_CASE(
                                       test_auto_pair_types_steps_and_erases),
                                   DEFINE_TEST_CASE(
                                       test_right_prompt_fits_the_first_row),
                                   DEFINE_TEST_CASE(
                                       test_right_prompt_follows_appends),
                                   DEFINE_TEST_CASE(
                                       test_transient_prompt_redraws_the_submitted_line),
                                   DEFINE_TEST_CASE(
                                       test_line_render_shows_control_bytes),
#endif
                                   DEFINE_TEST_CASE(
                                       test_hint_row_is_cut_to_the_width),
                                   DEFINE_TEST_CASE(
                                       test_hint_body_wraps_at_words),
                                   DEFINE_TEST_CASE(
                                       test_hint_rows_fit_a_short_terminal),
                                   DEFINE_TEST_CASE(
                                       test_hint_rows_erase_when_shrinking),
                                   DEFINE_TEST_CASE(
                                       test_control_bytes_draw_visibly),
                                   DEFINE_TEST_CASE(
                                       test_right_prompt_rejects_controls),
                                   DEFINE_TEST_CASE(
                                       test_last_argument_skips_invalid_words),
                                   DEFINE_TEST_CASE(
                                       test_menu_help_drops_items_past_the_last_row),
};

int
main(void)
{
  size_t i;
  bool   result;
  size_t failed_count = 0;

  for (i = 0; i < countof(test_cases); ++i) {
    result = test_cases[i].func();
    if (!result) {
      printf("%s: *** FAIL.\n", test_cases[i].name);
      failed_count += 1;
    } else {
      printf("%s: ok.\n", test_cases[i].name);
    }
  }

  return failed_count > 0 ? 1 : 0;
}
