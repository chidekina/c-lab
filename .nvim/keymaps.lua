-- c-lab: build/run/debug keymaps
local map = function(mode, lhs, rhs, desc)
  vim.keymap.set(mode, lhs, rhs, { silent = true, desc = desc })
end

-- Build & run
map('n', '<leader>cb', ':!make -C mysh<CR>',            'C: build mysh')
map('n', '<leader>cr', ':!make -C mysh run<CR>',        'C: run mysh')
map('n', '<leader>ct', ':!make -C mysh test<CR>',       'C: test mysh')
map('n', '<leader>cv', ':!make -C mysh valgrind<CR>',   'C: valgrind mysh')

-- DAP debug
local dap_ok, dap = pcall(require, 'dap')
if dap_ok then
  map('n', '<F5>',  dap.continue,          'Debug: continue')
  map('n', '<F10>', dap.step_over,         'Debug: step over')
  map('n', '<F11>', dap.step_into,         'Debug: step into')
  map('n', '<F12>', dap.step_out,          'Debug: step out')
  map('n', '<leader>db', dap.toggle_breakpoint, 'Debug: toggle breakpoint')
end

-- Phase navigation
map('n', '<leader>p0', ':e phases/00-tooling/README.md<CR>',  'Phase 00')
map('n', '<leader>p1', ':e phases/01-types/README.md<CR>',    'Phase 01')
map('n', '<leader>p2', ':e phases/02-pointers/README.md<CR>', 'Phase 02')
map('n', '<leader>p3', ':e phases/03-structs/README.md<CR>',  'Phase 03')
map('n', '<leader>p4', ':e phases/04-memory/README.md<CR>',   'Phase 04')
map('n', '<leader>p5', ':e phases/05-cpp/README.md<CR>',      'Phase 05')
