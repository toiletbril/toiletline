#define ITL_TTY_IS_TTY() (1)
#define TOILETLINE_IMPLEMENTATION
#include "toiletline.h"

#include <stdio.h>

#define BUFFER_SIZE 128

#define TEST_PRINTF(...)                                                       \
  do {                                                                         \
    fputs(__func__, stdout);                                                   \
    printf(": "__VA_ARGS__);                                                   \
  } while (0)

#define countof(a) (sizeof(a) / sizeof((a)[0]))

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
test_join_continuations(void)
{
  char out[64];

  itl_string_t *s = itl_string_alloc();

  const char with_continuation[] = {'a', 'b', 0x5C, 0x0A, 'c', 'd'};
  const char plain_newline[] = {'a', 0x0A, 'b'};

  itl_string_from_bytes(s, with_continuation, sizeof with_continuation);
  itl_string_join_continuations(s);
  itl_string_to_cstr(s, out, sizeof out);
  if (strcmp(out, "abcd") != 0) {
    TEST_PRINTF("joined: '%s', should be 'abcd'\n", out);
    ITL_STRING_FREE(s);
    return false;
  }

  itl_string_from_bytes(s, plain_newline, sizeof plain_newline);
  itl_string_join_continuations(s);
  itl_string_to_cstr(s, out, sizeof out);
  if (!(out[0] == 'a' && out[1] == 0x0A && out[2] == 'b' && out[3] == '\0')) {
    TEST_PRINTF("plain newline was not preserved, length %zu\n", s->length);
    ITL_STRING_FREE(s);
    return false;
  }

  ITL_STRING_FREE(s);
  return true;
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
test_history_concurrent_merge(void)
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

  /* Our next append must merge in the other session's entry before writing. */
  hist_append_cstr("third three");

  if (itl_g_history_total_count != 3 ||
      itl_g_last_history_event_number != 3)
  {
    TEST_PRINTF("merged total %zu and last %zu\n",
                itl_g_history_total_count,
                itl_g_last_history_event_number);
    ok = false;
  }

  if (itl_g_history_count != 3) {
    TEST_PRINTF("expected 3 merged entries, got %zu\n", itl_g_history_count);
    ok = false;
  }
  if (ok &&
      (!hist_entry_is(0, "first one") || !hist_entry_is(1, "second two") ||
       !hist_entry_is(2, "third three")))
  {
    TEST_PRINTF("merged entries are out of order or wrong\n");
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
  bool ghost_was_not_accepted;
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
  memcpy(itl_g_ghost, " tail", 6);
  itl_g_ghost_len = 5;
  itl_le_key_handle(&alt_le, TL_KEY_RIGHT | TL_MOD_ALT);
  ghost_was_not_accepted = line->length == 10 && itl_g_ghost_len == 5;
  itl_ghost_clear();

  ITL_STRING_FREE(line);
  return left_matches && right_matches && ghost_was_not_accepted;
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
      state.name_width != 0 || state.query_len != 0)
  {
    TEST_PRINTF("an empty source left %zu rows at width %zu\n",
                state.base.count, state.name_width);
    ITL_STRING_FREE(line);
    return false;
  }

  ITL_STRING_FREE(line);

  return true;
}

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
                                   DEFINE_TEST_CASE(test_join_continuations),
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
                                   DEFINE_TEST_CASE(test_history_concurrent_merge),
                                   DEFINE_TEST_CASE(
                                       test_completion_replacement_is_atomic),
                                   DEFINE_TEST_CASE(
                                       test_alt_arrows_use_word_movement),
                                   DEFINE_TEST_CASE(test_menu_match_rank),
                                   DEFINE_TEST_CASE(test_menu_filter_groups),
                                   DEFINE_TEST_CASE(
                                       test_menu_narrow_reuses_base),
                                   DEFINE_TEST_CASE(test_menu_cells),
                                   DEFINE_TEST_CASE(test_merge_visual_spans),
#if defined ITL_POSIX
                                   DEFINE_TEST_CASE(
                                       test_alt_backspace_sequences),
                                   DEFINE_TEST_CASE(
                                       test_pending_resize_wakes_input_wait),
#endif
                                   DEFINE_TEST_CASE(
                                       test_ghost_prefers_recent_history),
                                   DEFINE_TEST_CASE(
                                       test_ghost_history_corrects_case),
                                   DEFINE_TEST_CASE(
                                       test_ghost_completion_corrects_case),
                                   DEFINE_TEST_CASE(
                                       test_ghost_sticky_target_continues),
                                   DEFINE_TEST_CASE(
                                       test_ghost_clips_multiline_suggestion),
                                   DEFINE_TEST_CASE(
                                       test_tab_clears_stale_ghost_target),
                                   DEFINE_TEST_CASE(
                                       test_external_screen_requires_raw_mode),
                                   DEFINE_TEST_CASE(
                                       test_reflow_agrees_with_metrics),
                                   DEFINE_TEST_CASE(
                                       test_wrap_predicates_agree_with_reference),
                                   DEFINE_TEST_CASE(
                                       test_ascii_runs_agree_with_reference),
                                   DEFINE_TEST_CASE(test_csi_sequences),
                                   DEFINE_TEST_CASE(
                                       test_menu_band_survives_disabled_colors),
#if defined ITL_POSIX && !defined NDEBUG
                                   DEFINE_TEST_CASE(
                                       test_append_path_keeps_spans),
                                   DEFINE_TEST_CASE(
                                       test_drawn_metrics_match_the_walk),
                                   DEFINE_TEST_CASE(
                                       test_colors_disabled_drop_span_escapes),
#endif
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
