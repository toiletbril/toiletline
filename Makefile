CC ?=
CFLAGS := -Wall -Wextra -Werror -pedantic -std=c99
DBGFLAGS := -ggdb -O0 -fsanitize=address

default:
	@echo "Available targets: test, examples, clean"

%: %.c
	@echo "\t" CC $(CFLAGS) $< -o $@
ifeq ($(OS),Windows_NT)
	@$(CC) $(CFLAGS) $< -o $@.exe
else
	@$(CC) $(CFLAGS) $< -o $@
endif

test: CFLAGS += $(DBGFLAGS)
test: tests
	./tests

examples: example example_getc

clean:
	@echo "\t" RM ./example_getc ./example ./tests
	@rm -f ./example_getc ./example ./tests

.PHONY: default
.PHONY: test examples clean
