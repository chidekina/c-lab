# Phase 02: Pointers & Strings — Step-by-Step Walkthrough

**Audience:** Web developer (JavaScript/TypeScript background) learning C for the first time.

---

## What You Will Learn

By the end of this phase you will:

- Understand what a pointer *actually is* at the machine level (not magic, just an address).
- Read and write the pointer syntax: `int *p`, `&x`, `*p`.
- Walk an array using pointer arithmetic instead of index subscripts.
- Implement `strlen` and `strcpy` from scratch — the same way the C standard library does it.
- Know when and why `const char *` matters.
- Rewrite a real function in `mysh` (`mysh_tokenize`) using pointer walking instead of `strtok`.

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

### Step 2 — Compile and run

```bash
gcc -Wall -Wextra -o ex01 ex01.c && ./ex01
```

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

### Step 2 — Compile and run

```bash
gcc -Wall -Wextra -o ex02 ex02.c && ./ex02
```

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

### Step 2 — Compile and run

```bash
gcc -Wall -Wextra -o ex03 ex03.c && ./ex03
```

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

### Step 2 — Compile and run

```bash
gcc -Wall -Wextra -o ex04 ex04.c && ./ex04
```

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

### Step 2 — Compile and run

```bash
gcc -Wall -Wextra -o ex05 ex05.c && ./ex05
```

### Expected output

```
uppercase letters: 2
modified: hello World
```

### Step 3 — Intentionally trigger the error (learning exercise)

Uncomment the line `/* literal[0] = 'h'; */` and recompile. You should get:

```
ex05.c:26:15: error: assignment of read-only location '*(const char *)literal'
```

Then re-comment it.

### Why it works

| Declaration        | Can change what it points to? | Can change the value it points to? |
|--------------------|-------------------------------|------------------------------------|
| `char *p`          | Yes                           | Yes                                |
| `const char *p`    | Yes                           | No                                 |
| `char *const p`    | No                            | Yes                                |
| `const char *const p` | No                         | No                                 |

Rule of thumb: **`const` applies to what is to its left**. If nothing is to the left, it applies to what is to its right.

String literals like `"Hello World"` are stored in read-only memory. Writing to them is undefined behavior. Always declare them as `const char *`.

---

## Common Mistakes

### 1. NULL pointer dereference

```c
int *p = NULL;
*p = 42;  /* segmentation fault — there is no memory at address 0 */
```

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

Open `/home/hidekina/projetos/c-lab/mysh/src/tokenize.c` and replace its contents with:

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

### Step-by-step: apply and test

```bash
cd /home/hidekina/projetos/c-lab/mysh

# Edit the file (or paste the new code)
# Then rebuild:
make

# Run the shell and test:
./bin/mysh
> echo hello world     # should print: hello world
> ls -la               # should list files
> exit
```

If `make` is not available, compile directly:

```bash
gcc -Wall -Wextra -Iinclude -o bin/mysh \
    src/main.c src/readline.c src/tokenize.c src/builtins.c
```

### What to verify

- `echo hello world` still prints `hello world`.
- Multiple spaces between arguments are handled (they are skipped by the inner `while`).
- An empty input line returns `argc == 0` and does not crash.
- A line with only spaces/tabs returns `argc == 0`.

---

## Completion Checklist

Mark each item when done:

- [ ] ex01 compiled and ran — printed an address in hex.
- [ ] ex01 confirmed that writing through `*p` changed `x`.
- [ ] ex02 printed all 5 array values using `*(p+i)` and `p++` styles.
- [ ] ex02 observed the 4-byte gaps between `int` addresses.
- [ ] ex03 `my_strlen` returns correct length for empty string and non-empty strings.
- [ ] ex03 understands pointer subtraction as element count.
- [ ] ex04 `my_strcpy` copies correctly and null-terminates.
- [ ] ex04 attempted the compact `(*dst++ = *src++)` version (even just reading it).
- [ ] ex05 triggered the `const` compile error intentionally and read the message.
- [ ] ex05 can explain the difference between `const char *p` and `char *const p`.
- [ ] mysh tokenizer rewritten — `strtok` removed, pointer walk in place.
- [ ] mysh rebuilds with `make` without warnings.
- [ ] `echo hello world` works in the new `mysh`.
- [ ] Can draw the ASCII memory diagram for `int x = 5; int *p = &x;` from memory.

Once all items are checked, move to **Phase 03: Structs & Memory**.
