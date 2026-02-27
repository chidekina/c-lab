#include <stdio.h>
#include <stdlib.h>
#include "mysh.h"

int main(void) {
    char  buf[MAX_INPUT];
    char *argv[MAX_ARGS];
    int   argc;

    printf("mysh v%s — type 'exit' to quit\n", MYSH_VERSION);

    while (1) {
        printf("mysh> ");
        fflush(stdout);

        if (mysh_readline(buf, MAX_INPUT) < 0)
            break;  /* EOF (Ctrl+D) */

        argc = mysh_tokenize(buf, argv);
        if (argc == 0)
            continue;

        if (mysh_builtin(argc, argv))
            continue;

        /* Phase 04: fork + exec goes here */
        fprintf(stderr, "mysh: %s: command not found\n", argv[0]);
    }

    return 0;
}
