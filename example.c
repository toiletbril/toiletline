#define TOILETLINE_IMPLEMENTATION
#include "toiletline.h"

#define LINE_BUF_SIZE 128

int main(void)
{
    if (tl_init() != TL_SUCCESS) {
        printf("Failed to enter raw mode!\n");
        return 1;
    }

    printf("Welcome to toiletline test! This is tl_readline.\nUse up and down arrows to view history.\n");

    char line_buffer[LINE_BUF_SIZE] = {0};
    int code = -1;

    int i = 0;
    while (code < 0) {
        code = tl_readline(line_buffer, LINE_BUF_SIZE, "$ ");

        if (code == TL_PRESSED_CTRLC) {
            printf("\nInterrupted.\n");
            break;
        }

        printf("Received string: '%s' of length %zu, of size %zu\n",
                line_buffer, tl_utf8_strlen(line_buffer), strlen(line_buffer));

        fflush(stdout);

        if (i++ >= 10) {
            printf("Reached 10 messages, exiting!\n");
            break;
        }
    }

    if (code > 0)
        printf("An error occured.\n");

    tl_exit();

    return 0;
}
