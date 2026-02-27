# Phase 00: Tooling

Before writing a single line of C, you need to own your tools.

## Install

```bash
sudo apt install gcc g++ make bear clangd gdb valgrind clang-format
```

## gcc flags you must know

| Flag | Purpose |
|------|---------|
| `-Wall -Wextra` | Enable all useful warnings (treat them as errors) |
| `-Werror` | Turn warnings into errors |
| `-g` | Include debug symbols (needed for gdb) |
| `-O2` | Optimize (use in production builds) |
| `-fsanitize=address,undefined` | Detect memory bugs + UB at runtime |
| `-std=c17` | Use C17 standard |

## The Makefile

```makefile
CC     = gcc
CFLAGS = -Wall -Wextra -Werror -g -std=c17

SRC = main.c
BIN = main

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

run: all
	./$(BIN)

clean:
	rm -f $(BIN)
```

## gdb quick reference

```bash
gdb ./main         # start debugger
(gdb) break main   # set breakpoint at main
(gdb) run          # run the program
(gdb) next         # step over
(gdb) step         # step into
(gdb) print x      # print variable x
(gdb) backtrace    # show call stack
(gdb) quit         # exit
```

## Neovim workflow

1. Open `ex01-hello.c` — LSP gives you diagnostics inline
2. `<leader>cb` to build, `<leader>cr` to run
3. `<F5>` to start debug session

## Exercises

- `ex01-hello.c`: compile with and without `-Wall`, see the difference
- `ex02-flags.c`: deliberately trigger a warning, fix it, run with sanitizer
