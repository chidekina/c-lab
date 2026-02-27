# Neovim: Refactoring with clangd

## Rename symbol

Rename a function, variable, or type across **all files** at once:

1. Place cursor on the symbol (e.g., `mysh_readline`)
2. Press `<leader>rn`
3. Type the new name, press Enter

clangd finds every reference across `.c` and `.h` files and renames them atomically.

## Format on save

The `.nvim/init.lua` adds a `BufWritePre` autocmd that runs `clang-format` automatically on every save.

Style is defined in `.clang-format` at the project root (Google base, 4-space indent, 100 columns).

To format manually: `<leader>ca` → "Format document" or run:
```
:lua vim.lsp.buf.format()
```

## Code actions

`<leader>ca` — opens a menu with context-aware fixes:

| Situation | Code action available |
|-----------|----------------------|
| Missing `#include` | "Add #include <...>" |
| Unused variable | "Remove unused variable" |
| Wrong type | "Cast to correct type" |
| Missing return | "Add return statement" |

## Extract to function

clangd does not support extract-to-function yet. Workflow:
1. Select the lines in visual mode
2. Cut (`d`)
3. Write the new function signature above
4. Paste the body
5. Use `<leader>rn` to rename if needed

## Switch header ↔ source

```
:ClangdSwitchSourceHeader
```

Bind it for speed:
```lua
vim.keymap.set('n', '<leader>cs', ':ClangdSwitchSourceHeader<CR>', { desc = 'Switch header/source' })
```
