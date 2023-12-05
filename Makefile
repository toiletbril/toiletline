CC ?= clang
CFLAGS := -Wall -Wextra -Werror -pedantic -std=c99
DBGFLAGS := -ggdb -O0 -DITL_DEBUG

default:
	@echo "Available targets: test, examples, examples_debug, see_bytes, clean"

%: %.c
	@echo "CC $(CFLAGS) $< -o $@"
ifeq ($(OS),Windows_NT)
	@$(CC) $(CFLAGS) $< -o $@.exe
else
	@$(CC) $(CFLAGS) $< -o $@
endif

test: CFLAGS += $(DBGFLAGS)
test: tests
	./tests

examples: example example_getc

examples_debug: CFLAGS += $(DBGFLAGS)
examples_debug: examples

see_bytes: CFLAGS += "-DITL_SEE_BYTES"
see_bytes: example

clean:
	@echo "RM ./example_getc ./example ./tests"
	@rm -f ./example_getc ./example ./tests

.PHONY: default
.PHONY: test examples examples_debug see_bytes clean
