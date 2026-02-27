-- c-lab: project-local Neovim config
-- Auto-loaded when exrc = true

-- LSP: clangd
local ok, lspconfig = pcall(require, 'lspconfig')
if ok then
  lspconfig.clangd.setup({
    cmd = { 'clangd', '--background-index', '--clang-tidy', '--header-insertion=never' },
    on_attach = function(_, bufnr)
      local opts = { buffer = bufnr, silent = true }
      vim.keymap.set('n', 'gd', vim.lsp.buf.definition, opts)
      vim.keymap.set('n', 'gr', vim.lsp.buf.references, opts)
      vim.keymap.set('n', 'K', vim.lsp.buf.hover, opts)
      vim.keymap.set('n', '<leader>rn', vim.lsp.buf.rename, opts)
      vim.keymap.set('n', '<leader>ca', vim.lsp.buf.code_action, opts)
    end,
  })
end

-- DAP: gdb
local dap_ok, dap = pcall(require, 'dap')
if dap_ok then
  dap.adapters.gdb = {
    type = 'executable',
    command = 'gdb',
    args = { '--interpreter=dap', '--eval-command', 'set print pretty on' },
  }
  dap.configurations.c = {
    {
      name = 'Debug mysh',
      type = 'gdb',
      request = 'launch',
      program = '${workspaceFolder}/mysh/bin/mysh',
      cwd = '${workspaceFolder}/mysh',
      stopAtBeginningOfMainSubprogram = true,
    },
  }
  dap.configurations.cpp = dap.configurations.c
end

-- Format on save
vim.api.nvim_create_autocmd('BufWritePre', {
  pattern = { '*.c', '*.h', '*.cpp', '*.hpp' },
  callback = function() vim.lsp.buf.format({ async = false }) end,
})

-- Load keymaps
local km_ok = pcall(dofile, vim.fn.getcwd() .. '/.nvim/keymaps.lua')
if not km_ok then
  vim.notify('[c-lab] keymaps.lua not loaded', vim.log.levels.WARN)
end
