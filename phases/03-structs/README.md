# Phase 03: Structs, Enums & Typedef

## Key concepts
- `struct` groups related data (like a TypeScript interface, but in memory)
- `enum` gives names to integer constants
- `typedef` creates type aliases
- Memory layout: structs may have padding between fields

## Why this matters
In JavaScript you use `{}` objects freely. In C, every struct has a fixed size decided at compile time. Understanding layout lets you write efficient code — and understand why `sizeof(struct Foo)` might surprise you.

## Exercises
- [ ] ex01: define a `struct Point { int x; int y; }` and print its `sizeof`
- [ ] ex02: struct padding — add a `char` field, see how `sizeof` changes
- [ ] ex03: pass a struct by value vs by pointer — time the difference with a large struct
- [ ] ex04: `enum Color { RED, GREEN, BLUE }` — print the underlying int values
- [ ] ex05: `typedef struct` — clean up verbose struct declarations

## mysh milestone
After this phase: define `struct Command { char *name; char **args; int argc; }` and use it in the tokenizer.
See `mysh/CHANGELOG.md` for the v2 target.
