# Phase 04 — Memory Management: Walkthrough

> Audience: web developer (JavaScript/TypeScript background) learning C.
> Time estimate: 3–5 hours for exercises + mysh milestone.
> Open this phase in Neovim with `<leader>p4` from anywhere in the project.

---

## 1. You Are the Garbage Collector Now

In JavaScript or TypeScript you write this and move on:

```js
function makeList() {
  const items = [1, 2, 3]; // V8 allocates this
  return items;             // V8 keeps it alive as long as something holds a reference
}                           // When nothing refers to it, V8 frees it — you never asked
```

In C there is no V8. There is no runtime watching your back. When you allocate memory you get a raw pointer and a responsibility:

```c
int *items = malloc(3 * sizeof(int)); // YOU allocate
items[0] = 1; items[1] = 2; items[2] = 3;
// ... use it ...
free(items);  // YOU free — or it leaks forever
```

Forget `free` and the memory sits there, claimed but useless, until the process dies. On a desktop that is annoying. On an ESP32 with 520 KB of RAM, a 1 KB leak per request crashes your device after 520 requests.

**The mental model shift:**

| JavaScript / TypeScript | C |
|---|---|
| Allocate → use → forget | Allocate → use → free (always) |
| GC decides when to free | You decide when to free |
| Memory errors are rare | Memory errors are silent and deadly |
| `undefined` for missing values | Garbage bytes, crashes, security holes |

---

## 2. Stack vs Heap

Two regions of memory matter for this phase.

```
High addresses
+---------------------------+
|         Stack             |  <- grows downward
|  char buf[1024]           |  local variables, function args
|  int x = 5               |  automatic: freed when function returns
|  char *argv[64]           |  fast, limited (~8 MB default on Linux)
|           |               |
|           v               |
|                           |
|           ^               |
|           |               |
|         Heap              |  <- grows upward
|  malloc(n) → ptr         |  manual: YOU decide the lifetime
|  realloc(ptr, n) → ptr   |  can survive past function return
|  free(ptr)                |  unlimited (until RAM runs out)
+---------------------------+
|   BSS / Data / Text       |  global vars, constants, code
+---------------------------+
Low addresses
```

### Concrete example

```c
void example(void) {
    int x = 42;              // Stack: lives until example() returns
    int arr[10];             // Stack: 40 bytes, auto-freed on return

    int *heap_arr = malloc(10 * sizeof(int));  // Heap: survives return
    heap_arr[0] = 99;

    // arr disappears here automatically
    // heap_arr does NOT disappear — you must call free(heap_arr)
}
```

When `example()` returns, `x` and `arr` vanish. `heap_arr` (the pointer variable) also vanishes from the stack, but the memory it was pointing to on the heap is still allocated. You just lost the only handle to it. That is a memory leak.

> **Nvim tip:** With the cursor on `malloc` anywhere in this phase, press `K` to read clangd's inline documentation — it shows the signature, parameter types, and return value description without leaving the editor.

---

## 3. Exercise 01 — `malloc` + `free`

**Goal:** allocate an integer array on the heap, fill it, print it, free it cleanly.

### Step 1 — Create the file

From your terminal (or a Neovim `:terminal` split):

```bash
mkdir -p /home/hidekina/projetos/c-lab/phases/04-memory/ex01
```

Then open the file directly in Neovim:

```bash
nvim /home/hidekina/projetos/c-lab/phases/04-memory/ex01/heap_array.c
```

> **Nvim tip:** clangd activates automatically when you open a `.c` file. You will see LSP diagnostics appear in the gutter within a second or two. If you see a warning you did not expect, hover the squiggly with `K` to read the explanation inline.

### Step 2 — Write the code

```c
/* ex01/heap_array.c
 * Allocate an int array on the heap, use it, free it.
 */
#include <stdio.h>
#include <stdlib.h>   /* malloc, free, exit */

int main(void) {
    int    n = 5;
    int   *arr;
    int    i;

    /* malloc(bytes) returns void* — cast is optional in C but explicit here for clarity */
    arr = (int *)malloc(n * sizeof(int));

    /* ALWAYS check the return value. malloc returns NULL on failure. */
    if (arr == NULL) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    /* Use the array exactly like a stack array */
    for (i = 0; i < n; i++) {
        arr[i] = i * i;   /* 0, 1, 4, 9, 16 */
    }

    for (i = 0; i < n; i++) {
        printf("arr[%d] = %d\n", i, arr[i]);
    }

    /* Release the memory back to the OS */
    free(arr);

    /* Good practice: null the pointer so any accidental use-after-free crashes
     * immediately rather than reading stale data silently */
    arr = NULL;

    return 0;
}
```

The file is auto-formatted on save — indentation and braces are normalized without any manual effort.

### Step 3 — Build and run from Neovim

Press `<leader>cb` to build. The compiler output appears in a terminal split at the bottom. If there are errors, clangd has already underlined them in the editor — fix them and build again.

Press `<leader>cr` to run the binary. Output appears in the same terminal split:

```
arr[0] = 0
arr[1] = 1
arr[2] = 4
arr[3] = 9
arr[4] = 16
```

> **Nvim tip:** You never need to leave the editor. Code is in the top split, terminal output is in the bottom split. Use `Ctrl-w Ctrl-w` to move focus between splits.

### Step 4 — Run valgrind from Neovim

Press `<leader>cv`. This runs `valgrind --leak-check=full ./bin/mysh` — but for exercises, you can adapt the command or run valgrind in the terminal split directly. The output streams into the split below your code.

For this exercise, in the terminal split:

```bash
valgrind --leak-check=full --track-origins=yes ./heap_array
```

Expected valgrind output (clean):

```
==12345== Memcheck, a memory error detector
==12345== Command: ./heap_array
==12345==
arr[0] = 0
arr[1] = 1
arr[2] = 4
arr[3] = 9
arr[4] = 16
==12345==
==12345== HEAP SUMMARY:              <- everything that happened on the heap
==12345==     in use at exit: 0 bytes in 0 blocks   <- ZERO = no leaks
==12345==   total heap usage: 2 allocs, 2 frees, 1,044 bytes allocated
==12345==                                            ^ 1 from printf internals
==12345==                                              1 from your malloc(20)
==12345==
==12345== All heap blocks were freed -- no leaks are possible
==12345==
==12345== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

"In use at exit: 0 bytes" is the green light. Every byte allocated was freed.

> **Nvim tip:** The valgrind output stays in the split — scroll it with the mouse or with `Ctrl-w` focus + normal-mode `j`/`k`. You can cross-reference line numbers in the report against the code above without switching windows.

### Key concepts from ex01

- `sizeof(int)` returns the size of one `int` on your platform (usually 4). Always use `sizeof` — never hardcode 4.
- `malloc` returns `void *`. Assigning to `int *` is an implicit cast in C.
- If `malloc` fails (out of memory) it returns `NULL`. Dereferencing `NULL` is undefined behavior — always check.
- After `free`, set the pointer to `NULL`. It costs nothing and turns silent bugs into loud crashes.

---

## 4. Exercise 02 — Deliberate Memory Leak

**Goal:** write a program that leaks memory, run it through valgrind, and learn to read the leak report.

### Step 1 — Write the leaky code

Open `ex02/leak.c` in Neovim. Note that clangd does not flag the missing `free` as an error — static analysis has limits. This is exactly why you need valgrind.

```c
/* ex02/leak.c
 * Intentionally leaks memory so we can read the valgrind report.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *make_greeting(const char *name) {
    /* Allocate 64 bytes — caller is supposed to free this, but won't */
    char *buf = malloc(64);
    if (buf == NULL) return NULL;
    snprintf(buf, 64, "Hello, %s!", name);
    return buf;
}

int main(void) {
    char *msg = make_greeting("world");
    printf("%s\n", msg);

    /* BUG: we forgot free(msg) here */
    /* In a long-lived server this would accumulate on every request */
    return 0;
}
```

### Step 2 — Build with debug symbols

Press `<leader>cb` or in the terminal split:

```bash
cd /home/hidekina/projetos/c-lab/phases/04-memory/ex02
gcc -Wall -Wextra -g -o leak leak.c
./leak
```

Output: `Hello, world!` — looks fine. No crash. No warning. The bug is invisible to the naked eye.

The `-g` flag includes debug symbols (file names, line numbers) so valgrind can pinpoint the exact allocation site.

### Step 3 — Run through valgrind

In the terminal split (or with `<leader>cv` adapted for this binary):

```bash
valgrind --leak-check=full --track-origins=yes ./leak
```

### Step 4 — Read the report (annotated)

The valgrind output streams into your split. Keep your source code visible in the top window and read the report below — no context switching:

```
==13370== Memcheck, a memory error detector
==13370== Command: ./leak
==13370==
Hello, world!
==13370==
==13370== HEAP SUMMARY:
==13370==     in use at exit: 64 bytes in 1 blocks   <- (A) leak size
==13370==   total heap usage: 2 allocs, 1 frees, 1,088 bytes allocated
==13370==                                 ^^^^^^^^^
==13370==                                 Only 1 free for 2 allocs — mismatch!
==13370==
==13370== 64 bytes in 1 blocks are definitely lost in loss record 1 of 1  <- (B)
==13370==    at 0x4C2FB0F: malloc (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
==13370==    by 0x10869B: make_greeting (leak.c:9)   <- (C) where malloc was called
==13370==    by 0x1086C2: main (leak.c:16)            <- (D) call chain
==13370==
==13370== LEAK SUMMARY:
==13370==    definitely lost: 64 bytes in 1 blocks   <- (E)
==13370==    indirectly lost: 0 bytes in 0 blocks
==13370==      possibly lost: 0 bytes in 0 blocks
==13370==    still reachable: 0 bytes in 0 blocks
==13370==         suppressed: 0 bytes in 0 blocks
==13370==
==13370== ERROR SUMMARY: 0 errors from 0 contexts
```

**Annotation key:**

| Label | What it means |
|---|---|
| (A) "in use at exit: 64 bytes" | 64 bytes were allocated and never freed when the process exited |
| (B) "definitely lost" | valgrind has no pointer that could reach this block — it is truly gone |
| (C) `make_greeting (leak.c:9)` | the `malloc` call is on line 9 of `leak.c` inside `make_greeting` |
| (D) `main (leak.c:16)` | `make_greeting` was called from line 16 of `main` — full call chain |
| (E) LEAK SUMMARY | "definitely lost" = confirmed leak; "possibly lost" = pointer arithmetic may still reach it |

> **Nvim tip:** valgrind reports line numbers. With the report in the bottom split, note "leak.c:16", then switch focus to the source split (`Ctrl-w Ctrl-w`) and jump to line 16 with `16G`. You locate the exact bug site without ever leaving the editor.

### The four loss categories

- **definitely lost** — no pointer can reach this block. Classic leak. Fix it.
- **indirectly lost** — a block whose pointer was inside another leaked block.
- **possibly lost** — a pointer to the middle of a block exists. May be intentional (unusual).
- **still reachable** — memory still pointed to at exit (common for global init code — usually OK).

### Fix

Add `free(msg);` before `return 0;` in `main`. Press `<leader>cb` to rebuild, then re-run valgrind — the leak disappears.

---

## 5. How to Read a Valgrind Report (Reference)

The `:split | terminal` pattern is your friend for this entire phase: code in the top split, valgrind scrolling in the bottom split. To open it manually from Neovim:

```
:split | terminal
```

This opens a horizontal split with a shell. Run valgrind there. The report stays visible while you edit above.

```
==PID== HEAP SUMMARY:
==PID==     in use at exit: N bytes in B blocks
```
- `N bytes` = total unfreed bytes at program exit
- `B blocks` = number of separate `malloc` calls that were not freed
- Target: `0 bytes in 0 blocks`

```
==PID== N bytes in B blocks are definitely lost in loss record R of T
==PID==    at 0xADDR: malloc (vgpreload...)
==PID==    by 0xADDR: function_name (file.c:LINE)
==PID==    by 0xADDR: caller_name   (file.c:LINE)
```
- Read bottom-up: `caller_name` called `function_name` which called `malloc`
- The line number after the colon is where to look in your source
- Compile with `-g` to get line numbers; without it you only get addresses

```
==PID== ERROR SUMMARY: N errors from N contexts
```
- This counts invalid reads/writes, not leaks
- A program can have 0 errors but still leak (leaks are in LEAK SUMMARY)

**Common valgrind flags:**

```bash
--leak-check=full        # Show each leaked block individually
--track-origins=yes      # Show where uninitialized values came from
--show-leak-kinds=all    # Include "still reachable" in the report
-s                       # Print suppressed errors count
```

---

## 6. Exercise 03 — Use-After-Free

**Goal:** access memory after calling `free`, trigger it, detect it with AddressSanitizer, and watch it happen live in the DAP debugger.

### Why it is dangerous

After `free(ptr)`, the allocator may give that memory to a different part of your program or to another thread. Reading it returns stale or corrupt data. Writing to it corrupts data that belongs to something else. In security contexts this is a common exploit primitive.

Valgrind catches some use-after-free bugs but AddressSanitizer (ASan) is faster and more precise for this class.

### Step 1 — Write the buggy code

```c
/* ex03/uaf.c
 * Demonstrate use-after-free and detect it with AddressSanitizer.
 */
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int *p = malloc(sizeof(int));
    if (p == NULL) return 1;

    *p = 42;
    printf("before free: *p = %d\n", *p);

    free(p);

    /* BUG: p is now a dangling pointer — the memory was returned to the allocator */
    printf("after free: *p = %d\n", *p);  /* Use-after-free */

    return 0;
}
```

> **Nvim tip:** After you type the `printf` line that reads `*p` after `free(p)`, watch the gutter. clangd's static analyzer may draw a squiggly under `*p` with a "use of memory after it is freed" diagnostic. Hover it with `K` to read the explanation. This is clangd catching the bug *before* you even compile — but do not rely on this alone; not all use-after-free patterns are statically detectable.

### Step 2 — Explore the bug with DAP before compiling

Understanding use-after-free is much easier when you can watch the pointer's value change in real time. Open `uaf.c` in Neovim and do the following:

1. **Set a breakpoint after `malloc`**: move the cursor to the line `*p = 42;` and press `<leader>db`. A breakpoint marker appears in the gutter.

2. **Set a second breakpoint after `free`**: move the cursor to the `printf("after free..."` line and press `<leader>db`.

3. **Start the debugger**: press `<F5>`. GDB launches and the program stops at the first breakpoint.

4. **Open the DAP REPL**: run `:DapReplOpen`. A split opens with a GDB-style REPL.

5. **Inspect the pointer before free**: in the REPL type:
   ```
   p p
   ```
   You will see something like `$1 = (int *) 0x5555557592a0` — a real heap address.

   Then type:
   ```
   p *p
   ```
   Output: `$2 = 42` — the value you stored.

   For a raw memory view:
   ```
   x/8xb p
   ```
   This dumps 8 bytes at the address in hex — you can see the integer `42` encoded as `0x2a` in the first 4 bytes.

6. **Continue to the second breakpoint**: press `<F5>` again. The program executes `free(p)`.

7. **Inspect the pointer after free**: in the REPL:
   ```
   p *p
   ```
   This access is now undefined behavior. In GDB under valgrind or with ASan instrumentation the REPL will either show garbage, a crash, or an explicit error. This is the use-after-free made tangible — you are looking at the exact moment the pointer becomes dangerous.

> **Nvim tip:** Step over individual lines with `<F10>`. Step into a function call with `<F11>`. Step out of the current function back to the caller with `<F12>`. For this bug, `<F10>` through `malloc`, `*p = 42`, and `free(p)` one line at a time makes the sequence of events unmistakable.

### Step 3 — Compile with AddressSanitizer

```bash
cd /home/hidekina/projetos/c-lab/phases/04-memory/ex03
gcc -Wall -Wextra -g -fsanitize=address -o uaf uaf.c
./uaf
```

### Step 4 — Read the ASan report (annotated)

The ASan report streams into your terminal split. Keep the source above:

```
before free: *p = 42
=================================================================
==14221==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000000010   <- (A)
          at pc 0x55a1b3 bp 0x7ffd8c sp 0x7ffd7c
READ of size 4 at 0x602000000010 thread T0                                        <- (B)
    #0 0x55a1a9 in main uaf.c:16                                                  <- (C)

0x602000000010 is located 0 bytes inside of 4-byte region [0x602000000010,0x602000000014)
freed by thread T0 here:                                                           <- (D)
    #0 0x7f3c2b in __interceptor_free ...
    #1 0x55a17f in main uaf.c:13                                                  <- (E)

previously allocated by thread T0 here:
    #0 0x7f3c87 in __interceptor_malloc ...
    #1 0x55a143 in main uaf.c:8                                                   <- (F)

SUMMARY: AddressSanitizer: heap-use-after-free uaf.c:16 in main
Shadow bytes around the buggy address:                                             <- (G)
  ...
==14221==ABORTING
```

**Annotation key:**

| Label | What it means |
|---|---|
| (A) "heap-use-after-free on address 0x..." | which address was accessed illegally |
| (B) "READ of size 4" | it was a read (not a write) of 4 bytes (one int) |
| (C) `uaf.c:16` | line 16 of your source — the `printf` with `*p` after free |
| (D) "freed by thread T0 here" | the free that invalidated this pointer |
| (E) `uaf.c:13` | line 13 — that is the `free(p)` call |
| (F) `uaf.c:8` | line 8 — the original `malloc` |
| (G) Shadow bytes | ASan memory map showing poisoned (red) vs valid (green) regions |

ASan tells you three things in order: where you used the bad pointer, where it was freed, and where it was originally allocated. That is everything you need to fix the bug.

> **Nvim tip:** ASan reports `uaf.c:16` as the violation site. In the source split, jump there with `16G`. `gr` lists all references to `p` across the file — you can see the alloc, the free, and the bad access all in one list.

### Fix

Either stop using `p` after `free(p)`, or set `p = NULL` immediately after `free` so that any subsequent dereference crashes immediately on a null dereference rather than silently reading stale memory.

### Compile without sanitizer for comparison

```bash
gcc -Wall -Wextra -g -o uaf_plain uaf.c
./uaf_plain
```

You may see `before free: *p = 42` followed by `after free: *p = 42` — the stale value. No crash, no warning. This is why ASan exists: the bug is invisible without tooling.

---

## 7. How to Read an AddressSanitizer Report (Reference)

ASan instruments every memory access at compile time. When a violation occurs it prints:

```
ERROR: AddressSanitizer: <error-type> on address <addr>
```

**Common error types:**

| Type | Meaning |
|---|---|
| `heap-use-after-free` | read/write after `free()` |
| `heap-buffer-overflow` | read/write past end of `malloc`'d block |
| `stack-buffer-overflow` | read/write past end of a stack array |
| `double-free` | `free()` called twice on same pointer |
| `use-after-return` | pointer to a local variable used after function returned |

**The stack trace format:**

```
    #0 0xADDR in function_name file.c:LINE
    #1 0xADDR in caller_name  file.c:LINE
```

Frame `#0` is where the violation happened. Read downward for the call chain. The file and line are exact — go straight there with `<LINE>G` in Neovim.

**Enabling ASan:**

```bash
gcc -fsanitize=address -g -o program program.c
# or clang:
clang -fsanitize=address -g -o program program.c
```

ASan has ~2x memory overhead and ~2x slowdown — use it during development and testing, not in production.

---

## 8. Exercise 04 — Dynamic Array

**Goal:** implement a growable array similar to JavaScript's `Array` or C++'s `std::vector`, using `malloc`, `realloc`, and `free`. This exercise is where `K` and the DAP REPL will earn their keep.

### Step 1 — Design the data structure

```c
/* ex04/dynarray.h */
#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <stddef.h>  /* size_t */

typedef struct {
    int    *data;     /* pointer to heap-allocated buffer */
    size_t  len;      /* number of elements currently stored */
    size_t  cap;      /* allocated capacity (number of ints the buffer can hold) */
} DynArray;

/* Initialize a DynArray. Must be called before use. Returns 0 on success, -1 on error. */
int  dynarray_init(DynArray *da, size_t initial_cap);

/* Append a value. Doubles capacity when full. Returns 0 on success, -1 on error. */
int  dynarray_push(DynArray *da, int value);

/* Get element at index. Returns the value. Calls abort() on out-of-bounds. */
int  dynarray_get(const DynArray *da, size_t index);

/* Free all heap memory. Sets all fields to zero/NULL. */
void dynarray_free_all(DynArray *da);

#endif /* DYNARRAY_H */
```

> **Nvim tip:** `gd` on `dynarray_push` in `main.c` jumps to the declaration in `dynarray.h`. `gr` on it lists every call site in the project. These are your primary navigation tools for multi-file C code.

### Step 2 — Implement

```c
/* ex04/dynarray.c */
#include <stdio.h>
#include <stdlib.h>
#include "dynarray.h"

int dynarray_init(DynArray *da, size_t initial_cap) {
    da->data = malloc(initial_cap * sizeof(int));
    if (da->data == NULL) return -1;
    da->len  = 0;
    da->cap  = initial_cap;
    return 0;
}

int dynarray_push(DynArray *da, int value) {
    /* If full, double the capacity */
    if (da->len == da->cap) {
        size_t new_cap  = da->cap * 2;
        int   *new_data = realloc(da->data, new_cap * sizeof(int));
        /* IMPORTANT: do NOT write da->data = realloc(da->data, ...) directly.
         * If realloc fails it returns NULL but does NOT free the old block.
         * Writing NULL over da->data would lose the only pointer to it — a leak. */
        if (new_data == NULL) return -1;
        da->data = new_data;
        da->cap  = new_cap;
    }

    da->data[da->len] = value;
    da->len++;
    return 0;
}

int dynarray_get(const DynArray *da, size_t index) {
    if (index >= da->len) {
        fprintf(stderr, "dynarray_get: index %zu out of bounds (len=%zu)\n",
                index, da->len);
        abort();  /* crash loudly rather than silently returning garbage */
    }
    return da->data[index];
}

void dynarray_free_all(DynArray *da) {
    free(da->data);
    da->data = NULL;
    da->len  = 0;
    da->cap  = 0;
}
```

> **Nvim tip:** While typing the `realloc` call, position the cursor on `realloc` and press `K`. clangd displays the full signature:
> ```
> void *realloc(void *ptr, size_t size)
> ```
> and the man-page description: "If the reallocation fails, the original block is left untouched." This is the exact reason for the temp-pointer pattern — the documentation is one keystroke away.

### Step 3 — Watch realloc live with DAP

Before writing `main.c`, use the debugger to observe `realloc` moving the buffer:

1. Set a breakpoint inside `dynarray_push` at the `realloc` line with `<leader>db`.
2. Press `<F5>` to start the debugger.
3. Each time the breakpoint hits, open the REPL (`:DapReplOpen`) and type:
   ```
   p da->data
   p da->cap
   p da->len
   ```
4. Press `<F10>` to step over the `realloc` call.
5. In the REPL again:
   ```
   p da->data
   ```
   The address has changed — `realloc` moved the buffer to a larger region. The old address is now invalid. The temp-pointer pattern (`new_data`) protects you if this call had failed.

> **Nvim tip:** `<F11>` steps *into* `realloc`. You will land in glibc internals. Press `<F12>` to step back out immediately to your code.

### Step 4 — Write a test driver

```c
/* ex04/main.c */
#include <stdio.h>
#include "dynarray.h"

int main(void) {
    DynArray da;
    int      i;

    if (dynarray_init(&da, 2) != 0) {  /* start small to force realloc */
        fprintf(stderr, "init failed\n");
        return 1;
    }

    for (i = 0; i < 10; i++) {
        if (dynarray_push(&da, i * 3) != 0) {
            fprintf(stderr, "push failed at i=%d\n", i);
            dynarray_free_all(&da);
            return 1;
        }
    }

    printf("len=%zu, cap=%zu\n", da.len, da.cap);  /* expect len=10, cap=16 */

    for (i = 0; i < (int)da.len; i++) {
        printf("da[%d] = %d\n", i, dynarray_get(&da, i));
    }

    dynarray_free_all(&da);
    return 0;
}
```

### Step 5 — Build and run from Neovim

Press `<leader>cb` to build. Then `<leader>cr` to run. Expected output in the terminal split:

```
len=10, cap=16
da[0] = 0
da[1] = 3
da[2] = 6
da[3] = 9
da[4] = 12
da[5] = 15
da[6] = 18
da[7] = 21
da[8] = 24
da[9] = 27
```

Cap is 16 because: initial=2 → push fills it → realloc to 4 → realloc to 8 → realloc to 16.

### Step 6 — Run under valgrind from Neovim

In the terminal split (keep source visible above):

```bash
valgrind --leak-check=full --track-origins=yes ./dynarray
```

All heap blocks should be freed. No errors. The report confirms every `malloc`/`realloc` was matched by a `free`.

### Step 7 — Run with ASan

```bash
gcc -Wall -Wextra -g -fsanitize=address -o dynarray_asan main.c dynarray.c
./dynarray_asan
```

No errors should appear. If they do, the ASan report will give you the exact file and line — navigate there with `<LINE>G`.

### Key concept: the `realloc` safety pattern

This is the most common `realloc` mistake:

```c
/* WRONG — if realloc returns NULL, you leak the old block */
da->data = realloc(da->data, new_cap * sizeof(int));

/* CORRECT — save to a temp pointer first */
int *tmp = realloc(da->data, new_cap * sizeof(int));
if (tmp == NULL) {
    /* da->data still valid — caller can handle the error */
    return -1;
}
da->data = tmp;
```

---

## 9. Common Mistakes

### Double free

```c
int *p = malloc(sizeof(int));
free(p);
free(p);  /* undefined behavior — allocator internals are now corrupt */
```

**Detection:** ASan reports `double-free`. Valgrind reports `Invalid free`.

**Prevention:** set `p = NULL` after every `free`. `free(NULL)` is a no-op — safe.

> **Nvim tip:** `<leader>rn` renames a variable project-wide. If you rename a pointer to something more descriptive, clangd updates every reference — including all the places you need to add `= NULL` after `free`.

### Forgetting to free in error paths

```c
/* LEAKY */
int process(void) {
    char *buf = malloc(1024);
    if (buf == NULL) return -1;

    if (open_file() < 0) {
        return -1;   /* BUG: buf is never freed on this path */
    }

    /* ... */
    free(buf);
    return 0;
}

/* CORRECT */
int process(void) {
    char *buf = malloc(1024);
    if (buf == NULL) return -1;

    if (open_file() < 0) {
        free(buf);   /* every exit path must free */
        return -1;
    }

    /* ... */
    free(buf);
    return 0;
}
```

This is the most common real-world leak pattern. Every function that allocates memory must have a cleanup label or explicit `free` on every `return` path.

In C this is often handled with a `goto cleanup` pattern:

```c
int process(void) {
    char *buf  = NULL;
    FILE *file = NULL;
    int   ret  = -1;

    buf = malloc(1024);
    if (buf == NULL) goto cleanup;

    file = fopen("data.txt", "r");
    if (file == NULL) goto cleanup;

    /* ... do work ... */
    ret = 0;

cleanup:
    if (file) fclose(file);
    free(buf);      /* free(NULL) is safe */
    return ret;
}
```

### Off-by-one on malloc size

```c
char *str = malloc(strlen("hello"));    /* WRONG: no room for '\0' */
char *str = malloc(strlen("hello") + 1); /* CORRECT */
strcpy(str, "hello");
```

Valgrind and ASan will both catch writes past the allocated boundary.

### Using freed memory as a return value

```c
/* WRONG */
char *get_name(void) {
    char buf[32];           /* stack allocated */
    snprintf(buf, 32, "Alice");
    return buf;             /* BUG: returns pointer to stack memory that no longer exists */
}

/* CORRECT */
char *get_name(void) {
    char *buf = malloc(32); /* heap allocated — survives past function return */
    if (buf == NULL) return NULL;
    snprintf(buf, 32, "Alice");
    return buf;             /* caller must free this */
}
```

> **Nvim tip:** `<leader>ca` on a line with a clangd warning opens the code actions menu. For some mistakes (unused variables, missing `const`, simple fixes) clangd offers one-click repairs directly in the editor.

---

## 10. mysh Milestone — Phase 04

**Goal:** add external command execution to `mysh` using `fork()` + `execvp()`. Run the result under valgrind from inside Neovim without ever leaving the editor.

### Background

`mysh/src/main.c` already calls `mysh_builtin()` for commands like `cd` and `exit`. The stub comment at line 27 reads:

```c
/* Phase 04: fork + exec goes here */
fprintf(stderr, "mysh: %s: command not found\n", argv[0]);
```

You will replace that stub by adding `src/exec.c` and declaring `mysh_exec` in `mysh.h`.

### Step 1 — Navigate with LSP

Open `mysh/src/main.c` in Neovim. With the cursor on `mysh_builtin`, press `gd` to jump to its declaration in `mysh.h`. You can see the existing function signatures and the pattern you need to follow. Press `Ctrl-o` to jump back to `main.c`.

> **Nvim tip:** `gr` on `mysh_builtin` lists every call site across the project. Since you are about to add a parallel function `mysh_exec`, this gives you a sense of how `mysh_builtin` is wired in — then you mirror that pattern.

### Step 2 — Add the declaration to `mysh.h`

In `/home/hidekina/projetos/c-lab/mysh/include/mysh.h`, add after the existing declarations:

```c
/* Execute an external command by forking and calling execvp.
 * argv must be NULL-terminated.
 * Returns the exit status of the child, or -1 on fork/exec error. */
int mysh_exec(char **argv);
```

### Step 3 — Create `src/exec.c`

Open a new buffer in Neovim:

```
:e /home/hidekina/projetos/c-lab/mysh/src/exec.c
```

clangd activates immediately for the new file. As you type `#include "mysh.h"`, the LSP resolves the header and gives you completion for `mysh_exec`.

```c
/* mysh/src/exec.c
 * External command execution via fork + execvp.
 *
 * Memory note: execvp replaces the child process image entirely.
 * The child inherits the parent's heap but that heap is discarded
 * at exec — no leaks propagate from child to parent.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>    /* fork, execvp */
#include <sys/wait.h>  /* waitpid, WIFEXITED, WEXITSTATUS */
#include "mysh.h"

int mysh_exec(char **argv) {
    pid_t pid;
    int   status;

    pid = fork();

    if (pid < 0) {
        /* fork failed — OS could not create a new process */
        perror("mysh: fork");
        return -1;
    }

    if (pid == 0) {
        /* We are the child process.
         * execvp searches PATH for argv[0] and replaces this process image.
         * argv must be NULL-terminated — mysh_tokenize already does this. */
        execvp(argv[0], argv);

        /* execvp only returns on failure */
        perror(argv[0]);
        exit(127);  /* 127 = "command not found" convention */
    }

    /* We are the parent. Wait for the child to finish. */
    if (waitpid(pid, &status, 0) < 0) {
        perror("mysh: waitpid");
        return -1;
    }

    /* Extract the child's exit code from the status word */
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return -1;
}
```

> **Nvim tip:** With the cursor on `WIFEXITED` or `WEXITSTATUS`, press `K`. clangd shows the macro description: these extract the exit status from the raw `status` integer returned by `waitpid`. You understand the code without opening a browser or a man page.

### Step 4 — Wire it into `main.c`

Replace the stub in `mysh/src/main.c`:

```c
        if (mysh_builtin(argc, argv))
            continue;

        /* Phase 04: fork + exec goes here */    <- remove this comment
        fprintf(stderr, "mysh: %s: command not found\n", argv[0]);  <- remove this line
```

With:

```c
        if (mysh_builtin(argc, argv))
            continue;

        mysh_exec(argv);
```

### Step 5 — Build from Neovim

Press `<leader>cb`. The build output appears in the terminal split. Fix any errors (clangd will have already underlined them in the source).

> **Nvim tip:** If the build fails with "undefined reference to mysh_exec", verify that `exec.c` is included in the Makefile or build command. `gr` on `mysh_exec` in `main.c` will show zero results if the declaration is missing from `mysh.h` — that is your diagnostic.

### Step 6 — Quick smoke test

Press `<leader>cr` to launch `./bin/mysh` in the terminal split:

```
mysh> ls -l
mysh> echo hello world
mysh> exit
```

External commands execute normally — all without leaving Neovim.

### Step 7 — Verify no leaks with `<leader>cv`

This is the payoff of the entire phase. Press `<leader>cv`. This runs:

```
valgrind --leak-check=full ./bin/mysh
```

The valgrind output streams into the terminal split below your source code. At the valgrind prompt type:

```
mysh> echo test
mysh> exit
```

Then read the report in the split below. Expected result:

```
==XXXX== HEAP SUMMARY:
==XXXX==     in use at exit: 0 bytes in 0 blocks
==XXXX==   total heap usage: N allocs, N frees, N bytes allocated
==XXXX==
==XXXX== All heap blocks were freed -- no leaks are possible
==XXXX==
==XXXX== ERROR SUMMARY: 0 errors from 0 contexts
```

You ran valgrind, read the report, and confirmed zero leaks — all from within Neovim, with the source code visible in the split above.

> **Nvim tip:** If you see a non-zero "in use at exit", the leak report names the file and line. Switch focus to the source split with `Ctrl-w Ctrl-w`, jump to that line with `<LINE>G`, and fix. Press `<leader>cb` and `<leader>cv` again. Iterate until clean.

### How `fork` + `exec` relates to memory

When you call `fork()`:
- The OS creates a copy of your process (child gets a copy of the entire heap).
- The child's copy is independent — leaks in the child do not affect the parent.

When `execvp()` succeeds:
- The child's process image is completely replaced by the new program.
- The child's heap is thrown away by the OS — no cleanup needed in the child.

This is why `exec` is not a memory leak even though the child never calls `free`: the OS reclaims all memory when the process image is replaced (or when the child exits).

What you must verify in the parent:
- No buffers allocated before `fork` are leaked on the fork-failure path.
- `waitpid` is always called so zombie processes are reaped.

---

## 11. Neovim Workflow Summary for Phase 04

| Task | Keymap / Command |
|---|---|
| Jump to definition of a function | `gd` |
| List all references to a symbol | `gr` |
| Read docs / man-page for symbol under cursor | `K` |
| Rename symbol everywhere | `<leader>rn` |
| Code actions (quick fixes) | `<leader>ca` |
| Build | `<leader>cb` |
| Run | `<leader>cr` |
| Run valgrind on mysh | `<leader>cv` |
| Toggle breakpoint | `<leader>db` |
| Start / continue debugger | `<F5>` |
| Step over | `<F10>` |
| Step into | `<F11>` |
| Step out | `<F12>` |
| Open DAP REPL | `:DapReplOpen` |
| Open terminal split | `:split \| terminal` |
| Move between splits | `Ctrl-w Ctrl-w` |
| Jump to line N | `<N>G` |

**Key DAP REPL expressions for heap debugging:**

```
p ptr          -- print pointer address
p *ptr         -- dereference: print value at address
x/8xb ptr      -- dump 8 bytes at address in hex (raw memory view)
p da->len      -- struct field
p da->data[3]  -- array element through pointer
```

---

## 12. Checklist

Go through this list before moving to Phase 05:

- [ ] ex01: heap array compiles, runs, valgrind shows 0 leaks — verified from Neovim terminal split
- [ ] ex02: leaky program identified and fixed, valgrind report read without leaving the editor
- [ ] ex03: use-after-free triggered live in DAP — pointer inspected before and after `free` in the REPL
- [ ] ex03: ASan report annotated; clangd inline warning observed
- [ ] ex04: dynamic array with `init`, `push`, `get`, `free_all`; valgrind clean; ASan clean
- [ ] ex04: `K` on `realloc` used to read the signature before implementing the temp-pointer pattern
- [ ] ex04: DAP used to watch `da->data` address change across a `realloc` call
- [ ] ex04: the `realloc` safety pattern is used (temp pointer, not direct assign)
- [ ] ex04: all error paths call `free` before returning
- [ ] mysh: `src/exec.c` added with `mysh_exec(char **argv)`
- [ ] mysh: `mysh.h` declares `mysh_exec`
- [ ] mysh: `main.c` calls `mysh_exec` instead of printing "command not found"
- [ ] mysh: `<leader>cv` run — valgrind shows 0 bytes in use at exit after `exit` command
- [ ] Habit: every `malloc` is paired with `free`
- [ ] Habit: every `free` is followed by `ptr = NULL`
- [ ] Habit: `realloc` uses a temp pointer
- [ ] Habit: `-g` flag always used during development for readable tool output

---

*Next: Phase 05 — Structs, Pointers-to-Structs, and RAII-style cleanup patterns.*
