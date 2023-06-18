#define TOILETLINE_IMPL
#include "toiletline.c"

#define LINE_BUF_LEN 1024

int main(void)
{
    if (!tl_init()) {
        printf("Failed to enter raw mode!\n");
        return 1;
    }

    printf("Welcome to toiletline test! This is raw mode! You can use up and down arrow to scroll history.\n");

    char line_buffer[LINE_BUF_LEN];

    while (tl_readline(line_buffer, LINE_BUF_LEN) == 0) {
        printf("\nreceived string: '%s'\n", line_buffer);
        fflush(stdout);
    }

    tl_exit();

    return 0;
}