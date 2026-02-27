# Neovim Setup for C/C++

## 1. Install tools

```bash
sudo apt install clangd bear gdb
```

## 2. Enable exrc in Neovim

Add to `~/.config/nvim/init.lua`:
```lua
vim.opt.exrc = true
vim.opt.secure = true
```

This makes Neovim auto-load `.nvim/init.lua` when you open the project directory.

## 3. Generate compile_commands.json

`clangd` needs this file to understand your project's include paths and compiler flags.
Generate it with `bear` (Build EAR — intercepts compiler calls):

```bash
cd /path/to/c-lab/mysh
bear -- make clean all
# Creates compile_commands.json in mysh/
```

Move it to the project root so clangd finds it from any subdirectory:
```bash
mv compile_commands.json ../
```

Regenerate whenever you add new source files.

## 4. Open the project in Neovim

```bash
cd /path/to/c-lab
nvim .
```

Neovim auto-loads `.nvim/init.lua`. Watch the status bar — clangd starts indexing within a few seconds.

## 5. Verify LSP is working

1. Open `mysh/src/main.c`
2. Hover over `mysh_readline` and press `K`
3. You should see the function signature and comment from `mysh.h`

If nothing appears, run `:LspInfo` to check clangd status.

## 6. Troubleshooting

| Problem | Fix |
|---------|-----|
| LSP not starting | Check `:LspInfo` — clangd binary found? |
| No completions | `compile_commands.json` missing or stale — regenerate |
| exrc not loading | Add `vim.opt.exrc = true` to your global `init.lua` |
| DAP not working | Install `nvim-dap` via your plugin manager (lazy.nvim: see `02-debug.md`) |
