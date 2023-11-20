#define TOILETLINE_IMPLEMENTATION
#include "toiletline.h"

#define OUT_BUFFER_SIZE 128

int test_string_from_cstr()
{
    char original_cstr[] = "привет, мир";
    itl_string_t *str = itl_string_alloc();
    itl_string_from_cstr(str, original_cstr);

    char out_buffer[OUT_BUFFER_SIZE];
    itl_string_to_cstr(str, out_buffer, OUT_BUFFER_SIZE);

    int result = strcmp(out_buffer, original_cstr);

    if (result) {
        printf("*** Result: '%s', should be: '%s'\n",
               out_buffer, original_cstr);
    }

    if (result == 0 && str->size == 20 && str->length == 11)
        return 0;
    else
        return 1;
}

int test_string_shift_backward()
{
    char original_cstr[] = "hello world sailor";
    itl_string_t *str = itl_string_alloc();
    itl_string_from_cstr(str, original_cstr);

    itl_string_shift(str, 11, 7, 1);

    char out_buffer[OUT_BUFFER_SIZE];
    itl_string_to_cstr(str, out_buffer, OUT_BUFFER_SIZE);

    char should_be[] = "hello sailor";
    int result = strcmp(out_buffer, should_be);

    if (result) {
        printf("*** Result: '%s', should be: '%s'\n",
               out_buffer, should_be);
    }

    return result;
}

int test_string_shift_forward()
{
    char original_cstr[] = "hello world 69ilor";
    itl_string_t *str = itl_string_alloc();
    itl_string_from_cstr(str, original_cstr);

    itl_string_shift(str, 11, 2, 0);

    char out_buffer[OUT_BUFFER_SIZE];
    itl_string_to_cstr(str, out_buffer, OUT_BUFFER_SIZE);

    char should_be[] = "hello world 6969ilor";
    int result = strcmp(out_buffer, should_be);

    if (result) {
        printf("*** Result: '%s', should be: '%s'\n",
               out_buffer, should_be);
    }

    return result;
}

typedef int (*test_func)(void);

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
    for (size_t i = 0; i < TEST_CASES_COUNT; ++i) {
        int result = test_cases[i].func();

        printf("> Test #%zu '%s' ", i, test_cases[i].name);
        if (result) printf("failed.\n");
        else printf("passed.\n");
    }

    return 0;
}
