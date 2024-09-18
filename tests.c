#define TOILETLINE_IMPLEMENTATION
#include "toiletline.h"

#include <stdio.h>

#define BUFFER_SIZE 128

#define test_printf(...)                                                       \
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

    itl_string_from_cstr(str, test.original);
    itl_string_to_cstr(str, out_buffer, BUFFER_SIZE);

    result = strcmp(out_buffer, test.original);

    if (result != 0 || str->length != test.length || str->size != test.size) {
      test_printf("Result %zu: '%s', should be: '%s'. Length: %zu/%zu, "
                  "size: %zu/%zu\n",
                  i, out_buffer, test.original, str->length, test.length,
                  str->size, test.size);
      itl_string_free(str);
      return false;
    }
  }

  itl_string_free(str);

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

    itl_string_from_cstr(str, test.original);
    itl_string_shift(str, shift.pos, shift.count, shift.backwards);
    itl_string_to_cstr(str, out_buffer, BUFFER_SIZE);

    result = strcmp(out_buffer, test.should_be);
    if (result != 0) {
      test_printf("Result %zu: '%s', should be: '%s'\n", i, out_buffer,
                  test.should_be);
      itl_string_free(str);
      return false;
    }
  }

  itl_string_free(str);

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

    itl_string_from_cstr(str, test.original);
    itl_string_erase(str, erase.pos, erase.count, erase.backwards);
    itl_string_to_cstr(str, out_buffer, BUFFER_SIZE);

    result = strcmp(out_buffer, test.should_be);
    if (result != 0) {
      test_printf("Result %zu: '%s', should be: '%s'\n", i, out_buffer,
                  test.should_be);
      itl_string_free(str);
      return false;
    }
  }

  itl_string_free(str);

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

    itl_string_from_cstr(str, test.original);
    itl_string_insert(str, pos, A);
    itl_string_to_cstr(str, out_buffer, BUFFER_SIZE);

    result = strcmp(out_buffer, test.should_be);
    if (result != 0) {
      test_printf("Result %zu: '%s', should be: '%s'\n", i, out_buffer,
                  test.should_be);
      itl_string_free(str);
      return false;
    }
  }

  itl_string_free(str);

  return true;
}

typedef struct split_test_case split_test_case_t;

struct split_test_case
{
  size_t start;
  size_t end;
};

#define SPLIT_POSITION_COUNT 3

static bool
test_string_split(void)
{
  size_t              i, j;
  const char         *cstr;
  itl_split_t        *split;
  split_test_case_t   pos;
  const itl_offset_t *offset;

  itl_string_t *str = itl_string_alloc();

  const char *tests[] = {
      /* original */
      "hello world sailor",
      "привет как дела",
  };
  const split_test_case_t positions[][SPLIT_POSITION_COUNT] = {
  /*  {{start, end }, ... } */
      {{0, 5}, {6, 11}, {12, 18}},
      {{0, 6}, {7, 10}, {11, 15}}
  };

  for (i = 0; i < countof(tests); ++i) {
    cstr = tests[i];

    itl_string_from_cstr(str, cstr);
    split = itl_string_split(str, ' ');

    for (j = 0; j < SPLIT_POSITION_COUNT; ++j) {
      pos = positions[i][j];
      offset = split->offsets[j];
      if (offset->start != pos.start || offset->end != pos.end) {
        test_printf("Result %zu: '%s', split %zu: start %zu/%zu, end "
                    "%zu/%zu\n",
                    i, cstr, j, offset->start, pos.start, offset->end, pos.end);
        itl_string_free(str);
        itl_split_free(split);
        return false;
      }
    }

    itl_split_free(split);
  }

  itl_string_free(str);

  return true;
}

static bool
test_char_buf(void)
{
  itl_string_t   *str = itl_string_alloc();
  itl_char_buf_t *cb = itl_char_buf_alloc();

  const char *should_be = "привет, мир help me3912033312 ЛОЛ";

  itl_string_from_cstr(str, "привет, ");
  itl_char_buf_append_string(cb, str);
  itl_char_buf_append_cstr(cb, "мир ");
  itl_string_from_cstr(str, "help");
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
    test_printf("Result: '%s', should be: '%s', len: %zu/%zu\n", cb->data,
                should_be, cb->size, strlen(should_be));
    itl_string_free(str);
    itl_char_buf_free(cb);
    return false;
  }

  itl_string_free(str);
  itl_char_buf_free(cb);

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
      test_printf("Result: '%zu', should be: '%zu', diff: %zu, "
                  "offset %zu\n",
                  result, should_be[i], diff, offset);
      return false;
    }
    offset += diff + 1;
  }

  return true;
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
                                   DEFINE_TEST_CASE(test_string_split),
                                   DEFINE_TEST_CASE(test_char_buf),
                                   DEFINE_TEST_CASE(test_parse_size)};

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
