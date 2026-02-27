# c-lab Design Document
**Date:** 2026-02-27
**Format:** Hybrid (structured curriculum + real anchor project)
**Anchor project:** `mysh` — a mini shell in C/C++
**Neovim:** LSP + DAP config versioned + workflow guides per phase

---

## 1. Directory Structure

```
c-lab/
├── .nvim/
│   ├── init.lua              # LSP (clangd) + DAP (gdb) local config
│   └── keymaps.lua           # C/C++ specific keybindings
│
├── phases/
│   ├── 00-tooling/           # gcc flags, Makefile, gdb basics
│   ├── 01-types/             # types, sizeof, UB, sanitizers
│   ├── 02-pointers/          # pointers, strings, const correctness
│   ├── 03-structs/           # structs, enums, typedef, union
│   ├── 04-memory/            # heap vs stack, malloc/free, valgrind, RAII
│   └── 05-cpp/               # classes, smart pointers, templates, STL
│
├── mysh/
│   ├── Makefile
│   ├── README.md
│   ├── include/
│   ├── src/
│   └── CHANGELOG.md
│
├── docs/
│   └── nvim-workflows/
│       ├── 00-setup.md
│       ├── 01-navigation.md
│       ├── 02-debug.md
│       └── 03-refactor.md
│
├── .clang-format
├── .clangd
└── README.md
```

---

## 2. mysh Architecture & Phase Mapping

### Version 1 — Builtins (phases 01-02)
- Input loop: read → parse → execute
- Builtins: `echo`, `pwd`, `cd`, `exit`
- String parsing with `char*` (no stdlib)
- Manual `argv/argc`

### Version 2 — Pipes & Redirections (phases 03-04)
- `fork()` + `exec()` for external commands
- Pipes: `ls | grep foo`
- Redirections: `cmd > file`, `cmd < file`
- `struct Command { char* name; char** args; int fd_in; int fd_out; }`

### Version 3 — Job Control (phase 05)
- Background jobs: `cmd &`
- `fg`, `bg`, `jobs`
- Signal handling: `SIGCHLD`, `SIGINT`, `SIGTSTP`
- OO refactor: `class Shell`, `class Job` with RAII

### Phase → Concept → Shell mapping

| Phase | Concept | What enters mysh |
|-------|---------|-----------------|
| 00 | Tooling, Makefile, gdb | Project setup, first `make run` |
| 01 | Types, UB, sanitizers | Input loop + echo builtin |
| 02 | Pointers, strings | Command parser with `char*` |
| 03 | Structs, enums | `struct Command`, tokenizer |
| 04 | Heap/stack, valgrind | `fork()`, `exec()`, pipes |
| 05 | C++, RAII, classes | OO refactor + job control |

---

## 3. Neovim Configuration

### `.nvim/init.lua`
```lua
require('lspconfig').clangd.setup({
  cmd = { 'clangd', '--background-index', '--clang-tidy' },
})

local dap = require('dap')
dap.adapters.gdb = {
  type = 'executable',
  command = 'gdb',
  args = { '--interpreter=dap', '--eval-command', 'set print pretty on' }
}

vim.keymap.set('n', '<leader>cb', ':!make -C mysh<CR>')
vim.keymap.set('n', '<leader>cr', ':!make -C mysh run<CR>')
vim.keymap.set('n', '<leader>ct', ':!make -C mysh test<CR>')
vim.keymap.set('n', '<leader>cd', function() require('dap').continue() end)
```

### `.clangd`
```yaml
CompileFlags:
  Add: [-Wall, -Wextra, -std=c17]
  CompilationDatabase: .
Diagnostics:
  UnusedIncludes: Strict
```

### `.clang-format`
```yaml
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
```

### Workflow guides
| File | Content |
|------|---------|
| `00-setup.md` | Install clangd, bear, enable exrc, generate `compile_commands.json` |
| `01-navigation.md` | `gd` go-to-def, `gr` references, `K` hover, Telescope by symbol |
| `02-debug.md` | Breakpoints, variable inspection, call stack with gdb |
| `03-refactor.md` | Rename symbol, format on save with clang-format |
