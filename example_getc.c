#define TOILETLINE_IMPLEMENTATION
#include "toiletline.h"

#include <stdio.h>

// Character buffer can be as large as 4 bytes, plus \0 at the end
#define CHAR_BUF_SIZE 5

int main(void)
{
    if (tl_init() != TL_SUCCESS) {
        printf("Failed to enter raw mode!\n");
        return 1;
    }

    printf("Welcome to tl_getc example!\n"
           "Try to press keys while holding Control or Alt.\n"
           "You can also use non-latin keyboard layout.\n");

    char char_buffer[CHAR_BUF_SIZE] = {0};
    int code = 0;

    int i = 0;
    while (code >= 0) {
        code = tl_getc(char_buffer, CHAR_BUF_SIZE, "> ");

        if (tl_last_control == TL_KEY_INTERRUPT) {
            printf("\nInterrupted.\n");
            break;
        }

        switch (code) {
            case TL_PRESSED_CONTROL_SEQUENCE: {
                printf("\nReceived control sequence. tl_last_control: %X\n",
                       tl_last_control);
            } break;

            default: {
                size_t size = strlen(char_buffer);

                printf("\nReceived character: '%s' of of size %zu. Bytes:",
                       char_buffer, size);

                for (size_t j = 0; j < size; ++j)
                    printf(" %d", (uint8_t)char_buffer[j]);

                fputc('\n', stdout);
            }
        }

        fflush(stdout);

        if (i++ >= 20) {
            printf("Read 20 characters, exiting!\n");
            break;
        }
    }

    if (code < 0)
        printf("An error occured (%d)\n", code);

    fflush(stdout);

    tl_exit();

    return 0;
}
