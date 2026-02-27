# Neovim: Debugging with gdb + DAP

## Prerequisites

Install `nvim-dap` (and optionally `nvim-dap-ui` for a visual panel):

```lua
-- lazy.nvim
{ "mfussenegger/nvim-dap" },
{ "rcarriga/nvim-dap-ui", dependencies = { "mfussenegger/nvim-dap" } },
```

The `.nvim/init.lua` configures gdb as the DAP adapter automatically.

## Debug workflow

### 1. Build with debug symbols

```
<leader>cb
```

This runs `make` which uses `-g` flag — includes debug symbols in the binary.

### 2. Set a breakpoint

Navigate to the line you want to pause execution at, then:
```
<leader>db
```

A sign appears in the gutter (●) marking the breakpoint.

### 3. Start the debug session

```
<F5>
```

gdb launches and pauses at your breakpoint (or at `main` if configured).

### 4. Navigate execution

| Key | Action |
|-----|--------|
| `<F10>` | Step over (execute current line, move to next) |
| `<F11>` | Step into (enter the function being called) |
| `<F12>` | Step out (finish current function, return to caller) |
| `<F5>` | Continue (run until next breakpoint) |

### 5. Inspect variables

In the DAP REPL (`:DapReplOpen`):
```
p buf          -- print value of 'buf'
p argv[0]      -- print first argument
p *p           -- dereference pointer p
x/4xb buf      -- print 4 bytes at address buf in hex
```

## Terminal gdb (no plugins needed)

If you prefer the terminal:

```bash
cd mysh
make
gdb ./bin/mysh
```

```
(gdb) break mysh_readline      # breakpoint at function entry
(gdb) run                      # start program
(gdb) next                     # step over
(gdb) step                     # step into
(gdb) print buf                # inspect variable
(gdb) print argv[0]            # inspect array element
(gdb) backtrace                # show call stack
(gdb) info locals              # show all local variables
(gdb) quit
```
