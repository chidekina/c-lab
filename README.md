# c-lab

Learning C/C++ by building a mini shell (`mysh`) from scratch.

## Structure

| Directory | Purpose |
|-----------|---------|
| `phases/` | Structured curriculum — one folder per concept |
| `mysh/` | Anchor project — grows with each phase |
| `docs/` | Design docs and Neovim workflow guides |
| `.nvim/` | Project-local Neovim config (LSP + DAP) |

## Phases

| Phase | Concept | mysh milestone |
|-------|---------|----------------|
| 00 | Tooling (gcc, make, gdb) | Project setup |
| 01 | Types, UB, sanitizers | Input loop + echo |
| 02 | Pointers & strings | Command parser |
| 03 | Structs & enums | `struct Command`, tokenizer |
| 04 | Memory (heap/stack, valgrind) | fork + exec + pipes |
| 05 | C++ (classes, RAII, templates) | OO refactor + job control |

## Prerequisites

```bash
sudo apt install gcc g++ make bear clangd gdb valgrind clang-format
```

## Quick start

```bash
cd phases/00-tooling
make run
```
