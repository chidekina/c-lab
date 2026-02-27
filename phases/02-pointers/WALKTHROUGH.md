# Phase 02: Pointers & Strings — Step-by-Step Walkthrough

**Audience:** Web developer (JavaScript/TypeScript background) learning C for the first time.

> **Nvim tip:** Open this phase in Neovim and press `<leader>p2` to jump straight to this README. Keep it in a split while you work: `:vsplit phases/02-pointers/WALKTHROUGH.md`

---

## What You Will Learn

By the end of this phase you will:

- Understand what a pointer *actually is* at the machine level (not magic, just an address).
- Read and write the pointer syntax: `int *p`, `&x`, `*p`.
- Walk an array using pointer arithmetic instead of index subscripts.
- Implement `strlen` and `strcpy` from scratch — the same way the C standard library does it.
- Know when and why `const char *` matters.
- Rewrite a real function in `mysh` (`mysh_tokenize`) using pointer walking instead of `strtok`.
- Use DAP + gdb to *see* pointer values live inside the debugger.

---

## The JavaScript Mental Model: References vs C Pointers

In JavaScript you already work with references all the time:

```js
const obj = { x: 10 };
function double(o) { o.x *= 2; }   // 'o' is a reference — modifies the original
double(obj);
console.log(obj.x);  // 20
```

The engine does that automatically. You never see the address; you just get a reference.

In C **nothing is automatic**. If you want a function to modify a variable that lives in the caller's stack frame, you must hand it the *address* of that variable explicitly. The called function then uses that address to reach back into the caller's memory.

```
JavaScript:   obj  ─────────────────────────►  { x: 10 }
                         (hidden reference)

C:            int x = 10;
              int *p = &x;    // p holds the raw address, e.g. 0x7ffd2a30
              *p = 20;        // follow the address, write 20 there
```

A pointer is nothing more than an integer that happens to be interpreted as a memory address.

> **Nvim tip:** When you hover `K` over any pointer variable in a `.c` file, clangd shows the full type chain — e.g. hovering over `p` declared as `int *p` shows `int *` and the pointed-to type `int`. This is especially useful with multi-level pointers (`char **argv`) where the chain gets deep.

---

## ASCII Memory Diagram

After `int x = 42; int *p = &x;`:

```
Stack (grows downward)
╔══════════════╦══════════════════╗
║  Address     ║  Contents        ║
╠══════════════╬══════════════════╣
║  0x7ffd2a30  ║  42              ║  ← x lives here
╠══════════════╬══════════════════╣
║  0x7ffd2a28  ║  0x7ffd2a30      ║  ← p  (stores the address of x)
╚══════════════╩══════════════════╝

*p  →  reads the value at 0x7ffd2a30  →  42
 p  →  reads p itself                 →  0x7ffd2a30  (the address)
&x  →  address-of x                   →  0x7ffd2a30
&p  →  address-of p                   →  0x7ffd2a28
```

`*p = 99` writes 99 into the box at `0x7ffd2a30`, which is also where `x` lives. After that line, `x == 99`.

---

## Exercise 01: Basic Pointer

### Goal

Declare an `int`, take its address, store it in a pointer, dereference it, and print the address.

### Directory

```
phases/02-pointers/exercises/
```

### Step 1 — Create the file

```bash
mkdir -p /home/hidekina/projetos/c-lab/phases/02-pointers/exercises
cat > /home/hidekina/projetos/c-lab/phases/02-pointers/exercises/ex01.c << 'EOF'
#include <stdio.h>

int main(void) {
    int x = 42;
    int *p = &x;      /* p holds the address of x */

    printf("x        = %d\n",  x);
    printf("*p       = %d\n",  *p);    /* dereference: value at the address */
    printf("address  = %p\n",  (void *)p);  /* print the address itself */

    *p = 100;          /* modify x through the pointer */
    printf("x after *p=100: %d\n", x);

    return 0;
}
EOF
```

### Step 2 — Open in Neovim and compile

Open the file:

```bash
nvim /home/hidekina/projetos/c-lab/phases/02-pointers/exercises/ex01.c
```

The file is auto-formatted by clang-format on every save (`BufWritePre` hook). No need to run a formatter manually.

> **Nvim tip:** Place your cursor on `p` and press `K`. clangd's hover popup shows `int *` — confirming `p` is a pointer to `int`. Move the cursor to `x` and press `K` — it shows `int`. This type-at-a-glance feedback is one of the biggest advantages of editing C in Neovim over a plain terminal.

Build and run without leaving Neovim — press `<leader>cb` to build, then `<leader>cr` to run. Both commands are wired to your project's Makefile/compile command.

To compile manually from the terminal:

```bash
gcc -Wall -Wextra -o ex01 ex01.c && ./ex01
```

### Step 3 — Debug with DAP: make pointers visual

This is the most important step of the entire phase. The debugger turns abstract pointer notation into concrete addresses you can *see*.

Inside Neovim, with `ex01.c` open:

1. Move to the line `*p = 100;` and press `<leader>db` — a breakpoint marker appears in the sign column.
2. Press `<F5>` to start the debug session (DAP launches gdb under the hood).
3. Execution stops at the breakpoint. Now open the REPL:

```
:lua require('dap').repl.open()
```

In the REPL, type each expression and press Enter:

```
p x          -- prints: x = 42
p p          -- prints: p = 0x7ffd2a30  (the address)
p *p         -- prints: $1 = 42         (dereference)
p &x         -- prints: $2 = (int *) 0x7ffd2a30
p &p         -- prints: $3 = (int **) 0x7ffd2a28
```

Notice that `p` (the pointer variable) and `&x` print the *same* address. They are two names for the same location. This is not a concept anymore — it is a number on screen.

4. Press `<F10>` to step over the `*p = 100` line. Back in the REPL:

```
p x          -- prints: x = 100  (changed through the pointer)
p *p         -- prints: $4 = 100
```

`x` changed even though you only wrote `*p = 100`. That is what indirection means.

5. Press `<F5>` again to continue to the end, then `:lua require('dap').repl.close()` when done.

> **Nvim tip:** If you declare `int *p = NULL;` and try to dereference it, clangd shows an inline warning *before you even compile*: `Dereference of null pointer`. This is clangd's static analysis catching the bug at the source. NULL pointer dereferences are one of the most common causes of segfaults in C — having the editor flag it in real time saves you a debugging session.

### Expected output

```
x        = 42
*p       = 42
address  = 0x7ffd...   (some hex address — will differ each run)
x after *p=100: 100
```

### Why it works

- `&x` is the "address-of" operator. It returns the memory location where `x` is stored.
- `int *p` declares a variable that *holds* such an address.
- `*p` on the right of `=` *reads* whatever is at that address.
- `*p` on the left of `=` *writes* to that address — the same box as `x`.

Cast to `(void *)` before `%p` is required by the C standard; `printf`'s `%p` expects a `void *`.

---

## Exercise 02: Pointer Arithmetic

### Goal

Traverse an array using only a pointer — no `arr[i]` subscript notation.

### Step 1 — Create the file

```bash
cat > /home/hidekina/projetos/c-lab/phases/02-pointers/exercises/ex02.c << 'EOF'
#include <stdio.h>

int main(void) {
    int arr[] = {10, 20, 30, 40, 50};
    int *p = arr;          /* arr decays to &arr[0] */
    int len = 5;

    printf("Forward with pointer arithmetic:\n");
    for (int i = 0; i < len; i++) {
        printf("  *(p+%d) = %d   (address %p)\n", i, *(p + i), (void *)(p + i));
    }

    printf("\nWalking with p++:\n");
    int *end = arr + len;   /* one past the last element */
    for (p = arr; p < end; p++) {
        printf("  *p = %d\n", *p);
    }

    return 0;
}
EOF
```

### Step 2 — Open and read with Neovim

```bash
nvim /home/hidekina/projetos/c-lab/phases/02-pointers/exercises/ex02.c
```

> **Nvim tip:** Hover `K` over `arr` — clangd shows `int[5]`. Hover over `p` — it shows `int *`. This makes the "array decays to pointer" rule concrete: `arr` has type `int[5]`, but when you assign it to `p`, it decays to `int *` pointing to the first element. The types are different even though the address value is the same.

Build and run with `<leader>cb` then `<leader>cr`.

### Step 3 — Debug: inspect the 4-byte stride

Set a breakpoint on the `for` loop body (`printf` line inside the first loop) with `<leader>db`, then `<F5>`.

In the DAP REPL (`:lua require('dap').repl.open()`):

```
p p              -- address of arr[0], e.g. 0x7ffd1000
p p+1            -- 0x7ffd1004  (+4 bytes = sizeof(int))
p p+2            -- 0x7ffd1008  (+8 bytes)
p *(p+1)         -- 20
p *(p+2)         -- 30
```

You can see that `p+1` does not add 1 to the address — it adds `sizeof(int)` (4 bytes). The compiler knows the element size from the pointer type. Change `int arr[]` to `char arr[]` in your head and the stride would be 1.

Press `<F10>` repeatedly and watch `i` increment in the locals panel. Each step over the `printf` advances `i` by 1 and `p+i` moves 4 bytes forward.

### Expected output

```
Forward with pointer arithmetic:
  *(p+0) = 10   (address 0x...)
  *(p+1) = 20   (address 0x...+4)
  *(p+2) = 30   (address 0x...+8)
  *(p+3) = 40   (address 0x...+12)
  *(p+4) = 50   (address 0x...+16)

Walking with p++:
  *p = 10
  *p = 20
  *p = 30
  *p = 40
  *p = 50
```

### Why it works

When you add 1 to an `int *`, C does not add 1 *byte* — it adds `sizeof(int)` bytes (typically 4). The compiler knows the element size from the type of the pointer.

```
arr in memory (int = 4 bytes):
  [0x100] 10  [0x104] 20  [0x108] 30  [0x10C] 40  [0x110] 50
    ^                                                        ^
    p = arr                                           end = arr+5
```

`arr[i]` is literally defined by C as `*(arr + i)`. They are identical. The subscript notation is syntactic sugar.

---

## Exercise 03: Implement `my_strlen`

### Goal

Count the characters in a string without calling `strlen` from `<string.h>`. This forces you to understand the null-terminator convention.

### Step 1 — Create the file

```bash
cat > /home/hidekina/projetos/c-lab/phases/02-pointers/exercises/ex03.c << 'EOF'
#include <stdio.h>

/*
 * Walk forward from s until we hit the null terminator '\0'.
 * The distance walked is the length.
 */
size_t my_strlen(const char *s) {
    const char *p = s;
    while (*p != '\0') {
        p++;
    }
    return (size_t)(p - s);   /* pointer subtraction = element count */
}

int main(void) {
    const char *words[] = { "hello", "", "pointer", "C is fun", NULL };

    for (int i = 0; words[i] != NULL; i++) {
        printf("my_strlen(\"%s\") = %zu\n", words[i], my_strlen(words[i]));
    }
    return 0;
}
EOF
```

### Step 2 — Open in Neovim

```bash
nvim /home/hidekina/projetos/c-lab/phases/02-pointers/exercises/ex03.c
```

> **Nvim tip:** Hover `K` over the parameter `s` inside `my_strlen` — clangd shows `const char *`. This tells you: "I can move the pointer (`s++` would work on a local copy), but I cannot write through it (`*s = 'x'` would be a compile error)." The `const` communicates the read-only contract to both the compiler and the reader.

Build with `<leader>cb` then run with `<leader>cr`.

### Step 3 — Debug: watch pointer subtraction

Set a breakpoint on the `return` line inside `my_strlen`. Call the function with `"hello"`. In the DAP REPL (`:lua require('dap').repl.open()`):

```
p s          -- 0x...abc0  (start of "hello")
p p          -- 0x...abc5  (one past 'o', pointing at '\0')
p p - s      -- 5          (distance = length)
```

The subtraction of two pointers of the same type yields the *number of elements* between them — not the number of bytes (which would only differ for types wider than `char`, but it is the right mental model).

### Expected output

```
my_strlen("hello") = 5
my_strlen("") = 0
my_strlen("pointer") = 7
my_strlen("C is fun") = 8
```

### Why it works

A C string is a sequence of `char` values ending with a `\0` byte (ASCII 0). `"hello"` in memory is:

```
  'h' 'e' 'l' 'l' 'o' '\0'
   ^                    ^
   s                    p  (when the loop stops)
```

Subtracting two pointers of the same type gives the number of *elements* between them — exactly what we want as the length.

The parameter is `const char *s` because `my_strlen` only reads, never writes. This is correct C style and lets you call the function with string literals (which are read-only in C).

---

## Exercise 04: Implement `my_strcpy`

### Goal

Copy a string character by character into a destination buffer.

### Step 1 — Create the file

```bash
cat > /home/hidekina/projetos/c-lab/phases/02-pointers/exercises/ex04.c << 'EOF'
#include <stdio.h>

/*
 * Copy src into dst (including the null terminator).
 * dst must be large enough to hold all of src.
 * Returns dst (same convention as the standard strcpy).
 */
char *my_strcpy(char *dst, const char *src) {
    char *start = dst;
    while (*src != '\0') {
        *dst = *src;
        dst++;
        src++;
    }
    *dst = '\0';   /* null-terminate the copy */
    return start;
}

int main(void) {
    char buf[64];

    my_strcpy(buf, "hello, world");
    printf("copied: %s\n", buf);

    my_strcpy(buf, "");
    printf("empty copy length: %zu\n", buf[0] == '\0' ? 0UL : 1UL);

    return 0;
}
EOF
```

### Step 2 — Open in Neovim

```bash
nvim /home/hidekina/projetos/c-lab/phases/02-pointers/exercises/ex04.c
```

> **Nvim tip:** Place the cursor on `my_strcpy` in `main` and press `gd` — Neovim jumps to the function definition at the top of the file. Press `<C-o>` to jump back to the call site. This `gd` / `<C-o>` rhythm is how you navigate between callers and definitions throughout any C codebase — not just toy exercises, but also in `mysh` across multiple files.

Build with `<leader>cb` then run with `<leader>cr`.

### Step 3 — Debug: dual pointer walk

Set a breakpoint on `*dst = *src;` inside `my_strcpy`. Call with `"hi"`. In the DAP REPL (`:lua require('dap').repl.open()`):

```
p src        -- 0x...  pointing at 'h'
p *src       -- 104    (ASCII code for 'h')
p dst        -- 0x...  pointing at buf[0]
p *dst       -- whatever garbage was in buf before
```

Press `<F10>` once (step over the assignment). Back in the REPL:

```
p *dst       -- 104  (now 'h' is written into buf)
p dst        -- same address (dst++ hasn't run yet)
```

Press `<F10>` twice more (past `dst++` and `src++`). The REPL now shows both pointers advanced by 1 byte.

This makes explicit what the compact idiom `(*dst++ = *src++)` does in a single expression: copy, then advance both, then test the copied value.

### Expected output

```
copied: hello, world
empty copy length: 0
```

### Why it works

Both `dst` and `src` are local pointer variables — advancing them with `++` does not change the caller's pointers, only the local copy. We save `start` so we can return the original `dst` address.

The loop copies bytes one at a time until it hits `'\0'`, then copies the terminator. If you forget the final `*dst = '\0'`, the destination string will not be properly terminated and `printf` will keep reading garbage bytes beyond the end.

Compact idiomatic version (you will see this in real C code):

```c
while ((*dst++ = *src++))
    ;
```

That single expression: reads `*src`, writes into `*dst`, increments both, then tests the written value — the loop body is empty. It stops when the written byte is `'\0'` (falsy). It is valid but harder to read for a beginner; understand it, do not feel obligated to write it.

---

## Exercise 05: `const char *` vs `char *`

### Goal

Understand what `const` protects, where the compiler enforces it, and why it matters for string literals.

### Step 1 — Create the file

```bash
cat > /home/hidekina/projetos/c-lab/phases/02-pointers/exercises/ex05.c << 'EOF'
#include <stdio.h>

/* Takes a read-only view: cannot modify what p points to */
void print_upper_count(const char *p) {
    int count = 0;
    while (*p) {
        if (*p >= 'A' && *p <= 'Z') count++;
        /* *p = 'x';  <-- COMPILE ERROR: assignment of read-only location */
        p++;
    }
    printf("uppercase letters: %d\n", count);
}

int main(void) {
    /* Case 1: string literal — always read-only in C */
    const char *literal = "Hello World";
    print_upper_count(literal);
    /* literal[0] = 'h';   <-- COMPILE ERROR (and undefined behavior at runtime) */

    /* Case 2: char array — writable copy on the stack */
    char mutable[] = "Hello World";
    mutable[0] = 'h';   /* OK: mutable is a copy, not the literal */
    printf("modified: %s\n", mutable);

    /* Case 3: const pointer vs pointer to const */
    int val = 10;
    const int *pc = &val;   /* pointer to const int: can't write *pc */
    int *const cp = &val;   /* const pointer to int: can't change cp itself */
    (void)pc; (void)cp;     /* suppress unused-variable warnings */

    return 0;
}
EOF
```

### Step 2 — Open in Neovim and let clangd teach you

```bash
nvim /home/hidekina/projetos/c-lab/phases/02-pointers/exercises/ex05.c
```

> **Nvim tip:** Uncomment the line `/* *p = 'x'; */` inside `print_upper_count`. Before you even save, clangd underlines it with a red squiggle and shows the error inline: `Cannot assign to variable 'p' with const-qualified type`. This is the clangd static analysis working in real time — no compile step needed to see the error. The `const` violation is caught at edit time, not run time.

After seeing the inline error, re-comment the line and save. The squiggle disappears immediately.

> **Nvim tip:** In the `main` function, hover `K` over `literal` (declared as `const char *`) and over `mutable` (declared as `char[]`). The hover popups show different types. clangd is showing you that a string literal decays to `const char *`, while a `char[]` declaration allocates a writable copy on the stack. Same initial bytes, fundamentally different memory semantics.

Build with `<leader>cb` then run with `<leader>cr`.

### Step 3 — Intentionally trigger the compiler error (learning exercise)

Uncomment the line `/* literal[0] = 'h'; */` and try to build. You should get:

```
ex05.c:26:15: error: assignment of read-only location '*(const char *)literal'
```

Read the error carefully: "read-only location". The compiler is telling you exactly which memory region is protected. Re-comment the line before continuing.

### Expected output

```
uppercase letters: 2
modified: hello World
```

### Why it works

| Declaration           | Can change what it points to? | Can change the value it points to? |
|-----------------------|-------------------------------|------------------------------------|
| `char *p`             | Yes                           | Yes                                |
| `const char *p`       | Yes                           | No                                 |
| `char *const p`       | No                            | Yes                                |
| `const char *const p` | No                            | No                                 |

Rule of thumb: **`const` applies to what is to its left**. If nothing is to the left, it applies to what is to its right.

String literals like `"Hello World"` are stored in read-only memory. Writing to them is undefined behavior. Always declare them as `const char *`.

---

## Common Mistakes

### 1. NULL pointer dereference

```c
int *p = NULL;
*p = 42;  /* segmentation fault — there is no memory at address 0 */
```

> **Nvim tip:** If you write `int *p = NULL; *p = 42;` in a `.c` file, clangd underlines `*p` with a warning: `Dereference of null pointer (loaded from variable 'p')`. This is clangd's static analyzer catching the bug before you compile or run. It cannot catch all cases (e.g. a pointer that becomes NULL through a function call at runtime), but for obvious cases it is immediate feedback.

Always initialize pointers. If you do not have a valid address yet, set to `NULL` and check before use:

```c
if (p != NULL) {
    *p = 42;
}
```

### 2. Dangling pointer

```c
int *dangling(void) {
    int local = 5;
    return &local;   /* local is destroyed when the function returns */
}
/* using the returned pointer is undefined behavior */
```

A pointer to a stack variable becomes invalid the instant the function returns. Never return the address of a local variable. Return the value instead, or use heap allocation (`malloc`).

### 3. Off-by-one with null terminator

```c
char buf[5];
/* "hello" is 5 characters + 1 null terminator = 6 bytes needed */
my_strcpy(buf, "hello");   /* writes 6 bytes into 5-byte buffer — OVERFLOW */
```

When sizing a buffer for a string, always add 1 for `'\0'`:

```c
char buf[6];  /* correct for "hello" */
```

### 4. Forgetting `const` on read-only parameters

```c
void bad(char *s) { ... }   /* accepts a writable pointer */
bad("literal");             /* warning or error: passing const char* as char* */
```

If your function does not modify the string, declare the parameter `const char *`. It widens what callers can pass in (both `const` and non-`const` pointers) and documents intent.

### 5. Pointer arithmetic past array bounds

```c
int arr[3] = {1, 2, 3};
int *p = arr + 4;   /* one beyond the last valid element + 1 more = undefined */
*p = 99;            /* undefined behavior */
```

Reading `arr + 3` (one past the end) is defined only for comparison (`p < end`). Dereferencing it or going further is undefined behavior.

---

## mysh Milestone: Rewrite `mysh_tokenize` with Pointer Walking

### Why

The current `tokenize.c` uses `strtok`, which:

- Modifies the input buffer (replaces delimiters with `'\0'`) and depends on internal static state.
- Cannot be called re-entrantly (two nested uses would corrupt each other).
- Hides the pointer mechanics you just learned.

You will replace it with explicit pointer walking.

### Navigate to the code with Neovim

The milestone workflow happens entirely inside Neovim — no switching to a file manager or running `find`.

From anywhere in the project, open `main.c`:

```bash
nvim /home/hidekina/projetos/c-lab/mysh/src/main.c
```

> **Nvim tip:** Find the call to `mysh_tokenize` in `main.c`. Place your cursor on `mysh_tokenize` and press `gd`. clangd resolves the symbol and jumps directly to the implementation in `tokenize.c` — even though it is in a different file. No `:e`, no searching, no directory navigation. Press `<C-o>` to jump back to `main.c`. This is the primary navigation pattern when working across a multi-file C codebase.

Once you are in `tokenize.c`, open the header side by side:

```
:vsplit mysh/include/mysh.h
```

Now your screen is split: `tokenize.c` on the left, `mysh.h` on the right. You can see the `MAX_ARGS` constant and the `mysh_tokenize` declaration while editing the implementation. Use `<C-w>h` and `<C-w>l` to move between splits.

> **Nvim tip:** With your cursor on `argv` anywhere in `main.c` or `tokenize.c`, press `gr` to show all references. clangd lists every file and line where `argv` is used across the entire project — including how it is populated in `tokenize.c`, passed to `execvp` in the execution logic, and declared in the header. This is how you trace how a pointer array flows through the program before you rewrite the function that fills it.

### Current code (`mysh/src/tokenize.c`)

```c
#include <string.h>
#include "mysh.h"

int mysh_tokenize(char *buf, char **argv) {
    int argc = 0;
    char *token = strtok(buf, " \t");
    while (token != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = token;
        token = strtok(NULL, " \t");
    }
    argv[argc] = NULL;
    return argc;
}
```

### The new implementation

With `tokenize.c` open (navigate there via `gd` from `main.c`), replace its contents:

```c
#include "mysh.h"

/*
 * Tokenize buf in-place using pointer arithmetic.
 *
 * Algorithm:
 *   - Walk p through buf one character at a time.
 *   - Skip leading whitespace.
 *   - When a non-whitespace character is found, record its address as the
 *     start of a token and write it into argv.
 *   - Walk forward until whitespace or '\0' is found.
 *   - Write '\0' at the delimiter position to terminate the token in-place.
 *   - Repeat until end of string or MAX_ARGS-1 tokens collected.
 */
int mysh_tokenize(char *buf, char **argv) {
    int argc = 0;
    char *p = buf;

    while (*p != '\0' && argc < MAX_ARGS - 1) {
        /* Skip whitespace */
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p == '\0') {
            break;   /* only whitespace remained */
        }

        /* p now points to the first character of a token */
        argv[argc++] = p;

        /* Advance until whitespace or end of string */
        while (*p != ' ' && *p != '\t' && *p != '\0') {
            p++;
        }

        /* If we stopped on a delimiter, null-terminate the token */
        if (*p != '\0') {
            *p = '\0';   /* write '\0' into buf to end this token */
            p++;         /* move past the terminator we just wrote */
        }
    }

    argv[argc] = NULL;   /* argv must be NULL-terminated (execvp convention) */
    return argc;
}
```

Note that `#include <string.h>` is no longer needed — remove it.

After editing, save the file. clang-format runs automatically on save (the `BufWritePre` hook) and normalizes indentation.

> **Nvim tip:** With the new implementation written, use `<leader>rn` on `argc` to rename it if you ever want to try a different name — clangd renames every occurrence in the file simultaneously. This is safe refactoring: not a text search-and-replace, but a semantic rename that only touches the same variable, not unrelated strings that happen to match.

### Rebuild without leaving Neovim

Press `<leader>cb` to rebuild `mysh`. The build output appears in a floating window or the quickfix list. If there are compile errors, they appear as diagnostics in the sign column — press `<leader>ca` on a flagged line to see code actions clangd suggests.

If the build succeeds, press `<leader>cr` to run `mysh` and test it interactively.

### Memory diagram during tokenization

Input `buf`:  `"ls  -la /tmp\n"`

```
Before:
  buf → ['l']['s'][' '][' ']['-']['l']['a'][' ']['/']['t']['m']['p']['\n']['\0']

After tokenizing:
  buf → ['l']['s']['\0']['-']['l']['a']['\0']['/']['t']['m']['p']['\0']  ...
                   ^                        ^                   ^
         argv[0]  "ls"            argv[1]  "-la"    argv[2]  "/tmp"
         argv[3]  NULL
```

`buf` is mutated in place. Each token is not a new allocation — it is a pointer into `buf` itself. The `'\0'` bytes written by the tokenizer slice the original buffer into null-terminated segments.

### Debug the tokenizer with DAP

Set a breakpoint on `argv[argc++] = p;` inside `mysh_tokenize`. Then press `<F5>` to start a debug session with an input of `"ls -la"`.

In the DAP REPL (`:lua require('dap').repl.open()`):

```
p p          -- address inside buf, pointing at 'l'
p buf        -- same address: p starts at the beginning
p *p         -- 108  (ASCII 'l')
```

Press `<F10>` past the assignment. Now:

```
p argv[0]    -- same address as buf: argv[0] IS a pointer into buf
p argc       -- 1
```

Step through the whitespace-skip inner loop. Watch `p` advance byte by byte until it hits `-`. Then after the second token is recorded and null-terminated:

```
p argv[1]    -- address of '-' inside buf
p buf        -- look at buf now: "ls\0-la" — the space is gone, replaced with '\0'
```

This confirms that `argv[0]` and `argv[1]` are not copies — they are slices of the same original buffer.

### What to verify after the rewrite

```bash
./bin/mysh
> echo hello world     # should print: hello world
> ls -la               # should list files
> exit
```

- `echo hello world` still prints `hello world`.
- Multiple spaces between arguments are handled (they are skipped by the inner `while`).
- An empty input line returns `argc == 0` and does not crash.
- A line with only spaces/tabs returns `argc == 0`.

> **Nvim tip:** After verifying the shell works, run Valgrind through Neovim with `<leader>cv`. Valgrind checks for memory errors — invalid reads, writes past buffer bounds, use-after-free. If the tokenizer is correct, Valgrind should exit cleanly with `0 errors from 0 contexts`. If it reports an error, the output in the quickfix window tells you the exact line number and what went wrong.

---

## Completion Checklist

Mark each item when done:

- [ ] ex01 compiled and ran — printed an address in hex.
- [ ] ex01 used DAP REPL to inspect `p`, `*p`, `&x` and confirmed they are addresses.
- [ ] ex01 stepped over `*p = 100` in the debugger and saw `x` change to 100.
- [ ] ex02 printed all 5 array values using `*(p+i)` and `p++` styles.
- [ ] ex02 used DAP REPL to confirm the 4-byte stride between `int` addresses.
- [ ] ex03 `my_strlen` returns correct length for empty string and non-empty strings.
- [ ] ex03 used DAP REPL to confirm pointer subtraction gives the element count.
- [ ] ex04 `my_strcpy` copies correctly and null-terminates.
- [ ] ex04 stepped through the copy loop and watched `*dst` receive each character.
- [ ] ex05 saw clangd underline the `const` violation inline before compiling.
- [ ] ex05 can explain the difference between `const char *p` and `char *const p`.
- [ ] Navigated to `mysh_tokenize` implementation using `gd` from `main.c`.
- [ ] Used `gr` on `argv` to trace how the pointer array flows through `mysh`.
- [ ] Rewrote `mysh_tokenize` — `strtok` removed, pointer walk in place.
- [ ] Rebuilt `mysh` with `<leader>cb` without leaving Neovim.
- [ ] `echo hello world` works in the new `mysh`.
- [ ] Ran `<leader>cv` (Valgrind) and got 0 errors.
- [ ] Can draw the ASCII memory diagram for `int x = 5; int *p = &x;` from memory.

Once all items are checked, move to **Phase 03: Structs & Memory**.
