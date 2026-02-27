# Phase 00: Tooling — Walkthrough

Step-by-step guide to completing the exercises in this phase, entirely from inside Neovim.

---

## Prerequisites

Before opening any file, verify the tools are installed. Open a terminal **inside Neovim**:

```
:terminal
```

Then run:

```bash
which gcc g++ make clangd gdb valgrind
gcc --version
clangd --version
```

If anything is missing:

```bash
sudo apt install gcc g++ make bear clangd gdb valgrind clang-format
```

> **Nvim tip:** `:terminal` opens a full terminal buffer in the current window. Use `:split | terminal` to open it in a horizontal split so you keep your editor visible. Press `i` to enter terminal mode, `<C-\><C-n>` to return to normal mode.

---

## Exercise 1 — ex01-hello.c

### Step 1: Open the file from inside Neovim

From the project root (or from the Neovim session you already have open), run:

```
:e phases/00-tooling/exercises/ex01-hello.c
```

Or navigate there using the phase shortcut:

```
<leader>p0
```

That opens `phases/00-tooling/README.md`. From there, you can use `gf` on the filename or just `:e exercises/ex01-hello.c`.

> **Nvim tip:** `<leader>p0` is mapped to jump directly to this phase's README. Use it whenever you need to check the exercise brief without leaving Neovim.

The file looks like this:

```c
#include <stdio.h>

int main(void) {
    printf("Hello, C!\n");

    /* Exercise: add a variable 'x' of type int, print its value */
    /* Question: what does gcc -Wall tell you if you don't initialize it? */

    return 0;
}
```

### Step 2: Build and run without leaving Neovim

Save the file with `:w` (or it will be saved automatically before build), then:

```
<leader>cb   ← runs: make -C mysh
<leader>cr   ← runs: make -C mysh run
```

The output appears in a floating terminal window managed by the keymap. You should see:

```
Hello, C!
```

> **Nvim tip:** `<leader>cb` and `<leader>cr` are your primary build/run shortcuts in this project. You never need to leave Neovim to compile and test — use them after every edit.

### Step 3: Watch LSP diagnostics appear inline as you type

Now edit the file. Add these two lines inside `main`, before `return 0;`:

```c
int x;
printf("x = %d\n", x);
```

**Do not save yet.** As soon as you finish typing `x;`, clangd (the LSP server) will underline `x` in the `printf` call with a warning squiggle — without you running gcc at all.

To read the diagnostic message, hover over the squiggle:

```
K    ← show hover documentation / diagnostic detail
```

Or jump to the next diagnostic in the file:

```
]d   ← jump to next diagnostic
[d   ← jump to previous diagnostic
```

> **Nvim tip:** clangd is attached automatically when you open a `.c` or `.h` file in this project (configured in `.nvim/init.lua`). The diagnostics you see inline are the same warnings that `-Wall -Wextra` produces at compile time — you get them as you type, before any build.

### Step 4: Compile without flags — no warnings

Save with `:w` (auto-format via clang-format runs on save). Now build:

```
<leader>cb
```

This runs `make -C mysh`, which uses the project's Makefile with full `-Wall -Wextra -Werror -g -std=c17` flags. The build will **fail** because of the uninitialized variable — that is the correct behavior.

To understand what each flag does:

| Flag | What it does |
|------|-------------|
| `-Wall` | Enables all important warnings |
| `-Wextra` | Enables additional warnings that `-Wall` misses |
| `-Werror` | Turns warnings into errors — build fails |
| `-g` | Includes debug symbols (required for gdb/DAP) |
| `-std=c17` | Enforces the C17 standard |

> **Nvim tip:** Auto-format on save is configured via `BufWritePre` in `.nvim/init.lua` — it runs `clang-format` on every `*.c`, `*.h`, `*.cpp`, and `*.hpp` file before writing. Your code is always consistently formatted without any manual step.

### Step 5: Understand Undefined Behavior — then fix it

Reading an uninitialized variable in C is **Undefined Behavior**: the value can be anything, including sensitive memory left over from a previous function call. With `-Werror`, the compiler refuses to build rather than silently produce a broken binary.

Fix it by initializing: `int x = 42;`

After the edit, `]d` should show no more diagnostics. Build again:

```
<leader>cb
<leader>cr
```

Expected output:

```
Hello, C!
x = 42
```

### Step 6: Open the Makefile side by side

```
:vsplit Makefile
```

Use `<C-w>h` and `<C-w>l` to move between the exercise file and the Makefile.

> **Nvim tip:** `:vsplit <file>` opens a file in a vertical split. `<C-w>h` moves focus left, `<C-w>l` moves focus right. For a horizontal split use `:split`.

Study the key Makefile symbols:

| Symbol | Meaning |
|--------|---------|
| `$(CC)` | Expands to `gcc` |
| `$(CFLAGS)` | Expands to the full flags string |
| `$@` | The name of the current build target |
| `$^` | All dependency files for the current target |

Close the Makefile split when done: `<C-w>c` (close current window) or `:q`.

---

## Exercise 2 — ex02-flags.c (Sanitizers)

### Step 1: Open the file

```
:e phases/00-tooling/exercises/ex02-flags.c
```

Or, if you still have ex01 open, open ex02 side by side:

```
:vsplit phases/00-tooling/exercises/ex02-flags.c
```

> **Nvim tip:** `:vsplit exercises/ex02-flags.c` lets you compare both exercise files at the same time. Use `<C-w>h/l` to switch focus between them.

The file contains two **intentional** bugs:

```c
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

### Step 2: See LSP flag the out-of-bounds access immediately

Without compiling, clangd will underline `arr[5]` with a diagnostic. Move your cursor there and press:

```
K    ← read the diagnostic message
```

You will see something like: `array index 5 is past the end of the array (which contains 5 elements)`.

Use `gr` to check if clangd has any references to `arr`:

```
gr   ← show all references to symbol under cursor
```

> **Nvim tip:** `gr` (go to references) opens a list of every location where the symbol under the cursor is used. In a small file it is less dramatic, but in a large codebase it is invaluable for understanding how a variable or function is used.

### Step 3: Build without sanitizer — silent failure

```
<leader>cb
<leader>cr
```

The program may print `sum: 5` and then either print a garbage value for `arr[5]` or crash — depending on what happens to be on the stack. This is the danger of C: the bug is real but invisible.

### Step 4: Build and run with the sanitizer

```
<leader>ct
```

This runs `make -C mysh test`, which builds with `-fsanitize=address,undefined`. The AddressSanitizer will catch the out-of-bounds access at runtime and print a detailed report:

```
==12345==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x...
READ of size 4 at 0x... thread T0
    #0 0x... in main exercises/ex02-flags.c:19
```

**What this means:**
- `stack-buffer-overflow`: memory was accessed past the end of a stack-allocated array
- `READ of size 4`: tried to read 4 bytes (the size of an `int`)
- `ex02-flags.c:19`: the exact source line of the violation

> **Nvim tip:** `<leader>ct` runs the test/sanitizer target from your Makefile without ever leaving Neovim. The output appears inline so you can read the line number in the sanitizer report and immediately jump to it: type `:19` to go to line 19, or use `19G` in normal mode.

### Step 5: Navigate to the bug using diagnostics

The sanitizer confirms line 19. But you already knew: LSP told you the moment you opened the file. Jump directly:

```
19G   ← go to line 19
```

Or use diagnostic navigation to land on the squiggle:

```
]d   ← jump to next diagnostic
```

### Step 6: Fix the bug

Change `arr[5]` to `arr[4]` (the last valid index in a 5-element array). After the edit:

- The LSP diagnostic squiggle disappears immediately (no need to recompile to confirm)
- Run `<leader>ct` again — the sanitizer should exit clean

---

## Exercise 3 — Makefile Understanding

Keep `ex01-hello.c` open in one split and the Makefile in another:

```
:vsplit Makefile
```

Jump between them with `<C-w>h` / `<C-w>l`.

Use `gd` on any Makefile variable to see its definition (if your LSP supports it), or just navigate manually. Use `gg` to go to the top of the file, `G` to go to the bottom, `{` and `}` to jump between blank-line-separated blocks.

> **Nvim tip:** `gg` jumps to the first line of the file; `G` jumps to the last line. `{` and `}` jump backward and forward by paragraph (blank-line-separated blocks). These are essential navigation keys for scanning large files quickly.

Key concepts to confirm you understand before moving on:

| Symbol | Meaning |
|--------|---------|
| `$(CC)` | Variable — expands to `gcc` |
| `$(CFLAGS)` | Variable — expands to the flags string |
| `$@` | Automatic variable — the build target name |
| `$^` | Automatic variable — all prerequisite files |

Run the build one more time to watch the expanded command in the output:

```
<leader>cb
```

---

## Exercise 4 — DAP Debugger (gdb via Neovim)

This exercise debugs `ex01-hello.c` using the DAP integration (nvim-dap + gdb) — entirely from inside Neovim, without ever typing a `gdb` command in a terminal.

### Step 1: Open the file and build with debug symbols

The Makefile already uses `-g`, so a normal build is enough:

```
<leader>cb
```

Open the exercise file if it is not already open:

```
:e phases/00-tooling/exercises/ex01-hello.c
```

### Step 2: Set a breakpoint

Move your cursor to the line with `printf("Hello, C!\n");` — use `gg` to go to the top, then `}` to jump down to the function body, or just type the line number:

```
4G   ← go to line 4
```

Set a breakpoint on this line:

```
<leader>db   ← toggle breakpoint
```

A red `●` (or similar marker) appears in the sign column on the left of that line.

> **Nvim tip:** `<leader>db` toggles a breakpoint on the current line. Call it again on the same line to remove it. You can set multiple breakpoints across multiple files before starting the debug session.

### Step 3: Start the debug session

```
<F5>   ← start / continue debug session
```

The DAP UI opens: you will see a variables panel, a call stack panel, and the source file with an arrow indicating the current execution line (paused at the breakpoint).

> **Nvim tip:** `<F5>` starts a new debug session if none is running, or resumes execution (continue) if paused at a breakpoint. The gdb adapter is configured to attach to the binary produced by the Makefile — no manual `gdb ./ex01` command needed.

### Step 4: Step through the code

With execution paused at the `printf` line:

```
<F10>   ← step over (execute current line, move to next)
<F11>   ← step into (enter the function being called)
<F12>   ← step out (run until the current function returns)
```

Use `<F10>` to step over each `printf`. Watch the variables panel update as local variables come into scope.

> **Nvim tip:** Use `<F11>` (step into) when you want to follow execution inside a function you wrote. Use `<F10>` (step over) to execute it as a black box and move to the next line. Use `<F12>` (step out) to finish the current function and return to the caller.

### Step 5: Inspect variables with hover

While paused, move the cursor over any variable name and press:

```
K   ← hover — shows current value of variable under cursor
```

This is the same `K` used for LSP documentation, but during a debug session it shows the live runtime value.

### Step 6: Terminate the session

Press `<F5>` again to continue to program exit, or use the DAP command to stop:

```
:lua require('dap').terminate()
```

> **Nvim tip:** The gdb commands you would type in a terminal (`break main`, `run`, `next`, `step`, `print x`, `backtrace`, `info locals`) are all mapped to DAP actions in Neovim. You get them through keymaps and the UI — no raw gdb REPL needed.

For reference, the gdb mental model behind the DAP keymaps:

| gdb command | DAP keymap | What it does |
|-------------|-----------|-------------|
| `break <line>` | `<leader>db` | Set breakpoint |
| `run` | `<F5>` | Start program |
| `continue` | `<F5>` | Resume after pause |
| `next` | `<F10>` | Step over |
| `step` | `<F11>` | Step into |
| `finish` | `<F12>` | Step out |
| `print x` | `K` on variable | Inspect value |

---

## Summary of Neovim Keymaps Used in This Phase

| Keymap | Action |
|--------|--------|
| `<leader>p0` | Open phases/00-tooling/README.md |
| `<leader>cb` | Build (`make -C mysh`) |
| `<leader>cr` | Build + run (`make -C mysh run`) |
| `<leader>ct` | Build + test/sanitize (`make -C mysh test`) |
| `<leader>cv` | Run Valgrind (`make -C mysh valgrind`) |
| `<leader>db` | Toggle breakpoint |
| `<F5>` | Start / continue debug |
| `<F10>` | Step over |
| `<F11>` | Step into |
| `<F12>` | Step out |
| `gd` | Go to definition |
| `gr` | Show references |
| `K` | Hover docs / inspect value |
| `<leader>rn` | Rename symbol |
| `<leader>ca` | Code action / quick fix |
| `]d` / `[d` | Next / previous LSP diagnostic |
| `<C-w>h/l` | Move between splits |
| `:vsplit <file>` | Open file in vertical split |
| `:terminal` | Open terminal inside Neovim |
| `gg` / `G` | Jump to top / bottom of file |
| `{` / `}` | Jump by paragraph/block |
| `%` | Jump to matching brace |

---

## Completion Checklist

Before moving to Phase 01, confirm:

- [ ] Opened every exercise file from inside Neovim (`:e` or `:vsplit`)
- [ ] Used `<leader>cb` and `<leader>cr` — never typed `gcc` or `./ex01` in an external terminal
- [ ] Saw LSP diagnostic squiggles appear **before** compiling (uninitialized variable, out-of-bounds)
- [ ] Used `K` and `]d` to read inline diagnostics
- [ ] Compiled `ex01-hello.c` and triggered the `-Werror` failure for uninitialized `x`
- [ ] Ran `ex02-flags.c` with the sanitizer target (`<leader>ct`) and read the ASan report
- [ ] Fixed the `arr[5]` out-of-bounds and confirmed the squiggle disappeared
- [ ] Understand what `$@` and `$^` mean in the Makefile
- [ ] Set a breakpoint with `<leader>db`, started a DAP session with `<F5>`, stepped with `<F10>`

**When all items are checked, go to `phases/01-types/README.md`.**
