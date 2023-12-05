#define TOILETLINE_IMPLEMENTATION
#include "toiletline.h"

#define OUT_BUFFER_SIZE 128

static char current_function_name[128];

#define test_printf(...)                  \
  do {                                    \
    fputs(current_function_name, stdout); \
    printf(": "__VA_ARGS__);              \
  } while (0)

static bool test_string_from_cstr()
{
    int result;
    itl_string_t *str;
    char out_buffer[OUT_BUFFER_SIZE];

    char original_cstr[] = "привет, мир";

    str = itl_string_alloc();
    itl_string_from_cstr(str, original_cstr);
    itl_string_to_cstr(str, out_buffer, OUT_BUFFER_SIZE);

    result = strcmp(out_buffer, original_cstr);
    if (result != 0) {
        test_printf("Result: '%s', should be: '%s'\n",
                    out_buffer, original_cstr);
        return false;
    }

    return true;
}

static bool test_string_shift_backward()
{
    int result;
    size_t pos;
    itl_string_t *str;
    char out_buffer[OUT_BUFFER_SIZE];

    char original_cstr[] = "hello world sailor";
    char should_be[] = "hello sailor";

    str = itl_string_alloc();
    pos = strrchr(original_cstr, 's') - original_cstr;

    itl_string_from_cstr(str, original_cstr);
    itl_string_shift(str, pos, 6, true);
    itl_string_to_cstr(str, out_buffer, OUT_BUFFER_SIZE);

    result = strcmp(out_buffer, should_be);
    if (result != 0) {
        test_printf("Result: '%s', should be: '%s'\n",
                    out_buffer, should_be);
        return false;
    }

    return true;
}

static bool test_string_shift_forward()
{
    int result;
    size_t pos;
    itl_string_t *str;
    char out_buffer[OUT_BUFFER_SIZE];

    char original_cstr[] = "hello world 69ilor";
    char should_be[] = "hello world 6969ilor";

    str = itl_string_alloc();
    pos = strrchr(original_cstr, '6') - original_cstr;

    itl_string_from_cstr(str, original_cstr);
    itl_string_shift(str, pos, 2, false);
    itl_string_to_cstr(str, out_buffer, OUT_BUFFER_SIZE);

    result = strcmp(out_buffer, should_be);
    if (result != 0) {
        test_printf("Result: '%s', should be: '%s'\n",
                    out_buffer, should_be);
        return false;
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
    DEFINE_TEST_CASE(test_string_shift_backward),
    DEFINE_TEST_CASE(test_string_shift_forward),
};

#define TEST_CASES_COUNT \
    (sizeof(test_cases)/sizeof(test_cases[0]))

int main(void) {
    size_t i;
    bool result;

    for (i = 0; i < TEST_CASES_COUNT; ++i) {
        strncpy(current_function_name, test_cases[i].name, 32);
        result = test_cases[i].func();
        if (!result) {
            test_printf("FAIL.\n");
        } else {
            test_printf("ok.\n");
        }
    }

    return 0;
}
