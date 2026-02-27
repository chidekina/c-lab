# c-lab Scaffold Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Scaffold the complete c-lab repository structure — phases, mysh skeleton, Neovim config, and all supporting files — ready for day-one learning.

**Architecture:** Hybrid curriculum repo where each phase folder contains a README + exercises, and the `mysh/` project grows incrementally alongside the phases. Neovim config lives in `.nvim/` and is auto-loaded via `exrc`.

**Tech Stack:** C17, C++17, gcc, make, bear, clangd, gdb, clang-format, GitHub

---

### Task 1: Initialize repo root files

**Files:**
- Create: `README.md`
- Create: `.gitignore`
- Create: `.clang-format`
- Create: `.clangd`

**Step 1: Create `.gitignore`**

```
# Binaries
*.o
*.out
*.a
a.out

# Build dirs
build/
bin/

# clangd generated
compile_commands.json
.cache/

# OS
.DS_Store
```

**Step 2: Create `.clang-format`**

```yaml
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
AllowShortFunctionsOnASingleLine: None
```

**Step 3: Create `.clangd`**

```yaml
CompileFlags:
  Add: [-Wall, -Wextra, -std=c17]
  CompilationDatabase: .
Diagnostics:
  UnusedIncludes: Strict
```

**Step 4: Create `README.md`**

```markdown
# c-lab

Learning C/C++ by building a mini shell (`mysh`) from scratch.

## Structure

| Directory | Purpose |
|-----------|---------|
| `phases/` | Structured curriculum — one folder per concept |
| `mysh/` | Anchor project — grows with each phase |
| `docs/` | Design docs and Neovim workflow guides |
| `.nvim/` | Project-local Neovim config (LSP + DAP) |

## Phases

| Phase | Concept | mysh milestone |
|-------|---------|----------------|
| 00 | Tooling (gcc, make, gdb) | Project setup |
| 01 | Types, UB, sanitizers | Input loop + echo |
| 02 | Pointers & strings | Command parser |
| 03 | Structs & enums | `struct Command`, tokenizer |
| 04 | Memory (heap/stack, valgrind) | fork + exec + pipes |
| 05 | C++ (classes, RAII, templates) | OO refactor + job control |

## Prerequisites

```bash
sudo apt install gcc g++ make bear clangd gdb valgrind clang-format
```

## Quick start

```bash
cd phases/00-tooling
make run
```
```

**Step 5: Commit**

```bash
git add .gitignore .clang-format .clangd README.md
git commit -m "chore: add root config files and README"
```

---

### Task 2: Neovim project config

**Files:**
- Create: `.nvim/init.lua`
- Create: `.nvim/keymaps.lua`

**Step 1: Enable exrc in your Neovim config (one-time)**

In your `~/.config/nvim/init.lua` (or equivalent), add:
```lua
vim.opt.exrc = true
vim.opt.secure = true
```

This makes Neovim auto-load `.nvim/init.lua` when you open the project.

**Step 2: Create `.nvim/init.lua`**

```lua
-- c-lab: project-local Neovim config
-- Auto-loaded when exrc = true

-- LSP: clangd
local ok, lspconfig = pcall(require, 'lspconfig')
if ok then
  lspconfig.clangd.setup({
    cmd = { 'clangd', '--background-index', '--clang-tidy', '--header-insertion=never' },
    on_attach = function(_, bufnr)
      local opts = { buffer = bufnr, silent = true }
      vim.keymap.set('n', 'gd', vim.lsp.buf.definition, opts)
      vim.keymap.set('n', 'gr', vim.lsp.buf.references, opts)
      vim.keymap.set('n', 'K', vim.lsp.buf.hover, opts)
      vim.keymap.set('n', '<leader>rn', vim.lsp.buf.rename, opts)
      vim.keymap.set('n', '<leader>ca', vim.lsp.buf.code_action, opts)
    end,
  })
end

-- DAP: gdb
local dap_ok, dap = pcall(require, 'dap')
if dap_ok then
  dap.adapters.gdb = {
    type = 'executable',
    command = 'gdb',
    args = { '--interpreter=dap', '--eval-command', 'set print pretty on' },
  }
  dap.configurations.c = {
    {
      name = 'Debug mysh',
      type = 'gdb',
      request = 'launch',
      program = '${workspaceFolder}/mysh/bin/mysh',
      cwd = '${workspaceFolder}/mysh',
      stopAtBeginningOfMainSubprogram = true,
    },
  }
  dap.configurations.cpp = dap.configurations.c
end

-- Load keymaps
local km_ok = pcall(dofile, vim.fn.getcwd() .. '/.nvim/keymaps.lua')
if not km_ok then
  vim.notify('[c-lab] keymaps.lua not loaded', vim.log.levels.WARN)
end
```

**Step 3: Create `.nvim/keymaps.lua`**

```lua
-- c-lab: build/run/debug keymaps
local map = function(mode, lhs, rhs, desc)
  vim.keymap.set(mode, lhs, rhs, { silent = true, desc = desc })
end

-- Build & run
map('n', '<leader>cb', ':!make -C mysh<CR>',            'C: build mysh')
map('n', '<leader>cr', ':!make -C mysh run<CR>',        'C: run mysh')
map('n', '<leader>ct', ':!make -C mysh test<CR>',       'C: test mysh')
map('n', '<leader>cv', ':!make -C mysh valgrind<CR>',   'C: valgrind mysh')

-- DAP debug
local dap_ok, dap = pcall(require, 'dap')
if dap_ok then
  map('n', '<F5>',  dap.continue,          'Debug: continue')
  map('n', '<F10>', dap.step_over,         'Debug: step over')
  map('n', '<F11>', dap.step_into,         'Debug: step into')
  map('n', '<F12>', dap.step_out,          'Debug: step out')
  map('n', '<leader>db', dap.toggle_breakpoint, 'Debug: toggle breakpoint')
end

-- Phase navigation
map('n', '<leader>p0', ':e phases/00-tooling/README.md<CR>',  'Phase 00')
map('n', '<leader>p1', ':e phases/01-types/README.md<CR>',    'Phase 01')
map('n', '<leader>p2', ':e phases/02-pointers/README.md<CR>', 'Phase 02')
map('n', '<leader>p3', ':e phases/03-structs/README.md<CR>',  'Phase 03')
map('n', '<leader>p4', ':e phases/04-memory/README.md<CR>',   'Phase 04')
map('n', '<leader>p5', ':e phases/05-cpp/README.md<CR>',      'Phase 05')
```

**Step 4: Commit**

```bash
git add .nvim/
git commit -m "feat: add project-local Neovim config (LSP + DAP)"
```

---

### Task 3: Phase 00 — Tooling

**Files:**
- Create: `phases/00-tooling/README.md`
- Create: `phases/00-tooling/Makefile`
- Create: `phases/00-tooling/exercises/ex01-hello.c`
- Create: `phases/00-tooling/exercises/ex02-flags.c`

**Step 1: Create `phases/00-tooling/README.md`**

````markdown
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
````

**Step 2: Create `phases/00-tooling/Makefile`**

```makefile
CC     = gcc
CFLAGS = -Wall -Wextra -Werror -g -std=c17
SAN    = -fsanitize=address,undefined

SRC = exercises/ex01-hello.c
BIN = ex01

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

san: $(SRC)
	$(CC) $(CFLAGS) $(SAN) -o $(BIN)-san $^
	./$(BIN)-san

run: all
	./$(BIN)

clean:
	rm -f $(BIN) $(BIN)-san
```

**Step 3: Create `phases/00-tooling/exercises/ex01-hello.c`**

```c
#include <stdio.h>

int main(void) {
    printf("Hello, C!\n");

    /* Exercise: add a variable 'x' of type int, print its value */
    /* Question: what does gcc -Wall tell you if you don't initialize it? */

    return 0;
}
```

**Step 4: Create `phases/00-tooling/exercises/ex02-flags.c`**

```c
#include <stdio.h>

/* Exercise: this file has intentional issues.
 * 1. Compile with: gcc -Wall -Wextra ex02-flags.c -o ex02
 * 2. Read each warning. Fix them one by one.
 * 3. Compile with: gcc -Wall -Wextra -fsanitize=address,undefined ex02-flags.c -o ex02 && ./ex02
 * 4. What does the sanitizer report?
 */

int add(int a, int b) {
    int result;          /* uninitialized — intentional */
    result = a + b;
    return result;
}

int main(void) {
    int arr[5] = {1, 2, 3, 4, 5};
    printf("sum: %d\n", add(2, 3));
    printf("arr[5] = %d\n", arr[5]);  /* out-of-bounds — intentional */
    return 0;
}
```

**Step 5: Commit**

```bash
git add phases/00-tooling/
git commit -m "feat: add phase 00 — tooling (gcc, make, gdb)"
```

---

### Task 4: Phases 01–05 README scaffolds

**Files:**
- Create: `phases/01-types/README.md` + `exercises/.gitkeep`
- Create: `phases/02-pointers/README.md` + `exercises/.gitkeep`
- Create: `phases/03-structs/README.md` + `exercises/.gitkeep`
- Create: `phases/04-memory/README.md` + `exercises/.gitkeep`
- Create: `phases/05-cpp/README.md` + `exercises/.gitkeep`

**Step 1: Create each README with this template (repeat for 01–05)**

For `phases/01-types/README.md`:
```markdown
# Phase 01: Types & Undefined Behavior

## Key concepts
- Fixed-width types: `uint8_t`, `int16_t`, `int32_t` (from `<stdint.h>`)
- `sizeof` and memory layout
- Undefined Behavior (UB) — the silent killer
- Detecting UB with `-fsanitize=address,undefined`

## Exercises
- [ ] ex01: print `sizeof` of all basic types
- [ ] ex02: trigger and detect UB with sanitizer
- [ ] ex03: rewrite using `uint8_t`/`int32_t` from `<stdint.h>`

## mysh milestone
After this phase: implement the **input loop** — read a line, print it back (echo builtin).
See `mysh/CHANGELOG.md` for the v1 target.
```

**Step 2: Create `.gitkeep` in each exercises/ folder**

```bash
touch phases/01-types/exercises/.gitkeep
touch phases/02-pointers/exercises/.gitkeep
touch phases/03-structs/exercises/.gitkeep
touch phases/04-memory/exercises/.gitkeep
touch phases/05-cpp/exercises/.gitkeep
```

**Step 3: Commit**

```bash
git add phases/
git commit -m "feat: scaffold phases 01-05 README stubs"
```

---

### Task 5: mysh skeleton

**Files:**
- Create: `mysh/Makefile`
- Create: `mysh/README.md`
- Create: `mysh/CHANGELOG.md`
- Create: `mysh/include/mysh.h`
- Create: `mysh/src/main.c`

**Step 1: Create `mysh/include/mysh.h`**

```c
#ifndef MYSH_H
#define MYSH_H

#define MYSH_VERSION "0.1.0"
#define MAX_INPUT    1024
#define MAX_ARGS     64

/* Read a line from stdin into buf (max MAX_INPUT bytes).
 * Returns number of bytes read, or -1 on EOF. */
int mysh_readline(char *buf, int size);

/* Split buf into tokens (space-separated).
 * Writes pointers into argv, returns argc.
 * argv must have room for MAX_ARGS pointers. */
int mysh_tokenize(char *buf, char **argv);

/* Execute a builtin command. Returns 1 if handled, 0 if not a builtin. */
int mysh_builtin(int argc, char **argv);

#endif /* MYSH_H */
```

**Step 2: Create `mysh/src/main.c`**

```c
#include <stdio.h>
#include <stdlib.h>
#include "mysh.h"

int main(void) {
    char  buf[MAX_INPUT];
    char *argv[MAX_ARGS];
    int   argc;

    printf("mysh v%s — type 'exit' to quit\n", MYSH_VERSION);

    while (1) {
        printf("mysh> ");
        fflush(stdout);

        if (mysh_readline(buf, MAX_INPUT) < 0)
            break;  /* EOF (Ctrl+D) */

        argc = mysh_tokenize(buf, argv);
        if (argc == 0)
            continue;

        if (mysh_builtin(argc, argv))
            continue;

        /* Phase 04: fork + exec goes here */
        fprintf(stderr, "mysh: %s: command not found\n", argv[0]);
    }

    return 0;
}
```

**Step 3: Create `mysh/Makefile`**

```makefile
CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -g -std=c17 -Iinclude
SAN     = -fsanitize=address,undefined
SRCDIR  = src
INCDIR  = include
BINDIR  = bin
BIN     = $(BINDIR)/mysh

SRCS    = $(wildcard $(SRCDIR)/*.c)
OBJS    = $(SRCS:.c=.o)

all: $(BINDIR) $(BIN)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: all
	./$(BIN)

valgrind: all
	valgrind --leak-check=full --track-origins=yes ./$(BIN)

san: $(BINDIR)
	$(CC) $(CFLAGS) $(SAN) -o $(BINDIR)/mysh-san $(SRCS)
	./$(BINDIR)/mysh-san

clean:
	rm -f $(SRCDIR)/*.o $(BIN) $(BINDIR)/mysh-san

.PHONY: all run valgrind san clean
```

**Step 4: Create `mysh/README.md`**

```markdown
# mysh — mini shell

A minimal Unix shell built incrementally as part of `c-lab`.

## Build

```bash
make          # build
make run      # build and run
make valgrind # check for memory leaks
make san      # build with address + UB sanitizers
make clean    # remove binaries
```

## Architecture

```
mysh/
├── include/mysh.h   # all function declarations
├── src/main.c       # main loop
├── src/readline.c   # input reading
├── src/tokenize.c   # string splitting
├── src/builtins.c   # echo, cd, pwd, exit
├── src/exec.c       # fork + exec (phase 04)
└── src/jobs.c       # job control (phase 05)
```

## Versions

| Version | Features | Phase |
|---------|----------|-------|
| v0.1 | Input loop skeleton | 00 |
| v1.0 | Builtins: echo, pwd, cd, exit | 01-02 |
| v2.0 | Pipes, redirections, external cmds | 03-04 |
| v3.0 | Job control, signals, C++ refactor | 05 |
```

**Step 5: Create `mysh/CHANGELOG.md`**

```markdown
# mysh Changelog

## [Unreleased] v1.0.0
### Target (phases 01-02)
- [ ] `mysh_readline`: read input line
- [ ] `mysh_tokenize`: split by spaces
- [ ] Builtin: `echo`
- [ ] Builtin: `pwd`
- [ ] Builtin: `cd`
- [ ] Builtin: `exit`

## [0.1.0] — phase 00
- Project skeleton: main loop, Makefile, headers
```

**Step 6: Verify the skeleton compiles**

```bash
cd mysh && make
```

Expected output: `bin/mysh` created with no warnings.

**Step 7: Commit**

```bash
git add mysh/
git commit -m "feat: add mysh skeleton (main loop + Makefile)"
```

---

### Task 6: Nvim workflow docs

**Files:**
- Create: `docs/nvim-workflows/00-setup.md`
- Create: `docs/nvim-workflows/01-navigation.md`
- Create: `docs/nvim-workflows/02-debug.md`
- Create: `docs/nvim-workflows/03-refactor.md`

**Step 1: Create `docs/nvim-workflows/00-setup.md`**

````markdown
# Neovim Setup for C/C++

## 1. Install tools

```bash
sudo apt install clangd bear gdb
```

## 2. Enable exrc in Neovim

Add to `~/.config/nvim/init.lua`:
```lua
vim.opt.exrc = true
vim.opt.secure = true
```

## 3. Generate compile_commands.json

`clangd` needs this file to understand your project. Generate it with `bear`:

```bash
cd mysh
bear -- make clean all
# This creates compile_commands.json in mysh/
```

Move it to the root so clangd finds it from anywhere:
```bash
mv compile_commands.json ../
```

## 4. Open the project

```bash
nvim .
```

Neovim will auto-load `.nvim/init.lua`. You should see clangd start indexing in the status bar.

## 5. Verify LSP is working

Open `mysh/src/main.c`, hover over `mysh_readline` and press `K`.
You should see the function signature from the header file.
````

**Step 2: Create `docs/nvim-workflows/01-navigation.md`**

```markdown
# Neovim: Code Navigation

| Keymap | Action |
|--------|--------|
| `gd` | Go to definition |
| `gr` | Show all references |
| `K` | Hover documentation |
| `gi` | Go to implementation |
| `<leader>rn` | Rename symbol |
| `<C-o>` | Jump back |
| `<C-i>` | Jump forward |
| `<leader>p0`–`p5` | Open phase README |

## Telescope (if installed)

| Keymap | Action |
|--------|--------|
| `<leader>fs` | Find symbol in workspace |
| `<leader>ff` | Find file |
| `<leader>fg` | Live grep |
```

**Step 3: Create `docs/nvim-workflows/02-debug.md`**

```markdown
# Neovim: Debugging with gdb + DAP

## Prerequisites

Install `nvim-dap` and optionally `nvim-dap-ui`:
```lua
-- In your plugin manager
{ "mfussenegger/nvim-dap" }
{ "rcarriga/nvim-dap-ui" }
```

## Workflow

1. Build with debug symbols: `<leader>cb` (runs `make`)
2. Set a breakpoint: `<leader>db` on the line you want to pause
3. Start debug: `<F5>`
4. Step over: `<F10>` | Step into: `<F11>` | Step out: `<F12>`
5. In DAP REPL: `p variable_name` to inspect values

## gdb in terminal (alternative)

```bash
gdb ./mysh/bin/mysh
(gdb) break mysh_readline
(gdb) run
(gdb) print buf
(gdb) next
```
```

**Step 4: Create `docs/nvim-workflows/03-refactor.md`**

```markdown
# Neovim: Refactoring

## Rename symbol

Place cursor on any symbol, press `<leader>rn`, type the new name.
clangd renames all references across files automatically.

## Format on save

Add to `.nvim/init.lua`:
```lua
vim.api.nvim_create_autocmd('BufWritePre', {
  pattern = { '*.c', '*.h', '*.cpp', '*.hpp' },
  callback = function() vim.lsp.buf.format({ async = false }) end,
})
```

`.clang-format` in the root controls the style.

## Code actions

`<leader>ca` — shows quick fixes (missing includes, unused vars, etc.)
```

**Step 5: Commit**

```bash
git add docs/nvim-workflows/
git commit -m "docs: add Neovim workflow guides (setup, navigation, debug, refactor)"
```

---

### Task 7: Create GitHub repo and push

**Step 1: Create repo on GitHub**

```bash
gh repo create c-lab --public --description "Learning C/C++ by building a mini shell — with Neovim LSP/DAP setup"
```

**Step 2: Add remote and push**

```bash
cd /home/hidekina/projetos/c-lab
git remote add origin https://github.com/<your-username>/c-lab.git
git push -u origin main
```

**Step 3: Verify**

```bash
gh repo view --web
```

---

## Done ✅

At this point you have:
- Complete directory structure with all phases scaffolded
- `mysh` skeleton that compiles cleanly
- Neovim config with LSP (clangd) + DAP (gdb) + keymaps
- Workflow guides for navigation, debug, refactor
- Public GitHub repo

**Next step:** Open `phases/00-tooling/README.md` and start Phase 00.
