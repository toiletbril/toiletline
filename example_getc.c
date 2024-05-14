#define TOILETLINE_IMPLEMENTATION
#include "toiletline.h"

#include <stdio.h>

/* Character buffer can be as large as 4 bytes, with \0 at the end */
#define CHAR_BUF_SIZE 5
#define MAX_CHARS     20

int
main(void)
{
  int  i = 0, code = 0;
  char char_buffer[CHAR_BUF_SIZE] = {0};

  if (tl_init() != TL_SUCCESS) {
    printf("Failed to enter raw mode!\n");
    return 1;
  }

  printf("Welcome to tl_getc example!\n"
         "Try to press keys while holding Control or Alt.\n"
         "You can also use non-latin keyboard layout.\n");

  while (code >= 0) {
    fflush(stdout);

    code = tl_getc(char_buffer, CHAR_BUF_SIZE, "> ");

    if (tl_last_control == TL_KEY_INTERRUPT) {
      printf("Interrupted.\n");
      break;
    }

    if (code == TL_PRESSED_CONTROL_SEQUENCE) {
      printf("Received control sequence. tl_last_control: %X\n",
             tl_last_control);
    } else {
      size_t j, size = strlen(char_buffer);

      printf("Received character: '%s' of of size %zu. Bytes:", char_buffer,
             size);

      for (j = 0; j < size; ++j) {
        printf(" %d", (uint8_t) char_buffer[j]);
      }
      fputc('\n', stdout);
    }

    if (i++ >= MAX_CHARS) {
      printf("Read %d characters, exiting!\n", MAX_CHARS);
      break;
    }
  }

  if (code < 0) {
    printf("An error occured (%d)\n", code);
  }

  fflush(stdout);
  tl_exit();

  return 0;
}
