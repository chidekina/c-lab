# Neovim: Code Navigation

## LSP keymaps (set in `.nvim/init.lua`)

| Keymap | Action | When to use |
|--------|--------|-------------|
| `gd` | Go to definition | Jump to where a function/variable is defined |
| `gr` | Show all references | See everywhere a symbol is used |
| `K` | Hover documentation | Read the comment/signature without leaving cursor |
| `gi` | Go to implementation | For C++ virtual methods |
| `<leader>rn` | Rename symbol | Rename across all files at once |
| `<leader>ca` | Code action | Quick fixes (add missing include, etc.) |
| `<C-o>` | Jump back | Return to where you were before `gd` |
| `<C-i>` | Jump forward | Undo a `<C-o>` |

## Phase navigation (set in `.nvim/keymaps.lua`)

| Keymap | Opens |
|--------|-------|
| `<leader>p0` | `phases/00-tooling/README.md` |
| `<leader>p1` | `phases/01-types/README.md` |
| `<leader>p2` | `phases/02-pointers/README.md` |
| `<leader>p3` | `phases/03-structs/README.md` |
| `<leader>p4` | `phases/04-memory/README.md` |
| `<leader>p5` | `phases/05-cpp/README.md` |

## Telescope (if installed)

| Keymap | Action |
|--------|--------|
| `<leader>fs` | Find symbol in workspace |
| `<leader>ff` | Find file by name |
| `<leader>fg` | Live grep across all files |

## Workflow tip: header ↔ source

In C, you constantly switch between `.h` and `.c`. With clangd:
- Place cursor anywhere in a file
- Run `:ClangdSwitchSourceHeader` (or bind it)
- Jumps between `mysh.h` and the corresponding `.c` file
