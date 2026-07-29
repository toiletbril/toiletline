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

ifeq ($(OS),Windows_NT)
%.exe: %.c toiletline.h
	$(CC) $(CFLAGS) $< -o $@
else
%: %.c toiletline.h
	$(CC) $(CFLAGS) $< -o $@
endif

ifeq ($(OS),Windows_NT)
test: tests.exe
	./tests.exe
else
test: tests
	./tests
endif

examples: CFLAGS += -O0
examples: example example_character

examples_debug: CFLAGS += -DTL_DEBUG -O0
examples_debug: clean example example_character

see_bytes: CFLAGS += -DTL_SEE_BYTES
see_bytes: clean example

ITEMS_TO_REMOVE :=
ifeq ($(OS),Windows_NT)
	ITEMS_TO_REMOVE = ./*.exe ./*.raddbg ./*.pdb ./*.exp ./*.lib ./*.ilk \
					  ./example_history.txt ./tl_test_*.txt
else
	ITEMS_TO_REMOVE = ./example_character ./example ./tests ./example_history.txt \
					  ./tl_test_*.txt
endif

clean:
	@echo "RM $(ITEMS_TO_REMOVE)"
	@rm -f $(ITEMS_TO_REMOVE)
	@rm -rf ./*.dSYM

.PHONY: default
.PHONY: cte cted test examples examples_debug see_bytes clean
