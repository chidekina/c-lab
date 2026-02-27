# Phase 02: Pointers & Strings

## Key concepts
- A pointer is just a variable holding a memory address
- `int *p = &x` — p holds the address where x lives
- Dereferencing: `*p` reads the value at that address
- `const` correctness — promise not to modify through a pointer
- Strings in C are `char*` — a pointer to the first character, terminated by `\0`

## Why this matters
In JavaScript, `function foo(obj)` copies a reference automatically. In C, you choose: pass by value (copy) or pass by pointer (address). On embedded systems, copying a 1KB struct wastes RAM — you pass a pointer instead.

## Exercises
- [ ] ex01: basic pointer — declare, assign, dereference, print address
- [ ] ex02: pointer arithmetic — navigate an array using only a pointer
- [ ] ex03: implement `my_strlen` without using `string.h`
- [ ] ex04: implement `my_strcpy` — copy a string character by character
- [ ] ex05: `const char*` vs `char*` — when does the compiler complain?

## mysh milestone
After this phase: implement `mysh_tokenize` — split a string into tokens using pointer arithmetic (no `strtok`).
See `mysh/CHANGELOG.md` for the v1 target.
