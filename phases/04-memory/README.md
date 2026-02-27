# Phase 04: Memory Management

## Key concepts
- Stack: automatic, fast, limited size, freed when function returns
- Heap: manual (`malloc`/`free`), unlimited, you own the lifecycle
- Memory leaks: allocated but never freed → valgrind catches these
- Use-after-free: accessing freed memory → UB, crash, security bug
- RAII preview: allocate in constructor, free in destructor (full coverage in phase 05)

## Why this matters
In JavaScript, the garbage collector handles memory. In C, you are the garbage collector. `malloc` without `free` = memory leak. The ESP32 has 520KB of RAM. A leak that takes 1KB per request will crash your device after 520 requests.

## Tools
```bash
valgrind --leak-check=full --track-origins=yes ./program
gcc -fsanitize=address ./program.c && ./a.out
```

## Exercises
- [ ] ex01: `malloc` + `free` — allocate an array on the heap, use it, free it
- [ ] ex02: deliberate memory leak — run through valgrind, read the report
- [ ] ex03: use-after-free — trigger it, detect with AddressSanitizer
- [ ] ex04: implement a dynamic array (similar to `std::vector`) — `push`, `get`, `free`

## mysh milestone
After this phase: implement `fork()` + `exec()` to run external commands. Use valgrind to verify no leaks.
See `mysh/CHANGELOG.md` for the v2 target.
