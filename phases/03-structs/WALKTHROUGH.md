# Phase 03 Walkthrough — Structs, Enums & Typedef

> Audience: web developer with JavaScript/TypeScript background learning C.

---

## 1. The TypeScript analogy — and where it breaks

In TypeScript you write:

```ts
interface Point {
  x: number;
  y: number;
}

const p: Point = { x: 3, y: 7 };
```

At runtime, `p` is a JavaScript object: a heap-allocated hash map. Its memory
footprint is opaque to you. You never think about it.

In C, a `struct` is the same idea but with a completely different contract:

- The compiler decides the **exact byte layout** at compile time.
- `sizeof(struct Point)` is knowable — and fixed — before the program runs.
- Passing a struct to a function **copies every byte** unless you use a pointer.
- There is no garbage collector. If the struct lives on the stack, it disappears
  when the function returns.

This phase is about understanding that contract.

---

## 2. Exercise 01 — Define `struct Point` and print its size

### What you will build

A struct with two `int` fields and a `main` that prints how many bytes it
occupies.

### Create the file

```
phases/03-structs/ex01.c
```

### Full code

```c
#include <stdio.h>

struct Point {
    int x;
    int y;
};

int main(void) {
    struct Point p;
    p.x = 3;
    p.y = 7;

    printf("p.x = %d, p.y = %d\n", p.x, p.y);
    printf("sizeof(struct Point) = %zu bytes\n", sizeof(struct Point));

    return 0;
}
```

### Compile and run

```
gcc -Wall -o ex01 ex01.c && ./ex01
```

### Expected output

```
p.x = 3, p.y = 7
sizeof(struct Point) = 8 bytes
```

### Explanation

- `int` is 4 bytes on every common 32-bit and 64-bit platform.
- Two `int` fields = 4 + 4 = **8 bytes**. No surprises here.
- `%zu` is the correct format specifier for `size_t`, the type returned by
  `sizeof`.
- Access fields with the **dot operator**: `p.x`, `p.y`.

---

## 3. Exercise 02 — Struct padding

### What you will build

Add a single `char` field to `Point` and observe that `sizeof` does not grow
by 1.

### Create the file

```
phases/03-structs/ex02.c
```

### Full code

```c
#include <stdio.h>

struct Point2D {
    int  x;
    int  y;
};

struct Point3C {
    char label;   /* 1 byte — but watch what happens */
    int  x;
    int  y;
};

int main(void) {
    printf("sizeof(Point2D) = %zu\n", sizeof(struct Point2D));
    printf("sizeof(Point3C) = %zu\n", sizeof(struct Point3C));
    return 0;
}
```

### Expected output

```
sizeof(Point2D) = 8
sizeof(Point3C) = 12
```

### Explanation — padding

You added 1 byte (`char`) but the struct grew by 4 bytes. The compiler inserted
**3 bytes of padding** after `label` so that `x` starts on a 4-byte boundary.

CPUs read integers most efficiently when they are aligned to their own size.
A 4-byte `int` must start at an address divisible by 4. If `label` sits at
offset 0 (address `...000`), then offset 1 is not divisible by 4 — so the
compiler skips to offset 4.

### ASCII memory diagram

```
struct Point3C in memory (12 bytes total):

Offset:  0    1    2    3    4    5    6    7    8    9   10   11
        +----+----+----+----+----+----+----+----+----+----+----+----+
        |lbl | PAD PAD PAD |     x (4 bytes)    |     y (4 bytes)    |
        +----+----+----+----+----+----+----+----+----+----+----+----+
         ^              ^   ^
         label          |   x starts here (offset 4, divisible by 4)
                        padding (3 bytes, never used)
```

**Field order matters.** Reordering fields can eliminate padding:

```c
struct Point3C_packed {
    int  x;
    int  y;
    char label;
    /* 3 bytes of trailing padding added by compiler to keep array alignment */
};
/* sizeof = 12 still — but now padding is at the end, not the middle */
```

```c
struct Dense {
    int  x;       /* offset 0 */
    int  y;       /* offset 4 */
    char a;       /* offset 8 */
    char b;       /* offset 9 */
    char c;       /* offset 10 */
    char d;       /* offset 11 */
};
/* sizeof = 12 — four chars fill the gap that would have been wasted */
```

Rule of thumb: **declare larger types first**, smaller types last.

---

## 4. Exercise 03 — Pass by value vs pass by pointer

### What you will build

Two functions that accept a `struct Point`: one by value, one by pointer.
You will see that the by-value version cannot modify the original.

### Create the file

```
phases/03-structs/ex03.c
```

### Full code

```c
#include <stdio.h>

struct Point {
    int x;
    int y;
};

/* Receives a COPY — changes do not affect the caller */
void move_by_value(struct Point p, int dx, int dy) {
    p.x += dx;
    p.y += dy;
    printf("  inside move_by_value: (%d, %d)\n", p.x, p.y);
}

/* Receives a POINTER — changes affect the original */
void move_by_pointer(struct Point *p, int dx, int dy) {
    p->x += dx;
    p->y += dy;
    printf("  inside move_by_pointer: (%d, %d)\n", p->x, p->y);
}

int main(void) {
    struct Point a = {1, 2};

    printf("Before move_by_value:   (%d, %d)\n", a.x, a.y);
    move_by_value(a, 10, 20);
    printf("After  move_by_value:   (%d, %d)\n\n", a.x, a.y);

    printf("Before move_by_pointer: (%d, %d)\n", a.x, a.y);
    move_by_pointer(&a, 10, 20);
    printf("After  move_by_pointer: (%d, %d)\n", a.x, a.y);

    return 0;
}
```

### Expected output

```
Before move_by_value:   (1, 2)
  inside move_by_value: (11, 22)
After  move_by_value:   (1, 2)

Before move_by_pointer: (1, 2)
  inside move_by_pointer: (11, 22)
After  move_by_pointer: (11, 22)
```

### Explanation

**By value:** `move_by_value(a, ...)` pushes a full copy of `a` onto the call
stack. The function works on that copy. When it returns, the copy is discarded.
`a` in `main` is unchanged.

**By pointer:** `move_by_pointer(&a, ...)` pushes only the address of `a`
(8 bytes on a 64-bit machine). The function dereferences the pointer with `->`.
Changes go directly to the original struct.

### The `->` operator

When you have a pointer to a struct, `->` is shorthand for dereference + dot:

```c
p->x          /* same as */    (*p).x
```

Always use `->` with pointers, `.` with values. Mixing them up is the most
common beginner mistake (see Section 8).

### Performance note

For a 2-field struct the copy is trivial. For a struct with 256 fields the copy
is expensive. Prefer passing large structs by pointer (`const struct Foo *`)
when you do not need to mutate them.

---

## 5. Exercise 04 — `enum Color`

### What you will build

An enum that maps color names to integers and prints those integers.

### Create the file

```
phases/03-structs/ex04.c
```

### Full code

```c
#include <stdio.h>

enum Color {
    RED,    /* 0 */
    GREEN,  /* 1 */
    BLUE    /* 2 */
};

const char *color_name(enum Color c) {
    switch (c) {
        case RED:   return "RED";
        case GREEN: return "GREEN";
        case BLUE:  return "BLUE";
        default:    return "UNKNOWN";
    }
}

int main(void) {
    enum Color c = GREEN;

    printf("RED   = %d\n", RED);
    printf("GREEN = %d\n", GREEN);
    printf("BLUE  = %d\n", BLUE);
    printf("c = %d (%s)\n", c, color_name(c));

    /* Explicit values */
    enum StatusCode {
        OK      = 200,
        NOT_FOUND = 404,
        ERROR   = 500
    };

    printf("\nOK        = %d\n", OK);
    printf("NOT_FOUND = %d\n", NOT_FOUND);
    printf("ERROR     = %d\n", ERROR);

    return 0;
}
```

### Expected output

```
RED   = 0
GREEN = 1
BLUE  = 2
c = 1 (GREEN)

OK        = 200
NOT_FOUND = 404
ERROR     = 500
```

### Explanation

- Without explicit values, enum constants start at 0 and increment by 1.
- You can assign any integer value explicitly (e.g. HTTP status codes above).
- An `enum` is stored as an `int`. The compiler does **not** prevent assigning
  an out-of-range integer to an enum variable — there is no runtime check.
- TypeScript `enum` looks similar but is compiled away to a plain object;
  C enums are purely a compile-time naming convention.

---

## 6. Exercise 05 — `typedef struct`

### What you will build

Refactor `struct Point` with `typedef` so you can write `Point` instead of
`struct Point` everywhere.

### Create the file

```
phases/03-structs/ex05.c
```

### Full code

```c
#include <stdio.h>

/* Without typedef you must write "struct Point" every time */
struct Point_verbose {
    int x;
    int y;
};

/* With typedef you create a type alias */
typedef struct {
    int x;
    int y;
} Point;

/* You can also give the struct a tag AND a typedef */
typedef struct Vector2 {
    float x;
    float y;
} Vector2;   /* tag and alias share the same name — common C convention */

void print_point(const Point *p) {
    printf("Point(%d, %d)\n", p->x, p->y);
}

void print_vector(const Vector2 *v) {
    printf("Vector2(%.2f, %.2f)\n", v->x, v->y);
}

int main(void) {
    struct Point_verbose pv = {1, 2};   /* verbose form */
    Point                pt = {3, 4};   /* clean form */
    Vector2              v  = {1.5f, 2.5f};

    printf("verbose: (%d, %d)\n", pv.x, pv.y);
    print_point(&pt);
    print_vector(&v);

    return 0;
}
```

### Expected output

```
verbose: (1, 2)
Point(3, 4)
Vector2(1.50, 2.50)
```

### Explanation

`typedef` creates an **alias**. It does not create a new type; it just gives
the existing type a shorter name.

```c
typedef struct { int x; int y; } Point;
/*      ^^^^^^                   ^^^^^
        actual type              alias */
```

After this line, `Point` and `struct { int x; int y; }` are interchangeable.

**Why include the tag?**

```c
typedef struct Node {
    int        value;
    struct Node *next;   /* you need the tag here to self-reference */
} Node;
```

Self-referential structs (linked lists, trees) need the tag because the typedef
alias is not visible until the end of the declaration.

---

## 7. ASCII memory diagrams — padding summary

```
Rule: each field starts at an offset that is a multiple of its own size.
The struct's total size is a multiple of its largest field's alignment.

struct A { char a; int b; char c; }
==> sizeof = 12

  0     1     2     3     4     5     6     7     8     9    10    11
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
|  a  | pad   pad   pad |        b (4B)         |  c  | pad   pad   pad |
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+

struct B { int b; char a; char c; }
==> sizeof = 8   (2 bytes of trailing pad to align array elements)

  0     1     2     3     4     5     6     7
+-----+-----+-----+-----+-----+-----+-----+-----+
|        b (4B)         |  a  |  c  | pad   pad |
+-----+-----+-----+-----+-----+-----+-----+-----+

Lesson: same fields, different order, different size.
Always put larger fields first.
```

---

## 8. Common mistakes

### Mistake 1: Using `.` on a pointer

```c
struct Point *p = &some_point;
p.x = 5;   /* COMPILE ERROR: p is a pointer, not a struct */
p->x = 5;  /* correct */
```

The compiler will catch this, but the error message can be confusing the first
time: *"request for member 'x' in something not a structure or union"*.

### Mistake 2: Using `->` on a value

```c
struct Point p = {1, 2};
p->x = 5;   /* COMPILE ERROR: p is not a pointer */
p.x  = 5;   /* correct */
```

Mnemonic: **arrow for address, dot for direct**.

### Mistake 3: Accidentally copying a large struct

```c
typedef struct { char data[4096]; } BigBuffer;

void process(BigBuffer buf) {   /* copies 4096 bytes onto the stack! */
    /* ... */
}

/* Better: */
void process(const BigBuffer *buf) {   /* copies 8 bytes (pointer) */
    /* ... */
}
```

### Mistake 4: Forgetting that assignment copies a struct

```c
struct Point a = {1, 2};
struct Point b = a;   /* b is a full independent copy */
b.x = 99;
printf("%d\n", a.x);  /* still 1 — a was not changed */
```

This is different from JavaScript where `const b = a` would share the reference
(for objects). In C, struct assignment is always a copy.

### Mistake 5: Comparing structs with `==`

```c
struct Point a = {1, 2};
struct Point b = {1, 2};

if (a == b) { ... }   /* COMPILE ERROR: structs cannot be compared with == */
```

You must compare field by field, or use `memcmp` (which can give wrong results
if there is padding with undefined bytes).

---

## 9. mysh milestone — introduce `struct Command`

After completing the exercises, apply what you learned to `mysh`.

### Goal

Replace the raw `argc`/`argv` locals in `main.c` with a `struct Command` that
bundles all parsed input into one named value.

### Step 1 — Add `struct Command` to `mysh/include/mysh.h`

Open `mysh/include/mysh.h`. Add the struct definition and update the
`mysh_tokenize` signature:

```c
#ifndef MYSH_H
#define MYSH_H

#define MYSH_VERSION "0.1.0"
#define MAX_INPUT    1024
#define MAX_ARGS     64

/* Represents one parsed command line. */
typedef struct {
    char  *name;       /* points into buf — argv[0] */
    char **args;       /* NULL-terminated array of token pointers */
    int    argc;       /* number of tokens */
} Command;

/* Read a line from stdin into buf (max MAX_INPUT bytes).
 * Returns number of bytes read, or -1 on EOF. */
int mysh_readline(char *buf, int size);

/* Split buf into tokens; populate cmd.
 * cmd->args must point to a char* array with room for MAX_ARGS pointers.
 * Returns argc (same as cmd->argc). */
int mysh_tokenize(char *buf, Command *cmd);

/* Execute a builtin command. Returns 1 if handled, 0 if not a builtin. */
int mysh_builtin(Command *cmd);

#endif /* MYSH_H */
```

### Step 2 — Update `mysh/src/tokenize.c`

```c
#include <string.h>
#include "mysh.h"

int mysh_tokenize(char *buf, Command *cmd) {
    cmd->argc = 0;
    cmd->name = NULL;

    char *token = strtok(buf, " \t");
    while (token != NULL && cmd->argc < MAX_ARGS - 1) {
        cmd->args[cmd->argc++] = token;
        token = strtok(NULL, " \t");
    }
    cmd->args[cmd->argc] = NULL;

    if (cmd->argc > 0)
        cmd->name = cmd->args[0];

    return cmd->argc;
}
```

### Step 3 — Update `mysh/src/main.c`

```c
#include <stdio.h>
#include <stdlib.h>
#include "mysh.h"

int main(void) {
    char    buf[MAX_INPUT];
    char   *args[MAX_ARGS];
    Command cmd;

    cmd.args = args;   /* cmd borrows the stack-allocated array */

    printf("mysh v%s — type 'exit' to quit\n", MYSH_VERSION);

    while (1) {
        printf("mysh> ");
        fflush(stdout);

        if (mysh_readline(buf, MAX_INPUT) < 0)
            break;  /* EOF (Ctrl+D) */

        if (mysh_tokenize(buf, &cmd) == 0)
            continue;

        if (mysh_builtin(&cmd))
            continue;

        /* Phase 04: fork + exec goes here */
        fprintf(stderr, "mysh: %s: command not found\n", cmd.name);
    }

    return 0;
}
```

### Step 4 — Update `mysh/src/builtins.c`

Your `mysh_builtin` currently takes `(int argc, char **argv)`. Update its
signature to `(Command *cmd)` and access `cmd->argc`, `cmd->args[0]`, etc.
The internal logic stays the same — only the parameter names change.

### Why this is an improvement

Before:

```c
int   argc;
char *argv[MAX_ARGS];
/* caller must pass both separately and keep them in sync */
mysh_builtin(argc, argv);
```

After:

```c
Command cmd;
/* everything about the parsed input travels as one unit */
mysh_builtin(&cmd);
```

This is the core benefit of structs: **cohesion**. Data that belongs together
travels together. Functions that need that data accept one pointer instead of
three separate arguments.

---

## 10. Checklist

- [ ] ex01 compiles and prints `sizeof(struct Point) = 8`
- [ ] ex02 shows that adding `char label` makes the struct 12 bytes, not 9
- [ ] ex03 shows that by-value does not change the original, by-pointer does
- [ ] ex04 prints RED=0, GREEN=1, BLUE=2 and demonstrates explicit int values
- [ ] ex05 uses `typedef struct` and compiles without warnings
- [ ] `mysh.h` declares `typedef struct { ... } Command`
- [ ] `mysh_tokenize` signature updated to `(char *buf, Command *cmd)`
- [ ] `mysh_builtin` signature updated to `(Command *cmd)`
- [ ] `main.c` uses `Command cmd` and passes `&cmd` to tokenize/builtin
- [ ] `mysh` compiles without warnings after refactor (`gcc -Wall`)
