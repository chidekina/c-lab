# Phase 00: Walkthrough

Guia passo a passo para completar os exercícios desta fase.

---

## Pré-requisitos

Verifique se as ferramentas estão instaladas:

```bash
which gcc g++ make clangd gdb valgrind
gcc --version
clangd --version
```

Se alguma estiver faltando:

```bash
sudo apt install gcc g++ make bear clangd gdb valgrind clang-format
```

---

## Exercise 1 — ex01-hello.c

### Passo 1: Compile sem flags

```bash
cd ~/projetos/c-lab/phases/00-tooling
gcc -o ex01 exercises/ex01-hello.c
./ex01
```

Funciona. Mas o compilador não te avisou de nada — mesmo que você tenha cometido erros.

### Passo 2: Compile com flags de produção

```bash
gcc -Wall -Wextra -Werror -g -std=c17 -o ex01 exercises/ex01-hello.c
```

| Flag | O que faz |
|------|-----------|
| `-Wall` | Ativa todos os warnings importantes |
| `-Wextra` | Ativa warnings extras que `-Wall` não pega |
| `-Werror` | Transforma warnings em erros (build falha) |
| `-g` | Inclui símbolos de debug (necessário para gdb) |
| `-std=c17` | Usa o padrão C17 |

### Passo 3: Desafio — variável não inicializada

Edite `ex01-hello.c` e adicione:

```c
int x;
printf("x = %d\n", x);
```

Compile novamente com `-Wall`. O que acontece?

Com `-Werror`, o build vai **falhar** — esse é o comportamento correto. Em C, ler uma variável não inicializada é **Undefined Behavior**: o valor pode ser qualquer coisa, incluindo dados sensíveis da memória.

Corrija inicializando: `int x = 42;`

### Passo 4: Use o Makefile

Em vez de digitar `gcc ...` toda vez:

```bash
make        # compila
make run    # compila + roda
make clean  # remove binários
```

Abra o `Makefile` e entenda cada símbolo:

| Símbolo | Significado |
|---------|-------------|
| `$(CC)` | Variável expandida para `gcc` |
| `$(CFLAGS)` | Variável expandida para as flags |
| `$@` | O nome do alvo (target) sendo construído |
| `$^` | Todos os arquivos de dependência |

---

## Exercise 2 — ex02-flags.c (Sanitizers)

### Passo 1: Leia o arquivo

```bash
cat exercises/ex02-flags.c
```

O arquivo tem dois problemas **intencionais**:
1. Variável `result` declarada sem inicializar (mas atribuída antes do uso — ok aqui)
2. Acesso `arr[5]` em um array de 5 elementos (`arr[0]` a `arr[4]`) — **out-of-bounds**

### Passo 2: Compile sem sanitizer

```bash
gcc -Wall -Wextra exercises/ex02-flags.c -o ex02
./ex02
```

Pode rodar "normalmente" — mas está fazendo acesso inválido de memória. Isso é o perigo do C: bugs silenciosos.

### Passo 3: Compile com sanitizer

```bash
make san
```

Ou manualmente:

```bash
gcc -Wall -Wextra -fsanitize=address,undefined -g exercises/ex02-flags.c -o ex02-san
./ex02-san
```

### Passo 4: Leia o output do sanitizer

O AddressSanitizer vai imprimir algo parecido com:

```
==12345==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x...
READ of size 4 at 0x... thread T0
    #0 0x... in main exercises/ex02-flags.c:19
```

**O que isso significa:**
- `stack-buffer-overflow`: acessou memória além do limite do array
- `READ of size 4`: tentou ler 4 bytes (tamanho de um `int`)
- `ex02-flags.c:19`: a linha exata do problema

### Passo 5: Corrija o bug

Troque `arr[5]` por `arr[4]` (último elemento válido) e rode novamente — o sanitizer não deve reportar nada.

---

## Exercise 3 — gdb básico

### Passo 1: Compile com símbolos de debug

```bash
make  # já usa -g nas flags
```

### Passo 2: Inicie o debugger

```bash
gdb ./ex01
```

### Passo 3: Comandos essenciais

```
(gdb) break main      # pausa quando entrar em main()
(gdb) run             # inicia o programa
(gdb) next            # executa a linha atual, vai para a próxima
(gdb) step            # entra dentro da função chamada
(gdb) print x         # imprime o valor da variável x
(gdb) backtrace       # mostra a pilha de chamadas
(gdb) info locals     # mostra todas as variáveis locais
(gdb) quit            # sai do gdb
```

### Passo 4: No Neovim (se nvim-dap estiver configurado)

1. Abra `exercises/ex01-hello.c` no Neovim
2. Posicione o cursor na linha do `printf`
3. `<leader>db` — define um breakpoint (aparece um `●` na margem)
4. `<F5>` — inicia a sessão de debug
5. `<F10>` — step over | `<F11>` — step into | `<F12>` — step out

---

## Checklist de conclusão

Antes de ir para a Phase 01, confirme:

- [ ] Compilei `ex01-hello.c` sem e com `-Wall -Wextra -Werror`
- [ ] Entendo por que variável não inicializada causa erro com `-Werror`
- [ ] Rodei `ex02-flags.c` com e sem `-fsanitize=address,undefined`
- [ ] Li e entendi o output do AddressSanitizer
- [ ] Sei o que `$@` e `$^` significam no Makefile
- [ ] Rodei pelo menos `break main` + `run` + `next` no gdb

**Quando todos os itens estiverem marcados, vá para `phases/01-types/README.md`.**
