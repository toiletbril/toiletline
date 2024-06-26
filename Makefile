CC ?= clang
CFLAGS := -g3 -Wall -Wextra -Wconversion -Wno-sign-conversion -Wdouble-promotion \
		  -Werror -pedantic -std=c99

ifneq ($(OS),Windows_NT)
CFLAGS += -fsanitize=address -fsanitize=undefined
endif

default:
	@echo "Available targets: test, examples, examples_debug, see_bytes, clean"

cte: clean test examples
cted: clean test examples_debug

%: %.c
ifeq ($(OS),Windows_NT)
	$(CC) $(CFLAGS) $< -o $@.exe
else
	$(CC) $(CFLAGS) $< -o $@
endif

test: tests
ifeq ($(OS),Windows_NT)
	./tests.exe
else
	./tests
endif

examples: CFLAGS += -O0
examples: example example_getc

examples_debug: CFLAGS += -DTL_DEBUG -O0
examples_debug: example example_getc

see_bytes: CFLAGS += -DTL_SEE_BYTES
see_bytes: example

ITEMS_TO_REMOVE :=
ifeq ($(OS),Windows_NT)
	ITEMS_TO_REMOVE = ./*.exe ./*.raddbg ./*.pdb ./*.exp ./*.lib ./*.ilk \
					  ./example_history.txt
else
	ITEMS_TO_REMOVE = ./example_getc ./example ./tests ./example_history.txt
endif

clean:
	@echo "RM $(ITEMS_TO_REMOVE)"
	@rm -f $(ITEMS_TO_REMOVE)

.PHONY: default
.PHONY: test examples examples_debug see_bytes clean
