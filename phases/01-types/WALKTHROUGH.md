# Phase 01 Walkthrough: Types & Undefined Behavior

## What you will learn

Coming from JavaScript or TypeScript, you are used to a world where numbers just work. `42 + 1` is `43`. You never ask "how many bytes is that number?" because V8 handles it for you — all JS numbers are 64-bit IEEE 754 floats under the hood.

C is different. When you declare a variable, you are directly reserving memory. The type you pick determines exactly how many bytes are allocated and what values are valid. Pick the wrong type and the computer will silently do the wrong thing — no exception, no runtime error, no TypeScript squiggle.

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

The key insight: in JS, overflow wraps around in bitwise ops and saturates in arithmetic. In C, signed integer overflow is **Undefined Behavior** — the compiler is free to assume it never happens and may produce code that deletes your data, loops forever, or crashes. `uint8_t` overflow, on the other hand, is defined: it wraps modulo 256.

---

## Setup

All exercises live in `phases/01-types/exercises/`. Create that directory structure as you go. You will compile each file manually with `gcc` or `clang`.

```bash
cd /home/hidekina/projetos/c-lab/phases/01-types/exercises
```

---

## Exercise 01 — sizeof all basic types

**Goal:** print the size in bytes of every basic C type so you have a concrete mental map.

### Step 1: create the file

Create `ex01_sizeof.c`:

```c
#include <stdio.h>

int main(void) {
    printf("char       : %zu bytes\n", sizeof(char));
    printf("short      : %zu bytes\n", sizeof(short));
    printf("int        : %zu bytes\n", sizeof(int));
    printf("long       : %zu bytes\n", sizeof(long));
    printf("long long  : %zu bytes\n", sizeof(long long));
    printf("float      : %zu bytes\n", sizeof(float));
    printf("double     : %zu bytes\n", sizeof(double));
    printf("pointer    : %zu bytes\n", sizeof(void *));
    return 0;
}
```

Note: `sizeof` returns a value of type `size_t`. The correct format specifier for it is `%zu`. Using `%d` would work on most 64-bit machines but is technically UB — a habit worth avoiding from day one.

### Step 2: compile and run

```bash
gcc -Wall -Wextra -o ex01 ex01_sizeof.c && ./ex01
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
```

### Why it works this way

`sizeof` is evaluated **at compile time** — it is not a function call. The compiler replaces `sizeof(int)` with the literal `4` before your program runs. This is important: it means there is zero runtime cost.

The sizes above are for x86-64 Linux. On a 32-bit ARM microcontroller `long` might be 4 bytes. On MSVC (Windows) `long` is 4 bytes even on 64-bit. This is why you should never assume the size of `int` or `long` in portable code.

Notice `pointer` is 8 bytes on 64-bit. Every pointer — regardless of what it points to — is the same size, because a pointer is just a memory address.

---

## Exercise 02 — trigger integer overflow

**Goal:** make a signed integer exceed its maximum value and observe what happens (and what the compiler says).

### Step 1: create the file

Create `ex02_overflow.c`:

```c
#include <stdio.h>
#include <limits.h>

int main(void) {
    int max = INT_MAX;              /* 2,147,483,647 */
    int overflow = max + 1;         /* signed overflow: UB */

    printf("INT_MAX          : %d\n", max);
    printf("INT_MAX + 1      : %d\n", overflow);
    printf("Expected in JS   : 2147483648\n");
    return 0;
}
```

### Step 2: compile without optimizations first

```bash
gcc -Wall -Wextra -o ex02 ex02_overflow.c && ./ex02
```

Expected output (unoptimized build):

```
INT_MAX          : 2147483647
INT_MAX + 1      : -2147483648
Expected in JS   : 2147483648
```

The value wrapped around to the most negative `int`. This looks like "it works" — and it usually does without optimizations. That is the danger.

### Step 3: compile with optimizations and observe the compiler warning

```bash
gcc -Wall -Wextra -O2 -o ex02_opt ex02_overflow.c
```

GCC will emit:

```
ex02_overflow.c:6:24: warning: integer overflow in expression of type 'int' results
in '-2147483648' [-Woverflow]
    6 |     int overflow = max + 1;
      |                        ^
```

With `-O2` enabled, GCC may optimize out the entire branch guarded by a signed overflow assumption. A real-world example: a loop like `for (int i = 0; i >= 0; i++)` is an infinite loop in JS but the compiler may turn it into an infinite loop in C too — by assuming `i` never overflows (because that would be UB) and therefore the condition is always true.

### Step 4: try Clang for comparison

```bash
clang -Wall -Wextra -o ex02_clang ex02_overflow.c && ./ex02_clang
```

Clang may produce similar output but with slightly different warning text. Both compilers warn; neither treats this as a compile error by default.

---

## Exercise 03 — trigger and detect UB with sanitizers

**Goal:** read an uninitialized variable, which is classic Undefined Behavior. Use AddressSanitizer (ASan) and UBSan to catch it.

### Step 1: create the file

Create `ex03_ub.c`:

```c
#include <stdio.h>

int main(void) {
    int x;              /* declared but never assigned */
    printf("x = %d\n", x);  /* reading uninitialized memory: UB */
    return 0;
}
```

### Step 2: compile and run without sanitizers

```bash
gcc -Wall -Wextra -o ex03 ex03_ub.c && ./ex03
```

GCC will warn:

```
ex03_ub.c:5:5: warning: 'x' is used uninitialized [-Wuninitialized]
```

The program may print `x = 0` or `x = 32767` or any other garbage value from the stack. It might even print `x = 0` consistently, giving you false confidence. This is the core problem with UB: it is not reliably visible without tooling.

### Step 3: compile with sanitizers

```bash
gcc -fsanitize=address,undefined -g -o ex03_san ex03_ub.c && ./ex03_san
```

The `-g` flag includes debug symbols so the sanitizer can print line numbers. The `-fsanitize=address,undefined` flag instruments the binary to detect memory errors and UB at runtime.

Expected sanitizer output:

```
ex03_ub.c:5:20: runtime error: load of value 32764, which is not a valid value for type 'int'
```

Or from UBSan more specifically:

```
ex03_ub.c:5:20: runtime error: load of uninitialized value of type 'int'
```

The program aborts with a non-zero exit code. The sanitizer caught what the compiler only warned about.

### Step 4: understand what ASan and UBSan each do

**UBSan** (Undefined Behavior Sanitizer) instruments operations that the C standard defines as UB: signed overflow, uninitialized reads, null pointer dereference, misaligned access, out-of-bounds array indexing, and more. It adds checks around each such operation at runtime.

**ASan** (Address Sanitizer) focuses on memory safety: use-after-free, heap buffer overflow, stack buffer overflow, use-after-return. It works by inserting "poison" zones around allocations and checking every load/store.

Both are enabled with a single flag. The only cost is ~2x slowdown and increased memory — acceptable during development and in CI, but not in production binaries.

### Step 5: verify with Valgrind (optional, alternative tool)

If you have Valgrind installed:

```bash
gcc -g -o ex03_val ex03_ub.c && valgrind --track-origins=yes ./ex03_val
```

Valgrind will report:

```
==PID== Conditional jump or move depends on uninitialised value(s)
==PID==    at 0x... (printf)
==PID==    by 0x... (main) ex03_ub.c:5
==PID== Uninitialised value was created by a stack allocation
==PID==    at 0x... (main) ex03_ub.c:4
```

Sanitizers are faster than Valgrind and are preferred for regular development cycles.

---

## Exercise 04 — rewrite with fixed-width types

**Goal:** replace `int` with `uint8_t` and `int32_t` from `<stdint.h>` to make type sizes explicit and portable.

### Step 1: create the file

Create `ex04_stdint.c`:

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

### Step 2: compile and run

```bash
gcc -Wall -Wextra -o ex04 ex04_stdint.c && ./ex04
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

### Step 3: understand the printf macros

`PRIu8`, `PRId32`, etc. are macros from `<inttypes.h>` that expand to the correct format string for each fixed-width type on the current platform. On a 32-bit machine `int32_t` might be a `long`, so `%d` would be wrong. These macros guarantee correctness:

```c
printf("%" PRId32 "\n", my_int32);
/* expands to printf("%d\n", ...) on 64-bit Linux */
/* expands to printf("%ld\n", ...) where needed   */
```

This is verbose but correct. In practice, many developers skip the macros for quick programs and rely on the fact that `int32_t` is `int` on all mainstream 64-bit targets — but for embedded or cross-platform code, use the macros.

### Step 4: run with sanitizers to confirm no UB

```bash
gcc -fsanitize=address,undefined -g -o ex04_san ex04_stdint.c && ./ex04_san
```

No errors. The `uint8_t` wrap is defined behavior (unsigned overflow wraps modulo 256 by the C standard). The sanitizer stays silent.

---

## Common mistakes

### Mistake 1: using `%d` for `size_t`

```c
/* WRONG */
printf("size: %d\n", sizeof(int));

/* CORRECT */
printf("size: %zu\n", sizeof(int));
```

`sizeof` returns `size_t`, which is unsigned and may be 64-bit. `%d` expects a signed 32-bit int. On 64-bit Linux this usually prints the right number, but it is UB and UBSan will flag it.

### Mistake 2: assuming signed overflow wraps

```c
int x = INT_MAX;
if (x + 1 < 0) {    /* a common "check" for overflow */
    printf("overflow!\n");
}
```

With `-O2`, GCC sees that `x + 1 < 0` is UB (signed overflow), assumes it never happens, and **removes the branch entirely**. Use `__builtin_add_overflow` or cast to `unsigned` to check safely:

```c
unsigned int ux = (unsigned int)x;
if (ux + 1 > INT_MAX) { /* safe: unsigned overflow is defined */ }
```

### Mistake 3: forgetting `-g` with sanitizers

Without debug symbols the sanitizer output points to a hex address instead of a file and line. Always combine:

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

`-Wall` will warn: `comparison of integer expressions of different signedness`. Never ignore that warning.

### Mistake 5: leaving variables uninitialized "for performance"

In C you can declare a variable without initializing it. In JavaScript, declared variables are `undefined`. In C, the variable contains whatever bytes happened to be in that memory location. Always initialize:

```c
int x = 0;          /* safe */
uint8_t buf[256];   /* stack array: garbage until you fill it */
memset(buf, 0, sizeof(buf));  /* now it is zeroed */
```

---

## mysh milestone: implement the input loop

The mysh milestone for Phase 01 is to understand and verify the `mysh_readline` function and `echo` builtin. Both already exist as working implementations in `mysh/src/`. Your job in this phase is to **read and understand them** in the context of what you just learned about types, buffers, and UB.

### The readline implementation

Open `/home/hidekina/projetos/c-lab/mysh/src/readline.c`:

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

**What to notice through a Phase 01 lens:**

1. `buf` is a `char *` — a pointer to a region of memory the caller owns. The function writes into it. If the caller passes a buffer of 512 bytes but `size` says 1024, that is a buffer overflow — classic UB. The contract is: `size` must accurately reflect how many bytes `buf` can hold.

2. `fgets(buf, size, stdin)` reads at most `size - 1` characters and always null-terminates. This is the safe alternative to `gets()`, which has no size limit and was removed from C11.

3. `strlen` returns `size_t` (unsigned). The cast `(int)strlen(buf)` is intentional here — `size` is `int`, so keeping everything signed avoids mixed-signedness comparisons. On a 512-byte shell buffer this is fine; for large data it would be a concern.

4. The function returns `-1` on EOF (end of input / Ctrl+D). The caller in `main.c` uses this to exit the shell loop.

### The echo builtin

Open `/home/hidekina/projetos/c-lab/mysh/src/builtins.c`:

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

1. `argv` is `char **` — an array of string pointers. `argc` is the count. This mirrors the `main(int argc, char **argv)` signature you will see everywhere.

2. The loop variable `i` is `int`. `argc` is `int`. No mixed-signedness issue here — both are signed.

3. The function returns `1` to indicate "this was a builtin, handled." It returns `0` when no builtin matched, so the caller knows to look for an external command.

### Build and test the shell

```bash
cd /home/hidekina/projetos/c-lab/mysh
make
./bin/mysh
```

At the prompt, type:

```
echo hello world
```

Expected output:

```
hello world
```

Type `exit` to quit or press Ctrl+D.

### Build with sanitizers

```bash
cd /home/hidekina/projetos/c-lab/mysh
make clean
CFLAGS="-fsanitize=address,undefined -g" make
./bin/mysh
```

Run a few echo commands. If the sanitizer stays silent, the implementation is clean. This is your verification that the readline buffer handling has no UB.

---

## Completion checklist

Mark these off before moving to Phase 02:

- [ ] `ex01_sizeof.c` compiles and prints correct sizes for all 8 types listed above.
- [ ] `ex02_overflow.c` compiles; you have read and understood the `-Woverflow` warning.
- [ ] You can explain why `-O2` makes signed overflow more dangerous, not less.
- [ ] `ex03_ub.c` compiles and runs; the sanitizer catches the uninitialized read.
- [ ] You can run the same binary without sanitizers and observe that the bug is invisible.
- [ ] `ex04_stdint.c` compiles; `uint8_t` wraps to 0 at 255 + 1; no sanitizer errors.
- [ ] You know the difference between `uint8_t` wrap (defined) and `int32_t` overflow (UB).
- [ ] `mysh` builds with `make` and the `echo` builtin prints arguments correctly.
- [ ] `mysh` builds with `-fsanitize=address,undefined` and the sanitizer stays silent during normal use.
- [ ] You have read `readline.c` and `builtins.c` and can explain what each line does in terms of types and memory.
