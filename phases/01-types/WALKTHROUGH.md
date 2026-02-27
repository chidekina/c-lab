# Phase 01 Walkthrough: Types & Undefined Behavior

## What you will learn

Coming from JavaScript or TypeScript, you are used to a world where numbers just work. `42 + 1`
is `43`. You never ask "how many bytes is that number?" because V8 handles it for you — all JS
numbers are 64-bit IEEE 754 floats under the hood.

C is different. When you declare a variable, you are directly reserving memory. The type you pick
determines exactly how many bytes are allocated and what values are valid. Pick the wrong type and
the computer will silently do the wrong thing — no exception, no runtime error, no TypeScript
squiggle.

By the end of this phase you will:

- Know the size in bytes of every basic C type on your machine.
- Understand why integer overflow is not an error in C (and when it is UB).
- Trigger Undefined Behavior and catch it with sanitizers.
- Replace bare `int` with portable fixed-width types from `<stdint.h>`.
- Have a working `mysh_readline` + `echo` builtin wired into your shell project.

---

## Mental model: C types vs JavaScript

| JavaScript | C equivalent | Size | Range |
|---|---|---|---|
| `number` (integer use) | `int` | usually 4 bytes | -2,147,483,648 to 2,147,483,647 |
| `number` (byte) | `uint8_t` | exactly 1 byte | 0 to 255 |
| `number` (float) | `double` | 8 bytes | ±1.8 × 10^308 |
| `bigint` | `int64_t` | exactly 8 bytes | ±9.2 × 10^18 |
| (no equivalent) | `char` | 1 byte | holds one ASCII character |

The key insight: in JS, overflow wraps around in bitwise ops and saturates in arithmetic. In C,
signed integer overflow is **Undefined Behavior** — the compiler is free to assume it never happens
and may produce code that deletes your data, loops forever, or crashes. `uint8_t` overflow, on the
other hand, is defined: it wraps modulo 256.

---

## Your environment: Neovim + clangd + DAP

This project ships with a pre-configured Neovim setup in `.nvim/`. When you open any `.c` or `.h`
file, clangd attaches automatically and gives you:

- Inline diagnostics (warnings and errors as you type, before compiling).
- Type information on hover.
- Go-to-definition across headers.
- Rename refactoring across all files.

The keymaps you will use throughout every exercise are:

| Keymap | Action |
|---|---|
| `gd` | Go to definition |
| `gr` | Show all references |
| `K` | Hover: type info, size, comments from the header |
| `<leader>rn` | Rename symbol everywhere |
| `<leader>ca` | Code action / quick fix |
| `<leader>cb` | Build (`make -C mysh`) |
| `<leader>cr` | Build and run |
| `<leader>ct` | Run tests |
| `<leader>cv` | Run Valgrind |
| `<leader>db` | Toggle breakpoint |
| `<F5>` | Start / continue debug session |
| `<F10>` | Step over |
| `<F11>` | Step into |
| `<F12>` | Step out |
| `]d` / `[d` | Jump to next / previous LSP diagnostic |

**Auto-format on save:** every time you press `:w` on a `.c` or `.h` file, clang-format runs
automatically via a `BufWritePre` hook. You will never see misaligned braces in this project.

> **Nvim tip:** `<leader>p1` opens `phases/01-types/README.md` directly. Use it to jump back to the
> phase overview at any time without navigating the file tree.

---

## Exercise 01 — sizeof all basic types

**Goal:** print the size in bytes of every basic C type so you have a concrete mental map. Along
the way, use `K` to inspect what `uint8_t` actually is under the hood.

### Step 1: open the file in Neovim

From inside Neovim (you can open it from the project root):

```
:e phases/01-types/exercises/ex01_sizeof.c
```

If the file does not exist yet, Neovim creates a new buffer for it. Type `i` to enter insert mode
and write the code below. Press `<Esc>` when done, then `:w` to save (auto-format runs on save).

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    printf("char       : %zu bytes\n", sizeof(char));
    printf("short      : %zu bytes\n", sizeof(short));
    printf("int        : %zu bytes\n", sizeof(int));
    printf("long       : %zu bytes\n", sizeof(long));
    printf("long long  : %zu bytes\n", sizeof(long long));
    printf("float      : %zu bytes\n", sizeof(float));
    printf("double     : %zu bytes\n", sizeof(double));
    printf("pointer    : %zu bytes\n", sizeof(void *));
    printf("uint8_t    : %zu bytes\n", sizeof(uint8_t));
    printf("int32_t    : %zu bytes\n", sizeof(int32_t));
    printf("int64_t    : %zu bytes\n", sizeof(int64_t));
    return 0;
}
```

Note: `sizeof` returns a value of type `size_t`. The correct format specifier for it is `%zu`.
Using `%d` would work on most 64-bit machines but is technically UB — a habit worth avoiding from
day one.

### Step 2: let clangd reveal the typedef chain

After saving, place your cursor on `uint8_t` in the `sizeof(uint8_t)` line and press `K`.

The hover popup shows something like:

```
uint8_t
typedef unsigned char uint8_t
stdint.h
```

> **Nvim tip:** `K` invokes `vim.lsp.buf.hover()`. It asks clangd for the type information at the
> cursor and displays the result inline. You can see the full typedef chain — `uint8_t` is just
> `unsigned char` — without ever opening `stdint.h` manually.

Now press `gd` while the cursor is on `uint8_t`. Neovim jumps to the `typedef` line inside
`stdint.h` (or the compiler's internal header). Press `<C-o>` to jump back to your file.

> **Nvim tip:** `gd` (go to definition) works on any symbol clangd knows about — types, functions,
> macros. `<C-o>` moves back in the jump list. `<C-i>` moves forward.

### Step 3: build and run without leaving Neovim

Press `<leader>cb` to build. The output appears in the quickfix list. If there are errors, `]d`
jumps to the first diagnostic inline.

To run interactively, open a terminal split alongside your code:

```
:split | terminal
```

> **Nvim tip:** `:split | terminal` opens a horizontal split with a real terminal. Use `<C-w>h` and
> `<C-w>l` to move between the editor and the terminal pane. In terminal mode, press `<C-\><C-n>`
> to return to normal mode so you can use `<C-w>` movement again.

In the terminal pane, compile and run directly:

```bash
gcc -Wall -Wextra -o /tmp/ex01 phases/01-types/exercises/ex01_sizeof.c && /tmp/ex01
```

### Expected output (x86-64 Linux)

```
char       : 1 bytes
short      : 2 bytes
int        : 4 bytes
long       : 8 bytes
long long  : 8 bytes
float      : 4 bytes
double     : 8 bytes
pointer    : 8 bytes
uint8_t    : 1 bytes
int32_t    : 4 bytes
int64_t    : 8 bytes
```

### Why it works this way

`sizeof` is evaluated **at compile time** — it is not a function call. The compiler replaces
`sizeof(int)` with the literal `4` before your program runs. This means there is zero runtime cost.

The sizes above are for x86-64 Linux. On a 32-bit ARM microcontroller `long` might be 4 bytes. On
MSVC (Windows) `long` is 4 bytes even on 64-bit. This is why you should never assume the size of
`int` or `long` in portable code — and why `uint8_t`, `int32_t`, and friends exist.

Notice `pointer` is 8 bytes on 64-bit. Every pointer — regardless of what it points to — is the
same size, because a pointer is just a memory address.

---

## Exercise 02 — trigger integer overflow

**Goal:** make a signed integer exceed its maximum value and observe what happens. Notice that
clangd warns you inline *before* you compile.

### Step 1: open the file

```
:e phases/01-types/exercises/ex02_overflow.c
```

### Step 2: type the code and watch inline diagnostics appear

As you type the assignment `int overflow = max + 1;`, clangd detects a potential signed overflow
and places a diagnostic marker on that line — a yellow squiggle or a virtual text annotation,
depending on your colorscheme.

```c
#include <stdio.h>
#include <limits.h>

int main(void) {
    int max = INT_MAX;              /* 2,147,483,647 */
    int overflow = max + 1;         /* clangd warns here: signed overflow */

    printf("INT_MAX          : %d\n", max);
    printf("INT_MAX + 1      : %d\n", overflow);
    printf("Expected in JS   : 2147483648\n");
    return 0;
}
```

> **Nvim tip:** clangd runs in the background and sends diagnostics to Neovim via LSP without any
> compilation step. The diagnostic appears as you type, usually within a second of saving or even
> before saving. Press `K` on the squiggled token to see the full warning text in a hover popup.
> Use `]d` to jump to the next diagnostic and `[d` to jump to the previous one.

Save the file with `:w`. Auto-format aligns the comments. The inline diagnostic remains because
clangd re-evaluates on every save.

### Step 3: build without optimizations

In your terminal split (`<C-w>l` or `<C-w>j` to reach it):

```bash
gcc -Wall -Wextra -o /tmp/ex02 phases/01-types/exercises/ex02_overflow.c && /tmp/ex02
```

Expected output (unoptimized build):

```
INT_MAX          : 2147483647
INT_MAX + 1      : -2147483648
Expected in JS   : 2147483648
```

The value wrapped to the most negative `int`. This looks like "it works" — and it usually does
without optimizations. That is the danger.

### Step 4: compile with optimizations and observe the compiler warning

```bash
gcc -Wall -Wextra -O2 -o /tmp/ex02_opt phases/01-types/exercises/ex02_overflow.c
```

GCC emits:

```
ex02_overflow.c:6:24: warning: integer overflow in expression of type 'int' results
in '-2147483648' [-Woverflow]
```

With `-O2` enabled, GCC may optimize out an entire branch guarded by a signed overflow assumption.
A real-world example: a loop like `for (int i = 0; i >= 0; i++)` — the compiler may turn it into
an infinite loop by assuming `i` never overflows (because that would be UB) and therefore the
condition is always true.

### Step 5: use DAP to inspect the value at runtime

Set a breakpoint on the `printf` line:

1. Move your cursor to the `printf("INT_MAX + 1 ...")` line.
2. Press `<leader>db` to toggle a breakpoint (a red dot appears in the sign column).
3. Press `<F5>` to start the debug session. GDB launches and stops at your breakpoint.
4. Press `<F10>` (step over) to advance one line.
5. Hover over `overflow` in normal mode — the DAP virtual text shows its current value.

> **Nvim tip:** nvim-dap integrates GDB directly into Neovim. `<F5>` starts the session, `<F10>`
> steps over, `<F11>` steps into a function call, `<F12>` steps out. You can inspect variables by
> hovering with `K` during a session or by opening the DAP REPL (`:lua require("dap").repl.open()`).

Press `<F5>` again to continue execution to the end. The debug session terminates automatically.

---

## Exercise 03 — UB: uninitialized read

**Goal:** read an uninitialized variable, which is classic Undefined Behavior. See how clangd
catches it inline, then catch it again at runtime with sanitizers, then step through it in the
debugger to see the garbage value on the stack.

### Step 1: open the file

```
:e phases/01-types/exercises/ex03_ub.c
```

### Step 2: write the code — watch the diagnostic appear as you type

```c
#include <stdio.h>

int main(void) {
    int x;              /* declared but never assigned */
    printf("x = %d\n", x);  /* reading uninitialized memory: UB */
    return 0;
}
```

The moment you write `printf("x = %d\n", x)` without having assigned `x`, clangd places a
diagnostic on that line:

```
warning: variable 'x' is uninitialized when used here [-Wuninitialized]
```

> **Nvim tip:** notice the diagnostic appears *before you compile*. This is the core value of
> having clangd in your editor: the LSP server runs a semantic analysis pass that catches many
> errors instantly. Press `<leader>ca` on the diagnostic to see if clangd offers a quick fix
> (in this case it may suggest initializing `x = 0`).

Save with `:w`. The diagnostic stays because the UB is real.

### Step 3: compile and run without sanitizers

In the terminal split:

```bash
gcc -Wall -Wextra -o /tmp/ex03 phases/01-types/exercises/ex03_ub.c && /tmp/ex03
```

GCC warns:

```
ex03_ub.c:5:5: warning: 'x' is used uninitialized [-Wuninitialized]
```

The program may print `x = 0` or `x = 32767` or any other garbage value from the stack. It might
even print `x = 0` consistently, giving you false confidence. This is the core problem with UB: it
is not reliably visible without tooling.

### Step 4: compile with sanitizers

```bash
gcc -fsanitize=address,undefined -g -O0 \
    -o /tmp/ex03_san phases/01-types/exercises/ex03_ub.c \
    && /tmp/ex03_san
```

The `-g` flag includes debug symbols so the sanitizer can print file and line. Expected output:

```
ex03_ub.c:5:20: runtime error: load of value 32764, which is not a valid value for type 'int'
```

Or from UBSan more specifically:

```
ex03_ub.c:5:20: runtime error: load of uninitialized value of type 'int'
```

The program aborts with a non-zero exit code. The sanitizer caught what the compiler only warned
about.

### Step 5: inspect the garbage value in the debugger

1. Place a breakpoint on the `printf` line with `<leader>db`.
2. Press `<F5>` to start the debug session.
3. When execution stops at the breakpoint, move your cursor over `x`.

> **Nvim tip:** during a DAP session, hover (`K`) shows the current value of any variable the
> cursor is on. For an uninitialized variable you will see whatever bytes happened to be on the
> stack at that moment — not zero, not a crash, just random data. This makes the problem visceral
> in a way that reading about UB does not.

Press `<F10>` to step over the `printf` and observe the output in the terminal. Press `<F5>` to
finish the session.

### Step 6: understand what ASan and UBSan each catch

**UBSan** (Undefined Behavior Sanitizer) instruments operations the C standard defines as UB:
signed overflow, uninitialized reads, null pointer dereference, misaligned access, out-of-bounds
array indexing, and more. It adds checks around each such operation at runtime.

**ASan** (Address Sanitizer) focuses on memory safety: use-after-free, heap buffer overflow, stack
buffer overflow, use-after-return. It inserts "poison" zones around allocations and checks every
load/store.

Both are enabled with a single flag. The only cost is roughly 2x slowdown and increased memory —
acceptable during development and in CI, but not in production binaries.

### Step 7: verify with Valgrind (keymapped)

The project Makefile has a `valgrind` target. For the mysh exercises you can press `<leader>cv`
from inside Neovim. For the standalone exercises, use the terminal split:

```bash
gcc -g -o /tmp/ex03_val phases/01-types/exercises/ex03_ub.c
valgrind --track-origins=yes /tmp/ex03_val
```

Valgrind reports:

```
==PID== Conditional jump or move depends on uninitialised value(s)
==PID==    at 0x... (printf)
==PID==    by 0x... (main) ex03_ub.c:5
==PID== Uninitialised value was created by a stack allocation
==PID==    at 0x... (main) ex03_ub.c:4
```

Sanitizers are faster than Valgrind and are preferred for regular development cycles. Use Valgrind
when sanitizers are unavailable or when you need heap profiling.

---

## Exercise 04 — rewrite with fixed-width types

**Goal:** replace `int` with `uint8_t` and `int32_t` from `<stdint.h>` to make type sizes explicit
and portable. Use `K` to confirm the types and `gd` to navigate into the stdint.h definitions.

### Step 1: open the file

```
:e phases/01-types/exercises/ex04_stdint.c
```

### Step 2: write the code

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>   /* PRId32, PRIu8 macros for printf */

int main(void) {
    /* uint8_t: 0-255, wraps on overflow (defined behavior) */
    uint8_t byte_val = 255;
    uint8_t byte_wrap = byte_val + 1;   /* wraps to 0 — defined, not UB */

    /* int32_t: exactly 32-bit signed */
    int32_t big = 2147483647;
    /* int32_t overflow = big + 1; */   /* still UB — DO NOT DO THIS */

    printf("uint8_t max      : %" PRIu8 "\n", byte_val);
    printf("uint8_t wrap     : %" PRIu8 "\n", byte_wrap);
    printf("int32_t max      : %" PRId32 "\n", big);

    /* sizeof on fixed-width types */
    printf("sizeof uint8_t   : %zu\n", sizeof(uint8_t));
    printf("sizeof int32_t   : %zu\n", sizeof(int32_t));
    printf("sizeof int64_t   : %zu\n", sizeof(int64_t));
    printf("sizeof uint64_t  : %zu\n", sizeof(uint64_t));

    return 0;
}
```

Save with `:w`. Clang-format runs automatically and aligns the inline comments.

### Step 3: explore the types with LSP

Place your cursor on `PRIu8` and press `K`. The hover shows the macro expansion — something like
`"u"` on 64-bit Linux. Then press `gd` to jump to the definition inside `inttypes.h` and see all
the `PRI*` macros in context. Press `<C-o>` to return.

> **Nvim tip:** `gd` plus `<C-o>` is one of the fastest ways to learn the C standard library.
> Instead of searching the web, place your cursor on any standard type or macro and let clangd take
> you directly to the header that defines it.

Now place your cursor on `uint8_t` in the declaration line and press `gr` (show references). You
see every use of `uint8_t` in your current file highlighted in the quickfix list.

> **Nvim tip:** `gr` (go to references) uses LSP to find all usages of the symbol under the cursor.
> This is the same feature you know as "Find all references" in VS Code.

### Step 4: notice that clangd stays silent on the uint8_t wrap

The commented-out line `/* int32_t overflow = big + 1; */` would produce a clangd diagnostic
(signed overflow) if you uncommented it. The `uint8_t byte_wrap = byte_val + 1;` line produces no
diagnostic because unsigned overflow is defined by the C standard and is not a bug.

This is a key difference:

- `uint8_t` wraps modulo 256 — defined, predictable, no diagnostic.
- `int32_t` overflow — Undefined Behavior, clangd warns, sanitizer aborts.

### Step 5: build and run

In the terminal split:

```bash
gcc -Wall -Wextra -o /tmp/ex04 phases/01-types/exercises/ex04_stdint.c && /tmp/ex04
```

Expected output:

```
uint8_t max      : 255
uint8_t wrap     : 0
int32_t max      : 2147483647
sizeof uint8_t   : 1
sizeof int32_t   : 4
sizeof int64_t   : 8
sizeof uint64_t  : 8
```

### Step 6: run with sanitizers to confirm no UB

```bash
gcc -fsanitize=address,undefined -g -O0 \
    -o /tmp/ex04_san phases/01-types/exercises/ex04_stdint.c \
    && /tmp/ex04_san
```

No errors. The `uint8_t` wrap is defined behavior (unsigned overflow wraps modulo 256 by the C
standard). The sanitizer stays silent.

### Step 7: use DAP to confirm the wrap value

1. Place a breakpoint on the `printf("uint8_t wrap ...")` line with `<leader>db`.
2. Press `<F5>` to start the debug session.
3. Hover over `byte_wrap` with `K`. The value is `0`.
4. Press `<F11>` to step into the `printf` call and see how the format string is resolved.
5. Press `<F12>` to step out of `printf` back to `main`.

> **Nvim tip:** `<F11>` (step into) and `<F12>` (step out) let you follow execution into any
> function — including standard library calls like `printf`. This is useful for understanding how
> library functions interact with your data types. Press `<F5>` to continue to the end.

### Step 8: understand the printf macros

`PRIu8`, `PRId32`, etc. are macros from `<inttypes.h>` that expand to the correct format string for
each fixed-width type on the current platform. On a 32-bit machine `int32_t` might be a `long`, so
`%d` would be wrong. These macros guarantee correctness:

```c
printf("%" PRId32 "\n", my_int32);
/* expands to printf("%d\n", ...) on 64-bit Linux */
/* expands to printf("%ld\n", ...) where needed   */
```

This is verbose but correct. In practice, many developers skip the macros for quick programs and
rely on the fact that `int32_t` is `int` on all mainstream 64-bit targets — but for embedded or
cross-platform code, use the macros.

---

## Common mistakes

### Mistake 1: using `%d` for `size_t`

```c
/* WRONG */
printf("size: %d\n", sizeof(int));

/* CORRECT */
printf("size: %zu\n", sizeof(int));
```

`sizeof` returns `size_t`, which is unsigned and may be 64-bit. `%d` expects a signed 32-bit int.
On 64-bit Linux this usually prints the right number, but it is UB and UBSan will flag it.

> **Nvim tip:** if you type `printf("size: %d\n", sizeof(int))`, clangd shows an inline diagnostic
> immediately: *format specifies type 'int' but the argument has type 'unsigned long'*. Fix it with
> `<leader>ca` if a quick fix is offered, or edit manually and let `:w` auto-format.

### Mistake 2: assuming signed overflow wraps

```c
int x = INT_MAX;
if (x + 1 < 0) {    /* a common "check" for overflow */
    printf("overflow!\n");
}
```

With `-O2`, GCC sees that `x + 1 < 0` is UB (signed overflow), assumes it never happens, and
**removes the branch entirely**. Use `__builtin_add_overflow` or cast to `unsigned` to check safely:

```c
unsigned int ux = (unsigned int)x;
if (ux + 1 > INT_MAX) { /* safe: unsigned overflow is defined */ }
```

### Mistake 3: forgetting `-g` with sanitizers

Without debug symbols the sanitizer output points to a hex address instead of a file and line.
Always combine:

```bash
gcc -fsanitize=address,undefined -g -O0 -o program program.c
```

`-O0` disables optimizations, which keeps sanitizer output easier to read during learning.

### Mistake 4: mixing signed and unsigned comparisons

```c
int len = -1;
size_t buf_size = 100;
if (len < buf_size) {   /* WRONG: len is promoted to a huge unsigned number */
    /* this branch may never execute */
}
```

`-Wall` warns: *comparison of integer expressions of different signedness*. Clangd also flags this
inline. Never ignore that warning.

### Mistake 5: leaving variables uninitialized "for performance"

In C you can declare a variable without initializing it. In JavaScript, declared variables are
`undefined`. In C, the variable contains whatever bytes happened to be in that memory location.
Always initialize:

```c
int x = 0;          /* safe */
uint8_t buf[256];   /* stack array: garbage until you fill it */
memset(buf, 0, sizeof(buf));  /* now it is zeroed */
```

---

## mysh milestone: understand the input loop

The mysh milestone for Phase 01 is to read and understand the `mysh_readline` function and `echo`
builtin that already exist in `mysh/src/`. Your job is to analyze them through the lens of what you
just learned about types, buffers, and UB.

### Open the files in Neovim

Use two vertical splits to read both files side by side:

```
:e mysh/src/readline.c
:vsplit mysh/src/builtins.c
```

Use `<C-w>h` and `<C-w>l` to move between the splits.

> **Nvim tip:** `:vsplit <file>` opens a vertical split. `:split <file>` opens a horizontal one.
> You can have as many splits as you need. `<C-w>=` makes them all equal width.

### The readline implementation

```c
#include <stdio.h>
#include <string.h>
#include "mysh.h"

int mysh_readline(char *buf, int size) {
    if (fgets(buf, size, stdin) == NULL)
        return -1;
    /* Strip trailing newline */
    int len = (int)strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
        len--;
    }
    return len;
}
```

Place your cursor on `fgets` and press `K` to see its signature. Press `gd` to jump to the
declaration in `stdio.h`. Press `gr` on `mysh_readline` to find all the places it is called.

**What to notice through a Phase 01 lens:**

1. `buf` is a `char *` — a pointer to memory the caller owns. If the caller passes a buffer of 512
   bytes but `size` says 1024, that is a buffer overflow — classic UB. The contract is: `size` must
   accurately reflect how many bytes `buf` can hold.

2. `fgets(buf, size, stdin)` reads at most `size - 1` characters and always null-terminates. This
   is the safe alternative to `gets()`, which has no size limit and was removed from C11.

3. `strlen` returns `size_t` (unsigned). The cast `(int)strlen(buf)` is intentional — `size` is
   `int`, so keeping everything signed avoids mixed-signedness comparisons. On a 512-byte shell
   buffer this is fine; for large data it would be a concern.

4. The function returns `-1` on EOF (end of input / Ctrl+D). The caller in `main.c` uses this to
   exit the shell loop.

### The echo builtin

```c
/* echo */
if (strcmp(argv[0], "echo") == 0) {
    for (int i = 1; i < argc; i++) {
        printf("%s", argv[i]);
        if (i < argc - 1) printf(" ");
    }
    printf("\n");
    return 1;
}
```

**What to notice:**

1. `argv` is `char **` — an array of string pointers. `argc` is the count. This mirrors the
   `main(int argc, char **argv)` signature you will see everywhere.

2. The loop variable `i` is `int`. `argc` is `int`. No mixed-signedness issue here — both signed.

3. The function returns `1` to indicate "this was a builtin, handled." It returns `0` when no
   builtin matched, so the caller knows to look for an external command.

### Build and test the shell from inside Neovim

Press `<leader>cb` to build. The output appears in the quickfix list. If there are errors, `]d`
jumps to the first diagnostic.

Press `<leader>cr` to build and run the shell in the terminal. At the prompt, type:

```
echo hello world
```

Expected output:

```
hello world
```

Type `exit` to quit or press Ctrl+D.

> **Nvim tip:** `<leader>cb`, `<leader>cr`, `<leader>ct`, and `<leader>cv` all run `make` targets
> and surface output in Neovim's quickfix list. Errors become navigable entries — press `<Enter>` on
> a quickfix item to jump directly to the file and line the compiler complained about.

### Build with sanitizers

Open the terminal split (`:split | terminal`) and run:

```bash
cd /home/hidekina/projetos/c-lab/mysh
make clean
CFLAGS="-fsanitize=address,undefined -g" make
./bin/mysh
```

Run a few echo commands. If the sanitizer stays silent, the implementation is clean. This is your
verification that the readline buffer handling has no UB.

---

## Completion checklist

Mark these off before moving to Phase 02:

- [ ] `ex01_sizeof.c` compiles and prints correct sizes for all 11 types listed above.
- [ ] You used `K` on `uint8_t` to see the typedef chain in the hover popup.
- [ ] You used `gd` to jump to `uint8_t`'s definition in `stdint.h`.
- [ ] `ex02_overflow.c` shows a clangd inline diagnostic before you compile.
- [ ] You can explain why `-O2` makes signed overflow more dangerous, not less.
- [ ] You set a DAP breakpoint in ex02 and inspected the overflow value at runtime.
- [ ] `ex03_ub.c` shows a clangd diagnostic as you type the uninitialized read.
- [ ] The sanitizer catches the uninitialized read at runtime.
- [ ] You used the DAP to see the garbage stack value of `x` before `printf`.
- [ ] `ex04_stdint.c` compiles; `uint8_t` wraps to 0 at 255 + 1; no sanitizer errors.
- [ ] You know the difference between `uint8_t` wrap (defined) and `int32_t` overflow (UB).
- [ ] You used `gr` in `builtins.c` to find all callers of the echo logic.
- [ ] `mysh` builds with `<leader>cb` and the `echo` builtin prints arguments correctly.
- [ ] `mysh` builds with `-fsanitize=address,undefined` and the sanitizer stays silent.
- [ ] You have read `readline.c` and `builtins.c` and can explain each line in terms of types and
  memory.
