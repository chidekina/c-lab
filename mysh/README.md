# mysh — mini shell

A minimal Unix shell built incrementally as part of `c-lab`.

## Build

```bash
make          # build
make run      # build and run
make valgrind # check for memory leaks
make san      # build with address + UB sanitizers
make clean    # remove binaries
```

## Architecture

```
mysh/
├── include/mysh.h    # all function declarations
├── src/main.c        # main loop
├── src/readline.c    # input reading
├── src/tokenize.c    # string splitting
├── src/builtins.c    # echo, pwd, exit
├── src/exec.c        # fork + exec (phase 04)
└── src/jobs.c        # job control (phase 05)
```

## Versions

| Version | Features | Phase |
|---------|----------|-------|
| v0.1 | Input loop + builtins: echo, pwd, exit | 00-02 |
| v1.0 | `struct Command`, tokenizer refactor | 03 |
| v2.0 | Pipes, redirections, external cmds | 04 |
| v3.0 | Job control, signals, C++ refactor | 05 |
