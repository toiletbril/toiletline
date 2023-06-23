#define TOILETLINE_IMPLEMENTATION
#include "toiletline.h"

#define LINE_BUF_LEN 32

int main(void)
{
    if (!tl_init())
        printf("Failed to enter raw mode!\n");

    printf("Welcome to toiletline test! This is raw mode!\nUse up and down arrows to view history.\n");

    char line_buffer[LINE_BUF_LEN] = {0};
    int code;

    int i = 0;
    while ((code = tl_readline(line_buffer, LINE_BUF_LEN, "$ ")) == 0) {
        printf("Received string: '%s'\n", line_buffer);
        fflush(stdout);
        if (i++ >= 10) {
            printf("Reached 10 messages, exiting!\n");
            break;
        }
    }

    if (code == 1)
        printf("\nInterrupted.\n");
    else if (code != 0)
        printf("\nAn error occured.");

    tl_exit();

    return 0;
}
