#define TOILETLINE_IMPLEMENTATION
#include "toiletline.h"

// Characters can be of multiple bytes
#define CHAR_BUF_SIZE 8

int main(void)
{
    if (tl_init() != TL_SUCCESS)
        printf("Failed to enter raw mode!\n");

    printf("Welcome to toiletline test! This is tl_getc.\n");

    char char_buffer[CHAR_BUF_SIZE] = {0};
    int code = -1;

    int i = 0;
    while (code <= 0 && code != TL_PRESSED_CTRLC) {
        code = tl_getc(char_buffer, CHAR_BUF_SIZE, "> ");

        switch (code) {
            case TL_PRESSED_ENTER: {
                printf("Received Enter, control sequence %d\n", tl_last_control);
            } break;

            case TL_PRESSED_CTRLC: {
                // If a control character other than Enter was pressed, LF will not be displayed
                fputc('\n', stdout);
                printf("Interrupted.\n");
            } break;

            case TL_PRESSED_CONTROL_SEQUENCE: {
                fputc('\n', stdout);
                printf("Received control sequence %d\n", tl_last_control);
            } break;

            default: {
                printf("Received character: '%s' of of size %zu\n",
                        char_buffer, strlen(char_buffer));
            }
        }

        fflush(stdout);

        if (i++ >= 20) {
            printf("Read 20 characters, exiting!\n");
            break;
        }
    }

    if (code > 0)
        printf("\nAn error occured.");

    tl_exit();

    return 0;
}
