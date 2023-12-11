#define TOILETLINE_IMPLEMENTATION
#include "toiletline.h"

#define LINE_BUF_SIZE 1024

int main(void)
{
    if (tl_init() != TL_SUCCESS) {
        printf("Failed to enter raw mode!\n");
        return 1;
    }

    printf("Welcome to tl_readline example!\nUse up and down arrows to view "
           "history.\n");
#if defined _WIN32
    printf("NOTE: On Windows, UTF-8 feature is required for multibyte "
           "character support.\n");
#endif

    /* first         what           wow
         |          /    \
       second    other  something
         |
       third                      */

    tl_add_completion(NULL, "wow");
    void *first_completion = tl_add_completion(NULL, "first");
    void *what_completion = tl_add_completion(NULL, "what");
    tl_add_completion(what_completion, "other");
    tl_add_completion(what_completion, "something");
    void *second_completion = tl_add_completion(first_completion, "second");
    tl_add_completion(second_completion, "third");

    char line_buffer[LINE_BUF_SIZE] = {0};
    int code = 0;

    int i = 0;
    while (code >= 0) {
        switch (i) {
            case 0: tl_setline("erase me :3c"); break;
            case 1: tl_setline("я снова тут!"); break;
            case 2: tl_setline("leaving soon..."); break;
        }

        code = tl_readline(line_buffer, LINE_BUF_SIZE, "$ ");

        if (code == TL_PRESSED_INTERRUPT || code == TL_PRESSED_EOF) {
            printf("\nInterrupted.\n");
            break;
        }

        printf("\nReceived string: '%s' of length %zu, of size %zu\n",
               line_buffer, tl_utf8_strlen(line_buffer), strlen(line_buffer));

        fflush(stdout);

        if (i++ >= 10) {
            printf("Reached 10 messages, exiting!\n");
            break;
        }
    }

    if (code < 0)
        printf("An error occured (%d)\n", code);

    fflush(stdout);

    tl_exit();

    return 0;
}
