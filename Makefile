CC ?= clang
CFLAGS := -g3 -Wall -Wextra -Wconversion -Wdouble-promotion -Werror -pedantic
DBGFLAGS := -O0 -DITL_DEBUG

ifneq ($(OS),Windows_NT)
	CFLAGS += -fsanitize=undefined
	DBGFLAGS += -fsanitize=address
endif

default:
	@echo "Available targets: test, examples, examples_debug, see_bytes, clean"

cte: clean test examples
cted: clean test examples_debug

%: %.c
	@echo "CC $< -o $@"
ifeq ($(OS),Windows_NT)
	@$(CC) $(CFLAGS) $< -o $@.exe
else
	@$(CC) $(CFLAGS) $< -o $@
endif

test: tests
ifeq ($(OS),Windows_NT)
	./tests.exe
else
	./tests
endif

examples: CFLAGS += -std=c89
examples: example example_getc

examples_debug: CFLAGS += $(DBGFLAGS)
examples_debug: examples

see_bytes: CFLAGS += -DITL_SEE_BYTES
see_bytes: example

ITEMS_TO_REMOVE :=
ifeq ($(OS),Windows_NT)
	ITEMS_TO_REMOVE = ./*.exe ./*.raddbg ./*.pdb ./*.exp ./*.lib ./*.ilk ./example_history.txt
else
	ITEMS_TO_REMOVE = ./example_getc ./example ./tests ./example_history.txt
endif

clean:
	@echo "RM $(ITEMS_TO_REMOVE)"
	@rm -f $(ITEMS_TO_REMOVE)

.PHONY: default
.PHONY: test examples examples_debug see_bytes clean
