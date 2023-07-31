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
    while (code <= 0) {
        code = tl_getc(char_buffer, CHAR_BUF_SIZE, "> ");

        if (code == TL_PRESSED_CONTROL_SEQUENCE) {
            fputc('\n', stdout); // LF will not be displayed
            printf("Received control sequence %d\n", tl_last_control);
        }
        else {
            printf("Received character: '%s' of of size %zu\n",
                   char_buffer, strlen(char_buffer));
        }

        fflush(stdout);

        if (i++ >= 20) {
            printf("Read 20 characters, exiting!\n");
            break;
        }
    }

    if (code == TL_PRESSED_CTRLC)
        printf("\nInterrupted.\n");
    else if (code != TL_SUCCESS)
        printf("\nAn error occured.");

    tl_exit();

    return 0;
}
