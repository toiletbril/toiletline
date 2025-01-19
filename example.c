#define TOILETLINE_IMPLEMENTATION
#include "toiletline.h"

#include <stdio.h>

#define LINE_BUF_SIZE 1024
#define HISTORY_FILE  "example_history.txt"
#define MAX_MESSAGES  10

int
main(void)
{
  int  i = 0, code = 0;
  char line_buffer[LINE_BUF_SIZE] = {0};

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

  tl_history_load(HISTORY_FILE);

  while (code >= 0) {
    fflush(stdout);

    switch (i) {
    case 0: tl_set_predefined_input("erase me :3c"); break;
    case 1: tl_set_predefined_input("я снова тут!"); break;
    case 2: tl_set_predefined_input("leaving soon..."); break;
    }

    code = tl_get_input(line_buffer, LINE_BUF_SIZE, "$ ");
    tl_emit_newlines(line_buffer);

    if (code == TL_PRESSED_INTERRUPT || code == TL_PRESSED_EOF) {
      printf("Interrupted.\n");
      break;
    }

    printf("Received string: '%s' of length %zu, of size %zu\n", line_buffer,
           tl_utf8_strlen(line_buffer), strlen(line_buffer));

    if (i++ >= MAX_MESSAGES) {
      printf("Reached %d messages, exiting!\n", MAX_MESSAGES);
      break;
    }
  }

  if (code < 0) {
    printf("An error occured (%d)\n", code);
  }

  fflush(stdout);
  tl_history_dump(HISTORY_FILE);
  tl_exit();

  return 0;
}
