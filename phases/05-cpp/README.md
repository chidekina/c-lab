# Phase 05: C++ — Classes, RAII & Modern Idioms

## Key concepts
- Classes: data + functions bundled together
- Constructor/Destructor: called automatically at creation and destruction
- RAII (Resource Acquisition Is Initialization): acquire in constructor, release in destructor — memory safety without manual `free`
- `std::unique_ptr`: smart pointer that calls `delete` automatically
- Templates: write code once, use with any type

## Why this matters
RAII is the core idea behind safe C++. It's what makes C++ different from C for memory management. Instead of remembering to `free`, you structure code so cleanup is automatic.

## Exercises
- [ ] ex01: class `Stack<T>` with `push`, `pop`, `size` using a fixed array
- [ ] ex02: RAII — class `FileHandle` that opens a file in constructor, closes in destructor
- [ ] ex03: `std::unique_ptr` — replace manual `new`/`delete` with smart pointers
- [ ] ex04: template function `max<T>` — works with `int`, `float`, `char`
- [ ] ex05: `std::array` and range-based for loop

## mysh milestone
After this phase: refactor `mysh` to C++ — wrap the shell loop in a `class Shell`, use RAII for file descriptors in pipe/redirect code.
See `mysh/CHANGELOG.md` for the v3 target.
