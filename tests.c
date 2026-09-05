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

  if (itl_strn_display_width(invalid, sizeof(invalid)) != 3 ||
      itl_strn_display_width(truncated, sizeof(truncated)) != 2)
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

static bool
test_find_substring(void)
{
  bool ok = true;

  itl_string_t *hay = itl_string_alloc();
  itl_string_t *needle = itl_string_alloc();

  ITL_STRING_FROM_CSTR(hay, "Hello WORLD");

  ITL_STRING_FROM_CSTR(needle, "o wo");
  if (!itl_string_find_substring_ascii_casefold(hay, needle)) {
    ok = false;
  }
  ITL_STRING_FROM_CSTR(needle, "world");
  if (!itl_string_find_substring_ascii_casefold(hay, needle)) {
    ok = false;
  }
  ITL_STRING_FROM_CSTR(needle, "xyz");
  if (itl_string_find_substring_ascii_casefold(hay, needle)) {
    ok = false;
  }
  ITL_STRING_FROM_CSTR(needle, "");
  if (!itl_string_find_substring_ascii_casefold(hay, needle)) {
    ok = false;
  }
  ITL_STRING_FROM_CSTR(hay, "echo \xC3\x84");
  ITL_STRING_FROM_CSTR(needle, "\xC3\xA4");
  if (itl_string_find_substring_ascii_casefold(hay, needle)) {
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
  snprintf(expected, sizeof(expected), "cmd %zu",
           total - (size_t) TL_HISTORY_MAX_SIZE);
  if (ok && (itl_g_history_count != TL_HISTORY_MAX_SIZE ||
             !hist_entry_is(0, expected)))
  {
    TEST_PRINTF("grown history limit did not reload older entries\n");
    ok = false;
  }

  tl_set_history_limit(0);
  if (ok && itl_g_history_count != 0) {
    TEST_PRINTF("zero history limit retained %zu entries\n",
                itl_g_history_count);
    ok = false;
  }
  tl_set_history_limit(TL_HISTORY_MAX_SIZE);

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

static bool
test_history_search(void)
{
  const char *path = "tl_test_search.txt";
  bool ok = true;

  itl_string_t *query = itl_string_alloc();
  itl_string_t *scratch = itl_string_alloc();
  size_t        match;

  itl_g_is_active = true;
  remove(path);
  tl_history_load(path);

  hist_append_cstr("Git Status");
  hist_append_cstr("GIT commit");
  hist_append_cstr("make test");

  /* Searching backward from the newest finds "git commit" at index 1. */
  ITL_STRING_FROM_CSTR(query, "gIt");
  match = itl_history_find_match(query, itl_g_history_count - 1, scratch);
  if (match != 1) {
    TEST_PRINTF("expected match at index 1, got %zu\n", match);
    ok = false;
  }

  /* The next older match is "git status" at index 0. */
  if (ok) {
    match = itl_history_find_match(query, match - 1, scratch);
    if (match != 0) {
      TEST_PRINTF("expected older match at index 0, got %zu\n", match);
      ok = false;
    }
  }

  /* Searching forward from the oldest finds "git status" at index 0, and the
     next forward step finds "git commit" at index 1, the mirror of the
     backward walk. */
  if (ok) {
    ITL_STRING_FROM_CSTR(query, "GiT");
    match = itl_history_find_match_forward(query, 0, scratch);
    if (match != 0) {
      TEST_PRINTF("expected forward match at index 0, got %zu\n", match);
      ok = false;
    }
  }
  if (ok) {
    match = itl_history_find_match_forward(query, match + 1, scratch);
    if (match != 1) {
      TEST_PRINTF("expected newer forward match at index 1, got %zu\n", match);
      ok = false;
    }
  }

  /* A query that matches nothing returns the sentinel either way. */
  if (ok) {
    ITL_STRING_FROM_CSTR(query, "zzz");
    match = itl_history_find_match(query, itl_g_history_count - 1, scratch);
    if (match != ITL_HISTORY_NONE) {
      TEST_PRINTF("no match should return the sentinel, got %zu\n", match);
      ok = false;
    }
  }
  if (ok) {
    match = itl_history_find_match_forward(query, 0, scratch);
    if (match != ITL_HISTORY_NONE) {
      TEST_PRINTF("no forward match should return the sentinel, got %zu\n",
                  match);
      ok = false;
    }
  }

  remove(path);
  ITL_STRING_FREE(query);
  ITL_STRING_FREE(scratch);
  itl_g_history_free();
  itl_g_is_active = false;
  return ok;
}

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

  itl_string_t *query;
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

  query = itl_string_alloc();
  scratch = itl_string_alloc();
  ITL_STRING_FROM_CSTR(query, "alpha");
  (void) itl_history_find_match(query, itl_g_history_count - 1, scratch);
  ITL_STRING_FREE(query);
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

#if defined ITL_POSIX
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

static bool
test_tab_clears_stale_ghost_target(void)
{
  char out_buffer[BUFFER_SIZE];
  char line_buffer[BUFFER_SIZE];
  bool replacement_ok;
  bool was_handled;
  int previous_supports_decorations = itl_g_supports_decorations;
  itl_le_t le = ITL_ZERO_INIT;
  itl_string_t *line = itl_string_alloc();

  itl_g_supports_decorations = 0;
  ITL_STRING_FROM_CSTR(line, "ab");
  itl_le_init(&le, line, out_buffer, sizeof(out_buffer), "");
  memcpy(itl_g_ghost_sticky_target, "stale", 6);
  tl_set_complete_callback(test_completion_callback);
  was_handled = itl_completion_handle_tab(&le);
  itl_string_to_cstr(line, line_buffer, sizeof(line_buffer));
  replacement_ok = was_handled && strcmp(line_buffer, "alpha") == 0;

  memcpy(itl_g_ghost_sticky_target, "stale", 6);
  was_handled = itl_completion_handle_tab(&le);
  tl_set_complete_callback(NULL);
  itl_g_supports_decorations = previous_supports_decorations;

  ITL_STRING_FREE(line);
  return replacement_ok && was_handled &&
         itl_g_ghost_sticky_target[0] == '\0';
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
                                   DEFINE_TEST_CASE(test_history_search),
                                   DEFINE_TEST_CASE(test_history_short_entry_skipped),
                                   DEFINE_TEST_CASE(test_history_alloc_balance),
                                   DEFINE_TEST_CASE(test_history_concurrent_merge),
                                   DEFINE_TEST_CASE(
                                       test_completion_replacement_is_atomic),
                                   DEFINE_TEST_CASE(
                                       test_alt_arrows_use_word_movement),
#if defined ITL_POSIX
                                   DEFINE_TEST_CASE(
                                       test_alt_backspace_sequences),
#endif
                                   DEFINE_TEST_CASE(
                                       test_ghost_prefers_recent_history),
                                   DEFINE_TEST_CASE(
                                       test_tab_clears_stale_ghost_target)};

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
