CC ?=
CFLAGS := -Wall -Wextra -Werror -pedantic -std=c99
DBGFLAGS := -ggdb -O0 -fsanitize=address

default:
	@echo "Available targets: "
	@echo -n "\t"
	@cat Makefile | tail -n 1 | sed -r 's/^.{8}//;s/ /\n\t/g'

%: %.c
	@echo "\t" CC $(CFLAGS) $< -o $@
	@$(CC) $(CFLAGS) $< -o $@

test: CFLAGS += $(DBGFLAGS)
test: tests
	./tests

examples: example example_getc

clean:
	@echo "\t" RM ./example_getc ./example ./tests
	@rm -f ./example_getc ./example ./tests

.PHONY: default
.PHONY: test examples clean
