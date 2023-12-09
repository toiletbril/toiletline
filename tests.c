#define TOILETLINE_IMPLEMENTATION
#include "toiletline.h"

#define OUT_BUFFER_SIZE 128

static char current_function_name[128];

#define test_printf(...)                  \
  do {                                    \
    fputs(current_function_name, stdout); \
    printf(": "__VA_ARGS__);              \
  } while (0)

#define countof(a) (sizeof(a)/sizeof((a)[0]))

typedef struct string_result string_result_t;

struct string_result
{
    const char *original;
    const char *should_be;
};

typedef struct from_cstr_test_case from_cstr_test_case_t;

struct from_cstr_test_case
{
    const char *original;
    size_t length;
    size_t size;
};

static bool test_string_from_cstr()
{
    size_t i;
    int result;
    char out_buffer[OUT_BUFFER_SIZE];

    itl_string_t *str;
    from_cstr_test_case_t test;;

    from_cstr_test_case_t tests[] = {
        /* original, length, size */
        { "hello, world", 12, 12 },
        { "привет, мир", 11, 20 },
        { "你好世界", 4, 12 }
    };

    for (i = 0; i < countof(tests); ++i) {
        test = tests[i];
        str = itl_string_alloc();

        itl_string_from_cstr(str, test.original);
        itl_string_to_cstr(str, out_buffer, OUT_BUFFER_SIZE);

        result = strcmp(out_buffer, test.original);

        if (result != 0 || str->length != test.length
                        || str->size != test.size) {
          test_printf("Result %zu: '%s', should be: '%s'. Length: %zu/%zu, "
                      "size: %zu/%zu\n",
                      i, out_buffer, test.original, str->length, test.length,
                      str->size, test.size);
          return false;
        }
    }

    return true;
}

typedef struct shift_test_case shift_test_case_t;

struct shift_test_case
{
    size_t pos;
    size_t count;
    bool backwards;
};

static bool test_string_shift()
{
    size_t i;
    int result;
    char out_buffer[OUT_BUFFER_SIZE];

    itl_string_t *str;
    string_result_t test;
    shift_test_case_t shift;

    string_result_t tests[] = {
        /* original, should_be */
        { "hello world sailor", "hello sailor" },
        { "это строка", "то строка" },
    };
    shift_test_case_t settings[] = {
        /* pos, count, backwards */
        { 12, 6, true },
        { 1, 1, true }
    };

    for (i = 0; i < countof(tests); ++i) {
        test = tests[i];
        str = itl_string_alloc();
        shift = settings[i];

        itl_string_from_cstr(str, test.original);
        itl_string_shift(str, shift.pos, shift.count, shift.backwards);
        itl_string_to_cstr(str, out_buffer, OUT_BUFFER_SIZE);

        result = strcmp(out_buffer, test.should_be);
        if (result != 0) {
            test_printf("Result %zu: '%s', should be: '%s'\n",
                        i, out_buffer, test.should_be);
            return false;
        }
    }

    return true;
}

static bool test_string_erase()
{
    size_t i;
    int result;
    char out_buffer[OUT_BUFFER_SIZE];

    itl_string_t *str;
    string_result_t test;
    shift_test_case_t erase;

    string_result_t tests[] = {
        /* original, should_be */
        { "hello world sailor", "hello sailor" },
        { "это строка", "то строка" },
        { "это строка", "это стр" },
        { "это строка", "это строка" },
        { "это строка", "это строка" }
    };
    shift_test_case_t settings[] = {
        /* pos, count, backwards */
        { 12, 6, true },
        { 0, 1, false },
        { 10, 3, true },
        { 10, 3, false },
        { 0, 0, true }
    };

    for (i = 0; i < countof(tests); ++i) {
        test = tests[i];
        str = itl_string_alloc();
        erase = settings[i];

        itl_string_from_cstr(str, test.original);
        itl_string_erase(str, erase.pos, erase.count, erase.backwards);
        itl_string_to_cstr(str, out_buffer, OUT_BUFFER_SIZE);

        result = strcmp(out_buffer, test.should_be);
        if (result != 0) {
            test_printf("Result %zu: '%s', should be: '%s'\n",
                        i, out_buffer, test.should_be);
            return false;
        }
    }

    return true;
}

static bool test_string_insert()
{
    int result;
    size_t i, pos;
    char out_buffer[OUT_BUFFER_SIZE];

    itl_string_t *str;
    string_result_t test;

    itl_utf8_t A = itl_utf8_new((uint8_t[4]){ 0x41 }, 1); 

    string_result_t tests[] = {
        /* original, should_be */
        { "hello, wrld", "hello, wArld" },
        { "hello, wrld", "hello, wrldA" },
        { "hello, world", "Ahello, world" }
    };
    size_t positions[] = {
        /* pos */
        8,
        11,
        0
    };

    for (i = 0; i < countof(tests); ++i) {
        test = tests[i];
        pos = positions[i];
        str = itl_string_alloc();

        itl_string_from_cstr(str, test.original);
        itl_string_insert(str, pos, A);
        itl_string_to_cstr(str, out_buffer, OUT_BUFFER_SIZE);

        result = strcmp(out_buffer, test.should_be);
        if (result != 0) {
            test_printf("Result %zu: '%s', should be: '%s'\n",
                        i, out_buffer, test.should_be);
            return false;
        }
    }

    return true;
}

typedef bool (*test_func)(void);

typedef struct test_case test_case_t;

struct test_case
{
    const char *name;
    test_func func;
};

#define DEFINE_TEST_CASE(fn) \
    {                        \
        .name = #fn,         \
        .func = fn,          \
    }

test_case_t test_cases[] = {
    DEFINE_TEST_CASE(test_string_from_cstr),
    DEFINE_TEST_CASE(test_string_shift),
    DEFINE_TEST_CASE(test_string_erase),
    DEFINE_TEST_CASE(test_string_insert)
};

int main(void) {
    size_t i;
    bool result;

    for (i = 0; i < countof(test_cases); ++i) {
        strncpy(current_function_name, test_cases[i].name, 32);
        result = test_cases[i].func();
        if (!result) {
            test_printf("*** FAIL.\n");
        } else {
            test_printf("ok.\n");
        }
    }

    return 0;
}
