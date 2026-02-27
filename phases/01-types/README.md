# Phase 01: Types & Undefined Behavior

## Key concepts
- Fixed-width types: `uint8_t`, `int16_t`, `int32_t` (from `<stdint.h>`)
- `sizeof` and memory layout
- Undefined Behavior (UB) — the silent killer
- Detecting UB with `-fsanitize=address,undefined`

## Why this matters
In web dev, JavaScript numbers are all `float64`. In C, you choose the exact size. A `uint8_t` is exactly 1 byte (0–255). Pick the wrong type and you overflow silently — no exception, no error.

## Exercises
- [ ] ex01: print `sizeof` of all basic C types (`char`, `int`, `long`, `float`, `double`)
- [ ] ex02: trigger integer overflow — does the compiler warn you?
- [ ] ex03: trigger and detect UB (uninitialized read) with `-fsanitize=address,undefined`
- [ ] ex04: rewrite ex02 using `uint8_t`/`int32_t` from `<stdint.h>`

## mysh milestone
After this phase: implement the **input loop** — read a line from stdin, print it back (echo builtin).
See `mysh/CHANGELOG.md` for the v1 target.
