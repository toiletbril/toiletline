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
      {"это строка",         "это строка"}
  };
  /* clang-format on */

  const shift_test_case_t settings[] = {
      /*  pos, count, backwards */
      {12, 6, true },
      {0,  1, false},
      {10, 3, true },
      {10, 3, false},
      {0,  0, true }
  };

  for (i = 0; i < countof(tests); ++i) {
    test = tests[i];
    erase = settings[i];

    ITL_STRING_FROM_CSTR(str, test.original);
    itl_string_erase(str, erase.pos, erase.count, erase.backwards);
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
test_string_insert(void)
{
  int                result;
  size_t             i, pos;
  string_test_case_t test;
  char               out_buffer[BUFFER_SIZE];

  itl_string_t *str = itl_string_alloc();
  itl_utf8_t    A = itl_utf8_new((uint8_t[4]){0x41}, 1);

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
test_char_width(void)
{
  itl_utf8_t ascii = itl_utf8_new((uint8_t[4]){0x41}, 1);
  itl_utf8_t cjk = itl_utf8_new((uint8_t[4]){0xE4, 0xBD, 0xA0}, 3);
  itl_utf8_t combining = itl_utf8_new((uint8_t[4]){0xCC, 0x81}, 2);
  itl_utf8_t emoji = itl_utf8_new((uint8_t[4]){0xF0, 0x9F, 0x98, 0x80}, 4);

  if (itl_char_width(ascii) != 1 || itl_char_width(cjk) != 2 ||
      itl_char_width(combining) != 0 || itl_char_width(emoji) != 2)
  {
    TEST_PRINTF("widths: ascii %zu cjk %zu comb %zu emoji %zu\n",
                itl_char_width(ascii), itl_char_width(cjk),
                itl_char_width(combining), itl_char_width(emoji));
    return false;
  }
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

static bool
test_find_substring(void)
{
  bool ok = true;

  itl_string_t *hay = itl_string_alloc();
  itl_string_t *needle = itl_string_alloc();

  ITL_STRING_FROM_CSTR(hay, "hello world");

  ITL_STRING_FROM_CSTR(needle, "o wo");
  if (!itl_string_find_substring(hay, needle)) {
    ok = false;
  }
  ITL_STRING_FROM_CSTR(needle, "world");
  if (!itl_string_find_substring(hay, needle)) {
    ok = false;
  }
  ITL_STRING_FROM_CSTR(needle, "xyz");
  if (itl_string_find_substring(hay, needle)) {
    ok = false;
  }
  ITL_STRING_FROM_CSTR(needle, "");
  if (!itl_string_find_substring(hay, needle)) {
    ok = false;
  }

  if (!ok) {
    TEST_PRINTF("substring match mismatch\n");
  }

  ITL_STRING_FREE(hay);
  ITL_STRING_FREE(needle);
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

  itl_string_t *entry = itl_string_alloc();

  const char multiline[] = {'l', 's', 0x0A, 'p', 'w', 'd'};

  itl_g_is_active = true;

  itl_string_from_bytes(entry, multiline, sizeof multiline);
  itl_g_history_append(entry);

  if (tl_history_dump("/tmp/tl_test_history.txt") != TL_SUCCESS) {
    TEST_PRINTF("dump failed\n");
    ok = false;
  }

  itl_g_history_free();

  if (tl_history_load("/tmp/tl_test_history.txt") != TL_SUCCESS) {
    TEST_PRINTF("load failed\n");
    ok = false;
  }

  if (ok && (itl_g_history_last == NULL ||
             !itl_string_equal(itl_g_history_last->str, entry)))
  {
    TEST_PRINTF("multiline entry did not survive the roundtrip\n");
    ok = false;
  }

  itl_g_history_free();
  ITL_STRING_FREE(entry);
  itl_g_is_active = false;
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
                                   DEFINE_TEST_CASE(test_parse_size),
                                   DEFINE_TEST_CASE(test_utf8_strlen),
                                   DEFINE_TEST_CASE(test_char_width),
                                   DEFINE_TEST_CASE(test_metrics),
                                   DEFINE_TEST_CASE(test_find_substring),
                                   DEFINE_TEST_CASE(test_join_continuations),
                                   DEFINE_TEST_CASE(test_history_multiline_file)};

int
main(void)
{
  size_t i;
  bool   result;

  for (i = 0; i < countof(test_cases); ++i) {
    result = test_cases[i].func();
    if (!result) {
      printf("%s: *** FAIL.\n", test_cases[i].name);
    } else {
      printf("%s: ok.\n", test_cases[i].name);
    }
  }

  return 0;
}
