#define TOILETLINE_IMPLEMENTATION
#include "toiletline.h"

#define OUT_BUFFER_SIZE 128

static bool test_string_from_cstr()
{
    char original_cstr[] = "привет, мир";
    char out_buffer[OUT_BUFFER_SIZE];
    int result;

    itl_string_t *str = itl_string_alloc();

    itl_string_from_cstr(str, original_cstr);
    itl_string_to_cstr(str, out_buffer, OUT_BUFFER_SIZE);

    result = strcmp(out_buffer, original_cstr);
    if (result == 0)
        return true;
    else {
        printf("*** Result: '%s', should be: '%s'\n",
               out_buffer, original_cstr);
        return false;
    }
}

static bool test_string_shift_backward()
{
    char original_cstr[] = "hello world sailor";
    char should_be[] = "hello sailor";
    char out_buffer[OUT_BUFFER_SIZE];

    itl_string_t *str = itl_string_alloc();
    itl_string_from_cstr(str, original_cstr);
    itl_string_shift(str, 11, 7, 1);
    itl_string_to_cstr(str, out_buffer, OUT_BUFFER_SIZE);

    int result = strcmp(out_buffer, should_be);
    if (result != 0) {
        printf("*** Result: '%s', should be: '%s'\n",
               out_buffer, should_be);
        return false;
    }

    return true;
}

static bool test_string_shift_forward()
{
    char original_cstr[] = "hello world 69ilor";
    char should_be[] = "hello world 6969ilor";
    char out_buffer[OUT_BUFFER_SIZE];

    itl_string_t *str = itl_string_alloc();

    itl_string_from_cstr(str, original_cstr);
    itl_string_shift(str, 11, 2, 0);
    itl_string_to_cstr(str, out_buffer, OUT_BUFFER_SIZE);

    int result = strcmp(out_buffer, should_be);
    if (result != 0) {
        printf("*** Result: '%s', should be: '%s'\n",
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
    bool result;
    size_t i;

    for (i = 0; i < TEST_CASES_COUNT; ++i) {
        result = test_cases[i].func();

        printf("> Test #%zu '%s'", i, test_cases[i].name);
        if (!result)
            printf("failed.\n");
        else
            printf("...");
    }

    return 0;
}
