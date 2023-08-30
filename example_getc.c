#define TOILETLINE_IMPLEMENTATION
#include "toiletline.h"

// Character buffer can be as large as 4 bytes, plus \0 at the end
#define CHAR_BUF_SIZE 5

int main(void)
{
    if (tl_init() != TL_SUCCESS) {
        printf("Failed to enter raw mode!\n");
        return 1;
    }

    printf("Welcome to tl_getc example!\nTry to press keys while holding Control or Alt.\nYou can also use non-latin keyboard layout.\n");

    char char_buffer[CHAR_BUF_SIZE] = {0};
    int code = -1;

    int i = 0;
    while (code <= 0) {
        code = tl_getc(char_buffer, CHAR_BUF_SIZE, "> ");

        if (code == TL_PRESSED_CTRLC) {
            printf("\nInterrupted.\n");
            break;
        }

        // Note that if a control character other than Enter was pressed,
        // LF will not be displayed
        switch (code) {
            case TL_PRESSED_ENTER: {
                printf("Received Enter, control sequence %d\n", tl_last_control);
            } break;

            case TL_PRESSED_CONTROL_SEQUENCE: {
                printf("\nReceived control sequence %d\n", tl_last_control);
            } break;

            default: {
                size_t size = strlen(char_buffer);

                printf("Received character: '%s' of of size %zu. Bytes:",
                        char_buffer, size);

                for (size_t i = 0; i < size; ++i)
                    printf(" %d", (uint8_t)char_buffer[i]);

                fputc('\n', stdout);
            }
        }

        fflush(stdout);

        if (i++ >= 20) {
            printf("Read 20 characters, exiting!\n");
            break;
        }
    }

    if (code > 0)
        printf("An error occured.\n");

    fflush(stdout);

    tl_exit();

    return 0;
}
